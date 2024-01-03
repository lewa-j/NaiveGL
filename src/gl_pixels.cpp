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
	gs->raster_pos.color = gs->get_vertex_color(p_eye, true);
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
