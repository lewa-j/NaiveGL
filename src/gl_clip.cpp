#include "pch.h"
#include "gl_state.h"
#include "gl_exports.h"

bool gl_state::clip_point(const glm::vec4 &v_eye, const glm::vec4 &v_clip)
{
	if (v_clip.x < -v_clip.w || v_clip.x > v_clip.w ||
		v_clip.y < -v_clip.w || v_clip.y > v_clip.w ||
		v_clip.z < -v_clip.w || v_clip.z > v_clip.w)
		return false;

	if (!enabled_clipplanes)
		return true;

	for (int i = 0; i < gl_max_user_clip_planes; i++)
	{
		if (!(enabled_clipplanes & (1 << i)))
			continue;

		if (dot(clipplanes[i], v_eye) < 0)
			return false;
	}

	return true;
}

void APIENTRY glClipPlane(GLenum plane, const GLdouble *equation)
{
	gl_state *gs = gl_current_state();
	if (!gs) return;
	WRITE_DISPLAY_LIST_BULK(ClipPlane, equation, sizeof(double) * 4);
	VALIDATE_NOT_BEGIN_MODE;
	if (plane < GL_CLIP_PLANE0 || plane >= (GL_CLIP_PLANE0 + gl_max_user_clip_planes))
	{
		gl_set_error_a(GL_INVALID_ENUM, plane);
		return;
	}

	gs->clipplanes[plane - GL_CLIP_PLANE0] = glm::vec4(equation[0], equation[1], equation[2], equation[3]) * gs->get_inv_modelview();
}

static void interpolate(gl_processed_vertex& to, const gl_processed_vertex& v0, const gl_processed_vertex& v1, float t)
{
	to.position = glm::mix(v0.position, v1.position, t);
	to.clip = glm::mix(v0.clip, v1.clip, t);
	//Color and Texture Coordinate Clipping
	to.color = glm::mix(v0.color, v1.color, t);
	to.tex_coord = glm::mix(v0.tex_coord, v1.tex_coord, t);
	to.edge = v0.edge;
}

void gl_rasterize_clipped_line(gl_state& st, gl_processed_vertex v0, gl_processed_vertex v1)
{
	bool v0_in;
	bool v1_in;
#define clip_coord(c0,c1) \
	v0_in = c0 <= v0.clip.w;\
	v1_in = c1 <= v1.clip.w;\
	if (v0_in != v1_in)\
	{\
		float a = v0.clip.w - c0;\
		float t = a / (a - (v1.clip.w - c1));\
		interpolate(v0_in ? v1 : v0, v0, v1, t);\
	}\
	else if (!v0_in && !v1_in)\
		return;

	clip_coord(v0.clip.z, v1.clip.z);
	clip_coord(-v0.clip.z, -v1.clip.z);

	//optional
	clip_coord(v0.clip.x, v1.clip.x);
	clip_coord(-v0.clip.x, -v1.clip.x);

	clip_coord(v0.clip.y, v1.clip.y);
	clip_coord(-v0.clip.y, -v1.clip.y);
	//
#undef clip_coord

	if (st.enabled_clipplanes)
	{
		for (int i = 0; i < gl_max_user_clip_planes; i++)
		{
			if (!(st.enabled_clipplanes & (1 << i)))
				continue;

			float d0 = dot(st.clipplanes[i], v0.position);
			float d1 = dot(st.clipplanes[i], v1.position);

			v0_in = d0 >= 0;
			v1_in = d1 >= 0;
			if (!v0_in && !v1_in)
				return;
			if (v0_in && v1_in)
				continue;

			float t = d0 / (d0 - d1);
			interpolate(v0_in ? v1 : v0, v0, v1, t);
		}
	}

	gl_rasterize_line(st, v0, v1);
}

//based on McGuire2011Clipping

