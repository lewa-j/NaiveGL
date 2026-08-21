
#include "pch.h"
#include "gl_state.h"
#include "gl_pixels.h"
#include "gl_exports.h"
#include <glm/gtx/integer.hpp>
#include <array>

#define VALIDATE_TEX_LEVEL_(FUNC,LEVEL) \
if (LEVEL < 0 || LEVEL > gl_max_tex_level) \
{ \
	gl_set_error_a_(GL_INVALID_VALUE, LEVEL, FUNC); \
	return; \
}

#define VALIDATE_TEX_IMAGE_(FUNC,LEVEL,W,H,BORDER,BLW,BLH) \
VALIDATE_TEX_LEVEL_(FUNC,LEVEL) \
if (BORDER < 0 || BORDER > 1) \
{ \
	gl_set_error_a_(GL_INVALID_VALUE, BORDER, FUNC); \
	return; \
} \
if ((W) < 0 || (H) < 0) \
{ \
	gl_set_error_(GL_INVALID_VALUE, FUNC); \
	return; \
} \
if ((BLW) < 0 || (BLW) > gl_max_texture_size || !is_pow(BLW)) \
{ \
	gl_set_error_a_(GL_INVALID_VALUE, (W), FUNC); \
	return; \
} \
if ((BLH) < 0 || (BLH) > gl_max_texture_size || !is_pow(BLH)) \
{ \
	gl_set_error_a_(GL_INVALID_VALUE, (H), FUNC); \
	return; \
}

#define VALIDATE_TEX_IMAGE(LEVEL,W,H,BORDER,BLW,BLH) \
VALIDATE_TEX_IMAGE_(__FUNCTION__,LEVEL,W,H,BORDER,BLW,BLH)

#define VALIDATE_TEX_IMAGE_COMPONENTS \
if (components < 1 || components > 4) \
{ \
	gl_set_error_a(GL_INVALID_VALUE, components); \
	return; \
}

#define VALIDATE_TEX_IMAGE_FORMAT \
if (format != GL_COLOR_INDEX && (format < GL_RED || format > GL_LUMINANCE_ALPHA)) \
/*STENCIL_INDEX and DEPTH_COMPONENT are not allowed*/ \
{ \
	gl_set_error_a(GL_INVALID_ENUM, format); \
	return; \
} \
if (type != GL_BITMAP && (type < GL_BYTE || type > GL_FLOAT)) \
{ \
	gl_set_error_a(GL_INVALID_ENUM, type); \
	return; \
} \
if (type == GL_BITMAP && format != GL_COLOR_INDEX) \
{ \
	gl_set_error_a(GL_INVALID_ENUM, format); \
	return; \
}

static bool gl_is_texture_complete(const gl_texture &tex)
{
	int w = tex.arrays[0].width;
	int h = tex.arrays[0].height;
	if (w < 1 || h < 1)
		return false;

	bool mipmap = (tex.params.min_filter == GL_NEAREST_MIPMAP_NEAREST || tex.params.min_filter == GL_NEAREST_MIPMAP_LINEAR
		|| tex.params.min_filter == GL_LINEAR_MIPMAP_NEAREST || tex.params.min_filter == GL_LINEAR_MIPMAP_LINEAR);

	if (!mipmap)
		return true;

	int levels = 1 + (int)log2(glm::max(w, h));

	for (int i = 1; i < levels; i++)
	{
		w = glm::max(1, (w >> 1));
		h = glm::max(1, (h >> 1));
		if (w != tex.arrays[i].width || h != tex.arrays[i].height
#if NGL_VERISON >= 110 || GL_EXT_texture
			|| tex.arrays[i].internal_format != tex.arrays[0].internal_format
#else
			|| tex.arrays[i].components != tex.arrays[0].components
#endif
			|| tex.arrays[i].border != tex.arrays[0].border)
			return false;
	}
	return true;
}

static void gl_tex_store_pixel(const glm::vec4 &col, int components, int base_internal_format, uint8_t *dst)
{
#if NGL_VERISON >= 110 || GL_EXT_texture
	if (base_internal_format == GL_ALPHA)
		dst[0] = (uint8_t)lroundf(0xFF * glm::clamp(col.a, 0.f, 1.f));
	else
#endif
	if (components == 2)
	{
		dst[0] = (uint8_t)lroundf(0xFF * glm::clamp(col.r, 0.f, 1.f));
		dst[1] = (uint8_t)lroundf(0xFF * glm::clamp(col.a, 0.f, 1.f));
	}
	else
	{
		for (int c = 0; c < components; c++)
		{
			dst[c] = (uint8_t)lroundf(0xFF * glm::clamp(col[c], 0.f, 1.f));
		}
	}
}

