#pragma once
#include "gl_types.h"
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

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

	void init();
	void destroy();
};

gl_state *gl_current_state();

#define gl_set_error(e) gl_set_error_(e, __FUNCTION__)
#define gl_set_error_a(e, a) gl_set_error_a_(e, a, __FUNCTION__)
void gl_set_error_(GLenum error, const char *func);
void gl_set_error_a_(GLenum error, GLenum arg, const char *func);
