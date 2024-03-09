#include "pch.h"
#include "gl_state.h"
#include "gl_exports.h"

EXPORT void APIENTRY glNewList(GLuint list, GLenum mode) {}
EXPORT void APIENTRY glEndList(void) {}
EXPORT void APIENTRY glCallList(GLuint list) {}
EXPORT void APIENTRY glCallLists(GLsizei n, GLenum type, const void* lists) {}
EXPORT void APIENTRY glListBase(GLuint base) {}
EXPORT GLuint APIENTRY glGenLists(GLsizei range) { return 0; }
EXPORT GLboolean APIENTRY glIsList(GLuint list) { return false; }
EXPORT void APIENTRY glDeleteLists(GLuint list, GLsizei range) {}
