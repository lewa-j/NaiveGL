#pragma once
#include "gl_types.h"
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

constexpr int gl_max_viewmodel_mtx = 32;
constexpr int gl_max_projection_mtx = 2;
constexpr int gl_max_texture_mtx = 2;

struct gl_processed_vertex
{
	glm::vec4 position;
	glm::vec4 clip;
	glm::vec4 color;
	glm::vec4 tex_coord;
	bool edge;
};

struct gl_state
{
	uint32_t error_bits = 0;
	int begin_primitive_mode = -1;
	int begin_vertex_count = 0;
	gl_processed_vertex vertex;
	gl_processed_vertex last_vertices[3];//for lines, triangles and quads
	bool edge_flag = true;
	glm::vec4 current_tex_coord;
	glm::vec3 current_normal;
	glm::vec4 current_color;

	struct {
		int width;
		int height;
		int center_x;
		int center_y;
		float dfar;
		float dnear;
	} viewport;

	GLenum matrix_mode = GL_MODELVIEW;
	glm::mat4 modelview_stack[gl_max_viewmodel_mtx];
	int modelview_sp = 0;
	glm::mat4 projection_stack[gl_max_projection_mtx];
	int projection_sp = 0;
	glm::mat4 texture_mtx_stack[gl_max_texture_mtx];
	int texture_mtx_sp = 0;

	void init(int window_w, int window_h);
	void destroy();

	glm::vec3 get_window_coords(glm::vec3 device_coords);
};

gl_state *gl_current_state();

#define gl_set_error(e) gl_set_error_(e, __FUNCTION__)
#define gl_set_error_a(e, a) gl_set_error_a_(e, a, __FUNCTION__)
void gl_set_error_(GLenum error, const char *func);
void gl_set_error_a_(GLenum error, GLenum arg, const char *func);

#define VALIDATE_NOT_BEGIN_MODE \
if (gs->begin_primitive_mode != -1)\
{\
	gl_set_error(GL_INVALID_OPERATION);\
	return;\
}
