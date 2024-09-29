#include "pch.h"
#include "gl_state.h"
#include "gl_exports.h"

static void copy_color(GLdouble *dst, const GLfloat *src, int count)
{
	for (int i = 0; i < count; i++)
		dst[i] = src[i];
}

template<typename T>
static void copy_color(GLboolean *dst, const T *col, int count)
{
	for (int i = 0; i < count; i++)
		dst[i] = col[i] != 0;
}

template<typename T>
static void copy_color(T *dst, const glm::vec4 &col)
{
	copy_color(dst, &col.x, col.length());
}

template<typename T>
static void copy_vals(GLboolean *dst, const T *src, int count)
{
	for (int i = 0; i < count; i++)
		dst[i] = src[i] != 0;
}

template<typename T>
static void copy_vals(GLint *dst, const T *src, int count)
{
	for (int i = 0; i < count; i++)
		dst[i] = src[i];
}

static void copy_vals(GLfloat *dst, const bool *src, int count)
{
	for (int i = 0; i < count; i++)
		dst[i] = src[i] ? 1.f : 0.f;
}

static void copy_vals(GLdouble *dst, const bool *src, int count)
{
	for (int i = 0; i < count; i++)
		dst[i] = src[i] ? 1.0 : 0.0;
}

template<typename T>
static void copy_vals(GLfloat *dst, const T *src, int count)
{
	for (int i = 0; i < count; i++)
		dst[i] = (GLfloat)src[i];
}

template<typename T>
static void copy_vals(GLdouble *dst, const T *src, int count)
{
	for (int i = 0; i < count; i++)
		dst[i] = (GLdouble)src[i];
}

template<typename T>
static void copy_vals(T *dst, const glm::vec4 &val)
{
	copy_vals(dst, &val.x, val.length());
}

template<typename T>
static void copy_vals(T *dst, const glm::mat4 &val)
{
	copy_vals(dst, &val[0].x, val.length() * val[0].length());
}

