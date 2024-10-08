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
constexpr float gl_point_size_granularity = 0.1f;
constexpr float gl_line_width_range[2]{ 1, 1 };
constexpr float gl_line_width_granularity = 1;
constexpr int gl_max_pixel_map_table = 32;
constexpr int gl_max_texture_size = 64;
constexpr int gl_max_tex_level = 6;// log2(gl_max_texture_size)
constexpr int gl_max_aux_buffers = 0;
constexpr int gl_max_eval_order = 8;
constexpr int gl_max_name_stack_depth = 64;
constexpr int gl_max_list_nesting = 64;
constexpr int gl_max_attrib_stack_depth = 16;
constexpr int gl_subpixel_bits = 8;

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
	bool is_complete = false;//cached
	int max_lod = 0;

	struct params_t
	{
		glm::vec4 border_color{ 0,0,0,0 };
		int min_filter = GL_NEAREST_MIPMAP_LINEAR;
		int mag_filter = GL_LINEAR;
		int wrap_s = GL_REPEAT;
		int wrap_t = GL_REPEAT;
	} params;
};

struct gl_display_list_call
{
	enum eType
	{
		tBegin,//1i
		tEnd,
		tVertex,//4f
		tEdgeFlag,//1i
		tTexCoord,//4f
		tNormal,//3f
		tColor,//4f
		tRect,//4f
		tDepthRange,//2f
		tViewport,//4i
		tMatrixMode,//1i
		tLoadMatrix,//16f big
		tMultMatrix,//16f big
		tLoadIdentity,
		tRotate,//4f
		tTranslate,//3f
		tScale,//3f
		tFrustum,//6f
		tOrtho,//6f
		tPushMatrix,
		tPopMatrix,
		tEnable,//1i
		tDisable,//1i
		tTexGeni,//2i+1i
		tTexGenfv,//2i+1f/4f
		tClipPlane,//4d		big
		tRasterPos,//4f
		tFrontFace,//1i
		tColorMaterial,//2i
		tShadeModel,//1i
		tMaterial,//2i+1f
		tMaterialv,//2i+1f/4f/4i
		tLightf,//2i+1f
		tLightfv,//2i+1f/3f/4f
		tLightiv,//2i+1i/3i/4i
		tLightModeli,//1i+1i
		tLightModelfv,//1i+1f/4f
		tLightModeliv,//1i+1i/4i
		tPointSize,//1f
		tLineWidth,//1f
		tLineStipple,//2i
		tCullFace,//1i
		tPolygonStipple,//128b	big
		tPolygonMode,//2i
		tPixelTransfer,//1i,1f
		tPixelMap,//2i+n*ui/us/f big i[2] type i[3] size
		tPixelZoom,//2f
		tDrawPixels,//4i+n	big i[4] size
		tBitmap,//2i+4f+n	big i[2] size
		tTexImage2D,//1f+7i+n	big i[7] size
		tTexImage1D,//7i+n	big i[7] size
		tTexParameter,//2i+1i
		tTexParameteriv,//2i+1i/4i
		tTexParameterfv,//2i+1f/4f
		tTexEnv,//2i+1i
		tTexEnviv,//2i+1i/4i
		tTexEnvfv,//2i+1f/4f
		tFog,//1i+1f
		tFogiv,//1i+1i/4i
		tFogfv,//1i+1f/4f
		tScissor,//4i
		tAlphaFunc,//1i,1f
		tStencilFunc,//3i
		tStencilOp,//3i
		tDepthFunc,//1i
		tBlendFunc,//2i
		tLogicOp,//1i
		tDrawBuffer,//1i
		tColorMask,//4i
		tIndexMask,//1i
		tDepthMask,//1i
		tStencilMask,//1i
		tClear,//1i
		tClearColor,//4f
		tClearIndex,//1f
		tClearDepth,//1f
		tClearStencil,//1i
		tClearAccum,//4f
		tAccum,//1i,1f
		tReadBuffer,//1i
		tCopyPixels,//5i
		tMap1,//3i,2f+n big i[3] size
		tMap2,//5i,4f+n big i[5] size
		tEvalCoord1,//1f
		tEvalCoord2,//2f
		tMapGrid1,//1i,2f
		tMapGrid2,//2i,4f
		tEvalMesh1,//3i
		tEvalMesh2,//5i
		tEvalPoint1,//1i
		tEvalPoint2,//2i
		tInitNames,
		tPopName,
		tPushName,//1i
		tLoadName,//1i
		tPassThrough,//1f
		tCallList,//1i
		tCallLists,//2i+n big
		tListBase,//1i
		tHint,//2i
	};

