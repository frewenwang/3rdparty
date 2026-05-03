#!/usr/bin/env bash

# 优化后的ARM64编译脚本，支持自定义选项文件控制
TARGET_NAME="opencv"
TARGET_VERSION="4.11.0"
DIR_INSTALL="../../../lib/v$TARGET_VERSION"
BASH_PATH="./"

# 设置正确的NDK路径
export ANDROID_NDK_HOME="/Users/frewen/Library/AAura/Android/sdk/ndk/26.3.11579264"

# 进行下载和解压的资源的名称
URL_TARGET_ZIP="https://github.com/$TARGET_NAME/$TARGET_NAME/archive/refs/tags/$TARGET_VERSION.zip"
URL_TARGET_CONTRIB_ZIP="https://github.com/opencv/opencv_contrib/archive/refs/tags/$TARGET_VERSION.zip"

ZIP_OPENCV="$BASH_PATH/opencv-$TARGET_VERSION.zip"
ZIP_OPENCV_CONTRIB="$BASH_PATH/opencv_contrib-$TARGET_VERSION.zip"

TARGET_DL_DIR="$BASH_PATH/$TARGET_NAME-$TARGET_VERSION"
TARGET_CONTRIB_DL_DIR="$BASH_PATH/opencv_contrib-$TARGET_VERSION/modules"

#如果zip文件不存在。则进行下载
if [ ! -e "$ZIP_OPENCV" ]; then
    echo "[===Compiler===] start download $URL_TARGET_ZIP"
    wget -O "$ZIP_OPENCV" "$URL_TARGET_ZIP"
else
  echo "[===Compiler===] '$ZIP_OPENCV' File exists. Ignore Download!!!"
fi
if [ ! -e "$ZIP_OPENCV_CONTRIB" ]; then
    echo "[===Compiler===] start download $URL_TARGET_CONTRIB_ZIP"
    wget -O "$ZIP_OPENCV_CONTRIB" "$URL_TARGET_CONTRIB_ZIP"
else
  echo "[===Compiler===] $ZIP_OPENCV_CONTRIB File exists. Ignore Download!!!"
fi
echo "[===Compiler===] target download success !!!"

# 如果解压目录不存在，则进行解压
if [ ! -d "$TARGET_DL_DIR" ]; then
    echo "[===Compiler===] $TARGET_DL_DIR not exist and start unzip:$ZIP_OPENCV"
    unzip -q "$ZIP_OPENCV" -d "$BASH_PATH"
else
  echo "[===Compiler===] $TARGET_DL_DIR Dir exists. Ignore unzip!!!"
fi
if [ ! -d "$TARGET_CONTRIB_DL_DIR" ]; then
    echo "[===Compiler===] $TARGET_CONTRIB_DL_DIR not exist and start unzip:$ZIP_OPENCV_CONTRIB"
    unzip -q "$ZIP_OPENCV_CONTRIB" -d "$BASH_PATH"
else
  echo "[===Compiler===] $TARGET_CONTRIB_DL_DIR Dir exists. Ignore unzip!!!"
fi
echo "[===Compiler===] target unzip success !!!"

# Android arm64-v8a的专用CMake参数
CMAKE_EXTRA_ARGS="-DANDROID_ABI=arm64-v8a -DANDROID_PLATFORM=android-21 -DBUILD_JAVA=OFF -DBUILD_ANDROID_PROJECTS=OFF -DBUILD_ANDROID_EXAMPLES=OFF"

# 使用完整配置的Android选项文件
# 包含最大数量的OpenCV模块
./build.sh  -t  2 -r  -n  "$TARGET_NAME" -v "$TARGET_VERSION" \
    --platform-options "./cmake_args/android_options.txt" \
    --common-options "" \
    -a "$CMAKE_EXTRA_ARGS" \
    -i  "$DIR_INSTALL"