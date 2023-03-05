#include "pch.h"
#include "gl_exports.h"
#include "gl_state.h"

#define WIN32_LEAN_AND_MEAN // Exclude rarely-used stuff from Windows headers
#include <windows.h>

struct wgl_context
{
	HDC device_context = 0;
	gl_state state;
};

thread_local wgl_context* current_context = nullptr;
thread_local HDC current_hdc = nullptr;

constexpr int pixel_format_count = 1;

gl_state *gl_current_state()
{
	if (!current_context)
		return nullptr;

	return &current_context->state;
}

extern "C" {

EXPORT HGLRC APIENTRY wglCreateContext(HDC device_context)
{
	wgl_context *rc = new wgl_context;
	rc->device_context = device_context;
	rc->state.init();

	return (HGLRC)rc;
}

EXPORT BOOL APIENTRY wglDeleteContext(HGLRC rendering_context)
{
	wgl_context *rc = (wgl_context*)rendering_context;
	if (rc)
	{
		if (current_context == rc)
			current_context = nullptr;

		rc->state.destroy();
		delete rc;
	}
	return 1;
}

EXPORT BOOL APIENTRY wglMakeCurrent(HDC device_context, HGLRC rendering_context)
{
	current_hdc = device_context;
	current_context = (wgl_context *)rendering_context;
	return 1;
}

EXPORT PROC APIENTRY wglGetProcAddress(LPCSTR func_name)
{
#define X(name) \
	{#name, (PROC)name},

	static struct { const char *n; PROC pfn; } funcs[]{
		X(glGetError)
		X(glBegin)
		X(glEnd)
		X(glEdgeFlag)
		X(glEdgeFlagv)
		X(glVertex2d)
		X(glVertex2dv)
		X(glVertex2f)
		X(glVertex2fv)
		X(glVertex2i)
		X(glVertex2iv)
		X(glVertex2s)
		X(glVertex2sv)
		X(glVertex3d)
		X(glVertex3dv)
		X(glVertex3f)
		X(glVertex3fv)
		X(glVertex3i)
		X(glVertex3iv)
		X(glVertex3s)
		X(glVertex3sv)
		X(glVertex4d)
		X(glVertex4dv)
		X(glVertex4f)
		X(glVertex4fv)
		X(glVertex4i)
		X(glVertex4iv)
		X(glVertex4s)
		X(glVertex4sv)
		X(glTexCoord1d)
		X(glTexCoord1dv)
		X(glTexCoord1f)
		X(glTexCoord1fv)
		X(glTexCoord1i)
		X(glTexCoord1iv)
		X(glTexCoord1s)
		X(glTexCoord1sv)
		X(glTexCoord2d)
		X(glTexCoord2dv)
		X(glTexCoord2f)
		X(glTexCoord2fv)
		X(glTexCoord2i)
		X(glTexCoord2iv)
		X(glTexCoord2s)
		X(glTexCoord2sv)
		X(glTexCoord3d)
		X(glTexCoord3dv)
		X(glTexCoord3f)
		X(glTexCoord3fv)
		X(glTexCoord3i)
		X(glTexCoord3iv)
		X(glTexCoord3s)
		X(glTexCoord3sv)
		X(glTexCoord4d)
		X(glTexCoord4dv)
		X(glTexCoord4f)
		X(glTexCoord4fv)
		X(glTexCoord4i)
		X(glTexCoord4iv)
		X(glTexCoord4s)
		X(glTexCoord4sv)
		X(glNormal3b)
		X(glNormal3bv)
		X(glNormal3d)
		X(glNormal3dv)
		X(glNormal3f)
		X(glNormal3fv)
		X(glNormal3i)
		X(glNormal3iv)
		X(glNormal3s)
		X(glNormal3sv)
		X(glColor3b)
		X(glColor3bv)
		X(glColor3d)
		X(glColor3dv)
		X(glColor3f)
		X(glColor3fv)
		X(glColor3i)
		X(glColor3iv)
		X(glColor3s)
		X(glColor3sv)
		X(glColor3ub)
		X(glColor3ubv)
		X(glColor3ui)
		X(glColor3uiv)
		X(glColor3us)
		X(glColor3usv)
		X(glColor4b)
		X(glColor4bv)
		X(glColor4d)
		X(glColor4dv)
		X(glColor4f)
		X(glColor4fv)
		X(glColor4i)
		X(glColor4iv)
		X(glColor4s)
		X(glColor4sv)
		X(glColor4ub)
		X(glColor4ubv)
		X(glColor4ui)
		X(glColor4uiv)
		X(glColor4us)
		X(glColor4usv)
		X(glRects)
		X(glRecti)
		X(glRectf)
		X(glRectd)
		X(glRectsv)
		X(glRectiv)
		X(glRectfv)
		X(glRectdv)
		X(glDepthRange)
		X(glViewport)

		X(glGetString)
		X(glGetIntegerv)
		{nullptr, nullptr}
	};
#undef X

	PROC r = nullptr;
	for (int i = 0; funcs[i].n; i++)
	{
		if (!strcmp(func_name, funcs[i].n))
		{
			r = funcs[i].pfn;
			break;
		}
	}

	return r;
}

EXPORT HDC APIENTRY wglGetCurrentDC(void)
{
	return current_hdc;
}

EXPORT HGLRC APIENTRY wglGetCurrentContext(void)
{
	return (HGLRC)current_context;
}

//internal wgl functions used not directly by user but through gdi

EXPORT int WINAPI wglChoosePixelFormat(HDC device_context, const PIXELFORMATDESCRIPTOR *descriptor)
{
	return 1;
}

EXPORT BOOL WINAPI wglSetPixelFormat(HDC device_context, int pixel_format, const PIXELFORMATDESCRIPTOR *descriptor)
{
	return 1;
}

EXPORT int WINAPI wglDescribePixelFormat(HDC device_context, int pixel_format, UINT size, PPIXELFORMATDESCRIPTOR descriptor)
{
	if (!descriptor)
		return pixel_format_count;

	*descriptor = { 0 };
	descriptor->nSize = sizeof(PIXELFORMATDESCRIPTOR);
	descriptor->nVersion = 1;
	descriptor->dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;

	descriptor->cColorBits = 24;//without alpha?
	descriptor->cRedBits = 8;
	descriptor->cGreenBits = 8;
	descriptor->cBlueBits = 8;
	descriptor->cAlphaBits = 8;
	//bgra
	descriptor->cRedShift = 16;
	descriptor->cGreenShift = 8;
	descriptor->cBlueShift = 0;
	descriptor->cAlphaShift = 24;

	descriptor->cAccumBits = 0;
	descriptor->cAccumRedBits = 0;
	descriptor->cAccumGreenBits = 0;
	descriptor->cAccumBlueBits = 0;
	descriptor->cAccumAlphaBits = 0;

	descriptor->cDepthBits = 0;
	descriptor->cStencilBits = 0;
	descriptor->cAuxBuffers = 0;

	return pixel_format_count;
}

EXPORT BOOL WINAPI wglSwapBuffers(HDC device_context)
{
	return 1;
}

}