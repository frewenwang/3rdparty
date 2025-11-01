# ===============================================================
#
#  Copyright (c) 2021-2022 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#
# ===============================================================

LOCAL_PATH := $(call my-dir)
SUPPORTED_TARGET_ABI := arm64-v8a armeabi-v7a

#============================ Verify Target Info and Application Variables =========================================
ifneq ($(filter $(TARGET_ARCH_ABI),$(SUPPORTED_TARGET_ABI)),)
    ifneq ($(APP_STL), c++_shared)
        $(error Unsupported APP_STL: "$(APP_STL)")
    endif
else
    $(error Unsupported TARGET_ARCH_ABI: '$(TARGET_ARCH_ABI)')
endif

#============================ Define Common Variables ===============================================================
# QNN_SDK_ROOT should be set and points to the SDK path, it will be used.
ifdef QNN_SDK_ROOT
PACKAGE_C_INCLUDES += -I $(QNN_SDK_ROOT)/include
else
$(error QNN_SDK_ROOT: Please set QNN_SDK_ROOT)
endif

ifndef QNN_BACKEND
$(error QNN_BACKEND_NOT_SET)
endif

PACKAGE_MODULE = saver_output_$(QNN_BACKEND)

ifeq ($(TARGET_ARCH_ABI),arm64-v8a)
    PACKAGE_LDLIBS := -L$(QNN_SDK_ROOT)/target/aarch64-android/lib
else ifeq ($(TARGET_ARCH_ABI),armeabi-v7a)
    PACKAGE_LDLIBS := -L$(QNN_SDK_ROOT)/target/arm-android/lib
endif

ifeq ($(QNN_BACKEND),$(filter $(QNN_BACKEND), QnnDsp QnnDsp QnnHtp))
    ifeq ($(TARGET_ARCH_ABI),arm64-v8a)
      PACKAGE_LDLIBS += -L$(HEXAGON_SDK_ROOT)/ipc/fastrpc/remote/ship/android_aarch64 -lcdsprpc
    endif
endif

PACKAGE_LDLIBS += -l$(QNN_BACKEND)

#========================== Define saver_output.c Executable Build Variables =============================================
include $(CLEAR_VARS)
LOCAL_C_INCLUDES := $(PACKAGE_C_INCLUDES)
LOCAL_MODULE     := $(PACKAGE_MODULE)
LOCAL_SRC_FILES  := ../saver_output.c
LOCAL_LDLIBS     := $(PACKAGE_LDLIBS)
include $(BUILD_EXECUTABLE)