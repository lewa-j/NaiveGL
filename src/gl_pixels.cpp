#include "pch.h"
#include "gl_state.h"
#include "gl_exports.h"

void APIENTRY glRasterPos4f(GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
	gl_state *gs = gl_current_state();
	if (!gs) return;
	glm::vec4 p_object(x, y, z, w);
	glm::vec4 p_eye{ gs->get_modelview() * p_object };
	glm::vec4 p_clip{ gs->get_projection() * p_eye };
	gs->raster_pos.valid = gs->clip_point(p_eye, p_clip);
	if (!gs->raster_pos.valid)
		return;

	gs->raster_pos.tex_coord = gs->get_vertex_texcoord(p_object, p_eye);
	gs->raster_pos.color = gs->get_vertex_color_current(p_eye, true);
	gs->raster_pos.distance = glm::length(glm::vec3(p_eye));//can be approximated p_eye.z
	gs->raster_pos.coords = glm::vec4(gs->get_window_coords(glm::vec3(p_clip) / p_clip.w), p_clip.w);
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
void APIENTRY glPixelStoref(GLenum pname, GLfloat param) { glPixelStorei(pname, (GLint)param); }

void APIENTRY glPixelTransferf(GLenum pname, GLfloat param)
{
	gl_state* gs = gl_current_state();
	if (!gs) return;
	VALIDATE_NOT_BEGIN_MODE

	if (pname < GL_MAP_COLOR || (pname > GL_RED_BIAS && pname < GL_GREEN_SCALE) || pname > GL_DEPTH_BIAS)
	{
		gl_set_error_a(GL_INVALID_ENUM, pname);
		return;
	}

	if (pname == GL_MAP_COLOR)
		gs->map_color = !!param;
	else if (pname == GL_MAP_STENCIL)
		gs->map_stencil = !!param;
	else if (pname == GL_INDEX_SHIFT)
		gs->index_shift = (int)param;
	else if (pname == GL_INDEX_OFFSET)
		gs->index_offset = (int)param;
	else if (pname == GL_RED_SCALE)
		gs->color_scale.r = param;
	else if (pname == GL_GREEN_SCALE)
		gs->color_scale.g = param;
	else if (pname == GL_BLUE_SCALE)
		gs->color_scale.b = param;
	else if (pname == GL_ALPHA_SCALE)
		gs->color_scale.a = param;
	else if (pname == GL_RED_BIAS)
		gs->color_bias.r = param;
	else if (pname == GL_GREEN_BIAS)
		gs->color_bias.g = param;
	else if (pname == GL_BLUE_BIAS)
		gs->color_bias.b = param;
	else if (pname == GL_ALPHA_BIAS)
		gs->color_bias.a = param;
	else if (pname == GL_DEPTH_SCALE)
		gs->depth_scale = param;
	else if (pname == GL_DEPTH_BIAS)
		gs->depth_bias = param;
}
void APIENTRY glPixelTransferi(GLenum pname, GLint param) { glPixelTransferf(pname, (GLfloat)param); }

#define VALIDATE_PIXEL_MAP \
VALIDATE_NOT_BEGIN_MODE;\
if (map < GL_PIXEL_MAP_I_TO_I || map > GL_PIXEL_MAP_A_TO_A)\
{\
	gl_set_error_a(GL_INVALID_ENUM, map);\
	return;\
}\
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

void APIENTRY glPixelMapuiv(GLenum map, GLsizei mapsize, const GLuint* values)
{
	gl_state* gs = gl_current_state();
	if (!gs) return;
	VALIDATE_PIXEL_MAP;

	if (map >= GL_PIXEL_MAP_I_TO_I && map <= GL_PIXEL_MAP_S_TO_S)
	{
		gl_state::pixelMapIndex& m = gs->pixel_map_index_table[map - GL_PIXEL_MAP_I_TO_I];
		m.size = mapsize;
		memcpy(m.data, values, mapsize * sizeof(*values));
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
	VALIDATE_PIXEL_MAP;

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
	VALIDATE_PIXEL_MAP;

	if (map >= GL_PIXEL_MAP_I_TO_I && map <= GL_PIXEL_MAP_S_TO_S)
	{
		gl_state::pixelMapIndex& m = gs->pixel_map_index_table[map - GL_PIXEL_MAP_I_TO_I];
		m.size = mapsize;
		for (int i = 0; i < mapsize; i++)
			m.data[i] = (GLint)values[i];
	}
	else if (map >= GL_PIXEL_MAP_I_TO_R && map <= GL_PIXEL_MAP_A_TO_A)
	{
		gl_state::pixelMapColor& m = gs->pixel_map_color_table[map - GL_PIXEL_MAP_I_TO_R];
		m.size = mapsize;
		memcpy(m.data, values, mapsize * sizeof(*values));
	}
}

void APIENTRY glPixelZoom(GLfloat xfactor, GLfloat yfactor)
{
	gl_state* gs = gl_current_state();
	if (!gs) return;
	VALIDATE_NOT_BEGIN_MODE
	gs->pixel_zoom = glm::vec2(xfactor, yfactor);
}

template<typename T>
static glm::vec4 pixel_to_float(const T* data, GLenum format)
{
	if (format == GL_GREEN)
		return glm::vec4(0, GLtof(data[0]), 0, 1);
	if (format == GL_BLUE)
		return glm::vec4(0, 0, GLtof(data[0]), 1);
	if (format == GL_ALPHA)
		return glm::vec4(0, 0, 0, GLtof(data[0]));
	if (format == GL_RGB)
		return glm::vec4(GLtof(data[0]), GLtof(data[1]), GLtof(data[2]), 1);
	if (format == GL_RGBA)
		return glm::vec4(GLtof(data[0]), GLtof(data[1]), GLtof(data[2]), GLtof(data[3]));
	if (format == GL_LUMINANCE)
	{
		float l = GLtof(data[0]);
		return glm::vec4(l, l, l, 1);
	}
	if (format == GL_LUMINANCE_ALPHA)
	{
		float l = GLtof(data[0]);
		return glm::vec4(l, l, l, GLtof(data[1]));
	}

	return glm::vec4(GLtof(data[0]), 0, 0, 1);
}

static glm::vec4 remap_color(const glm::vec4 &c, gl_state::pixelMapColor *tables)
{
	glm::vec4 r;
	for (int i = 0; i < 4; i++)
	{
		int ci = (int)roundf(glm::clamp(c[i], 0.f, 1.f) * tables[i].size - 1);
		r[i] = tables[i].data[ci];
	}
	return r;
}

static glm::vec4 index_to_rgba(int index, gl_state::pixelMapColor* tables)
{
	glm::vec4 r{};
	for (int k = 0; k < 4; k++)
	{
		int ti = index & (tables[k].size - 1);
		r[k] = tables[k].data[ti];
	}
	return r;
}

static void emit_stencil(gl_state& st, int x, int y, uint8_t index)
{
	gl_framebuffer& fb = *st.framebuffer;
	if (x < 0 || x >= fb.width || y < 0 || y >= fb.height)
		return;

	if (st.scissor_test)
	{
		const glm::ivec4& s = st.scissor_rect;
		if (x < s.x || x >= s.x + s.z || y < s.y || y >= s.y + s.w)
			return;
	}

	int pi = (fb.width * y + x);
	fb.stencil[pi] = (index & st.stencil_writemask) | (fb.stencil[pi] & ~st.stencil_writemask);
}

void APIENTRY glDrawPixels(GLsizei width, GLsizei height, GLenum format, GLenum type, const void* data)
{
	gl_state* gs = gl_current_state();
	if (!gs) return;
	VALIDATE_NOT_BEGIN_MODE

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
		if (!(gs->stencil_writemask & 0xFF))
			return;
	}

	if (!gs->raster_pos.valid)
		return;

	const gl_state::pixelStore& ps = gs->pixel_unpack;
	int row_length = (ps.row_length > 0) ? ps.row_length : width;

	const uint8_t* pixels = (const uint8_t*)data;

	if (type == GL_BITMAP)
	{
		int stride = (int)(ps.alignment * glm::ceil(row_length / float(8 * ps.alignment)));
		int skip_bits = ps.skip_pixels & 7;

		pixels += ps.skip_pixels/8 + ps.skip_rows * stride;

		glm::vec4 bitmap_colors[2];
		if (format != GL_STENCIL_INDEX)
		{
			bitmap_colors[0] = index_to_rgba(0, gs->pixel_map_color_table);
			bitmap_colors[1] = index_to_rgba(1, gs->pixel_map_color_table);
		}

		gl_frag_data fdata;
		fdata.color = gs->raster_pos.color;
		fdata.tex_coord = gs->raster_pos.tex_coord;
		fdata.z = gs->raster_pos.coords.z;
		fdata.fog_z = gs->raster_pos.distance;
		fdata.lod = 0;

		for (int j = 0; j < height; j++)
		{
			const uint8_t* group = pixels;

			int pixel = skip_bits;
			for (int ix = 0; ix < width; ix++)
			{
				bool b = 0;
				if (ps.lsb_first)
					b = !!((*group) & (1 << pixel));
				else
					b = !!((*group) & (0x80 >> pixel));

				int x = (int)(gs->raster_pos.coords.x + gs->pixel_zoom.x * ix);
				int y = (int)(gs->raster_pos.coords.y + gs->pixel_zoom.y * j);
				if (format == GL_STENCIL_INDEX)
				{
					uint8_t index = b ? 1 : 0;
					if (gs->map_stencil)
					{
						int ti = index & (gs->pixel_map_index_table[1].size - 1);
						index = gs->pixel_map_index_table[1].data[ti];
					}

					for (int sy = 0; sy < gs->pixel_zoom.y; sy++)
						for (int sx = 0; sx < gs->pixel_zoom.x; sx++)
							emit_stencil(*gs, x + sx, y + sy, index);
				}
				else
				{
					fdata.color = bitmap_colors[b];

					for (int sy = 0; sy < gs->pixel_zoom.y; sy++)
						for (int sx = 0; sx < gs->pixel_zoom.x; sx++)
							gl_emit_fragment(*gs, x + sx, y + sy, fdata);
				}

				pixel++;

				if (pixel >= 8)
				{
					pixel = pixel & 7;
					group++;
				}
			}
			pixels += stride;
		}
	}
	else
	{
		int pixel_size = 1;
		if (format == GL_LUMINANCE_ALPHA)
			pixel_size = 2;
		else if (format == GL_RGB)
			pixel_size = 3;
		else if (format == GL_RGBA)
			pixel_size = 4;

		int element_size = 1;
		if (type == GL_UNSIGNED_SHORT || type == GL_SHORT)
			element_size = 2;
		else if (type == GL_UNSIGNED_INT || type == GL_INT || type == GL_FLOAT)
			element_size = 4;

		int elements_stride = int((element_size >= ps.alignment) ?
			(pixel_size * row_length) :
			(ps.alignment / element_size * glm::ceil((element_size * pixel_size * row_length) / (float)ps.alignment)));

		pixels += (ps.skip_pixels * pixel_size + ps.skip_rows * elements_stride) * element_size;

		gl_frag_data fdata;
		fdata.color = gs->raster_pos.color;
		fdata.tex_coord = gs->raster_pos.tex_coord;
		fdata.z = gs->raster_pos.coords.z;
		fdata.fog_z = gs->raster_pos.distance;
		fdata.lod = 0;

		for (int j = 0; j < height; j++)
		{
			const uint8_t* row = pixels;
			for (int i = 0; i < width; i++)
			{
				uint8_t group[16];
				memcpy(group, row, element_size * pixel_size);
				if (ps.swap_bytes && element_size > 1)
				{
					for (int c = 0; c < pixel_size; c++)
					{
						std::swap(group[c * element_size], group[(c + 1) * element_size - 1]);
						if (element_size == 4)
							std::swap(group[c * element_size + 1], group[c * element_size + 2]);
					}
				}
				glm::vec4 pixel{};
				uint32_t index = 0;
				if (format == GL_COLOR_INDEX || format == GL_STENCIL_INDEX)
				{
					if (type == GL_BYTE)
						index = *(const GLbyte*)group;
					else if (type == GL_UNSIGNED_BYTE)
						index = *(const GLubyte*)group;
					else if (type == GL_SHORT)
						index = *(const GLshort*)group;
					else if (type == GL_UNSIGNED_SHORT)
						index = *(const GLushort*)group;
					else if (type == GL_INT)
						index = *(const GLint*)group;
					else if (type == GL_UNSIGNED_INT)
						index = *(const GLuint*)group;
					else if (type == GL_FLOAT)
						index = (uint32_t)*(const float*)group;

					if (gs->index_shift > 0)
						index <<= gs->index_shift;
					else if(gs->index_shift < 0)
						index >>= -gs->index_shift;

					index += gs->index_offset;

					if (format == GL_COLOR_INDEX)
					{
						//only in rgba mode
						pixel = index_to_rgba(index, gs->pixel_map_color_table);
					}
					else if (format == GL_STENCIL_INDEX && gs->map_stencil)
					{
						int ti = index & (gs->pixel_map_index_table[1].size - 1);
						index = gs->pixel_map_index_table[1].data[ti];
					}
				}
				else
				{
					if (type == GL_BYTE)
						pixel = pixel_to_float((const GLbyte*)group, format);
					else if (type == GL_UNSIGNED_BYTE)
						pixel = pixel_to_float((const GLubyte*)group, format);
					else if (type == GL_SHORT)
						pixel = pixel_to_float((const GLshort*)group, format);
					else if (type == GL_UNSIGNED_SHORT)
						pixel = pixel_to_float((const GLushort*)group, format);
					else if (type == GL_INT)
						pixel = pixel_to_float((const GLint*)group, format);
					else if (type == GL_UNSIGNED_INT)
						pixel = pixel_to_float((const GLuint*)group, format);
					else if (type == GL_FLOAT)
						pixel = pixel_to_float((const float*)group, format);

					if (format == GL_DEPTH_COMPONENT)
						pixel.r = pixel.r * gs->depth_scale + gs->depth_bias;
					else
					{
						pixel = pixel * gs->color_scale + gs->color_bias;
						if (gs->map_color)
							pixel = remap_color(pixel, gs->pixel_map_color_table + 4);
					}
				}

				int x = int(gs->raster_pos.coords.x + gs->pixel_zoom.x * i);
				int y = int(gs->raster_pos.coords.y + gs->pixel_zoom.y * j);
				if (format == GL_STENCIL_INDEX)
				{
					for (int sy = 0; sy < gs->pixel_zoom.y; sy++)
						for (int sx = 0; sx < gs->pixel_zoom.x; sx++)
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

					for (int sy = 0; sy < gs->pixel_zoom.y; sy++)
						for (int sx = 0; sx < gs->pixel_zoom.x; sx++)
							gl_emit_fragment(*gs, x + sx, y + sy, fdata);
				}

				row += pixel_size * element_size;
			}
			pixels += elements_stride * element_size;
		}
	}
}

void APIENTRY glBitmap(GLsizei width, GLsizei height, GLfloat xorig, GLfloat yorig, GLfloat xmove, GLfloat ymove, const GLubyte* data)
{
	gl_state* gs = gl_current_state();
	if (!gs) return;
	VALIDATE_NOT_BEGIN_MODE

	if (!gs->raster_pos.valid)
		return;

	if (!data)
	{
		gs->raster_pos.coords += glm::vec4(xmove, ymove, 0, 0);
		return;
	}

	const gl_state::pixelStore& ps = gs->pixel_unpack;
	int row_length = (ps.row_length > 0) ? ps.row_length : width;

	int stride = (int)(ps.alignment * glm::ceil(row_length / float(8 * ps.alignment)));
	int skip_bits = ps.skip_pixels & 7;

	data += ps.skip_pixels / 8 + ps.skip_rows * stride;

	int x = (int)floorf(gs->raster_pos.coords.x - xorig);
	int y = (int)floorf(gs->raster_pos.coords.y - yorig);

	gl_frag_data fdata;
	fdata.color = gs->raster_pos.color;
	fdata.tex_coord = gs->raster_pos.tex_coord;
	fdata.z = gs->raster_pos.coords.z;
	fdata.fog_z = gs->raster_pos.distance;
	fdata.lod = 0;

	for (int j = 0; j < height; j++)
	{
		const uint8_t* group = data;

		int pixel = skip_bits;
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
		data += stride;
	}

	gs->raster_pos.coords += glm::vec4(xmove, ymove, 0, 0);
}


void APIENTRY glReadPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, void* data)
{
	gl_state* gs = gl_current_state();
	if (!gs) return;
	VALIDATE_NOT_BEGIN_MODE;

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
		return;
	}

	const gl_framebuffer& fb = *gs->framebuffer;

	const gl_state::pixelStore& ps = gs->pixel_pack;
	int row_length = (ps.row_length > 0) ? ps.row_length : width;

	uint8_t* pixels = (uint8_t*)data;

	int pixel_size = 1;
	if (format == GL_LUMINANCE_ALPHA)
		pixel_size = 2;
	else if (format == GL_RGB)
		pixel_size = 3;
	else if (format == GL_RGBA)
		pixel_size = 4;

	int element_size = 1;
	if (type == GL_UNSIGNED_SHORT || type == GL_SHORT)
		element_size = 2;
	else if (type == GL_UNSIGNED_INT || type == GL_INT || type == GL_FLOAT)
		element_size = 4;

	int elements_stride = int((element_size >= ps.alignment) ?
		(pixel_size * row_length) :
		(ps.alignment / element_size * glm::ceil((element_size * pixel_size * row_length) / (float)ps.alignment)));

	pixels += (ps.skip_pixels * pixel_size + ps.skip_rows * elements_stride) * element_size;

	if (y < 0)
	{
		pixels += (-y * elements_stride) * element_size;
		height += y;
		y = 0;
	}
	if (x < 0)
	{
		pixels += (-x * pixel_size) * element_size;
		width += x;
		x = 0;
	}

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

				if (gs->index_shift > 0)
					index <<= gs->index_shift;
				else if (gs->index_shift < 0)
					index >>= -gs->index_shift;

				index += gs->index_offset;

				if (gs->map_stencil)
				{
					int ti = index & (gs->pixel_map_index_table[1].size - 1);
					index = gs->pixel_map_index_table[1].data[ti];
				}
			}
			else if (format == GL_DEPTH_COMPONENT)
			{
				float depth = fb.depth[fbi] / (float)0xffff;
				depth = depth * gs->depth_scale + gs->depth_bias;
				col.r = depth;
			}
			else
			{
				const int ci = fbi * 4;
				
				glm::ivec4 icol(fb.color[ci + 2], fb.color[ci + 1], fb.color[ci + 0], fb.color[ci + 3]);//bgra
				col = glm::vec4(icol) / (float)0xff;
				col = col * gs->color_scale + gs->color_bias;
				if (gs->map_color)
					col = remap_color(col, gs->pixel_map_color_table + 4);

				if (format == GL_LUMINANCE || format == GL_LUMINANCE_ALPHA)
				{
					col.r = col.r + col.g + col.b;
					col.g = col.a;
				}
				else if (format == GL_GREEN)
					col.r = col.g;
				else if (format == GL_BLUE)
					col.r = col.b;
				else if (format == GL_ALPHA)
					col.r = col.a;
			}

			if (format == GL_STENCIL_INDEX)
			{
				if (type == GL_BYTE)
					*(GLbyte*)row = index & 0x7F;
				else if (type == GL_UNSIGNED_BYTE)
					*(GLubyte*)row = index & 0xFF;
				else if (type == GL_SHORT)
					*(GLshort*)row = index & 0x7FFF;
				else if (type == GL_UNSIGNED_SHORT)
					*(GLushort*)row = index & 0xFFFF;
				else if (type == GL_INT)
					*(GLint*)row = index & 0x7FFFFFFF;
				else if (type == GL_UNSIGNED_INT)
					*(GLuint*)row = index & 0xFFFFFFFF;
				else if (type == GL_FLOAT)
					*(float*)row = index;
			}
			else
			{
				col = glm::clamp(col, 0.f, 1.f);
				for (int ci = 0; ci < pixel_size; ci++)
				{
					if (type == GL_BYTE)
						((GLbyte*)row)[ci] = (GLbyte)((0xFF * col[ci] - 1) / 2);
					else if (type == GL_UNSIGNED_BYTE)
						((GLubyte*)row)[ci] = (GLubyte)(col[ci] * 0xFF);
					else if (type == GL_SHORT)
						((GLshort*)row)[ci] = GLshort((0xFFFF * col[ci] - 1) / 2);
					else if (type == GL_UNSIGNED_SHORT)
						((GLushort*)row)[ci] = (GLushort)(col[ci] * 0xFFFF);
					else if (type == GL_INT)
						((GLint*)row)[ci] = (GLint)((0xFFFFFFFF * col[ci] - 1) / 2);
					else if (type == GL_UNSIGNED_INT)
						((GLuint*)row)[ci] = (GLuint)(col[ci] * 0xFFFFFFFF);
					else if (type == GL_FLOAT)
						((float*)row)[ci] = (float)col[ci];
				}
			}

			if (ps.swap_bytes && element_size > 1)
			{
				for (int c = 0; c < pixel_size; c++)
				{
					std::swap(row[c * element_size], row[(c + 1) * element_size - 1]);
					if (element_size == 4)
						std::swap(row[c * element_size + 1], row[c * element_size + 2]);
				}
			}

			fbi++;
			row += pixel_size * element_size;
		}
		pixels += elements_stride * element_size;
	}
}
