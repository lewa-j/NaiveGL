#include "pch.h"
#include "gl_state.h"
#include "gl_exports.h"

glm::vec3 gl_state::get_eye_normal()
{
	//TODO cache. May be used twice per vertex when spheremap and lighting are enabled
	glm::vec3 eye_normal = current_normal * glm::mat3(get_inv_modelview());

	if (normalize)
		eye_normal = glm::normalize(eye_normal);
	return eye_normal;
}

void APIENTRY glBegin(GLenum mode)
{
	gl_state *gs = gl_current_state();
	if (!gs) return;

	if (gs->begin_primitive_mode != -1)
	{
		gl_set_error(GL_INVALID_OPERATION);
		return;
	}
	if (mode < GL_POINTS || mode > GL_POLYGON)
	{
		gl_set_error_a(GL_INVALID_ENUM, mode);
		return;
	}

	gs->begin_primitive_mode = mode;
	gs->begin_vertex_count = 0;
	gs->line_stipple_counter = 0;
}

void APIENTRY glVertex4f(GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
	gl_state *gs = gl_current_state();
	if (!gs) return;

	if (gs->begin_primitive_mode == -1)
	{
		//undefined behaviour
		return;
	}

	bool front_face = true;//TODO

	glm::vec4 v_object(x, y, z, w);
	gl_processed_vertex vertex;
	vertex.position = gs->get_modelview() * v_object;
	vertex.tex_coord = gs->get_vertex_texcoord(v_object, vertex.position);
	vertex.color = gs->get_vertex_color(vertex.position, front_face);
	vertex.clip = gs->get_projection() * vertex.position;

	if (gs->begin_primitive_mode == GL_TRIANGLES || gs->begin_primitive_mode == GL_QUADS || gs->begin_primitive_mode == GL_POLYGON)
		vertex.edge = gs->edge_flag;
	else
		vertex.edge = true;

	switch (gs->begin_primitive_mode)
	{
	case GL_POINTS:
		gl_emit_point(*gs, vertex);
		break;
	case GL_LINES:
		if (gs->begin_vertex_count & 1)
		{
			gl_emit_line(*gs, gs->last_vertices[0], vertex);
			gs->line_stipple_counter = 0;
		}
		else
			gs->last_vertices[0] = vertex;
		break;
	case GL_LINE_LOOP:
		if (!gs->begin_vertex_count)
			gs->last_vertices[1] = vertex;// save first to loop back later
		[[fallthrough]];
	case GL_LINE_STRIP:
		if (gs->begin_vertex_count)
			gl_emit_line(*gs, gs->last_vertices[0], vertex);
		gs->last_vertices[0] = vertex;
		break;

	case GL_TRIANGLES:
		if (gs->begin_vertex_count < 2)
		{
			gs->last_vertices[gs->begin_vertex_count] = vertex;
		}
		else
		{
			gl_emit_triangle(*gs, gs->last_vertices[0], gs->last_vertices[1], vertex);
			gs->begin_vertex_count = -1;
		}
		break;
	case GL_TRIANGLE_STRIP:
		if (gs->begin_vertex_count >= 2)
			gl_emit_triangle(*gs, gs->last_vertices[0], gs->last_vertices[1], vertex);
		gs->last_vertices[gs->begin_vertex_count & 1] = vertex;
		break;
	case GL_POLYGON:
#if 0
		if (gs->polygon_mode[0] == GL_POINT)
		{
			if(gs->begin_vertex_count < 2)
				gs->last_vertices[gs->begin_vertex_count ? 1 : 0] = vertex;
			else if (gs->begin_vertex_count == 2)
			{
				gl_emit_point(*gs, gs->last_vertices[0]);
				gl_emit_point(*gs, gs->last_vertices[1]);
				gl_emit_point(*gs, vertex);
			}
			else if (gs->begin_vertex_count >= 2)
				gl_emit_point(*gs, vertex);
			break;
		}
		else if (gs->polygon_mode[0] == GL_LINE)
		{
			if (!gs->begin_vertex_count)
				gs->last_vertices[1] = vertex;// save first to loop back later
			else if (gs->begin_vertex_count == 2)
				gl_emit_line(*gs, gs->last_vertices[1], gs->last_vertices[0]);

			if (gs->begin_vertex_count >= 2)
				gl_emit_line(*gs, gs->last_vertices[0], vertex);
			gs->last_vertices[0] = vertex;
			break;
		}
#endif
		if (gs->begin_vertex_count >= 2)
		{
			vertex.color = gs->last_vertices[1].color;
			vertex.edge = false;
		}

		[[fallthrough]];
	case GL_TRIANGLE_FAN:
		if (gs->begin_vertex_count >= 2)
			gl_emit_triangle(*gs, gs->last_vertices[1], gs->last_vertices[0], vertex);
		if (gs->begin_primitive_mode == GL_POLYGON)
		{
			vertex.edge = gs->edge_flag;
			if (gs->begin_vertex_count == 2)
				gs->last_vertices[1].edge = false;
		}
		gs->last_vertices[gs->begin_vertex_count ? 0 : 1] = vertex;
		break;

	case GL_QUADS:
	{
		int j = gs->begin_vertex_count & 3;
		if (j < 3)
			gs->last_vertices[j] = vertex;
		else
			gl_emit_quad(*gs, gs->last_vertices[0], gs->last_vertices[1], gs->last_vertices[2], vertex);
	}
		break;
	case GL_QUAD_STRIP:
	{
		int j = gs->begin_vertex_count & 3;
		if (gs->begin_vertex_count >= 3)
		{
			if (j == 3)
				gl_emit_quad(*gs, gs->last_vertices[0], gs->last_vertices[1], vertex, gs->last_vertices[2]);
			else if (j == 1)
				gl_emit_quad(*gs, gs->last_vertices[2], gs->last_vertices[1], vertex, gs->last_vertices[0]);
		}

		gs->last_vertices[j < 3 ? j : 1] = vertex;
	}
		break;
	default:
		break;
	}

	gs->begin_vertex_count++;
}

