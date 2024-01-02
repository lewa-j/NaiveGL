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

void APIENTRY glBlendFunc(GLenum sfactor, GLenum dfactor)
{
	gl_state* gs = gl_current_state();
	if (!gs) return;
	VALIDATE_NOT_BEGIN_MODE;
	if (sfactor != 0 && sfactor != 1 && (sfactor < GL_SRC_ALPHA || sfactor > GL_SRC_ALPHA_SATURATE))
	{
		gl_set_error_a(GL_INVALID_ENUM, sfactor);
		return;
	}
	if (dfactor != 0 && dfactor != 1 && (dfactor < GL_SRC_COLOR || dfactor > GL_ONE_MINUS_DST_ALPHA))
	{
		gl_set_error_a(GL_INVALID_ENUM, dfactor);
		return;
	}

	gs->blend_func_src = sfactor;
	gs->blend_func_dst = dfactor;
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

uint32_t gl_color_to_framebuffer(const glm::vec4 &c)
{
	//bgra
	return uint32_t(c.b * 0xFF) | (uint32_t(c.g * 0xFF) << 8) | (uint32_t(c.r * 0xFF) << 16) | (uint32_t(c.a * 0xFF) << 24);
}

void APIENTRY glClear(GLbitfield mask)
{
	gl_state *gs = gl_current_state();
	if (!gs) return;
	VALIDATE_NOT_BEGIN_MODE;
	if (mask != (mask & (GL_DEPTH_BUFFER_BIT | GL_ACCUM_BUFFER_BIT | GL_STENCIL_BUFFER_BIT | GL_COLOR_BUFFER_BIT)))
	{
		gl_set_error_a(GL_INVALID_VALUE, mask);
		return;
	}

	glm::ivec4 s = gs->scissor_rect;
	if (gs->scissor_test)
	{
		s.x = glm::max(0, s.x);
		s.y = glm::max(0, s.y);
		s.z = glm::min(s.z,gs->framebuffer->width - s.x);
		s.w = glm::min(s.w,gs->framebuffer->height - s.y);
	}

	const glm::bvec4 &cm = gs->color_mask;
	if (mask & GL_COLOR_BUFFER_BIT && (cm.r || cm.g || cm.b || cm.a))
	{
		uint32_t value = gl_color_to_framebuffer(gs->clear_color);
		uint32_t *dst = (uint32_t *)gs->framebuffer->color;

		//TODO color mask components
		//TODO dither
		if (!gs->scissor_test || (s.x == 0 && s.x + s.z == gs->framebuffer->width && s.y == 0 && s.y + s.w == gs->framebuffer->height))
		{
			int count = gs->framebuffer->width * gs->framebuffer->height;
			for (int i = 0; i < count; i++)
			{
				*dst = value;
				dst++;
			}
		}
		else
		{
			dst += gs->framebuffer->width * s.y;
			dst += s.x;
			for (int iy = 0; iy < s.w; iy++)
			{
				uint32_t *row = dst;
				for (int ix = 0; ix < s.z; ix++)
				{
					*row = value;
					row++;
				}
				row += gs->framebuffer->width * s.y;
			}
		}
	}

	//TODO depth, stencil, accum
}

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
