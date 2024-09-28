
#include "pch.h"
#include "gl_exports.h"
#include "gl_state.h"
#include "gl_pixels.h"

void APIENTRY glEdgeFlagPointer(GLsizei stride, const void *pointer)
{
	gl_state *gs = gl_current_state();
	if (!gs) return;
	VALIDATE_NOT_BEGIN_MODE;

	if (stride < 0)
	{
		gl_set_error(GL_INVALID_VALUE);
		return;
	}

	gs->va.edge_flag.stride = stride;
	gs->va.edge_flag.pointer = pointer;
}

void APIENTRY glTexCoordPointer(GLint size, GLenum type, GLsizei stride, const void *pointer)
{
	gl_state *gs = gl_current_state();
	if (!gs) return;
	VALIDATE_NOT_BEGIN_MODE;

	if (size < 1 || size > 4)
	{
		gl_set_error(GL_INVALID_VALUE);
		return;
	}
	if (type != GL_SHORT && type != GL_INT && type != GL_FLOAT && type != GL_DOUBLE)
	{
		gl_set_error_a(GL_INVALID_ENUM, type);
		return;
	}
	if (stride < 0)
	{
		gl_set_error(GL_INVALID_VALUE);
		return;
	}

	gs->va.tex_coord.size = size;
	gs->va.tex_coord.type = type;
	gs->va.tex_coord.stride = stride;
	gs->va.tex_coord.pointer = pointer;
}

void APIENTRY glColorPointer(GLint size, GLenum type, GLsizei stride, const void *pointer)
{
	gl_state *gs = gl_current_state();
	if (!gs) return;
	VALIDATE_NOT_BEGIN_MODE;

	if (size < 3 || size > 4)
	{
		gl_set_error(GL_INVALID_VALUE);
		return;
	}
	if ((type < GL_BYTE || type > GL_FLOAT) && type != GL_DOUBLE)
	{
		gl_set_error_a(GL_INVALID_ENUM, type);
		return;
	}
	if (stride < 0)
	{
		gl_set_error(GL_INVALID_VALUE);
		return;
	}

	gs->va.color.size = size;
	gs->va.color.type = type;
	gs->va.color.stride = stride;
	gs->va.color.pointer = pointer;
}

void APIENTRY glIndexPointer(GLenum type, GLsizei stride, const void *pointer)
{
	gl_state *gs = gl_current_state();
	if (!gs) return;
	VALIDATE_NOT_BEGIN_MODE;

	if (type != GL_UNSIGNED_BYTE && type != GL_SHORT && type != GL_INT && type != GL_FLOAT && type != GL_DOUBLE)
	{
		gl_set_error_a(GL_INVALID_ENUM, type);
		return;
	}
	if (stride < 0)
	{
		gl_set_error(GL_INVALID_VALUE);
		return;
	}

	gs->va.index.type = type;
	gs->va.index.stride = stride;
	gs->va.index.pointer = pointer;
}

void APIENTRY glNormalPointer(GLenum type, GLsizei stride, const void *pointer)
{
	gl_state *gs = gl_current_state();
	if (!gs) return;
	VALIDATE_NOT_BEGIN_MODE;

	if (type != GL_BYTE && type != GL_SHORT && type != GL_INT && type != GL_FLOAT && type != GL_DOUBLE)
	{
		gl_set_error_a(GL_INVALID_ENUM, type);
		return;
	}
	if (stride < 0)
	{
		gl_set_error(GL_INVALID_VALUE);
		return;
	}

	gs->va.normal.type = type;
	gs->va.normal.stride = stride;
	gs->va.normal.pointer = pointer;
}

void APIENTRY glVertexPointer(GLint size, GLenum type, GLsizei stride, const void *pointer)
{
	gl_state *gs = gl_current_state();
	if (!gs) return;
	VALIDATE_NOT_BEGIN_MODE;

	if (size < 2 || size > 4)
	{
		gl_set_error(GL_INVALID_VALUE);
		return;
	}
	if (type != GL_SHORT && type != GL_INT && type != GL_FLOAT && type != GL_DOUBLE)
	{
		gl_set_error_a(GL_INVALID_ENUM, type);
		return;
	}
	if (stride < 0)
	{
		gl_set_error(GL_INVALID_VALUE);
		return;
	}

	gs->va.vertex.size = size;
	gs->va.vertex.type = type;
	gs->va.vertex.stride = stride;
	gs->va.vertex.pointer = pointer;
}

static void set_bit(uint8_t &set, int bit, bool val)
{
	if (val)
		set |= (1 << bit);
	else
		set &= ~(1 << bit);
}

