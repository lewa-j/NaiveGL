#include "pch.h"
#include "gl_state.h"
#include "gl_exports.h"
#include <glm/gtc/type_ptr.hpp>

constexpr int maps_k[9]{ 4,1,3,1,2,3,4,3,4 };

template<typename T>
static void gl_map1v(GLenum target, T u1, T u2, GLint stride, GLint order, const T *points)
{
	gl_state *gs = gl_current_state();
	if (!gs) return;
	VALIDATE_NOT_BEGIN_MODE;

	if (target < GL_MAP1_COLOR_4 || target > GL_MAP1_VERTEX_4)
	{
		gl_set_error_a(GL_INVALID_ENUM, target);
		return;
	}

	if (order < 1 || order > gl_max_eval_order)
	{
		gl_set_error_a(GL_INVALID_VALUE, order);
		return;
	}
	if (u1 == u2)
	{
		gl_set_error(GL_INVALID_VALUE);
		return;
	}

	int k = maps_k[target - GL_MAP1_COLOR_4];
	if (stride < k)
	{
		gl_set_error_a(GL_INVALID_VALUE, stride);
		return;
	}

	gl_state::mapSpec1D &map = gs->eval_maps_1d[target - GL_MAP1_COLOR_4];

	map.order_u = order;
	map.domain_u[0] = (float)u1;
	map.domain_u[1] = (float)u2;
	map.control_points.resize(order * k);

	for (int i = 0; i < order; i++)
	{
		for (int c = 0; c < k; c++)
		{
			map.control_points[i * k + c] = (float)points[i * stride + c];
		}
	}
}

template<typename T>
static void APIENTRY gl_map2v(GLenum target, T u1, T u2, GLint ustride, GLint uorder, T v1, T v2, GLint vstride, GLint vorder, const T *points)
{
	gl_state *gs = gl_current_state();
	if (!gs) return;
	VALIDATE_NOT_BEGIN_MODE;

	if (target < GL_MAP2_COLOR_4 || target > GL_MAP2_VERTEX_4)
	{
		gl_set_error_a(GL_INVALID_ENUM, target);
		return;
	}

	if (uorder < 1 || uorder > gl_max_eval_order)
	{
		gl_set_error_a(GL_INVALID_VALUE, uorder);
		return;
	}
	if (vorder < 1 || vorder > gl_max_eval_order)
	{
		gl_set_error_a(GL_INVALID_VALUE, vorder);
		return;
	}
	if (u1 == u2)
	{
		gl_set_error(GL_INVALID_VALUE);
		return;
	}
	if (v1 == v2)
	{
		gl_set_error(GL_INVALID_VALUE);
		return;
	}

	int k = maps_k[target - GL_MAP2_COLOR_4];
	if (ustride < k)
	{
		gl_set_error_a(GL_INVALID_VALUE, ustride);
		return;
	}
	if (vstride < k)
	{
		gl_set_error_a(GL_INVALID_VALUE, vstride);
		return;
	}

	gl_state::mapSpec2D &map = gs->eval_maps_2d[target - GL_MAP2_COLOR_4];

	map.order_u = uorder;
	map.order_v = vorder;
	map.domain_u[0] = (float)u1;
	map.domain_u[1] = (float)u2;
	map.domain_v[0] = (float)v1;
	map.domain_v[1] = (float)v2;
	map.control_points.resize(uorder * vorder * k);

	for (int i = 0; i < uorder; i++)
	{
		for (int j = 0; j < vorder; j++)
		{
			for (int c = 0; c < k; c++)
			{
				map.control_points[(i * vorder + j) * k + c] = (float)points[i * ustride + j * vstride + c];
			}
		}
	}
}

void APIENTRY glMap1f(GLenum target, GLfloat u1, GLfloat u2, GLint stride, GLint order, const GLfloat *points)
{ gl_map1v(target, u1, u2, stride, order, points); }
void APIENTRY glMap1d(GLenum target, GLdouble u1, GLdouble u2, GLint stride, GLint order, const GLdouble *points)
{ gl_map1v(target, u1, u2, stride, order, points); }

