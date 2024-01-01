
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_CPP_EXTENSION := .cpp .cc
LOCAL_MODULE    := opengl_10
LOCAL_SRC_FILES := gl_begin.cpp \
gl_clip.cpp \
gl_export.cpp \
gl_framebuffer.cpp \
gl_lighting.cpp \
gl_matrix_stack.cpp \
gl_pixels.cpp \
gl_rasterization.cpp \
gl_state.cpp \
gl_texgen.cpp \
gl_viewport.cpp \
headless/context.cpp

LOCAL_C_INCLUDES := $(LOCAL_PATH)/../include $(LOCAL_PATH)/../../glm
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/../include

include $(BUILD_STATIC_LIBRARY)

