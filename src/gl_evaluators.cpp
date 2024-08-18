#include "pch.h"
#include "gl_state.h"
#include "gl_exports.h"

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

void APIENTRY glEvalCoord1f(GLfloat u) {}
void APIENTRY glEvalCoord1d(GLdouble u) {}
void APIENTRY glEvalCoord1fv(const GLfloat *u) {}
void APIENTRY glEvalCoord1dv(const GLdouble *u) {}
void APIENTRY glEvalCoord2f(GLfloat u, GLfloat v) {}
void APIENTRY glEvalCoord2d(GLdouble u, GLdouble v) {}
void APIENTRY glEvalCoord2fv(const GLfloat *u) {}
void APIENTRY glEvalCoord2dv(const GLdouble *u) {}
void APIENTRY glMapGrid1f(GLint un, GLfloat u1, GLfloat u2) {}
void APIENTRY glMapGrid1d(GLint un, GLdouble u1, GLdouble u2) {}
void APIENTRY glMapGrid2f(GLint un, GLfloat u1, GLfloat u2, GLint vn, GLfloat v1, GLfloat v2) {}
void APIENTRY glMapGrid2d(GLint un, GLdouble u1, GLdouble u2, GLint vn, GLdouble v1, GLdouble v2) {}
void APIENTRY glEvalMesh1(GLenum mode, GLint p1, GLint p2) {}
void APIENTRY glEvalMesh2(GLenum mode, GLint p1, GLint p2, GLint q1, GLint q2) {}
void APIENTRY glEvalPoint1(GLint p) {}
void APIENTRY glEvalPoint2(GLint p, GLint q) {}
