LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := vendor.mokee.livedisplay@2.0-init.sh

LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(TARGET_OUT_VENDOR_EXECUTABLES)

LOCAL_SRC_FILES := vendor.mokee.livedisplay@2.0-init.sh

include $(BUILD_PREBUILT)
