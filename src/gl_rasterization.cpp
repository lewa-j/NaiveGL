#include "pch.h"
#include "gl_state.h"
#include "gl_exports.h"
#include <utility>

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
	WRITE_DISPLAY_LIST_BULK(PolygonStipple, mask, 4 * 32);
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

static void gl_fog_scalar(const char *func, gl_state *gs, GLenum pname, GLfloat param)
{
	if (pname != GL_FOG_MODE && pname != GL_FOG_DENSITY && pname != GL_FOG_START && pname != GL_FOG_END && pname != GL_FOG_INDEX)\
	{
		gl_set_error_a_(GL_INVALID_ENUM, pname, func);
		return;
	}
	if (pname == GL_FOG_MODE && param != GL_EXP && param != GL_EXP2 && param != GL_LINEAR)
	{
		gl_set_error_a_(GL_INVALID_ENUM, (int)param, func);
		return;
	}
	if ((pname == GL_FOG_DENSITY || pname == GL_FOG_START || pname == GL_FOG_END) && param < 0)
	{
		gl_set_error_(GL_INVALID_VALUE, func);
		return;
	}

	if (pname == GL_FOG_MODE)
		gs->fog_mode = (int)param;
	else if (pname == GL_FOG_DENSITY)
		gs->fog_density = param;
	else if (pname == GL_FOG_START)
		gs->fog_start = param;
	else if (pname == GL_FOG_END)
		gs->fog_end = param;
	//else GL_FOG_INDEX
}

void APIENTRY glFogf(GLenum pname, GLfloat param)
{
	gl_state* gs = gl_current_state();
	if (!gs) return;
	WRITE_DISPLAY_LIST(Fog, { param }, { (int)pname });
	VALIDATE_NOT_BEGIN_MODE;

	gl_fog_scalar(__FUNCTION__, gs, pname, param);
}

void APIENTRY glFogi(GLenum pname, GLint param) { glFogf(pname, (GLfloat)param); }

static int gl_fogv_size(GLenum pname)
{
	if (pname >= GL_FOG_INDEX && pname <= GL_FOG_MODE)
		return 1;
	if (pname == GL_FOG_COLOR)
		return 4;
	return 0;
}

template<typename T>
void gl_fogv(gl_state *gs, GLenum pname, const T* params)
{
	VALIDATE_NOT_BEGIN_MODE;
	if (pname == GL_FOG_COLOR)
		gs->fog_color = glm::clamp(glm::vec4(GLtof(params[0]), GLtof(params[1]), GLtof(params[2]), GLtof(params[3])), 0.0f, 1.0f);
	else
		gl_fog_scalar(__FUNCTION__, gs, pname, (GLfloat)params[0]);
}

void APIENTRY glFogiv(GLenum pname, const GLint* params)
{
	gl_state *gs = gl_current_state();
	if (!gs) return;
	WRITE_DISPLAY_LIST_IV(Fogiv, params, gl_fogv_size(pname), 1, {}, { (int)pname });
	gl_fogv(gs, pname, params);
}

void APIENTRY glFogfv(GLenum pname, const GLfloat* params)
{
	gl_state *gs = gl_current_state();
	if (!gs) return;
	WRITE_DISPLAY_LIST_FV(Fogfv, params, gl_fogv_size(pname), {}, { (int)pname });
	gl_fogv(gs, pname, params);
}


static void apply_texture(gl_state& st, glm::vec4& color, const gl_frag_data &data)
{
	glm::vec4 tex_color(1);
	int cpts = 0;
	if (st.texture_2d_enabled)
	{
		if (!st.texture_2d.is_complete)
			return;
		tex_color = st.sample_tex2d(st.texture_2d, data.tex_coord, data.lod);
		cpts = st.texture_2d.arrays[0].components;
	}
	else if (st.texture_1d_enabled)
	{
		if (!st.texture_1d.is_complete)
			return;
		glm::vec4 t = data.tex_coord;
		t.y = 0.5f;
		tex_color = st.sample_tex2d(st.texture_1d, t, data.lod);
		cpts = st.texture_1d.arrays[0].components;
	}

	if (st.texture_env_function == GL_DECAL)
	{
		if (cpts == 4)
			color = glm::vec4((1 - tex_color.a) * glm::vec3(color) + tex_color.a * glm::vec3(tex_color), color.a);
		else
			color = glm::vec4(glm::vec3(tex_color), color.a);
		// 1 and 2 components are undefined
	}
	else if (st.texture_env_function == GL_BLEND)
	{
		color = glm::vec4((1 - tex_color.r) * glm::vec3(color) + tex_color.r * glm::vec3(st.texture_env_color), tex_color.a * color.a);
		// 3 and 4 components are undefined
	}
	else //if (st.texture_env_function == GL_MODULATE)
	{
		if (cpts < 3)
			color = glm::vec4(tex_color.r * glm::vec3(color), tex_color.a * color.a);
		else
			color *= tex_color;
	}
}

