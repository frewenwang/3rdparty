# ===============================================================
#
#  Copyright (c) 2021-2022 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#
# ===============================================================

# define default
default: all

make_dir := make

# specify compiler
export CC := clang

# combined targets
all: x86 android   

x86: cpu_x86 htp_x86 hta_x86

android: aarch64 arm

aarch64: cpu_aarch64 gpu_aarch64 dsp_aarch64 htp_aarch64 hta_aarch64

arm: cpu_arm gpu_arm dsp_arm htp_arm hta_arm

qnx: cpu_qnx

clean: clean_x86 clean_arm clean_aarch64

# x86 targets
cpu_x86:
	$(call build_if_exists,saver_output.c,-$(MAKE) -f $(make_dir)/Makefile.linux-x86_64 cpu)

htp_x86:
	$(call build_if_exists,saver_output.c,-$(MAKE) -f $(make_dir)/Makefile.linux-x86_64 htp)

hta_x86:
	$(call build_if_exists,saver_output.c,-$(MAKE) -f $(make_dir)/Makefile.linux-x86_64 hta)

clean_x86:
	@rm -rf x86_64-linux-clang

# qnx targets
cpu_qnx: check_qnx
	$(call build_if_exists,saver_output.c,-$(MAKE) -f $(make_dir)/Makefile.qnx-aarch64 cpu)

clean_qnx:
	@rm -rf aarch64-qnx







# arm targets
cpu_arm: check_ndk
	$(call build_if_exists,saver_output.c,$(ANDROID_NDK_ROOT)/ndk-build APP_ABI="armeabi-v7a" NDK_PROJECT_PATH=./ NDK_APPLICATION_MK=$(make_dir)/Application.mk APP_BUILD_SCRIPT=$(make_dir)/Android.mk QNN_BACKEND="QnnCpu")
	@$(rename_target_dirs)

gpu_arm: check_ndk
	$(call build_if_exists,saver_output.c,$(ANDROID_NDK_ROOT)/ndk-build APP_ABI="armeabi-v7a" NDK_PROJECT_PATH=./ NDK_APPLICATION_MK=$(make_dir)/Application.mk APP_BUILD_SCRIPT=$(make_dir)/Android.mk QNN_BACKEND="QnnGpu")
	@$(rename_target_dirs)

dsp_arm: check_ndk
	$(call build_if_exists,saver_output.c,$(ANDROID_NDK_ROOT)/ndk-build APP_ABI="armeabi-v7a" NDK_PROJECT_PATH=./ NDK_APPLICATION_MK=$(make_dir)/Application.mk APP_BUILD_SCRIPT=$(make_dir)/Android.mk QNN_BACKEND="QnnDsp")
	@$(rename_target_dirs)

htp_arm: check_ndk
	$(call build_if_exists,saver_output.c,$(ANDROID_NDK_ROOT)/ndk-build APP_ABI="armeabi-v7a" NDK_PROJECT_PATH=./ NDK_APPLICATION_MK=$(make_dir)/Application.mk APP_BUILD_SCRIPT=$(make_dir)/Android.mk QNN_BACKEND="QnnHtp")
	@$(rename_target_dirs)

hta_arm: check_ndk
	$(call build_if_exists,saver_output.c,$(ANDROID_NDK_ROOT)/ndk-build APP_ABI="armeabi-v7a" NDK_PROJECT_PATH=./ NDK_APPLICATION_MK=$(make_dir)/Application.mk APP_BUILD_SCRIPT=$(make_dir)/Android.mk QNN_BACKEND="QnnHta")
	@$(rename_target_dirs)

clean_arm:
	@rm -rf arm-android

# aarch64 targets
cpu_aarch64: check_ndk
	$(call build_if_exists,saver_output.c,$(ANDROID_NDK_ROOT)/ndk-build APP_ABI="arm64-v8a" NDK_PROJECT_PATH=./ NDK_APPLICATION_MK=$(make_dir)/Application.mk APP_BUILD_SCRIPT=$(make_dir)/Android.mk QNN_BACKEND="QnnCpu")
	@$(rename_target_dirs)

gpu_aarch64: check_ndk
	$(call build_if_exists,saver_output.c,$(ANDROID_NDK_ROOT)/ndk-build APP_ABI="arm64-v8a" NDK_PROJECT_PATH=./ NDK_APPLICATION_MK=$(make_dir)/Application.mk APP_BUILD_SCRIPT=$(make_dir)/Android.mk QNN_BACKEND="QnnGpu")
	@$(rename_target_dirs)

dsp_aarch64: check_ndk check_hexagon_sdk
	$(call build_if_exists,saver_output.c,$(ANDROID_NDK_ROOT)/ndk-build APP_ABI="arm64-v8a" NDK_PROJECT_PATH=./ NDK_APPLICATION_MK=$(make_dir)/Application.mk APP_BUILD_SCRIPT=$(make_dir)/Android.mk QNN_BACKEND="QnnDsp")
	@$(rename_target_dirs)

htp_aarch64: check_ndk check_hexagon_sdk
	$(call build_if_exists,saver_output.c,$(ANDROID_NDK_ROOT)/ndk-build APP_ABI="arm64-v8a" NDK_PROJECT_PATH=./ NDK_APPLICATION_MK=$(make_dir)/Application.mk APP_BUILD_SCRIPT=$(make_dir)/Android.mk QNN_BACKEND="QnnHtp")
	@$(rename_target_dirs)

hta_aarch64: check_ndk
	$(call build_if_exists,saver_output.c,$(ANDROID_NDK_ROOT)/ndk-build APP_ABI="arm64-v8a" NDK_PROJECT_PATH=./ NDK_APPLICATION_MK=$(make_dir)/Application.mk APP_BUILD_SCRIPT=$(make_dir)/Android.mk QNN_BACKEND="QnnHta")
	@$(rename_target_dirs)

clean_aarch64:
	@rm -rf aarch64-android

# utilities
# Syntax: $(call build_if_exists <dir>,<cmd>)
build_if_exists = $(if $(wildcard $(1)),$(2),$(warning WARNING: $(1) does not exist. Skipping Compilation))
rename_target_dirs = find . -type d -execdir rename 's/arm64-v8a/aarch64-android/' '{}' \+ \
                     && find . -type d -execdir rename 's/armeabi-v7a/arm-android/' '{}' \+ \
                     && rsync -a libs/* ./  \
                     && rm -rf libs obj

check_ndk:
ifeq ($(ANDROID_NDK_ROOT),)
	$(error ERROR: ANDROID_NDK_ROOT not set, skipping compilation for Android platform(s).)
endif

check_hexagon_sdk:
ifeq ($(HEXAGON_SDK_ROOT),)
	$(error ERROR: HEXAGON_SDK_ROOT not set, skipping compilation for DSP/HTP backends on aarch64.)
endif

check_qnx:
ifeq ($(QNX_HOST),)
	$(error ERROR: QNX_HOST not set, skipping compilation for QNX platform.)
endif
ifeq ($(QNX_TARGET),)
	$(error ERROR: QNX_TARGET not set, skipping compilation for QNX platform.)
endif
