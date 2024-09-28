#pragma once

#include "gl_types.h"

struct gl_state;

int gl_pixels_size(GLsizei width, GLsizei height, GLenum format, GLenum type);
void gl_unpack_pixels(gl_state *gs, GLsizei width, GLsizei height, GLenum format, GLenum type, const void *data, uint8_t *dst);

inline int gl_pixel_format_size(GLenum format)
{
	if (format == GL_LUMINANCE_ALPHA)
		return 2;
	if (format == GL_RGB)
		return 3;
	if (format == GL_RGBA)
		return 4;

	return 1;
}

inline int gl_pixel_type_size(GLenum type)
{
	if (type == GL_SHORT || type == GL_UNSIGNED_SHORT)
		return 2;
	if (type == GL_INT || type == GL_UNSIGNED_INT || type == GL_FLOAT)
		return 4;
#if NGL_VERISON >= 110
	if (type == GL_DOUBLE)
		return 8;
#endif
	return 1;
}

struct gl_PixelStoreSetup
{
	int components = 0;
	int element_size = 0;
	int group_size = 0;
	int stride = 0;
	int skip_bytes = 0;
	int skip_bits = 0;

	void init(const gl_state::pixelStore &ps, GLsizei width, GLsizei height, GLenum format, GLenum type)
	{
		int row_length = (ps.row_length > 0) ? ps.row_length : width;

		if (type == GL_BITMAP)
		{
			components = 1;
			element_size = 1;
			group_size = 1;

			stride = (int)(ps.alignment * glm::ceil(row_length / float(8 * ps.alignment)));
			skip_bits = ps.skip_pixels & 7;

			skip_bytes = ps.skip_pixels / 8 + ps.skip_rows * stride;
		}
		else
		{
			components = gl_pixel_format_size(format);
			element_size = gl_pixel_type_size(type);
			group_size = components * element_size;

			stride = row_length * group_size;
			if (element_size < ps.alignment)
				stride = (int)((ps.alignment / element_size * glm::ceil(stride / (float)ps.alignment)) * element_size);

			skip_bytes = (ps.skip_pixels * group_size + ps.skip_rows * stride);
		}
	}
};

inline glm::vec4 remap_color(const glm::vec4 &c, gl_state::pixelMapColor *tables)
{
	glm::vec4 r;
	for (int i = 0; i < 4; i++)
	{
		int ci = lroundf(glm::clamp(c[i], 0.f, 1.f) * (tables[i].size - 1));
		r[i] = tables[i].data[ci];
	}
	return r;
}

inline glm::vec4 gl_pixel_format_conversion(GLenum format, const glm::vec4 &color)
{
	glm::vec4 col = color;

	if (format == GL_LUMINANCE || format == GL_LUMINANCE_ALPHA)
	{
		col.r = col.r + col.g + col.b;
		col.g = col.a;
	}
	else if (format == GL_GREEN)
		col.r = col.g;
	else if (format == GL_BLUE)
		col.r = col.b;
	else if (format == GL_ALPHA)
		col.r = col.a;

	return col;
}

inline glm::vec4 index_to_rgba(int index, gl_state::pixelMapColor *tables)
{
	glm::vec4 r{};
	for (int k = 0; k < 4; k++)
	{
		int ti = index & (tables[k].size - 1);
		r[k] = tables[k].data[ti];
	}
	return r;
}

inline uint32_t gl_index_arithmetic(gl_state *gs, uint32_t index)
{
	if (gs->pixel.index_shift > 0)
		index <<= gs->pixel.index_shift;
	else if (gs->pixel.index_shift < 0)
		index >>= -gs->pixel.index_shift;

	index += gs->pixel.index_offset;
	return index;
}

