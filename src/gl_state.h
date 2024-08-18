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
constexpr int gl_max_texture_size = 64;
constexpr int gl_max_tex_level = 6;// log2(gl_max_texture_size)

constexpr int gl_max_aux_buffers = 0;

constexpr int gl_max_eval_order = 8;

struct gl_framebuffer
{
	int width = 0;
	int height = 0;
	bool doublebuffer = false;
	uint8_t *color = nullptr;
	uint16_t *depth = nullptr;
	uint8_t *stencil = nullptr;
	glm::vec4 *accum = nullptr;
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

struct gl_texture_array
{
	uint8_t* data = nullptr;
	int width = 0;
	int height = 0;
	int border = 0;
	int components = 1;
};

struct gl_texture
{
	gl_texture_array arrays[gl_max_tex_level + 1];
	int num_arrays = gl_max_tex_level + 1;
	int min_filter = GL_NEAREST_MIPMAP_LINEAR;
	int mag_filter = GL_LINEAR;
	int wrap_s = GL_REPEAT;
	int wrap_t = GL_REPEAT;
	glm::vec4 border_color{ 0,0,0,0 };
	bool is_complete = false;//cached
	int max_lod = 0;
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
	} pixel_unpack, pixel_pack; //client state
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
	} pixel_map_color_table[8];//4 index to rgba tables and 4 rgba to rgba tables
	struct pixelMapIndex
	{
		int size = 1;
		int data[gl_max_pixel_map_table];
	} pixel_map_index_table[2];//color, stencil
	glm::vec2 pixel_zoom{ 1, 1 };

	gl_texture texture_2d;
	gl_texture texture_1d;
	int texture_env_function = GL_MODULATE;
	glm::vec4 texture_env_color{ 0,0,0,0 };
	bool texture_2d_enabled = false;
	bool texture_1d_enabled = false;

	bool fog_enabled = false;
	int fog_mode = GL_EXP;
	float fog_density = 1;
	float fog_start = 0;
	float fog_end = 1;
	glm::vec4 fog_color{ 0,0,0,0 };

	bool scissor_test = false;
	glm::ivec4 scissor_rect;
	bool alpha_test = false;
	GLenum alpha_test_func = GL_ALWAYS;
	GLfloat alpha_test_ref = 0;
	bool stencil_test = false;
	GLenum stencil_func = GL_ALWAYS;
	GLint stencil_test_ref = 0;
	GLuint stencil_test_mask = 0xFFFFFFFF;
	GLenum stencil_op_sfail = GL_KEEP;
	GLenum stencil_op_dpfail = GL_KEEP;
	GLenum stencil_op_dppass = GL_KEEP;
	bool depth_test = false;
	GLenum depth_func = GL_LESS;
	bool blend = false;
	GLenum blend_func_src = GL_ONE;
	GLenum blend_func_dst = GL_ZERO;
	bool dither = true;
	bool logic_op = false;
	GLenum logic_op_mode = GL_COPY;

	int draw_buffer = GL_FRONT;
	int read_buffer = GL_FRONT;
	glm::bvec4 color_mask{ true,true,true,true };
	bool depth_mask = true;
	uint32_t stencil_writemask = 0xFFFFFFFF;
	glm::vec4 clear_color{ 0,0,0,0 };
	float clear_depth = 1;
	int clear_stencil = 0;
	glm::vec4 clear_accum{ 0,0,0,0 };

	uint32_t enabled_eval_maps = 0;// bits 0-8 1D maps, bits 9-17 2D maps
	struct mapSpec1D
	{
		int order_u = 1;
		GLfloat domain_u[2]{ 0, 1 };
		std::vector<float> control_points;
	} eval_maps_1d[9];
	struct mapSpec2D
	{
		int order_u = 1;
		GLfloat domain_u[2]{ 0, 1 };
		int order_v = 1;
		GLfloat domain_v[2]{ 0, 1 };
		std::vector<float> control_points;
	} eval_maps_2d[9];
	bool eval_auto_normal = false;
	GLint eval_1d_grid_segments = 1;
	GLfloat eval_1d_grid_domain[2]{ 0, 1 };
	GLint eval_2d_grid_segments[2]{ 1, 1 };
	GLfloat eval_2d_grid_domain_u[2]{ 0, 1 };
	GLfloat eval_2d_grid_domain_v[2]{ 0, 1 };

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
	glm::vec3 get_eye_normal(const glm::vec3 &norm);
	glm::vec4 get_vertex_color(const glm::vec4& vertex_view, const glm::vec4& color, glm::vec3 normal, bool front_face);
	glm::vec4 get_vertex_texcoord(glm::vec4 tc_in, const glm::vec3 &norm, const glm::vec4 &v_object, const glm::vec4 &v_eye);
	glm::vec3 get_window_coords(glm::vec3 device_coords);
	bool clip_point(const glm::vec4 &v_eye, const glm::vec4 &v_clip);
	void update_color_material();
	void set_material_color(GLenum face, GLenum pname, const glm::vec4& param, bool force = false);

	bool need_tex_lod();
	glm::vec4 sample_tex2d(const gl_texture &tex, const glm::vec4 &tex_coord, float lod);

	glm::vec4 get_fog_color(const glm::vec4& cr, float c);
};

gl_state *gl_current_state();

#if NAGL_DEBUG_LOG
#define gl_log printf
#else
#define gl_log(...)
#endif

#define gl_set_error(e) gl_set_error_(e, __FUNCTION__)
#define gl_set_error_a(e, a) gl_set_error_a_(e, a, __FUNCTION__)
void gl_set_error_(GLenum error, const char *func);
void gl_set_error_a_(GLenum error, GLenum arg, const char *func);

//fragment's associated data
struct gl_frag_data
{
	glm::vec4 color;
	glm::vec4 tex_coord;
	float lod;
	float fog_z;
	float z;
};

void gl_emit_vertex(gl_state *gs, glm::vec4 v_object, glm::vec4 col, glm::vec4 tex, glm::vec3 norm);
void gl_emit_fragment(gl_state& st, int x, int y, gl_frag_data& data);
void gl_emit_point(gl_state &st, const gl_processed_vertex &vertex);
void gl_emit_line(gl_state& st, gl_processed_vertex &v0, gl_processed_vertex &v1);
void gl_emit_triangle(gl_state& st, gl_full_vertex &v0, gl_full_vertex&v1, gl_full_vertex&v2);
void gl_emit_quad(gl_state& st, gl_full_vertex &v0, gl_full_vertex &v1, gl_full_vertex &v2, gl_full_vertex &v3);

void gl_rasterize_line(gl_state& st, const gl_processed_vertex& v0, const gl_processed_vertex& v1);
void gl_rasterize_clipped_line(gl_state& st, gl_processed_vertex v0, gl_processed_vertex v1);
void gl_rasterize_triangle(gl_state& st, gl_processed_vertex& v0, gl_processed_vertex& v1, gl_processed_vertex& v2);
void gl_rasterize_clipped_triangle(gl_state& st, const gl_processed_vertex& v0, const gl_processed_vertex& v1, const gl_processed_vertex& v2);

void gl_dither(glm::vec4& color, int x, int y);

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
