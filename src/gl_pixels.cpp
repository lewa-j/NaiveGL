#include "pch.h"
#include "gl_state.h"
#include "gl_pixels.h"
#include "gl_exports.h"

void APIENTRY glRasterPos4f(GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
	gl_state *gs = gl_current_state();
	if (!gs) return;
	WRITE_DISPLAY_LIST(RasterPos, { x,y,z,w });
	VALIDATE_NOT_BEGIN_MODE

	glm::vec4 p_object(x, y, z, w);
	glm::vec4 p_eye{ gs->get_modelview() * p_object };
	glm::vec4 p_clip{ gs->get_projection() * p_eye };
	gs->current.raster.valid = gs->clip_point(p_eye, p_clip);
	if (!gs->current.raster.valid)
		return;

	gs->current.raster.tex_coord = gs->get_vertex_texcoord(gs->current.tex_coord, gs->current.normal, p_object, p_eye);
	gs->current.raster.color = gs->get_vertex_color(p_eye, gs->current.color, gs->get_eye_normal(gs->current.normal), true);
	gs->current.raster.distance = glm::length(glm::vec3(p_eye));//can be approximated p_eye.z
	gs->current.raster.position = glm::vec4(gs->get_window_coords(glm::vec3(p_clip) / p_clip.w), p_clip.w);

	if (gs->render_mode == GL_SELECT)
	{
		gl_add_selection_depth(*gs, gs->current.raster.position.z);
		gs->select_hit = true;
		return;
	}
}

#define grp4f(x,y,z,w) glRasterPos4f((GLfloat)(x), (GLfloat)(y), (GLfloat)(z), (GLfloat)(w))

void APIENTRY glRasterPos2s(GLshort x, GLshort y)	{ grp4f(x, y, 0, 1); }
void APIENTRY glRasterPos2i(GLint x, GLint y)		{ grp4f(x, y, 0, 1); }
void APIENTRY glRasterPos2f(GLfloat x, GLfloat y)	{ glRasterPos4f(x, y, 0, 1); }
void APIENTRY glRasterPos2d(GLdouble x, GLdouble y) { grp4f(x, y, 0, 1); }
void APIENTRY glRasterPos2sv(const GLshort *v)		{ grp4f(v[0], v[1], 0, 1); }
void APIENTRY glRasterPos2iv(const GLint *v)		{ grp4f(v[0], v[1], 0, 1); }
void APIENTRY glRasterPos2fv(const GLfloat *v)		{ glRasterPos4f(v[0], v[1], 0, 1); }
void APIENTRY glRasterPos2dv(const GLdouble *v)		{ grp4f(v[0], v[1], 0, 1); }
void APIENTRY glRasterPos3s(GLshort x, GLshort y, GLshort z)	{ grp4f(x, y, z, 1); }
void APIENTRY glRasterPos3i(GLint x, GLint y, GLint z)			{ grp4f(x, y, z, 1); }
void APIENTRY glRasterPos3f(GLfloat x, GLfloat y, GLfloat z)	{ glRasterPos4f(x, y, z, 1); }
void APIENTRY glRasterPos3d(GLdouble x, GLdouble y, GLdouble z) { grp4f(x, y, z, 1); }
void APIENTRY glRasterPos3sv(const GLshort *v)	{ grp4f(v[0], v[1], v[2], 1); }
void APIENTRY glRasterPos3iv(const GLint *v)	{ grp4f(v[0], v[1], v[2], 1); }
void APIENTRY glRasterPos3fv(const GLfloat *v)	{ glRasterPos4f(v[0], v[1], v[2], 1); }
void APIENTRY glRasterPos3dv(const GLdouble *v) { grp4f(v[0], v[1], v[2], 1); }
void APIENTRY glRasterPos4s(GLshort x, GLshort y, GLshort z, GLshort w)		{ grp4f(x, y, z, w); }
void APIENTRY glRasterPos4i(GLint x, GLint y, GLint z, GLint w)				{ grp4f(x, y, z, w); }
void APIENTRY glRasterPos4d(GLdouble x, GLdouble y, GLdouble z, GLdouble w) { grp4f(x, y, z, w); }
void APIENTRY glRasterPos4sv(const GLshort *v)	{ grp4f(v[0], v[1], v[2], v[3]); }
void APIENTRY glRasterPos4iv(const GLint *v)	{ grp4f(v[0], v[1], v[2], v[3]); }
void APIENTRY glRasterPos4fv(const GLfloat *v)	{ glRasterPos4f(v[0], v[1], v[2], v[3]); }
void APIENTRY glRasterPos4dv(const GLdouble *v)	{ grp4f(v[0], v[1], v[2], v[3]); }
#undef grp4f

