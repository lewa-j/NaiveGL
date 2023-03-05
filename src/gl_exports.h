#pragma once
#include "gl_types.h"
#include <stdint.h>

#define EXPORT __declspec(dllexport) extern
#define APIENTRY __stdcall

#ifdef __cplusplus
extern "C" {
#endif

EXPORT GLenum APIENTRY glGetError();

EXPORT void APIENTRY glBegin(GLenum mode);
EXPORT void APIENTRY glEnd();

EXPORT void APIENTRY glEdgeFlag(GLboolean flag);
EXPORT void APIENTRY glEdgeFlagv(GLboolean *flag);

EXPORT void APIENTRY glVertex2d(GLdouble x, GLdouble y);
EXPORT void APIENTRY glVertex2dv(const GLdouble *v);
EXPORT void APIENTRY glVertex2f(GLfloat x, GLfloat y);
EXPORT void APIENTRY glVertex2fv(const GLfloat *v);
EXPORT void APIENTRY glVertex2i(GLint x, GLint y);
EXPORT void APIENTRY glVertex2iv(const GLint *v);
EXPORT void APIENTRY glVertex2s(GLshort x, GLshort y);
EXPORT void APIENTRY glVertex2sv(const GLshort *v);
EXPORT void APIENTRY glVertex3d(GLdouble x, GLdouble y, GLdouble z);
EXPORT void APIENTRY glVertex3dv(const GLdouble *v);
EXPORT void APIENTRY glVertex3f(GLfloat x, GLfloat y, GLfloat z);
EXPORT void APIENTRY glVertex3fv(const GLfloat *v);
EXPORT void APIENTRY glVertex3i(GLint x, GLint y, GLint z);
EXPORT void APIENTRY glVertex3iv(const GLint *v);
EXPORT void APIENTRY glVertex3s(GLshort x, GLshort y, GLshort z);
EXPORT void APIENTRY glVertex3sv(const GLshort *v);
EXPORT void APIENTRY glVertex4d(GLdouble x, GLdouble y, GLdouble z, GLdouble w);
EXPORT void APIENTRY glVertex4dv(const GLdouble *v);
EXPORT void APIENTRY glVertex4f(GLfloat x, GLfloat y, GLfloat z, GLfloat w);
EXPORT void APIENTRY glVertex4fv(const GLfloat *v);
EXPORT void APIENTRY glVertex4i(GLint x, GLint y, GLint z, GLint w);
EXPORT void APIENTRY glVertex4iv(const GLint *v);
EXPORT void APIENTRY glVertex4s(GLshort x, GLshort y, GLshort z, GLshort w);
EXPORT void APIENTRY glVertex4sv(const GLshort *v);

EXPORT void APIENTRY glTexCoord1d(GLdouble s);
EXPORT void APIENTRY glTexCoord1dv(const GLdouble *v);
EXPORT void APIENTRY glTexCoord1f(GLfloat s);
EXPORT void APIENTRY glTexCoord1fv(const GLfloat *v);
EXPORT void APIENTRY glTexCoord1i(GLint s);
EXPORT void APIENTRY glTexCoord1iv(const GLint *v);
EXPORT void APIENTRY glTexCoord1s(GLshort s);
EXPORT void APIENTRY glTexCoord1sv(const GLshort *v);
EXPORT void APIENTRY glTexCoord2d(GLdouble s, GLdouble t);
EXPORT void APIENTRY glTexCoord2dv(const GLdouble *v);
EXPORT void APIENTRY glTexCoord2f(GLfloat s, GLfloat t);
EXPORT void APIENTRY glTexCoord2fv(const GLfloat *v);
EXPORT void APIENTRY glTexCoord2i(GLint s, GLint t);
EXPORT void APIENTRY glTexCoord2iv(const GLint *v);
EXPORT void APIENTRY glTexCoord2s(GLshort s, GLshort t);
EXPORT void APIENTRY glTexCoord2sv(const GLshort *v);
EXPORT void APIENTRY glTexCoord3d(GLdouble s, GLdouble t, GLdouble r);
EXPORT void APIENTRY glTexCoord3dv(const GLdouble *v);
EXPORT void APIENTRY glTexCoord3f(GLfloat s, GLfloat t, GLfloat r);
EXPORT void APIENTRY glTexCoord3fv(const GLfloat *v);
EXPORT void APIENTRY glTexCoord3i(GLint s, GLint t, GLint r);
EXPORT void APIENTRY glTexCoord3iv(const GLint *v);
EXPORT void APIENTRY glTexCoord3s(GLshort s, GLshort t, GLshort r);
EXPORT void APIENTRY glTexCoord3sv(const GLshort *v);
EXPORT void APIENTRY glTexCoord4d(GLdouble s, GLdouble t, GLdouble r, GLdouble q);
EXPORT void APIENTRY glTexCoord4dv(const GLdouble *v);
EXPORT void APIENTRY glTexCoord4f(GLfloat s, GLfloat t, GLfloat r, GLfloat q);
EXPORT void APIENTRY glTexCoord4fv(const GLfloat *v);
EXPORT void APIENTRY glTexCoord4i(GLint s, GLint t, GLint r, GLint q);
EXPORT void APIENTRY glTexCoord4iv(const GLint *v);
EXPORT void APIENTRY glTexCoord4s(GLshort s, GLshort t, GLshort r, GLshort q);
EXPORT void APIENTRY glTexCoord4sv(const GLshort *v);

EXPORT void APIENTRY glNormal3b(GLbyte nx, GLbyte ny, GLbyte nz);
EXPORT void APIENTRY glNormal3bv(const GLbyte *v);
EXPORT void APIENTRY glNormal3d(GLdouble nx, GLdouble ny, GLdouble nz);
EXPORT void APIENTRY glNormal3dv(const GLdouble *v);
EXPORT void APIENTRY glNormal3f(GLfloat nx, GLfloat ny, GLfloat nz);
EXPORT void APIENTRY glNormal3fv(const GLfloat *v);
EXPORT void APIENTRY glNormal3i(GLint nx, GLint ny, GLint nz);
EXPORT void APIENTRY glNormal3iv(const GLint *v);
EXPORT void APIENTRY glNormal3s(GLshort nx, GLshort ny, GLshort nz);
EXPORT void APIENTRY glNormal3sv(const GLshort *v);

EXPORT void APIENTRY glColor3b(GLbyte red, GLbyte green, GLbyte blue);
EXPORT void APIENTRY glColor3bv(const GLbyte *v);
EXPORT void APIENTRY glColor3d(GLdouble red, GLdouble green, GLdouble blue);
EXPORT void APIENTRY glColor3dv(const GLdouble *v);
EXPORT void APIENTRY glColor3f(GLfloat red, GLfloat green, GLfloat blue);
EXPORT void APIENTRY glColor3fv(const GLfloat *v);
EXPORT void APIENTRY glColor3i(GLint red, GLint green, GLint blue);
EXPORT void APIENTRY glColor3iv(const GLint *v);
EXPORT void APIENTRY glColor3s(GLshort red, GLshort green, GLshort blue);
EXPORT void APIENTRY glColor3sv(const GLshort *v);
EXPORT void APIENTRY glColor3ub(GLubyte red, GLubyte green, GLubyte blue);
EXPORT void APIENTRY glColor3ubv(const GLubyte *v);
EXPORT void APIENTRY glColor3ui(GLuint red, GLuint green, GLuint blue);
EXPORT void APIENTRY glColor3uiv(const GLuint *v);
EXPORT void APIENTRY glColor3us(GLushort red, GLushort green, GLushort blue);
EXPORT void APIENTRY glColor3usv(const GLushort *v);
EXPORT void APIENTRY glColor4b(GLbyte red, GLbyte green, GLbyte blue, GLbyte alpha);
EXPORT void APIENTRY glColor4bv(const GLbyte *v);
EXPORT void APIENTRY glColor4d(GLdouble red, GLdouble green, GLdouble blue, GLdouble alpha);
EXPORT void APIENTRY glColor4dv(const GLdouble *v);
EXPORT void APIENTRY glColor4f(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
EXPORT void APIENTRY glColor4fv(const GLfloat *v);
EXPORT void APIENTRY glColor4i(GLint red, GLint green, GLint blue, GLint alpha);
EXPORT void APIENTRY glColor4iv(const GLint *v);
EXPORT void APIENTRY glColor4s(GLshort red, GLshort green, GLshort blue, GLshort alpha);
EXPORT void APIENTRY glColor4sv(const GLshort *v);
EXPORT void APIENTRY glColor4ub(GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha);
EXPORT void APIENTRY glColor4ubv(const GLubyte *v);
EXPORT void APIENTRY glColor4ui(GLuint red, GLuint green, GLuint blue, GLuint alpha);
EXPORT void APIENTRY glColor4uiv(const GLuint *v);
EXPORT void APIENTRY glColor4us(GLushort red, GLushort green, GLushort blue, GLushort alpha);
EXPORT void APIENTRY glColor4usv(const GLushort *v);

EXPORT void APIENTRY glEnable(GLenum cap);
EXPORT void APIENTRY glDisable(GLenum cap);
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