inline uint32_t gl_unpack_index_pixel(GLenum type, const void *src)
{
	if (type == GL_BYTE)
		return *(const GLbyte *)src;
	if (type == GL_UNSIGNED_BYTE)
		return *(const GLubyte *)src;
	if (type == GL_SHORT)
		return *(const GLshort *)src;
	if (type == GL_UNSIGNED_SHORT)
		return *(const GLushort *)src;
	if (type == GL_INT)
		return *(const GLint *)src;
	if (type == GL_UNSIGNED_INT)
		return *(const GLuint *)src;
	if (type == GL_FLOAT)
		return (uint32_t) * (const float *)src;
	return 0;
}

template<typename T>
inline glm::vec4 pixel_to_float(const T *data, GLenum format)
{
	if (format == GL_GREEN)
		return glm::vec4(0, GLtof(data[0]), 0, 1);
	if (format == GL_BLUE)
		return glm::vec4(0, 0, GLtof(data[0]), 1);
	if (format == GL_ALPHA)
		return glm::vec4(0, 0, 0, GLtof(data[0]));
	if (format == GL_RGB)
		return glm::vec4(GLtof(data[0]), GLtof(data[1]), GLtof(data[2]), 1);
	if (format == GL_RGBA)
		return glm::vec4(GLtof(data[0]), GLtof(data[1]), GLtof(data[2]), GLtof(data[3]));
	if (format == GL_LUMINANCE)
	{
		float l = GLtof(data[0]);
		return glm::vec4(l, l, l, 1);
	}
	if (format == GL_LUMINANCE_ALPHA)
	{
		float l = GLtof(data[0]);
		return glm::vec4(l, l, l, GLtof(data[1]));
	}

	return glm::vec4(GLtof(data[0]), 0, 0, 1);
}

inline glm::vec4 gl_unpack_color_pixel(GLenum format, GLenum type, const void *src)
{
	if (type == GL_BYTE)
		return pixel_to_float((const GLbyte *)src, format);
	if (type == GL_UNSIGNED_BYTE)
		return pixel_to_float((const GLubyte *)src, format);
	if (type == GL_SHORT)
		return pixel_to_float((const GLshort *)src, format);
	if (type == GL_UNSIGNED_SHORT)
		return pixel_to_float((const GLushort *)src, format);
	if (type == GL_INT)
		return pixel_to_float((const GLint *)src, format);
	if (type == GL_UNSIGNED_INT)
		return pixel_to_float((const GLuint *)src, format);
	if (type == GL_FLOAT)
		return pixel_to_float((const float *)src, format);
	return glm::vec4(0);
}

inline void gl_pack_color_pixel(int pixel_size, GLenum type, glm::vec4 color, void *dst)
{
	color = glm::clamp(color, 0.f, 1.f);
	for (int ci = 0; ci < pixel_size; ci++)
	{
		if (type == GL_BYTE)
			((GLbyte *)dst)[ci] = (GLbyte)((0xFF * color[ci] - 1) / 2);
		else if (type == GL_UNSIGNED_BYTE)
			((GLubyte *)dst)[ci] = (GLubyte)(color[ci] * 0xFF);
		else if (type == GL_SHORT)
			((GLshort *)dst)[ci] = GLshort((0xFFFF * color[ci] - 1) / 2);
		else if (type == GL_UNSIGNED_SHORT)
			((GLushort *)dst)[ci] = (GLushort)(color[ci] * 0xFFFF);
		else if (type == GL_INT)
			((GLint *)dst)[ci] = (GLint)((0xFFFFFFFF * color[ci] - 1) / 2);
		else if (type == GL_UNSIGNED_INT)
			((GLuint *)dst)[ci] = (GLuint)(color[ci] * 0xFFFFFFFF);
		else if (type == GL_FLOAT)
			((float *)dst)[ci] = (float)color[ci];
	}
}

inline void gl_swap_bytes(int element_size, int pixel_size, uint8_t *data)
{
	for (int c = 0; c < pixel_size; c++)
	{
		std::swap(data[c * element_size], data[(c + 1) * element_size - 1]);
		if (element_size == 4)
			std::swap(data[c * element_size + 1], data[c * element_size + 2]);
	}
}