glm::vec4 gl_state::get_fog_color(const glm::vec4& cr, float c)
{
	float f = 0;

	if (fog_mode == GL_EXP)
		f = exp(-fog_density * c);
	else if (fog_mode == GL_EXP2)
		f = exp(-(fog_density * c) * (fog_density * c));
	else if (fog_mode == GL_LINEAR)
		f = (fog_end - c) / (fog_end - fog_start);

	f = glm::clamp(f, 0.f, 1.f);
	return glm::vec4(f * glm::vec3(cr) + (1 - f) * glm::vec3(fog_color), cr.w);
}

static bool gl_test_value(GLenum func, int a, int b)
{
	if (func == GL_NEVER)
		return false;
	if (func == GL_LESS)
		return a < b;
	if (func == GL_LEQUAL)
		return a <= b;
	if (func == GL_EQUAL)
		return a == b;
	if (func == GL_GREATER)
		return a > b;
	if (func == GL_NOTEQUAL)
		return a != b;
	if (func == GL_GEQUAL)
		return a >= b;
	if (func == GL_ALWAYS)
		return true;
	return false;
}

static void gl_stencil_op(GLenum op, GLint ref, GLint& sv)
{
	if (op == GL_ZERO)
		sv = 0;
	else if (op == GL_REPLACE)
		sv = ref;
	else if (op == GL_INCR)
		sv = glm::clamp(sv + 1, 0, 0xFF);
	else if (op == GL_DECR)
		sv = glm::clamp(sv - 1, 0, 0xFF);
	else if (op == GL_INVERT)
		sv = ~sv;
}

static glm::vec4 gl_blend(GLenum sfactor, GLenum dfactor, glm::vec4 color_src, const glm::vec4 &color_dst)
{
	glm::vec4 src;
	glm::vec4 dst;

	if (sfactor == GL_ZERO)
		src = glm::vec4(0);
	else if (sfactor == GL_ONE)
		src = glm::vec4(1);
	else if (sfactor == GL_DST_COLOR)
		src = color_dst;
	else if (sfactor == GL_ONE_MINUS_DST_COLOR)
		src = glm::vec4(1) - color_dst;
	else if (sfactor == GL_SRC_ALPHA)
		src = glm::vec4(color_src.a);
	else if(sfactor == GL_ONE_MINUS_SRC_ALPHA)
		src = glm::vec4(1 - color_src.a);
	else if (sfactor == GL_DST_ALPHA)
		src = glm::vec4(color_dst.a);
	else if (sfactor == GL_ONE_MINUS_DST_ALPHA)
		src = glm::vec4(1 - color_dst.a);
	else if (sfactor == GL_SRC_ALPHA_SATURATE)
	{
		float f = glm::min(color_src.a, 1 - color_dst.a);
		src = glm::vec4(f, f, f, 1);
	}

	if (dfactor == GL_ZERO)
		dst = glm::vec4(0);
	else if (dfactor == GL_ONE)
		dst = glm::vec4(1);
	else if (dfactor == GL_SRC_COLOR)
		dst = color_src;
	else if (dfactor == GL_ONE_MINUS_SRC_COLOR)
		dst = glm::vec4(1) - color_src;
	else if (dfactor == GL_SRC_ALPHA)
		dst = glm::vec4(color_src.a);
	else if (dfactor == GL_ONE_MINUS_SRC_ALPHA)
		dst = glm::vec4(1 - color_src.a);
	else if (dfactor == GL_DST_ALPHA)
		dst = glm::vec4(color_dst.a);
	else if (dfactor == GL_ONE_MINUS_DST_ALPHA)
		dst = glm::vec4(1 - color_dst.a);

	return glm::clamp(color_src * src + color_dst * dst, 0.f, 1.f);
}

