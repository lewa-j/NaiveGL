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

struct gl_frag_data
{
	glm::vec4 color;
	glm::vec4 tex_coord;
	float z;
};

void gl_emit_fragment(gl_state *gs, int x, int y, gl_frag_data &data)
{
	gl_framebuffer &fb = *gs->framebuffer;
	if (x < 0 || x >= fb.width || y < 0 || y >= fb.height)
		return;

	if (gs->scissor_test)
	{
		const glm::ivec4 &s = gs->scissor_rect;
		if (x < s.x || x >= s.x + s.z || y < s.y || y >= s.y + s.w)
			return;
	}

	int i = (fb.width * y + x) * 4;
	fb.color[i]     = uint8_t(data.color.b * 0xFF);
	fb.color[i + 1] = uint8_t(data.color.g * 0xFF);
	fb.color[i + 2] = uint8_t(data.color.r * 0xFF);
	fb.color[i + 3] = uint8_t(data.color.a * 0xFF);
}

void gl_emit_point(gl_processed_vertex &vertex)
{
	gl_state *gs = gl_current_state();
	if (!gs) return;
	if (!gs->clip_point(vertex.position, vertex.clip))
		return;

	glm::vec3 device_c = glm::vec3(vertex.clip) / vertex.clip.w;
	glm::vec3 win_c = gs->get_window_coords(device_c);

	gl_frag_data data;
	data.color = vertex.color;
	data.tex_coord = vertex.tex_coord;
	data.z = win_c.z;

	if (!gs->point_smooth)
	{
		int w = (int)round(gs->point_size);//TODO clamp to max
		w = w < 1 ? 1 : w;
		glm::ivec2 ic(floor(win_c + (w & 1 ? 0 : 0.5f)));//add half pixel when size is even. then truncate to int
		if (w == 1)
			gl_emit_fragment(gs, ic.x, ic.y, data);
		else
		{
			ic -= (w >> 1);//floor(w * 0.5)
			for (int ix = 0; ix < w; ix++)
				for (int iy = 0; iy < w; iy++)
					gl_emit_fragment(gs, ic.x + ix, ic.y + iy, data);
		}
	}
	else
	{
		float w = gs->point_size;
		int wi = int(ceil(w) + 1);
		glm::ivec2 ic(floor(win_c));
		ic -= (wi >> 1);
		w *= 0.5f;
		for (int ix = 0; ix < wi; ix++)
			for (int iy = 0; iy < wi; iy++)
			{
				glm::vec2 p{ ic.x + ix + 0.5f, ic.y + iy + 0.5f };
				float coverage = w - glm::distance(p, glm::vec2(win_c)) + 0.5f;
				if (coverage <= 0.f)
					continue;
				data.color.a = vertex.color.a * glm::min(coverage, 1.0f);
				gl_emit_fragment(gs, ic.x + ix, ic.y + iy, data);
			}
	}
}

void gl_emit_line(gl_processed_vertex &v0, gl_processed_vertex &v1) {}
void gl_emit_triangle(gl_processed_vertex &v0, gl_processed_vertex &v1, gl_processed_vertex &v2) {}
void gl_emit_quad(gl_processed_vertex &v0, gl_processed_vertex &v1, gl_processed_vertex &v2, gl_processed_vertex &v3) {}
