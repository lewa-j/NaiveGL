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
		printf("glEndList list %d recorded %d calls, %d bulk bytes\n", gs->display_list_begun, (int)it->second.calls.size(), (int)it->second.data.size());
		it->second.recorded = true;
		std::swap(gs->display_list_indices[gs->display_list_begun], it->second);
		gs->display_list_indices.erase(it);
	}

	gs->display_list_begun = 0;
}

static int gl_callLists_size(GLsizei n, GLenum type)
{
	if (n <= 0)
		return 0;

	if (type == GL_BYTE || type == GL_UNSIGNED_BYTE)
		return n;
	if (type == GL_SHORT || type == GL_UNSIGNED_SHORT || type == GL_2_BYTES)
		return n * 2;
	if (type == GL_3_BYTES)
		return n * 3;
	if (type == GL_INT || type == GL_UNSIGNED_INT || type == GL_FLOAT || type == GL_4_BYTES)
		return n * 4;

	return 0;
}

void gl_callList(gl_state *gs, GLuint list)
{
	if (list == 0)
		return;

	auto it = gs->display_list_indices.find(list);
	if (it == gs->display_list_indices.end())
		return;

	if (!it->second.recorded)
		return;

	const uint8_t *data = it->second.data.data();
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
			glLoadMatrixf((const float*)data);
			data += 16 * sizeof(float);
			break;
		case gl_display_list_call::tMultMatrix:
			glMultMatrixf((const float *)data);
			data += 16 * sizeof(float);
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
		case gl_display_list_call::tTexGeni:
			glTexGeni(call.argsi[0], call.argsi[1], call.argsi[2]);
			break;
		case gl_display_list_call::tTexGenfv:
			glTexGenfv(call.argsi[0], call.argsi[1], call.argsf);
			break;
		case gl_display_list_call::tClipPlane:
			glClipPlane(call.argsi[0], (const double *)data);
			data += 4 * sizeof(double);
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
			glMaterialf(call.argsi[0], call.argsi[1], call.argsf[0]);
			break;
		case gl_display_list_call::tMaterialv:
			glMaterialfv(call.argsi[0], call.argsi[1], call.argsf);
			break;
		case gl_display_list_call::tLightf:
			glLightf(call.argsi[0], call.argsi[1], call.argsf[0]);
			break;
		case gl_display_list_call::tLightfv:
			glLightfv(call.argsi[0], call.argsi[1], call.argsf);
			break;
		case gl_display_list_call::tLightiv:
			glLightiv(call.argsi[0], call.argsi[1], call.argsi + 2);
			break;
		case gl_display_list_call::tLightModeli:
			glLightModeli(call.argsi[0], call.argsi[1]);
			break;
		case gl_display_list_call::tLightModelfv:
			glLightModelfv(call.argsi[0], call.argsf);
			break;
		case gl_display_list_call::tLightModeliv:
			glLightModeliv(call.argsi[0], call.argsi + 1);
			break;
		case gl_display_list_call::tPointSize:
			glPointSize(call.argsf[0]);
			break;
		case gl_display_list_call::tLineWidth:
			glLineWidth(call.argsf[0]);
			break;
		case gl_display_list_call::tLineStipple:
			glLineStipple(call.argsi[0], call.argsi[1]);
			break;
		case gl_display_list_call::tCullFace:
			glCullFace(call.argsi[0]);
			break;
		case gl_display_list_call::tPolygonStipple:
			glPolygonStipple(data);
			data += 4 * 32;
			break;
		case gl_display_list_call::tPolygonMode:
			glPolygonMode(call.argsi[0], call.argsi[1]);
			break;
		case gl_display_list_call::tPixelTransfer:
			glPixelTransferf(call.argsi[0], call.argsf[0]);
			break;
		case gl_display_list_call::tPixelMap:
			if (call.argsi[2] == 1)
				glPixelMapuiv(call.argsi[0], call.argsi[1], (const GLuint *)data);
			else if (call.argsi[2] == 2)
				glPixelMapusv(call.argsi[0], call.argsi[1], (const GLushort *)data);
			else if (call.argsi[2] == 3)
				glPixelMapfv(call.argsi[0], call.argsi[1], (const GLfloat *)data);
			data += call.argsi[3];
			break;
		case gl_display_list_call::tPixelZoom:
			glPixelZoom(call.argsf[0], call.argsf[1]);
			break;
		//DrawPixels
		//Bitmap
		//TexImage
		case gl_display_list_call::tTexParameter:
			glTexParameteri(call.argsi[0], call.argsi[1], call.argsi[2]);
			break;
		case gl_display_list_call::tTexParameteriv:
			glTexParameteriv(call.argsi[0], call.argsi[1], call.argsi + 2);
			break;
		case gl_display_list_call::tTexParameterfv:
			glTexParameterfv(call.argsi[0], call.argsi[1], call.argsf);
			break;
		case gl_display_list_call::tTexEnv:
			glTexEnvi(call.argsi[0], call.argsi[1], call.argsi[2]);
			break;
		case gl_display_list_call::tTexEnviv:
			glTexEnviv(call.argsi[0], call.argsi[1], call.argsi + 2);
			break;
		case gl_display_list_call::tTexEnvfv:
			glTexEnvfv(call.argsi[0], call.argsi[1], call.argsf);
			break;
		case gl_display_list_call::tFog:
			glFogf(call.argsi[0], call.argsf[0]);
			break;
		case gl_display_list_call::tFogiv:
			glFogiv(call.argsi[0], call.argsi + 1);
			break;
		case gl_display_list_call::tFogfv:
			glFogfv(call.argsi[0], call.argsf);
			break;
		case gl_display_list_call::tScissor:
			glScissor(call.argsi[0], call.argsi[1], call.argsi[2], call.argsi[3]);
			break;
		case gl_display_list_call::tAlphaFunc:
			glAlphaFunc(call.argsi[0], call.argsf[0]);
			break;
		case gl_display_list_call::tStencilFunc:
			glStencilFunc(call.argsi[0], call.argsi[1], call.argsi[2]);
			break;
		case gl_display_list_call::tStencilOp:
			glStencilOp(call.argsi[0], call.argsi[1], call.argsi[2]);
			break;
		case gl_display_list_call::tDepthFunc:
			glDepthFunc(call.argsi[0]);
			break;
		case gl_display_list_call::tBlendFunc:
			glBlendFunc(call.argsi[0], call.argsi[1]);
			break;
		case gl_display_list_call::tLogicOp:
			glLogicOp(call.argsi[0]);
			break;
		case gl_display_list_call::tDrawBuffer:
			glDrawBuffer(call.argsi[0]);
			break;
		case gl_display_list_call::tColorMask:
			glColorMask(call.argsi[0], call.argsi[1], call.argsi[2], call.argsi[3]);
			break;
		case gl_display_list_call::tIndexMask:
			glIndexMask(call.argsi[0]);
			break;
		case gl_display_list_call::tDepthMask:
			glDepthMask(call.argsi[0]);
			break;
		case gl_display_list_call::tStencilMask:
			glStencilMask(call.argsi[0]);
			break;
		case gl_display_list_call::tClear:
			glClear(call.argsi[0]);
			break;
		case gl_display_list_call::tClearColor:
			glClearColor(call.argsf[0], call.argsf[1], call.argsf[2], call.argsf[3]);
			break;
		case gl_display_list_call::tClearIndex:
			glClearIndex(call.argsf[0]);
			break;
		case gl_display_list_call::tClearDepth:
			glClearDepth(call.argsf[0]);
			break;
		case gl_display_list_call::tClearStencil:
			glClearStencil(call.argsi[0]);
			break;
		case gl_display_list_call::tClearAccum:
			glClearAccum(call.argsf[0], call.argsf[1], call.argsf[2], call.argsf[3]);
			break;
		case gl_display_list_call::tAccum:
			glAccum(call.argsi[0], call.argsf[0]);
			break;
		case gl_display_list_call::tReadBuffer:
			glReadBuffer(call.argsi[0]);
			break;
		//CopyPixels
		case gl_display_list_call::tMap1:
			glMap1f(call.argsi[0], call.argsf[0], call.argsf[1], call.argsi[1], call.argsi[2], (const float *)data);
			data += call.argsi[3];
			break;
		case gl_display_list_call::tMap2:
			glMap2f(call.argsi[0], call.argsf[0], call.argsf[1], call.argsi[1], call.argsi[2], call.argsf[2], call.argsf[3], call.argsi[3], call.argsi[4], (const float *)data);
			data += call.argsi[5];
			break;
		case gl_display_list_call::tEvalCoord1:
			glEvalCoord1fv(call.argsf);
			break;
		case gl_display_list_call::tEvalCoord2:
			glEvalCoord2fv(call.argsf);
			break;
		case gl_display_list_call::tMapGrid1:
			glMapGrid1f(call.argsi[0], call.argsf[0], call.argsf[1]);
			break;
		case gl_display_list_call::tMapGrid2:
			glMapGrid2f(call.argsi[0], call.argsf[0], call.argsf[1], call.argsi[1], call.argsf[2], call.argsf[3]);
			break;
		case gl_display_list_call::tEvalMesh1:
			glEvalMesh1(call.argsi[0], call.argsi[1], call.argsi[2]);
			break;
		case gl_display_list_call::tEvalMesh2:
			glEvalMesh2(call.argsi[0], call.argsi[1], call.argsi[2], call.argsi[3], call.argsi[4]);
			break;
		case gl_display_list_call::tEvalPoint1:
			glEvalPoint1(call.argsi[0]);
			break;
		case gl_display_list_call::tEvalPoint2:
			glEvalPoint2(call.argsi[0], call.argsi[1]);
			break;
		case gl_display_list_call::tInitNames:
			glInitNames();
			break;
		case gl_display_list_call::tPopName:
			glPopName();
			break;
		case gl_display_list_call::tPushName:
			glPushName(call.argsi[0]);
			break;
		case gl_display_list_call::tLoadName:
			glLoadName(call.argsi[0]);
			break;
		case gl_display_list_call::tPassThrough:
			glPassThrough(call.argsf[0]);
			break;
		case gl_display_list_call::tCallList:
			glCallList(call.argsi[0]);
			break;
		case gl_display_list_call::tCallLists:
			glCallLists(call.argsi[0], call.argsi[1], data);
			data += gl_callLists_size(call.argsi[0], call.argsi[1]);
			break;
		case gl_display_list_call::tListBase:
			glListBase(call.argsi[0]);
			break;
		case gl_display_list_call::tHint:
			glHint(call.argsi[0], call.argsi[1]);
			break;
		}
	}
}

