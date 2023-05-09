#pragma once
#include <stdint.h>

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

inline float GLtof(GLubyte v) { return float(v) / 0xFF; }
inline float GLtof(GLbyte v) { return float(2 * v + 1) / 0xFF; }
inline float GLtof(GLushort v) { return float(v) / 0xFFFF; }
inline float GLtof(GLshort v) { return float(2 * v + 1) / 0xFFFF; }
inline float GLtof(GLuint v) { return float(v) / 0xFFFFFFFF; }
inline float GLtof(GLint v) { return float(2 * v + 1) / 0xFFFFFFFF; }
inline float GLtof(GLfloat v) { return v; }
inline float GLtof(GLdouble v) { return (float)v; }

#define GL_NO_ERROR 0
#define GL_INVALID_ENUM 0x0500
#define GL_INVALID_VALUE 0x0501
#define GL_INVALID_OPERATION 0x0502
#define GL_STACK_OVERFLOW 0x0503
#define GL_STACK_UNDERFLOW 0x0504
#define GL_OUT_OF_MEMORY 0x0505

#define GL_POINTS 0x0000
#define GL_LINES 0x0001
#define GL_LINE_LOOP 0x0002
#define GL_LINE_STRIP 0x0003
#define GL_TRIANGLES 0x0004
#define GL_TRIANGLE_STRIP 0x0005
#define GL_TRIANGLE_FAN 0x0006
#define GL_QUADS 0x0007
#define GL_QUAD_STRIP 0x0008
#define GL_POLYGON 0x0009

#define GL_MODELVIEW 0x1700
#define GL_PROJECTION 0x1701
#define GL_TEXTURE 0x1702

#define GL_NORMALIZE 0x0BA1

#define GL_TEXTURE_GEN_S 0x0C60
#define GL_TEXTURE_GEN_T 0x0C61
#define GL_TEXTURE_GEN_R 0x0C62
#define GL_TEXTURE_GEN_Q 0x0C63

#define GL_S 0x2000
#define GL_T 0x2001
#define GL_R 0x2002
#define GL_Q 0x2003

#define GL_TEXTURE_GEN_MODE 0x2500
#define GL_OBJECT_PLANE 0x2501
#define GL_EYE_PLANE 0x2502

#define GL_EYE_LINEAR 0x2400
#define GL_OBJECT_LINEAR 0x2401
#define GL_SPHERE_MAP 0x2402

#define GL_CLIP_PLANE0 0x3000
#define GL_CLIP_PLANE1 0x3001
#define GL_CLIP_PLANE2 0x3002
#define GL_CLIP_PLANE3 0x3003
#define GL_CLIP_PLANE4 0x3004
#define GL_CLIP_PLANE5 0x3005

#define GL_LIGHTING 0x0B50

#define GL_CW 0x0900
#define GL_CCW 0x0901

#define GL_FRONT 0x0404
#define GL_BACK 0x0405
#define GL_FRONT_AND_BACK 0x0408

#define GL_LIGHT0 0x4000
#define GL_LIGHT1 0x4001
#define GL_LIGHT2 0x4002
#define GL_LIGHT3 0x4003
#define GL_LIGHT4 0x4004
#define GL_LIGHT5 0x4005
#define GL_LIGHT6 0x4006
#define GL_LIGHT7 0x4007

#define GL_AMBIENT 0x1200
#define GL_DIFFUSE 0x1201
#define GL_SPECULAR 0x1202
#define GL_POSITION 0x1203
#define GL_SPOT_DIRECTION 0x1204
#define GL_SPOT_EXPONENT 0x1205
#define GL_SPOT_CUTOFF 0x1206
#define GL_CONSTANT_ATTENUATION 0x1207
#define GL_LINEAR_ATTENUATION 0x1208
#define GL_QUADRATIC_ATTENUATION 0x1209

#define GL_EMISSION 0x1600
#define GL_SHININESS 0x1601
#define GL_AMBIENT_AND_DIFFUSE 0x1602

#define GL_LIGHT_MODEL_LOCAL_VIEWER 0x0B51
#define GL_LIGHT_MODEL_TWO_SIDE 0x0B52
#define GL_LIGHT_MODEL_AMBIENT 0x0B53

#define GL_COLOR_MATERIAL 0x0B57

#define GL_FLAT 0x1D00
#define GL_SMOOTH 0x1D01

#define GL_POINT_SMOOTH 0x0B10
#define GL_LINE_SMOOTH 0x0B20
#define GL_LINE_STIPPLE 0x0B24
#define GL_POLYGON_SMOOTH 0x0B41
#define GL_CULL_FACE 0x0B44
#define GL_POLYGON_STIPPLE 0x0B42

#define GL_POINT 0x1B00
#define GL_LINE 0x1B01
#define GL_FILL 0x1B02

#define GL_SCISSOR_TEST 0x0C11

#define GL_NONE 0
#define GL_FRONT_LEFT 0x0400
#define GL_FRONT_RIGHT 0x0401
#define GL_BACK_LEFT 0x0402
#define GL_BACK_RIGHT 0x0403
#define GL_FRONT 0x0404
#define GL_BACK 0x0405
#define GL_LEFT 0x0406
#define GL_RIGHT 0x0407
#define GL_FRONT_AND_BACK 0x0408
#define GL_AUX0 0x0409
#define GL_AUX1 0x040A
#define GL_AUX2 0x040B
#define GL_AUX3 0x040C

#define GL_DEPTH_BUFFER_BIT 0x00000100
#define GL_ACCUM_BUFFER_BIT 0x00000200
#define GL_STENCIL_BUFFER_BIT 0x00000400
#define GL_COLOR_BUFFER_BIT 0x00004000

#define GL_VENDOR 0x1F00
#define GL_RENDERER 0x1F01
#define GL_VERSION 0x1F02
#define GL_EXTENSIONS 0x1F03