void gl_dither(glm::vec4& color, int x, int y)
{
	// https://bisqwit.iki.fi/story/howto/dither/jy/
	int i = x & 0x7;
	int j = y & 0x7;
	int ij = i ^ j;
	uint8_t d = ((j >> 2) & 1) | (((ij >> 2) & 1) << 1) | (((j >> 1) & 1) << 2) | (((ij >> 1) & 1) << 3) | ((j & 1) << 4) | ((ij & 1) << 5);
	color += d / (64.f * 256.f);
}

void gl_emit_fragment(gl_state &st, int x, int y, gl_frag_data &data)
{
	gl_framebuffer &fb = *st.framebuffer;
	if (x < 0 || x >= fb.width || y < 0 || y >= fb.height)
		return;

	if (st.scissor_test)
	{
		const glm::ivec4 &s = st.scissor_rect;
		if (x < s.x || x >= s.x + s.z || y < s.y || y >= s.y + s.w)
			return;
	}

	glm::vec4 color = data.color;

	if (st.texture_2d_enabled || st.texture_1d_enabled)
		apply_texture(st, color, data);

	// TODO apply coverage to alpha for antialiasing
	// NOTE smooth point rasterization already applies coverage to pixel alpha

	if (st.alpha_test && !gl_test_value(st.alpha_test_func, int(color.a * 255), int(st.alpha_test_ref * 255)))
		return;

	int pi = (fb.width * y + x);

	if (st.stencil_test && fb.stencil)
	{
		int sv = fb.stencil[pi];
		if (!gl_test_value(st.stencil_func, st.stencil_test_ref & st.stencil_test_mask, sv & st.stencil_test_mask))
		{
			gl_stencil_op(st.stencil_op_sfail, st.stencil_test_ref, sv);
			if (st.stencil_writemask)
				fb.stencil[pi] = (sv & 0xFF & st.stencil_writemask) | (fb.stencil[pi] & ~st.stencil_writemask);
			return;
		}
	}

	if (st.depth_test && fb.depth)
	{
		uint16_t dv = fb.depth[pi];
		uint16_t dn = uint16_t(glm::clamp(data.z,0.f,1.f) * 0xFFFF);
		if (!gl_test_value(st.depth_func, dn, dv))
		{
			//depth fail
			if (st.stencil_test && fb.stencil)
			{
				int sv = fb.stencil[pi];
				gl_stencil_op(st.stencil_op_dpfail, st.stencil_test_ref, sv);
				if (st.stencil_writemask)
					fb.stencil[pi] = (sv & 0xFF & st.stencil_writemask) | (fb.stencil[pi] & ~st.stencil_writemask);
			}
			return;
		}
		if (st.depth_mask)
			fb.depth[pi] = dn;
	}

	//depth pass
	if (st.stencil_test && fb.stencil)
	{
		int sv = fb.stencil[pi];
		gl_stencil_op(st.stencil_op_dppass, st.stencil_test_ref, sv);
		if (st.stencil_writemask)
			fb.stencil[pi] = (sv & 0xFF & st.stencil_writemask) | (fb.stencil[pi] & ~st.stencil_writemask);
	}

	if (st.draw_buffer == GL_NONE)
		return;

	if (st.fog_enabled)
		color = st.get_fog_color(color, data.fog_z);

	int ci = pi * 4;
	if (st.blend && (st.blend_func_src != GL_ONE || st.blend_func_dst != GL_ZERO))
	{
		//bgra
		glm::vec4 dst_color = glm::vec4(
			fb.color[ci + 2],
			fb.color[ci + 1],
			fb.color[ci + 0],
			fb.color[ci + 3]) / 255.f;
		color = gl_blend(st.blend_func_src, st.blend_func_dst, color, dst_color);
	}

	if (st.dither)
	{
		gl_dither(color, x, y);
	}
	
	//bgra
	if (st.color_mask.b)
		fb.color[ci]     = uint8_t(color.b * 0xFF);
	if (st.color_mask.g)
		fb.color[ci + 1] = uint8_t(color.g * 0xFF);
	if (st.color_mask.r)
		fb.color[ci + 2] = uint8_t(color.r * 0xFF);
	if (st.color_mask.a)
		fb.color[ci + 3] = uint8_t(color.a * 0xFF);
}

