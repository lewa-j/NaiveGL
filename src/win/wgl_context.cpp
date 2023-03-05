#include "pch.h"

#define WIN32_LEAN_AND_MEAN // Exclude rarely-used stuff from Windows headers
#include <windows.h>

#define EXPORT __declspec(dllexport)

extern "C" {

EXPORT HGLRC APIENTRY wglCreateContext(HDC device_context)
{
	return 0;
}

EXPORT BOOL APIENTRY wglDeleteContext(HGLRC rendering_context)
{
	return 1;
}

EXPORT BOOL APIENTRY wglMakeCurrent(HDC device_context, HGLRC rendering_context)
{
	return 1;
}

EXPORT PROC APIENTRY wglGetProcAddress(LPCSTR func_name)
{
	return nullptr;
}

EXPORT HDC APIENTRY wglGetCurrentDC(void)
{
	return 0;
}

EXPORT HGLRC APIENTRY wglGetCurrentContext(void)
{
	return 0;
}

}