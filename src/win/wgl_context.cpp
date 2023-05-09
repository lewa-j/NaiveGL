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

	int w = 640;
	int h = 480;
	int depth = 0;//16
	int stencil = 0;//8
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
	rc->bitmap = CreateDIBSection(device_context, &rc->bitmap_info, DIB_RGB_COLORS, (void **)&rc->framebuffer.color, 0, 0);
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
	rc->state.init(w, h);

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
		X(glMatrixMode)
		X(glLoadMatrixd)
		X(glLoadMatrixf)
		X(glMultMatrixd)
		X(glMultMatrixf)
		X(glLoadIdentity)
		X(glRotated)
		X(glRotatef)
		X(glTranslated)
		X(glTranslatef)
		X(glScaled)
		X(glScalef)
		X(glFrustum)
		X(glOrtho)
		X(glPushMatrix)
		X(glPopMatrix)
		X(glEnable)
		X(glDisable)
		X(glTexGeni)
		X(glTexGenf)
		X(glTexGend)
		X(glTexGeniv)
		X(glTexGenfv)
		X(glTexGendv)
		X(glRasterPos2s)
		X(glRasterPos2i)
		X(glRasterPos2f)
		X(glRasterPos2d)
		X(glRasterPos2sv)
		X(glRasterPos2iv)
		X(glRasterPos2fv)
		X(glRasterPos2dv)
		X(glRasterPos3s)
		X(glRasterPos3i)
		X(glRasterPos3f)
		X(glRasterPos3d)
		X(glRasterPos3sv)
		X(glRasterPos3iv)
		X(glRasterPos3fv)
		X(glRasterPos3dv)
		X(glRasterPos4s)
		X(glRasterPos4i)
		X(glRasterPos4f)
		X(glRasterPos4d)
		X(glRasterPos4sv)
		X(glRasterPos4iv)
		X(glRasterPos4fv)
		X(glRasterPos4dv)
		X(glFrontFace)
		X(glColorMaterial)
		X(glShadeModel)
		X(glMateriali)
		X(glMaterialf)
		X(glMaterialiv)
		X(glMaterialfv)
		X(glLighti)
		X(glLightf)
		X(glLightiv)
		X(glLightfv)
		X(glLightModeli)
		X(glLightModelf)
		X(glLightModeliv)
		X(glLightModelfv)
		X(glPointSize)
		X(glLineWidth)
		X(glLineStipple)
		X(glCullFace)
		X(glPolygonStipple)
		X(glPolygonMode)

		X(glScissor)
		X(glDrawBuffer)
		X(glReadBuffer)
		X(glColorMask)
		X(glIndexMask)
		X(glDepthMask)
		X(glStencilMask)
		X(glClear)
		X(glClearColor)
		X(glClearIndex)
		X(glClearDepth)
		X(glClearStencil)
		X(glClearAccum)
		X(glAccum)

		X(glTexImage2D)
		X(glTexParameterf)
		X(glTexEnvf)
		X(glAlphaFunc)
		X(glDepthFunc)
		X(glBlendFunc)
		X(glReadPixels)
		X(glGetFloatv)
		X(glFinish)
		X(glHint)
		X(glBitmap)
		X(glFlush)
		X(glPopAttrib)
		X(glPushAttrib)
		X(glGetBooleanv)
		X(glPixelStorei)

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

	descriptor->cDepthBits = 0;
	descriptor->cStencilBits = 0;
	descriptor->cAuxBuffers = 0;

	return pixel_format_count;
}

EXPORT BOOL WINAPI wglSwapBuffers(HDC device_context)
{
	if (current_context && current_context->bitmap_dc)
	{
		BitBlt(device_context, 0, 0, current_context->framebuffer.width, current_context->framebuffer.height, current_context->bitmap_dc, 0, 0, SRCCOPY);
	}
	return 1;
}

}