void APIENTRY glEnd()
{
	gl_state *gs = gl_current_state();
	if (!gs) return;

	if (gs->begin_primitive_mode == -1)
	{
		gl_set_error(GL_INVALID_OPERATION);
		return;
	}

	if (gs->begin_primitive_mode == GL_LINE_LOOP && gs->begin_vertex_count >= 2)
		gl_emit_line(*gs, gs->last_vertices[0], gs->last_vertices[1]);// loop back from last to first
	else if (gs->begin_primitive_mode == GL_POLYGON && gs->begin_vertex_count >= 3 && gs->last_vertices[0].edge)
	{
		if (gs->polygon_mode[gs->last_side] == GL_LINE)
			gl_emit_line(*gs, gs->last_vertices[0], gs->last_vertices[1]);
		else if (gs->polygon_mode[gs->last_side] == GL_POINT)
			gl_emit_point(*gs, gs->last_vertices[0]);
	}

	gs->begin_primitive_mode = -1;
}

void APIENTRY glEdgeFlag(GLboolean flag)
{
	gl_state *gs = gl_current_state();
	if (!gs) return;
	gs->edge_flag = !!flag;
}
void APIENTRY glEdgeFlagv(GLboolean *flag) { glEdgeFlag(*flag); }

#define gv4f(x,y,z,w) glVertex4f((GLfloat)(x), (GLfloat)(y), (GLfloat)(z), (GLfloat)(w))

