#include "pch.h"
#include "gl_state.h"

#define WIN32_LEAN_AND_MEAN // Exclude rarely-used stuff from Windows headers
#include <windows.h>

#include "gl_exports.h"

struct wgl_context
{
	HDC device_context = 0;
	gl_state state;
	gl_framebuffer framebuffer;

	BITMAPINFO bitmap_info;
	HBITMAP bitmap = 0;
	HDC bitmap_dc = 0;
};

thread_local wgl_context* current_context = nullptr;
thread_local HDC current_hdc = nullptr;
thread_local int current_pixel_format = 1;

constexpr int pixel_format_count = 4;
//color
//color + depth
//color + stencil
//color + depth + stencil

gl_state *gl_current_state()
{
	if (!current_context)
		return nullptr;

	return &current_context->state;
}

extern "C" {

EXPORT HGLRC APIENTRY wglCreateContext(HDC device_context)
{
	printf("wglCreateContext(%p) pixelformat = %d\n", device_context, current_pixel_format);
	wgl_context *rc = new wgl_context;
	rc->device_context = device_context;

	int w = 640;
	int h = 480;
	int depth = ((current_pixel_format - 1) & 1) ? 16 : 0;
	int stencil = ((current_pixel_format - 1) & 2) ? 8 : 0;
	bool doublebuffer = true;
	HWND win = WindowFromDC(device_context);
	if (win)
	{
		RECT rect{ 0 };
		GetClientRect(win, &rect);
		w = rect.right - rect.left;
		h = rect.bottom - rect.top;
	}

	BITMAPINFOHEADER &bmih = rc->bitmap_info.bmiHeader;
	bmih.biSize = sizeof(BITMAPINFOHEADER);
	bmih.biWidth = w;
	bmih.biHeight = h;
	bmih.biPlanes = 1;
	bmih.biBitCount = 32;
	bmih.biCompression = BI_RGB;
	rc->bitmap_dc = CreateCompatibleDC(device_context);
	rc->bitmap = CreateDIBSection(device_context, &rc->bitmap_info, DIB_RGB_COLORS, (void **)&rc->framebuffer.color, NULL, 0);
	if (rc->bitmap)
		SelectObject(rc->bitmap_dc, rc->bitmap);

	rc->framebuffer.width = w;
	rc->framebuffer.height = h;
	rc->framebuffer.doublebuffer = doublebuffer;
	rc->framebuffer.depth = nullptr;
	rc->framebuffer.stencil = nullptr;
	if (depth)
		rc->framebuffer.depth = new uint16_t[w * h];
	if (stencil)
		rc->framebuffer.stencil = new uint8_t[w * h];

	rc->state.framebuffer = &rc->framebuffer;
	rc->state.init(w, h, doublebuffer);

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
		delete[] rc->framebuffer.depth;
		delete[] rc->framebuffer.stencil;

		DeleteObject(rc->bitmap);
		DeleteDC(rc->bitmap_dc);

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
	if (!current_context)
		return nullptr;

#define X(name) \
	{#name, (PROC)name},

	static struct { const char *n; PROC pfn; } funcs[]{
#include "gl_funcs_x.h"
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
	int ret = 0;
	if (descriptor)
	{
		if (descriptor->cDepthBits)
			ret += 1;
		if (descriptor->cStencilBits)
			ret += 2;
	}
	return ret + 1;
}

EXPORT BOOL WINAPI wglSetPixelFormat(HDC device_context, int pixel_format, const PIXELFORMATDESCRIPTOR *descriptor)
{
	if (pixel_format < 1 || pixel_format > pixel_format_count)
		return 0;

	current_pixel_format = pixel_format;
	return 1;
}

EXPORT int WINAPI wglGetPixelFormat(HDC device_context)
{
	return current_pixel_format;
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
	descriptor->cDepthBits = ((pixel_format - 1) & 1) ? 16 : 0;
	descriptor->cStencilBits = ((pixel_format - 1) & 2) ? 8 : 0;
	descriptor->cAuxBuffers = 0;

	return pixel_format_count;
}

static void resize_context(wgl_context* rc, int w, int h)
{
	DeleteObject(rc->bitmap);
	BITMAPINFOHEADER& bmih = rc->bitmap_info.bmiHeader;
	bmih.biWidth = w;
	bmih.biHeight = h;
	rc->bitmap = CreateDIBSection(rc->device_context, &rc->bitmap_info, DIB_RGB_COLORS, (void**)&rc->framebuffer.color, 0, 0);
	if (rc->bitmap)
		SelectObject(rc->bitmap_dc, rc->bitmap);

	rc->framebuffer.width = w;
	rc->framebuffer.height = h;
	if (rc->framebuffer.depth)
	{
		delete[] rc->framebuffer.depth;
		rc->framebuffer.depth = new uint16_t[w * h];
	}
	if (rc->framebuffer.stencil)
	{
		delete[] rc->framebuffer.stencil;
		rc->framebuffer.stencil = new uint8_t[w * h];
	}
}

EXPORT BOOL WINAPI wglSwapBuffers(HDC device_context)
{
	if (current_context && current_context->bitmap_dc)
	{
		BitBlt(device_context, 0, 0, current_context->framebuffer.width, current_context->framebuffer.height, current_context->bitmap_dc, 0, 0, SRCCOPY);

		HWND win = WindowFromDC(device_context);
		if (win)
		{
			RECT rect{ 0 };
			GetClientRect(win, &rect);
			int w = rect.right - rect.left;
			int h = rect.bottom - rect.top;

			wgl_context* rc = current_context;
			if (w != rc->framebuffer.width || h != rc->framebuffer.height)
			{
				printf("wglSwapBuffers: need resize %dx%d %dx%d\n", rc->framebuffer.width, rc->framebuffer.height, w, h);
				resize_context(rc, w, h);
			}
		}
	}
	return 1;
}

}
