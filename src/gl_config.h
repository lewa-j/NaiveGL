#pragma once

#ifndef NGL_VERISON
	#define NGL_VERISON 110
#endif

#define NGL_MINIMAL 0
#define NGL_DEBUG_LOG 1

#ifdef ANDROID
	#define NGL_FLIP_VIEWPORT_Y 1
#endif

#define GL_EXT_polygon_offset 1
#define GL_EXT_texture 1
#define GL_EXT_subtexture 1
#define GL_EXT_vertex_array 1
#define GL_EXT_blend_logic_op 1

// checks
#if GL_EXT_subtexture && !GL_EXT_texture
#error "GL_EXT_subtexture depends on GL_EXT_texture"
#endif
