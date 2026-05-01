#!/usr/bin/env bash

# 注意此脚本需要在build.sh所在的目录下进行执行。
# sh ./scripts/build-mac-x86_64-release.sh

TARGET_NAME="opencv"
TARGET_VERSION="4.11.0"
DIR_INSTALL="./lib/v$TARGET_VERSION"
BASH_PATH="./src"

# 进行下载和解压的资源的名称
# 下载路径：https://github.com/opencv/opencv/archive/refs/tags/4.11.0.zip
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



#shellcheck disable=SC2046
./build.sh  -t  2 -r  -n  "$TARGET_NAME" -v "$TARGET_VERSION" -a $(cat ./cmake_args/common_options.txt)  -i  "$DIR_INSTALL"
