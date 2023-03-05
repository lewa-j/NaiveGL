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