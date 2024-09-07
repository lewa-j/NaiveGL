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

void APIENTRY glPushAttrib(GLbitfield mask)
{
	gl_state *gs = gl_current_state();
	if (!gs) return;
	VALIDATE_NOT_BEGIN_MODE;

	if (gs->attrib_sp >= gl_max_attrib_stack_depth)
	{
		gl_set_error(GL_STACK_OVERFLOW);
		return;
	}

	gs->attrib_stack[gs->attrib_sp] = mask;
	gs->attrib_sp++;
}

void APIENTRY glPopAttrib(void)
{
	gl_state *gs = gl_current_state();
	if (!gs) return;
	VALIDATE_NOT_BEGIN_MODE;

	if (gs->attrib_sp == 0)
	{
		gl_set_error(GL_STACK_UNDERFLOW);
		return;
	}

	gs->attrib_sp--;
}
