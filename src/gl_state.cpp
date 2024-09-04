#include "pch.h"
#include "gl_state.h"
#include "gl_exports.h"
#include <string.h>

void gl_state::init(int window_w, int window_h, bool doublebuffer)
{
	error_bits = 0;

	begin_primitive_mode = -1;
	begin_vertex_count = 0;
	edge_flag = true;
	current_tex_coord = glm::vec4(0, 0, 0, 1);
	current_normal = glm::vec3(0, 0, 1);
	current_color = glm::vec4(1, 1, 1, 1);

	set_viewport(0, 0, window_w, window_h);
	viewport.dnear = 0;
	viewport.dfar = 1;

	matrix_mode = GL_MODELVIEW;
	modelview_sp = 0;
	projection_sp = 0;
	texture_mtx_sp = 0;
	modelview_stack[0] = glm::mat4(1);
	projection_stack[0] = glm::mat4(1);
	texture_mtx_stack[0] = glm::mat4(1);

	normalize = false;

	for (int i = 0; i < 4; i++)
	{
		texgen[i].enabled = false;
		texgen[i].mode = GL_EYE_LINEAR;
		texgen[i].eye_plane = glm::vec4(0, 0, 0, 0);
		texgen[i].object_plane = glm::vec4(0, 0, 0, 0);
	}
	texgen[0].eye_plane = glm::vec4(1, 0, 0, 0);
	texgen[1].eye_plane = glm::vec4(0, 1, 0, 0);
	texgen[0].object_plane = glm::vec4(1, 0, 0, 0);
	texgen[1].object_plane = glm::vec4(0, 1, 0, 0);

	enabled_clipplanes = 0;
	for (int i = 0; i < gl_max_user_clip_planes; i++)
		clipplanes[i] = glm::vec4(0);

	raster_pos.coords = glm::vec4{ 0,0,0,1 };
	raster_pos.distance = 0;
	raster_pos.valid = true;
	raster_pos.color = glm::vec4{ 0,0,0,1 };
	raster_pos.tex_coord = glm::vec4{ 1,1,1,1 };

	lighting_enabled = false;
	front_face_ccw = true;
	for (int i = 0; i < 2; i++)
	{
		materials[i].ambient = glm::vec4(0.2f, 0.2f, 0.2f, 1);
		materials[i].diffuse = glm::vec4(0.8f, 0.8f, 0.8f, 1);
		materials[i].specular = glm::vec4(0, 0, 0, 1);
		materials[i].emission = glm::vec4(0, 0, 0, 1);
		materials[i].shininess = 0.0f;
	}
	enabled_lights = 0;
	for (int i = 0; i < gl_max_lights; i++)
	{
		lights[i].ambient = glm::vec4(0, 0, 0, 1);
		lights[i].diffuse = glm::vec4(0, 0, 0, 1);
		lights[i].specular = glm::vec4(0, 0, 0, 1);
		lights[i].position = glm::vec4(0, 0, 1, 0);
		lights[i].spot_direction = glm::vec3(0, 0, -1);
		lights[i].spot_exponent = 0.0f;
		lights[i].spot_cutoff = 180.0f;
		lights[i].attenuation[0] = 1.0f;//const
		lights[i].attenuation[1] = 0.0f;//linear
		lights[i].attenuation[2] = 0.0f;//quad
	}
	lights[0].diffuse = glm::vec4(1, 1, 1, 1);
	lights[0].specular = glm::vec4(1, 1, 1, 1);

	light_model_ambient = glm::vec4(0.2f, 0.2f, 0.2f, 1);
	light_model_local_viewer = false;
	light_model_two_side = false;

	color_material = false;
	color_material_face = GL_FRONT_AND_BACK;
	color_material_mode = GL_AMBIENT_AND_DIFFUSE;

	shade_model_flat = false;

	point_size = 1.0f;
	point_smooth = false;
	line_width = 1.0f;
	line_smooth = false;
	line_stipple = false;
	line_stipple_factor = 1;
	line_stipple_pattern = 0xFFFF;
	line_stipple_counter = 0;
	polygon_smooth = false;
	cull_face = false;
	cull_face_mode = GL_BACK;
	polygon_stipple = false;
	memset(polygon_stipple_mask, 0xFFFFFFFF, sizeof(polygon_stipple_mask));
	polygon_mode[0] = GL_FILL;
	polygon_mode[1] = GL_FILL;

	pixel_unpack = {};
	pixel_pack = {};
	map_color = false;
	map_stencil = false;
	index_shift = 0;
	index_offset = 0;
	color_scale = glm::vec4{ 1,1,1,1 };
	color_bias = glm::vec4{ 0,0,0,0 };
	depth_scale = 1;
	depth_bias = 0;
	for (int i = 0; i < 8; i++)
	{
		pixel_map_color_table[i].size = 1;
		memset(pixel_map_color_table[i].data, 0, sizeof(pixel_map_color_table[i].data));
	}
	for (int i = 0; i < 2; i++)
	{
		pixel_map_index_table[i].size = 1;
		memset(pixel_map_index_table[i].data, 0, sizeof(pixel_map_index_table[i].data));
	}
	pixel_zoom = glm::vec2{ 1, 1 };

	texture_1d = {};
	texture_2d = {};

	texture_env_function = GL_MODULATE;
	texture_env_color = glm::vec4{ 0,0,0,0 };
	texture_2d_enabled = false;
	texture_1d_enabled = false;

	fog_enabled = false;
	fog_mode = GL_EXP;
	fog_density = 1;
	fog_start = 0;
	fog_end = 1;
	fog_color = glm::vec4{ 0,0,0,0 };

	scissor_test = false;
	scissor_rect = glm::ivec4{ 0, 0, window_w, window_h };
	alpha_test = false;
	alpha_test_func = GL_ALWAYS;
	alpha_test_ref = 0;
	stencil_test = false;
	stencil_func = GL_ALWAYS;
	stencil_test_ref = 0;
	stencil_test_mask = 0xFFFFFFFF;
	stencil_op_sfail = GL_KEEP;
	stencil_op_dpfail = GL_KEEP;
	stencil_op_dppass = GL_KEEP;
	depth_test = false;
	depth_func = GL_LESS;
	blend = false;
	blend_func_src = GL_ONE;
	blend_func_dst = GL_ZERO;
	dither = true;
	logic_op = false;
	logic_op_mode = GL_COPY;

	draw_buffer = doublebuffer ? GL_BACK : GL_FRONT;
	read_buffer = doublebuffer ? GL_BACK : GL_FRONT;
	color_mask = glm::bvec4{ true,true,true,true };
	depth_mask = true;
	stencil_writemask = 0xFFFFFFFF;
	clear_color = glm::vec4{ 0, 0, 0, 0 };
	clear_depth = 1;
	clear_stencil = 0;
	clear_accum = glm::vec4{ 0, 0, 0, 0 };

	enabled_eval_maps = 0;
	for (int i = 0; i < 9; i++)
	{
		eval_maps_1d[i] = mapSpec1D{};
		eval_maps_2d[i] = mapSpec2D{};
	}
	eval_maps_1d[0].control_points = { 1,1,1,1 };
	eval_maps_1d[2].control_points = { 0,0,1 };
	eval_maps_1d[3].control_points = { 0 };
	eval_maps_1d[4].control_points = { 0,0};
	eval_maps_1d[5].control_points = { 0,0,0 };
	eval_maps_1d[6].control_points = { 0,0,0,1 };
	eval_maps_1d[7].control_points = { 0,0,0 };
	eval_maps_1d[8].control_points = { 0,0,0,1 };

	eval_maps_2d[0].control_points = { 1,1,1,1 };
	eval_maps_2d[2].control_points = { 0,0,1 };
	eval_maps_2d[3].control_points = { 0 };
	eval_maps_2d[4].control_points = { 0,0 };
	eval_maps_2d[5].control_points = { 0,0,0 };
	eval_maps_2d[6].control_points = { 0,0,0,1 };
	eval_maps_2d[7].control_points = { 0,0,0 };
	eval_maps_2d[8].control_points = { 0,0,0,1 };
	eval_auto_normal = false;
	eval_1d_grid_segments = 1;
	eval_1d_grid_domain[0] = 0;
	eval_1d_grid_domain[1] = 1;
	eval_2d_grid_segments[0] = 1;
	eval_2d_grid_segments[1] = 1;
	eval_2d_grid_domain_u[0] = 0;
	eval_2d_grid_domain_u[1] = 1;
	eval_2d_grid_domain_v[0] = 0;
	eval_2d_grid_domain_v[1] = 1;

	render_mode = GL_RENDER;
	select_name_sp = 0;
	select_min_depth = UINT_MAX;
	select_max_depth = 0;
	bool select_hit = false;
	selection_array = nullptr;
	selection_array_pos = nullptr;
	selection_array_max_size = 0;
	select_overflow = false;
	select_hit_records = 0;

	feedback_array_max_size = 0;
	feedback_type = 0;
	feedback_array = nullptr;
	feedback_overflow = false;
	feedback_array_pos = nullptr;
	feedback_reset_line = true;

	display_list_begun = 0;
	display_list_execute = false;
	display_list_base = 0;
	display_list_indices.clear();

	hints = {};
}

