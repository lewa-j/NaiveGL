#pragma once
#include "gl_types.h"

struct gl_state
{
	uint32_t error_bits = 0;

	void init();
	void destroy();
};

gl_state *gl_current_state();

#define gl_set_error(e) gl_set_error_(e, __FUNCTION__)
#define gl_set_error_a(e, a) gl_set_error_a_(e, a, __FUNCTION__)
void gl_set_error_(GLenum error, const char *func);
void gl_set_error_a_(GLenum error, GLenum arg, const char *func);