void gl_emit_point(gl_state& st, const gl_processed_vertex &vertex)
{
	if (!st.clip_point(vertex.position, vertex.clip))
		return;

	glm::vec3 device_c = glm::vec3(vertex.clip) / vertex.clip.w;
	glm::vec3 win_c = st.get_window_coords(device_c);

	if (st.render_mode == GL_SELECT)
	{
		gl_add_selection_depth(st, win_c.z);
		st.select_hit = true;
		return;
	}
	else if (st.render_mode == GL_FEEDBACK)
	{
		gl_feedback_write(st, GL_POINT_TOKEN);
		gl_feedback_write_vertex(st, vertex);
		return;
	}

	gl_frag_data data;
	data.color = vertex.color;
	data.tex_coord = vertex.tex_coord;
	data.z = win_c.z;
	data.fog_z = abs(vertex.position.z);
	data.lod = 0;

	if (!st.point_smooth)
	{
		int w = std::min((int)round(st.point_size), gl_max_point_size);
		w = w < 1 ? 1 : w;
		glm::ivec2 ic(floor(win_c + (w & 1 ? 0 : 0.5f)));//add half pixel when size is even. then truncate to int
		if (w == 1)
			gl_emit_fragment(st, ic.x, ic.y, data);
		else
		{
			ic -= (w >> 1);//floor(w * 0.5)
			for (int ix = 0; ix < w; ix++)
				for (int iy = 0; iy < w; iy++)
					gl_emit_fragment(st, ic.x + ix, ic.y + iy, data);
		}
	}
	else
	{
		//NOTE: POINT_SIZE_GRANULARITY ignored
		float w = glm::clamp(st.point_size, gl_point_size_range[0], gl_point_size_range[1]);
		int wi = int(ceil(w) + 1);
		glm::ivec2 ic(floor(win_c));
		ic -= (wi >> 1);
		w *= 0.5f;
		for (int ix = 0; ix < wi; ix++)
			for (int iy = 0; iy < wi; iy++)
			{
				glm::vec2 p{ ic.x + ix + 0.5f, ic.y + iy + 0.5f };
				//rough approximation of pixel/circle intersection
				float coverage = w - glm::distance(p, glm::vec2(win_c)) + 0.5f;
				if (coverage <= 0.f)
					continue;
				data.color.a = vertex.color.a * glm::min(coverage, 1.0f);
				gl_emit_fragment(st, ic.x + ix, ic.y + iy, data);
			}
	}
}

static bool line_stipple(gl_state &st)
{
	if (!st.line_stipple)
		return true;

	int b = (st.line_stipple_counter / st.line_stipple_factor) & 0xF;
	st.line_stipple_counter++;

	return (st.line_stipple_pattern >> b) & 1;
}

