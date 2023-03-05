#include "pch.h"
#include "gl_exports.h"

EXPORT void APIENTRY glEnable(GLenum cap) {}
EXPORT void APIENTRY glDisable(GLenum cap) {}
EXPORT void APIENTRY glShadeModel(GLenum mode) {}
EXPORT void APIENTRY glCullFace(GLenum mode) {}
EXPORT void APIENTRY glPolygonMode(GLenum face, GLenum mode) {}
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
