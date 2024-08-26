#include "pch.h"
#include "gl_state.h"
#include "gl_exports.h"

void APIENTRY glNewList(GLuint list, GLenum mode)
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

	gs->display_list_execute = (mode == GL_COMPILE_AND_EXECUTE);

	gs->display_list_begun = list;
	gs->display_list_indices[0] = {};
}

void APIENTRY glEndList(void)
{
	gl_state* gs = gl_current_state();
	if (!gs) return;
	VALIDATE_NOT_BEGIN_MODE;

	if (gs->display_list_begun == 0)
	{
		gl_set_error(GL_INVALID_OPERATION);
		return;
	}

	auto it = gs->display_list_indices.find(0);
	if (it != gs->display_list_indices.end())
	{
		printf("glEndList list %d recorded %d calls\n", gs->display_list_begun, (int)it->second.calls.size());
		it->second.recorded = true;
		std::swap(gs->display_list_indices[gs->display_list_begun], it->second);
		gs->display_list_indices.erase(it);
	}

	gs->display_list_begun = 0;
}

void gl_callList(GLuint list)
{
	if (list == 0)
		return;

	gl_state* gs = gl_current_state();
	if (!gs) return;

	auto it = gs->display_list_indices.find(list);
	if (it == gs->display_list_indices.end())
		return;

	if (!it->second.recorded)
		return;

	for (size_t i = 0; i < it->second.calls.size(); i++)
	{
		const gl_display_list_call& call = it->second.calls[i];
		switch (call.type)
		{
		case gl_display_list_call::tBegin:
			glBegin(call.argsi[0]);
			break;
		case gl_display_list_call::tEnd:
			glEnd();
			break;
		case gl_display_list_call::tVertex:
			glVertex4fv(call.argsf);
			break;
		case gl_display_list_call::tEdgeFlag:
			glEdgeFlag(call.argsi[0]);
			break;
		case gl_display_list_call::tTexCoord:
			glTexCoord4fv(call.argsf);
			break;
		case gl_display_list_call::tNormal:
			glNormal3fv(call.argsf);
			break;
		case gl_display_list_call::tColor:
			glColor4fv(call.argsf);
			break;
		case gl_display_list_call::tRect:
			glRectfv(call.argsf, call.argsf + 2);
			break;
		case gl_display_list_call::tDepthRange:
			glDepthRange(call.argsf[0], call.argsf[1]);
			break;
		case gl_display_list_call::tViewport:
			glViewport(call.argsi[0], call.argsi[1], call.argsi[2], call.argsi[3]);
			break;
		case gl_display_list_call::tMatrixMode:
			glMatrixMode(call.argsi[0]);
			break;
		case gl_display_list_call::tLoadMatrix:
			glLoadMatrixf(call.argsf);
			break;
		case gl_display_list_call::tMultMatrix:
			glMultMatrixf(call.argsf);
			break;
		case gl_display_list_call::tLoadIdentity:
			glLoadIdentity();
			break;
		case gl_display_list_call::tRotate:
			glRotatef(call.argsf[0], call.argsf[1], call.argsf[2], call.argsf[3]);
			break;
		case gl_display_list_call::tTranslate:
			glTranslatef(call.argsf[0], call.argsf[1], call.argsf[2]);
			break;
		case gl_display_list_call::tScale:
			glScalef(call.argsf[0], call.argsf[1], call.argsf[2]);
			break;
		case gl_display_list_call::tFrustum:
			glFrustum(call.argsf[0], call.argsf[1], call.argsf[2], call.argsf[3], call.argsf[4], call.argsf[5]);
			break;
		case gl_display_list_call::tOrtho:
			glOrtho(call.argsf[0], call.argsf[1], call.argsf[2], call.argsf[3], call.argsf[4], call.argsf[5]);
			break;
		case gl_display_list_call::tPushMatrix:
			glPushMatrix();
			break;
		case gl_display_list_call::tPopMatrix:
			glPopMatrix();
			break;
		case gl_display_list_call::tEnable:
			glEnable(call.argsi[0]);
			break;
		case gl_display_list_call::tDisable:
			glDisable(call.argsi[0]);
			break;
		case gl_display_list_call::tTexGen:
			glTexGenfv(call.argsi[0], call.argsi[1], call.argsf);
			break;
		case gl_display_list_call::tClipPlane:
			glClipPlane(call.argsi[0], (double *)call.argsf);
			break;
		case gl_display_list_call::tRasterPos:
			glRasterPos4fv(call.argsf);
			break;
		case gl_display_list_call::tFrontFace:
			glFrontFace(call.argsi[0]);
			break;
		case gl_display_list_call::tColorMaterial:
			glColorMaterial(call.argsi[0], call.argsi[1]);
			break;
		case gl_display_list_call::tShadeModel:
			glShadeModel(call.argsi[0]);
			break;
		case gl_display_list_call::tMaterial:
			glMaterialfv(call.argsi[0], call.argsi[1], call.argsf);
			break;
		case gl_display_list_call::tLightf:
			glLightfv(call.argsi[0], call.argsi[1], call.argsf);
			break;
		case gl_display_list_call::tLighti:
			glLightiv(call.argsi[0], call.argsi[1], call.argsi + 2);
			break;
		case gl_display_list_call::tLightModel:
			glLightModelfv(call.argsi[0], call.argsf);
			break;
		}
	}
}