void APIENTRY glVertex4d(GLdouble x, GLdouble y, GLdouble z, GLdouble w)	{ gv4f(x, y, z, w); }
void APIENTRY glVertex4i(GLint x, GLint y, GLint z, GLint w)				{ gv4f(x, y, z, w); }
void APIENTRY glVertex4s(GLshort x, GLshort y, GLshort z, GLshort w)		{ gv4f(x, y, z, w); }
void APIENTRY glVertex4dv(const GLdouble* v)	{ gv4f(v[0], v[1], v[2], v[3]); }
void APIENTRY glVertex4fv(const GLfloat* v)		{ glVertex4f(v[0], v[1], v[2], v[3]); }
void APIENTRY glVertex4iv(const GLint* v)		{ gv4f(v[0], v[1], v[2], v[3]); }
void APIENTRY glVertex4sv(const GLshort* v)		{ gv4f(v[0], v[1], v[2], v[3]); }

void APIENTRY glVertex3d(GLdouble x, GLdouble y, GLdouble z)	{ gv4f(x, y, z, 1); }
void APIENTRY glVertex3f(GLfloat x, GLfloat y, GLfloat z)		{ glVertex4f(x, y, z, 1); }
void APIENTRY glVertex3i(GLint x, GLint y, GLint z)				{ gv4f(x, y, z, 1); }
void APIENTRY glVertex3s(GLshort x, GLshort y, GLshort z)		{ gv4f(x, y, z, 1); }
void APIENTRY glVertex3dv(const GLdouble* v)	{ gv4f(v[0], v[1], v[2], 1); }
void APIENTRY glVertex3fv(const GLfloat* v)		{ glVertex4f(v[0], v[1], v[2], 1); }
void APIENTRY glVertex3iv(const GLint* v)		{ gv4f(v[0], v[1], v[2], 1); }
void APIENTRY glVertex3sv(const GLshort* v)		{ gv4f(v[0], v[1], v[2], 1); }

void APIENTRY glVertex2d(GLdouble x, GLdouble y)	{ gv4f(x, y, 0, 1); }
void APIENTRY glVertex2f(GLfloat x, GLfloat y)		{ glVertex4f(x, y, 0, 1); }
void APIENTRY glVertex2i(GLint x, GLint y)			{ gv4f(x, y, 0, 1); }
void APIENTRY glVertex2s(GLshort x, GLshort y)		{ gv4f(x, y, 0, 1); }
void APIENTRY glVertex2dv(const GLdouble* v)		{ gv4f(v[0], v[1], 0, 1); }
void APIENTRY glVertex2fv(const GLfloat* v)			{ glVertex4f(v[0], v[1], 0, 1); }
void APIENTRY glVertex2iv(const GLint* v)			{ gv4f(v[0], v[1], 0, 1); }
void APIENTRY glVertex2sv(const GLshort* v)			{ gv4f(v[0], v[1], 0, 1); }
#undef gv4f

//======TexCoord=======

void APIENTRY glTexCoord4f(GLfloat s, GLfloat t, GLfloat r, GLfloat q)
{
	gl_state *gs = gl_current_state();
	if (!gs) return;

	gs->current_tex_coord = glm::vec4(s, t, r, q);
}

#define gtc4f(x,y,z,w) glTexCoord4f((GLfloat)(x), (GLfloat)(y), (GLfloat)(z), (GLfloat)(w))

void APIENTRY glTexCoord4d(GLdouble s, GLdouble t, GLdouble r, GLdouble q)	{ gtc4f(s, t, r, q); }
void APIENTRY glTexCoord4i(GLint s, GLint t, GLint r, GLint q)				{ gtc4f(s, t, r, q); }
void APIENTRY glTexCoord4s(GLshort s, GLshort t, GLshort r, GLshort q)		{ gtc4f(s, t, r, q); }
void APIENTRY glTexCoord4dv(const GLdouble* v)	{ gtc4f(v[0], v[1], v[2], v[3]); }
void APIENTRY glTexCoord4fv(const GLfloat* v)	{ glTexCoord4f(v[0], v[1], v[2], v[3]); }
void APIENTRY glTexCoord4iv(const GLint* v)		{ gtc4f(v[0], v[1], v[2], v[3]); }
void APIENTRY glTexCoord4sv(const GLshort* v)	{ gtc4f(v[0], v[1], v[2], v[3]); }

