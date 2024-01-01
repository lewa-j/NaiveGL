#pragma once
#include <stdint.h>

extern "C" {
void naglInit(int w, int h, uint8_t *color, int depth, int stencil, int samples, bool doublebuffer);
void naglSetBuffer(int w, int h, uint8_t *color);
void naglShutdown();
void *naglGetProcAddress(const char *func_name);
void naglSwapBuffers();
}

