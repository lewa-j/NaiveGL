#include "pch.h"
#include "gl_state.h"
#include "gl_exports.h"
#include <string.h>

void gl_state::init(int window_w, int window_h, bool doublebuffer)
{
	begin_primitive_mode = -1;
	begin_vertex_count = 0;
	line_stipple_counter = 0;

	current = {};

	viewport = {};
	set_viewport(0, 0, window_w, window_h);

	modelview_stack_depth = 1;
	projection_stack_depth = 1;
	texture_mtx_stack_depth = 1;
	modelview_stack[0] = glm::mat4(1);
	projection_stack[0] = glm::mat4(1);
	texture_mtx_stack[0] = glm::mat4(1);

	transform = {};
	for (int i = 0; i < gl_max_user_clip_planes; i++)
		transform.clip_planes[i] = glm::vec4(0);

	fog = {};
	lighting = {};
	lighting.lights[0].diffuse = glm::vec4(1, 1, 1, 1);
	lighting.lights[0].specular = glm::vec4(1, 1, 1, 1);

	point = {};
	line = {};
	polygon = {};
	memset(polygon_stipple_mask, 0xFFFFFFFF, sizeof(polygon_stipple_mask));

	texture_2d_enabled = false;
	texture_1d_enabled = false;
	texture_1d = {};
	texture_2d = {};
	texture_env = {};

	for (int i = 0; i < 4; i++)
		texgen[i] = {};
	texgen[0].eye_plane = glm::vec4(1, 0, 0, 0);
	texgen[0].object_plane = glm::vec4(1, 0, 0, 0);
	texgen[1].eye_plane = glm::vec4(0, 1, 0, 0);
	texgen[1].object_plane = glm::vec4(0, 1, 0, 0);

	scissor = {};
	scissor.box = glm::ivec4{ 0, 0, window_w, window_h };

	stencil = {};
	color_buffer = {};
	color_buffer.draw_buffer = doublebuffer ? GL_BACK : GL_FRONT;
	depth = {};
	accum_clear_value = glm::vec4{ 0, 0, 0, 0 };
	pixel_unpack = {};
	pixel_pack = {};
	pixel = {};
	pixel.read_buffer = doublebuffer ? GL_BACK : GL_FRONT;

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

	eval = {};

	for (int i = 0; i < 9; i++)
	{
		eval_maps_1d[i] = {};
		eval_maps_2d[i] = {};
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
	hint = {};

	display_list_indices.clear();
	display_list_base = 0;
	display_list_begun = 0;
	display_list_execute = false;

	attrib_sp = 0;
	select_name_sp = 0;

	render_mode = GL_RENDER;

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

	error_bits = 0;
}

void gl_state::destroy()
{
	display_list_indices.clear();
}

GLint APIENTRY glRenderMode(GLenum mode)
{
	gl_state *gs = gl_current_state();
	if (!gs) return 0;
	VALIDATE_NOT_BEGIN_MODE_RET(0);

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

bool &gl_get_enabled_ref(gl_state *gs, GLenum cap, bool &fail)
{
	if (cap == GL_NORMALIZE)
	{
		return gs->transform.normalize;
	}
	else if (cap >= GL_TEXTURE_GEN_S && cap <= GL_TEXTURE_GEN_Q)
	{
		return gs->texgen[cap - GL_TEXTURE_GEN_S].enabled;
	}
	else if (cap == GL_LIGHTING)
	{
		return gs->lighting.enabled;
	}
	else if (cap == GL_COLOR_MATERIAL)
	{
		return gs->lighting.color_material;
	}
	else if (cap == GL_POINT_SMOOTH)
	{
		return gs->point.smooth;
	}
	else if (cap == GL_LINE_SMOOTH)
	{
		return gs->line.smooth;
	}
	else if (cap == GL_LINE_STIPPLE)
	{
		return gs->line.stipple;
	}
	else if (cap == GL_POLYGON_SMOOTH)
	{
		return gs->polygon.smooth;
	}
	else if (cap == GL_POLYGON_STIPPLE)
	{
		return gs->polygon.stipple;
	}
	else if (cap == GL_CULL_FACE)
	{
		return gs->polygon.cull_face;
	}
	else if (cap == GL_TEXTURE_2D)
	{
		return gs->texture_2d_enabled;
	}
	else if (cap == GL_TEXTURE_1D)
	{
		return gs->texture_1d_enabled;
	}
	else if (cap == GL_FOG)
	{
		return gs->fog.enabled;
	}
	else if (cap == GL_SCISSOR_TEST)
	{
		return gs->scissor.test;
	}
	else if (cap == GL_ALPHA_TEST)
	{
		return gs->color_buffer.alpha_test;
	}
	else if (cap == GL_STENCIL_TEST)
	{
		return gs->stencil.test;
	}
	else if (cap == GL_DEPTH_TEST)
	{
		return gs->depth.test;
	}
	else if (cap == GL_BLEND)
	{
		return gs->color_buffer.blend;
	}
	else if (cap == GL_DITHER)
	{
		return gs->color_buffer.dither;
	}
	else if (cap == GL_LOGIC_OP)
	{
		return gs->color_buffer.logic_op;
	}
	else if (cap == GL_AUTO_NORMAL)
	{
		return gs->eval.auto_normal;
	}
	else
	{
		fail = true;
	}
	static bool _ = false;
	return _;
}

static void set_bit(uint32_t &set, int bit, bool val)
{
	if (val)
		set |= (1 << bit);
	else
		set &= ~(1 << bit);
}

//return false if cap is invalid
bool gl_setable(gl_state *gs, GLenum cap, bool val)
{
	bool fail = false;
	bool &state = gl_get_enabled_ref(gs, cap, fail);
	if (!fail)
	{
		state = val;
		return true;
	}
	else if (cap >= GL_CLIP_PLANE0 && cap < (GL_CLIP_PLANE0 + gl_max_user_clip_planes))
	{
		set_bit(gs->transform.enabled_clip_planes, cap - GL_CLIP_PLANE0, val);
	}
	else if (cap >= GL_LIGHT0 && cap < (GL_LIGHT0 + gl_max_lights))
	{
		set_bit(gs->lighting.enabled_lights, cap - GL_LIGHT0, val);
	}
	else if (cap >= GL_MAP1_COLOR_4 && cap <= GL_MAP1_VERTEX_4)
	{
		set_bit(gs->eval.enabled_maps, cap - GL_MAP1_COLOR_4, val);
	}
	else if (cap >= GL_MAP2_COLOR_4 && cap <= GL_MAP2_VERTEX_4)
	{
		set_bit(gs->eval.enabled_maps, cap - GL_MAP2_COLOR_4 + 9, val);
	}
	else
	{
		return false;
	}
	return true;
}

void APIENTRY glEnable(GLenum cap)
{
	gl_state *gs = gl_current_state();
	if (!gs) return;
	WRITE_DISPLAY_LIST(Enable, {}, {(int)cap});
	VALIDATE_NOT_BEGIN_MODE;

	if (cap == GL_COLOR_MATERIAL)
	{
		bool old = gs->lighting.color_material;
		gs->lighting.color_material = true;
		if (!old)
			gs->update_color_material();
		return;
	}

	if (!gl_setable(gs, cap, true))
		gl_set_error_a(GL_INVALID_ENUM, cap);
}

void APIENTRY glDisable(GLenum cap)
{
	gl_state *gs = gl_current_state();
	if (!gs) return;
	WRITE_DISPLAY_LIST(Disable, {}, { (int)cap });
	VALIDATE_NOT_BEGIN_MODE;

	if (!gl_setable(gs, cap, false))
		gl_set_error_a(GL_INVALID_ENUM, cap);
}

//return -1 if cap is invalid
int gl_isEnabled(gl_state *gs, GLenum cap)
{
	bool fail = false;
	bool &state = gl_get_enabled_ref(gs, cap, fail);
	if (!fail)
	{
		return state ? 1 : 0;
	}
	else if (cap >= GL_CLIP_PLANE0 && cap < (GL_CLIP_PLANE0 + gl_max_user_clip_planes))
	{
		return (gs->transform.enabled_clip_planes & (1 << (cap - GL_CLIP_PLANE0))) ? 1 : 0;
	}
	else if (cap >= GL_LIGHT0 && cap < (GL_LIGHT0 + gl_max_lights))
	{
		return (gs->lighting.enabled_lights & (1 << (cap - GL_LIGHT0))) ? 1 : 0;
	}
	else if (cap >= GL_MAP1_COLOR_4 && cap <= GL_MAP1_VERTEX_4)
	{
		return (gs->eval.enabled_maps & (1 << (cap - GL_MAP1_COLOR_4))) ? 1 : 0;
	}
	else if (cap >= GL_MAP2_COLOR_4 && cap <= GL_MAP2_VERTEX_4)
	{
		return (gs->eval.enabled_maps & (1 << (cap - GL_MAP2_COLOR_4 + 9))) ? 1 : 0;
	}

	return -1;
}

GLboolean APIENTRY glIsEnabled(GLenum cap)
{
	gl_state *gs = gl_current_state();
	if (!gs) return GL_FALSE;
	VALIDATE_NOT_BEGIN_MODE_RET(GL_FALSE);

	int r = gl_isEnabled(gs, cap);
	if (r == -1)
	{
		gl_set_error_a(GL_INVALID_ENUM, cap);
		return GL_FALSE;
	}

	return r;
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
		gs->hint.perspective_correction = mode;
	else if (target == GL_POINT_SMOOTH_HINT)
		gs->hint.point_smooth = mode;
	else if (target == GL_LINE_SMOOTH_HINT)
		gs->hint.line_smooth = mode;
	else if (target == GL_POLYGON_SMOOTH_HINT)
		gs->hint.polygon_smooth = mode;
	else if (target == GL_FOG_HINT)
		gs->hint.fog = mode;
}

const char *APIENTRY glGetString(GLenum name)
{
	gl_state *gs = gl_current_state();
	if (!gs) return nullptr;
	VALIDATE_NOT_BEGIN_MODE_RET(nullptr);

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
		return nullptr;
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
	VALIDATE_NOT_BEGIN_MODE_RET(0);

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