void gl_state::destroy()
{
	display_list_indices.clear();
}

GLint APIENTRY glRenderMode(GLenum mode)
{
	gl_state *gs = gl_current_state();
	if (!gs) return 0;
	if (gs->begin_primitive_mode != -1)
	{
		gl_set_error(GL_INVALID_OPERATION);
		return 0;
	}

	if (mode < GL_RENDER || mode > GL_SELECT)
	{
		gl_set_error_a(GL_INVALID_ENUM, mode);
		return 0;
	}

	GLint ret = 0;

	if (gs->render_mode == GL_SELECT)
	{
		if (gs->select_hit)
			gl_flush_selection_hit_record(*gs);

		ret = gs->select_overflow ? -1 : gs->select_hit_records;
		gs->select_overflow = false;
		gs->select_hit_records = 0;
		gs->select_name_sp = 0;
		gs->selection_array_pos = gs->selection_array;
	}
	else if (gs->render_mode == GL_FEEDBACK)
	{
		ret = gs->feedback_overflow ? -1 : (int)(gs->feedback_array_pos - gs->feedback_array);
		gs->feedback_overflow = false;
		gs->feedback_array_pos = gs->feedback_array;
	}

	if (mode == GL_SELECT && !gs->selection_array)
	{
		gl_set_error(GL_INVALID_OPERATION);
		return ret;
	}

	if (mode == GL_FEEDBACK && !gs->feedback_array)
	{
		gl_set_error(GL_INVALID_OPERATION);
		return ret;
	}

	if (mode == GL_SELECT)
	{
		gs->select_hit = false;
		gs->select_overflow = false;
		gs->selection_array_pos = gs->selection_array;
	}
	else if (mode == GL_FEEDBACK)
	{
		gs->feedback_overflow = false;
		gs->feedback_array_pos = gs->feedback_array;
	}

	gs->render_mode = mode;
	return ret;
}

