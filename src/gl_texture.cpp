
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
/*STENCIL_INDEX and DEPTH_COMPONENT are not allowed*/ \
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

static bool gl_is_texture_complete(gl_texture &tex)
{
	int w = tex.arrays[0].width;
	int h = tex.arrays[0].height;
	if (w < 1 || h < 1)
		return false;

	bool mipmap = (tex.min_filter == GL_NEAREST_MIPMAP_NEAREST || tex.min_filter == GL_NEAREST_MIPMAP_LINEAR
		|| tex.min_filter == GL_LINEAR_MIPMAP_NEAREST || tex.min_filter == GL_LINEAR_MIPMAP_LINEAR);

	if (!mipmap)
		return true;

	int levels = 1 + log2(glm::max(w, h));

	for (int i = 1; i < levels; i++)
	{
		w = glm::max(1, (w >> 1));
		h = glm::max(1, (h >> 1));
		if (w != tex.arrays[i].width || h != tex.arrays[i].height || tex.arrays[i].components != tex.arrays[0].components || tex.arrays[i].border != tex.arrays[0].border)
			return false;
	}
	return true;
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

	gl_texture& tex = gs->texture_2d;
	gl_texture_array& ta = tex.arrays[level];

	if (width == 0 || height == 0)
	{
		if (ta.data)
			delete[] ta.data;
		ta.data = nullptr;
		ta.width = 0;
		ta.height = 0;
		return;
	}

	size_t size = width * height * components;

	if (ta.width * ta.height * ta.components != size)
	{
		if (ta.data)
			delete[] ta.data;

		ta.data = new uint8_t[size];
	}
	ta.width = width;
	ta.height = height;
	ta.components = components;
	ta.border = border;

	if (!data)
	{
		tex.is_complete = gl_is_texture_complete(tex);
		return;
	}

	const gl_state::pixelStore& ps = gs->pixel_unpack;

	int pixel_size = 1;
	if (format == GL_LUMINANCE_ALPHA)
		pixel_size = 2;
	else if (format == GL_RGB)
		pixel_size = 3;
	else if (format == GL_RGBA)
		pixel_size = 4;

	if (type == GL_UNSIGNED_BYTE && border == 0 && (ps.row_length == 0 || ps.row_length == width) && (pixel_size * width % ps.alignment) == 0 && ps.skip_rows == 0 && ps.skip_pixels == 0 &&
		!gs->map_color && gs->color_scale == glm::vec4{ 1,1,1,1 } && gs->color_bias == glm::vec4{ 0,0,0,0 })
	{
		if (format == GL_LUMINANCE && components == 1 || format == GL_LUMINANCE_ALPHA && components == 2 || format == GL_RGB && components == 3 || format == GL_RGBA && components == 4)
		{
			memcpy(ta.data, data, size);
			tex.is_complete = gl_is_texture_complete(tex);
			return;
		}
		else
		{
			if (format == GL_RGBA && components == 3)
			{
				const uint8_t* src = (const uint8_t*)data;
				for (size_t i = 0; i < size; i += 3)
				{
					ta.data[i] = src[0];
					ta.data[i + 1] = src[1];
					ta.data[i + 2] = src[2];
					src += 4;
				}
				tex.is_complete = gl_is_texture_complete(tex);
				return;
			}
		}
	}

	printf("glTexImage2D(%d,%d,%d,%d,%d,%X,%X) al=%d unhandled combination\n", level, components, width, height, border, format, type, ps.alignment);
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
		return;
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

	tex.is_complete = gl_is_texture_complete(tex);
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

	tex.is_complete = gl_is_texture_complete(tex);
}

void APIENTRY glTexParameteriv(GLenum target, GLenum pname, const GLint* params)
{
	gl_texparameterv(target, pname, params);
}
void APIENTRY glTexParameterfv(GLenum target, GLenum pname, const GLfloat* params)
{
	gl_texparameterv(target, pname, params);
}

#define VALIDATE_TEX_ENV_V(p) \
gl_state* gs = gl_current_state(); \
if (!gs) return; \
VALIDATE_NOT_BEGIN_MODE; \
if (target != GL_TEXTURE_ENV) \
{ \
	gl_set_error_a(GL_INVALID_ENUM, target); \
	return; \
} \
if (pname != GL_TEXTURE_ENV_MODE && pname != GL_TEXTURE_ENV_COLOR) \
{ \
	gl_set_error_a(GL_INVALID_ENUM, pname); \
	return; \
} \
if (pname == GL_TEXTURE_ENV_MODE && (p != GL_MODULATE && p != GL_DECAL && p != GL_BLEND)) \
{ \
	gl_set_error_a(GL_INVALID_ENUM, p); \
	return; \
}

void APIENTRY glTexEnvi(GLenum target, GLenum pname, GLint param)
{
	VALIDATE_TEX_ENV_V(param);

	if (pname == GL_TEXTURE_ENV_COLOR)
	{
		gl_set_error_a(GL_INVALID_ENUM, pname);
		return;
	}

	if (pname == GL_TEXTURE_ENV_MODE)
	{
		gs->texture_env_function = param;
	}
}
void APIENTRY glTexEnvf(GLenum target, GLenum pname, GLfloat param)
{
	glTexEnvi(target, pname, (GLint)param);
}

void APIENTRY glTexEnviv(GLenum target, GLenum pname, const GLint* params)
{
	VALIDATE_TEX_ENV_V(params[0]);

	if (pname == GL_TEXTURE_ENV_MODE)
		gs->texture_env_function = params[0];
	else if (pname == GL_TEXTURE_ENV_COLOR)
		gs->texture_env_color = glm::vec4(GLtof(params[0]), GLtof(params[1]), GLtof(params[2]), GLtof(params[3]));
}

void APIENTRY glTexEnvfv(GLenum target, GLenum pname, const GLfloat* params)
{
	VALIDATE_TEX_ENV_V((GLenum)params[0]);

	if (pname == GL_TEXTURE_ENV_MODE)
		gs->texture_env_function = (int)params[0];
	else if (pname == GL_TEXTURE_ENV_COLOR)
		gs->texture_env_color = glm::vec4(params[0], params[1], params[2], params[3]);
}


glm::vec4 gl_state::sample_tex2d(const gl_texture& tex, const glm::vec4& tex_coord)
{
	const gl_texture_array& a = tex.arrays[0];
	if (!a.data)
		return glm::vec4(1, 1, 1, 1);

	glm::vec2 c = glm::fract(glm::vec2(tex_coord));
	int x = glm::clamp((int)floor(c.x * a.width), 0, a.width - 1);
	int y = glm::clamp((int)floor(c.y * a.height), 0, a.height - 1);
	uint8_t* d = a.data + (y * a.width + x) * 4;
	glm::vec4 col{ GLtof(d[0]), GLtof(d[1]), GLtof(d[2]), a.components == 4 ? GLtof(d[3]) : 1 };
	return col;
}