	eType type;
	float argsf[8];
	int argsi[8];
};

struct gl_state
{
	gl_framebuffer *framebuffer;

	int begin_primitive_mode = -1;
	gl_full_vertex last_vertices[3];//for lines, triangles and quads
	int begin_vertex_count = 0;
	int line_stipple_counter = 0;

	bool last_side = false;

	struct current_t
	{
		glm::vec4 color{ 1,1,1,1 };
		//index 1
		glm::vec4 tex_coord{ 0,0,0,1 };
		glm::vec3 normal{ 0,0,1 };
		struct raster_t
		{
			glm::vec4 position{ 0,0,0,1 }; //window xyz, clip w
			float distance = 0;
			glm::vec4 color{ 1,1,1,1 };
			//index 1
			glm::vec4 tex_coord{ 0,0,0,1 };
			bool valid = true;
		} raster;
		bool edge_flag = true;
	} current;

	struct viewport_t
	{
		int width;
		int height;
		int center_x;
		int center_y;
		float dnear = 0;
		float dfar = 1;
	} viewport;

	glm::mat4 modelview_stack[gl_max_viewmodel_mtx];
	int modelview_stack_depth = 1;
	glm::mat4 projection_stack[gl_max_projection_mtx];
	int projection_stack_depth = 1;
	glm::mat4 texture_mtx_stack[gl_max_texture_mtx];
	int texture_mtx_stack_depth = 1;

	struct transform_t
	{
		GLenum matrix_mode = GL_MODELVIEW;
		bool normalize = false;
		glm::vec4 clip_planes[gl_max_user_clip_planes];
		uint32_t enabled_clip_planes = 0;
	} transform;

	struct fog_t
	{
		glm::vec4 color{ 0,0,0,0 };
		//index 0
		float density = 1;
		float start = 0;
		float end = 1;
		int mode = GL_EXP;
		bool enabled = false;
	} fog;

	struct lighting_t
	{
		bool shade_model_flat = false;//GL_SMOOTH
		bool enabled = false;
		bool color_material = false;
		int color_material_param = GL_AMBIENT_AND_DIFFUSE;
		int color_material_face = GL_FRONT_AND_BACK;
		struct material {
			glm::vec4 ambient{ 0.2,0.2,0.2,1 };
			glm::vec4 diffuse{ 0.8,0.8,0.8,1 };
			glm::vec4 specular{ 0,0,0,1 };
			glm::vec4 emission{ 0,0,0,1 };
			float shininess = 0.0f;
		} materials[2]; // front and back
		glm::vec4 light_model_ambient{ 0.2,0.2,0.2,1 };
		bool light_model_local_viewer = false;
		bool light_model_two_side = false;
		struct light {
			glm::vec4 ambient{ 0,0,0,1 };
			glm::vec4 diffuse;
			glm::vec4 specular;
			glm::vec4 position{ 0,0,1,0 };
			float attenuation[3]{ 1,0,0 };//const, linear, quad
			glm::vec3 spot_direction{ 0,0,-1 };
			float spot_exponent = 0.0f;
			float spot_cutoff = 180.0f;
		} lights[gl_max_lights];
		uint32_t enabled_lights = 0;
		//color_indexes 0,1,1
	} lighting;

	struct point_t
	{
		float size = 1.0f;
		bool smooth = false;
	} point;

	struct line_t
	{
		float width = 1.0f;
		bool smooth = false;
		uint16_t stipple_pattern = 0xFFFF;
		int stipple_repeat = 1;
		bool stipple = false;
	} line;

	struct polygon_t
	{
		bool cull_face = false;
		GLenum cull_face_mode = GL_BACK;
		bool front_face_ccw = true;
		bool smooth = false;
		GLenum mode[2]{ GL_FILL,GL_FILL };//front and back
		bool stipple = false;
	} polygon;
	uint8_t polygon_stipple_mask[128];

	bool texture_1d_enabled = false;
	bool texture_2d_enabled = false;
	gl_texture texture_1d;
	gl_texture texture_2d;

	struct texture_env_t
	{
		int mode = GL_MODULATE;
		glm::vec4 color{ 0,0,0,0 };
	} texture_env;

	struct texture_gen_t
	{
		bool enabled = false;
		glm::vec4 eye_plane = glm::vec4(0, 0, 0, 0);
		glm::vec4 object_plane = glm::vec4(0, 0, 0, 0);
		GLenum mode = GL_EYE_LINEAR;
	} texgen[4];

	struct scissor_t
	{
		bool test = false;
		glm::ivec4 box;
	} scissor;