void APIENTRY glEnableClientState(GLenum array)
{
	gl_state *gs = gl_current_state();
	if (!gs) return;
	VALIDATE_NOT_BEGIN_MODE;

	if (array < GL_VERTEX_ARRAY || array > GL_EDGE_FLAG_ARRAY)
	{
		gl_set_error_a(GL_INVALID_ENUM, array);
		return;
	}

	set_bit(gs->va.enabled, array - GL_VERTEX_ARRAY, true);
}

void APIENTRY glDisableClientState(GLenum array)
{
	gl_state *gs = gl_current_state();
	if (!gs) return;
	VALIDATE_NOT_BEGIN_MODE;

	if (array < GL_VERTEX_ARRAY || array > GL_EDGE_FLAG_ARRAY)
	{
		gl_set_error_a(GL_INVALID_ENUM, array);
		return;
	}

	set_bit(gs->va.enabled, array - GL_VERTEX_ARRAY, false);
}

static void gl_elementArray(gl_state::vertex_array_t &va, int i)
{
	//GL_NORMAL_ARRAY
	if ((va.enabled & 2) && va.normal.pointer)
	{
		int stride = va.normal.stride;
		if (!stride)
			stride = 3 * gl_pixel_type_size(va.normal.type);
		const uint8_t *data = ((const uint8_t *)va.normal.pointer) + stride * i;

		switch (va.normal.type)
		{
		case GL_BYTE:glNormal3bv((const GLbyte*)data); break;
		case GL_SHORT:glNormal3sv((const GLshort *)data); break;
		case GL_INT:glNormal3iv((const GLint *)data); break; break;
		case GL_FLOAT:glNormal3fv((const GLfloat *)data); break; break;
		case GL_DOUBLE:glNormal3dv((const GLdouble *)data); break; break;
		}
	}
	//GL_COLOR_ARRAY
	if ((va.enabled & 4) && va.color.pointer)
	{
		int stride = va.color.stride;
		if (!stride)
			stride = va.color.size * gl_pixel_type_size(va.color.type);
		const uint8_t *data = ((const uint8_t *)va.color.pointer) + stride * i;
		switch (va.color.size)
		{
		case 3:
			switch (va.color.type){
			case GL_BYTE:glColor3bv((const GLbyte *)data); break;
			case GL_UNSIGNED_BYTE:glColor3ubv((const GLubyte *)data); break;
			case GL_SHORT:glColor3sv((const GLshort *)data); break;
			case GL_UNSIGNED_SHORT:glColor3usv((const GLushort *)data); break;
			case GL_INT:glColor3iv((const GLint *)data); break; break;
			case GL_UNSIGNED_INT:glColor3uiv((const GLuint *)data); break; break;
			case GL_FLOAT:glColor3fv((const GLfloat *)data); break; break;
			case GL_DOUBLE:glColor3dv((const GLdouble *)data); break; break;
			}break;
		case 4:
			switch (va.color.type) {
			case GL_BYTE:glColor4bv((const GLbyte *)data); break;
			case GL_UNSIGNED_BYTE:glColor4ubv((const GLubyte *)data); break;
			case GL_SHORT:glColor4sv((const GLshort *)data); break;
			case GL_UNSIGNED_SHORT:glColor4usv((const GLushort *)data); break;
			case GL_INT:glColor4iv((const GLint *)data); break; break;
			case GL_UNSIGNED_INT:glColor4uiv((const GLuint *)data); break; break;
			case GL_FLOAT:glColor4fv((const GLfloat *)data); break; break;
			case GL_DOUBLE:glColor4dv((const GLdouble *)data); break; break;
			}break;
		}
	}
	//GL_INDEX_ARRAY
	//index color mode unimplemented
	//GL_TEXTURE_COORD_ARRAY
	if ((va.enabled & 16) && va.tex_coord.pointer)
	{
		int stride = va.tex_coord.stride;
		if (!stride)
			stride = va.tex_coord.size * gl_pixel_type_size(va.tex_coord.type);
		const uint8_t *data = ((const uint8_t *)va.tex_coord.pointer) + stride * i;
		switch (va.tex_coord.type)
		{
		case GL_SHORT:
			switch (va.tex_coord.size) {
			case 1:glTexCoord1sv((const GLshort *)data); break;
			case 2:glTexCoord2sv((const GLshort *)data); break;
			case 3:glTexCoord3sv((const GLshort *)data); break;
			case 4:glTexCoord4sv((const GLshort *)data); break;
			}break;
		case GL_INT:
			switch (va.tex_coord.size) {
			case 1:glTexCoord1iv((const GLint *)data); break;
			case 2:glTexCoord2iv((const GLint *)data); break;
			case 3:glTexCoord3iv((const GLint *)data); break;
			case 4:glTexCoord4iv((const GLint *)data); break;
			}break;
		case GL_FLOAT:
			switch (va.tex_coord.size) {
			case 1:glTexCoord1fv((const GLfloat *)data); break;
			case 2:glTexCoord2fv((const GLfloat *)data); break;
			case 3:glTexCoord3fv((const GLfloat *)data); break;
			case 4:glTexCoord4fv((const GLfloat *)data); break;
			}break;
		case GL_DOUBLE:
			switch (va.tex_coord.size) {
			case 1:glTexCoord1dv((const GLdouble *)data); break;
			case 2:glTexCoord2dv((const GLdouble *)data); break;
			case 3:glTexCoord3dv((const GLdouble *)data); break;
			case 4:glTexCoord4dv((const GLdouble *)data); break;
			}break;
		}
	}
	//GL_EDGE_FLAG_ARRAY
	if ((va.enabled & 32) && va.edge_flag.pointer)
	{
		int stride = va.edge_flag.stride;
		if (!stride)
			stride = sizeof(GLboolean);
		const uint8_t *data = ((const uint8_t *)va.edge_flag.pointer) + stride * i;

		glEdgeFlagv((const GLboolean *)data);
	}
	//GL_VERTEX_ARRAY
	if ((va.enabled & 1) && va.vertex.pointer)
	{
		int stride = va.vertex.stride;
		if (!stride)
			stride = va.vertex.size * gl_pixel_type_size(va.vertex.type);
		const uint8_t *data = ((const uint8_t *)va.vertex.pointer) + stride * i;
		switch (va.vertex.type)
		{
		case GL_SHORT:
			switch (va.vertex.size){
			case 2:glVertex2sv((const GLshort *)data); break;
			case 3:glVertex3sv((const GLshort *)data); break;
			case 4:glVertex4sv((const GLshort *)data); break;
			}break;
		case GL_INT:
			switch (va.vertex.size){
			case 2:glVertex2iv((const GLint *)data); break;
			case 3:glVertex3iv((const GLint *)data); break;
			case 4:glVertex4iv((const GLint *)data); break;
			}break;
		case GL_FLOAT:
			switch (va.vertex.size){
			case 2:glVertex2fv((const GLfloat *)data); break;
			case 3:glVertex3fv((const GLfloat *)data); break;
			case 4:glVertex4fv((const GLfloat *)data); break;
			}break;
		case GL_DOUBLE:
			switch (va.vertex.size){
			case 2:glVertex2dv((const GLdouble *)data); break;
			case 3:glVertex3dv((const GLdouble *)data); break;
			case 4:glVertex4dv((const GLdouble *)data); break;
			}break;
		}
	}
}