template<typename T>
static bool gl_get(gl_state &gs, GLenum pname, T *data)
{
	int r = gl_isEnabled(gs, pname);
	if (r != -1)
	{
		*data = (T)r;// 0 or 1
		return true;
	}

	bool keep_going = false;

	if (pname == GL_CURRENT_COLOR)
		copy_color(data, gs.current.color);
	else if (pname == GL_CURRENT_INDEX) {}
	else if (pname == GL_CURRENT_TEXTURE_COORDS)
		copy_vals(data, gs.current.tex_coord);
	else if (pname == GL_CURRENT_NORMAL)
		copy_color(data, &gs.current.normal.x, 3);
	else if (pname == GL_CURRENT_RASTER_POSITION)
		copy_vals(data, gs.current.raster.position);
	else if (pname == GL_CURRENT_RASTER_DISTANCE)
		copy_vals(data, &gs.current.raster.distance, 1);
	else if (pname == GL_CURRENT_RASTER_COLOR)
		copy_color(data, gs.current.raster.color);
	else if (pname == GL_CURRENT_RASTER_INDEX) {}
	else if (pname == GL_CURRENT_RASTER_TEXTURE_COORDS)
		copy_vals(data, gs.current.raster.tex_coord);
	else if (pname == GL_CURRENT_RASTER_POSITION_VALID)
		copy_vals(data, &gs.current.raster.valid, 1);
	else if (pname == GL_EDGE_FLAG)
		copy_vals(data, &gs.current.edge_flag, 1);
	else if (pname == GL_MODELVIEW_MATRIX)
		copy_vals(data, gs.modelview_stack[gs.modelview_stack_depth - 1]);
	else if (pname == GL_PROJECTION_MATRIX)
		copy_vals(data, gs.projection_stack[gs.projection_stack_depth - 1]);
	else if (pname == GL_TEXTURE_MATRIX)
		copy_vals(data, gs.texture_mtx_stack[gs.texture_mtx_stack_depth - 1]);
	else if (pname == GL_VIEWPORT)
	{
		int vals[4]{ gs.viewport.center_x, gs.viewport.center_y, gs.viewport.width, gs.viewport.height };
		vals[0] -= vals[2] / 2;
		vals[1] -= vals[3] / 2;
		copy_vals(data, vals, 4);
	}
	else if (pname == GL_DEPTH_RANGE)
	{
		copy_color(data, &gs.viewport.dnear, 1);
		copy_color(data + 1, &gs.viewport.dfar, 1);
	}
	else if (pname == GL_MODELVIEW_STACK_DEPTH)
		copy_vals(data, &gs.modelview_stack_depth, 1);
	else if (pname == GL_PROJECTION_STACK_DEPTH)
		copy_vals(data, &gs.projection_stack_depth, 1);
	else if (pname == GL_TEXTURE_STACK_DEPTH)
		copy_vals(data, &gs.texture_mtx_stack_depth, 1);
	else if (pname == GL_MATRIX_MODE)
		copy_vals(data, &gs.transform.matrix_mode, 1);
	else if (pname == GL_FOG_COLOR)
		copy_color(data, gs.fog.color);
	else if (pname == GL_FOG_INDEX) {}
	else if (pname == GL_FOG_DENSITY)
		copy_vals(data, &gs.fog.density, 1);
	else if (pname == GL_FOG_START)
		copy_vals(data, &gs.fog.start, 1);
	else if (pname == GL_FOG_END)
		copy_vals(data, &gs.fog.end, 1);
	else if (pname == GL_FOG_MODE)
		copy_vals(data, &gs.fog.mode, 1);
	else if (pname == GL_SHADE_MODEL)
	{
		GLenum shade_model = gs.lighting.shade_model_flat ? GL_FLAT : GL_SMOOTH;
		copy_vals(data, &shade_model, 1);
	}
	else if (pname == GL_COLOR_MATERIAL_PARAMETER)
		copy_vals(data, &gs.lighting.color_material_param, 1);
	else if (pname == GL_COLOR_MATERIAL_FACE)
		copy_vals(data, &gs.lighting.color_material_face, 1);
	else if (pname == GL_LIGHT_MODEL_AMBIENT)
		copy_color(data, gs.lighting.light_model_ambient);
	else if (pname == GL_LIGHT_MODEL_LOCAL_VIEWER)
		copy_vals(data, &gs.lighting.light_model_local_viewer, 1);
	else if (pname == GL_LIGHT_MODEL_TWO_SIDE)
		copy_vals(data, &gs.lighting.light_model_two_side, 1);
	else if (pname == GL_COLOR_INDEXES) {}
	else if (pname == GL_POINT_SIZE)
		copy_vals(data, &gs.point.size, 1);
	else if (pname == GL_LINE_WIDTH)
		copy_vals(data, &gs.line.width, 1);
	else if (pname == GL_LINE_STIPPLE_PATTERN)
		copy_vals(data, &gs.line.stipple_pattern, 1);
	else if (pname == GL_LINE_STIPPLE_REPEAT)
		copy_vals(data, &gs.line.stipple_repeat, 1);
	else if (pname == GL_CULL_FACE_MODE)
		copy_vals(data, &gs.polygon.cull_face_mode, 1);
	else if (pname == GL_FRONT_FACE)
	{
		GLenum front_face = gs.polygon.front_face_ccw ? GL_CCW : GL_CW;
		copy_vals(data, &front_face, 1);
	}
	else
		keep_going = true;
	// Break up else if chain to avoid exceeding `error C1061: compiler limit: blocks nested too deeply`
	if (!keep_going)
		return true;
	keep_going = false;

	if (pname == GL_POLYGON_MODE)
		copy_vals(data, gs.polygon.mode, 2);
	else if (pname == GL_SCISSOR_BOX)
		copy_vals(data, &gs.scissor.box.x, 4);
	else if (pname == GL_STENCIL_FUNC)
		copy_vals(data, &gs.stencil.func, 1);
	else if (pname == GL_STENCIL_VALUE_MASK)
		copy_vals(data, &gs.stencil.value_mask, 1);
	else if (pname == GL_STENCIL_REF)
		copy_vals(data, &gs.stencil.ref, 1);
	else if (pname == GL_STENCIL_FAIL)
		copy_vals(data, &gs.stencil.fail, 1);
	else if (pname == GL_STENCIL_PASS_DEPTH_FAIL)
		copy_vals(data, &gs.stencil.dpfail, 1);
	else if (pname == GL_STENCIL_PASS_DEPTH_PASS)
		copy_vals(data, &gs.stencil.dppass, 1);
	else if (pname == GL_ALPHA_TEST_FUNC)
		copy_vals(data, &gs.color_buffer.alpha_test_func, 1);
	else if (pname == GL_ALPHA_TEST_REF)
		copy_vals(data, &gs.color_buffer.alpha_test_ref, 1);
	else if (pname == GL_DEPTH_FUNC)
		copy_vals(data, &gs.depth.func, 1);
	else if (pname == GL_BLEND_SRC)
		copy_vals(data, &gs.color_buffer.blend_func_src, 1);
	else if (pname == GL_BLEND_DST)
		copy_vals(data, &gs.color_buffer.blend_func_dst, 1);
	else if (pname == GL_LOGIC_OP_MODE)
		copy_vals(data, &gs.color_buffer.logic_op_mode, 1);
	else if (pname == GL_DRAW_BUFFER)
		copy_vals(data, &gs.color_buffer.draw_buffer, 1);
	else if (pname == GL_INDEX_WRITEMASK) {}
	else if (pname == GL_COLOR_WRITEMASK)
		copy_vals(data, &gs.color_buffer.color_writemask.x, 4);
	else if (pname == GL_DEPTH_WRITEMASK)
		copy_vals(data, &gs.depth.writemask, 1);
	else if (pname == GL_STENCIL_WRITEMASK)
		copy_vals(data, &gs.stencil.writemask, 1);
	else if (pname == GL_COLOR_CLEAR_VALUE)
		copy_color(data, gs.color_buffer.color_clear_value);
	else if (pname == GL_INDEX_CLEAR_VALUE) {}
	else if (pname == GL_DEPTH_CLEAR_VALUE)
		copy_vals(data, &gs.depth.clear_value, 1);
	else if (pname == GL_STENCIL_CLEAR_VALUE)
		copy_vals(data, &gs.stencil.clear_value, 1);
	else if (pname == GL_ACCUM_CLEAR_VALUE)
		copy_vals(data, &gs.accum_clear_value.x, 4);//not color
	else if (pname == GL_UNPACK_SWAP_BYTES)
		copy_vals(data, &gs.pixel_unpack.swap_bytes, 1);
	else if (pname == GL_UNPACK_LSB_FIRST)
		copy_vals(data, &gs.pixel_unpack.lsb_first, 1);
	else if (pname == GL_UNPACK_ROW_LENGTH)
		copy_vals(data, &gs.pixel_unpack.row_length, 1);
	else if (pname == GL_UNPACK_SKIP_ROWS)
		copy_vals(data, &gs.pixel_unpack.skip_rows, 1);
	else if (pname == GL_UNPACK_SKIP_PIXELS)
		copy_vals(data, &gs.pixel_unpack.skip_pixels, 1);
	else if (pname == GL_UNPACK_ALIGNMENT)
		copy_vals(data, &gs.pixel_unpack.alignment, 1);
	else if (pname == GL_PACK_SWAP_BYTES)
		copy_vals(data, &gs.pixel_pack.swap_bytes, 1);
	else if (pname == GL_PACK_LSB_FIRST)
		copy_vals(data, &gs.pixel_pack.lsb_first, 1);
	else if (pname == GL_PACK_ROW_LENGTH)
		copy_vals(data, &gs.pixel_pack.row_length, 1);
	else if (pname == GL_PACK_SKIP_ROWS)
		copy_vals(data, &gs.pixel_pack.skip_rows, 1);
	else if (pname == GL_PACK_SKIP_PIXELS)
		copy_vals(data, &gs.pixel_pack.skip_pixels, 1);
	else if (pname == GL_PACK_ALIGNMENT)
		copy_vals(data, &gs.pixel_pack.alignment, 1);
	else if (pname == GL_MAP_COLOR)
		copy_vals(data, &gs.pixel.map_color, 1);
	else if (pname == GL_MAP_STENCIL)
		copy_vals(data, &gs.pixel.map_stencil, 1);
	else if (pname == GL_INDEX_SHIFT)
		copy_vals(data, &gs.pixel.index_shift, 1);
	else if (pname == GL_INDEX_OFFSET)
		copy_vals(data, &gs.pixel.index_offset, 1);
	else if (pname == GL_RED_SCALE)
		copy_vals(data, &gs.pixel.color_scale.r, 1);
	else if (pname == GL_GREEN_SCALE)
		copy_vals(data, &gs.pixel.color_scale.g, 1);
	else if (pname == GL_BLUE_SCALE)
		copy_vals(data, &gs.pixel.color_scale.b, 1);
	else if (pname == GL_ALPHA_SCALE)
		copy_vals(data, &gs.pixel.color_scale.a, 1);
	else if (pname == GL_DEPTH_SCALE)
		copy_vals(data, &gs.pixel.depth_scale, 1);
	else if (pname == GL_RED_BIAS)
		copy_vals(data, &gs.pixel.color_bias.r, 1);
	else if (pname == GL_GREEN_BIAS)
		copy_vals(data, &gs.pixel.color_bias.g, 1);
	else if (pname == GL_BLUE_BIAS)
		copy_vals(data, &gs.pixel.color_bias.b, 1);
	else if (pname == GL_ALPHA_BIAS)
		copy_vals(data, &gs.pixel.color_bias.a, 1);
	else if (pname == GL_DEPTH_BIAS)
		copy_vals(data, &gs.pixel.depth_bias, 1);
	else if (pname == GL_ZOOM_X)
		copy_vals(data, &gs.pixel.zoom.x, 1);
	else if (pname == GL_ZOOM_Y)
		copy_vals(data, &gs.pixel.zoom.y, 1);
	else if (pname == GL_PIXEL_MAP_I_TO_I_SIZE)
		copy_vals(data, &gs.pixel_map_index_table[0].size, 1);
	else if (pname == GL_PIXEL_MAP_S_TO_S_SIZE)
		copy_vals(data, &gs.pixel_map_index_table[1].size, 1);
	else if (pname >= GL_PIXEL_MAP_I_TO_R_SIZE && pname <= GL_PIXEL_MAP_A_TO_A_SIZE)
		copy_vals(data, &gs.pixel_map_color_table[pname - GL_PIXEL_MAP_I_TO_R_SIZE].size, 1);
	else if (pname == GL_READ_BUFFER)
		copy_vals(data, &gs.pixel.read_buffer, 1);
	else
		keep_going = true;
	// Break up else if chain to avoid exceeding `error C1061: compiler limit: blocks nested too deeply`
	if (!keep_going)
		return true;
	keep_going = false;

	if (pname == GL_MAP1_GRID_DOMAIN)
		copy_vals(data, gs.eval.map1d_grid_domain, 2);
	else if (pname == GL_MAP2_GRID_DOMAIN)
		copy_vals(data, gs.eval.map2d_grid_domain, 4);
	else if (pname == GL_MAP1_GRID_SEGMENTS)
		copy_vals(data, &gs.eval.map1d_grid_segments, 1);
	else if (pname == GL_MAP2_GRID_SEGMENTS)
		copy_vals(data, gs.eval.map2d_grid_segments, 2);
	else if (pname == GL_PERSPECTIVE_CORRECTION_HINT)
		copy_vals(data, &gs.hint.perspective_correction, 1);
	else if (pname == GL_POINT_SMOOTH_HINT)
		copy_vals(data, &gs.hint.point_smooth, 1);
	else if (pname == GL_LINE_SMOOTH_HINT)
		copy_vals(data, &gs.hint.line_smooth, 1);
	else if (pname == GL_POLYGON_SMOOTH_HINT)
		copy_vals(data, &gs.hint.polygon_smooth, 1);
	else if (pname == GL_FOG_HINT)
		copy_vals(data, &gs.hint.fog, 1);
	else if (pname == GL_MAX_LIGHTS)
		copy_vals(data, &gl_max_lights, 1);
	else if (pname == GL_MAX_CLIP_PLANES)
		copy_vals(data, &gl_max_user_clip_planes, 1);
	else if (pname == GL_MAX_MODELVIEW_STACK_DEPTH)
		copy_vals(data, &gl_max_viewmodel_mtx, 1);
	else if (pname == GL_MAX_PROJECTION_STACK_DEPTH)
		copy_vals(data, &gl_max_projection_mtx, 1);
	else if (pname == GL_MAX_TEXTURE_STACK_DEPTH)
		copy_vals(data, &gl_max_texture_mtx, 1);
	else if (pname == GL_SUBPIXEL_BITS)
		copy_vals(data, &gl_subpixel_bits, 1);
	else if (pname == GL_MAX_TEXTURE_SIZE)
		copy_vals(data, &gl_max_texture_size, 1);
	else if (pname == GL_MAX_PIXEL_MAP_TABLE)
		copy_vals(data, &gl_max_pixel_map_table, 1);
	else if (pname == GL_MAX_NAME_STACK_DEPTH)
		copy_vals(data, &gl_max_name_stack_depth, 1);
	else if (pname == GL_MAX_LIST_NESTING)
		copy_vals(data, &gl_max_list_nesting, 1);
	else if (pname == GL_MAX_EVAL_ORDER)
		copy_vals(data, &gl_max_eval_order, 1);
	else if (pname == GL_MAX_VIEWPORT_DIMS)
		copy_vals(data, gl_max_viewport_dims, 2);
	else if (pname == GL_MAX_ATTRIB_STACK_DEPTH)
		copy_vals(data, &gl_max_attrib_stack_depth, 1);
	else if (pname == GL_AUX_BUFFERS)
		copy_vals(data, &gl_max_aux_buffers, 1);
	else if (pname == GL_RGBA_MODE)
	{
		bool rgba_mode = true;
		copy_vals(data, &rgba_mode, 1);
	}
	else if (pname == GL_INDEX_MODE)
	{
		bool index_mode = false;
		copy_vals(data, &index_mode, 1);
	}
	else if (pname == GL_DOUBLEBUFFER)
		copy_vals(data, &gs.framebuffer->doublebuffer, 1);
	else if (pname == GL_STEREO)
	{
		bool stereo = false;
		copy_vals(data, &stereo, 1);
	}
	else if (pname == GL_POINT_SIZE_RANGE)
		copy_vals(data, gl_point_size_range, 2);
	else if (pname == GL_POINT_SIZE_GRANULARITY)
		copy_vals(data, &gl_point_size_granularity, 1);
	else if (pname == GL_LINE_WIDTH_RANGE)
		copy_vals(data, gl_line_width_range, 2);
	else if (pname == GL_LINE_WIDTH_GRANULARITY)
		copy_vals(data, &gl_line_width_granularity, 1);
	else if (pname >= GL_RED_BITS && pname <= GL_ALPHA_BITS)
	{
		int bits = 8;
		copy_vals(data, &bits, 1);
	}
	else if (pname == GL_INDEX_BITS) {}
	else if (pname == GL_DEPTH_BITS)
	{
		int depth_bits = gs.framebuffer->depth ? 16 : 0;
		copy_vals(data, &depth_bits, 1);
	}
	else if (pname == GL_STENCIL_BITS)
	{
		int stencil_bits = gs.framebuffer->stencil ? 8 : 0;
		copy_vals(data, &stencil_bits, 1);
	}
	else if (pname >= GL_ACCUM_RED_BITS && pname <= GL_ACCUM_ALPHA_BITS)
	{
		int bits = 0;
		copy_vals(data, &bits, 1);
	}
	else if (pname == GL_LIST_BASE)
		copy_vals(data, &gs.display_list_base, 1);
	else if (pname == GL_LIST_INDEX)
		copy_vals(data, &gs.display_list_begun, 1);
	else if (pname == GL_LIST_MODE)
	{
		GLenum list_mode = gs.display_list_execute ? GL_COMPILE_AND_EXECUTE : GL_COMPILE;
		copy_vals(data, &list_mode, 1);
	}
	else if (pname == GL_ATTRIB_STACK_DEPTH)
		copy_vals(data, &gs.attrib_sp, 1);
	else if (pname == GL_NAME_STACK_DEPTH)
		copy_vals(data, &gs.select_name_sp, 1);
	else if (pname == GL_RENDER_MODE)
		copy_vals(data, &gs.render_mode, 1);
	else
		return false;

	return true;
}

