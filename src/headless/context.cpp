
#include "nagl.h"
#include "gl_state.h"
#include "gl_exports.h"
#include <string.h>

static gl_state state;
static gl_framebuffer framebuffer;

gl_state *gl_current_state()
{
	return &state;
}

void naglInit(int w, int h, uint8_t *color, int depth, int stencil, int samples, bool doublebuffer)
{
	framebuffer.width = w;
	framebuffer.height = h;
	framebuffer.doublebuffer = doublebuffer;
	
	framebuffer.color = color;
	framebuffer.depth = nullptr;
	framebuffer.stencil = nullptr;
	if (depth)
		framebuffer.depth = new uint16_t[w * h];
	if (stencil)
		framebuffer.stencil = new uint8_t[w * h];

	state.framebuffer = &framebuffer;
	state.init(w, h, doublebuffer);
}

void naglSetBuffer(int w, int h, uint8_t *color)
{
	if(framebuffer.width * framebuffer.height != w*h)
	{
		if(framebuffer.depth)
		{
			delete[] framebuffer.depth;
			framebuffer.depth = new uint16_t[w * h];
		}
		if(framebuffer.stencil)
		{
			delete[] framebuffer.stencil;
			framebuffer.stencil = new uint8_t[w * h];
		}
	}
	framebuffer.width = w;
	framebuffer.height = h;
	framebuffer.color = color;
}

void naglShutdown()
{
	state.destroy();
	delete[] framebuffer.depth;
	delete[] framebuffer.stencil;
	framebuffer.color = nullptr;
}

void *naglGetProcAddress(const char *func_name)
{
#define X(name) \
	{#name, (void*)name},

	static struct { const char *n; void* pfn; } funcs[]{
#include "gl_funcs_x.h"
		{nullptr, nullptr}
	};
#undef X

	void *r = nullptr;
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

void naglSwapBuffers()
{
	
}

