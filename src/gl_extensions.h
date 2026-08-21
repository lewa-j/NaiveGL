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

#if GL_EXT_subtexture
void APIENTRY glTexSubImage1DEXT(GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const void *pixels);
void APIENTRY glTexSubImage2DEXT(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void *pixels);
#if GL_EXT_texture3D
void APIENTRY glTexSubImage3DEXT(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void *pixels);
#endif
#endif

#if GL_EXT_copy_texture
void APIENTRY glCopyTexImage1DEXT(GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLint border);
void APIENTRY glCopyTexImage2DEXT(GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border);
#if GL_EXT_subtexture
void APIENTRY glCopyTexSubImage1DEXT(GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width);
void APIENTRY glCopyTexSubImage2DEXT(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height);
#if GL_EXT_texture3D
void APIENTRY glCopyTexSubImage3DEXT(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height);
#endif
#endif
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
