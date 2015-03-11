LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := TestA
LOCAL_SRC_FILES := TestA.cpp

include $(BUILD_SHARED_LIBRARY)