static void gl_texSubImage(gl_state *gs, gl_texture_array &ta, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const uint8_t *src)
{
	if (!src)
		return;
	if (!ta.data)
		return;

	int components = ta.components;
	size_t size = width * height * components;

	const gl_state::pixelStore &ps = gs->pixel_unpack;
	gl_PixelStoreSetup pstore;
	pstore.init(ps, width, height, format, type);

	src += pstore.skip_bytes;

	int dst_stride = ta.width * components;
	int dst_sub_stride = width * components;

	uint8_t *dst = ta.data;
	dst += yoffset * dst_stride + xoffset * components;

	// fast path
	if (type == GL_UNSIGNED_BYTE && format >= GL_RED && format <= GL_LUMINANCE_ALPHA &&
		!gs->pixel.map_color && gs->pixel.color_scale == glm::vec4{ 1,1,1,1 } && gs->pixel.color_bias == glm::vec4{ 0,0,0,0 })
	{
		if (format >= GL_RGB && format <= GL_LUMINANCE_ALPHA)
		{
			if (components == pstore.components)
			{
				if (pstore.stride == dst_stride && dst_stride == dst_sub_stride)
				{
					memcpy(dst, src, size);
					return;
				}

				for (int j = 0; j < height; j++)
				{
					memcpy(dst, src, dst_sub_stride);
					src += pstore.stride;
					dst += dst_stride;
				}
				return;
			}

			if (components < pstore.components && components != 2
#if NGL_VERISON >= 110 || GL_EXT_texture
				&& ta.base_internal_format != GL_ALPHA
#endif
				)
			{
				if (width == ta.width && pstore.stride == width * pstore.components)
				{
					for (size_t i = 0; i < size; i += components)
					{
						memcpy(dst, src, components);
						src += pstore.components;
						dst += components;
					}
					return;
				}

				for (int j = 0; j < height; j++)
				{
					const uint8_t *row = src;
					uint8_t *dst_row = dst;
					for (int i = 0; i < width; i ++)
					{
						memcpy(dst_row, row, components);
						row += pstore.components;
						dst_row += components;
					}
					src += pstore.stride;
					dst += dst_stride;
				}
				return;
			}
		}

		for (int j = 0; j < height; j++)
		{
			const uint8_t *row = src;
			uint8_t *dst_row = dst;
			for (int i = 0; i < width; i++)
			{
				uint8_t pixel[4]{ 0,0,0,0xff };
				switch (format)
				{
				case GL_RED:
					pixel[0] = row[0]; break;
				case GL_GREEN:
					pixel[1] = row[0]; break;
				case GL_BLUE:
					pixel[2] = row[0]; break;
				case GL_ALPHA:
					pixel[3] = row[0]; break;
				case GL_RGB:
					memcpy(pixel, row, 3); break;
				case GL_RGBA:
					memcpy(pixel, row, 4); break;
				case GL_LUMINANCE:
					pixel[0] = pixel[1] = pixel[2] = row[0]; break;
				case GL_LUMINANCE_ALPHA:
					pixel[0] = pixel[1] = pixel[2] = row[0];
					pixel[3] = row[1];
					break;
				}

#if NGL_VERISON >= 110 || GL_EXT_texture
				if (ta.base_internal_format == GL_ALPHA)
					dst_row[0] = pixel[3];
				else
#endif
				if (components == 2)
				{
					dst_row[0] = pixel[0];
					dst_row[1] = pixel[3];
				}
				else
					memcpy(dst_row, pixel, components);

				row += pstore.components;
				dst_row += components;
			}
			src += pstore.stride;
			dst += dst_stride;
		}

		return;
	}

	gl_log("glTexImage(c %d,%dx%d,b %d,f %X,t %X) al=%d map %d slow path\n", components, width, height, ta.border, format, type, ps.alignment, gs->pixel.map_color);

	int base_internal_format = components;
#if NGL_VERISON >= 110 || GL_EXT_texture
	base_internal_format = ta.base_internal_format;
#endif

	if (type != GL_BITMAP)
	{
		for (int j = 0; j < height; j++)
		{
			const uint8_t *row = src;
			uint8_t *dst_row = dst;
			for (int i = 0; i < width; i++)
			{
				uint8_t group[16];
				memcpy(group, row, pstore.group_size);
				if (ps.swap_bytes && pstore.element_size > 1)
					gl_swap_bytes(pstore.element_size, pstore.components, group);

				glm::vec4 pixel{};
				if (format == GL_COLOR_INDEX)
				{
					uint32_t index = gl_unpack_index_pixel(type, group);
					index = gl_index_arithmetic(gs, index);
					pixel = index_to_rgba(index, gs->pixel_map_color_table);
				}
				else
				{
					pixel = gl_unpack_color_pixel(format, type, group);
					pixel = pixel * gs->pixel.color_scale + gs->pixel.color_bias;
					if (gs->pixel.map_color)
						pixel = remap_color(pixel, gs->pixel_map_color_table + 4);
				}

				gl_tex_store_pixel(pixel, components, base_internal_format, dst_row);

				dst_row += components;
				row += pstore.group_size;
			}
			src += pstore.stride;
			dst += dst_stride;
		}
	}
	else //GL_BITMAP
	{
		uint8_t bitmap_colors[2][4];
		gl_tex_store_pixel(index_to_rgba(0, gs->pixel_map_color_table), components, base_internal_format, bitmap_colors[0]);
		gl_tex_store_pixel(index_to_rgba(1, gs->pixel_map_color_table), components, base_internal_format, bitmap_colors[1]);

		for (int j = 0; j < height; j++)
		{
			const uint8_t *group = src;
			uint8_t *dst_row = dst;

			int pixel = pstore.skip_bits;
			for (int ix = 0; ix < width; ix++)
			{
				bool b = 0;
				if (ps.lsb_first)
					b = !!((*group) & (1 << pixel));
				else
					b = !!((*group) & (0x80 >> pixel));

				memcpy(dst_row, bitmap_colors[b], components);
				dst_row += components;
				pixel++;

				if (pixel >= 8)
				{
					pixel = pixel & 7;
					group++;
				}
			}
			src += pstore.stride;
			dst += dst_stride;
		}
	}
}

static void gl_texImage(gl_state *gs, gl_texture_array &ta, GLenum target, GLint components, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const uint8_t *src)
{
	size_t size = width * height * components;
	size_t old_size = ta.width * ta.height * ta.components;

	ta.width = width;
	ta.height = height;
	ta.components = components;
	ta.border = border;

#if NGL_VERISON >= 110 || GL_EXT_texture
	if (target == GL_PROXY_TEXTURE_1D || target == GL_PROXY_TEXTURE_2D)
		return;
#endif

	if (old_size != size)
	{
		if (ta.data)
			delete[] ta.data;

		ta.data = new uint8_t[size];
	}

	gl_texSubImage(gs, ta, 0, 0, width, height, format, type, src);
}

#if NGL_VERISON >= 110 || GL_EXT_texture
static bool gl_derive_format(int internalformat, GLenum &baseformat, int &components)
{
	switch (internalformat)
	{
	case GL_ALPHA:
	case GL_ALPHA4:
	case GL_ALPHA8:
	case GL_ALPHA12:
	case GL_ALPHA16:
		components = 1;
		baseformat = GL_ALPHA;
		break;
	case 1:
	case GL_LUMINANCE:
	case GL_LUMINANCE4:
	case GL_LUMINANCE8:
	case GL_LUMINANCE12:
	case GL_LUMINANCE16:
		components = 1;
		baseformat = GL_LUMINANCE;
		break;
	case 2:
	case GL_LUMINANCE_ALPHA:
	case GL_LUMINANCE4_ALPHA4:
	case GL_LUMINANCE6_ALPHA2:
	case GL_LUMINANCE8_ALPHA8:
	case GL_LUMINANCE12_ALPHA4:
	case GL_LUMINANCE12_ALPHA12:
	case GL_LUMINANCE16_ALPHA16:
		components = 2;
		baseformat = GL_LUMINANCE_ALPHA;
		break;
	case GL_INTENSITY:
	case GL_INTENSITY4:
	case GL_INTENSITY8:
	case GL_INTENSITY12:
	case GL_INTENSITY16:
		components = 1;
		baseformat = GL_INTENSITY;
		break;
	case 3:
	case GL_RGB:
#if NGL_VERISON >= 110
	case GL_R3_G3_B2:
#endif
#if GL_EXT_texture
	case GL_RGB2_EXT:
#endif
	case GL_RGB4:
	case GL_RGB5:
	case GL_RGB8:
	case GL_RGB10:
	case GL_RGB12:
	case GL_RGB16:
		components = 3;
		baseformat = GL_RGB;
		break;
	case 4:
	case GL_RGBA:
	case GL_RGBA2:
	case GL_RGBA4:
	case GL_RGB5_A1:
	case GL_RGBA8:
	case GL_RGB10_A2:
	case GL_RGBA12:
	case GL_RGBA16:
		components = 4;
		baseformat = GL_RGBA;
		break;
	default:
		return false;
	}
	return true;
}
#endif

void APIENTRY glTexImage2D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void* data)
{
	gl_state *gs = gl_current_state();
	if (!gs) return;
	if (gs->display_list_begun
#if NGL_VERISON >= 110 || GL_EXT_texture
		&& target != GL_PROXY_TEXTURE_2D
#endif
		)
	{
		auto &dl = gs->display_list_indices[0];
		size_t old_size = dl.data.size();
		int pix_size = 0;
		if (data && target == GL_TEXTURE_2D)
			pix_size = gl_pixels_size(width, height, format, type);
		if (pix_size)
		{
			dl.data.resize(old_size + pix_size);
			gl_unpack_pixels(gs, width, height, format, type, data, dl.data.data() + old_size);
		}
		dl.calls.push_back({ gl_display_list_call::tTexImage2D, {(float)target}, {level, internalformat, width, height, border, (int)format, (int)type, pix_size} });
		if (!gs->display_list_execute)
			return;
	}
	VALIDATE_NOT_BEGIN_MODE

	if (target != GL_TEXTURE_2D
#if NGL_VERISON >= 110 || GL_EXT_texture
		&& target != GL_PROXY_TEXTURE_2D
#endif
		)
	{
		gl_set_error_a(GL_INVALID_ENUM, target);
		return;
	}
	int borderless_width = width - border * 2;
	int borderless_height = height - border * 2;
	VALIDATE_TEX_IMAGE(level, width, height, border, borderless_width, borderless_height);
	VALIDATE_TEX_IMAGE_FORMAT;

	GLint components = internalformat;
#if NGL_VERISON >= 110 || GL_EXT_texture
	GLenum baseformat = 0;
	if (!gl_derive_format(internalformat, baseformat, components))
	{
		gl_set_error_a(GL_INVALID_VALUE, internalformat);
		return;
	}
#endif
	VALIDATE_TEX_IMAGE_COMPONENTS;

	gl_texture &tex_params = gs->get_texture_2d();
#if NGL_VERISON >= 110 || GL_EXT_texture
	gl_texture_base &tex = target == GL_PROXY_TEXTURE_2D ? gs->proxy_texture_2d : tex_params;
#else
	gl_texture &tex = gs->texture_2d;
#endif
	gl_texture_array& ta = tex.arrays[level];

	if (width == 0 || height == 0)
	{
		if (ta.data)
			delete[] ta.data;
		ta.data = nullptr;
		ta.width = 0;
		ta.height = 0;
		tex.is_complete = gl_is_texture_complete(tex_params);
		return;
	}

	if (level == 0)
		tex.max_lod = 1 + (int)log2(glm::max(borderless_width, borderless_height));

	if (border != 0)
	{
		gl_log("glTexImage2D(%d,%d,%d,%d,%d,%X,%X) unhandled combination\n", level, components, width, height, border, format, type);
		tex.is_complete = false;
		return;
	}

#if NGL_VERISON >= 110 || GL_EXT_texture
	ta.internal_format = internalformat;
	ta.base_internal_format = baseformat;
#endif
	gl_texImage(gs, ta, target, components, width, height, border, format, type, (const uint8_t *)data);
	tex.is_complete = gl_is_texture_complete(tex_params);
}