	struct stencil_t
	{
		bool test = false;
		GLenum func = GL_ALWAYS;
		GLuint value_mask = 0xFFFFFFFF;
		GLint ref = 0;
		GLenum fail = GL_KEEP;
		GLenum dpfail = GL_KEEP;
		GLenum dppass = GL_KEEP;
		uint32_t writemask = 0xFFFFFFFF;
		int clear_value = 0;
	} stencil;

	struct color_t
	{
		bool alpha_test = false;
		GLenum alpha_test_func = GL_ALWAYS;
		GLfloat alpha_test_ref = 0;
		bool blend = false;
		GLenum blend_func_src = GL_ONE;
		GLenum blend_func_dst = GL_ZERO;
		bool logic_op = false;
		GLenum logic_op_mode = GL_COPY;
		bool dither = true;
		int draw_buffer = GL_FRONT;
		//index_writemask 0xFFFFFFFF
		glm::bvec4 color_writemask{ true,true,true,true };
		glm::vec4 color_clear_value{ 0,0,0,0 };
		//index_clear_value 0
	} color_buffer;

	struct depth_t
	{
		bool test = false;
		GLenum func = GL_LESS;
		bool writemask = true;
		float clear_value = 1;
	} depth;

	glm::vec4 accum_clear_value{ 0,0,0,0 };

	struct pixelStore
	{
		bool swap_bytes = false;
		bool lsb_first = false;
		int row_length = 0;
		int skip_rows = 0;
		int skip_pixels = 0;
		int alignment = 4;
	} pixel_unpack, pixel_pack; //client state

	struct pixel_t
	{
		bool map_color = false;
		bool map_stencil = false;
		int index_shift = 0;
		int index_offset = 0;
		glm::vec4 color_scale{ 1,1,1,1 };
		glm::vec4 color_bias{ 0,0,0,0 };
		float depth_scale = 1;
		float depth_bias = 0;
		glm::vec2 zoom{ 1, 1 };
		int read_buffer = GL_FRONT;
	} pixel;

	struct pixelMapColor
	{
		int size = 1;
		float data[gl_max_pixel_map_table];
	} pixel_map_color_table[8];//4 index to rgba tables and 4 rgba to rgba tables
	struct pixelMapIndex
	{
		int size = 1;
		GLuint data[gl_max_pixel_map_table];
	} pixel_map_index_table[2];//color, stencil

	struct mapSpec1D
	{
		int order = 1;
		GLfloat domain[2]{ 0, 1 };
		std::vector<float> control_points;
	} eval_maps_1d[9];
	struct mapSpec2D
	{
		int order[2] = { 1,1 };
		GLfloat domain[4]{ 0, 1, 0, 1 };
		std::vector<float> control_points;
	} eval_maps_2d[9];

	struct eval_t
	{
		uint32_t enabled_maps = 0;// bits 0-8 1D maps, bits 9-17 2D maps
		GLfloat map1d_grid_domain[2]{ 0, 1 };
		GLfloat map2d_grid_domain[4]{ 0, 1, 0, 1 };
		GLint map1d_grid_segments = 1;
		GLint map2d_grid_segments[2]{ 1, 1 };
		bool auto_normal = false;
	} eval;

	struct hint_t
	{
		GLenum perspective_correction = GL_DONT_CARE;
		GLenum point_smooth = GL_DONT_CARE;
		GLenum line_smooth = GL_DONT_CARE;
		GLenum polygon_smooth = GL_DONT_CARE;
		GLenum fog = GL_DONT_CARE;
	} hint;

	struct displayList
	{
		bool recorded = false;
		std::vector<gl_display_list_call> calls;
		std::vector<uint8_t> data;
	};
	std::map<int, displayList> display_list_indices;

	GLuint display_list_base = 0;
	int display_list_begun = 0;
	bool display_list_execute = false;
	int display_list_nesting = 0;

	struct gl_state_attribs
	{
		GLbitfield attrib_mask;
		current_t current;
		viewport_t viewport;
		transform_t transform;
		fog_t fog;
		lighting_t lighting;
		point_t point;
		line_t line;
		polygon_t polygon;
		uint8_t polygon_stipple_mask[128];

		bool texture_1d_enabled = false;
		bool texture_2d_enabled = false;
		gl_texture::params_t texture_1d;
		gl_texture::params_t texture_2d;
		texture_env_t texture_env;
		texture_gen_t texgen[4];

		scissor_t scissor;
		stencil_t stencil;
		color_t color_buffer;
		depth_t depth;
		glm::vec4 accum_clear_value;
		pixel_t pixel;
		eval_t eval;
		hint_t hint;
		GLuint display_list_base;
	};

	gl_state_attribs attrib_stack[gl_max_attrib_stack_depth];
	int attrib_sp = 0;