static int clip3(float d0, float d1, float d2, gl_processed_vertex& v0, gl_processed_vertex& v1, gl_processed_vertex& v2, gl_processed_vertex& v3)
{
	constexpr float eps = 0.00001f;
	constexpr float eps2 = 0.01f;

	if (!(d0 >= eps2 || d1 >= eps2 || d2 >= eps2))
		return 0;
	if (d0 >= -eps && d1 >= -eps && d2 >= -eps)
		return 3;

	bool v0_in = d0 >= 0;
	bool v1_in = d1 >= 0;
	bool v2_in = d2 >= 0;

	bool n_in;
	if (v1_in && !v0_in)
	{
		n_in = v2_in;
		v3 = v0; v0 = v1; v1 = v2; v2 = v3;
		float d3 = d0; d0 = d1; d1 = d2; d2 = d3;
	}
	else if (v2_in && !v1_in)
	{
		n_in = v0_in;
		v3 = v2; v2 = v1; v1 = v0; v0 = v3;
		float d3 = d2; d2 = d1; d1 = d0; d0 = d3;
	}
	else
		n_in = v1_in;

	float t3 = d0 / (d0 - d2);
	interpolate(v3, v0, v2, t3);
	v3.edge = v2.edge;

	if (n_in)
	{
		//quad
		float t2 = d1 / (d1 - d2);
		interpolate(v2, v1, v2, t2);
		return 4;
	}
	else
	{
		float t1 = d0 / (d0 - d1);
		interpolate(v1, v0, v1, t1);
		v1.edge = true;
		v2 = v3;
	}
	return 3;
}

static int clip3(const glm::vec4 &plane, gl_processed_vertex& v0, gl_processed_vertex& v1, gl_processed_vertex& v2, gl_processed_vertex& v3)
{
	float d0 = dot(plane, v0.position);
	float d1 = dot(plane, v1.position);
	float d2 = dot(plane, v2.position);
	return clip3(d0, d1, d2, v0, v1, v2, v3);
}

static void clip_triangle(gl_state& st, gl_processed_vertex v0, gl_processed_vertex v1, gl_processed_vertex v2, int step = 0)
{
	gl_processed_vertex v3;
	int r = 0;
#define clip_coord(c0,c1,c2) \
	{\
		step++;\
		r = clip3(v0.clip.w - c0, v1.clip.w - c1, v2.clip.w - c2, v0, v1, v2, v3);\
		if (!r)\
			return;\
		if (r == 4)\
		{\
			v2.edge = false;\
			clip_triangle(st, v0, v1, v2, step);\
			v0.edge = false;\
			v2.edge = true;\
			clip_triangle(st, v0, v2, v3, step);\
			return;\
		}\
	}

	if (step < 1)
		clip_coord(-v0.clip.z, -v1.clip.z, -v2.clip.z);
	if (step < 2)
		clip_coord(v0.clip.z, v1.clip.z, v2.clip.z);

	if (step < 3)
		clip_coord(-v0.clip.x, -v1.clip.x, -v2.clip.x);
	if (step < 4)
		clip_coord(v0.clip.x, v1.clip.x, v2.clip.x);

	if (step < 5)
		clip_coord(-v0.clip.y, -v1.clip.y, -v2.clip.y);
	if (step < 6)
		clip_coord(v0.clip.y, v1.clip.y, v2.clip.y);
#undef clip_coord

#if 1
	if (st.enabled_clipplanes)
	{
		for (int i = 0; i < gl_max_user_clip_planes; i++)
		{
			if (!(st.enabled_clipplanes & (1 << i)))
				continue;
			if (step >= 7 + i)
				continue;

			step++;
			r = clip3(st.clipplanes[i], v0, v1, v2, v3);
			if (!r)
				return;
			if (r == 4)
			{
				v2.edge = false;
				clip_triangle(st, v0, v1, v2, step);
				v0.edge = false;
				v2.edge = true;
				clip_triangle(st, v0, v2, v3, step);
				return;
			}
		}
	}
#endif
	gl_rasterize_triangle(st, v0, v1, v2);
}

void gl_rasterize_clipped_triangle(gl_state& st, const gl_processed_vertex& v0, const gl_processed_vertex& v1, const gl_processed_vertex& v2)
{
	clip_triangle(st, v0, v1, v2, 0);
}
