
#include "pch.h"
#include "gl_state.h"
#include "gl_pixels.h"
#include "gl_exports.h"
#include <glm/gtx/integer.hpp>

#define VALIDATE_TEX_IMAGE \
if (level < 0 || level > gl_max_tex_level) \
{ \
	gl_set_error_a(GL_INVALID_VALUE, level); \
	return; \
} \
if (components < 0 || components > 4) \
{ \
	gl_set_error_a(GL_INVALID_VALUE, components); \
	return; \
} \
if (border != 0 && border != 1) \
{ \
	gl_set_error_a(GL_INVALID_VALUE, border); \
	return; \
} \
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

static bool gl_is_texture_complete(gl_texture &tex)
{
	int w = tex.arrays[0].width;
	int h = tex.arrays[0].height;
	if (w < 1 || h < 1)
		return false;

	bool mipmap = (tex.min_filter == GL_NEAREST_MIPMAP_NEAREST || tex.min_filter == GL_NEAREST_MIPMAP_LINEAR
		|| tex.min_filter == GL_LINEAR_MIPMAP_NEAREST || tex.min_filter == GL_LINEAR_MIPMAP_LINEAR);

	if (!mipmap)
		return true;

	int levels = 1 + (int)log2(glm::max(w, h));

	for (int i = 1; i < levels; i++)
	{
		w = glm::max(1, (w >> 1));
		h = glm::max(1, (h >> 1));
		if (w != tex.arrays[i].width || h != tex.arrays[i].height || tex.arrays[i].components != tex.arrays[0].components || tex.arrays[i].border != tex.arrays[0].border)
			return false;
	}
	return true;
}

void APIENTRY glTexImage2D(GLenum target, GLint level, GLint components, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void* data)
{
	gl_state* gs = gl_current_state();
	if (!gs) return;
	if (gs->display_list_begun)
	{
		auto &dl = gs->display_list_indices[0];
		size_t old_size = dl.data.size();
		int pix_size = gl_pixels_size(width, height, format, type);
		if (pix_size)
		{
			dl.data.resize(old_size + pix_size);
			gl_unpack_pixels(gs, width, height, format, type, data, dl.data.data() + old_size);
		}
		dl.calls.push_back({ gl_display_list_call::tTexImage2D, {(float)target}, {level, components, width, height, border, (int)format, (int)type, pix_size} });
		if (!gs->display_list_execute)
			return;
	}
	VALIDATE_NOT_BEGIN_MODE

	if (target != GL_TEXTURE_2D)
	{
		gl_set_error_a(GL_INVALID_ENUM, target);
		return;
	}
	VALIDATE_TEX_IMAGE

	int borderless_width = width - border * 2;
	int borderless_height = height - border * 2;
	if (borderless_width < 0 || borderless_width > gl_max_texture_size || !is_pow(borderless_width) ||
		borderless_height < 0 || borderless_height > gl_max_texture_size || !is_pow(borderless_height))
	{
		gl_set_error(GL_INVALID_VALUE);
		return;
	}

	gl_texture& tex = gs->texture_2d;
	gl_texture_array& ta = tex.arrays[level];

	if (width == 0 || height == 0)
	{
		if (ta.data)
			delete[] ta.data;
		ta.data = nullptr;
		ta.width = 0;
		ta.height = 0;
		return;
	}

	size_t size = width * height * components;

	if (ta.width * ta.height * ta.components != size)
	{
		if (ta.data)
			delete[] ta.data;

		ta.data = new uint8_t[size];
	}
	ta.width = width;
	ta.height = height;
	ta.components = components;
	ta.border = border;

	if (level == 0)
	{
		tex.max_lod = 1 + (int)log2(glm::max(borderless_width, borderless_height));
	}

	if (!data)
	{
		tex.is_complete = gl_is_texture_complete(tex);
		return;
	}

	const uint8_t *src = (const uint8_t *)data;

	const gl_state::pixelStore& ps = gs->pixel_unpack;
	gl_PixelStoreSetup pstore;
	pstore.init(ps, width, height, format, type);

	src += pstore.skip_bytes;

	if (type == GL_UNSIGNED_BYTE && border == 0 &&
		!gs->map_color && gs->color_scale == glm::vec4{ 1,1,1,1 } && gs->color_bias == glm::vec4{ 0,0,0,0 })
	{
		if (pstore.stride == width * components)
		{
			memcpy(ta.data, data, size);
			tex.is_complete = gl_is_texture_complete(tex);
			return;
		}

		if (format == GL_LUMINANCE && components == 1 || format == GL_LUMINANCE_ALPHA && components == 2 || format == GL_RGB && components == 3 || format == GL_RGBA && components == 4)
		{
			uint8_t *dst = ta.data;
			for (int j = 0; j < height; j++)
			{
				const uint8_t *row = src;
				memcpy(dst, data, size);
				dst += width * components;
				src += pstore.stride;
			}

			tex.is_complete = gl_is_texture_complete(tex);
			return;
		}
		else
		{
			if (pstore.stride == width * pstore.components && format == GL_RGBA && components == 3)
			{
				for (size_t i = 0; i < size; i += 3)
				{
					ta.data[i] = src[0];
					ta.data[i + 1] = src[1];
					ta.data[i + 2] = src[2];
					src += 4;
				}
				tex.is_complete = gl_is_texture_complete(tex);
				return;
			}
		}
	}

	//TODO
	printf("glTexImage2D(%d,%d,%d,%d,%d,%X,%X) al=%d unhandled combination\n", level, components, width, height, border, format, type, ps.alignment);
}

