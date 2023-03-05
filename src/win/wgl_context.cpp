#include "pch.h"
#include "gl_exports.h"

#define WIN32_LEAN_AND_MEAN // Exclude rarely-used stuff from Windows headers
#include <windows.h>

struct wgl_context
{
	HDC device_context;
};

constexpr int pixel_format_count = 1;

extern "C" {

EXPORT HGLRC APIENTRY wglCreateContext(HDC device_context)
{
	wgl_context *rc = new wgl_context;
	rc->device_context = device_context;
	return (HGLRC)rc;
}

EXPORT BOOL APIENTRY wglDeleteContext(HGLRC rendering_context)
{
	wgl_context *rc = (wgl_context*)rendering_context;
	delete rc;
	return 1;
}

EXPORT BOOL APIENTRY wglMakeCurrent(HDC device_context, HGLRC rendering_context)
{
	return 1;
}

EXPORT PROC APIENTRY wglGetProcAddress(LPCSTR func_name)
{
#define X(name) \
	{#name, (PROC)name},

	static struct { const char *n; PROC pfn; } funcs[]{
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
	return 0;
}

EXPORT HGLRC APIENTRY wglGetCurrentContext(void)
{
	return 0;
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

	return pixel_format_count;
}

EXPORT BOOL WINAPI wglSwapBuffers(HDC device_context)
{
	return 1;
}

}