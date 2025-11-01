#!/usr/bin/env bash

#################################################################################
# 基础的默认配置
I_TARGET_INDEX="0"
I_BUILD_TYPE="release"
I_BUILD_TARGET_OS=""
I_BUILD_CMAKE_ARGS=""
I_TARGET_TOOLCHAIN=""
I_HOST_OS="macos"
I_HOST_ARCH="x86_64"

# 自定义的默认配置
I_BUILD_NAME=""  #  编译产出
I_BUILD_VERSION=""  # 变异版本
I_INSTALL_PATH="./"
I_BUILD_TARGET_ARCH=""
I_EXTRA_MODULES_PATH=""

#################################################################################
# 设置环境变量 (通过环境变量或手动设置ANDROID_NDK_HOME路径)

#  判断当前编译宿主机的HOST_OS和
if [ "$(uname)" == "Darwin" ]; then
    I_HOST_OS="macos"
    I_HOST_ARCH="x86_64"
elif [ "$(expr substr $(uname -s) 1 5)" == "Linux" ]; then
    I_HOST_OS="linux"
    I_HOST_ARCH="x86_64"
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
        I_BUILD_CMAKE_ARGS=$2
        shift
        ;;
    -t)
        I_TARGET_INDEX=$2
        shift
        ;;
    --target)
        I_TARGET_INDEX=$2
        shift
        ;;
    -r)
        I_BUILD_TYPE="release"
        ;;
    --release)
        I_BUILD_TYPE="release"
        ;;
    -d)
        I_BUILD_TYPE="debug"
        ;;
    --debug)
        I_BUILD_TYPE="debug"
        ;;
    --RelWithDebInfo)
        I_BUILD_TYPE="relWithDebInfo"
        ;;
    -install)
        I_INSTALL_PATH=$2
        shift
        ;;
    -i)
        I_INSTALL_PATH=$2
        shift
        ;;
    -n)
        I_BUILD_NAME=$2
        shift
        ;;
    -v)
        I_BUILD_VERSION=$2
        shift
        ;;
    -e)
        I_EXTRA_MODULES_PATH=$2
        shift
        ;;
    -extral)
        I_EXTRA_MODULES_PATH=$2
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

case "$I_TARGET_INDEX" in
0)
    if [ "$I_HOST_OS" == "macos" ]; then
        I_BUILD_TARGET_OS="macos"
        I_BUILD_TARGET_ARCH="x86_64"
    elif [ "$I_HOST_OS" == "linux" ]; then
        I_BUILD_TARGET_OS="linux"
        I_BUILD_TARGET_ARCH="x86_64"
    fi
    ;;
1)
    I_BUILD_TARGET_OS="android"
    I_BUILD_TARGET_ARCH="armeabi-v7a"
    ;;
2)
    I_BUILD_TARGET_OS="android"
    I_BUILD_TARGET_ARCH="arm64-v8a"
    ;;
3)
    I_BUILD_TARGET_OS="android"
    I_BUILD_TARGET_ARCH="x86"
    ;;
4)
    I_BUILD_TARGET_OS="android"
    I_BUILD_TARGET_ARCH="x86_64"
    ;;
5)
    I_BUILD_TARGET_OS="qnx"
    I_BUILD_TARGET_ARCH="armv7le"
    ;;
6)
    I_BUILD_TARGET_OS="qnx"
    I_BUILD_TARGET_ARCH="aarch64le"
    ;;
7)
    I_BUILD_TARGET_OS="ios"
    I_BUILD_TARGET_ARCH="x86_64"
    ;;
8)
    I_BUILD_TARGET_OS="ios"
    I_BUILD_TARGET_ARCH="arm64"
    ;;
9)
    I_BUILD_TARGET_OS="ax620a"
    I_BUILD_TARGET_ARCH="arm7"
    ;;    
esac

I_TARGET=$I_BUILD_TARGET_OS-$I_BUILD_TARGET_ARCH

if [[ $I_TARGET == *"android"* ]]; then
  # 编译Android版本，需要看一下DNK的环境变量
  if [ "$ANDROID_NDK_HOME" = "" ]; then
      echo "[===Compiler===] ANDROID_NDK_HOME is not set in environment!!!"
      exit 0
  fi
  I_TARGET_TOOLCHAIN=$ANDROID_NDK_HOME/build/cmake/android.toolchain.cmake
elif [[ "$I_TARGET" == *"qnx"* ]]; then
    if [ "$QNX_HOST" = "" ]; then
        echo "NOTE: QNX_HOST / QNX_TARGET is not set in environment!!!"
        exit 0
    fi
    I_TARGET_TOOLCHAIN=cmake/arm-unknown-nto-qnx700eabi.toolchain.cmake
elif [[ "$I_TARGET" == *"ios"* ]]; then
    I_TARGET_TOOLCHAIN=cmake/ios/ios.toolchain.cmake
elif [[ "$I_TARGET" == *"ax620a"* ]]; then
    I_TARGET_TOOLCHAIN=/home/baiduiov/03.ProgramSpace/20.AI/01.WorkSpace/opencv-library/cmake/arm-linux-arm7-aixin.toolchain.cmake  
fi
# 初始化编译环境
echo "[===Compiler===] build target:$I_TARGET, build type:$I_BUILD_TYPE, toolchain:$I_TARGET_TOOLCHAIN"

# create build dir if not exists
if [ ! -d build ]; then
    mkdir -p build
fi
cd build


buildTarget(){
    BUILD_DIR="$I_TARGET-$I_BUILD_TYPE"
    echo "[===Compiler===] begin build output in (${BUILD_DIR}) "
    if [ ! -d $BUILD_DIR ]; then
        mkdir -p $BUILD_DIR
    fi
    cd $BUILD_DIR

    cmake   -D TARGET_OS=$I_BUILD_TARGET_OS \
            -D TARGET_ARCH=$I_BUILD_TARGET_ARCH \
            -D HOST_OS=$I_HOST_OS \
            -D HOST_ARCH=$I_HOST_ARCH \
            -D CMAKE_BUILD_TYPE=$I_BUILD_TYPE \
            -D CMAKE_TOOLCHAIN_FILE="$I_TARGET_TOOLCHAIN" \
            -D CMAKE_INSTALL_PREFIX="$I_INSTALL_PATH"/$BUILD_DIR \
            "$I_BUILD_CMAKE_ARGS" \
            ../../"$I_BUILD_NAME"-"$I_BUILD_VERSION"/

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