void gl_rasterize_line(gl_state& st, const gl_processed_vertex& v0, const gl_processed_vertex& v1)
{
	if (st.render_mode == GL_FEEDBACK)
	{
		gl_feedback_write(st, (float) (st.feedback_reset_line ? GL_LINE_RESET_TOKEN : GL_LINE_TOKEN));
		st.feedback_reset_line = false;
		gl_feedback_write_vertex(st, v0);
		gl_feedback_write_vertex(st, v1);
		return;
	}

	glm::vec3 device_c0 = glm::vec3(v0.clip) / v0.clip.w;
	glm::vec3 win_c0 = st.get_window_coords(device_c0);

	glm::vec3 device_c1 = glm::vec3(v1.clip) / v1.clip.w;
	glm::vec3 win_c1 = st.get_window_coords(device_c1);

	if (st.render_mode == GL_SELECT)
	{
		gl_add_selection_depth(st, win_c0.z);
		gl_add_selection_depth(st, win_c1.z);
		st.select_hit = true;
		return;
	}

	glm::ivec2 ic0(floor(win_c0));
	glm::ivec2 ic1(floor(win_c1));

	bool low = false;
	bool left = false;
	if (glm::abs(ic0.x - ic1.x) < glm::abs(ic0.y - ic1.y))
	{
		low = true;
		std::swap(ic0.x, ic0.y);
		std::swap(ic1.x, ic1.y);
	}

	if (ic0.x > ic1.x)
	{
		left = true;
		std::swap(ic0, ic1);
	}

	gl_frag_data data;

	int dx = ic1.x - ic0.x;
	int dy = ic1.y - ic0.y;

	int derror2 = glm::abs(dy) * 2;
	int error2 = 0;
	int y = ic0.y;
	int ydir = (ic1.y > ic0.y ? 1 : -1);

	float t = 0;
	float fdx;
	if (left)
	{
		t = 1;
		fdx = -1.0f / dx; //invert t direction instead of swaping verts data

		//skip last point
		ic0.x++;
		error2 += derror2;
		if (error2 > dx)
		{
			y += ydir;
			error2 -= dx * 2;
		}
		t += fdx;
	}
	else
	{
		fdx = 1.0f / dx;

		ic1.x--;//skip last point
	}

	//TODO wide lines
	//TODO antialiasing

	for (int x = ic0.x; x <= ic1.x; x++)
	{
		if (line_stipple(st))
		{
			data.color = glm::mix(v0.color / v0.clip.w, v1.color / v1.clip.w, t) / glm::mix(1.0f / v0.clip.w, 1.0f / v1.clip.w, t);
			data.tex_coord = glm::mix(v0.tex_coord / v0.clip.w, v1.tex_coord / v1.clip.w, t) / glm::mix(v0.tex_coord.q / v0.clip.w, v1.tex_coord.q / v1.clip.w, t);
			data.lod = 0;
			if (st.need_tex_lod())
			{
				float l = glm::sqrt(float(dx * dx + dy * dy));

				float t2 = t + fdx;
				glm::vec2 tex_coord2 = glm::mix(v0.tex_coord / v0.clip.w, v1.tex_coord / v1.clip.w, t2) / glm::mix(v0.tex_coord.q / v0.clip.w, v1.tex_coord.q / v1.clip.w, t2);
				glm::vec2 duv = (tex_coord2 - glm::vec2(data.tex_coord)) * glm::vec2(st.texture_2d.arrays[0].width, st.texture_2d.arrays[0].height);

				float duxy = duv.x * dx + duv.x * dy;
				float dvxy = duv.y * dx + duv.y * dy;
				float scale_factor = glm::sqrt(duxy * duxy + dvxy * dvxy) / l;
				data.lod = glm::log2(scale_factor);
			}

			if (st.fog_enabled)
				data.fog_z = abs(glm::mix(v0.position.z, v1.position.z, t));

			if (st.depth_test && st.framebuffer->depth)
				data.z = glm::mix(win_c0.z, win_c1.z, t);
			gl_emit_fragment(st, low ? y : x, low ? x : y, data);
		}
		t += fdx;

		error2 += derror2;
		if (error2 > dx)
		{
			y += ydir;
			error2 -= dx * 2;
		}
	}
}

void gl_emit_line(gl_state& st, gl_processed_vertex &v0, gl_processed_vertex &v1)
{
	if (v0.clip.z > v0.clip.w && v1.clip.z > v1.clip.w)
		return;
	if (v0.clip.z < -v0.clip.w && v1.clip.z < -v1.clip.w)
		return;
	if (v0.clip.x < -v0.clip.w && v1.clip.x < -v1.clip.w)
		return;
	if (v0.clip.x > v0.clip.w && v1.clip.x > v1.clip.w)
		return;
	if (v0.clip.y < -v0.clip.w && v1.clip.y < -v1.clip.w)
		return;
	if (v0.clip.y > v0.clip.w && v1.clip.y > v1.clip.w)
		return;

	if (st.shade_model_flat)
		v0.color = v1.color;

	if (st.clip_point(v0.position, v0.clip) && st.clip_point(v1.position, v1.clip))
		gl_rasterize_line(st, v0, v1);
	else
		gl_rasterize_clipped_line(st, v0, v1);
}

static bool triangle_side(gl_state& st, const gl_processed_vertex& v0, const gl_processed_vertex& v1, const gl_processed_vertex& v2)
{
	glm::vec3 pts[3];

	pts[0] = st.get_window_coords(glm::vec3(v0.clip) / v0.clip.w);
	pts[1] = st.get_window_coords(glm::vec3(v1.clip) / v1.clip.w);
	pts[2] = st.get_window_coords(glm::vec3(v2.clip) / v2.clip.w);

	float sarea = (pts[2].x - pts[1].x) * (pts[0].y - pts[1].y) - (pts[0].x - pts[1].x) * (pts[2].y - pts[1].y);
	return sarea > 0 != st.front_face_ccw;
}

glm::vec3 barycentric(glm::vec3* pts, glm::vec2 P)
{
	glm::vec3 u = glm::cross(glm::vec3(pts[2].x - pts[0].x, pts[1].x - pts[0].x, pts[0].x - P.x), glm::vec3(pts[2].y - pts[0].y, pts[1].y - pts[0].y, pts[0].y - P.y));
	if (std::abs(u[2]) < 1)
		return glm::vec3(-1, 1, 1); // triangle is degenerate, in this case return smth with negative coordinates
	return glm::vec3(1.f - (u.x + u.y) / u.z, u.y / u.z, u.x / u.z);
}

