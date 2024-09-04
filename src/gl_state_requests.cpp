#include "pch.h"
#include "gl_state.h"
#include "gl_exports.h"

template<typename T>
static bool gl_get(gl_state *gs, GLenum pname, T *data)
{
	int r = gl_isEnabled(gs, pname);
	if (r != -1)
	{
		*data = (T)r;// 0 or 1
		return true;
	}

	return false;
}

void APIENTRY glGetBooleanv(GLenum pname, GLboolean *data)
{
	gl_state *gs = gl_current_state();
	if (!gs) return;
	VALIDATE_NOT_BEGIN_MODE;
	if (!gl_get(gs, pname, data))
		gl_set_error_a(GL_INVALID_ENUM, pname);
}

void APIENTRY glGetIntegerv(GLenum pname, GLint *data)
{
	gl_state *gs = gl_current_state();
	if (!gs) return;
	VALIDATE_NOT_BEGIN_MODE;
	if (!gl_get(gs, pname, data))
		gl_set_error_a(GL_INVALID_ENUM, pname);
}

void APIENTRY glGetFloatv(GLenum pname, GLfloat *data)
{
	gl_state *gs = gl_current_state();
	if (!gs) return;
	VALIDATE_NOT_BEGIN_MODE;
	if (!gl_get(gs, pname, data))
		gl_set_error_a(GL_INVALID_ENUM, pname);
}

void APIENTRY glGetDoublev(GLenum pname, GLdouble *data)
{
	gl_state *gs = gl_current_state();
	if (!gs) return;
	VALIDATE_NOT_BEGIN_MODE;
	if (!gl_get(gs, pname, data))
		gl_set_error_a(GL_INVALID_ENUM, pname);
}

void APIENTRY glGetPixelMapuiv(GLenum map, GLuint *values) {}
void APIENTRY glGetPixelMapusv(GLenum map, GLushort *values) {}
void APIENTRY glGetPixelMapfv(GLenum map, GLfloat *values) {}
void APIENTRY glGetMapiv(GLenum target, GLenum query, GLint *v) {}
void APIENTRY glGetMapfv(GLenum target, GLenum query, GLfloat *v) {}
void APIENTRY glGetMapdv(GLenum target, GLenum query, GLdouble *v) {}
void APIENTRY glGetTexImage(GLenum target, GLint level, GLenum format, GLenum type, void *pixels) {}
void APIENTRY glGetPolygonStipple(GLubyte *mask) {}

void APIENTRY glPushAttrib(GLbitfield mask) {}
void APIENTRY glPopAttrib(void) {}
