#include "pch.h"
#include "gl_state.h"
#include "gl_exports.h"

void APIENTRY glScissor(GLint x, GLint y, GLsizei width, GLsizei height)
{
	gl_state *gs = gl_current_state();
	if (!gs) return;
	VALIDATE_NOT_BEGIN_MODE;
	if (width < 0 || height < 0)
	{
		gl_set_error(GL_INVALID_VALUE);
		return;
	}
	gs->scissor_rect = glm::ivec4(x, y, width, height);
}

#define VALIDATE_DR_BUFFER(buf) \
if (buf != GL_NONE && (buf < GL_FRONT_LEFT && buf >= (GL_AUX0 + gl_max_aux_buffers)))\
{\
	gl_set_error_a(GL_INVALID_ENUM, buf);\
	return;\
}

void APIENTRY glDrawBuffer(GLenum buf)
{
	gl_state *gs = gl_current_state();
	if (!gs) return;
	VALIDATE_NOT_BEGIN_MODE;
	VALIDATE_DR_BUFFER(buf);

	if (!gs->framebuffer->doublebuffer && (buf == GL_BACK_LEFT || buf == GL_BACK_RIGHT || buf == GL_BACK))
	{
		gl_set_error_a(GL_INVALID_OPERATION, buf);
		return;
	}

	gs->draw_buffer = buf;
}

void APIENTRY glReadBuffer(GLenum src)
{
	gl_state *gs = gl_current_state();
	if (!gs) return;
	VALIDATE_NOT_BEGIN_MODE;
	VALIDATE_DR_BUFFER(src);

	if (src == GL_NONE || src == GL_FRONT_AND_BACK)
	{
		gl_set_error_a(GL_INVALID_ENUM, src);
		return;
	}

	if (!gs->framebuffer->doublebuffer && (src == GL_BACK_LEFT || src == GL_BACK_RIGHT || src == GL_BACK))
	{
		gl_set_error_a(GL_INVALID_OPERATION, src);
		return;
	}

	//TODO here or on use?
	if (src == GL_FRONT || src == GL_LEFT)
		src = GL_FRONT_LEFT;
	else if (src == GL_BACK)
		src = GL_BACK_LEFT;
	else if (src == GL_RIGHT)
		src = GL_FRONT_RIGHT;

	gs->read_buffer = src;
}

void APIENTRY glColorMask(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha)
{
	gl_state *gs = gl_current_state();
	if (!gs) return;
	VALIDATE_NOT_BEGIN_MODE;
	gs->color_mask = glm::bvec4{ red, green, blue, alpha };
}

void APIENTRY glIndexMask(GLuint mask)
{
	gl_state *gs = gl_current_state();
	if (!gs) return;
	VALIDATE_NOT_BEGIN_MODE;
	//color index mode
}

void APIENTRY glDepthMask(GLboolean flag)
{
	gl_state *gs = gl_current_state();
	if (!gs) return;
	VALIDATE_NOT_BEGIN_MODE;
	gs->depth_mask = flag;
}

void APIENTRY glStencilMask(GLuint mask)
{
	gl_state *gs = gl_current_state();
	if (!gs) return;
	VALIDATE_NOT_BEGIN_MODE;
	gs->stencil_writemask = mask;
}

void APIENTRY glClear(GLbitfield mask) {}

void APIENTRY glClearColor(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)
{
	gl_state *gs = gl_current_state();
	if (!gs) return;
	VALIDATE_NOT_BEGIN_MODE;
	gs->clear_color = glm::clamp(glm::vec4(red, green, blue, alpha), 0.f, 1.f);
}

void APIENTRY glClearIndex(GLfloat c)
{
	gl_state *gs = gl_current_state();
	if (!gs) return;
	VALIDATE_NOT_BEGIN_MODE;
	//color index mode
}

void APIENTRY glClearDepth(GLdouble depth)
{
	gl_state *gs = gl_current_state();
	if (!gs) return;
	VALIDATE_NOT_BEGIN_MODE;
	gs->clear_depth = glm::clamp((float)depth, 0.f, 1.f);
}

void APIENTRY glClearStencil(GLint s)
{
	gl_state *gs = gl_current_state();
	if (!gs) return;
	VALIDATE_NOT_BEGIN_MODE;
	gs->clear_stencil = s;
}

void APIENTRY glClearAccum(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)
{
	gl_state *gs = gl_current_state();
	if (!gs) return;
	VALIDATE_NOT_BEGIN_MODE;
	gs->clear_accum = glm::clamp(glm::vec4(red, green, blue, alpha), -1.f, 1.f);
}

void APIENTRY glAccum(GLenum op, GLfloat value) {}