void gl_rasterize_triangle(gl_state& st, gl_processed_vertex& v0, gl_processed_vertex& v1, gl_processed_vertex& v2)
{
	if (st.cull_face)
	{
		if (st.cull_face_mode == GL_FRONT_AND_BACK)
			return;

		if (st.last_side != (st.cull_face_mode == GL_FRONT))
			return;
	}

	if (st.render_mode == GL_SELECT)
	{
		gl_add_selection_depth(st, st.get_window_coords(glm::vec3(v0.clip) / v0.clip.w).z);
		gl_add_selection_depth(st, st.get_window_coords(glm::vec3(v1.clip) / v1.clip.w).z);
		gl_add_selection_depth(st, st.get_window_coords(glm::vec3(v2.clip) / v2.clip.w).z);
		st.select_hit = true;
		return;
	}

	if (st.polygon_mode[st.last_side] == GL_LINE)
	{
		if (v0.edge) gl_rasterize_line(st, v0, v1);
		if (v1.edge) gl_rasterize_line(st, v1, v2);
		if (v2.edge) gl_rasterize_line(st, v2, v0);
		return;
	}

	if (st.render_mode == GL_FEEDBACK)
	{
		gl_feedback_write(st, GL_POLYGON_TOKEN);
		gl_feedback_write(st, 3);
		gl_feedback_write_vertex(st, v0);
		gl_feedback_write_vertex(st, v1);
		gl_feedback_write_vertex(st, v2);
		return;
	}

	glm::ivec4 rect(0, 0, st.framebuffer->width-1, st.framebuffer->height-1);
	if (st.scissor_test)
	{
		rect.x = glm::max(0, st.scissor_rect.x);
		rect.y = glm::max(0, st.scissor_rect.y);
		rect.z = glm::min(st.scissor_rect.z, st.framebuffer->width - st.scissor_rect.x);
		rect.w = glm::min(st.scissor_rect.w, st.framebuffer->height - st.scissor_rect.y);
	}

	glm::ivec2 bbmin(rect.x + rect.z, rect.y + rect.w);
	glm::ivec2 bbmax(rect.x, rect.y);

	glm::vec3 device_c[3]{
		glm::vec3(v0.clip) / v0.clip.w,
		glm::vec3(v1.clip) / v1.clip.w,
		glm::vec3(v2.clip) / v2.clip.w };

	glm::vec3 win_c[3];
	for (int i = 0; i < 3; i++)
	{
		win_c[i] = st.get_window_coords(device_c[i]);
		glm::ivec2 ic = glm::floor(win_c[i]);

		for (int j = 0; j < 2; j++)
		{
			bbmin[j] = glm::clamp(ic[j], rect[j], bbmin[j]);
			bbmax[j] = glm::clamp(ic[j], bbmax[j], rect[j] + rect[j + 2]);
		}
	}

	gl_frag_data data;

	glm::ivec2 P;
	for (P.x = bbmin.x; P.x <= bbmax.x; P.x++)
	{
		for (P.y = bbmin.y; P.y <= bbmax.y; P.y++)
		{
			if (st.polygon_stipple)
			{
				if (!(st.polygon_stipple_mask[(P.y & 0x1F) * 4 + ((P.x >> 3) & 3)] & (128 >> (P.x & 7))))
					continue;
			}

			glm::vec3 bc_screen = barycentric(win_c, glm::vec2(P) + glm::vec2(0.5f));
			if (bc_screen.x < 0 || bc_screen.y < 0 || bc_screen.z <= 0)
				continue;

			data.color = bc_screen.x * v0.color + bc_screen.y * v1.color + bc_screen.z * v2.color;
			//affine
			//data.tex_coord = bc_screen.x * v0.tex_coord + bc_screen.y * v1.tex_coord + bc_screen.z * v2.tex_coord;
			//perspective
			//data.tex_coord = (bc_screen.x * v0.tex_coord / v0.clip.w + bc_screen.y * v1.tex_coord / v1.clip.w + bc_screen.z * v2.tex_coord / v2.clip.w) 
			//	/ (bc_screen.x * v0.tex_coord.w / v0.clip.w + bc_screen.y * v1.tex_coord.w / v1.clip.w + bc_screen.z * v2.tex_coord.w / v2.clip.w);
			glm::vec3 bc_clip{ bc_screen.x / v0.clip.w, bc_screen.y / v1.clip.w, bc_screen.z / v2.clip.w };
			bc_clip = bc_clip / (bc_clip.x + bc_clip.y + bc_clip.z);
			data.tex_coord = bc_clip.x * v0.tex_coord + bc_clip.y * v1.tex_coord + bc_clip.z * v2.tex_coord;

			data.lod = 0;
			if (st.need_tex_lod())
			{
				//dx and dy == 1
				glm::vec3 bc_screen2 = barycentric(win_c, glm::vec2(P) + glm::vec2(1.5f));
				//affine
				//glm::vec2 tex_coord2 = bc_screen2.x * v0.tex_coord + bc_screen2.y * v1.tex_coord + bc_screen2.z * v2.tex_coord;
				//perspective
				glm::vec3 bc_clip2{ bc_screen2.x / v0.clip.w, bc_screen2.y / v1.clip.w, bc_screen2.z / v2.clip.w };
				bc_clip2 = bc_clip2 / (bc_clip2.x + bc_clip2.y + bc_clip2.z);
				glm::vec2 tex_coord2 = bc_clip2.x * v0.tex_coord + bc_clip2.y * v1.tex_coord + bc_clip2.z * v2.tex_coord;

				glm::vec2 duv = (tex_coord2 - glm::vec2(data.tex_coord)) * glm::vec2(st.texture_2d.arrays[0].width, st.texture_2d.arrays[0].height);
				float scale_factor = glm::sqrt(duv.x * duv.x + duv.y * duv.y);
				data.lod = glm::log2(scale_factor);
			}

			if (st.fog_enabled)
				data.fog_z = abs(bc_screen.x * v0.position.z + bc_screen.y * v1.position.z + bc_screen.z * v2.position.z);

			if (st.depth_test && st.framebuffer->depth)
				data.z = abs(bc_screen.x * win_c[0].z + bc_screen.y * win_c[1].z + bc_screen.z * win_c[2].z);

			gl_emit_fragment(st, P.x, P.y, data);
		}
	}
}