void APIENTRY glMap2f(GLenum target, GLfloat u1, GLfloat u2, GLint ustride, GLint uorder, GLfloat v1, GLfloat v2, GLint vstride, GLint vorder, const GLfloat *points)
{ gl_map2v(target, u1, u2, ustride, uorder, v1, v2, vstride, vorder, points); }
void APIENTRY glMap2d(GLenum target, GLdouble u1, GLdouble u2, GLint ustride, GLint uorder, GLdouble v1, GLdouble v2, GLint vstride, GLint vorder, const GLdouble *points)
{ gl_map2v(target, u1, u2, ustride, uorder, v1, v2, vstride, vorder, points); }

static int C(int n, int k)
{
	if (k == 0)
		return 1;
	int a = n - k + 1;
	int b;
	for (int i = 1; i < k; i++)
	{
		b = a * (n - k + 1 + i) / (i + 1);
		a = b;
	}
	return a;
}

static float B(int n, int i, float u)
{
	return C(n, i) * powf(u, (float)i) * powf(1 - u, (float)n - i);
}

template<int size>
static glm::vec<size, float> evaluate1d(const gl_state::mapSpec1D &map, float in_u)
{
	float u = (in_u - map.domain_u[0]) / (map.domain_u[1] - map.domain_u[0]);
	glm::vec<size, float> p(0);
	for (int i = 0; i < map.order_u; i++)
	{
		glm::vec<size, float> point;
		memcpy(&point.x, &map.control_points[i * size], sizeof(point));
		p += B(map.order_u - 1, i, u) * point;
	}
	return p;
}

static int map1_index(int name)
{
	return name - GL_MAP1_COLOR_4;
}

void APIENTRY glEvalCoord1f(GLfloat u)
{
	gl_state *gs = gl_current_state();
	if (!gs) return;

	if (gs->begin_primitive_mode == -1)
	{
		//undefined behaviour
		return;
	}

	if (!(gs->enabled_eval_maps & 0x180)) // GL_MAP1_VERTEX_3 and GL_MAP1_VERTEX_4 disabled
		return;

	glm::vec4 color = gs->current_color;
	glm::vec4 tex_coord = gs->current_tex_coord;
	glm::vec3 normal = gs->current_normal;

	if (gs->enabled_eval_maps & (1 << map1_index(GL_MAP1_COLOR_4)))
		color = evaluate1d<4>(gs->eval_maps_1d[map1_index(GL_MAP1_COLOR_4)], u);

	// color index mode
	// GL_MAP1_INDEX

	if (gs->enabled_eval_maps & (1 << map1_index(GL_MAP1_TEXTURE_COORD_4)))
		tex_coord = evaluate1d<4>(gs->eval_maps_1d[map1_index(GL_MAP1_TEXTURE_COORD_4)], u);
	else if (gs->enabled_eval_maps & (1 << map1_index(GL_MAP1_TEXTURE_COORD_3)))
		tex_coord = glm::make_vec4(evaluate1d<3>(gs->eval_maps_1d[map1_index(GL_MAP1_TEXTURE_COORD_3)], u));
	else if (gs->enabled_eval_maps & (1 << map1_index(GL_MAP1_TEXTURE_COORD_2)))
		tex_coord = glm::make_vec4(evaluate1d<2>(gs->eval_maps_1d[map1_index(GL_MAP1_TEXTURE_COORD_2)], u));
	else if (gs->enabled_eval_maps & (1 << map1_index(GL_MAP1_TEXTURE_COORD_1)))
		tex_coord = glm::make_vec4(evaluate1d<1>(gs->eval_maps_1d[map1_index(GL_MAP1_TEXTURE_COORD_1)], u));

	if (gs->enabled_eval_maps & (1 << map1_index(GL_MAP1_NORMAL)))
		normal = evaluate1d<3>(gs->eval_maps_1d[map1_index(GL_MAP1_NORMAL)], u);

	if (gs->enabled_eval_maps & (1 << map1_index(GL_MAP1_VERTEX_4)))
	{
		glm::vec4 p = evaluate1d<4>(gs->eval_maps_1d[map1_index(GL_MAP1_VERTEX_4)], u);
		gl_emit_vertex(gs, p, color, tex_coord, normal);
	}
	else if (gs->enabled_eval_maps & (1 << map1_index(GL_MAP1_VERTEX_3)))
	{
		glm::vec3 p = evaluate1d<3>(gs->eval_maps_1d[map1_index(GL_MAP1_VERTEX_3)], u);
		gl_emit_vertex(gs, glm::make_vec4(p), color, tex_coord, normal);
	}
}