void APIENTRY glGetBooleanv(GLenum pname, GLboolean *data)
{
	gl_state *gs = gl_current_state();
	if (!gs) return;
	VALIDATE_NOT_BEGIN_MODE;
	if (!gl_get(*gs, pname, data))
		gl_set_error_a(GL_INVALID_ENUM, pname);
}

void APIENTRY glGetIntegerv(GLenum pname, GLint *data)
{
	gl_state *gs = gl_current_state();
	if (!gs) return;
	VALIDATE_NOT_BEGIN_MODE;
	if (!gl_get(*gs, pname, data))
		gl_set_error_a(GL_INVALID_ENUM, pname);
}

void APIENTRY glGetFloatv(GLenum pname, GLfloat *data)
{
	gl_state *gs = gl_current_state();
	if (!gs) return;
	VALIDATE_NOT_BEGIN_MODE;
	if (!gl_get(*gs, pname, data))
		gl_set_error_a(GL_INVALID_ENUM, pname);
}

void APIENTRY glGetDoublev(GLenum pname, GLdouble *data)
{
	gl_state *gs = gl_current_state();
	if (!gs) return;
	VALIDATE_NOT_BEGIN_MODE;
	if (!gl_get(*gs, pname, data))
		gl_set_error_a(GL_INVALID_ENUM, pname);
}