void APIENTRY glEnable(GLenum cap)
{
	gl_state *gs = gl_current_state();
	if (!gs) return;
	WRITE_DISPLAY_LIST(Enable, {}, {(int)cap});
	VALIDATE_NOT_BEGIN_MODE;

	if (cap == GL_NORMALIZE)
	{
		gs->normalize = true;
	}
	else if (cap >= GL_TEXTURE_GEN_S && cap <= GL_TEXTURE_GEN_Q)
	{
		gs->texgen[cap - GL_TEXTURE_GEN_S].enabled = true;
	}
	else if (cap >= GL_CLIP_PLANE0 && cap < (GL_CLIP_PLANE0 + gl_max_user_clip_planes))
	{
		gs->enabled_clipplanes |= (1 << (cap - GL_CLIP_PLANE0));
	}
	else if (cap == GL_LIGHTING)
	{
		gs->lighting_enabled = true;
	}
	else if (cap >= GL_LIGHT0 && cap < (GL_LIGHT0 + gl_max_lights))
	{
		gs->enabled_lights |= (1 << (cap - GL_LIGHT0));
	}
	else if (cap == GL_COLOR_MATERIAL)
	{
		bool old = gs->color_material;
		gs->color_material = true;
		if (!old)
			gs->update_color_material();
	}
	else if (cap == GL_POINT_SMOOTH)
	{
		gs->point_smooth = true;
	}
	else if (cap == GL_LINE_SMOOTH)
	{
		gs->line_smooth = true;
	}
	else if (cap == GL_LINE_STIPPLE)
	{
		gs->line_stipple = true;
	}
	else if (cap == GL_POLYGON_SMOOTH)
	{
		gs->polygon_smooth = true;
	}
	else if (cap == GL_POLYGON_STIPPLE)
	{
		gs->polygon_stipple = true;
	}
	else if (cap == GL_CULL_FACE)
	{
		gs->cull_face = true;
	}
	else if (cap == GL_TEXTURE_2D)
	{
		gs->texture_2d_enabled = true;
	}
	else if (cap == GL_TEXTURE_1D)
	{
		gs->texture_1d_enabled = true;
	}
	else if (cap == GL_FOG)
	{
		gs->fog_enabled = true;
	}
	else if (cap == GL_SCISSOR_TEST)
	{
		gs->scissor_test = true;
	}
	else if (cap == GL_ALPHA_TEST)
	{
		gs->alpha_test = true;
	}
	else if (cap == GL_STENCIL_TEST)
	{
		gs->stencil_test = true;
	}
	else if (cap == GL_DEPTH_TEST)
	{
		gs->depth_test = true;
	}
	else if (cap == GL_BLEND)
	{
		gs->blend = true;
	}
	else if (cap == GL_DITHER)
	{
		gs->dither = true;
	}
	else if (cap == GL_LOGIC_OP)
	{
		gs->logic_op = true;
	}
	else if (cap >= GL_MAP1_COLOR_4 && cap <= GL_MAP1_VERTEX_4)
	{
		gs->enabled_eval_maps |= (1 << (cap - GL_MAP1_COLOR_4));
	}
	else if (cap >= GL_MAP2_COLOR_4 && cap <= GL_MAP2_VERTEX_4)
	{
		gs->enabled_eval_maps |= (1 << (cap - GL_MAP2_COLOR_4 + 9));
	}
	else if (cap == GL_AUTO_NORMAL)
	{
		gs->eval_auto_normal = true;
	}
	else
	{
		gl_set_error_a(GL_INVALID_ENUM, cap);
		return;
	}
}