void APIENTRY glPixelStorei(GLenum pname, GLint param)
{
	gl_state* gs = gl_current_state();
	if (!gs) return;
	VALIDATE_NOT_BEGIN_MODE

	if ((pname < GL_UNPACK_SWAP_BYTES || pname > GL_UNPACK_ALIGNMENT) && (pname < GL_PACK_SWAP_BYTES || pname > GL_PACK_ALIGNMENT))
	{
		gl_set_error_a(GL_INVALID_ENUM, pname);
		return;
	}
	if (((pname >= GL_UNPACK_ROW_LENGTH && pname <= GL_UNPACK_SKIP_PIXELS) || (pname >= GL_PACK_ROW_LENGTH && pname <= GL_PACK_SKIP_PIXELS)) && param < 0)
	{
		gl_set_error(GL_INVALID_VALUE);
		return;
	}
	if ((pname == GL_UNPACK_ALIGNMENT || pname == GL_PACK_ALIGNMENT) && (param != 1 && param != 2 && param != 4 && param != 8))
	{
		gl_set_error(GL_INVALID_VALUE);
		return;
	}

	gl_state::pixelStore& ps = ((pname & 0x0FF0) == 0x0CF0) ? gs->pixel_unpack : gs->pixel_pack;
	if (pname >= GL_PACK_SWAP_BYTES)
		pname = (pname & 0x000F) | 0x0CF0;

	if (pname == GL_UNPACK_SWAP_BYTES)
		ps.swap_bytes = !!param;
	else if (pname == GL_UNPACK_LSB_FIRST)
		ps.lsb_first = !!param;
	else if (pname == GL_UNPACK_ROW_LENGTH)
		ps.row_length = param;
	else if (pname == GL_UNPACK_SKIP_ROWS)
		ps.skip_rows = param;
	else if (pname == GL_UNPACK_SKIP_PIXELS)
		ps.skip_pixels = param;
	else if (pname == GL_UNPACK_ALIGNMENT)
		ps.alignment = param;
}

void APIENTRY glPixelStoref(GLenum pname, GLfloat param) { glPixelStorei(pname, to_int(param)); }

void APIENTRY glPixelTransferf(GLenum pname, GLfloat param)
{
	gl_state* gs = gl_current_state();
	if (!gs) return;
	WRITE_DISPLAY_LIST(PixelTransfer, { param }, { (int)pname });
	VALIDATE_NOT_BEGIN_MODE

	if (pname < GL_MAP_COLOR || (pname > GL_RED_BIAS && pname < GL_GREEN_SCALE) || pname > GL_DEPTH_BIAS)
	{
		gl_set_error_a(GL_INVALID_ENUM, pname);
		return;
	}

	if (pname == GL_MAP_COLOR)
		gs->pixel.map_color = !!param;
	else if (pname == GL_MAP_STENCIL)
		gs->pixel.map_stencil = !!param;
	else if (pname == GL_INDEX_SHIFT)
		gs->pixel.index_shift = to_int(param);
	else if (pname == GL_INDEX_OFFSET)
		gs->pixel.index_offset = to_int(param);
	else if (pname == GL_RED_SCALE)
		gs->pixel.color_scale.r = param;
	else if (pname == GL_GREEN_SCALE)
		gs->pixel.color_scale.g = param;
	else if (pname == GL_BLUE_SCALE)
		gs->pixel.color_scale.b = param;
	else if (pname == GL_ALPHA_SCALE)
		gs->pixel.color_scale.a = param;
	else if (pname == GL_RED_BIAS)
		gs->pixel.color_bias.r = param;
	else if (pname == GL_GREEN_BIAS)
		gs->pixel.color_bias.g = param;
	else if (pname == GL_BLUE_BIAS)
		gs->pixel.color_bias.b = param;
	else if (pname == GL_ALPHA_BIAS)
		gs->pixel.color_bias.a = param;
	else if (pname == GL_DEPTH_SCALE)
		gs->pixel.depth_scale = param;
	else if (pname == GL_DEPTH_BIAS)
		gs->pixel.depth_bias = param;
}

void APIENTRY glPixelTransferi(GLenum pname, GLint param) { glPixelTransferf(pname, (GLfloat)param); }

#define VALIDATE_PIXEL_MAP \
VALIDATE_NOT_BEGIN_MODE;\
if (map < GL_PIXEL_MAP_I_TO_I || map > GL_PIXEL_MAP_A_TO_A)\
{\
	gl_set_error_a(GL_INVALID_ENUM, map);\
	return;\
}

#define VALIDATE_PIXEL_MAP_SIZE(map, mapsize) \
if (mapsize < 0 || mapsize > gl_max_pixel_map_table)\
{\
	gl_set_error(GL_INVALID_VALUE);\
	return;\
}\
if (map >= GL_PIXEL_MAP_I_TO_I && map <= GL_PIXEL_MAP_I_TO_A && !is_pow(mapsize)) \
{\
	gl_set_error(GL_INVALID_VALUE);\
	return;\
}