template<typename DT, typename ST>
static void gl_copyAttribs(DT &d, const ST &s, GLbitfield mask)
{
	if (mask & GL_CURRENT_BIT)
		d.current = s.current;
	if (mask & GL_VIEWPORT_BIT)
		d.viewport = s.viewport;
	if (mask & GL_TRANSFORM_BIT)
		d.transform = s.transform;
	if (mask & GL_FOG_BIT)
		d.fog = s.fog;
	if (mask & GL_LIGHTING_BIT)
		d.lighting = s.lighting;
	if (mask & GL_POINT_BIT)
		d.point = s.point;
	if (mask & GL_LINE_BIT)
		d.line = s.line;
	if (mask & GL_POLYGON_BIT)
		d.polygon = s.polygon;
	if (mask & GL_POLYGON_STIPPLE_BIT)
		memcpy(d.polygon_stipple_mask, s.polygon_stipple_mask, sizeof(d.polygon_stipple_mask));
	if (mask & GL_TEXTURE_BIT)
	{
		d.texture_1d_enabled = s.texture_1d_enabled;
		d.texture_2d_enabled = s.texture_2d_enabled;
		d.texture_env = s.texture_env;
		memcpy(d.texgen, s.texgen, sizeof(d.texgen));
	}
	if (mask & GL_SCISSOR_BIT)
		d.scissor = s.scissor;
	if (mask & GL_STENCIL_BUFFER_BIT)
		d.stencil = s.stencil;
	if (mask & GL_COLOR_BUFFER_BIT)
		d.color_buffer = s.color_buffer;
	if (mask & GL_DEPTH_BUFFER_BIT)
		d.depth = s.depth;
	if (mask & GL_ACCUM_BUFFER_BIT)
		d.accum_clear_value = s.accum_clear_value;
	if (mask & GL_PIXEL_MODE_BIT)
		d.pixel = s.pixel;
	if (mask & GL_EVAL_BIT)
		d.eval = s.eval;
	if (mask & GL_HINT_BIT)
		d.hint = s.hint;
	if (mask & GL_LIST_BIT)
		d.display_list_base = s.display_list_base;

	if (!(mask & GL_ENABLE_BIT))
		return;

	d.transform.normalize = s.transform.normalize;
	d.transform.enabled_clip_planes = s.transform.enabled_clip_planes;
	d.fog.enabled = s.fog.enabled;
	d.lighting.enabled = s.lighting.enabled;
	d.lighting.color_material = s.lighting.color_material;
	d.lighting.enabled_lights = s.lighting.enabled_lights;
	d.point.smooth = s.point.smooth;
	d.line.smooth = s.line.smooth;
	d.line.stipple = s.line.stipple;
	d.polygon.cull_face = s.polygon.cull_face;
	d.polygon.smooth = s.polygon.smooth;
	d.polygon.stipple = s.polygon.stipple;
	d.texture_1d_enabled = s.texture_1d_enabled;
	d.texture_2d_enabled = s.texture_2d_enabled;
	for (int i = 0; i < 4; i++)
		d.texgen[i].enabled = s.texgen[i].enabled;
	d.scissor.test = s.scissor.test;
	d.stencil.test = s.stencil.test;
	d.color_buffer.alpha_test = s.color_buffer.alpha_test;
	d.depth.test = s.depth.test;
	d.color_buffer.blend = s.color_buffer.blend;
	d.color_buffer.logic_op = s.color_buffer.logic_op;
	d.color_buffer.dither = s.color_buffer.dither;
	d.eval.enabled_maps = s.eval.enabled_maps;
}

