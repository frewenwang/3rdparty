#!/usr/bin/env bash

# 注意此脚本需要在build.sh所在的目录下进行执行。
# sh ./scripts/build-mac-x86_64-release.sh

TARGET_VERSION="2.2.0"
TARGET_NAME="ceres-solver"
TARGET_URL="https://github.com/$TARGET_NAME/$TARGET_NAME/archive/refs/tags/$TARGET_VERSION.zip"

TARGET_DL_ZIP="$TARGET_NAME-$TARGET_VERSION.zip"
TARGET_DL_DIR="$TARGET_NAME-$TARGET_VERSION"


# 先删除原来下载的产出，以及产出解压的路径
#rm -rf "$TARGET_DL_ZIP"
#rm -rf "$TARGET_DL_DIR"


#如果zip文件不存在。则进行下载
if [ ! -e "$TARGET_DL_ZIP" ]; then
    echo "[===Compiler===] start download $TARGET_URL"
    wget -O "$TARGET_DL_ZIP" "$TARGET_URL"
else
  echo "[===Compiler===] $TARGET_DL_ZIP File exists. Ignore Download!!!"
fi

# 如果解压目录不存在，则进行解压
if [ ! -d "$TARGET_DL_DIR" ]; then
    echo "[===Compiler===] $TARGET_DL_DIR not exist and start unzip $TARGET_DL_ZIP"
    unzip -q "$TARGET_DL_ZIP"
else
  echo "[===Compiler===] $TARGET_DL_DIR Dir exists. Ignore unzip!!!"
fi

echo "[===Compiler===] target download success !!!"

DIR_INSTALL="../../../lib"

#shellcheck disable=SC2046
./build.sh  -r  -n  "$TARGET_NAME" -v "$TARGET_VERSION" -a $(cat ./cmake_args/options.txt)  -i  "$DIR_INSTALL"