void APIENTRY glDisable(GLenum cap)
{
	gl_state *gs = gl_current_state();
	if (!gs) return;
	WRITE_DISPLAY_LIST(Disable, {}, { (int)cap });
	VALIDATE_NOT_BEGIN_MODE;

	if (cap == GL_NORMALIZE)
	{
		gs->normalize = false;
	}
	else if (cap >= GL_TEXTURE_GEN_S && cap <= GL_TEXTURE_GEN_Q)
	{
		gs->texgen[cap - GL_TEXTURE_GEN_S].enabled = false;
	}
	else if (cap >= GL_CLIP_PLANE0 && cap < (GL_CLIP_PLANE0 + gl_max_user_clip_planes))
	{
		gs->enabled_clipplanes &= ~(1 << (cap - GL_CLIP_PLANE0));
	}
	else if (cap == GL_LIGHTING)
	{
		gs->lighting_enabled = false;
	}
	else if (cap >= GL_LIGHT0 && cap < (GL_LIGHT0 + gl_max_lights))
	{
		gs->enabled_lights &= ~(1 << (cap - GL_LIGHT0));
	}
	else if (cap == GL_COLOR_MATERIAL)
	{
		gs->color_material = false;
	}
	else if (cap == GL_POINT_SMOOTH)
	{
		gs->point_smooth = false;
	}
	else if (cap == GL_LINE_SMOOTH)
	{
		gs->line_smooth = false;
	}
	else if (cap == GL_LINE_STIPPLE)
	{
		gs->line_stipple = false;
	}
	else if (cap == GL_POLYGON_SMOOTH)
	{
		gs->polygon_smooth = false;
	}
	else if (cap == GL_POLYGON_STIPPLE)
	{
		gs->polygon_stipple = false;
	}
	else if (cap == GL_CULL_FACE)
	{
		gs->cull_face = false;
	}
	else if (cap == GL_TEXTURE_2D)
	{
		gs->texture_2d_enabled = false;
	}
	else if (cap == GL_TEXTURE_1D)
	{
		gs->texture_1d_enabled = false;
	}
	else if (cap == GL_FOG)
	{
		gs->fog_enabled = false;
	}
	else if (cap == GL_SCISSOR_TEST)
	{
		gs->scissor_test = false;
	}
	else if (cap == GL_ALPHA_TEST)
	{
		gs->alpha_test = false;
	}
	else if (cap == GL_STENCIL_TEST)
	{
		gs->stencil_test = false;
	}
	else if (cap == GL_DEPTH_TEST)
	{
		gs->depth_test = false;
	}
	else if (cap == GL_BLEND)
	{
		gs->blend = false;
	}
	else if (cap == GL_DITHER)
	{
		gs->dither = false;
	}
	else if (cap == GL_LOGIC_OP)
	{
		gs->logic_op = false;
	}
	else if (cap >= GL_MAP1_COLOR_4 && cap <= GL_MAP1_VERTEX_4)
	{
		gs->enabled_eval_maps &= ~(1 << (cap - GL_MAP1_COLOR_4));
	}
	else if (cap >= GL_MAP2_COLOR_4 && cap <= GL_MAP2_VERTEX_4)
	{
		gs->enabled_eval_maps &= ~(1 << (cap - GL_MAP2_COLOR_4 + 9));
	}
	else if (cap == GL_AUTO_NORMAL)
	{
		gs->eval_auto_normal = false;
	}
	else
	{
		gl_set_error_a(GL_INVALID_ENUM, cap);
		return;
	}
}

GLboolean APIENTRY glIsEnabled(GLenum cap)
{
	return false;
}

void APIENTRY glHint(GLenum target, GLenum mode)
{
	gl_state *gs = gl_current_state();
	if (!gs) return;
	WRITE_DISPLAY_LIST(Hint, {}, { (int)target, (int)mode });
	VALIDATE_NOT_BEGIN_MODE;
	if (target < GL_PERSPECTIVE_CORRECTION_HINT || target > GL_FOG_HINT)
	{
		gl_set_error_a(GL_INVALID_ENUM, target);
		return;
	}
	if (mode < GL_DONT_CARE || mode > GL_NICEST)
	{
		gl_set_error_a(GL_INVALID_ENUM, mode);
		return;
	}

	if (target == GL_PERSPECTIVE_CORRECTION_HINT)
		gs->hints.perspective_correction = mode;
	else if (target == GL_POINT_SMOOTH_HINT)
		gs->hints.point_smooth = mode;
	else if (target == GL_LINE_SMOOTH_HINT)
		gs->hints.line_smooth = mode;
	else if (target == GL_POLYGON_SMOOTH_HINT)
		gs->hints.polygon_smooth = mode;
	else if (target == GL_FOG_HINT)
		gs->hints.fog = mode;
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