static int gl_pixelMapv_size(GLenum map, GLsizei mapsize)
{
	if (mapsize < 0 || mapsize > gl_max_pixel_map_table)
		return 0;
	if (map >= GL_PIXEL_MAP_I_TO_I && map <= GL_PIXEL_MAP_I_TO_A && !is_pow(mapsize))
		return 0;
	return mapsize;
}

void APIENTRY glPixelMapuiv(GLenum map, GLsizei mapsize, const GLuint* values)
{
	gl_state* gs = gl_current_state();
	if (!gs) return;
	int s = gl_pixelMapv_size(map, mapsize) * sizeof(GLuint);
	WRITE_DISPLAY_LIST_BULK(PixelMap, values, s, {}, { (int)map, mapsize, 1, s });
	VALIDATE_PIXEL_MAP;
	VALIDATE_PIXEL_MAP_SIZE(map, mapsize);

	if (map >= GL_PIXEL_MAP_I_TO_I && map <= GL_PIXEL_MAP_S_TO_S)
	{
		gl_state::pixelMapIndex& m = gs->pixel_map_index_table[map - GL_PIXEL_MAP_I_TO_I];
		m.size = mapsize;
		memcpy(m.data, values, mapsize * sizeof(GLuint));
	}
	else if (map >= GL_PIXEL_MAP_I_TO_R && map <= GL_PIXEL_MAP_A_TO_A)
	{
		gl_state::pixelMapColor& m = gs->pixel_map_color_table[map - GL_PIXEL_MAP_I_TO_R];
		m.size = mapsize;
		for (int i = 0; i < mapsize; i++)
			m.data[i] = GLtof(values[i]);
	}
}

void APIENTRY glPixelMapusv(GLenum map, GLsizei mapsize, const GLushort* values)
{
	gl_state* gs = gl_current_state();
	if (!gs) return;
	int s = gl_pixelMapv_size(map, mapsize) * sizeof(GLushort);
	WRITE_DISPLAY_LIST_BULK(PixelMap, values, s, {}, { (int)map, mapsize, 2, s });
	VALIDATE_PIXEL_MAP;
	VALIDATE_PIXEL_MAP_SIZE(map, mapsize);

	if (map >= GL_PIXEL_MAP_I_TO_I && map <= GL_PIXEL_MAP_S_TO_S)
	{
		gl_state::pixelMapIndex& m = gs->pixel_map_index_table[map - GL_PIXEL_MAP_I_TO_I];
		m.size = mapsize;
		for (int i = 0; i < mapsize; i++)
			m.data[i] = values[i];
	}
	else if (map >= GL_PIXEL_MAP_I_TO_R && map <= GL_PIXEL_MAP_A_TO_A)
	{
		gl_state::pixelMapColor& m = gs->pixel_map_color_table[map - GL_PIXEL_MAP_I_TO_R];
		m.size = mapsize;
		for (int i = 0; i < mapsize; i++)
			m.data[i] = GLtof(values[i]);
	}
}

void APIENTRY glPixelMapfv(GLenum map, GLsizei mapsize, const GLfloat* values)
{
	gl_state* gs = gl_current_state();
	if (!gs) return;
	int s = gl_pixelMapv_size(map, mapsize) * sizeof(GLfloat);
	WRITE_DISPLAY_LIST_BULK(PixelMap, values, s, {}, { (int)map, mapsize, 3, s });
	VALIDATE_PIXEL_MAP;
	VALIDATE_PIXEL_MAP_SIZE(map, mapsize);

	if (map >= GL_PIXEL_MAP_I_TO_I && map <= GL_PIXEL_MAP_S_TO_S)
	{
		gl_state::pixelMapIndex& m = gs->pixel_map_index_table[map - GL_PIXEL_MAP_I_TO_I];
		m.size = mapsize;
		for (int i = 0; i < mapsize; i++)
			m.data[i] = to_int(values[i]);
	}
	else if (map >= GL_PIXEL_MAP_I_TO_R && map <= GL_PIXEL_MAP_A_TO_A)
	{
		gl_state::pixelMapColor& m = gs->pixel_map_color_table[map - GL_PIXEL_MAP_I_TO_R];
		m.size = mapsize;
		memcpy(m.data, values, mapsize * sizeof(GLfloat));
	}
}

