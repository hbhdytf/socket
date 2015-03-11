LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)
LOCAL_LDLIBS += -L$(SYSROOT)/usr/lib -llog
LOCAL_SRC_FILES:=  elf.c inj_dalvik.c ptrace.c process.c
LOCAL_MODULE := inj_dalvik
LOCAL_MODULE_TAGS := optional
LOCAL_CFLAGS := -DANDROID -DTHUMB
#LOCAL_C_INCLUDES := 
include $(BUILD_EXECUTABLE)



include $(CLEAR_VARS)
LOCAL_LDLIBS += -L$(SYSROOT)/usr/lib -llog
LOCAL_LDLIBS += -L$(SYSROOT)/usr/lib -lcrypto
LOCAL_LDLIBS += -L$(SYSROOT)/usr/lib -lssl
LOCAL_SRC_FILES:=  hook.c process.c
LOCAL_MODULE := libhook
LOCAL_MODULE_TAGS := optional
#LOCAL_C_INCLUDES := 
include $(BUILD_SHARED_LIBRARY)