void APIENTRY glPushAttrib(GLbitfield mask)
{
	gl_state *gs = gl_current_state();
	if (!gs) return;
	VALIDATE_NOT_BEGIN_MODE;

	if (gs->attrib_sp >= gl_max_attrib_stack_depth)
	{
		gl_set_error(GL_STACK_OVERFLOW);
		return;
	}

	auto &d = gs->attrib_stack[gs->attrib_sp];
	d.attrib_mask = mask;

	gl_copyAttribs(d, *gs, mask);
	if (mask & GL_TEXTURE_BIT)
	{
		d.texture_1d = gs->texture_1d.params;
		d.texture_2d = gs->texture_2d.params;
	}

	gs->attrib_sp++;
}

void APIENTRY glPopAttrib(void)
{
	gl_state *gs = gl_current_state();
	if (!gs) return;
	VALIDATE_NOT_BEGIN_MODE;

	if (gs->attrib_sp == 0)
	{
		gl_set_error(GL_STACK_UNDERFLOW);
		return;
	}

	gs->attrib_sp--;
	const auto &s = gs->attrib_stack[gs->attrib_sp];

	gl_copyAttribs(*gs, s, s.attrib_mask);
	if (s.attrib_mask & GL_TEXTURE_BIT)
	{
		gs->texture_1d.params = s.texture_1d;
		gs->texture_2d.params = s.texture_2d;
	}
}
