#pragma once
#include "gl_types.h"

#ifndef APIENTRY
#ifdef WIN32
#define APIENTRY __stdcall
#else
#define APIENTRY
#endif
#endif

#if GL_EXT_polygon_offset
void APIENTRY glPolygonOffsetEXT(GLfloat factor, GLfloat bias);
#endif

#if GL_EXT_vertex_array
void APIENTRY glArrayElementEXT(GLint i);
void APIENTRY glDrawArraysEXT(GLenum mode, GLint first, GLsizei count);
void APIENTRY glVertexPointerEXT(GLint size, GLenum type, GLsizei stride, GLsizei count, const void *pointer);
void APIENTRY glNormalPointerEXT(GLenum type, GLsizei stride, GLsizei count, const void *pointer);
void APIENTRY glColorPointerEXT(GLint size, GLenum type, GLsizei stride, GLsizei count, const void *pointer);
void APIENTRY glIndexPointerEXT(GLenum type, GLsizei stride, GLsizei count, const void *pointer);
void APIENTRY glTexCoordPointerEXT(GLint size, GLenum type, GLsizei stride, GLsizei count, const void *pointer);
void APIENTRY glEdgeFlagPointerEXT(GLsizei stride, GLsizei count, const void *pointer);
void APIENTRY glGetPointervEXT(GLenum pname, void **params);
#endif

#if GL_EXT_blend_logic_op
//originally from GL_EXT_blend_minmax but not required
void APIENTRY glBlendEquationEXT(GLenum mode);
#endif