void APIENTRY glTexCoord1d(GLdouble s)	{ gtc4f(s, 0, 0, 1); }
void APIENTRY glTexCoord1f(GLfloat s)	{ glTexCoord4f(s, 0, 0, 1); }
void APIENTRY glTexCoord1s(GLshort s)	{ gtc4f(s, 0, 0, 1); }
void APIENTRY glTexCoord1i(GLint s)		{ gtc4f(s, 0, 0, 1); }
void APIENTRY glTexCoord1dv(const GLdouble* v)	{ gtc4f(v[0], 0, 0, 1); }
void APIENTRY glTexCoord1fv(const GLfloat* v)	{ glTexCoord4f(v[0], 0, 0, 1); }
void APIENTRY glTexCoord1iv(const GLint* v)		{ gtc4f(v[0], 0, 0, 1); }
void APIENTRY glTexCoord1sv(const GLshort* v)	{ gtc4f(v[0], 0, 0, 1); }

void APIENTRY glTexCoord2d(GLdouble s, GLdouble t)	{ gtc4f(s, t, 0, 1); }
void APIENTRY glTexCoord2f(GLfloat s, GLfloat t)	{ glTexCoord4f(s, t, 0, 1); }
void APIENTRY glTexCoord2i(GLint s, GLint t)		{ gtc4f(s, t, 0, 1); }
void APIENTRY glTexCoord2s(GLshort s, GLshort t)	{ gtc4f(s, t, 0, 1); }
void APIENTRY glTexCoord2dv(const GLdouble* v)	{ gtc4f(v[0], v[1], 0, 1); }
void APIENTRY glTexCoord2fv(const GLfloat* v)	{ glTexCoord4f(v[0], v[1], 0, 1); }
void APIENTRY glTexCoord2iv(const GLint* v)		{ gtc4f(v[0], v[1], 0, 1); }
void APIENTRY glTexCoord2sv(const GLshort* v)	{ gtc4f(v[0], v[1], 0, 1); }

void APIENTRY glTexCoord3d(GLdouble s, GLdouble t, GLdouble r)	{ gtc4f(s, t, r, 1); }
void APIENTRY glTexCoord3f(GLfloat s, GLfloat t, GLfloat r)		{ glTexCoord4f(s, t, r, 1); }
void APIENTRY glTexCoord3i(GLint s, GLint t, GLint r)			{ gtc4f(s, t, r, 1); }
void APIENTRY glTexCoord3s(GLshort s, GLshort t, GLshort r)		{ gtc4f(s, t, r, 1); }
void APIENTRY glTexCoord3dv(const GLdouble* v)	{ gtc4f(v[0], v[1], v[2], 1); }
void APIENTRY glTexCoord3fv(const GLfloat* v)	{ glTexCoord4f(v[0], v[1], v[2], 1); }
void APIENTRY glTexCoord3iv(const GLint* v)		{ gtc4f(v[0], v[1], v[2], 1); }
void APIENTRY glTexCoord3sv(const GLshort* v)	{ gtc4f(v[0], v[1], v[2], 1); }
#undef gtc4f

//=======Normal=======

void APIENTRY glNormal3f(GLfloat nx, GLfloat ny, GLfloat nz)
{
	gl_state *gs = gl_current_state();
	if (!gs) return;

	gs->current_normal = glm::vec3(nx, ny, nz);
}

