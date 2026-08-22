# NaiveGL

Naive implementation of OpenGL 1.0 and 1.1 in C++

Almost complete 1.1 feature set, except

* Wide lines
* Antialiasing
* Texture border
* FRONT and AUX color buffers
* Clear color dithering
* ReadPixels BITMAP
* Index color mode

To build only specific version, set define `NGL_VERSION` to `100` for 1.0 or `110` for 1.1  
Also for 1.0 set Module Definition File to `win/opengl32_100.def` in Linker > Input.

## Extensions

* [GL_EXT_vertex_array](https://registry.khronos.org/OpenGL/extensions/EXT/EXT_vertex_array.txt)
* [GL_EXT_polygon_offset](https://registry.khronos.org/OpenGL/extensions/EXT/EXT_polygon_offset.txt)
* [GL_EXT_blend_logic_op](https://registry.khronos.org/OpenGL/extensions/EXT/EXT_blend_logic_op.txt)
* [GL_EXT_texture](https://registry.khronos.org/OpenGL/extensions/EXT/EXT_texture.txt)
* [GL_EXT_subtexture](https://registry.khronos.org/OpenGL/extensions/EXT/EXT_subtexture.txt)
* [GL_EXT_copy_texture](https://registry.khronos.org/OpenGL/extensions/EXT/EXT_copy_texture.txt)
* [GL_EXT_texture_object](https://registry.khronos.org/OpenGL/extensions/EXT/EXT_texture_object.txt)
* [GL_EXT_draw_range_elements](https://registry.khronos.org/OpenGL/extensions/EXT/EXT_draw_range_elements.txt)

## Acknowledgments

specification [OpenGL 1.0](https://registry.khronos.org/OpenGL/specs/gl/glspec10.pdf) [OpenGL 1.1](https://registry.khronos.org/OpenGL/specs/gl/glspec11.pdf)

generated headers [glad](https://github.com/Dav1dde/glad)

triangle clipping [McGuire2011Clipping](https://casual-effects.com/research/McGuire2011Clipping/index.html)

line and triangle rasterization [tinyrenderer](https://github.com/ssloy/tinyrenderer)

dither [Algorithm for generating a rectangle-shaped matrix](https://bisqwit.iki.fi/story/howto/dither/jy/)
