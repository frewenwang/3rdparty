#
# Copyright (c) 2020-2022 Qualcomm Technologies, Inc.
# All Rights Reserved.
# Confidential and Proprietary - Qualcomm Technologies, Inc.
#

# define default
default: all

# define package name
PACKAGE_NAME := $(notdir $(shell pwd))

# define library prerequisites list
sample_app := src
make_dir := make
EXE_SOURCES = $(sample_app)

# define target_architecture
export TARGET_AARCH_VARS:= -march=x86-64

# define target name
export TARGET = linux-x86_64

# specify compiler
export CXX := clang++

.PHONY: all $(EXE_SOURCES) setup_flatbuffers_include_folder all_x86 all_android
all: $(EXE_SOURCES) setup_flatbuffers_include_folder all_x86 all_android   

#setup include folders for flatbuffers
setup_flatbuffers_include_folder:
	@mkdir -p include
	@mkdir -p include/flatbuffers
	@ln -sf /usr/include/flatbuffers include/flatbuffers

# Combined Targets
clean: clean_x86 clean_android clean_qnx

all_x86: setup_flatbuffers_include_folder
	$(call build_if_exists,$(sample_app),-$(MAKE) -f $(make_dir)/Makefile.linux-x86_64)

clean_x86:
	@rm -rf bin obj include

# Android Targets

all_android: setup_flatbuffers_include_folder aarch64-android arm-android

aarch64-android: check_ndk clean_aarch64-android
	$(call build_if_exists,$(sample_app),$(ANDROID_NDK_ROOT)/ndk-build APP_ALLOW_MISSING_DEPS=true APP_ABI="arm64-v8a" NDK_PROJECT_PATH=./ NDK_APPLICATION_MK=$(make_dir)/Application.mk APP_BUILD_SCRIPT=$(make_dir)/Android.mk)
	@$(rename_target_dirs)

arm-android: check_ndk clean_arm-android
	$(call build_if_exists,$(sample_app),$(ANDROID_NDK_ROOT)/ndk-build APP_ALLOW_MISSING_DEPS=true APP_ABI="armeabi-v7a" NDK_PROJECT_PATH=./ NDK_APPLICATION_MK=$(make_dir)/Application.mk APP_BUILD_SCRIPT=$(make_dir)/Android.mk)
	@$(rename_target_dirs)

clean_android: check_ndk clean_arm-android clean_aarch64-android

clean_arm-android:
	@rm -rf bin/arm-android
	@rm -rf obj/local/arm-android

clean_aarch64-android:
	@rm -rf bin/aarch64-android
	@rm -rf obj/local/aarch64-android

# QNX Target

all_qnx: setup_flatbuffers_include_folder check_qnx
	$(call build_if_exists,$(sample_app),-$(MAKE) -f $(make_dir)/Makefile.qnx-aarch64)

clean_qnx:
	@rm -rf bin obj include







# utilities
# Syntax: $(call build_if_exists <dir>,<cmd>)
build_if_exists = $(if $(wildcard $(1)),$(2),$(warning WARNING: $(1) does not exist. Skipping Compilation))
rename_target_dirs = find . -type d -execdir rename 's/arm64-v8a/aarch64-android/' '{}' \+ \
                     && find . -type d -execdir rename 's/armeabi-v7a/arm-android/' '{}' \+ \
                     && mv libs/* bin/  \
                     && rm -rf libs \

check_ndk:
ifeq ($(ANDROID_NDK_ROOT),)
	$(error ERROR: ANDROID_NDK_ROOT not set, skipping compilation for Android platform(s).)
endif

check_qnx:
ifeq ($(QNX_HOST),)
	$(error ERROR: QNX_HOST not set, skipping compilation for QNX platform.)
endif
ifeq ($(QNX_TARGET),)
	$(error ERROR: QNX_TARGET not set, skipping compilation for QNX platform.)
endif
