LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_CPP_EXTENSION := .cpp .cc

ifeq ($(TARGET_ARCH_ABI), armeabi-v7a)
    LOCAL_MODULE := GameInfoOverlay
else
    LOCAL_MODULE := GameInfoOverlay64
endif

LOCAL_SRC_FILES := main.cpp \
                   mod/logger.cpp \
                   mod/config.cpp

LOCAL_C_INCLUDES += $(LOCAL_PATH)

LOCAL_CFLAGS += -O2 -DNDEBUG
LOCAL_CXXFLAGS += -O2 -DNDEBUG -std=c++17

LOCAL_LDLIBS += -llog

include $(BUILD_SHARED_LIBRARY)