void APIENTRY glGetPixelMapuiv(GLenum map, GLuint *values)
{
	gl_state *gs = gl_current_state();
	if (!gs) return;
	VALIDATE_PIXEL_MAP;

	if (map >= GL_PIXEL_MAP_I_TO_I && map <= GL_PIXEL_MAP_S_TO_S)
	{
		gl_state::pixelMapIndex &m = gs->pixel_map_index_table[map - GL_PIXEL_MAP_I_TO_I];
		memcpy(values, m.data, m.size * sizeof(GLuint));
	}
	else if (map >= GL_PIXEL_MAP_I_TO_R && map <= GL_PIXEL_MAP_A_TO_A)
	{
		gl_state::pixelMapColor &m = gs->pixel_map_color_table[map - GL_PIXEL_MAP_I_TO_R];
		for (int i = 0; i < m.size; i++)
			values[i] = (GLuint)(0xFFFFFFFF * m.data[i]);
	}
}

void APIENTRY glGetPixelMapusv(GLenum map, GLushort *values)
{
	gl_state *gs = gl_current_state();
	if (!gs) return;
	VALIDATE_PIXEL_MAP;

	if (map >= GL_PIXEL_MAP_I_TO_I && map <= GL_PIXEL_MAP_S_TO_S)
	{
		gl_state::pixelMapIndex &m = gs->pixel_map_index_table[map - GL_PIXEL_MAP_I_TO_I];
		for (int i = 0; i < m.size; i++)
			values[i] = m.data[i];
	}
	else if (map >= GL_PIXEL_MAP_I_TO_R && map <= GL_PIXEL_MAP_A_TO_A)
	{
		gl_state::pixelMapColor &m = gs->pixel_map_color_table[map - GL_PIXEL_MAP_I_TO_R];
		for (int i = 0; i < m.size; i++)
			values[i] = (GLushort)(0xFFFF * m.data[i]);
	}
}

void APIENTRY glGetPixelMapfv(GLenum map, GLfloat *values)
{
	gl_state *gs = gl_current_state();
	if (!gs) return;
	VALIDATE_PIXEL_MAP;

	if (map >= GL_PIXEL_MAP_I_TO_I && map <= GL_PIXEL_MAP_S_TO_S)
	{
		gl_state::pixelMapIndex &m = gs->pixel_map_index_table[map - GL_PIXEL_MAP_I_TO_I];
		for (int i = 0; i < m.size; i++)
			values[i] = (float)m.data[i];
	}
	else if (map >= GL_PIXEL_MAP_I_TO_R && map <= GL_PIXEL_MAP_A_TO_A)
	{
		gl_state::pixelMapColor &m = gs->pixel_map_color_table[map - GL_PIXEL_MAP_I_TO_R];
		memcpy(values, m.data, m.size * sizeof(GLfloat));
	}
}

void APIENTRY glPixelZoom(GLfloat xfactor, GLfloat yfactor)
{
	gl_state* gs = gl_current_state();
	if (!gs) return;
	WRITE_DISPLAY_LIST(PixelZoom, { xfactor, yfactor });
	VALIDATE_NOT_BEGIN_MODE
	gs->pixel.zoom = glm::vec2(xfactor, yfactor);
}

void gl_pack_index_pixel(GLenum type, uint32_t index, void *dst)
{
	if (type == GL_BYTE)
		*(GLbyte *)dst = index & 0x7F;
	else if (type == GL_UNSIGNED_BYTE)
		*(GLubyte *)dst = index & 0xFF;
	else if (type == GL_SHORT)
		*(GLshort *)dst = index & 0x7FFF;
	else if (type == GL_UNSIGNED_SHORT)
		*(GLushort *)dst = index & 0xFFFF;
	else if (type == GL_INT)
		*(GLint *)dst = index & 0x7FFFFFFF;
	else if (type == GL_UNSIGNED_INT)
		*(GLuint *)dst = index & 0xFFFFFFFF;
	else if (type == GL_FLOAT)
		*(float *)dst = (float)index;
}

static void emit_stencil(gl_state& st, int x, int y, uint8_t index)
{
	gl_framebuffer& fb = *st.framebuffer;
	if (x < 0 || x >= fb.width || y < 0 || y >= fb.height)
		return;

	if (st.scissor.test)
	{
		const glm::ivec4& s = st.scissor.box;
		if (x < s.x || x >= s.x + s.z || y < s.y || y >= s.y + s.w)
			return;
	}

	int pi = (fb.width * y + x);
	fb.stencil[pi] = (index & st.stencil.writemask) | (fb.stencil[pi] & ~st.stencil.writemask);
}

int gl_pixels_size(GLsizei width, GLsizei height, GLenum format, GLenum type)
{
	if (width < 0 || height < 0)
		return 0;

	if (type == GL_BITMAP)
	{
		if (format != GL_COLOR_INDEX && format != GL_STENCIL_INDEX)
			return 0;
		return (int)(glm::ceil(width / 8.f)) * height;
	}

	if ((format < GL_COLOR_INDEX || format > GL_LUMINANCE_ALPHA) || (type < GL_BYTE || type > GL_FLOAT))
		return 0;

	return gl_pixel_type_size(type) * gl_pixel_format_size(format) * width * height;
}