	GLint select_name_stack[gl_max_name_stack_depth];
	int select_name_sp = 0;

	GLint render_mode = GL_RENDER;

	GLuint select_min_depth = UINT_MAX;
	GLuint select_max_depth = 0;
	bool select_hit = false;
	//client state
	GLuint *selection_array = nullptr;
	GLsizei selection_array_max_size = 0;
	bool select_overflow = false;
	GLuint *selection_array_pos = nullptr;
	int select_hit_records = 0;

	//client state
	GLsizei feedback_array_max_size = 0;
	GLenum feedback_type = 0;
	GLfloat *feedback_array = nullptr;
	bool feedback_overflow = false;
	GLfloat *feedback_array_pos = nullptr;
	bool feedback_reset_line = true;

	uint32_t error_bits = 0;

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

void gl_rasterize_line(gl_state& st, const gl_processed_vertex& v0, const gl_processed_vertex& v1);
void gl_rasterize_clipped_line(gl_state& st, gl_processed_vertex v0, gl_processed_vertex v1);
void gl_rasterize_triangle(gl_state& st, gl_processed_vertex& v0, gl_processed_vertex& v1, gl_processed_vertex& v2);
void gl_rasterize_clipped_triangle(gl_state& st, const gl_processed_vertex& v0, const gl_processed_vertex& v1, const gl_processed_vertex& v2);

void gl_dither(glm::vec4& color, int x, int y);

void gl_add_selection_depth(gl_state &st, float z);
void gl_flush_selection_hit_record(gl_state &st);

void gl_feedback_write(gl_state &st, float f);
void gl_feedback_write_vertex(gl_state &st, glm::vec4 pos, glm::vec4 color, glm::vec4 tex_coord);
void gl_feedback_write_vertex(gl_state &st, const gl_processed_vertex &v);

//return -1 if cap is invalid
int gl_isEnabled(gl_state &gs, GLenum cap);

#define VALIDATE_NOT_BEGIN_MODE \
if (gs->begin_primitive_mode != -1)\
{\
	gl_set_error(GL_INVALID_OPERATION);\
	return;\
}

#define VALIDATE_NOT_BEGIN_MODE_RET(RET) \
if (gs->begin_primitive_mode != -1)\
{\
	gl_set_error(GL_INVALID_OPERATION);\
	return RET;\
}

#define VALIDATE_FACE \
if (face != GL_FRONT && face != GL_BACK && face != GL_FRONT_AND_BACK)\
{\
	gl_set_error_a(GL_INVALID_ENUM, face);\
	return;\
}

#define WRITE_DISPLAY_LIST(FUNC_NAME, ...) \
if (gs->display_list_begun) \
{ \
	gs->display_list_indices[0].calls.push_back({ gl_display_list_call::t##FUNC_NAME, __VA_ARGS__ }); \
	if (!gs->display_list_execute) \
		return; \
}

#define WRITE_DISPLAY_LIST_V(FUNC_NAME, ARR, COUNT, CAST, DST, DST_OFF, ...) \
if (gs->display_list_begun) \
{ \
	gl_display_list_call call{ gl_display_list_call::t##FUNC_NAME, __VA_ARGS__}; \
	int n = (COUNT); \
	for (int i = 0; i < n; i++) \
		call.DST[DST_OFF + i] = CAST(ARR[i]); \
	gs->display_list_indices[0].calls.push_back(call); \
	if (!gs->display_list_execute) \
		return; \
}

#define WRITE_DISPLAY_LIST_FV(FUNC_NAME, ARR, COUNT, ...) WRITE_DISPLAY_LIST_V(FUNC_NAME, ARR, COUNT, (float), argsf, 0, __VA_ARGS__)

#define WRITE_DISPLAY_LIST_IV(FUNC_NAME, ARR, COUNT, DST_OFF, ...) WRITE_DISPLAY_LIST_V(FUNC_NAME, ARR, COUNT, to_int, argsi, DST_OFF, __VA_ARGS__)

#define WRITE_DISPLAY_LIST_BULK(FUNC_NAME, SRC_DATA, SRC_SIZE, ...) \
if (gs->display_list_begun) \
{ \
	auto &dl = gs->display_list_indices[0]; \
	dl.calls.push_back({ gl_display_list_call::t##FUNC_NAME, ##__VA_ARGS__ }); \
	size_t old_size = dl.data.size(); \
	size_t temp_size = (SRC_SIZE); \
	dl.data.resize(old_size + temp_size); \
	memcpy(dl.data.data() + old_size, (SRC_DATA), temp_size); \
	if (!gs->display_list_execute) \
		return; \
}