void APIENTRY glTexImage1D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLint border, GLenum format, GLenum type, const void *data)
{
	gl_state *gs = gl_current_state();
	if (!gs) return;
	if (gs->display_list_begun
#if NGL_VERISON >= 110 || GL_EXT_texture
		&& target != GL_PROXY_TEXTURE_1D
#endif
		)
	{
		auto &dl = gs->display_list_indices[0];
		size_t old_size = dl.data.size();
		int pix_size = 0;
		if (data && target == GL_TEXTURE_1D)
			pix_size = gl_pixels_size(width, 1, format, type);
		if (pix_size)
		{
			dl.data.resize(old_size + pix_size);
			gl_unpack_pixels(gs, width, 1, format, type, data, dl.data.data() + old_size);
		}
		dl.calls.push_back({ gl_display_list_call::tTexImage1D, {}, {(int)target, level, internalformat, width, border, (int)format, (int)type, pix_size} });
		if (!gs->display_list_execute)
			return;
	}
	VALIDATE_NOT_BEGIN_MODE

	if (target != GL_TEXTURE_1D
#if NGL_VERISON >= 110 || GL_EXT_texture
		&& target != GL_PROXY_TEXTURE_1D
#endif
		)
	{
		gl_set_error_a(GL_INVALID_ENUM, target);
		return;
	}

	int borderless_width = width - border * 2;
	VALIDATE_TEX_IMAGE(level, width, 1, border, borderless_width, 1);
	VALIDATE_TEX_IMAGE_FORMAT;

	GLint components = internalformat;
#if NGL_VERISON >= 110 || GL_EXT_texture
	GLenum baseformat = 0;
	if (!gl_derive_format(internalformat, baseformat, components))
	{
		gl_set_error_a(GL_INVALID_VALUE, internalformat);
		return;
	}
#endif
	VALIDATE_TEX_IMAGE_COMPONENTS;

	gl_texture &tex_params = gs->get_texture_1d();
#if NGL_VERISON >= 110 || GL_EXT_texture
	gl_texture_base &tex = target == GL_PROXY_TEXTURE_1D ? gs->proxy_texture_1d : tex_params;
#else
	gl_texture &tex = gs->texture_1d;
#endif
	gl_texture_array &ta = tex.arrays[level];

	if (width == 0)
	{
		if (ta.data)
			delete[] ta.data;
		ta.data = nullptr;
		ta.width = 0;
		ta.height = 0;
		tex.is_complete = gl_is_texture_complete(tex_params);
		return;
	}

	if (level == 0)
		tex.max_lod = 1 + (int)log2(borderless_width);

	if (border != 0)
	{
		gl_log("glTexImage1D(%d,%d,%d,%d,%X,%X) unhandled combination\n", level, components, width, border, format, type);
		tex.is_complete = false;
		return;
	}

#if NGL_VERISON >= 110 || GL_EXT_texture
	ta.internal_format = internalformat;
	ta.base_internal_format = baseformat;
#endif
	gl_texImage(gs, ta, target, components, width, 1, border, format, type, (const uint8_t *)data);
	tex.is_complete = gl_is_texture_complete(tex_params);
}