void APIENTRY glEvalCoord1d(GLdouble u)
{ glEvalCoord1f((float)u); }
void APIENTRY glEvalCoord1fv(const GLfloat *u)
{ glEvalCoord1f(*u); }
void APIENTRY glEvalCoord1dv(const GLdouble *u)
{ glEvalCoord1f((float)*u); }

template<int size>
static glm::vec<size, float> evaluate2d(const gl_state::mapSpec2D &map, float in_u, float in_v)
{
	float u = (in_u - map.domain_u[0]) / (map.domain_u[1] - map.domain_u[0]);
	float v = (in_v - map.domain_v[0]) / (map.domain_v[1] - map.domain_v[0]);
	glm::vec<size, float> p(0);
	for (int i = 0; i < map.order_u; i++)
	{
		for (int j = 0; j < map.order_v; j++)
		{
			glm::vec<size, float> point;
			memcpy(&point.x, &map.control_points[(i * map.order_v + j) * size], sizeof(point));
			p += B(map.order_u - 1, i, u) * B(map.order_v - 1, j, v) * point;
		}
	}
	return p;
}

static int map2_index(int name)
{
	return name - GL_MAP2_COLOR_4;
}

void APIENTRY glEvalCoord2f(GLfloat u, GLfloat v)
{
	gl_state *gs = gl_current_state();
	if (!gs) return;

	if (gs->begin_primitive_mode == -1)
	{
		//undefined behaviour
		return;
	}

	if (!(gs->enabled_eval_maps & 0x30000)) // GL_MAP2_VERTEX_3 and GL_MAP2_VERTEX_4 disabled
		return;

	glm::vec4 color = gs->current_color;
	glm::vec4 tex_coord = gs->current_tex_coord;
	glm::vec3 normal = gs->current_normal;

	if (gs->enabled_eval_maps & 1 << (map2_index(GL_MAP2_COLOR_4) + 9))
		color = evaluate2d<4>(gs->eval_maps_2d[map2_index(GL_MAP2_COLOR_4)], u, v);

	// color index mode
	// GL_MAP2_INDEX

	if (gs->enabled_eval_maps & 1 << (map2_index(GL_MAP2_TEXTURE_COORD_4) + 9))
		tex_coord = evaluate2d<4>(gs->eval_maps_2d[map2_index(GL_MAP2_TEXTURE_COORD_4)], u, v);
	else if (gs->enabled_eval_maps & 1 << (map2_index(GL_MAP2_TEXTURE_COORD_3) + 9))
		tex_coord = glm::make_vec4(evaluate2d<3>(gs->eval_maps_2d[map2_index(GL_MAP2_TEXTURE_COORD_3)], u, v));
	else if (gs->enabled_eval_maps & 1 << (map2_index(GL_MAP2_TEXTURE_COORD_2) + 9))
		tex_coord = glm::make_vec4(evaluate2d<2>(gs->eval_maps_2d[map2_index(GL_MAP2_TEXTURE_COORD_2)], u, v));
	else if (gs->enabled_eval_maps & 1 << (map2_index(GL_MAP2_TEXTURE_COORD_1) + 9))
		tex_coord = glm::make_vec4(evaluate2d<1>(gs->eval_maps_2d[map2_index(GL_MAP2_TEXTURE_COORD_1)], u, v));

	if (!gs->eval_auto_normal && gs->enabled_eval_maps & 1 << (map2_index(GL_MAP2_NORMAL) + 9))
		normal = evaluate2d<3>(gs->eval_maps_2d[map2_index(GL_MAP2_NORMAL)], u, v);

	if (gs->enabled_eval_maps & 1 << (map2_index(GL_MAP2_VERTEX_4) + 9))
	{
		glm::vec4 p = evaluate2d<4>(gs->eval_maps_2d[map2_index(GL_MAP2_VERTEX_4)], u, v);
		if (gs->eval_auto_normal)
		{
			float du = 0.001f;
			float dv = 0.001f;
			glm::vec4 p1 = evaluate2d<4>(gs->eval_maps_2d[map2_index(GL_MAP2_VERTEX_4)], u + du, v);
			glm::vec4 p2 = evaluate2d<4>(gs->eval_maps_2d[map2_index(GL_MAP2_VERTEX_4)], u, v + dv);
			p1 /= p1.w;
			p2 /= p1.w;
			glm::vec3 q = p / p.w;
			normal = glm::cross((glm::vec3(p1) - q) / du, (glm::vec3(p2) - q) / dv);
			normal = normalize(normal);
		}
		gl_emit_vertex(gs, p, color, tex_coord, normal);
	}
	else if (gs->enabled_eval_maps & 1 << (map2_index(GL_MAP2_VERTEX_3) + 9))
	{
		glm::vec3 p = evaluate2d<3>(gs->eval_maps_2d[map2_index(GL_MAP2_VERTEX_3)], u, v);
		if (gs->eval_auto_normal)
		{
			float du = 0.001f;
			float dv = 0.001f;
			glm::vec3 p1 = evaluate2d<3>(gs->eval_maps_2d[map2_index(GL_MAP2_VERTEX_3)], u + du, v);
			glm::vec3 p2 = evaluate2d<3>(gs->eval_maps_2d[map2_index(GL_MAP2_VERTEX_3)], u, v + dv);
			normal = glm::cross((p1 - p) / du, (p2 - p) / dv);
			normal = normalize(normal);
		}

		gl_emit_vertex(gs, glm::make_vec4(p), color, tex_coord, normal);
	}
}
void APIENTRY glEvalCoord2d(GLdouble u, GLdouble v)
{ glEvalCoord2f((float)u, (float)v); }
void APIENTRY glEvalCoord2fv(const GLfloat *u) 
{ glEvalCoord2f(u[0], u[1]); }
void APIENTRY glEvalCoord2dv(const GLdouble *u)
{ glEvalCoord2f((float)u[0], (float)u[1]); }