void APIENTRY glTexImage1D(GLenum target, GLint level, GLint components, GLsizei width, GLint border, GLenum format, GLenum type, const void* data)
{
	gl_state* gs = gl_current_state();
	if (!gs) return;
	if (gs->display_list_begun)
	{
		auto &dl = gs->display_list_indices[0];
		size_t old_size = dl.data.size();
		int pix_size = gl_pixels_size(width, 1, format, type);
		if (pix_size)
		{
			dl.data.resize(old_size + pix_size);
			gl_unpack_pixels(gs, width, 1, format, type, data, dl.data.data() + old_size);
		}
		dl.calls.push_back({ gl_display_list_call::tTexImage1D, {}, {(int)target, level, components, width, border, (int)format, (int)type, pix_size} });
		if (!gs->display_list_execute)
			return;
	}
	VALIDATE_NOT_BEGIN_MODE

	if (target != GL_TEXTURE_1D)
	{
		gl_set_error_a(GL_INVALID_ENUM, target);
		return;
	}
	VALIDATE_TEX_IMAGE

	int borderless_width = width - border * 2;
	if (borderless_width < 0 || borderless_width > gl_max_texture_size || !is_pow(borderless_width))
	{
		gl_set_error_a(GL_INVALID_VALUE, width);
		return;
	}

	//TODO
}

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

	const gl_texture &tex = (target == GL_TEXTURE_1D) ? gs->texture_1d : gs->texture_2d;
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

			col = col * gs->color_scale + gs->color_bias;

			if (gs->map_color)
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

#define VALIDATE_TEX_PARAMETER \
VALIDATE_NOT_BEGIN_MODE; \
if (target != GL_TEXTURE_1D && target != GL_TEXTURE_2D) \
{ \
	gl_set_error_a(GL_INVALID_ENUM, target); \
	return; \
} \
if ((pname < GL_TEXTURE_MAG_FILTER || pname > GL_TEXTURE_WRAP_T) && pname != GL_TEXTURE_BORDER_COLOR) \
{ \
	gl_set_error_a(GL_INVALID_ENUM, pname); \
	return; \
}

