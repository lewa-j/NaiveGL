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

#define VALIDATE_TEST_FUNC(func) \
if (func < GL_NEVER || func > GL_ALWAYS)\
{\
	gl_set_error_a(GL_INVALID_ENUM, func);\
	return;\
}

void APIENTRY glAlphaFunc(GLenum func, GLfloat ref)
{
	gl_state* gs = gl_current_state();
	if (!gs) return;
	VALIDATE_NOT_BEGIN_MODE;
	VALIDATE_TEST_FUNC(func);

	gs->alpha_test_func = func;
	gs->alpha_test_ref = glm::clamp(ref, 0.f, 1.f);
}

void APIENTRY glStencilFunc(GLenum func, GLint ref, GLuint mask)
{
	gl_state* gs = gl_current_state();
	if (!gs) return;
	VALIDATE_NOT_BEGIN_MODE;
	VALIDATE_TEST_FUNC(func);

	gs->stencil_func = func;
	//TODO clamp to actual max stencil buffer value
	gs->stencil_test_ref = glm::clamp(ref, 0, 0xFF);
	gs->stencil_test_mask = mask;
}

#define VALIDATE_STENCIL_OP(op) \
if ((op < GL_KEEP || op > GL_DECR) && op != GL_ZERO && op != GL_INVERT)\
{\
	gl_set_error_a(GL_INVALID_ENUM, op);\
	return;\
}

void APIENTRY glStencilOp(GLenum sfail, GLenum dpfail, GLenum dppass)
{
	gl_state* gs = gl_current_state();
	if (!gs) return;
	VALIDATE_NOT_BEGIN_MODE;
	VALIDATE_STENCIL_OP(sfail);
	VALIDATE_STENCIL_OP(dpfail);
	VALIDATE_STENCIL_OP(dppass);

	gs->stencil_op_sfail = sfail;
	gs->stencil_op_dpfail = dpfail;
	gs->stencil_op_dppass = dppass;
}