void APIENTRY glMapGrid1f(GLint un, GLfloat u1, GLfloat u2)
{
	gl_state *gs = gl_current_state();
	if (!gs) return;
	VALIDATE_NOT_BEGIN_MODE;
	if (un <= 0)
	{
		gl_set_error_a(GL_INVALID_VALUE, un);
		return;
	}

	gs->eval_1d_grid_segments = un;
	gs->eval_1d_grid_domain[0] = u1;
	gs->eval_1d_grid_domain[1] = u2;
}
void APIENTRY glMapGrid1d(GLint un, GLdouble u1, GLdouble u2)
{ glMapGrid1f(un, (float)u1, (float)u2); }

void APIENTRY glMapGrid2f(GLint un, GLfloat u1, GLfloat u2, GLint vn, GLfloat v1, GLfloat v2)
{
	gl_state *gs = gl_current_state();
	if (!gs) return;
	VALIDATE_NOT_BEGIN_MODE;
	if (un <= 0)
	{
		gl_set_error_a(GL_INVALID_VALUE, un);
		return;
	}
	if (vn <= 0)
	{
		gl_set_error_a(GL_INVALID_VALUE, vn);
		return;
	}

	gs->eval_2d_grid_segments[0] = un;
	gs->eval_2d_grid_domain_u[0] = u1;
	gs->eval_2d_grid_domain_u[1] = u2;
	gs->eval_2d_grid_segments[1] = vn;
	gs->eval_2d_grid_domain_v[0] = v1;
	gs->eval_2d_grid_domain_v[1] = v2;
}
void APIENTRY glMapGrid2d(GLint un, GLdouble u1, GLdouble u2, GLint vn, GLdouble v1, GLdouble v2)
{ glMapGrid2f(un, (float)u1, (float)u2, vn, (float)v1, (float)v2); }

static void gl_evalPoint1(gl_state *gs, float du, GLint p)
{
	if (p == 0)
		glEvalCoord1f(gs->eval_1d_grid_domain[0]);
	else if (p == gs->eval_1d_grid_segments)
		glEvalCoord1f(gs->eval_1d_grid_domain[1]);
	else
		glEvalCoord1f(p * du + gs->eval_1d_grid_domain[0]);
}