#define VALIDATE_TEX_PARAMETER_PARAM(p) \
if ((pname == GL_TEXTURE_WRAP_S || pname == GL_TEXTURE_WRAP_T) && (p != GL_CLAMP && p != GL_REPEAT)) \
{ \
	gl_set_error_a(GL_INVALID_ENUM, p); \
	return; \
} \
if ((pname == GL_TEXTURE_MAG_FILTER && p != GL_NEAREST && p != GL_LINEAR) \
	|| (pname == GL_TEXTURE_MIN_FILTER && p != GL_NEAREST && p != GL_LINEAR && (p < GL_NEAREST_MIPMAP_NEAREST || p > GL_LINEAR_MIPMAP_LINEAR))) \
{ \
	gl_set_error_a(GL_INVALID_ENUM, p); \
	return; \
}

void APIENTRY glTexParameteri(GLenum target, GLenum pname, GLint param)
{
	gl_state *gs = gl_current_state();
	if (!gs) return;
	WRITE_DISPLAY_LIST(TexParameter, {}, { (int)target, (int)pname, param });
	VALIDATE_TEX_PARAMETER;
	VALIDATE_TEX_PARAMETER_PARAM(param);
	if (pname == GL_TEXTURE_BORDER_COLOR)
	{
		gl_set_error_a(GL_INVALID_ENUM, pname);
		return;
	}

	gl_texture& tex = target == GL_TEXTURE_2D ? gs->texture_2d : gs->texture_1d;

	if (pname == GL_TEXTURE_MAG_FILTER)
		tex.mag_filter = param;
	else if (pname == GL_TEXTURE_MIN_FILTER)
		tex.min_filter = param;
	else if (pname == GL_TEXTURE_WRAP_S)
		tex.wrap_s = param;
	else if (pname == GL_TEXTURE_WRAP_T)
		tex.wrap_t = param;

	tex.is_complete = gl_is_texture_complete(tex);
}
void APIENTRY glTexParameterf(GLenum target, GLenum pname, GLfloat param)
{
	glTexParameteri(target, pname, (int)param);
}

static int gl_texParameterv_size(GLenum pname)
{
	if (pname >= GL_TEXTURE_MAG_FILTER && pname <= GL_TEXTURE_WRAP_T)
		return 1;
	if (pname == GL_TEXTURE_BORDER_COLOR)
		return 4;
	return 0;
}

template<typename T>
void gl_texParameterv(gl_state *gs, GLenum target, GLenum pname, const T* params)
{
	VALIDATE_TEX_PARAMETER;
	VALIDATE_TEX_PARAMETER_PARAM(int(params[0]));

	gl_texture& tex = target == GL_TEXTURE_2D ? gs->texture_2d : gs->texture_1d;

	if (pname == GL_TEXTURE_MAG_FILTER)
		tex.mag_filter = (int)params[0];
	else if (pname == GL_TEXTURE_MIN_FILTER)
		tex.min_filter = (int)params[0];
	else if (pname == GL_TEXTURE_WRAP_S)
		tex.wrap_s = (int)params[0];
	else if (pname == GL_TEXTURE_WRAP_T)
		tex.wrap_t = (int)params[0];
	else if (pname == GL_TEXTURE_BORDER_COLOR)
		tex.border_color = glm::vec4(GLtof(params[0]), GLtof(params[1]), GLtof(params[2]), GLtof(params[3]));

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
	VALIDATE_TEX_PARAMETER;

	gl_texture &tex = target == GL_TEXTURE_2D ? gs->texture_2d : gs->texture_1d;

	if (pname == GL_TEXTURE_MAG_FILTER)
		*params = (T)tex.mag_filter;
	else if (pname == GL_TEXTURE_MIN_FILTER)
		*params = (T)tex.min_filter;
	else if (pname == GL_TEXTURE_WRAP_S)
		*params = (T)tex.wrap_s;
	else if (pname == GL_TEXTURE_WRAP_T)
		*params = (T)tex.wrap_t;
	else if (pname == GL_TEXTURE_BORDER_COLOR)
		copy_color(params, &tex.border_color.x);
}

void APIENTRY glGetTexParameteriv(GLenum target, GLenum pname, GLint *params)
{
	gl_getTexParameterv(target, pname, params);
}

void APIENTRY glGetTexParameterfv(GLenum target, GLenum pname, GLfloat *params)
{
	gl_getTexParameterv(target, pname, params);
}

