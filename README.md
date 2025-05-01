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

## Acknowledgments

specification [OpenGL 1.0](https://registry.khronos.org/OpenGL/specs/gl/glspec10.pdf) [OpenGL 1.1](https://registry.khronos.org/OpenGL/specs/gl/glspec11.pdf)

generated headers [glad](https://github.com/Dav1dde/glad)

triangle clipping [McGuire2011Clipping](https://casual-effects.com/research/McGuire2011Clipping/index.html)

line and triangle rasterization [tinyrenderer](https://github.com/ssloy/tinyrenderer)

dither [Algorithm for generating a rectangle-shaped matrix](https://bisqwit.iki.fi/story/howto/dither/jy/)
