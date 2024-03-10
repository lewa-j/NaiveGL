#pragma once
#include "gl_types.h"
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <map>
#include <vector>

#ifdef ANDROID
	#define NAGL_FLIP_VIEWPORT_Y 1
#endif

constexpr int gl_max_viewport_dims[2]{0x4000,0x4000};

constexpr int gl_max_viewmodel_mtx = 32;
constexpr int gl_max_projection_mtx = 2;
constexpr int gl_max_texture_mtx = 2;

//max possible is 32, cause of enabled_* type
constexpr int gl_max_user_clip_planes = 6;
constexpr int gl_max_lights = 8;

constexpr int gl_max_point_size = 2048;
constexpr float gl_point_size_range[2]{ 0.1f, 2048 };

constexpr int gl_max_pixel_map_table = 32;

constexpr int gl_max_aux_buffers = 0;

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

struct gl_full_vertex : gl_processed_vertex
{
	glm::vec4 original_color;
	glm::vec3 normal;//for two sided lighting
};

struct gl_display_list_call
{
	int type;
	float argsf[4];
	int argsi[4];

	enum eType
	{
		tBegin,
		tEnd,
		tVertex,
		tEdgeFlag,
		tTexCoord,
		tNormal,
		tColor,
		tMaterial,
		tShadeModel,
		tMatrixMode,
		tLoadIdentity,
		tRotate,
		tTranslate,
		tScale
	};
};

struct gl_state
{
	gl_framebuffer *framebuffer;

	uint32_t error_bits = 0;
	int begin_primitive_mode = -1;
	int begin_vertex_count = 0;
	gl_full_vertex last_vertices[3];//for lines, triangles and quads
	bool last_side = false;

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
		glm::vec4 color{ 1,1,1,1 };
		glm::vec4 tex_coord{ 0,0,0,1 };
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
	int line_stipple_counter = 0;
	bool polygon_smooth = false;
	bool cull_face = false;
	GLenum cull_face_mode = GL_BACK;
	bool polygon_stipple = false;
	uint8_t polygon_stipple_mask[128];
	GLenum polygon_mode[2]{ GL_FILL,GL_FILL };//front and back

	struct pixelStore
	{
		bool swap_bytes = false;
		bool lsb_first = false;
		int row_length = 0;
		int skip_rows = 0;
		int skip_pixels = 0;
		int alignment = 4;
	} pixel_unpack, pixel_pack;
	bool map_color = false;
	bool map_stencil = false;
	int index_shift = 0;
	int index_offset = 0;
	glm::vec4 color_scale{ 1,1,1,1 };
	glm::vec4 color_bias{ 0,0,0,0 };
	float depth_scale = 1;
	float depth_bias = 0;
	struct pixelMapColor
	{
		int size = 1;
		float data[gl_max_pixel_map_table];
	} pixel_map_color_table[8];//4 index to rgba tabes and 4 rgba to rgba tables
	struct pixelMapIndex
	{
		int size = 1;
		int data[gl_max_pixel_map_table];
	} pixel_map_index_table[2];

	bool scissor_test = false;
	glm::ivec4 scissor_rect;
	bool blend = false;
	GLenum blend_func_src = GL_ONE;
	GLenum blend_func_dst = GL_ZERO;

	int draw_buffer = GL_FRONT;
	int read_buffer = GL_FRONT;
	glm::bvec4 color_mask{ true,true,true,true };
	bool depth_mask = true;
	uint32_t stencil_writemask = 0xFFFFFFFF;
	glm::vec4 clear_color{ 0,0,0,0 };
	float clear_depth = 1;
	int clear_stencil = 0;
	glm::vec4 clear_accum{ 0,0,0,0 };

	int display_list_begun = 0;
	bool display_list_execute = false;
	GLuint display_list_base = 0;
	struct displayList
	{
		bool recorded = false;
		std::vector<gl_display_list_call> calls;
	};
	std::map<int, displayList> display_list_indices;

	void init(int window_w, int window_h, bool doublebuffer);
	void destroy();

	void set_viewport(int x, int y, int w, int h);
	const glm::mat4 &get_modelview();
	glm::mat4 get_inv_modelview();
	const glm::mat4 &get_projection();
	const glm::mat4 &get_mtx_texture();
	glm::vec3 get_eye_normal();
	glm::vec4 get_vertex_color_current(const glm::vec4 &vertex_view, bool front_face);
	glm::vec4 get_vertex_color(const glm::vec4& vertex_view, const glm::vec4& color, glm::vec3 normal, bool front_face);
	glm::vec4 get_vertex_texcoord(const glm::vec4 &v_object, const glm::vec4 &v_eye);
	glm::vec3 get_window_coords(glm::vec3 device_coords);
	bool clip_point(const glm::vec4 &v_eye, const glm::vec4 &v_clip);
	void update_color_material();
	void set_material_color(GLenum face, GLenum pname, const glm::vec4& param, bool force = false);
};

gl_state *gl_current_state();

#define gl_set_error(e) gl_set_error_(e, __FUNCTION__)
#define gl_set_error_a(e, a) gl_set_error_a_(e, a, __FUNCTION__)
void gl_set_error_(GLenum error, const char *func);
void gl_set_error_a_(GLenum error, GLenum arg, const char *func);

void gl_emit_point(gl_state &st, const gl_processed_vertex &vertex);
void gl_emit_line(gl_state& st, gl_processed_vertex &v0, gl_processed_vertex &v1);
void gl_emit_triangle(gl_state& st, gl_full_vertex &v0, gl_full_vertex&v1, gl_full_vertex&v2);
void gl_emit_quad(gl_state& st, gl_processed_vertex &v0, gl_processed_vertex &v1, gl_processed_vertex &v2, gl_processed_vertex &v3);

void rasterize_line(gl_state& st, const gl_processed_vertex& v0, const gl_processed_vertex& v1);
void rasterize_clipped_line(gl_state& st, gl_processed_vertex v0, gl_processed_vertex v1);
void rasterize_triangle(gl_state& st, gl_processed_vertex& v0, gl_processed_vertex& v1, gl_processed_vertex& v2);
void rasterize_clipped_triangle(gl_state& st, const gl_processed_vertex& v0, const gl_processed_vertex& v1, const gl_processed_vertex& v2);

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