void APIENTRY glNormal3b(GLbyte nx, GLbyte ny, GLbyte nz)			{ glNormal3f(GLtof(nx), GLtof(ny), GLtof(nz)); }
void APIENTRY glNormal3s(GLshort nx, GLshort ny, GLshort nz)		{ glNormal3f(GLtof(nx), GLtof(ny), GLtof(nz)); }
void APIENTRY glNormal3i(GLint nx, GLint ny, GLint nz)				{ glNormal3f(GLtof(nx), GLtof(ny), GLtof(nz)); }
void APIENTRY glNormal3d(GLdouble nx, GLdouble ny, GLdouble nz)		{ glNormal3f(GLtof(nx), GLtof(ny), GLtof(nz)); }
void APIENTRY glNormal3bv(const GLbyte* v)		{ glNormal3b(v[0], v[1], v[2]); }
void APIENTRY glNormal3sv(const GLshort* v)		{ glNormal3s(v[0], v[1], v[2]); }
void APIENTRY glNormal3iv(const GLint* v)		{ glNormal3i(v[0], v[1], v[2]); }
void APIENTRY glNormal3fv(const GLfloat* v)		{ glNormal3f(v[0], v[1], v[2]); }
void APIENTRY glNormal3dv(const GLdouble* v)	{ glNormal3d(v[0], v[1], v[2]); }

//==========Color=========

void APIENTRY glColor4f(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)
{
	gl_state *gs = gl_current_state();
	if (!gs) return;

	gs->current_color = glm::vec4(red, green, blue, alpha);

	if (gs->color_material)
		gs->update_color_material();
}

void APIENTRY glColor4b(GLbyte red, GLbyte green, GLbyte blue, GLbyte alpha)			{ glColor4f(GLtof(red), GLtof(green), GLtof(blue), GLtof(alpha)); }
void APIENTRY glColor4ub(GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha)		{ glColor4f(GLtof(red), GLtof(green), GLtof(blue), GLtof(alpha)); }
void APIENTRY glColor4s(GLshort red, GLshort green, GLshort blue, GLshort alpha)		{ glColor4f(GLtof(red), GLtof(green), GLtof(blue), GLtof(alpha)); }
void APIENTRY glColor4us(GLushort red, GLushort green, GLushort blue, GLushort alpha)	{ glColor4f(GLtof(red), GLtof(green), GLtof(blue), GLtof(alpha)); }
void APIENTRY glColor4i(GLint red, GLint green, GLint blue, GLint alpha)				{ glColor4f(GLtof(red), GLtof(green), GLtof(blue), GLtof(alpha)); }
void APIENTRY glColor4ui(GLuint red, GLuint green, GLuint blue, GLuint alpha)			{ glColor4f(GLtof(red), GLtof(green), GLtof(blue), GLtof(alpha)); }
void APIENTRY glColor4d(GLdouble red, GLdouble green, GLdouble blue, GLdouble alpha)	{ glColor4f(GLtof(red), GLtof(green), GLtof(blue), GLtof(alpha)); }
void APIENTRY glColor4bv(const GLbyte* v)	{ glColor4f(GLtof(v[0]), GLtof(v[1]), GLtof(v[2]), GLtof(v[3])); }
void APIENTRY glColor4ubv(const GLubyte* v) { glColor4f(GLtof(v[0]), GLtof(v[1]), GLtof(v[2]), GLtof(v[3])); }
void APIENTRY glColor4sv(const GLshort* v)	{ glColor4f(GLtof(v[0]), GLtof(v[1]), GLtof(v[2]), GLtof(v[3])); }
void APIENTRY glColor4usv(const GLushort* v){ glColor4f(GLtof(v[0]), GLtof(v[1]), GLtof(v[2]), GLtof(v[3])); }
void APIENTRY glColor4iv(const GLint* v)	{ glColor4f(GLtof(v[0]), GLtof(v[1]), GLtof(v[2]), GLtof(v[3])); }
void APIENTRY glColor4uiv(const GLuint* v)	{ glColor4f(GLtof(v[0]), GLtof(v[1]), GLtof(v[2]), GLtof(v[3])); }
void APIENTRY glColor4fv(const GLfloat* v)	{ glColor4f(v[0], v[1], v[2], v[3]); }
void APIENTRY glColor4dv(const GLdouble* v)	{ glColor4f(GLtof(v[0]), GLtof(v[1]), GLtof(v[2]), GLtof(v[3])); }

