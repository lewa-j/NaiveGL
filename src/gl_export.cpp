#include "pch.h"
#include "gl_exports.h"

EXPORT void APIENTRY glTexImage2D(GLenum target, GLint level, GLint components, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void *pixels) {}
EXPORT void APIENTRY glTexParameterf(GLenum target, GLenum pname, GLfloat param) {}
EXPORT void APIENTRY glTexEnvf(GLenum target, GLenum pname, GLfloat param) {}
EXPORT void APIENTRY glAlphaFunc(GLenum func, GLfloat ref) {}
EXPORT void APIENTRY glDepthFunc(GLenum func) {}
EXPORT void APIENTRY glBlendFunc(GLenum sfactor, GLenum dfactor) {}
EXPORT void APIENTRY glDrawBuffer(GLenum buf) {}
EXPORT void APIENTRY glReadBuffer(GLenum src) {}
EXPORT void APIENTRY glDepthMask(GLboolean flag) {}
EXPORT void APIENTRY glClear(GLbitfield mask) {}
EXPORT void APIENTRY glClearColor(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha) {}
EXPORT void APIENTRY glReadPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, void *pixels) {}
EXPORT void APIENTRY glGetFloatv(GLenum pname, GLfloat *data) {}
EXPORT void APIENTRY glGetIntegerv(GLenum pname, GLint *data) {}
EXPORT void APIENTRY glFinish(void) {}
EXPORT void APIENTRY glHint(GLenum target, GLenum mode) {}

///!!!!!!!!!! gl 1.1
EXPORT void APIENTRY glTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void *pixels) {}
EXPORT void APIENTRY glBindTexture(GLenum target, GLuint texture) {}
//!!!!!!!!!!!!!!!!!!

//freeglut
EXPORT void APIENTRY glBitmap(GLsizei width, GLsizei height, GLfloat xorig, GLfloat yorig, GLfloat xmove, GLfloat ymove, const GLubyte *bitmap) {}
EXPORT void APIENTRY glFlush(void) {}
EXPORT void APIENTRY glPopAttrib(void) {}
EXPORT void APIENTRY glPushAttrib(GLbitfield mask) {}
EXPORT void APIENTRY glGetBooleanv(GLenum pname, GLboolean *data) {}
EXPORT void APIENTRY glPixelStorei(GLenum pname, GLint param) {}
