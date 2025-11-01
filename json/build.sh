#!/usr/bin/env bash

#export QNX_HOST="/home/baiduiov/code/tools/prebuilt_QNX700_r17/host/linux/x86_64"
#export QNX_TARGET="/home/baiduiov/code/tools/prebuilt_QNX700_r17/target/qnx7"
#export QNX_HOST="/home/baiduiov/tool/sdp_toolchain/sdp/qnx710/host/linux/x86_64"
#export QNX_TARGET="/home/baiduiov/tool/sdp_toolchain/sdp/qnx710/target/qnx7"
#export QNX_HOST="/home/baiduiov/tool/qnx/qsdk700r17/host/linux/x86_64"
#export QNX_TARGET="/home/baiduiov/tool/qnx/qsdk700r17/target/qnx7"

if [ "$QNX_HOST" == "" ]; then
  export QNX_HOST="/home/baiduiov/tool/sdp_toolchain/sdp/qnx710/host/linux/x86_64"
  export QNX_TARGET="/home/baiduiov/tool/sdp_toolchain/sdp/qnx710/target/qnx7"
fi

echo "JSON - QNX_HOST = $QNX_HOST"
echo "JSON - QNX_TARGET = $QNX_TARGET"

mkdir -p build-aarch64-qnx-gnu
pushd build-aarch64-qnx-gnu
cmake -DCMAKE_TOOLCHAIN_FILE=/home/baiduiov/03.ProgramSpace/15.CLang/01.WorkSpace/AliceCLang/AliceVisionAbility/aura-cmake/arm-linux-arm7-aixin.toolchain.cmake ..
make -j22
make install
popd
