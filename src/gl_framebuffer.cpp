#include "pch.h"
#include "gl_state.h"
#include "gl_exports.h"

void APIENTRY glScissor(GLint x, GLint y, GLsizei width, GLsizei height) {}

void APIENTRY glDrawBuffer(GLenum buf) {}
void APIENTRY glReadBuffer(GLenum src) {}
void APIENTRY glColorMask(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha) {}
void APIENTRY glIndexMask(GLuint mask) {}
void APIENTRY glDepthMask(GLboolean flag) {}
void APIENTRY glStencilMask(GLuint mask) {}
void APIENTRY glClear(GLbitfield mask) {}
void APIENTRY glClearColor(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha) {}
void APIENTRY glClearIndex(GLfloat c) {}
void APIENTRY glClearDepth(GLdouble depth) {}
void APIENTRY glClearStencil(GLint s) {}
void APIENTRY glClearAccum(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha) {}
void APIENTRY glAccum(GLenum op, GLfloat value) {}
