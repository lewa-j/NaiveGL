
cmake_minimum_required(VERSION 3.7)

project(NaiveGL CXX)

set(CMAKE_CXX_STANDARD 17)

#set(NGL_VERISON 100)
set(NGL_VERISON 110)

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
	)

if(${NGL_VERISON} GREATER_EQUAL 110)
	target_sources(opengl32 PUBLIC
		win/opengl32_110.def)
else()
	target_sources(opengl32 PUBLIC
		win/opengl32.def)
endif()

target_include_directories(opengl32 PUBLIC
	${PROJECT_SOURCE_DIR}
	${PROJECT_SOURCE_DIR}/../deps/glm
)

target_compile_definitions(opengl32 PUBLIC NGL_VERISON=${NGL_VERISON})