void APIENTRY glArrayElement(GLint i)
{
	gl_state *gs = gl_current_state();
	if (!gs) return;
	gl_elementArray(gs->va, i);
}

void APIENTRY glDrawArrays(GLenum mode, GLint first, GLsizei count)
{
	gl_state *gs = gl_current_state();
	if (!gs) return;
	VALIDATE_NOT_BEGIN_MODE;

	if (mode < GL_POINTS || mode > GL_POLYGON)
	{
		gl_set_error_a(GL_INVALID_ENUM, mode);
		return;
	}
	if (count < 0)
	{
		gl_set_error(GL_INVALID_VALUE);
		return;
	}

	glBegin(mode);
	for (int i = 0; i < count; i++)
		glArrayElement(i);
	glEnd();
}

void APIENTRY glDrawElements(GLenum mode, GLsizei count, GLenum type, const void *indices)
{
	gl_state *gs = gl_current_state();
	if (!gs) return;
	VALIDATE_NOT_BEGIN_MODE;

	if (mode < GL_POINTS || mode > GL_POLYGON)
	{
		gl_set_error_a(GL_INVALID_ENUM, mode);
		return;
	}
	if (count < 0)
	{
		gl_set_error(GL_INVALID_VALUE);
		return;
	}
	if (type != GL_UNSIGNED_BYTE && type != GL_UNSIGNED_SHORT && type != GL_UNSIGNED_INT)
	{
		gl_set_error_a(GL_INVALID_ENUM, type);
		return;
	}

	glBegin(mode);
	if (type == GL_UNSIGNED_BYTE)
	{
		for (int i = 0; i < count; i++)
			glArrayElement(((const GLubyte*)indices)[i]);
	}
	else if (type == GL_UNSIGNED_SHORT)
	{
		for (int i = 0; i < count; i++)
			glArrayElement(((const GLshort *)indices)[i]);
	}
	else if (type == GL_UNSIGNED_INT)
	{
		for (int i = 0; i < count; i++)
			glArrayElement(((const GLint *)indices)[i]);
	}
	glEnd();
}

void APIENTRY glInterleavedArrays(GLenum format, GLsizei stride, const void *pointer)
{

}