#if NGL_VERISON >= 110
static void gl_copyTexImage(gl_state *gs, const char *func, GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border)
{
	if (gs->begin_primitive_mode != -1)
	{
		gl_set_error_(GL_INVALID_OPERATION, func);
		return;
	}
	int borderless_width = width - border * 2;
	int borderless_height = (target == GL_TEXTURE_1D) ? 1 : (height - border * 2);
	VALIDATE_TEX_IMAGE_(func, level, width, height, border, borderless_width, borderless_height);

	GLint components = 0;
	GLenum baseformat = 0;
	if ((internalformat >= 1 && internalformat <= 4) || !gl_derive_format(internalformat, baseformat, components))
	{
		gl_set_error_a_(GL_INVALID_ENUM, internalformat, func);
		return;
	}

	// prevent glDrawPixels from writing into display list
	int save_dl = gs->display_list_begun;
	gs->display_list_begun = 0;

	// may be optimized

	gl_state::pixelStore save_pack{};
	gl_state::pixelStore save_unpack{};
	std::swap(save_pack, gs->pixel_pack);
	std::swap(save_unpack, gs->pixel_unpack);
	gl_state::pixel_t save_pixel{};
	std::swap(save_pixel, gs->pixel);
	std::vector<uint8_t> pixels(width * height * 4);
	glReadPixels(x, y, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
	gs->pixel = save_pixel;
	if (target == GL_TEXTURE_2D)
		glTexImage2D(target, level, internalformat, width, height, border, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
	else
		glTexImage1D(target, level, internalformat, width, border, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());

	gs->pixel_pack = save_pack;
	gs->pixel_unpack = save_unpack;

	gs->display_list_begun = save_dl;
}

void APIENTRY glCopyTexImage2D(GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border)
{
	gl_state *gs = gl_current_state();
	if (!gs) return;
	WRITE_DISPLAY_LIST(CopyTexImage2D, {}, { (int)target, level, (int)internalformat, x, y, width, height, border });
	if (target != GL_TEXTURE_2D)
	{
		gl_set_error_a(GL_INVALID_ENUM, target);
		return;
	}
	gl_copyTexImage(gs, __FUNCTION__, target, level, internalformat, x, y, width, height, border);
}

void APIENTRY glCopyTexImage1D(GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLint border)
{
	gl_state *gs = gl_current_state();
	if (!gs) return;
	WRITE_DISPLAY_LIST(CopyTexImage1D, {}, { (int)target, level, (int)internalformat, x, y, width, border });
	if (target != GL_TEXTURE_1D)
	{
		gl_set_error_a(GL_INVALID_ENUM, target);
		return;
	}
	gl_copyTexImage(gs, __FUNCTION__, target, level, internalformat, x, y, width, 1, border);
}
#endif

#if NGL_VERISON >= 110 || GL_EXT_subtexture

#define VALIDATE_TEX_SUB_IMAGE(FUNC,TARGET,LEVEL,W,H) \
if (target != TARGET) \
{ \
	gl_set_error_a_(GL_INVALID_ENUM, target, FUNC); \
	return; \
} \
VALIDATE_TEX_LEVEL_(FUNC,LEVEL) \
if ((W) < 0 || (H) < 0) \
{ \
	gl_set_error_(GL_INVALID_VALUE, FUNC); \
	return; \
}

void APIENTRY glTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void *pixels)
{
	gl_state *gs = gl_current_state();
	if (!gs) return;
	if (gs->display_list_begun)
	{
		auto &dl = gs->display_list_indices[0];
		size_t old_size = dl.data.size();
		int pix_size = 0;
		if (pixels && target == GL_TEXTURE_2D)
			pix_size = gl_pixels_size(width, height, format, type);
		if (pix_size)
		{
			dl.data.resize(old_size + pix_size);
			gl_unpack_pixels(gs, width, height, format, type, pixels, dl.data.data() + old_size);
		}
		dl.calls.push_back({ gl_display_list_call::tTexSubImage2D, {(float)target}, {level, xoffset, yoffset, width, height, (int)format, (int)type, pix_size} });
		if (!gs->display_list_execute)
			return;
	}
	VALIDATE_NOT_BEGIN_MODE;
	VALIDATE_TEX_SUB_IMAGE(__FUNCTION__, GL_TEXTURE_2D, level, width, height);
	VALIDATE_TEX_IMAGE_FORMAT;

	gl_texture &tex = gs->get_texture_2d();
	gl_texture_array &ta = tex.arrays[level];

	if (xoffset < -ta.border
		|| xoffset + width > ta.width - ta.border
		|| yoffset < -ta.border
		|| yoffset + height > ta.height - ta.border)
	{
		gl_set_error(GL_INVALID_VALUE);
		return;
	}

	gl_texSubImage(gs, ta, xoffset, yoffset, width, height, format, type, (const uint8_t*)pixels);
}

void APIENTRY glTexSubImage1D(GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const void *pixels)
{
	gl_state *gs = gl_current_state();
	if (!gs) return;
	if (gs->display_list_begun)
	{
		auto &dl = gs->display_list_indices[0];
		size_t old_size = dl.data.size();
		int pix_size = 0;
		if (pixels && target == GL_TEXTURE_1D)
			pix_size = gl_pixels_size(width, 1, format, type);
		if (pix_size)
		{
			dl.data.resize(old_size + pix_size);
			gl_unpack_pixels(gs, width, 1, format, type, pixels, dl.data.data() + old_size);
		}
		dl.calls.push_back({ gl_display_list_call::tTexSubImage1D, {}, {(int)target, level, xoffset, width, (int)format, (int)type, pix_size} });
		if (!gs->display_list_execute)
			return;
	}
	VALIDATE_NOT_BEGIN_MODE;
	VALIDATE_TEX_SUB_IMAGE(__FUNCTION__, GL_TEXTURE_1D, level, width, 1);
	VALIDATE_TEX_IMAGE_FORMAT;

	gl_texture &tex = gs->get_texture_1d();
	gl_texture_array &ta = tex.arrays[level];

	if (xoffset < -ta.border
		|| xoffset + width > ta.width - ta.border)
	{
		gl_set_error(GL_INVALID_VALUE);
		return;
	}

	gl_texSubImage(gs, ta, xoffset, 0, width, 1, format, type, (const uint8_t *)pixels);
}

#if GL_EXT_subtexture
void APIENTRY glTexSubImage1DEXT(GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const void *pixels)
{
	glTexSubImage1D(target, level, xoffset, width, format, type, pixels);
}
void APIENTRY glTexSubImage2DEXT(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void *pixels)
{
	glTexSubImage2D(target, level, xoffset, yoffset, width, height, format, type, pixels);
}
#if GL_EXT_texture3D
void APIENTRY glTexSubImage3DEXT(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void *pixels)
{
#error "glTexSubImage3DEXT unimplemented"
}
#endif
#endif

#endif

#if NGL_VERISON >= 110
void APIENTRY glCopyTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height)
{
	gl_state *gs = gl_current_state();
	if (!gs) return;
	WRITE_DISPLAY_LIST(CopyTexSubImage2D, {}, { (int)target, level, xoffset, yoffset, x, y, width, height });
	VALIDATE_NOT_BEGIN_MODE;
	VALIDATE_TEX_SUB_IMAGE(__FUNCTION__, GL_TEXTURE_2D, level, width, height);

	gl_texture &tex = gs->get_texture_2d();
	gl_texture_array &ta = tex.arrays[level];

	if (xoffset < -ta.border
		|| xoffset + width > ta.width - ta.border
		|| yoffset < -ta.border
		|| yoffset + height > ta.height - ta.border)
	{
		gl_set_error(GL_INVALID_VALUE);
		return;
	}

	gl_state::pixelStore save_pack{};
	gl_state::pixelStore save_unpack{};
	std::swap(save_pack, gs->pixel_pack);
	std::swap(save_unpack, gs->pixel_unpack);
	gl_state::pixel_t save_pixel{};
	std::swap(save_pixel, gs->pixel);

	std::vector<uint8_t> pixels(width * height * 4);
	glReadPixels(x, y, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
	gs->pixel = save_pixel;
	gl_texSubImage(gs, ta, xoffset, yoffset, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());

	gs->pixel_pack = save_pack;
	gs->pixel_unpack = save_unpack;
}

void APIENTRY glCopyTexSubImage1D(GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width)
{
	gl_state *gs = gl_current_state();
	if (!gs) return;
	WRITE_DISPLAY_LIST(CopyTexSubImage1D, {}, { (int)target, level, xoffset, x, y, width });
	VALIDATE_NOT_BEGIN_MODE;
	VALIDATE_TEX_SUB_IMAGE(__FUNCTION__, GL_TEXTURE_1D, level, width, 1);

	gl_texture &tex = gs->get_texture_1d();
	gl_texture_array &ta = tex.arrays[level];

	if (xoffset < -ta.border
		|| xoffset + width > ta.width - ta.border)
	{
		gl_set_error(GL_INVALID_VALUE);
		return;
	}

	gl_state::pixelStore save_pack{};
	gl_state::pixelStore save_unpack{};
	std::swap(save_pack, gs->pixel_pack);
	std::swap(save_unpack, gs->pixel_unpack);
	gl_state::pixel_t save_pixel{};
	std::swap(save_pixel, gs->pixel);

	std::vector<uint8_t> pixels(width * 4);
	glReadPixels(x, y, width, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
	gs->pixel = save_pixel;
	gl_texSubImage(gs, ta, xoffset, 0, width, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());

	gs->pixel_pack = save_pack;
	gs->pixel_unpack = save_unpack;
}
#endif

void APIENTRY glGetTexImage(GLenum target, GLint level, GLenum format, GLenum type, void *pixels)
{
	gl_state *gs = gl_current_state();
	if (!gs) return;
	VALIDATE_NOT_BEGIN_MODE;
	if (target != GL_TEXTURE_1D && target != GL_TEXTURE_2D)
	{
		gl_set_error_a(GL_INVALID_ENUM, target);
		return;
	}
	if (level < 0 || level > gl_max_tex_level)
	{
		gl_set_error_a(GL_INVALID_VALUE, level);
		return;
	}
	// *_INDEX and DEPTH_COMPONENT are not allowed
	if (format < GL_RED || format > GL_LUMINANCE_ALPHA)
	{
		gl_set_error_a(GL_INVALID_ENUM, target);
		return;
	}
	if (type < GL_BYTE || type > GL_FLOAT)
	{
		gl_set_error_a(GL_INVALID_ENUM, type);
		return;
	}

	const gl_texture &tex = gs->get_texture(target);
	const gl_texture_array &ta = tex.arrays[level];

	if (!ta.data)
		return;

	uint8_t *dst = (uint8_t *)pixels;

	const gl_state::pixelStore &ps = gs->pixel_pack;
	gl_PixelStoreSetup pstore;
	pstore.init(ps, ta.width, ta.height, format, type);

	dst += pstore.skip_bytes;

	const uint8_t *src = ta.data;

	for (int iy = 0; iy < ta.height; iy++)
	{
		uint8_t *row = dst;
		for (int ix = 0; ix < ta.width; ix++)
		{
			glm::vec4 col(0);
			for (int ci = 0; ci < ta.components; ci++)
				col[ci] = GLtof(src[ci]);

			if (ta.components == 2)
				std::swap(col.g, col.a);
#if NGL_VERISON >= 110 || GL_EXT_texture
			else if (ta.base_internal_format == GL_ALPHA)
				col = glm::vec4(0, 0, 0, col.r);

			if (ta.base_internal_format == GL_LUMINANCE
				|| ta.base_internal_format == GL_RGB
				|| ta.base_internal_format == GL_INTENSITY)
				col.a = 1;
#endif

			col = col * gs->pixel.color_scale + gs->pixel.color_bias;

			if (gs->pixel.map_color)
				col = remap_color(col, gs->pixel_map_color_table + 4);
			col = gl_pixel_format_conversion(format, col);

			gl_pack_color_pixel(pstore.components, type, col, row);

			if (ps.swap_bytes && pstore.element_size > 1)
				gl_swap_bytes(pstore.element_size, pstore.components, row);

			row += pstore.group_size;
			src += ta.components;
		}
		dst += pstore.stride;
	}
}

#if NGL_VERISON >= 110
#define VALIDATE_TEX_PARAMETER_PNAME_CHECK_ \
(pname < GL_TEXTURE_MAG_FILTER || pname > GL_TEXTURE_WRAP_T) && pname != GL_TEXTURE_BORDER_COLOR && pname != GL_TEXTURE_PRIORITY
#define VALIDATE_GET_TEX_PARAMETER_PNAME_CHECK_ \
VALIDATE_TEX_PARAMETER_PNAME_CHECK_ && pname != GL_TEXTURE_RESIDENT
#else
#define VALIDATE_TEX_PARAMETER_PNAME_CHECK_ \
(pname < GL_TEXTURE_MAG_FILTER || pname > GL_TEXTURE_WRAP_T) && pname != GL_TEXTURE_BORDER_COLOR
#define VALIDATE_GET_TEX_PARAMETER_PNAME_CHECK_ VALIDATE_TEX_PARAMETER_PNAME_CHECK_
#endif

#define VALIDATE_TEX_PARAMETER \
VALIDATE_NOT_BEGIN_MODE; \
if (target != GL_TEXTURE_1D && target != GL_TEXTURE_2D) \
{ \
	gl_set_error_a(GL_INVALID_ENUM, target); \
	return; \
} \
if (VALIDATE_TEX_PARAMETER_PNAME_CHECK_) \
{ \
	gl_set_error_a(GL_INVALID_ENUM, pname); \
	return; \
}

#define VALIDATE_TEX_PARAMETER_PARAM(p) \
if ((pname == GL_TEXTURE_WRAP_S || pname == GL_TEXTURE_WRAP_T) && ((p) != GL_CLAMP && (p) != GL_REPEAT)) \
{ \
	gl_set_error_a(GL_INVALID_ENUM, (p)); \
	return; \
} \
if ((pname == GL_TEXTURE_MAG_FILTER && (p) != GL_NEAREST && (p) != GL_LINEAR) \
	|| (pname == GL_TEXTURE_MIN_FILTER && (p) != GL_NEAREST && (p) != GL_LINEAR && ((p) < GL_NEAREST_MIPMAP_NEAREST || (p) > GL_LINEAR_MIPMAP_LINEAR))) \
{ \
	gl_set_error_a(GL_INVALID_ENUM, (p)); \
	return; \
}

void APIENTRY glTexParameteri(GLenum target, GLenum pname, GLint param)
{
	glTexParameterf(target, pname, (float)param);
}
void APIENTRY glTexParameterf(GLenum target, GLenum pname, GLfloat param)
{
	gl_state *gs = gl_current_state();
	if (!gs) return;
	WRITE_DISPLAY_LIST(TexParameter, { param }, { (int)target, (int)pname });
	VALIDATE_TEX_PARAMETER;
	VALIDATE_TEX_PARAMETER_PARAM((int)param);
	if (pname == GL_TEXTURE_BORDER_COLOR)
	{
		gl_set_error_a(GL_INVALID_ENUM, pname);
		return;
	}

	gl_texture &tex = gs->get_texture(target);
	gl_texture::params_t &p = tex.params;

	if (pname == GL_TEXTURE_MAG_FILTER)
		p.mag_filter = to_int(param);
	else if (pname == GL_TEXTURE_MIN_FILTER)
		p.min_filter = to_int(param);
	else if (pname == GL_TEXTURE_WRAP_S)
		p.wrap_s = to_int(param);
	else if (pname == GL_TEXTURE_WRAP_T)
		p.wrap_t = to_int(param);
#if NGL_VERISON >= 110
	else if (pname == GL_TEXTURE_PRIORITY)
		tex.params.priority = glm::clamp(param, 0.f, 1.f);
#endif

	if (pname == GL_TEXTURE_MIN_FILTER)
		tex.is_complete = gl_is_texture_complete(tex);
}

static int gl_texParameterv_size(GLenum pname)
{
	if (pname >= GL_TEXTURE_MAG_FILTER && pname <= GL_TEXTURE_WRAP_T)
		return 1;
	if (pname == GL_TEXTURE_BORDER_COLOR)
		return 4;
#if NGL_VERISON >= 110
	if (pname == GL_TEXTURE_PRIORITY)
		return 1;
#endif
	return 0;
}

template<typename T>
void gl_texParameterv(gl_state *gs, GLenum target, GLenum pname, const T* params)
{
	VALIDATE_TEX_PARAMETER;
	VALIDATE_TEX_PARAMETER_PARAM(int(params[0]));

	gl_texture& tex = gs->get_texture(target);

	if (pname == GL_TEXTURE_MAG_FILTER)
		tex.params.mag_filter = to_int(params[0]);
	else if (pname == GL_TEXTURE_MIN_FILTER)
		tex.params.min_filter = to_int(params[0]);
	else if (pname == GL_TEXTURE_WRAP_S)
		tex.params.wrap_s = to_int(params[0]);
	else if (pname == GL_TEXTURE_WRAP_T)
		tex.params.wrap_t = to_int(params[0]);
	else if (pname == GL_TEXTURE_BORDER_COLOR)
		tex.params.border_color = glm::clamp(glm::vec4(GLtof(params[0]), GLtof(params[1]), GLtof(params[2]), GLtof(params[3])), glm::vec4(0), glm::vec4(1));
#if NGL_VERISON >= 110
	else if (pname == GL_TEXTURE_PRIORITY)
		tex.params.priority = (float)params[0];
#endif

	if (pname == GL_TEXTURE_MIN_FILTER)
		tex.is_complete = gl_is_texture_complete(tex);
}

void APIENTRY glTexParameteriv(GLenum target, GLenum pname, const GLint* params)
{
	gl_state *gs = gl_current_state();
	if (!gs) return;
	WRITE_DISPLAY_LIST_IV(TexParameteriv, params, gl_texParameterv_size(pname), 2, {}, { (int)target, (int)pname });
	gl_texParameterv(gs, target, pname, params);
}
void APIENTRY glTexParameterfv(GLenum target, GLenum pname, const GLfloat* params)
{
	gl_state *gs = gl_current_state();
	if (!gs) return;
	WRITE_DISPLAY_LIST_FV(TexParameterfv, params, gl_texParameterv_size(pname), {}, { (int)target, (int)pname });
	gl_texParameterv(gs, target, pname, params);
}

template<typename T>
void gl_getTexParameterv(GLenum target, GLenum pname, T *params)
{
	gl_state *gs = gl_current_state();
	if (!gs) return;
	VALIDATE_NOT_BEGIN_MODE;
	if (target != GL_TEXTURE_1D && target != GL_TEXTURE_2D)
	{
		gl_set_error_a(GL_INVALID_ENUM, target);
		return;
	}
	if (VALIDATE_GET_TEX_PARAMETER_PNAME_CHECK_)
	{
		gl_set_error_a(GL_INVALID_ENUM, pname);
		return;
	}

	gl_texture &tex = gs->get_texture(target);

	if (pname == GL_TEXTURE_MAG_FILTER)
		*params = (T)tex.params.mag_filter;
	else if (pname == GL_TEXTURE_MIN_FILTER)
		*params = (T)tex.params.min_filter;
	else if (pname == GL_TEXTURE_WRAP_S)
		*params = (T)tex.params.wrap_s;
	else if (pname == GL_TEXTURE_WRAP_T)
		*params = (T)tex.params.wrap_t;
	else if (pname == GL_TEXTURE_BORDER_COLOR)
		copy_color(params, &tex.params.border_color.x);
#if NGL_VERISON >= 110
	else if (pname == GL_TEXTURE_PRIORITY)
		*params = (T)tex.params.priority;
	else if (pname == GL_TEXTURE_RESIDENT)
		*params = (T)GL_TRUE;
#endif
}

void APIENTRY glGetTexParameteriv(GLenum target, GLenum pname, GLint *params)
{
	gl_getTexParameterv(target, pname, params);
}

void APIENTRY glGetTexParameterfv(GLenum target, GLenum pname, GLfloat *params)
{
	gl_getTexParameterv(target, pname, params);
}

static gl_texture_base &gl_get_texture_or_proxy(gl_state *gs, GLenum target)
{
	if (target == GL_TEXTURE_1D)
		return gs->get_texture_1d();
	else if (target == GL_TEXTURE_2D)
		return gs->get_texture_2d();
#if NGL_VERISON >= 110 || GL_EXT_texture
	else if (target == GL_PROXY_TEXTURE_1D)
		return gs->proxy_texture_1d;
	else if (target == GL_PROXY_TEXTURE_2D)
		return gs->proxy_texture_2d;
#endif
	fprintf(stderr, "gl_get_texture_or_proxy invalid target 0x%X\n", target);
	abort();
}

#if NGL_VERISON >= 110 || GL_EXT_texture
static int get_base_format_component_bits(GLenum fmt, GLenum pname)
{
	if (pname < GL_TEXTURE_RED_SIZE || pname > GL_TEXTURE_INTENSITY_SIZE)
		return 0;

	static std::unordered_map<int, std::array<int, 6>> formats{
		{GL_LUMINANCE,		{0,0,0,0,8,0}},
		{GL_LUMINANCE_ALPHA,{0,0,0,8,8,0}},
		{GL_RGB,			{8,8,8,0,0,0}},
		{GL_RGBA,			{8,8,8,8,0,0}},
		{GL_ALPHA,			{0,0,0,8,0,0}},
		{GL_INTENSITY,		{0,0,0,0,0,8}},
		};

	if (formats.find(fmt) == formats.end())
		return 0;

	return formats[fmt][pname - GL_TEXTURE_RED_SIZE];
}
#endif

template<typename T>
void gl_getTexLevelParameterv(GLenum target, GLint level, GLenum pname, T *params)
{
	gl_state *gs = gl_current_state();
	if (!gs) return;
	VALIDATE_NOT_BEGIN_MODE;
	if (target != GL_TEXTURE_1D && target != GL_TEXTURE_2D
#if NGL_VERISON >= 110 || GL_EXT_texture
		&& target != GL_PROXY_TEXTURE_1D && target != GL_PROXY_TEXTURE_2D
#endif
		)
	{
		gl_set_error_a(GL_INVALID_ENUM, target);
		return;
	}
	if (level < 0 || level > gl_max_tex_level)
	{
		gl_set_error_a(GL_INVALID_VALUE, level);
		return;
	}

	gl_texture_array &ta = gl_get_texture_or_proxy(gs, target).arrays[level];

	if (pname == GL_TEXTURE_WIDTH)
		*params = (T)ta.width;
	else if (pname == GL_TEXTURE_HEIGHT)
		*params = (T)ta.height;
	else if (pname == GL_TEXTURE_BORDER)
		*params = (T)ta.border;
#if NGL_VERISON >= 110 || GL_EXT_texture
	else if (pname == GL_TEXTURE_INTERNAL_FORMAT)
		*params = (T)ta.internal_format;
	else if (pname >= GL_TEXTURE_RED_SIZE && pname <= GL_TEXTURE_INTENSITY_SIZE)
		*params = (T)get_base_format_component_bits(ta.base_internal_format, pname);
#else
	else if (pname == GL_TEXTURE_COMPONENTS)
		*params = (T)ta.components;
#endif
	else
		gl_set_error_a(GL_INVALID_ENUM, target);
}

void APIENTRY glGetTexLevelParameteriv(GLenum target, GLint level, GLenum pname, GLint *params)
{
	gl_getTexLevelParameterv(target, level, pname, params);
}

void APIENTRY glGetTexLevelParameterfv(GLenum target, GLint level, GLenum pname, GLfloat *params)
{
	gl_getTexLevelParameterv(target, level, pname, params);
}

#if NGL_VERISON >= 110
void APIENTRY glBindTexture(GLenum target, GLuint texture)
{
	gl_state *gs = gl_current_state();
	if (!gs) return;
	WRITE_DISPLAY_LIST(BindTexture, {}, { (int)target, (int)texture });
	VALIDATE_NOT_BEGIN_MODE;

	if (target != GL_TEXTURE_1D && target != GL_TEXTURE_2D)
	{
		gl_set_error_a(GL_INVALID_ENUM, target);
		return;
	}

	if (texture != 0)
	{
		auto fit = gs->texture_objects.find(texture);

		if (fit == gs->texture_objects.end() || fit->second.target == 0)
		{
			// unallocated or allocated with GenTextures but unitialized
			gs->texture_objects[texture].target = target;
		}
		else if (fit->second.target != target)
		{
			gl_set_error(GL_INVALID_OPERATION);
			return;
		}
	}

	if (target == GL_TEXTURE_1D)
		gs->texture_binding_1d = texture;
	else if(target == GL_TEXTURE_2D)
		gs->texture_binding_2d = texture;
}

void APIENTRY glDeleteTextures(GLsizei n, const GLuint *textures)
{
	gl_state *gs = gl_current_state();
	if (!gs) return;
	VALIDATE_NOT_BEGIN_MODE;

	if (n < 0)
	{
		gl_set_error(GL_INVALID_VALUE);
		return;
	}

	for (int ti = 0; ti < n; ti++)
	{
		if (textures[ti] == 0)
			continue;

		if (gs->texture_binding_1d == textures[ti])
			gs->texture_binding_1d = 0;
		if (gs->texture_binding_2d == textures[ti])
			gs->texture_binding_2d = 0;
		auto it = gs->texture_objects.find(textures[ti]);
		if (it != gs->texture_objects.end())
		{
			gl_log("glDeleteTextures %d\n", textures[ti]);
			gs->texture_objects.erase(it);
		}
	}
}

void APIENTRY glGenTextures(GLsizei n, GLuint *textures)
{
	gl_state *gs = gl_current_state();
	if (!gs) return;
	VALIDATE_NOT_BEGIN_MODE;

	if (n < 0)
	{
		gl_set_error(GL_INVALID_VALUE);
		return;
	}

	int last_unused_texture = 1;
	for (int i = 0; i < n; last_unused_texture++)
	{
		auto it = gs->texture_objects.find(last_unused_texture);
		if (it == gs->texture_objects.end())
		{
			textures[i] = last_unused_texture;
			gs->texture_objects[last_unused_texture];
			i++;
		}
	}
}

GLboolean APIENTRY glAreTexturesResident(GLsizei n, const GLuint *textures, GLboolean *residences)
{
	gl_state *gs = gl_current_state();
	if (!gs) return GL_FALSE;
	VALIDATE_NOT_BEGIN_MODE_RET(GL_FALSE);

	if (n < 0)
	{
		gl_set_error(GL_INVALID_VALUE);
		return GL_FALSE;
	}

	for (int i = 0; i < n; i++)
	{
		auto it = gs->texture_objects.find(textures[i]);
		if (it == gs->texture_objects.end())
		{
			gl_set_error(GL_INVALID_VALUE);
			return GL_FALSE;
		}
	}

	return GL_TRUE;
}

void APIENTRY glPrioritizeTextures(GLsizei n, const GLuint *textures, const GLfloat *priorities)
{
	gl_state *gs = gl_current_state();
	if (!gs) return;
	if (gs->display_list_begun)
	{
		auto &dl = gs->display_list_indices[0];
		int s = (sizeof(GLuint) + sizeof(GLfloat)) * n;
		if (s)
		{
			size_t old_size = dl.data.size();
			dl.data.resize(old_size + s);
			memcpy(dl.data.data() + old_size, textures, sizeof(GLuint) * n);
			old_size += sizeof(GLuint) * n;
			memcpy(dl.data.data() + old_size, priorities, sizeof(GLfloat) * n);
			dl.calls.push_back({ gl_display_list_call::tPrioritizeTextures, {}, { n, s } });
		}
		if (!gs->display_list_execute)
			return;
	}
	VALIDATE_NOT_BEGIN_MODE;

	if (n < 0)
	{
		gl_set_error(GL_INVALID_VALUE);
		return;
	}

	for (int i = 0; i < n; i++)
	{
		if (textures[i] == 0)
			continue;
		auto it = gs->texture_objects.find(textures[i]);
		if (it != gs->texture_objects.end())
		{
			it->second.data.params.priority = glm::clamp(priorities[i], 0.f, 1.f);
		}
	}
}

GLboolean APIENTRY glIsTexture(GLuint texture)
{
	gl_state *gs = gl_current_state();
	if (!gs) return GL_FALSE;
	VALIDATE_NOT_BEGIN_MODE_RET(GL_FALSE);
	
	if (texture == 0)
		return GL_FALSE;

	auto it = gs->texture_objects.find(texture);
	if (it == gs->texture_objects.end() || it->second.target == 0)
		return GL_FALSE;

	return GL_TRUE;
}
#endif

#define VALIDATE_TEX_ENV \
VALIDATE_NOT_BEGIN_MODE; \
if (target != GL_TEXTURE_ENV) \
{ \
	gl_set_error_a(GL_INVALID_ENUM, target); \
	return; \
} \
if (pname != GL_TEXTURE_ENV_MODE && pname != GL_TEXTURE_ENV_COLOR) \
{ \
	gl_set_error_a(GL_INVALID_ENUM, pname); \
	return; \
}

static bool is_valid_tex_env_mode(GLenum p)
{
	switch (p)
	{
	case GL_MODULATE:
	case GL_DECAL:
	case GL_BLEND:
#if GL_EXT_texture
	case GL_REPLACE_EXT:
#endif
#if NGL_VERISON >= 110
	case GL_REPLACE:
#endif
		return true;
	default:
		return false;
	}
}

#define VALIDATE_TEX_ENV_PARAM(p) \
if (pname == GL_TEXTURE_ENV_MODE && !is_valid_tex_env_mode(p)) \
{ \
	gl_set_error_a(GL_INVALID_ENUM, p); \
	return; \
}

void APIENTRY glTexEnvi(GLenum target, GLenum pname, GLint param)
{
	gl_state *gs = gl_current_state();
	if (!gs) return;
	WRITE_DISPLAY_LIST(TexEnv, {}, { (int)target, (int)pname, param });
	VALIDATE_TEX_ENV;
	VALIDATE_TEX_ENV_PARAM(param);

	if (pname == GL_TEXTURE_ENV_COLOR)
	{
		gl_set_error_a(GL_INVALID_ENUM, pname);
		return;
	}

	if (pname == GL_TEXTURE_ENV_MODE)
	{
		gs->texture_env.mode = param;
	}
}
void APIENTRY glTexEnvf(GLenum target, GLenum pname, GLfloat param)
{
	glTexEnvi(target, pname, to_int(param));
}

static int gl_texEnvv_size(GLenum pname)
{
	if (pname == GL_TEXTURE_ENV_MODE)
		return 1;
	if (pname == GL_TEXTURE_ENV_COLOR)
		return 4;
	return 0;
}

void APIENTRY glTexEnviv(GLenum target, GLenum pname, const GLint* params)
{
	gl_state *gs = gl_current_state();
	if (!gs) return;
	WRITE_DISPLAY_LIST_IV(TexEnviv, params, gl_texEnvv_size(pname), 2, {}, { (int)target, (int)pname });
	VALIDATE_TEX_ENV;
	VALIDATE_TEX_ENV_PARAM(params[0]);

	if (pname == GL_TEXTURE_ENV_MODE)
		gs->texture_env.mode = params[0];
	else if (pname == GL_TEXTURE_ENV_COLOR)
		gs->texture_env.color = glm::vec4(GLtof(params[0]), GLtof(params[1]), GLtof(params[2]), GLtof(params[3]));
}

void APIENTRY glTexEnvfv(GLenum target, GLenum pname, const GLfloat* params)
{
	gl_state *gs = gl_current_state();
	if (!gs) return;
	WRITE_DISPLAY_LIST_FV(TexEnvfv, params, gl_texEnvv_size(pname), {}, { (int)target, (int)pname });
	VALIDATE_TEX_ENV;
	VALIDATE_TEX_ENV_PARAM((GLenum)params[0]);

	if (pname == GL_TEXTURE_ENV_MODE)
		gs->texture_env.mode = to_int(params[0]);
	else if (pname == GL_TEXTURE_ENV_COLOR)
		gs->texture_env.color = glm::vec4(params[0], params[1], params[2], params[3]);
}

template<typename T>
void gl_getTexEnvv(GLenum target, GLenum pname, T *params)
{
	gl_state *gs = gl_current_state();
	if (!gs) return;
	VALIDATE_TEX_ENV;

	if (pname == GL_TEXTURE_ENV_MODE)
		*params = (T)gs->texture_env.mode;
	else if (pname == GL_TEXTURE_ENV_COLOR)
		copy_color(params, &gs->texture_env.color.x);
}

void APIENTRY glGetTexEnviv(GLenum target, GLenum pname, GLint *params)
{
	gl_getTexEnvv(target, pname, params);
}

void APIENTRY glGetTexEnvfv(GLenum target, GLenum pname, GLfloat *params)
{
	gl_getTexEnvv(target, pname, params);
}

gl_texture &gl_state::get_texture(GLenum target)
{
	if (target == GL_TEXTURE_1D)
		return get_texture_1d();
	else if (target == GL_TEXTURE_2D)
		return get_texture_2d();

	fprintf(stderr, "gl_state::get_texture invalid target 0x%X\n", target);
	abort();
}

gl_texture &gl_state::get_texture_1d()
{
#if NGL_VERISON >= 110
	if (texture_binding_1d)
	{
		return texture_objects[texture_binding_1d].data;
	}
#endif
	return texture_1d;
}

gl_texture &gl_state::get_texture_2d()
{
#if NGL_VERISON >= 110
	if (texture_binding_2d)
	{
		return texture_objects[texture_binding_2d].data;
	}
#endif
	return texture_2d;
}

glm::vec4 gl_tex_tap(const gl_texture_array& a, glm::ivec2 uv)
{
	glm::ivec2 c{ glm::clamp(uv, glm::ivec2(0), glm::ivec2(a.width - 1, a.height - 1)) };
	uint8_t* d = a.data + (c.y * a.width + c.x) * a.components;
	
#if NGL_VERISON >= 110 || GL_EXT_texture
	if (a.base_internal_format == GL_INTENSITY)
	{
		float i = GLtof(d[0]);
		return glm::vec4(i, i, i, i);
	}
	else if (a.base_internal_format == GL_ALPHA)
		return glm::vec4(1, 1, 1, GLtof(d[0]));
#endif
	if (a.components >= 3)
		return glm::vec4{ GLtof(d[0]), GLtof(d[1]), GLtof(d[2]), a.components == 4 ? GLtof(d[3]) : 1 };
	return glm::vec4{ GLtof(d[0]), 0, 0, a.components == 2 ? GLtof(d[1]) : 1 };
}

glm::vec4 gl_tex_nearest_tap(const gl_texture& tex, const gl_texture_array& a, glm::vec2 c)
{
	if (tex.params.wrap_s == GL_REPEAT)
		c.x = glm::fract(c.x);
	if (tex.params.wrap_t == GL_REPEAT)
		c.y = glm::fract(c.y);
	c = glm::clamp(c, glm::vec2(0), glm::vec2(1));
	glm::ivec2 uv{ floor(c.x * a.width), floor(c.y * a.height) };
	return gl_tex_tap(a, uv);
}

glm::vec4 gl_texb_tap(const gl_texture &tex, const gl_texture_array &a, glm::ivec2 uv)
{
	if (!a.border && uv.x < 0 || uv.x >= a.width || uv.y < 0 || uv.y >= a.height)
		return tex.params.border_color;
	return gl_tex_tap(a, uv);
}

glm::vec4 gl_tex_linear_tap(const gl_texture &tex, const gl_texture_array& a, glm::vec2 c)
{
	if (tex.params.wrap_s == GL_CLAMP)
			c.x = glm::clamp(c.x, 0.f, 1.f);
	if (tex.params.wrap_t == GL_CLAMP)
			c.y = glm::clamp(c.y, 0.f, 1.f);
	glm::vec2 uv{ c.x * a.width, c.y * a.height };

	int i0 = (int)glm::floor(uv.x - 0.5f);
	int j0 = (int)glm::floor(uv.y - 0.5f);
	int i1 = i0 + 1;
	int j1 = j0 + 1;
	if (tex.params.wrap_s == GL_REPEAT)
	{
		i0 = glm::mod(i0, a.width);
		i1 = glm::mod(i1, a.width);
	}
	if (tex.params.wrap_t == GL_REPEAT)
	{
		j0 = glm::mod(j0, a.height);
		j1 = glm::mod(j1, a.height);
	}
	float al = glm::fract(uv.x - 0.5f);
	float be = glm::fract(uv.y - 0.5f);
	return gl_texb_tap(tex, a, { i0,j0 }) * (1 - al) * (1 - be)
		+ gl_texb_tap(tex, a, { i1,j0 }) * al * (1 - be)
		+ gl_texb_tap(tex, a, { i0,j1 }) * (1 - al) * be
		+ gl_texb_tap(tex, a, { i1,j1 }) * al * be;
}

#if 0
glm::vec4 gl_tex_1d_linear_tap(const gl_texture &tex, const gl_texture_array &a, glm::vec2 c)
{
	if (tex.params.wrap_s == GL_CLAMP)
		c.x = glm::clamp(c.x, 0.f, 1.f);
	glm::vec2 uv{ c.x * a.width, 0.5};

	int i0 = (int)glm::floor(uv.x - 0.5f);
	int i1 = i0 + 1;
	if (tex.params.wrap_s == GL_REPEAT)
	{
		i0 = glm::mod(i0, a.width);
		i1 = glm::mod(i1, a.width);
	}

	float al = glm::fract(uv.x - 0.5f);
	return gl_texb_tap(tex, a, { i0,0 }) * (1 - al)
		+ gl_texb_tap(tex, a, { i1,0 }) * al;
}
#endif

bool gl_state::need_tex_lod()
{
	if (!texture_2d_enabled && !texture_1d_enabled)
		return false;

	auto& tex = texture_2d_enabled ? texture_2d : texture_1d;

	if (!tex.is_complete)
		return false;

	return tex.params.mag_filter != tex.params.min_filter;
}

glm::vec4 gl_state::sample_tex2d(const gl_texture& tex, const glm::vec4& tex_coord, float lod)
{
	float c = 0;
	if (tex.params.mag_filter == GL_LINEAR && (tex.params.min_filter == GL_NEAREST_MIPMAP_NEAREST || tex.params.min_filter == GL_NEAREST_MIPMAP_LINEAR))
		c = 0.5;

	if (lod < c || tex.params.min_filter == tex.params.mag_filter)
	{
		const gl_texture_array& a = tex.arrays[0];
		if (!a.data)
			return glm::vec4(1, 1, 1, 1);

		if (tex.params.mag_filter == GL_NEAREST)
			return gl_tex_nearest_tap(tex, a, tex_coord);
		else
			return gl_tex_linear_tap(tex, a, tex_coord);
	}

	int ai = 0;
	if (tex.params.min_filter == GL_NEAREST_MIPMAP_NEAREST || tex.params.min_filter == GL_LINEAR_MIPMAP_NEAREST)
	{
		if (lod <= 0.5)
			ai = 0;
		else
			ai = glm::clamp((int)glm::floor(lod + 0.5f), 0, tex.max_lod - 1);
	}
	else if (tex.params.min_filter == GL_NEAREST_MIPMAP_LINEAR || tex.params.min_filter == GL_LINEAR_MIPMAP_LINEAR)
	{
		ai = glm::clamp((int)glm::floor(lod), 0, tex.max_lod - 1);
	}

	const gl_texture_array& a = tex.arrays[ai];
	if (!a.data)
		return glm::vec4(1, 1, 1, 1);

	glm::vec4 col;
	if (tex.params.min_filter == GL_LINEAR || tex.params.min_filter == GL_LINEAR_MIPMAP_NEAREST || tex.params.min_filter == GL_LINEAR_MIPMAP_LINEAR)
	{
		if (tex.params.min_filter == GL_LINEAR_MIPMAP_LINEAR && ai < tex.max_lod - 1 && tex.arrays[ai + 1].data)
		{
			float f = glm::fract(lod);
			col = (1 - f) * gl_tex_linear_tap(tex, a, tex_coord)
				+ f * gl_tex_linear_tap(tex, tex.arrays[ai + 1], tex_coord);
		}
		else
			col = gl_tex_linear_tap(tex, a, tex_coord);
	}
	else if (tex.params.min_filter == GL_NEAREST_MIPMAP_LINEAR && ai < tex.max_lod - 1 && tex.arrays[ai + 1].data)
	{
		float f = glm::fract(lod);
		col = (1 - f) * gl_tex_nearest_tap(tex, a, tex_coord)
			+ f * gl_tex_nearest_tap(tex, tex.arrays[ai + 1], tex_coord);
	}
	else
	{
		col = gl_tex_nearest_tap(tex, a, tex_coord);
	}

	return col;
}