template<typename T>
void gl_getTexLevelParameterv(GLenum target, GLint level, GLenum pname, T *params)
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

	gl_texture_array &ta = (target == GL_TEXTURE_2D ? gs->texture_2d : gs->texture_1d).arrays[level];

	if (pname == GL_TEXTURE_WIDTH)
		*params = (T)ta.width;
	else if (pname == GL_TEXTURE_HEIGHT)
		*params = (T)ta.height;
	else if (pname == GL_TEXTURE_COMPONENTS)
		*params = (T)ta.components;
	else if (pname != GL_TEXTURE_BORDER)
		*params = (T)ta.border;
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

#define VALIDATE_TEX_ENV_PARAM(p) \
if (pname == GL_TEXTURE_ENV_MODE && (p != GL_MODULATE && p != GL_DECAL && p != GL_BLEND)) \
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
		gs->texture_env_function = param;
	}
}
void APIENTRY glTexEnvf(GLenum target, GLenum pname, GLfloat param)
{
	glTexEnvi(target, pname, (GLint)param);
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
		gs->texture_env_function = params[0];
	else if (pname == GL_TEXTURE_ENV_COLOR)
		gs->texture_env_color = glm::vec4(GLtof(params[0]), GLtof(params[1]), GLtof(params[2]), GLtof(params[3]));
}

void APIENTRY glTexEnvfv(GLenum target, GLenum pname, const GLfloat* params)
{
	gl_state *gs = gl_current_state();
	if (!gs) return;
	WRITE_DISPLAY_LIST_FV(TexEnvfv, params, gl_texEnvv_size(pname), {}, { (int)target, (int)pname });
	VALIDATE_TEX_ENV;
	VALIDATE_TEX_ENV_PARAM((GLenum)params[0]);

	if (pname == GL_TEXTURE_ENV_MODE)
		gs->texture_env_function = (int)params[0];
	else if (pname == GL_TEXTURE_ENV_COLOR)
		gs->texture_env_color = glm::vec4(params[0], params[1], params[2], params[3]);
}

template<typename T>
void gl_getTexEnvv(GLenum target, GLenum pname, T *params)
{
	gl_state *gs = gl_current_state();
	if (!gs) return;
	VALIDATE_TEX_ENV;

	if (pname == GL_TEXTURE_ENV_MODE)
		*params = (T)gs->texture_env_function;
	else if (pname == GL_TEXTURE_ENV_COLOR)
		copy_color(params, &gs->texture_env_color.x);
}

void APIENTRY glGetTexEnviv(GLenum target, GLenum pname, GLint *params)
{
	gl_getTexEnvv(target, pname, params);
}

void APIENTRY glGetTexEnvfv(GLenum target, GLenum pname, GLfloat *params)
{
	gl_getTexEnvv(target, pname, params);
}

glm::vec4 gl_tex_tap(const gl_texture_array& a, glm::ivec2 uv)
{
	glm::ivec2 c{ glm::clamp(uv, glm::ivec2(0), glm::ivec2(a.width - 1, a.height - 1)) };
	uint8_t* d = a.data + (c.y * a.width + c.x) * a.components;
	
	if (a.components >= 3)
		return glm::vec4{ GLtof(d[0]), GLtof(d[1]), GLtof(d[2]), a.components == 4 ? GLtof(d[3]) : 1 };
	return glm::vec4{ GLtof(d[0]), 0, 0, a.components == 2 ? GLtof(d[1]) : 1 };
}

glm::vec4 gl_tex_nearest_tap(const gl_texture& tex, const gl_texture_array& a, glm::vec2 c)
{
	if (tex.wrap_s == GL_REPEAT)
		c.x = glm::fract(c.x);
	if (tex.wrap_t == GL_REPEAT)
		c.y = glm::fract(c.y);
	c = glm::clamp(c, glm::vec2(0), glm::vec2(1));
	glm::ivec2 uv{ floor(c.x * a.width), floor(c.y * a.height) };
	return gl_tex_tap(a, uv);
}

