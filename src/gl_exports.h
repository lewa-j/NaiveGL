#pragma once
#include <stdint.h>

#define EXPORT __declspec(dllexport) extern
#define APIENTRY __stdcall

typedef unsigned int GLenum;
typedef unsigned char GLboolean;
typedef unsigned int GLbitfield;
typedef int8_t GLbyte;
typedef uint8_t GLubyte;
typedef int16_t GLshort;
typedef uint16_t GLushort;
typedef int GLint;
typedef unsigned int GLuint;
typedef int GLsizei;
typedef float GLfloat;
typedef double GLdouble;

#ifdef __cplusplus
extern "C" {
#endif

EXPORT void APIENTRY glEnable(GLenum cap);
EXPORT void APIENTRY glDisable(GLenum cap);
EXPORT void APIENTRY glBegin(GLenum mode);
EXPORT void APIENTRY glEnd();
EXPORT void APIENTRY glVertex2f(GLfloat x, GLfloat y);
EXPORT void APIENTRY glVertex3f(GLfloat x, GLfloat y, GLfloat z);
EXPORT void APIENTRY glVertex3fv(const GLfloat *v);
EXPORT void APIENTRY glTexCoord2f(GLfloat s, GLfloat t);
EXPORT void APIENTRY glColor3f(GLfloat red, GLfloat green, GLfloat blue);
EXPORT void APIENTRY glColor3ubv(const GLubyte *v);
EXPORT void APIENTRY glColor4f(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
EXPORT void APIENTRY glColor4fv(const GLfloat *v);
EXPORT void APIENTRY glDepthRange(GLdouble n, GLdouble f);
EXPORT void APIENTRY glViewport(GLint x, GLint y, GLsizei width, GLsizei height);
EXPORT void APIENTRY glMatrixMode(GLenum mode);
EXPORT void APIENTRY glLoadMatrixf(const GLfloat *m);
EXPORT void APIENTRY glLoadIdentity(void);
EXPORT void APIENTRY glRotatef(GLfloat angle, GLfloat x, GLfloat y, GLfloat z);
EXPORT void APIENTRY glTranslatef(GLfloat x, GLfloat y, GLfloat z);
EXPORT void APIENTRY glScalef(GLfloat x, GLfloat y, GLfloat z);
EXPORT void APIENTRY glFrustum(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar);
EXPORT void APIENTRY glOrtho(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar);
EXPORT void APIENTRY glPushMatrix();
EXPORT void APIENTRY glPopMatrix();
EXPORT void APIENTRY glShadeModel(GLenum mode);
EXPORT void APIENTRY glCullFace(GLenum mode);
EXPORT void APIENTRY glPolygonMode(GLenum face, GLenum mode);
EXPORT void APIENTRY glTexImage2D(GLenum target, GLint level, GLint components, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void *pixels);
EXPORT void APIENTRY glTexParameterf(GLenum target, GLenum pname, GLfloat param);
EXPORT void APIENTRY glTexEnvf(GLenum target, GLenum pname, GLfloat param);
EXPORT void APIENTRY glAlphaFunc(GLenum func, GLfloat ref);
EXPORT void APIENTRY glDepthFunc(GLenum func);
EXPORT void APIENTRY glBlendFunc(GLenum sfactor, GLenum dfactor);
EXPORT void APIENTRY glDrawBuffer(GLenum buf);
EXPORT void APIENTRY glReadBuffer(GLenum src);
EXPORT void APIENTRY glDepthMask(GLboolean flag);
EXPORT void APIENTRY glClear(GLbitfield mask);
EXPORT void APIENTRY glClearColor(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
EXPORT void APIENTRY glReadPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, void *pixels);
EXPORT void APIENTRY glGetFloatv(GLenum pname, GLfloat *data);
EXPORT void APIENTRY glGetIntegerv(GLenum pname, GLint *data);
EXPORT const char *APIENTRY glGetString(GLenum name);
EXPORT void APIENTRY glFinish(void);
EXPORT void APIENTRY glHint(GLenum target, GLenum mode);

///!!!!!!!!!! gl 1.1
EXPORT void APIENTRY glTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void *pixels);
EXPORT void APIENTRY glBindTexture(GLenum target, GLuint texture);
//!!!!!!!!!!!!!!!!!!

#ifdef __cplusplus
}
#endif
