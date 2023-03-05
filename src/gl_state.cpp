#include "pch.h"
#include "gl_state.h"
#include "gl_exports.h"

void gl_state::init()
{
	error_bits = 0;

	begin_primitive_mode = -1;
	begin_vertex_count = 0;
	vertex = { glm::vec4(0, 0, 0, 1), glm::vec4(0, 0, 0, 1), glm::vec4(1, 1, 1, 1), glm::vec4(0, 0, 0, 1), true };
	edge_flag = true;
	current_tex_coord = glm::vec4(0, 0, 0, 1);
	current_normal = glm::vec3(0, 0, 1);
	current_color = glm::vec4(1, 1, 1, 1);
}

void gl_state::destroy()
{
}

const char *APIENTRY glGetString(GLenum name)
{
	switch (name)
	{
	case GL_VENDOR:
		return "lewa_j";
	case GL_RENDERER:
		return "NaiveGL";
	case GL_VERSION:
		return "1.0.0";
	case GL_EXTENSIONS:
		return "";
	default:
		gl_set_error_a(GL_INVALID_ENUM, name);
		return "";
	}
}

static const char *error_to_str(GLenum e)
{
	switch (e)
	{
	case GL_INVALID_ENUM:
		return "GL_INVALID_ENUM";
	case GL_INVALID_VALUE:
		return "GL_INVALID_VALUE";
	case GL_INVALID_OPERATION:
		return "GL_INVALID_OPERATION";
	case GL_STACK_OVERFLOW:
		return "GL_STACK_OVERFLOW";
	case GL_STACK_UNDERFLOW:
		return "GL_STACK_UNDERFLOW";
	case GL_OUT_OF_MEMORY:
		return "GL_OUT_OF_MEMORY";
	}
	return "?";
}

#if DEBUG_LOG
#define gl_log printf
#else
#define gl_log()
#endif

void gl_set_error_a_(GLenum error, GLenum arg, const char *func)
{
	gl_state *gs = gl_current_state();
	if (!gs) return;
	if (error < GL_INVALID_ENUM || error > GL_OUT_OF_MEMORY)
		return;

	gl_log("%s: error %s 0x%X\n", func, error_to_str(error), arg);

	gs->error_bits |= (1 << (error - GL_INVALID_ENUM));
}

void gl_set_error_(GLenum error, const char *func)
{
	gl_state *gs = gl_current_state();
	if (!gs) return;
	if (error < GL_INVALID_ENUM || error > GL_OUT_OF_MEMORY)
		return;

	gl_log("%s: error %s\n", func, error_to_str(error));

	gs->error_bits |= (1 << (error - GL_INVALID_ENUM));
}

GLenum APIENTRY glGetError()
{
	gl_state *gs = gl_current_state();
	if (!gs) return 0;

	if (gs->error_bits == 0)
		return GL_NO_ERROR;

	for (int i = 0; i < 6; i++)
	{
		if (gs->error_bits & (1 << i))
		{
			gs->error_bits &= ~(1 << i);
			return GL_INVALID_ENUM + i;
		}
	}

	return GL_NO_ERROR;
}
