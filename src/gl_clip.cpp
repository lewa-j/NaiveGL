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
	to.color = glm::mix(v0.color, v1.color, t);
	to.tex_coord = glm::mix(v0.tex_coord, v1.tex_coord, t);
	to.edge = v0.edge;
}

void rasterize_clipped_line(gl_state& st, gl_processed_vertex v0, gl_processed_vertex v1)
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

	rasterize_line(st, v0, v1);
}
