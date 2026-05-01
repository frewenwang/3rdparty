#!/usr/bin/env bash

#################################################################################
# 基础的默认配置
TARGET_INDEX="0"
BUILD_TYPE="release"
BUILD_TARGET_OS=""
BUILD_CMAKE_ARGS=""
TARGET_TOOLCHAIN=""
HOST_OS="mac"
HOST_ARCH="x86_64"

# 自定义的默认配置
BUILD_NAME=""  #  编译产出
BUILD_VERSION=""  # 变异版本
INSTALL_PATH="./"
BUILD_TARGET_ARCH=""
EXTRA_MODULES_PATH=""

#################################################################################
# 设置环境变量 (通过环境变量或手动设置ANDROID_NDK_HOME路径)

#  判断当前编译宿主机的HOST_OS和
if [ "$(uname)" == "Darwin" ]; then
    HOST_OS="mac"
    HOST_ARCH="x86_64"
elif [ "$(expr substr $(uname -s) 1 5)" == "Linux" ]; then
    HOST_OS="linux"
    HOST_ARCH="x86_64"
fi

#################################################################################
# 0- same as host os
# 1-android-armv7a
# 2-android-armv8a
# 3-android-x86_64
# 4-android-x86
# 5-android-armv7a and x86 (For QA)
# 6-android-armv8a and x86 (For QA)
# 9-ax620a-arm7

show_help() {
    echo "Usage: $0 [option...]" >&2
    echo
    echo "   -r, --release           Set build type to Release [default]"
    echo "   -d, --debug             Set build type to Debug"
    echo "   --RelWithDebInfo        Set build type to RelWithDebInfo"
    echo "   -t, --target            Set build target:"
    echo "                              0 - osx or ubuntu,same as host_os and host_arch"
    echo "                              1 - android-armv7a"
    echo "                              2 - android-armv8a"
    echo "                              3 - android-x86"
    echo "                              4 - android-x86_64"
    echo "                              5 - qnx-armv7le"
    echo "                              6 - qnx-aarch64"
    echo "                              7 - ios-armv7"
    echo "                              8 - ios-armv8"
    echo "                              9 - ax720a-arm7"
    echo "   -c, --toolchain         Target compile toolchain"
    echo "   -i, --install           Target install path"
    echo "   -e, --extral            Extral module path"
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
    -install)
        INSTALL_PATH=$2
        shift
        ;;
    -i)
        INSTALL_PATH=$2
        shift
        ;;
    -n)
        BUILD_NAME=$2
        shift
        ;;
    -v)
        BUILD_VERSION=$2
        shift
        ;;
    -e)
        EXTRA_MODULES_PATH=$2
        shift
        ;;
    -extral)
        EXTRA_MODULES_PATH=$2
        shift
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
    if [ "$HOST_OS" == "mac" ]; then
        BUILD_TARGET_OS="mac"
        BUILD_TARGET_ARCH="x86_64"
    elif [ "$HOST_OS" == "linux" ]; then
        BUILD_TARGET_OS="linux"
        BUILD_TARGET_ARCH="x86_64"
    fi
    ;;
1)
    BUILD_TARGET_OS="android"
    BUILD_TARGET_ARCH="armeabi-v7a"
    ;;
2)
    BUILD_TARGET_OS="android"
    BUILD_TARGET_ARCH="arm64-v8a"
    ;;
3)
    BUILD_TARGET_OS="android"
    BUILD_TARGET_ARCH="x86"
    ;;
4)
    BUILD_TARGET_OS="android"
    BUILD_TARGET_ARCH="x86_64"
    ;;
5)
    BUILD_TARGET_OS="qnx"
    BUILD_TARGET_ARCH="armv7le"
    ;;
6)
    BUILD_TARGET_OS="qnx"
    BUILD_TARGET_ARCH="aarch64le"
    ;;
7)
    BUILD_TARGET_OS="ios"
    BUILD_TARGET_ARCH="x86_64"
    ;;
8)
    BUILD_TARGET_OS="ios"
    BUILD_TARGET_ARCH="arm64"
    ;;
9)
    BUILD_TARGET_OS="ax620a"
    BUILD_TARGET_ARCH="arm7"
    ;;    
esac

TARGET=$BUILD_TARGET_OS-$BUILD_TARGET_ARCH

if [[ $TARGET == *"android"* ]]; then
  # 编译Android版本，需要看一下DNK的环境变量
  if [ "$ANDROID_NDK_HOME" = "" ]; then
      echo "[===Compiler===] ANDROID_NDK_HOME is not set in environment!!!"
      exit 0
  fi
  TARGET_TOOLCHAIN=$ANDROID_NDK_HOME/build/cmake/android.toolchain.cmake
elif [[ "$TARGET" == *"qnx"* ]]; then
    if [ "$QNX_HOST" = "" ]; then
        echo "NOTE: QNX_HOST / QNX_TARGET is not set in environment!!!"
        exit 0
    fi
    TARGET_TOOLCHAIN=cmake/arm-unknown-nto-qnx700eabi.toolchain.cmake
elif [[ "$TARGET" == *"ios"* ]]; then
    TARGET_TOOLCHAIN=cmake/ios/ios.toolchain.cmake
elif [[ "$TARGET" == *"ax620a"* ]]; then
    TARGET_TOOLCHAIN=/home/baiduiov/03.ProgramSpace/20.AI/01.WorkSpace/opencv-library/cmake/arm-linux-arm7-aixin.toolchain.cmake  
fi
# 初始化编译环境
echo "[===Compiler===] build target:$TARGET, build type:$BUILD_TYPE, toolchain:$TARGET_TOOLCHAIN"

# create build dir if not exists
if [ ! -d build ]; then
    mkdir -p build
fi

cd build


buildTarget(){
    BUILD_DIR="$TARGET-$BUILD_TYPE"
    echo "[===Compiler===] begin build target in (${BUILD_DIR}) "
    if [ ! -d $BUILD_DIR ]; then
        mkdir -p $BUILD_DIR
    fi
    cd $BUILD_DIR

    cmake   -D TARGET_OS=$BUILD_TARGET_OS \
            -D TARGET_ARCH=$BUILD_TARGET_ARCH \
            -D HOST_OS=$HOST_OS \
            -D HOST_ARCH=$HOST_ARCH \
            -D CMAKE_BUILD_TYPE=$BUILD_TYPE \
            -D CMAKE_TOOLCHAIN_FILE="$TARGET_TOOLCHAIN" \
            -D CMAKE_INSTALL_PREFIX="$INSTALL_PATH"/$BUILD_DIR \
            "$BUILD_CMAKE_ARGS" \
            ../../src/"$BUILD_NAME"-"$BUILD_VERSION"/

    if [ "$(uname)" == "Darwin" ]; then
        cpu="$(sysctl -n hw.ncpu)"
    elif [ "$(expr substr $(uname -s) 1 5)" == "Linux" ]; then
        cpu=$(cat /proc/cpuinfo | grep processor | wc -l)
    fi
    
    threads=$(( $cpu - 2 ));
    echo "[===Compiler===] make target with $threads threads"
    make -j "$threads"
    echo "[===Compiler===] make target success!!!"
    make install
    echo "[===Compiler===] install target success!!!"
    cd -
}

# 开始编译产出
buildTarget

cd ..


exit 0
