#include "pch.h"
#include "gl_state.h"
#include "gl_exports.h"

EXPORT void APIENTRY glNewList(GLuint list, GLenum mode)
{
	gl_state* gs = gl_current_state();
	if (!gs) return;
	VALIDATE_NOT_BEGIN_MODE;

	if (mode != GL_COMPILE && mode != GL_COMPILE_AND_EXECUTE)
	{
		gl_set_error_a(GL_INVALID_ENUM, mode);
		return;
	}

	if (list == 0)
	{
		gl_set_error(GL_INVALID_VALUE);
		return;
	}

	if (gs->display_list_begun)
	{
		gl_set_error(GL_INVALID_OPERATION);
		return;
	}

	gs->display_list_begun = list;
}

EXPORT void APIENTRY glEndList(void)
{
	gl_state* gs = gl_current_state();
	if (!gs) return;
	VALIDATE_NOT_BEGIN_MODE;

	if (gs->display_list_begun == 0)
	{
		gl_set_error(GL_INVALID_OPERATION);
		return;
	}

	gs->display_list_begun = 0;
}

void gl_callList(GLuint list)
{
	//TODO impl
}

EXPORT void APIENTRY glCallList(GLuint list)
{
	if (list == 0)
	{
		gl_set_error(GL_INVALID_VALUE);
		return;
	}

	gl_callList(list);
}

EXPORT void APIENTRY glCallLists(GLsizei n, GLenum type, const void* lists)
{
	gl_state* gs = gl_current_state();
	if (!gs) return;

	if (n < 0)
	{
		gl_set_error(GL_INVALID_VALUE);
		return;
	}

	switch (type)
	{
	case GL_BYTE:
		for (int i = 0; i < n; i++)
			gl_callList(gs->display_list_base + ((int8_t*)lists)[i]);
		break;
	case GL_UNSIGNED_BYTE:
		for (int i = 0; i < n; i++)
			gl_callList(gs->display_list_base + ((uint8_t*)lists)[i]);
		break;
	case GL_SHORT:
		for (int i = 0; i < n; i++)
			gl_callList(gs->display_list_base + ((int16_t*)lists)[i]);
		break;
	case GL_UNSIGNED_SHORT:
		for (int i = 0; i < n; i++)
			gl_callList(gs->display_list_base + ((uint16_t*)lists)[i]);
		break;
	case GL_INT:
		for (int i = 0; i < n; i++)
			gl_callList(gs->display_list_base + ((int32_t*)lists)[i]);
		break;
	case GL_UNSIGNED_INT:
		for (int i = 0; i < n; i++)
			gl_callList(gs->display_list_base + ((uint32_t*)lists)[i]);
		break;
	case GL_FLOAT:
		for (int i = 0; i < n; i++)
			gl_callList(gs->display_list_base + (GLuint)((float*)lists)[i]);
		break;
	case GL_2_BYTES:
	{
		const uint8_t* bytes = (uint8_t*)lists;
		for (int i = 0; i < n; i++)
		{
			gl_callList(gs->display_list_base + bytes[0] * 0x100 + bytes[1]);
			bytes += 2;
		}
		break;
	}
	case GL_3_BYTES:
	{
		const uint8_t* bytes = (uint8_t*)lists;
		for (int i = 0; i < n; i++)
		{
			gl_callList(gs->display_list_base + bytes[0] * 0x10000 + bytes[1] * 0x100 + bytes[2]);
			bytes += 3;
		}
		break;
	}
	case GL_4_BYTES:
	{
		const uint8_t* bytes = (uint8_t*)lists;
		for (int i = 0; i < n; i++)
		{
			gl_callList(gs->display_list_base + bytes[0] * 0x1000000 + bytes[1] * 0x10000 + bytes[2] * 0x100 + bytes[3]);
			bytes += 4;
		}
		break;
	}
	default:
		gl_set_error_a(GL_INVALID_ENUM, type);
		return;
	}
}

EXPORT void APIENTRY glListBase(GLuint base)
{
	gl_state* gs = gl_current_state();
	if (!gs) return;
	VALIDATE_NOT_BEGIN_MODE;

	gs->display_list_base = base;
}

EXPORT GLuint APIENTRY glGenLists(GLsizei range)
{
	gl_state* gs = gl_current_state();
	if (!gs) return 0;

	if (gs->begin_primitive_mode != -1)
	{
		gl_set_error(GL_INVALID_OPERATION);
		return 0;
	}

	if (range < 0)
	{
		gl_set_error(GL_INVALID_VALUE);
		return 0;
	}

	//TODO impl
	return 0;
}

EXPORT GLboolean APIENTRY glIsList(GLuint list)
{
	gl_state* gs = gl_current_state();
	if (!gs) return GL_FALSE;

	if (gs->begin_primitive_mode != -1)
	{
		gl_set_error(GL_INVALID_OPERATION);
		return GL_FALSE;
	}

	//TODO impl
	return GL_FALSE;
}

EXPORT void APIENTRY glDeleteLists(GLuint list, GLsizei range)
{
	gl_state* gs = gl_current_state();
	if (!gs) return;
	VALIDATE_NOT_BEGIN_MODE;

	if (range < 0)
	{
		gl_set_error(GL_INVALID_VALUE);
		return;
	}

	//TODO impl
}
