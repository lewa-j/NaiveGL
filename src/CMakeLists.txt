
cmake_minimum_required(VERSION 3.5)

project(NaiveGL CXX)

set(CMAKE_CXX_STANDARD 17)

add_library (opengl32 SHARED
	gl_begin.cpp
	gl_clip.cpp
	gl_display_lists.cpp
	gl_evaluators.cpp
	gl_export.cpp
	gl_feedback.cpp
	gl_framebuffer.cpp
	gl_lighting.cpp
	gl_matrix_stack.cpp
	gl_pixels.cpp
	gl_rasterization.cpp
	gl_selection.cpp
	gl_state.cpp
	gl_state_requests.cpp
	gl_texgen.cpp
	gl_texture.cpp
	gl_viewport.cpp
	win/dllmain.cpp
	win/wgl_context.cpp
	win/opengl32.def
	)

target_include_directories(opengl32 PUBLIC
	${PROJECT_SOURCE_DIR}
	${PROJECT_SOURCE_DIR}/../deps/glm
)