void gl_unpack_pixels(gl_state *gs, GLsizei width, GLsizei height, GLenum format, GLenum type, const void *data, uint8_t *dst)
{
	if ((format < GL_COLOR_INDEX || format > GL_LUMINANCE_ALPHA) || (type != GL_BITMAP && (type < GL_BYTE || type > GL_FLOAT)))
		return;
	if (type == GL_BITMAP && format != GL_COLOR_INDEX && format != GL_STENCIL_INDEX)
		return;

	const uint8_t *src = (const uint8_t *)data;

	const gl_state::pixelStore &ps = gs->pixel_unpack;
	gl_PixelStoreSetup pstore;
	pstore.init(ps, width, height, format, type);

	src += pstore.skip_bytes;

	if (type == GL_BITMAP)
	{
		int dst_stride = (int)glm::ceil(width / 8.f);
		if (pstore.stride == dst_stride && !ps.lsb_first && !pstore.skip_bits)
		{
			memcpy(dst, src, dst_stride * height);
			return;
		}

		for (int j = 0; j < height; j++)
		{
			if (!ps.lsb_first && !pstore.skip_bits)
			{
				memcpy(dst, src, dst_stride);
				src += pstore.stride;
				dst += dst_stride;
				continue;
			}

			const uint8_t *row = src;
			int pixel = pstore.skip_bits;

			uint8_t *dst_row = dst;
			int dst_pixel = 0;
			uint8_t dst_byte = 0;

			for (int ix = 0; ix < width; ix++)
			{
				bool b = 0;
				if (ps.lsb_first)
					b = !!((*row) & (1 << pixel));
				else
					b = !!((*row) & (0x80 >> pixel));

				if (b)
					dst_byte |= 1;

				pixel++;
				dst_pixel++;
				if (dst_pixel >= 8)
				{
					dst_pixel = dst_pixel & 7;
					*dst_row = dst_byte;
					dst_byte = 0;
					dst_row++;
				}
				else
				{
					dst_byte <<= 1;
				}

				if (pixel >= 8)
				{
					pixel = pixel & 7;
					row++;
				}
			}
			src += pstore.stride;
			dst += dst_stride;
		}
	}
	else
	{
		int dst_stride = pstore.group_size * width;
		if (pstore.stride == dst_stride && !(ps.swap_bytes && pstore.element_size > 1))
		{
			memcpy(dst, src, dst_stride * height);
			return;
		}

		for (int j = 0; j < height; j++)
		{
			memcpy(dst, src, dst_stride);

			if (ps.swap_bytes && pstore.element_size > 1)
			{
				//swap in-place
				uint8_t *dst_row = dst;
				for (int i = 0; i < width; i++)
				{
					gl_swap_bytes(pstore.element_size, pstore.components, dst_row);
					dst_row += pstore.group_size;
				}
			}
			src += pstore.stride;
			dst += dst_stride;
		}
	}
}

void APIENTRY glDrawPixels(GLsizei width, GLsizei height, GLenum format, GLenum type, const void* data)
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
		dl.calls.push_back({ gl_display_list_call::tDrawPixels, {}, { width, height, (int)format, (int)type, pix_size} });
		if (!gs->display_list_execute)
			return;
	}
	VALIDATE_NOT_BEGIN_MODE;

	if (width < 0 || height < 0)
	{
		gl_set_error(GL_INVALID_VALUE);
		return;
	}
	if (format < GL_COLOR_INDEX || format > GL_LUMINANCE_ALPHA)
	{
		gl_set_error_a(GL_INVALID_ENUM, format);
		return;
	}
	if (type != GL_BITMAP && (type < GL_BYTE || type > GL_FLOAT))
	{
		gl_set_error_a(GL_INVALID_ENUM, type);
		return;
	}
	if (type == GL_BITMAP && format != GL_COLOR_INDEX && format != GL_STENCIL_INDEX)
	{
		gl_set_error_a(GL_INVALID_ENUM, format);
		return;
	}

#if 0
	//color index mode only
	if (format != GL_COLOR_INDEX && format != GL_STENCIL_INDEX && format != GL_DEPTH_COMPONENT)
	{
		gl_set_error(GL_INVALID_OPERATION);
		return;
	}
#endif

	if (format == GL_STENCIL_INDEX)
	{
		if (!gs->framebuffer->stencil)
		{
			gl_set_error(GL_INVALID_OPERATION);
			return;
		}
		if (!(gs->stencil.writemask & 0xFF))
			return;
	}

	if (!gs->current.raster.valid)
		return;

	if (gs->render_mode == GL_SELECT)
		return;
	else if (gs->render_mode == GL_FEEDBACK)
	{
		gl_feedback_write(*gs, GL_DRAW_PIXEL_TOKEN);
		gl_feedback_write_vertex(*gs, gs->current.raster.position, gs->current.raster.color, gs->current.raster.tex_coord);
		return;
	}

	const uint8_t *pixels = (const uint8_t *)data;

	const gl_state::pixelStore& ps = gs->pixel_unpack;
	gl_PixelStoreSetup pstore;
	pstore.init(ps, width, height, format, type);

	pixels += pstore.skip_bytes;

	gl_frag_data fdata;
	fdata.color = gs->current.raster.color;
	fdata.tex_coord = gs->current.raster.tex_coord;
