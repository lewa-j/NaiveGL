#pragma once
#include "gl_types.h"
#include <stdint.h>

#undef EXPORT
#undef APIENTRY
#ifdef WIN32
#define EXPORT __declspec(dllexport) extern
#define APIENTRY __stdcall
#else
#define EXPORT extern
#define APIENTRY
#endif

#ifdef __cplusplus
extern "C" {
#endif

EXPORT GLenum APIENTRY glGetError();

EXPORT void APIENTRY glBegin(GLenum mode);
EXPORT void APIENTRY glEnd();

EXPORT void APIENTRY glEdgeFlag(GLboolean flag);
EXPORT void APIENTRY glEdgeFlagv(GLboolean *flag);

EXPORT void APIENTRY glVertex2s(GLshort x, GLshort y);
EXPORT void APIENTRY glVertex2sv(const GLshort *v);
EXPORT void APIENTRY glVertex2i(GLint x, GLint y);
EXPORT void APIENTRY glVertex2iv(const GLint *v);
EXPORT void APIENTRY glVertex2f(GLfloat x, GLfloat y);
EXPORT void APIENTRY glVertex2fv(const GLfloat *v);
EXPORT void APIENTRY glVertex2d(GLdouble x, GLdouble y);
EXPORT void APIENTRY glVertex2dv(const GLdouble *v);
EXPORT void APIENTRY glVertex3s(GLshort x, GLshort y, GLshort z);
EXPORT void APIENTRY glVertex3sv(const GLshort *v);
EXPORT void APIENTRY glVertex3i(GLint x, GLint y, GLint z);
EXPORT void APIENTRY glVertex3iv(const GLint *v);
EXPORT void APIENTRY glVertex3f(GLfloat x, GLfloat y, GLfloat z);
EXPORT void APIENTRY glVertex3fv(const GLfloat *v);
EXPORT void APIENTRY glVertex3d(GLdouble x, GLdouble y, GLdouble z);
EXPORT void APIENTRY glVertex3dv(const GLdouble *v);
EXPORT void APIENTRY glVertex4s(GLshort x, GLshort y, GLshort z, GLshort w);
EXPORT void APIENTRY glVertex4sv(const GLshort *v);
EXPORT void APIENTRY glVertex4i(GLint x, GLint y, GLint z, GLint w);
EXPORT void APIENTRY glVertex4iv(const GLint *v);
EXPORT void APIENTRY glVertex4f(GLfloat x, GLfloat y, GLfloat z, GLfloat w);
EXPORT void APIENTRY glVertex4fv(const GLfloat *v);
EXPORT void APIENTRY glVertex4d(GLdouble x, GLdouble y, GLdouble z, GLdouble w);
EXPORT void APIENTRY glVertex4dv(const GLdouble *v);

EXPORT void APIENTRY glTexCoord1s(GLshort s);
EXPORT void APIENTRY glTexCoord1sv(const GLshort *v);
EXPORT void APIENTRY glTexCoord1i(GLint s);
EXPORT void APIENTRY glTexCoord1iv(const GLint *v);
EXPORT void APIENTRY glTexCoord1f(GLfloat s);
EXPORT void APIENTRY glTexCoord1fv(const GLfloat *v);
EXPORT void APIENTRY glTexCoord1d(GLdouble s);
EXPORT void APIENTRY glTexCoord1dv(const GLdouble *v);
EXPORT void APIENTRY glTexCoord2s(GLshort s, GLshort t);
EXPORT void APIENTRY glTexCoord2sv(const GLshort *v);
EXPORT void APIENTRY glTexCoord2i(GLint s, GLint t);
EXPORT void APIENTRY glTexCoord2iv(const GLint *v);
EXPORT void APIENTRY glTexCoord2f(GLfloat s, GLfloat t);
EXPORT void APIENTRY glTexCoord2fv(const GLfloat *v);
EXPORT void APIENTRY glTexCoord2d(GLdouble s, GLdouble t);
EXPORT void APIENTRY glTexCoord2dv(const GLdouble *v);
EXPORT void APIENTRY glTexCoord3s(GLshort s, GLshort t, GLshort r);
EXPORT void APIENTRY glTexCoord3sv(const GLshort *v);
EXPORT void APIENTRY glTexCoord3i(GLint s, GLint t, GLint r);
EXPORT void APIENTRY glTexCoord3iv(const GLint *v);
EXPORT void APIENTRY glTexCoord3f(GLfloat s, GLfloat t, GLfloat r);
EXPORT void APIENTRY glTexCoord3fv(const GLfloat *v);
EXPORT void APIENTRY glTexCoord3d(GLdouble s, GLdouble t, GLdouble r);
EXPORT void APIENTRY glTexCoord3dv(const GLdouble *v);
EXPORT void APIENTRY glTexCoord4s(GLshort s, GLshort t, GLshort r, GLshort q);
EXPORT void APIENTRY glTexCoord4sv(const GLshort *v);
EXPORT void APIENTRY glTexCoord4i(GLint s, GLint t, GLint r, GLint q);
EXPORT void APIENTRY glTexCoord4iv(const GLint *v);
EXPORT void APIENTRY glTexCoord4f(GLfloat s, GLfloat t, GLfloat r, GLfloat q);
EXPORT void APIENTRY glTexCoord4fv(const GLfloat *v);
EXPORT void APIENTRY glTexCoord4d(GLdouble s, GLdouble t, GLdouble r, GLdouble q);
EXPORT void APIENTRY glTexCoord4dv(const GLdouble *v);

EXPORT void APIENTRY glNormal3b(GLbyte nx, GLbyte ny, GLbyte nz);
EXPORT void APIENTRY glNormal3bv(const GLbyte *v);
EXPORT void APIENTRY glNormal3s(GLshort nx, GLshort ny, GLshort nz);
EXPORT void APIENTRY glNormal3sv(const GLshort *v);
EXPORT void APIENTRY glNormal3i(GLint nx, GLint ny, GLint nz);
EXPORT void APIENTRY glNormal3iv(const GLint *v);
EXPORT void APIENTRY glNormal3f(GLfloat nx, GLfloat ny, GLfloat nz);
EXPORT void APIENTRY glNormal3fv(const GLfloat *v);
EXPORT void APIENTRY glNormal3d(GLdouble nx, GLdouble ny, GLdouble nz);
EXPORT void APIENTRY glNormal3dv(const GLdouble *v);

EXPORT void APIENTRY glColor3b(GLbyte red, GLbyte green, GLbyte blue);
EXPORT void APIENTRY glColor3bv(const GLbyte *v);
EXPORT void APIENTRY glColor3s(GLshort red, GLshort green, GLshort blue);
EXPORT void APIENTRY glColor3sv(const GLshort *v);
EXPORT void APIENTRY glColor3i(GLint red, GLint green, GLint blue);
EXPORT void APIENTRY glColor3iv(const GLint *v);
EXPORT void APIENTRY glColor3f(GLfloat red, GLfloat green, GLfloat blue);
EXPORT void APIENTRY glColor3fv(const GLfloat *v);
EXPORT void APIENTRY glColor3d(GLdouble red, GLdouble green, GLdouble blue);
EXPORT void APIENTRY glColor3dv(const GLdouble *v);
EXPORT void APIENTRY glColor3ub(GLubyte red, GLubyte green, GLubyte blue);
EXPORT void APIENTRY glColor3ubv(const GLubyte *v);
EXPORT void APIENTRY glColor3us(GLushort red, GLushort green, GLushort blue);
EXPORT void APIENTRY glColor3usv(const GLushort *v);
EXPORT void APIENTRY glColor3ui(GLuint red, GLuint green, GLuint blue);
EXPORT void APIENTRY glColor3uiv(const GLuint *v);
EXPORT void APIENTRY glColor4b(GLbyte red, GLbyte green, GLbyte blue, GLbyte alpha);
EXPORT void APIENTRY glColor4bv(const GLbyte *v);
EXPORT void APIENTRY glColor4s(GLshort red, GLshort green, GLshort blue, GLshort alpha);
EXPORT void APIENTRY glColor4sv(const GLshort *v);
EXPORT void APIENTRY glColor4i(GLint red, GLint green, GLint blue, GLint alpha);
EXPORT void APIENTRY glColor4iv(const GLint *v);
EXPORT void APIENTRY glColor4f(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
EXPORT void APIENTRY glColor4fv(const GLfloat *v);
EXPORT void APIENTRY glColor4d(GLdouble red, GLdouble green, GLdouble blue, GLdouble alpha);
EXPORT void APIENTRY glColor4dv(const GLdouble *v);
EXPORT void APIENTRY glColor4ub(GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha);
EXPORT void APIENTRY glColor4ubv(const GLubyte *v);
EXPORT void APIENTRY glColor4us(GLushort red, GLushort green, GLushort blue, GLushort alpha);
EXPORT void APIENTRY glColor4usv(const GLushort *v);
EXPORT void APIENTRY glColor4ui(GLuint red, GLuint green, GLuint blue, GLuint alpha);
EXPORT void APIENTRY glColor4uiv(const GLuint *v);

EXPORT void APIENTRY glIndexs(GLshort c);
EXPORT void APIENTRY glIndexsv(const GLshort *c);
EXPORT void APIENTRY glIndexi(GLint c);
EXPORT void APIENTRY glIndexiv(const GLint *c);
EXPORT void APIENTRY glIndexf(GLfloat c);
EXPORT void APIENTRY glIndexfv(const GLfloat *c);
EXPORT void APIENTRY glIndexd(GLdouble c);
EXPORT void APIENTRY glIndexdv(const GLdouble *c);

EXPORT void APIENTRY glRects(GLshort x1, GLshort y1, GLshort x2, GLshort y2);
EXPORT void APIENTRY glRecti(GLint x1, GLint y1, GLint x2, GLint y2);
EXPORT void APIENTRY glRectf(GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2);
EXPORT void APIENTRY glRectd(GLdouble x1, GLdouble y1, GLdouble x2, GLdouble y2);
EXPORT void APIENTRY glRectsv(const GLshort *v1, const GLshort *v2);
EXPORT void APIENTRY glRectiv(const GLint *v1, const GLint *v2);
EXPORT void APIENTRY glRectfv(const GLfloat *v1, const GLfloat *v2);
EXPORT void APIENTRY glRectdv(const GLdouble *v1, const GLdouble *v2);

EXPORT void APIENTRY glDepthRange(GLdouble n, GLdouble f);
EXPORT void APIENTRY glViewport(GLint x, GLint y, GLsizei width, GLsizei height);

EXPORT void APIENTRY glMatrixMode(GLenum mode);
EXPORT void APIENTRY glLoadMatrixd(const GLdouble *m);
EXPORT void APIENTRY glLoadMatrixf(const GLfloat *m);
EXPORT void APIENTRY glMultMatrixd(const GLdouble *m);
EXPORT void APIENTRY glMultMatrixf(const GLfloat *m);
EXPORT void APIENTRY glLoadIdentity(void);
EXPORT void APIENTRY glRotated(GLdouble angle, GLdouble x, GLdouble y, GLdouble z);
EXPORT void APIENTRY glRotatef(GLfloat angle, GLfloat x, GLfloat y, GLfloat z);
EXPORT void APIENTRY glTranslated(GLdouble x, GLdouble y, GLdouble z);
EXPORT void APIENTRY glTranslatef(GLfloat x, GLfloat y, GLfloat z);
EXPORT void APIENTRY glScaled(GLdouble x, GLdouble y, GLdouble z);
EXPORT void APIENTRY glScalef(GLfloat x, GLfloat y, GLfloat z);
EXPORT void APIENTRY glFrustum(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar);
EXPORT void APIENTRY glOrtho(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar);
EXPORT void APIENTRY glPushMatrix();
EXPORT void APIENTRY glPopMatrix();

EXPORT void APIENTRY glEnable(GLenum cap);
EXPORT void APIENTRY glDisable(GLenum cap);

EXPORT void APIENTRY glTexGeni(GLenum coord, GLenum pname, GLint param);
EXPORT void APIENTRY glTexGenf(GLenum coord, GLenum pname, GLfloat param);
EXPORT void APIENTRY glTexGend(GLenum coord, GLenum pname, GLdouble param);
EXPORT void APIENTRY glTexGeniv(GLenum coord, GLenum pname, const GLint *params);
EXPORT void APIENTRY glTexGenfv(GLenum coord, GLenum pname, const GLfloat *params);
EXPORT void APIENTRY glTexGendv(GLenum coord, GLenum pname, const GLdouble *params);

EXPORT void APIENTRY glClipPlane(GLenum plane, const GLdouble *equation);

EXPORT void APIENTRY glRasterPos2s(GLshort x, GLshort y);
EXPORT void APIENTRY glRasterPos2i(GLint x, GLint y);
EXPORT void APIENTRY glRasterPos2f(GLfloat x, GLfloat y);
EXPORT void APIENTRY glRasterPos2d(GLdouble x, GLdouble y);
EXPORT void APIENTRY glRasterPos2sv(const GLshort *v);
EXPORT void APIENTRY glRasterPos2iv(const GLint *v);
EXPORT void APIENTRY glRasterPos2fv(const GLfloat *v);
EXPORT void APIENTRY glRasterPos2dv(const GLdouble *v);
EXPORT void APIENTRY glRasterPos3s(GLshort x, GLshort y, GLshort z);
EXPORT void APIENTRY glRasterPos3i(GLint x, GLint y, GLint z);
EXPORT void APIENTRY glRasterPos3f(GLfloat x, GLfloat y, GLfloat z);
EXPORT void APIENTRY glRasterPos3d(GLdouble x, GLdouble y, GLdouble z);
EXPORT void APIENTRY glRasterPos3sv(const GLshort *v);
EXPORT void APIENTRY glRasterPos3iv(const GLint *v);
EXPORT void APIENTRY glRasterPos3fv(const GLfloat *v);
EXPORT void APIENTRY glRasterPos3dv(const GLdouble *v);
EXPORT void APIENTRY glRasterPos4s(GLshort x, GLshort y, GLshort z, GLshort w);
EXPORT void APIENTRY glRasterPos4i(GLint x, GLint y, GLint z, GLint w);
EXPORT void APIENTRY glRasterPos4f(GLfloat x, GLfloat y, GLfloat z, GLfloat w);
EXPORT void APIENTRY glRasterPos4d(GLdouble x, GLdouble y, GLdouble z, GLdouble w);
EXPORT void APIENTRY glRasterPos4sv(const GLshort *v);
EXPORT void APIENTRY glRasterPos4iv(const GLint *v);
EXPORT void APIENTRY glRasterPos4fv(const GLfloat *v);
EXPORT void APIENTRY glRasterPos4dv(const GLdouble *v);

EXPORT void APIENTRY glFrontFace(GLenum mode);
EXPORT void APIENTRY glColorMaterial(GLenum face, GLenum mode);
EXPORT void APIENTRY glShadeModel(GLenum mode);
EXPORT void APIENTRY glMateriali(GLenum face, GLenum pname, GLint param);
EXPORT void APIENTRY glMaterialf(GLenum face, GLenum pname, GLfloat param);
EXPORT void APIENTRY glMaterialiv(GLenum face, GLenum pname, const GLint *params);
EXPORT void APIENTRY glMaterialfv(GLenum face, GLenum pname, const GLfloat *params);
EXPORT void APIENTRY glLighti(GLenum light, GLenum pname, GLint param);
EXPORT void APIENTRY glLightf(GLenum light, GLenum pname, GLfloat param);
EXPORT void APIENTRY glLightiv(GLenum light, GLenum pname, const GLint *params);
EXPORT void APIENTRY glLightfv(GLenum light, GLenum pname, const GLfloat *params);
EXPORT void APIENTRY glLightModeli(GLenum pname, GLint param);
EXPORT void APIENTRY glLightModelf(GLenum pname, GLfloat param);
EXPORT void APIENTRY glLightModeliv(GLenum pname, const GLint *params);
EXPORT void APIENTRY glLightModelfv(GLenum pname, const GLfloat *params);

EXPORT void APIENTRY glPointSize(GLfloat size);
EXPORT void APIENTRY glLineWidth(GLfloat width);
EXPORT void APIENTRY glLineStipple(GLint factor, GLushort pattern);
EXPORT void APIENTRY glCullFace(GLenum mode);
EXPORT void APIENTRY glPolygonStipple(const GLubyte *mask);
EXPORT void APIENTRY glPolygonMode(GLenum face, GLenum mode);

EXPORT void APIENTRY glPixelStorei(GLenum pname, GLint param);
EXPORT void APIENTRY glPixelStoref(GLenum pname, GLfloat param);
EXPORT void APIENTRY glPixelTransferi(GLenum pname, GLint param);
EXPORT void APIENTRY glPixelTransferf(GLenum pname, GLfloat param);
EXPORT void APIENTRY glPixelMapuiv(GLenum map, GLsizei mapsize, const GLuint* values);
EXPORT void APIENTRY glPixelMapusv(GLenum map, GLsizei mapsize, const GLushort* values);
EXPORT void APIENTRY glPixelMapfv(GLenum map, GLsizei mapsize, const GLfloat* values);
EXPORT void APIENTRY glPixelZoom(GLfloat xfactor, GLfloat yfactor);
EXPORT void APIENTRY glDrawPixels(GLsizei width, GLsizei height, GLenum format, GLenum type, const void* data);
EXPORT void APIENTRY glBitmap(GLsizei width, GLsizei height, GLfloat xorig, GLfloat yorig, GLfloat xmove, GLfloat ymove, const GLubyte* bitmap);
EXPORT void APIENTRY glTexImage2D(GLenum target, GLint level, GLint components, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void *data);
EXPORT void APIENTRY glTexImage1D(GLenum target, GLint level, GLint components, GLsizei width, GLint border, GLenum format, GLenum type, const void* data);
EXPORT void APIENTRY glTexParameteri(GLenum target, GLenum pname, GLint param);
EXPORT void APIENTRY glTexParameterf(GLenum target, GLenum pname, GLfloat param);
EXPORT void APIENTRY glTexParameteriv(GLenum target, GLenum pname, const GLint* params);
EXPORT void APIENTRY glTexParameterfv(GLenum target, GLenum pname, const GLfloat* params);
EXPORT void APIENTRY glTexEnvi(GLenum target, GLenum pname, GLint param);
EXPORT void APIENTRY glTexEnvf(GLenum target, GLenum pname, GLfloat param);
EXPORT void APIENTRY glTexEnviv(GLenum target, GLenum pname, const GLint* params);
EXPORT void APIENTRY glTexEnvfv(GLenum target, GLenum pname, const GLfloat* params);

EXPORT void APIENTRY glFogi(GLenum pname, GLint param);
EXPORT void APIENTRY glFogf(GLenum pname, GLfloat param);
EXPORT void APIENTRY glFogiv(GLenum pname, const GLint* params);
EXPORT void APIENTRY glFogfv(GLenum pname, const GLfloat* params);

EXPORT void APIENTRY glScissor(GLint x, GLint y, GLsizei width, GLsizei height);
EXPORT void APIENTRY glAlphaFunc(GLenum func, GLfloat ref);
EXPORT void APIENTRY glStencilFunc(GLenum func, GLint ref, GLuint mask);
EXPORT void APIENTRY glStencilOp(GLenum sfail, GLenum dpfail, GLenum dppass);
EXPORT void APIENTRY glDepthFunc(GLenum func);
EXPORT void APIENTRY glBlendFunc(GLenum sfactor, GLenum dfactor);
EXPORT void APIENTRY glLogicOp(GLenum opcode);

EXPORT void APIENTRY glDrawBuffer(GLenum buf);
EXPORT void APIENTRY glColorMask(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha);
EXPORT void APIENTRY glIndexMask(GLuint mask);
EXPORT void APIENTRY glDepthMask(GLboolean flag);
EXPORT void APIENTRY glStencilMask(GLuint mask);
EXPORT void APIENTRY glClear(GLbitfield mask);
EXPORT void APIENTRY glClearColor(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
EXPORT void APIENTRY glClearIndex(GLfloat c);
EXPORT void APIENTRY glClearDepth(GLdouble depth);
EXPORT void APIENTRY glClearStencil(GLint s);
EXPORT void APIENTRY glClearAccum(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
EXPORT void APIENTRY glAccum(GLenum op, GLfloat value);
EXPORT void APIENTRY glReadPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, void* pixels);
EXPORT void APIENTRY glReadBuffer(GLenum src);
EXPORT void APIENTRY glCopyPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum type);

EXPORT void APIENTRY glMap1f(GLenum target, GLfloat u1, GLfloat u2, GLint stride, GLint order, const GLfloat *points);
EXPORT void APIENTRY glMap1d(GLenum target, GLdouble u1, GLdouble u2, GLint stride, GLint order, const GLdouble *points);
EXPORT void APIENTRY glMap2f(GLenum target, GLfloat u1, GLfloat u2, GLint ustride, GLint uorder, GLfloat v1, GLfloat v2, GLint vstride, GLint vorder, const GLfloat *points);
EXPORT void APIENTRY glMap2d(GLenum target, GLdouble u1, GLdouble u2, GLint ustride, GLint uorder, GLdouble v1, GLdouble v2, GLint vstride, GLint vorder, const GLdouble *points);
EXPORT void APIENTRY glEvalCoord1f(GLfloat u);
EXPORT void APIENTRY glEvalCoord1d(GLdouble u);
EXPORT void APIENTRY glEvalCoord1fv(const GLfloat *u);
EXPORT void APIENTRY glEvalCoord1dv(const GLdouble *u);
EXPORT void APIENTRY glEvalCoord2f(GLfloat u, GLfloat v);
EXPORT void APIENTRY glEvalCoord2d(GLdouble u, GLdouble v);
EXPORT void APIENTRY glEvalCoord2fv(const GLfloat *u);
EXPORT void APIENTRY glEvalCoord2dv(const GLdouble *u);
EXPORT void APIENTRY glMapGrid1f(GLint un, GLfloat u1, GLfloat u2);
EXPORT void APIENTRY glMapGrid1d(GLint un, GLdouble u1, GLdouble u2);
EXPORT void APIENTRY glMapGrid2f(GLint un, GLfloat u1, GLfloat u2, GLint vn, GLfloat v1, GLfloat v2);
EXPORT void APIENTRY glMapGrid2d(GLint un, GLdouble u1, GLdouble u2, GLint vn, GLdouble v1, GLdouble v2);
EXPORT void APIENTRY glEvalMesh1(GLenum mode, GLint p1, GLint p2);
EXPORT void APIENTRY glEvalMesh2(GLenum mode, GLint p1, GLint p2, GLint q1, GLint q2);
EXPORT void APIENTRY glEvalPoint1(GLint p);
EXPORT void APIENTRY glEvalPoint2(GLint p, GLint q);
EXPORT void APIENTRY glInitNames(void);
EXPORT void APIENTRY glPopName(void);
EXPORT void APIENTRY glPushName(GLuint name);
EXPORT void APIENTRY glLoadName(GLuint name);
EXPORT GLint APIENTRY glRenderMode(GLenum mode);
EXPORT void APIENTRY glSelectBuffer(GLsizei size, GLuint *buffer);
EXPORT void APIENTRY glFeedbackBuffer(GLsizei size, GLenum type, GLfloat *buffer);
EXPORT void APIENTRY glPassThrough(GLfloat token);
EXPORT void APIENTRY glNewList(GLuint list, GLenum mode);
EXPORT void APIENTRY glEndList(void);
EXPORT void APIENTRY glCallList(GLuint list);
EXPORT void APIENTRY glCallLists(GLsizei n, GLenum type, const void* lists);
EXPORT void APIENTRY glListBase(GLuint base);
EXPORT GLuint APIENTRY glGenLists(GLsizei range);
EXPORT GLboolean APIENTRY glIsList(GLuint list);
EXPORT void APIENTRY glDeleteLists(GLuint list, GLsizei range);

EXPORT void APIENTRY glFlush(void);
EXPORT void APIENTRY glFinish(void);
EXPORT void APIENTRY glHint(GLenum target, GLenum mode);

EXPORT const char* APIENTRY glGetString(GLenum name);
EXPORT void APIENTRY glGetBooleanv(GLenum pname, GLboolean* data);
EXPORT void APIENTRY glGetIntegerv(GLenum pname, GLint* data);
EXPORT void APIENTRY glGetFloatv(GLenum pname, GLfloat* data);
EXPORT void APIENTRY glPushAttrib(GLbitfield mask);
EXPORT void APIENTRY glPopAttrib(void);

///!!!!!!!!!! gl 1.1
EXPORT void APIENTRY glTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void *pixels);
EXPORT void APIENTRY glBindTexture(GLenum target, GLuint texture);
//!!!!!!!!!!!!!!!!!!

#ifdef __cplusplus
}
#endif
