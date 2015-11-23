LOCAL_PATH              := $(call my-dir)
EXTERNALS_INCLUDE       := externals/include
EXTERNALS_LIB           := externals/lib

include $(CLEAR_VARS)
LOCAL_MODULE            := libSDL2
LOCAL_SRC_FILES         := $(EXTERNALS_LIB)/$(TARGET_ARCH_ABI)/libSDL2.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE            := libSDL2_image
LOCAL_SRC_FILES         := $(EXTERNALS_LIB)/$(TARGET_ARCH_ABI)/libSDL2_image.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE            := Stereo4VR
LOCAL_SRC_FILES         := GL4D/aes.c GL4D/bin_tree.c GL4D/gl4dm.c GL4D/gl4dq.c GL4D/gl4droid.c GL4D/gl4du.c GL4D/gl4dummies.c GL4D/list.c GL4D/vector.c AssetTool.c gl_code.c
LOCAL_C_INCLUDES        := $(LOCAL_PATH)/$(EXTERNALS_INCLUDE)
LOCAL_CFLAGS            := -std=c11
LOCAL_LDLIBS            := -landroid -llog -lGLESv2
LOCAL_SHARED_LIBRARIES  := libSDL2 libSDL2_image
include $(BUILD_SHARED_LIBRARY)