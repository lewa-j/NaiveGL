#pragma once
#include "gl_types.h"
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

constexpr int gl_max_viewmodel_mtx = 32;
constexpr int gl_max_projection_mtx = 2;
constexpr int gl_max_texture_mtx = 2;

//max possible is 32, cause of enabled_* type
constexpr int gl_max_user_clip_planes = 6;
constexpr int gl_max_lights = 8;

struct gl_framebuffer
{
	int width = 0;
	int height = 0;
	bool doublebuffer = false;
	uint8_t *color = nullptr;
	uint16_t *depth = nullptr;
	uint8_t *stencil = nullptr;
};

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

	bool normalize = false;
	struct texGen {
		bool enabled = false;
		GLenum mode = GL_EYE_LINEAR;
		glm::vec4 eye_plane;
		glm::vec4 object_plane;
	} texgen[4];
	glm::vec4 clipplanes[gl_max_user_clip_planes];
	uint32_t enabled_clipplanes = 0;
	struct {
		glm::vec4 coords{ 0,0,0,1 }; //window xyz, clip w
		float distance = 0;
		bool valid = true;
		glm::vec4 color{ 0,0,0,1 };
		glm::vec4 tex_coord{ 1,1,1,1 };
	} raster_pos;
	bool lighting_enabled = false;
	bool front_face_ccw = true;
	struct material {
		glm::vec4 ambient;
		glm::vec4 diffuse;
		glm::vec4 specular;
		glm::vec4 emission;
		float shininess = 0.0f;
	} materials[2]; // front and back
	uint32_t enabled_lights = 0;
	struct light {
		glm::vec4 ambient;
		glm::vec4 diffuse;
		glm::vec4 specular;
		glm::vec4 position;
		glm::vec3 spot_direction;
		float spot_exponent = 0.0f;
		float spot_cutoff = 180.0f;
		float attenuation[3]{ 1,0,0 };//const, linear, quad
	} lights[gl_max_lights];
	glm::vec4 light_model_ambient;
	bool light_model_local_viewer = false;
	bool light_model_two_side = false;
	bool color_material = false;
	int color_material_face = GL_FRONT_AND_BACK;
	int color_material_mode = GL_AMBIENT_AND_DIFFUSE;
	bool shade_model_flat = false;

	float point_size = 1.0f;
	bool point_smooth = false;
	float line_width = 1.0f;
	bool line_smooth = false;
	bool line_stipple = false;
	int line_stipple_factor = 1;
	uint16_t line_stipple_pattern = 0xFFFF;
	bool polygon_smooth = false;
	bool cull_face = false;
	GLenum cull_face_mode = GL_BACK;
	bool polygon_stipple = false;
	uint32_t polygon_stipple_mask[32];
	GLenum polygon_mode[2]{ GL_FILL,GL_FILL };//front and back

	void init(int window_w, int window_h);
	void destroy();

	const glm::mat4 &get_modelview();
	glm::mat4 get_inv_modelview();
	const glm::mat4 &get_projection();
	const glm::mat4 &get_mtx_texture();
	glm::vec3 get_eye_normal();
	glm::vec4 get_vertex_color(const glm::vec4 &vertex_object, const glm::vec4 &vertex_view, bool front_face);
	glm::vec4 get_vertex_texcoord(const glm::vec4 &v_object, const glm::vec4 &v_eye);
	glm::vec3 get_window_coords(glm::vec3 device_coords);
	bool clip_point(const glm::vec4 &v_eye, const glm::vec4 &v_clip);
	void update_color_material();
};

gl_state *gl_current_state();

#define gl_set_error(e) gl_set_error_(e, __FUNCTION__)
#define gl_set_error_a(e, a) gl_set_error_a_(e, a, __FUNCTION__)
void gl_set_error_(GLenum error, const char *func);
void gl_set_error_a_(GLenum error, GLenum arg, const char *func);

void gl_emit_point(gl_processed_vertex &vertex);
void gl_emit_line(gl_processed_vertex &v0, gl_processed_vertex &v1);
void gl_emit_triangle(gl_processed_vertex &v0, gl_processed_vertex &v1, gl_processed_vertex &v2);
void gl_emit_quad(gl_processed_vertex &v0, gl_processed_vertex &v1, gl_processed_vertex &v2, gl_processed_vertex &v3);

#define VALIDATE_NOT_BEGIN_MODE \
if (gs->begin_primitive_mode != -1)\
{\
	gl_set_error(GL_INVALID_OPERATION);\
	return;\
}

#define VALIDATE_FACE \
if (face != GL_FRONT && face != GL_BACK && face != GL_FRONT_AND_BACK)\
{\
	gl_set_error_a(GL_INVALID_ENUM, face);\
	return;\
}