#if NGL_VERISON >= 110
	fdata.tex_coord /= fdata.tex_coord.q;
#endif
	fdata.z = gs->current.raster.position.z;
	fdata.fog_z = gs->current.raster.distance;
	fdata.lod = 0;

	if (type == GL_BITMAP)
	{
		glm::vec4 bitmap_colors[2];
		if (format != GL_STENCIL_INDEX)
		{
			bitmap_colors[0] = index_to_rgba(0, gs->pixel_map_color_table);
			bitmap_colors[1] = index_to_rgba(1, gs->pixel_map_color_table);
		}

		for (int j = 0; j < height; j++)
		{
			const uint8_t* group = pixels;

			int pixel = pstore.skip_bits;
			for (int ix = 0; ix < width; ix++)
			{
				bool b = 0;
				if (ps.lsb_first)
					b = !!((*group) & (1 << pixel));
				else
					b = !!((*group) & (0x80 >> pixel));

				int x = (int)(gs->current.raster.position.x + gs->pixel.zoom.x * ix);
				int y = (int)(gs->current.raster.position.y + gs->pixel.zoom.y * j);
				if (format == GL_STENCIL_INDEX)
				{
					uint8_t index = b ? 1 : 0;
					if (gs->pixel.map_stencil)
					{
						int ti = index & (gs->pixel_map_index_table[1].size - 1);
						index = gs->pixel_map_index_table[1].data[ti];
					}

					for (int sy = 0; sy < gs->pixel.zoom.y; sy++)
						for (int sx = 0; sx < gs->pixel.zoom.x; sx++)
							emit_stencil(*gs, x + sx, y + sy, index);
				}
				else
				{
					fdata.color = bitmap_colors[b];

					for (int sy = 0; sy < gs->pixel.zoom.y; sy++)
						for (int sx = 0; sx < gs->pixel.zoom.x; sx++)
							gl_emit_fragment(*gs, x + sx, y + sy, fdata);
				}

				pixel++;

				if (pixel >= 8)
				{
					pixel = pixel & 7;
					group++;
				}
			}
			pixels += pstore.stride;
		}
	}
	else
	{
		for (int j = 0; j < height; j++)
		{
			const uint8_t* row = pixels;
			for (int i = 0; i < width; i++)
			{
				uint8_t group[16];
				memcpy(group, row, pstore.group_size);
				if (ps.swap_bytes && pstore.element_size > 1)
					gl_swap_bytes(pstore.element_size, pstore.components, group);

				glm::vec4 pixel{};
				uint32_t index = 0;
				if (format == GL_COLOR_INDEX || format == GL_STENCIL_INDEX)
				{
					index = gl_unpack_index_pixel(type, group);
					index = gl_index_arithmetic(gs, index);

					if (format == GL_COLOR_INDEX)
					{
						//only in rgba mode
						pixel = index_to_rgba(index, gs->pixel_map_color_table);
					}
					else if (format == GL_STENCIL_INDEX && gs->pixel.map_stencil)
					{
						int ti = index & (gs->pixel_map_index_table[1].size - 1);
						index = gs->pixel_map_index_table[1].data[ti];
					}
				}
				else
				{
					pixel = gl_unpack_color_pixel(format, type, group);
					if (format == GL_DEPTH_COMPONENT)
						pixel.r = pixel.r * gs->pixel.depth_scale + gs->pixel.depth_bias;
					else
					{
						pixel = pixel * gs->pixel.color_scale + gs->pixel.color_bias;
						if (gs->pixel.map_color)
							pixel = remap_color(pixel, gs->pixel_map_color_table + 4);
					}
				}

				int x = int(gs->current.raster.position.x + gs->pixel.zoom.x * i);
				int y = int(gs->current.raster.position.y + gs->pixel.zoom.y * j);
				if (format == GL_STENCIL_INDEX)
				{
					for (int sy = 0; sy < gs->pixel.zoom.y; sy++)
						for (int sx = 0; sx < gs->pixel.zoom.x; sx++)
							emit_stencil(*gs, x + sx, y + sy, index);
				}
				else
				{
					if (format == GL_DEPTH_COMPONENT)
					{
						fdata.z = pixel.r;
					}
					else
					{
						fdata.color = pixel;
					}

					for (int sy = 0; sy < gs->pixel.zoom.y; sy++)
						for (int sx = 0; sx < gs->pixel.zoom.x; sx++)
							gl_emit_fragment(*gs, x + sx, y + sy, fdata);
				}

				row += pstore.group_size;
			}
			pixels += pstore.stride;
		}
	}
}