void gl_emit_triangle(gl_state& st, gl_full_vertex &v0, gl_full_vertex&v1, gl_full_vertex&v2)
{
	if (v0.clip.z > v0.clip.w && v1.clip.z > v1.clip.w && v2.clip.z > v2.clip.w)
		return;
	if (v0.clip.z < -v0.clip.w && v1.clip.z < -v1.clip.w && v2.clip.z < -v2.clip.w)
		return;
	if (v0.clip.x < -v0.clip.w && v1.clip.x < -v1.clip.w && v2.clip.x < -v2.clip.w)
		return;
	if (v0.clip.x > v0.clip.w && v1.clip.x > v1.clip.w && v2.clip.x > v2.clip.w)
		return;
	if (v0.clip.y < -v0.clip.w && v1.clip.y < -v1.clip.w && v2.clip.y < -v2.clip.w)
		return;
	if (v0.clip.y > v0.clip.w && v1.clip.y > v1.clip.w && v2.clip.y > v2.clip.w)
		return;

	st.last_side = triangle_side(st, v0, v1, v2);

	if (st.light_model_two_side)
	{
		if (!st.shade_model_flat)
		{
			v0.color = st.get_vertex_color(v0.position, v0.original_color, v0.normal, !st.last_side);
			v1.color = st.get_vertex_color(v1.position, v1.original_color, v1.normal, !st.last_side);
		}
		v2.color = st.get_vertex_color(v2.position, v2.original_color, v2.normal, !st.last_side);
	}

	if (st.shade_model_flat)
	{
		v0.color = v2.color;
		v1.color = v2.color;
	}

	if (st.polygon_mode[st.last_side] == GL_POINT)
	{
		if (v0.edge) gl_emit_point(st, v0);
		if (v1.edge) gl_emit_point(st, v1);
		if (v2.edge) gl_emit_point(st, v2);
	}
	else
	{
		if (st.clip_point(v0.position, v0.clip) && st.clip_point(v1.position, v1.clip) && st.clip_point(v2.position, v2.clip))
			gl_rasterize_triangle(st, v0, v1, v2);
		else
			gl_rasterize_clipped_triangle(st, v0, v1, v2);
	}
}
