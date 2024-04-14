
#include "pch.h"
#include "gl_state.h"
#include "gl_exports.h"

#define VALIDATE_TEX_IMAGE \
if (level < 0 || level > gl_max_tex_level) \
{ \
	gl_set_error_a(GL_INVALID_VALUE, level); \
	return; \
} \
if (components < 0 || components > 4) \
{ \
	gl_set_error_a(GL_INVALID_VALUE, components); \
	return; \
} \
if (border != 0 && border != 1) \
{ \
	gl_set_error_a(GL_INVALID_VALUE, border); \
	return; \
} \
if (format != GL_COLOR_INDEX && (format < GL_RED || format > GL_LUMINANCE_ALPHA)) \
{ \
	gl_set_error_a(GL_INVALID_ENUM, format); \
	return; \
} \
if (type != GL_BITMAP && (type < GL_BYTE || type > GL_FLOAT)) \
{ \
	gl_set_error_a(GL_INVALID_ENUM, type); \
	return; \
} \
if (type == GL_BITMAP && format != GL_COLOR_INDEX) \
{ \
	gl_set_error_a(GL_INVALID_ENUM, format); \
	return; \
}

void APIENTRY glTexImage2D(GLenum target, GLint level, GLint components, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void* data)
{
	gl_state* gs = gl_current_state();
	if (!gs) return;
	VALIDATE_NOT_BEGIN_MODE

	if (target != GL_TEXTURE_2D)
	{
		gl_set_error_a(GL_INVALID_ENUM, target);
		return;
	}
	VALIDATE_TEX_IMAGE

	int borderless_width = width - border * 2;
	int borderless_height = height - border * 2;
	if (borderless_width < 0 || borderless_width > gl_max_texture_size || !is_pow(borderless_width) ||
		borderless_height < 0 || borderless_height > gl_max_texture_size || !is_pow(borderless_height))
	{
		gl_set_error(GL_INVALID_VALUE);
		return;
	}
}

void APIENTRY glTexImage1D(GLenum target, GLint level, GLint components, GLsizei width, GLint border, GLenum format, GLenum type, const void* data)
{
	gl_state* gs = gl_current_state();
	if (!gs) return;
	VALIDATE_NOT_BEGIN_MODE

	if (target != GL_TEXTURE_1D)
	{
		gl_set_error_a(GL_INVALID_ENUM, target);
		return;
	}
	VALIDATE_TEX_IMAGE

	int borderless_width = width - border * 2;
	if (borderless_width < 0 || borderless_width > gl_max_texture_size || !is_pow(borderless_width))
	{
		gl_set_error_a(GL_INVALID_VALUE, width);
	}
}

#define VALIDATE_TEX_PARAMETER_V(p) \
gl_state* gs = gl_current_state(); \
if (!gs) return; \
VALIDATE_NOT_BEGIN_MODE; \
if (target != GL_TEXTURE_1D && target != GL_TEXTURE_2D) \
{ \
	gl_set_error_a(GL_INVALID_ENUM, target); \
	return; \
} \
if ((pname < GL_TEXTURE_MAG_FILTER || pname > GL_TEXTURE_WRAP_T) && pname != GL_TEXTURE_BORDER_COLOR) \
{ \
	gl_set_error_a(GL_INVALID_ENUM, pname); \
	return; \
} \
if ((pname == GL_TEXTURE_WRAP_S || pname == GL_TEXTURE_WRAP_T) && (p != GL_CLAMP && p != GL_REPEAT)) \
{ \
	gl_set_error_a(GL_INVALID_ENUM, p); \
	return; \
} \
if ((pname == GL_TEXTURE_MAG_FILTER && p != GL_NEAREST && p != GL_LINEAR) \
	|| (pname == GL_TEXTURE_MIN_FILTER && p != GL_NEAREST && p != GL_LINEAR && (p < GL_NEAREST_MIPMAP_NEAREST || p > GL_LINEAR_MIPMAP_LINEAR))) \
{ \
	gl_set_error_a(GL_INVALID_ENUM, p); \
	return; \
}

void APIENTRY glTexParameteri(GLenum target, GLenum pname, GLint param)
{
	VALIDATE_TEX_PARAMETER_V(param);
	if (pname == GL_TEXTURE_BORDER_COLOR)
	{
		gl_set_error_a(GL_INVALID_ENUM, pname);
		return;
	}

	gl_texture& tex = target == GL_TEXTURE_2D ? gs->texture_2d : gs->texture_1d;

	if (pname == GL_TEXTURE_MAG_FILTER)
		tex.mag_filter = param;
	else if (pname == GL_TEXTURE_MIN_FILTER)
		tex.min_filter = param;
	else if (pname == GL_TEXTURE_WRAP_S)
		tex.wrap_s = param;
	else if (pname == GL_TEXTURE_WRAP_T)
		tex.wrap_t = param;
}
void APIENTRY glTexParameterf(GLenum target, GLenum pname, GLfloat param)
{
	glTexParameteri(target, pname, (int)param);
}

template<typename T>
void gl_texparameterv(GLenum target, GLenum pname, const T* params)
{
	VALIDATE_TEX_PARAMETER_V(int(params[0]));

	gl_texture& tex = target == GL_TEXTURE_2D ? gs->texture_2d : gs->texture_1d;

	if (pname == GL_TEXTURE_MAG_FILTER)
		tex.mag_filter = (int)params[0];
	else if (pname == GL_TEXTURE_MIN_FILTER)
		tex.min_filter = (int)params[0];
	else if (pname == GL_TEXTURE_WRAP_S)
		tex.wrap_s = (int)params[0];
	else if (pname == GL_TEXTURE_WRAP_T)
		tex.wrap_t = (int)params[0];
	else if (pname == GL_TEXTURE_BORDER_COLOR)
		tex.border_color = glm::vec4(GLtof(params[0]), GLtof(params[1]), GLtof(params[2]), GLtof(params[3]));
}

void APIENTRY glTexParameteriv(GLenum target, GLenum pname, const GLint* params)
{
	gl_texparameterv(target, pname, params);
}
void APIENTRY glTexParameterfv(GLenum target, GLenum pname, const GLfloat* params)
{
	gl_texparameterv(target, pname, params);
}

void APIENTRY glTexEnvf(GLenum target, GLenum pname, GLfloat param)
{}