void APIENTRY glBitmap(GLsizei width, GLsizei height, GLfloat xorig, GLfloat yorig, GLfloat xmove, GLfloat ymove, const GLubyte* data)
{
	gl_state* gs = gl_current_state();
	if (!gs) return;
	if (gs->display_list_begun)
	{
		auto &dl = gs->display_list_indices[0];
		size_t old_size = dl.data.size();
		int pix_size = gl_pixels_size(width, height, GL_COLOR_INDEX, GL_BITMAP);
		if (pix_size)
		{
			dl.data.resize(old_size + pix_size);
			gl_unpack_pixels(gs, width, height, GL_COLOR_INDEX, GL_BITMAP, data, dl.data.data() + old_size);
		}
		dl.calls.push_back({ gl_display_list_call::tBitmap, {xorig, yorig, xmove, ymove}, { width, height, pix_size} });
		if (!gs->display_list_execute)
			return;
	}
	VALIDATE_NOT_BEGIN_MODE

	if (!gs->current.raster.valid)
		return;

	if (!data)
	{
		gs->current.raster.position += glm::vec4(xmove, ymove, 0, 0);
		return;
	}

	if (gs->render_mode == GL_SELECT)
		return;
	else if (gs->render_mode == GL_FEEDBACK)
	{
		gl_feedback_write(*gs, GL_BITMAP_TOKEN);
		gl_feedback_write_vertex(*gs, gs->current.raster.position, gs->current.raster.color, gs->current.raster.tex_coord);
		return;
	}

	const gl_state::pixelStore &ps = gs->pixel_unpack;
	gl_PixelStoreSetup pstore;
	pstore.init(ps, width, height, GL_COLOR_INDEX, GL_BITMAP);

	data += pstore.skip_bytes;

	int x = (int)floorf(gs->current.raster.position.x - xorig);
	int y = (int)floorf(gs->current.raster.position.y - yorig);

	gl_frag_data fdata;
	fdata.color = gs->current.raster.color;
	fdata.tex_coord = gs->current.raster.tex_coord;
#if NGL_VERISON >= 110
	fdata.tex_coord /= fdata.tex_coord.q;
#endif
	fdata.z = gs->current.raster.position.z;
	fdata.fog_z = gs->current.raster.distance;
	fdata.lod = 0;

	for (int j = 0; j < height; j++)
	{
		const uint8_t* group = data;

		int pixel = pstore.skip_bits;
		for (int ix = 0; ix < width; ix++)
		{
			bool b = 0;
			if (ps.lsb_first)
				b = !!((*group) & (1 << pixel));
			else
				b = !!((*group) & (0x80 >> pixel));

			if (b)
			{
				gl_emit_fragment(*gs, x + ix, y + j, fdata);
			}

			pixel++;

			if (pixel >= 8)
			{
				pixel = pixel & 7;
				group++;
			}
		}
		data += pstore.stride;
	}

	gs->current.raster.position += glm::vec4(xmove, ymove, 0, 0);
}