void APIENTRY glColor3b(GLbyte red, GLbyte green, GLbyte blue)			{ glColor4f(GLtof(red), GLtof(green), GLtof(blue), 1); }
void APIENTRY glColor3ub(GLubyte red, GLubyte green, GLubyte blue)		{ glColor4f(GLtof(red), GLtof(green), GLtof(blue), 1); }
void APIENTRY glColor3s(GLshort red, GLshort green, GLshort blue)		{ glColor4f(GLtof(red), GLtof(green), GLtof(blue), 1); }
void APIENTRY glColor3us(GLushort red, GLushort green, GLushort blue)	{ glColor4f(GLtof(red), GLtof(green), GLtof(blue), 1); }
void APIENTRY glColor3i(GLint red, GLint green, GLint blue)				{ glColor4f(GLtof(red), GLtof(green), GLtof(blue), 1); }
void APIENTRY glColor3ui(GLuint red, GLuint green, GLuint blue)			{ glColor4f(GLtof(red), GLtof(green), GLtof(blue), 1); }
void APIENTRY glColor3f(GLfloat red, GLfloat green, GLfloat blue)		{ glColor4f(red, green, blue, 1); }
void APIENTRY glColor3d(GLdouble red, GLdouble green, GLdouble blue)	{ glColor4f(GLtof(red), GLtof(green), GLtof(blue), 1); }
void APIENTRY glColor3bv(const GLbyte* v)	{ glColor4f(GLtof(v[0]), GLtof(v[1]), GLtof(v[2]), 1); }
void APIENTRY glColor3ubv(const GLubyte* v) { glColor4f(GLtof(v[0]), GLtof(v[1]), GLtof(v[2]), 1); }
void APIENTRY glColor3sv(const GLshort* v)	{ glColor4f(GLtof(v[0]), GLtof(v[1]), GLtof(v[2]), 1); }
void APIENTRY glColor3usv(const GLushort* v){ glColor4f(GLtof(v[0]), GLtof(v[1]), GLtof(v[2]), 1); }
void APIENTRY glColor3iv(const GLint* v)	{ glColor4f(GLtof(v[0]), GLtof(v[1]), GLtof(v[2]), 1); }
void APIENTRY glColor3uiv(const GLuint* v)	{ glColor4f(GLtof(v[0]), GLtof(v[1]), GLtof(v[2]), 1); }
void APIENTRY glColor3fv(const GLfloat* v)	{ glColor4f(v[0], v[1], v[2], 1); }
void APIENTRY glColor3dv(const GLdouble* v)	{ glColor4f(GLtof(v[0]), GLtof(v[1]), GLtof(v[2]), 1); }

void APIENTRY glRectf(GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2)
{
	glBegin(GL_POLYGON);
	glVertex2f(x1, y1);
	glVertex2f(x2, y1);
	glVertex2f(x2, y2);
	glVertex2f(x1, y2);
	glEnd();
}

#define grf(x1,y1,x2,y2) glRectf((GLfloat)(x1), (GLfloat)(y1), (GLfloat)(x2), (GLfloat)(y2))

void APIENTRY glRectd(GLdouble x1, GLdouble y1, GLdouble x2, GLdouble y2)	{ grf(x1, y1, x2, y2); }
void APIENTRY glRecti(GLint x1, GLint y1, GLint x2, GLint y2)				{ grf(x1, y1, x2, y2); }
void APIENTRY glRects(GLshort x1, GLshort y1, GLshort x2, GLshort y2)		{ grf(x1, y1, x2, y2); }
void APIENTRY glRectdv(const GLdouble* v1, const GLdouble* v2)		{ grf(v1[0], v1[1], v2[0], v2[1]); }
void APIENTRY glRectfv(const GLfloat* v1, const GLfloat* v2)		{ glRectf(v1[0], v1[1], v2[0], v2[1]); }
void APIENTRY glRectiv(const GLint* v1, const GLint* v2)			{ grf(v1[0], v1[1], v2[0], v2[1]); }
void APIENTRY glRectsv(const GLshort* v1, const GLshort* v2)		{ grf(v1[0], v1[1], v2[0], v2[1]); }
#undef grf
