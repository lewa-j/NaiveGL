#include "pch.h"
#include "gl_state.h"
#include "gl_exports.h"


void APIENTRY glPointSize(GLfloat size)
{
	gl_state *gs = gl_current_state();
	if (!gs) return;
	VALIDATE_NOT_BEGIN_MODE;
	if (size <= 0)
	{
		gl_set_error(GL_INVALID_VALUE);
		return;
	}

	gs->point_size = size;
}

void APIENTRY glLineWidth(GLfloat width)
{
	gl_state *gs = gl_current_state();
	if (!gs) return;
	VALIDATE_NOT_BEGIN_MODE;
	if (width <= 0)
	{
		gl_set_error(GL_INVALID_VALUE);
		return;
	}

	gs->line_width = width;
}

void APIENTRY glLineStipple(GLint factor, GLushort pattern)
{
	gl_state *gs = gl_current_state();
	if (!gs) return;
	VALIDATE_NOT_BEGIN_MODE;

	gs->line_stipple_factor = glm::clamp(factor, 1, 256);
	gs->line_stipple_pattern = pattern;
}

void APIENTRY glCullFace(GLenum face)
{
	gl_state *gs = gl_current_state();
	if (!gs) return;
	VALIDATE_NOT_BEGIN_MODE;
	VALIDATE_FACE;

	gs->cull_face_mode = face;
}

void APIENTRY glPolygonStipple(const GLubyte *mask)
{
	gl_state *gs = gl_current_state();
	if (!gs) return;
	VALIDATE_NOT_BEGIN_MODE;

	memcpy(gs->polygon_stipple_mask, mask, sizeof(gs->polygon_stipple_mask));
}

void APIENTRY glPolygonMode(GLenum face, GLenum mode)
{
	gl_state *gs = gl_current_state();
	if (!gs) return;
	VALIDATE_NOT_BEGIN_MODE;
	VALIDATE_FACE;
	if (mode < GL_POINT && mode > GL_FILL)
	{
		gl_set_error_a(GL_INVALID_ENUM, mode);
		return;
	}

	if (face != GL_BACK)
		gs->polygon_mode[0] = mode;
	if (face != GL_FRONT)
		gs->polygon_mode[1] = mode;
}

void gl_emit_point(gl_processed_vertex &vertex) {}
void gl_emit_line(gl_processed_vertex &v0, gl_processed_vertex &v1) {}
void gl_emit_triangle(gl_processed_vertex &v0, gl_processed_vertex &v1, gl_processed_vertex &v2) {}
void gl_emit_quad(gl_processed_vertex &v0, gl_processed_vertex &v1, gl_processed_vertex &v2, gl_processed_vertex &v3) {}