void APIENTRY glDepthFunc(GLenum func)
{
	gl_state* gs = gl_current_state();
	if (!gs) return;
	VALIDATE_NOT_BEGIN_MODE;
	VALIDATE_TEST_FUNC(func);

	gs->depth_func = func;
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

void APIENTRY glLogicOp(GLenum opcode)
{
	gl_state* gs = gl_current_state();
	if (!gs) return;
	VALIDATE_NOT_BEGIN_MODE;
	if (opcode < GL_CLEAR || opcode > GL_SET)
	{
		gl_set_error_a(GL_INVALID_ENUM, opcode);
		return;
	}

	gs->logic_op_mode = opcode;
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

	// Monoscopic context
	if (buf == GL_BACK_RIGHT || buf == GL_FRONT_RIGHT || buf == GL_RIGHT)
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

	// Monoscopic context
	if (src == GL_BACK_RIGHT || src == GL_FRONT_RIGHT || src == GL_RIGHT)
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

	if (gs->render_mode != GL_RENDER)
		return;

	gl_framebuffer& fb = *gs->framebuffer;

	glm::ivec4 s = gs->scissor_rect;
	if (gs->scissor_test)
	{
		s.x = glm::max(0, s.x);
		s.y = glm::max(0, s.y);
		s.z = glm::min(s.z, fb.width - gs->scissor_rect.x);
		s.w = glm::min(s.w, fb.height - gs->scissor_rect.y);
	}

	const glm::bvec4 &cm = gs->color_mask;
	if (mask & GL_COLOR_BUFFER_BIT && (cm.r || cm.g || cm.b || cm.a) && gs->draw_buffer != GL_NONE)
	{
		uint32_t mask = (cm.r ? 0xFF0000 : 0) | (cm.g ? 0xFF00 : 0) | (cm.b ? 0xFF : 0) | (cm.a ? 0xFF0000 : 0);//bgra
		uint32_t value = gl_color_to_framebuffer(gs->clear_color) & mask;
		uint32_t *dst = (uint32_t *)fb.color;

		//TODO dither
		if (!gs->scissor_test || (s.x == 0 && s.x + s.z == fb.width && s.y == 0 && s.y + s.w == fb.height))
		{
			int count = fb.width * fb.height;
			if (mask == 0xFFFFFFFF)
			{
				for (int i = 0; i < count; i++)
					*(dst++) = value;
			}
			else
			{
				for (int i = 0; i < count; i++)
				{
					*dst = value| (*dst & ~mask);
					dst++;
				}
			}
		}
		else
		{
			dst += fb.width * s.y;
			dst += s.x;
			for (int iy = 0; iy < s.w; iy++)
			{
				uint32_t *row = dst;
				if (mask == 0xFFFFFFFF)
				{
					for (int ix = 0; ix < s.z; ix++)
						*(row++) = value;
				}
				else
				{
					for (int ix = 0; ix < s.z; ix++)
					{
						*row = value | (*row & ~mask);
						row++;
					}
				}
				dst += fb.width;
			}
		}
	}

	uint8_t stmask = (gs->stencil_writemask & 0xFF);
	if (mask & GL_STENCIL_BUFFER_BIT && fb.stencil && stmask != 0)
	{
		uint8_t val = gs->clear_stencil & stmask;
		uint8_t* dst = fb.stencil;

		if (!gs->scissor_test || (s.x == 0 && s.x + s.z == fb.width && s.y == 0 && s.y + s.w == fb.height))
		{
			int count = fb.width * fb.height;
			if (stmask == 0xFF)
			{
				for (int i = 0; i < count; i++)
					*(dst++) = val;
			}
			else
			{
				for (int i = 0; i < count; i++)
				{
					*dst = val | (*dst & ~stmask);
					dst++;
				}
			}
		}
		else
		{
			dst += fb.width * s.y;
			dst += s.x;
			for (int iy = 0; iy < s.w; iy++)
			{
				uint8_t* row = dst;
				if (stmask == 0xFF)
				{
					for (int ix = 0; ix < s.z; ix++)
						*(row++) = val;
				}
				else
				{
					for (int ix = 0; ix < s.z; ix++)
					{
						*row = val | (*row & ~stmask);
						row++;
					}
				}
				dst += fb.width;
			}
		}
	}

	if (mask & GL_DEPTH_BUFFER_BIT && fb.depth && gs->depth_mask)
	{
		uint16_t value = uint16_t(gs->clear_depth * 0xFFFF);
		uint16_t* dst = fb.depth;

		if (!gs->scissor_test || (s.x == 0 && s.x + s.z == fb.width && s.y == 0 && s.y + s.w == fb.height))
		{
			int count = fb.width * fb.height;
			for (int i = 0; i < count; i++)
				*(dst++) = value;
		}
		else
		{
			dst += fb.width * s.y;
			dst += s.x;
			for (int iy = 0; iy < s.w; iy++)
			{
				uint16_t* row = dst;
				for (int ix = 0; ix < s.z; ix++)
					*(row++) = value;
				dst += fb.width;
			}
		}
	}

	//TODO accum
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

void APIENTRY glAccum(GLenum op, GLfloat value)
{
	gl_state* gs = gl_current_state();
	if (!gs) return;
	VALIDATE_NOT_BEGIN_MODE;
	if (op < GL_ACCUM || op > GL_ADD)
	{
		gl_set_error_a(GL_INVALID_ENUM, op);
		return;
	}

	gl_framebuffer& fb = *gs->framebuffer;
	glm::vec4* dst = fb.accum;

	if (!dst)
	{
		gl_set_error(GL_INVALID_OPERATION);
		return;
	}

	glm::ivec4 s{ 0, 0, fb.width, fb.height };
	if (gs->scissor_test)
	{
		s = gs->scissor_rect;
		s.x = glm::max(0, s.x);
		s.y = glm::max(0, s.y);
		s.z = glm::min(s.z, fb.width - gs->scissor_rect.x);
		s.w = glm::min(s.w, fb.height - gs->scissor_rect.y);
	}

	uint8_t* src = fb.color + (fb.width * s.y + s.x) * 4;
	dst += fb.width * s.y;
	dst += s.x;

	if (op == GL_ACCUM || op == GL_LOAD)
	{
		for (int iy = 0; iy < s.w; iy++)
		{
			glm::vec4* row = dst;
				
			for (int ix = 0; ix < s.z; ix++)
			{
				glm::vec4 src_color = glm::vec4(src[2], src[1], src[0], src[3]) / 255.f;

				if (op == GL_ACCUM)
					(*row) += src_color * value;
				else if (op == GL_LOAD)
					(*row) = src_color * value;
					
				src += 4;
				row++;
			}
				
			src += fb.width * 4;
			dst += fb.width;
		}
	}
	else if (op == GL_RETURN)
	{
		if (gs->draw_buffer == GL_NONE)
			return;

		for (int iy = 0; iy < s.w; iy++)
		{
			glm::vec4* row = dst;

			for (int ix = 0; ix < s.z; ix++)
			{
				glm::vec4 color = glm::clamp(*(row++) * value, 0.f, 1.f);

				if (gs->dither)
					gl_dither(color, s.x + ix, s.y + iy);

				//bgra
				if (gs->color_mask.b)
					src[0] = uint8_t(color.b * 0xFF);
				if (gs->color_mask.g)
					src[1] = uint8_t(color.g * 0xFF);
				if (gs->color_mask.r)
					src[2] = uint8_t(color.r * 0xFF);
				if (gs->color_mask.a)
					src[3] = uint8_t(color.a * 0xFF);

				src += 4;
			}
			src += fb.width * 4;
			dst += fb.width;
		}
	}
	else if (op == GL_MULT)
	{
		for (int iy = 0; iy < s.w; iy++)
		{
			glm::vec4* row = dst;

			for (int ix = 0; ix < s.z; ix++)
				*(row++) *= value;

			dst += fb.width;
		}
	}
	else if (op == GL_ADD)
	{
		for (int iy = 0; iy < s.w; iy++)
		{
			glm::vec4* row = dst;

			for (int ix = 0; ix < s.z; ix++)
				*(row++) += value;

			dst += fb.width;
		}
	}
}