void APIENTRY glEvalMesh1(GLenum mode, GLint p1, GLint p2)
{
	gl_state *gs = gl_current_state();
	if (!gs) return;
	VALIDATE_NOT_BEGIN_MODE;

	if (mode != GL_POINT && mode != GL_LINE)
	{
		gl_set_error_a(GL_INVALID_ENUM, mode);
		return;
	}

	float du = (gs->eval_1d_grid_domain[1] - gs->eval_1d_grid_domain[0]) / gs->eval_1d_grid_segments;

	glBegin((mode == GL_POINT) ? GL_POINTS : GL_LINE_STRIP);
	for (int i = p1; i <= p2; i++)
	{
		gl_evalPoint1(gs, du, i);
	}
	glEnd();
}

static void gl_evalPoint2(gl_state *gs, float du, float dv, GLint p, GLint q)
{
	float u;
	float v;
	if (p == 0)
		u = gs->eval_2d_grid_domain_u[0];
	else if (p == gs->eval_2d_grid_segments[0])
		u = gs->eval_2d_grid_domain_u[1];
	else
		u = p * du + gs->eval_2d_grid_domain_u[0];

	if (q == 0)
		v = gs->eval_2d_grid_domain_v[0];
	else if (q == gs->eval_2d_grid_segments[1])
		v = gs->eval_2d_grid_domain_v[1];
	else
		v = q * dv + gs->eval_2d_grid_domain_v[0];

	glEvalCoord2f(u, v);
}

void APIENTRY glEvalMesh2(GLenum mode, GLint p1, GLint p2, GLint q1, GLint q2)
{
	gl_state *gs = gl_current_state();
	if (!gs) return;
	VALIDATE_NOT_BEGIN_MODE;

	if (mode != GL_POINT && mode != GL_LINE && mode != GL_FILL)
	{
		gl_set_error_a(GL_INVALID_ENUM, mode);
		return;
	}

	float du = (gs->eval_2d_grid_domain_u[1] - gs->eval_2d_grid_domain_u[0]) / gs->eval_2d_grid_segments[0];
	float dv = (gs->eval_2d_grid_domain_v[1] - gs->eval_2d_grid_domain_v[0]) / gs->eval_2d_grid_segments[1];
	if (mode == GL_FILL)
	{
		for (int i = q1; i < q2; i++)
		{
			glBegin(GL_QUAD_STRIP);
			for (int j = p1; i <= p2; i++)
			{
				gl_evalPoint2(gs, du, dv, j, i);
				gl_evalPoint2(gs, du, dv, j, (i + 1));
			}
			glEnd();
		}
	}
	else if (mode == GL_LINE)
	{
		for (int i = q1; i <= q2; i++)
		{
			glBegin(GL_LINE_STRIP);
			for (int j = p1; i <= p2; i++)
			{
				gl_evalPoint2(gs, du, dv, j, i);
			}
			glEnd();
		}
		for (int i = p1; i <= p2; i++)
		{
			glBegin(GL_LINE_STRIP);
			for (int j = q1; i <= q2; i++)
			{
				gl_evalPoint2(gs, du, dv, i, j);
			}
			glEnd();
		}
	}
	else if (mode == GL_POINT)
	{
		glBegin(GL_POINTS);
		for (int i = q1; i <= q2; i++)
		{
			for (int j = p1; i <= p2; i++)
			{
				gl_evalPoint2(gs, du, dv, j, i);
			}
		}
		glEnd();
	}
}

void APIENTRY glEvalPoint1(GLint p)
{
	gl_state *gs = gl_current_state();
	if (!gs) return;

	float du = (gs->eval_1d_grid_domain[1] - gs->eval_1d_grid_domain[0]) / gs->eval_1d_grid_segments;
	gl_evalPoint1(gs, du, p);
}

void APIENTRY glEvalPoint2(GLint p, GLint q)
{
	gl_state *gs = gl_current_state();
	if (!gs) return;

	float du = (gs->eval_2d_grid_domain_u[1] - gs->eval_2d_grid_domain_u[0]) / gs->eval_2d_grid_segments[0];
	float dv = (gs->eval_2d_grid_domain_v[1] - gs->eval_2d_grid_domain_v[0]) / gs->eval_2d_grid_segments[1];
	gl_evalPoint2(gs, du, dv, p, q);
}