void APIENTRY glCallList(GLuint list)
{
	gl_state *gs = gl_current_state();
	if (!gs) return;
	WRITE_DISPLAY_LIST(CallList, {}, { (int)list });
	if (list == 0)
	{
		gl_set_error(GL_INVALID_VALUE);
		return;
	}

	int save = gs->display_list_begun;
	gs->display_list_begun = 0;

	gl_callList(gs, list);

	gs->display_list_begun = save;
}

void APIENTRY glCallLists(GLsizei n, GLenum type, const void* lists)
{
	gl_state* gs = gl_current_state();
	if (!gs) return;
	WRITE_DISPLAY_LIST_BULK(CallLists, lists, gl_callLists_size(n, type), {}, { n, (int)type });
	if (n < 0)
	{
		gl_set_error(GL_INVALID_VALUE);
		return;
	}

	int save = gs->display_list_begun;
	gs->display_list_begun = 0;
	
	const int base = gs->display_list_base;

	switch (type)
	{
	case GL_BYTE:
		for (int i = 0; i < n; i++)
			gl_callList(gs, base + ((int8_t*)lists)[i]);
		break;
	case GL_UNSIGNED_BYTE:
		for (int i = 0; i < n; i++)
			gl_callList(gs, base + ((uint8_t*)lists)[i]);
		break;
	case GL_SHORT:
		for (int i = 0; i < n; i++)
			gl_callList(gs, base + ((int16_t*)lists)[i]);
		break;
	case GL_UNSIGNED_SHORT:
		for (int i = 0; i < n; i++)
			gl_callList(gs, base + ((uint16_t*)lists)[i]);
		break;
	case GL_INT:
		for (int i = 0; i < n; i++)
			gl_callList(gs, base + ((int32_t*)lists)[i]);
		break;
	case GL_UNSIGNED_INT:
		for (int i = 0; i < n; i++)
			gl_callList(gs, base + ((uint32_t*)lists)[i]);
		break;
	case GL_FLOAT:
		for (int i = 0; i < n; i++)
			gl_callList(gs, base + (GLuint)((float*)lists)[i]);
		break;
	case GL_2_BYTES:
	{
		const uint8_t* bytes = (uint8_t*)lists;
		for (int i = 0; i < n; i++)
		{
			gl_callList(gs, base + bytes[0] * 0x100 + bytes[1]);
			bytes += 2;
		}
		break;
	}
	case GL_3_BYTES:
	{
		const uint8_t* bytes = (uint8_t*)lists;
		for (int i = 0; i < n; i++)
		{
			gl_callList(gs, base + bytes[0] * 0x10000 + bytes[1] * 0x100 + bytes[2]);
			bytes += 3;
		}
		break;
	}
	case GL_4_BYTES:
	{
		const uint8_t* bytes = (uint8_t*)lists;
		for (int i = 0; i < n; i++)
		{
			gl_callList(gs, base + bytes[0] * 0x1000000 + bytes[1] * 0x10000 + bytes[2] * 0x100 + bytes[3]);
			bytes += 4;
		}
		break;
	}
	default:
		gl_set_error_a(GL_INVALID_ENUM, type);
	}

	gs->display_list_begun = save;
}

void APIENTRY glListBase(GLuint base)
{
	gl_state* gs = gl_current_state();
	if (!gs) return;
	WRITE_DISPLAY_LIST(ListBase, {}, { (int)base });
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
