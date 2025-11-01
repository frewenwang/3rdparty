#!/usr/bin/env bash

#################################################################################
# 设置环境变量 (通过环境变量或手动设置ANDROID_NDK_HOME路径)

# NDK
if [ "$ANDROID_NDK_HOME" = "" ]; then
    echo "NOTE: ANDROID_NDK_HOME is not set in environment, export by self:"
    export ANDROID_NDK_HOME=/Users/liwendong/tools/adt-bundle/sdk/ndk/19.2.5345600
    echo "ANDROID_NDK_HOME = $ANDROID_NDK_HOME"
    # r18b / 19.2.5345600 / 21.3.6528147
fi

if [ "$QNX_HOST" = "" ]; then
    echo "NOTE: QNX_HOST / QNX_TARGET is not set in environment, export by self:"
    export QNX_HOST="/home/baiduiov/code/tools/prebuilt_QNX700_r17/host/linux/x86_64"
    export QNX_TARGET="/home/baiduiov/code/tools/prebuilt_QNX700_r17/target/qnx7"
    echo "QNX_HOST = $QNX_HOST"
    echo "QNX_TARGET = $QNX_TARGET"
fi


#################################################################################
# default setting
TARGET_INDEX="6"
PRODUCT="mainline"
BUILD_TYPE="release"
use_external_model=false
use_external_encrypt=false
build_aar=false
BUILD_ICP_TARGET_OS=""
BUILD_ICP_TARGET_ARCH=""
BUILD_CMAKE_ARGS=""

#################################################################################
# 0-osx
# 1-android-armv7a
# 2-android-armv8a
# 3-android-x86_64
# 4-android-x86
# 5-android-armv7a and x86 (For QA)
# 6-android-armv8a and x86 (For QA)

show_help() {
    echo "Usage: $0 [option...]" >&2
    echo
    echo "   -r, --release           Set build type to Release [default]"
    echo "   -d, --debug             Set build type to Debug"
    echo "   --RelWithDebInfo        Set build type to RelWithDebInfo"
    echo "   -t, --target            Set build target:"
    echo "                              0 - osx or ubuntu"
    echo "                              1 - android-armv7a"
    echo "                              2 - android-armv8a"
    echo "                              3 - android-x86"
    echo "                              4 - android-x86_64"
    echo "                              5 - qnx-armv7le"
    echo "                              6 - qnx-aarch64"
    echo "                              7 - ios-armv7"
    echo "                              8 - ios-armv8"
    echo "   --ext_model             use external model file"
    echo "   --ext_encrypt           use baidu_protect encryption"
    echo "   --with_aar              build aar lib. [default : False]"
    echo "   -p, --product           Set build product, for example: mainline [default], jidu, ford_cd542, toyota_760, toyota_030d,
                                     chery, hyundai_SGEN5WxC, evergrande, honda_23m ..."
    echo "   -h, --help              show help message"
    echo
}

# parse arguments
while [ $# != 0 ]
do
  case "$1" in
    -a)
        BUILD_CMAKE_ARGS=$2
        shift
        ;;
    -t)
        TARGET_INDEX=$2
        shift
        ;;
    --target)
        TARGET_INDEX=$2
        shift
        ;;
    -p)
        PRODUCT=$2
        shift
        ;;
    --product)
        PRODUCT=$2
        shift
        ;;
    -r)
        BUILD_TYPE="release"
        ;;
    --release)
        BUILD_TYPE="release"
        ;;
    -d)
        BUILD_TYPE="debug"
        ;;
    --debug)
        BUILD_TYPE="debug"
        ;;
    --RelWithDebInfo)
        BUILD_TYPE="relWithDebInfo"
        ;;
    --ext_model)
        use_external_model=true
        ;;
    --ext_encrypt)
        use_external_encrypt=true
        ;;
    --with_aar)
        build_aar=false
        ;;
    -h)
        show_help
        exit 1
        ;;
    --help)
    show_help
        exit 1
        ;;
    *)
        ;;
  esac
  shift
done

case "$TARGET_INDEX" in
0)
    if [ "$(uname)" == "Darwin" ]; then
        BUILD_ICP_TARGET_OS="osx"
        BUILD_ICP_TARGET_ARCH="x86_64"
    elif [ "$(expr substr $(uname -s) 1 5)" == "Linux" ]; then
        BUILD_ICP_TARGET_OS="linux"
        BUILD_ICP_TARGET_ARCH="x86_64"
    fi
    ;;
1)
    BUILD_ICP_TARGET_OS="android"
    BUILD_ICP_TARGET_ARCH="armeabi-v7a"
    ;;
2)
    BUILD_ICP_TARGET_OS="android"
    BUILD_ICP_TARGET_ARCH="arm64-v8a"
    ;;
3)
    BUILD_ICP_TARGET_OS="android"
    BUILD_ICP_TARGET_ARCH="x86"
    ;;
4)
    BUILD_ICP_TARGET_OS="android"
    BUILD_ICP_TARGET_ARCH="x86_64"
    ;;
5)
    BUILD_ICP_TARGET_OS="qnx"
    BUILD_ICP_TARGET_ARCH="armv7le"
    ;;
6)
    BUILD_ICP_TARGET_OS="qnx"
    BUILD_ICP_TARGET_ARCH="aarch64le"
    ;;
7)
    BUILD_ICP_TARGET_OS="ios"
    BUILD_ICP_TARGET_ARCH="armv7"
    ;;
8)
    BUILD_ICP_TARGET_OS="ios"
    BUILD_ICP_TARGET_ARCH="armv8"
    ;;
esac
TARGET=$BUILD_ICP_TARGET_OS-$BUILD_ICP_TARGET_ARCH

echo "===== build target is $TARGET, build type is $BUILD_TYPE"

android_target_tag="android"
if [[ $TARGET == *$android_target_tag* ]]; then
  if [ "$ANDROID_NDK_HOME" = "" ]; then
    echo "ERROR: Please set ANDROID_NDK_HOME environment"
    exit
  fi
  echo "===== ANDROID_NDK_HOME=$ANDROID_NDK_HOME"
  TARGET_TOOLCHAIN=$ANDROID_NDK_HOME/build/cmake/android.toolchain.cmake
elif [[ "$TARGET" == *"qnx"* ]]; then
    TARGET_TOOLCHAIN=./arm-unknown-nto-qnx700eabi.toolchain.cmake
elif [[ "$TARGET" == *"ios"* ]]; then
    TARGET_TOOLCHAIN=./cmake/ios/ios.toolchain.cmake
fi

echo "TARGET_TOOLCHAIN=$TARGET_TOOLCHAIN"

USE_EXTERNAL_MODEL=false
if [ "$use_external_model" = true ]; then
  USE_EXTERNAL_MODEL=true
fi

# create build dir if not exists
if [ ! -d build ]; then
    mkdir -p build
fi
cd build


buildNative(){
  BUILD_DIR="$TARGET-$BUILD_TYPE"
  echo "===== ------ Begin to build libvision (${BUILD_DIR}) ------"
  if [ ! -d $BUILD_DIR ]; then
      mkdir -p $BUILD_DIR
  fi
  cd $BUILD_DIR

  # compile & install

  echo "===== cmake target: qnx-aarch64le"
   cmake   -DICP_TARGET_OS=qnx \
           -DICP_TARGET_ARCH=aarch64le \
           -DCMAKE_BUILD_TYPE=$BUILD_TYPE \
           -DCMAKE_TOOLCHAIN_FILE=$TARGET_TOOLCHAIN \
           ../..

  make -j 12
  echo "===== make libvision finished!"
}

buildNative

exit 0