void APIENTRY glReadPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, void* data)
{
	gl_state* gs = gl_current_state();
	if (!gs) return;
	VALIDATE_NOT_BEGIN_MODE;

	if (width < 0 || height < 0)
	{
		gl_set_error(GL_INVALID_VALUE);
		return;
	}
	if (format < GL_COLOR_INDEX || format > GL_LUMINANCE_ALPHA)
	{
		gl_set_error_a(GL_INVALID_ENUM, format);
		return;
	}
	if (type != GL_BITMAP && (type < GL_BYTE || type > GL_FLOAT))
	{
		gl_set_error_a(GL_INVALID_ENUM, type);
		return;
	}

	//rgba mode context
	if (format == GL_COLOR_INDEX)
	{
		gl_set_error(GL_INVALID_OPERATION);
		return;
	}

	if (format == GL_DEPTH_COMPONENT && !gs->framebuffer->depth)
	{
		gl_set_error(GL_INVALID_OPERATION);
		return;
	}
	if (format == GL_STENCIL_INDEX && !gs->framebuffer->stencil)
	{
		gl_set_error(GL_INVALID_OPERATION);
		return;
	}

	if (type == GL_BITMAP && format != GL_COLOR_INDEX && format != GL_STENCIL_INDEX)
	{
		//undefined
		return;
	}

	if (type == GL_BITMAP)
	{
		//TODO unimplemented
		printf("glReadPixels GL_BITMAP unimplemented\n");
		return;
	}

	uint8_t *pixels = (uint8_t *)data;

	const gl_framebuffer& fb = *gs->framebuffer;
	const gl_state::pixelStore& ps = gs->pixel_pack;
	gl_PixelStoreSetup pstore;
	pstore.init(ps, width, height, format, type);
	pstore.framebufferClamp(fb, x, y, width, height);

	pixels += pstore.skip_bytes;

	for (int iy = 0; iy < height; iy++)
	{
		int fbi = (y + iy) * fb.width + x;
		uint8_t* row = pixels;
		for (int ix = 0; ix < width; ix++)
		{
			glm::vec4 col;
			uint8_t index = 0;
			if (format == GL_STENCIL_INDEX)
			{
				index = fb.stencil[fbi];
				index = gl_index_arithmetic(gs, index);
				if (gs->pixel.map_stencil)
				{
					int ti = index & (gs->pixel_map_index_table[1].size - 1);
					index = gs->pixel_map_index_table[1].data[ti];
				}
			}
			else if (format == GL_DEPTH_COMPONENT)
			{
				//TODO depth buffer assumed to be 16 bit
				float depth = fb.depth[fbi] / (float)0xffff;
				depth = depth * gs->pixel.depth_scale + gs->pixel.depth_bias;
				col.r = depth;
			}
			else
			{
				const int ci = fbi * 4;
				glm::ivec4 icol(fb.color[ci + 2], fb.color[ci + 1], fb.color[ci + 0], fb.color[ci + 3]);//bgra
				col = glm::vec4(icol) / (float)0xff;
				col = col * gs->pixel.color_scale + gs->pixel.color_bias;
				if (gs->pixel.map_color)
					col = remap_color(col, gs->pixel_map_color_table + 4);
				col = gl_pixel_format_conversion(format, col);
			}

			if (format == GL_STENCIL_INDEX)
				gl_pack_index_pixel(type, index, row);
			else
				gl_pack_color_pixel(pstore.components, type, col, row);

			if (ps.swap_bytes && pstore.element_size > 1)
				gl_swap_bytes(pstore.element_size, pstore.components, row);

			fbi++;
			row += pstore.group_size;
		}
		pixels += pstore.stride;
	}
}

void APIENTRY glCopyPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum type)
{
	gl_state* gs = gl_current_state();
	if (!gs) return;
	WRITE_DISPLAY_LIST(CopyPixels, {}, { x, y, width, height, (int)type });
	VALIDATE_NOT_BEGIN_MODE;

	if (width < 0 || height < 0)
	{
		gl_set_error(GL_INVALID_VALUE);
		return;
	}
	if (type < GL_COLOR || type > GL_STENCIL)
	{
		gl_set_error_a(GL_INVALID_ENUM, type);
		return;
	}

	if (!gs->current.raster.valid)
		return;

	if (gs->render_mode == GL_SELECT)
		return;
	else if (gs->render_mode == GL_FEEDBACK)
	{
		gl_feedback_write(*gs, GL_COPY_PIXEL_TOKEN);
		gl_feedback_write_vertex(*gs, gs->current.raster.position, gs->current.raster.color, gs->current.raster.tex_coord);
		return;
	}

	// prevent glDrawPixels from writing into display list
	int save_dl = gs->display_list_begun;
	gs->display_list_begun = 0;

	// may be optimized

	// ignore pixel store
	gl_state::pixelStore save_pack{};
	gl_state::pixelStore save_unpack{};
	std::swap(save_pack, gs->pixel_pack);
	std::swap(save_unpack, gs->pixel_unpack);

	// map_color, scale, bias and index arithmetic will be applied twice, first on Read and then on Draw.
	// So reset pixel_t after Read and restore after Draw

	gl_state::pixel_t save_pixel{};

	if (type == GL_COLOR)
	{
		std::vector<uint8_t> pixels(width * height * 4);
		glReadPixels(x, y, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
		std::swap(save_pixel, gs->pixel);
		glDrawPixels(width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
	}
	else if (type == GL_DEPTH)
	{
		std::vector<uint16_t> pixels(width * height);
		glReadPixels(x, y, width, height, GL_DEPTH_COMPONENT, GL_UNSIGNED_SHORT, pixels.data());
		std::swap(save_pixel, gs->pixel);
		glDrawPixels(width, height, GL_DEPTH_COMPONENT, GL_UNSIGNED_SHORT, pixels.data());
	}
	else if (type == GL_STENCIL)
	{
		std::vector<uint8_t> pixels(width * height);
		glReadPixels(x, y, width, height, GL_STENCIL_INDEX, GL_UNSIGNED_BYTE, pixels.data());
		std::swap(save_pixel, gs->pixel);
		glDrawPixels(width, height, GL_STENCIL_INDEX, GL_UNSIGNED_BYTE, pixels.data());
		
	}
	gs->pixel = save_pixel;
	gs->pixel_pack = save_pack;
	gs->pixel_unpack = save_unpack;

	gs->display_list_begun = save_dl;
}