glm::vec4 gl_tex_linear_tap(const gl_texture &tex, const gl_texture_array& a, glm::vec2 c)
{
	//TODO border color

	glm::vec2 uv{ c.x * a.width, c.y * a.height };

	int i0 = (int)glm::floor(uv.x - 0.5f);
	int j0 = (int)glm::floor(uv.y - 0.5f);
	int i1 = i0 + 1;
	int j1 = j0 + 1;
	if (tex.wrap_s == GL_REPEAT)
	{
		i0 = glm::mod(i0, a.width);
		i1 = glm::mod(i1, a.width);
	}
	if (tex.wrap_t == GL_REPEAT)
	{
		j0 = glm::mod(j0, a.height);
		j1 = glm::mod(j1, a.height);
	}
	float al = glm::fract(uv.x - 0.5f);
	float be = glm::fract(uv.y - 0.5f);
	return gl_tex_tap(a, { i0,j0 }) * (1 - al) * (1 - be)
		+ gl_tex_tap(a, { i1,j0 }) * al * (1 - be)
		+ gl_tex_tap(a, { i0,j1 }) * (1 - al) * be
		+ gl_tex_tap(a, { i1,j1 }) * al * be;
}

bool gl_state::need_tex_lod()
{
	if (!texture_2d_enabled && !texture_1d_enabled)
		return false;

	auto& tex = texture_2d_enabled ? texture_2d : texture_1d;

	if (!tex.is_complete)
		return false;

	return tex.mag_filter != tex.min_filter;
}

glm::vec4 gl_state::sample_tex2d(const gl_texture& tex, const glm::vec4& tex_coord, float lod)
{
	float c = 0;
	if (tex.mag_filter == GL_LINEAR && (tex.min_filter == GL_NEAREST_MIPMAP_NEAREST || tex.min_filter == GL_LINEAR_MIPMAP_NEAREST))
		c = 0.5;

	if (lod < c || tex.min_filter == tex.mag_filter)
	{
		const gl_texture_array& a = tex.arrays[0];
		if (!a.data)
			return glm::vec4(1, 1, 1, 1);

		if (tex.mag_filter == GL_NEAREST)
			return gl_tex_nearest_tap(tex, a, tex_coord);
		else
			return gl_tex_linear_tap(tex, a, tex_coord);
	}

	int ai = 0;
	if (tex.min_filter == GL_NEAREST_MIPMAP_NEAREST || tex.min_filter == GL_LINEAR_MIPMAP_NEAREST)
	{
		if (lod <= 0.5)
			ai = 0;
		else
			ai = glm::clamp((int)glm::floor(lod + 0.5f), 0, tex.max_lod - 1);
	}
	else if (tex.min_filter == GL_NEAREST_MIPMAP_LINEAR || tex.min_filter == GL_LINEAR_MIPMAP_LINEAR)
	{
		ai = glm::clamp((int)glm::floor(lod), 0, tex.max_lod - 1);
	}

	const gl_texture_array& a = tex.arrays[ai];
	if (!a.data)
		return glm::vec4(1, 1, 1, 1);

	glm::vec4 col;
	if (tex.min_filter == GL_LINEAR || tex.min_filter == GL_LINEAR_MIPMAP_NEAREST || tex.min_filter == GL_LINEAR_MIPMAP_LINEAR)
	{
		if (tex.min_filter == GL_LINEAR_MIPMAP_LINEAR && ai < tex.max_lod - 1 && tex.arrays[ai + 1].data)
		{
			float f = glm::fract(lod);
			col = (1 - f) * gl_tex_linear_tap(tex, a, tex_coord)
				+ f * gl_tex_linear_tap(tex, tex.arrays[ai + 1], tex_coord);
		}
		else
			col = gl_tex_linear_tap(tex, a, tex_coord);
	}
	else if (tex.min_filter == GL_NEAREST_MIPMAP_LINEAR && ai < tex.max_lod - 1 && tex.arrays[ai + 1].data)
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
