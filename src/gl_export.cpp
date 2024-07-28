#include "pch.h"
#include "gl_exports.h"

EXPORT void APIENTRY glReadPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, void *pixels) {}
EXPORT void APIENTRY glGetFloatv(GLenum pname, GLfloat *data) {}
EXPORT void APIENTRY glGetIntegerv(GLenum pname, GLint *data) {}
EXPORT void APIENTRY glFinish(void) {}
EXPORT void APIENTRY glHint(GLenum target, GLenum mode) {}

//freeglut
EXPORT void APIENTRY glFlush(void) {}
EXPORT void APIENTRY glPopAttrib(void) {}
EXPORT void APIENTRY glPushAttrib(GLbitfield mask) {}
EXPORT void APIENTRY glGetBooleanv(GLenum pname, GLboolean *data) {}

///!!!!!!!!!! gl 1.1
EXPORT void APIENTRY glTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void *pixels) {}
EXPORT void APIENTRY glBindTexture(GLenum target, GLuint texture) {}
//!!!!!!!!!!!!!!!!!!