void APIENTRY glCallList(GLuint list)
{
	if (list == 0)
	{
		gl_set_error(GL_INVALID_VALUE);
		return;
	}

	gl_callList(list);
}

void APIENTRY glCallLists(GLsizei n, GLenum type, const void* lists)
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

void APIENTRY glListBase(GLuint base)
{
	gl_state* gs = gl_current_state();
	if (!gs) return;
	VALIDATE_NOT_BEGIN_MODE;

	gs->display_list_base = base;
}

GLuint APIENTRY glGenLists(GLsizei range)
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

	if (range == 0)
		return 0;

	int start = 1;
	while (start < 0xFFFFFF)
	{
		while (gs->display_list_indices.find(start) != gs->display_list_indices.end())
			start++;

		int i = 1;
		for (; i < range; i++)
		{
			if (gs->display_list_indices.find(start + i) != gs->display_list_indices.end())
				break;
		}
		if (i == range)
		{
			for (int l = start; l < start + range; l++)
				gs->display_list_indices[l] = {};
			printf("glGenLists(%d) = %d (total size %d)\n", range, start, (int)gs->display_list_indices.size());
			return start;
		}
		start += i;
	}

	printf("glGenLists(%d) failed\n", range);
	return 0;
}

GLboolean APIENTRY glIsList(GLuint list)
{
	gl_state* gs = gl_current_state();
	if (!gs) return GL_FALSE;

	if (gs->begin_primitive_mode != -1)
	{
		gl_set_error(GL_INVALID_OPERATION);
		return GL_FALSE;
	}

	auto it = gs->display_list_indices.find(list);
	if (it == gs->display_list_indices.end())
		return GL_FALSE;

	return it->second.recorded ? GL_TRUE : GL_FALSE;
}

void APIENTRY glDeleteLists(GLuint list, GLsizei range)
{
	gl_state* gs = gl_current_state();
	if (!gs) return;
	VALIDATE_NOT_BEGIN_MODE;

	if (range < 0)
	{
		gl_set_error(GL_INVALID_VALUE);
		return;
	}

	for (GLuint i = list; i < list + range; i++)
	{
		auto it = gs->display_list_indices.find(list);
		if (it != gs->display_list_indices.end())
		{
			gs->display_list_indices.erase(it);
			printf("glDeleteLists(%d, %d) %d deleted (total size %d)\n", list, range, i, (int)gs->display_list_indices.size());
		}
	}
}
