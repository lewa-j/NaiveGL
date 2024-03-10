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

inline bool is_pow(int a) { return !(a & (a - 1)); }

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
