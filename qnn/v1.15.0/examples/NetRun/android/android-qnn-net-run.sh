#!/usr/bin/env bash

#
#  Copyright (c) 2020-2022 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#

# This script makes the assumption that there is only one Android device connected at a time.
# If the user would like to execute this script in an environment containing multiple
# devices, ANDROID_SERIAL environment variable will have to be set accordingly before
# executing this script, or any other method that lets the adb server to appropriately
# recognize the intended device.

set -e
#set -x

helpMessage(){
cat << EOF
Usage: $(basename -- $0) [-h] [-b BACKEND_TO_USE] [-o OUTPUT_DIR] [-a ARCHITECTURE]

Required argument(s):
 -b BACKEND_NAME                       Either cpu, gpu, htp-v68, htp-v69, htp-v73, dsp-v66, dsp-v65, or saver

optional argument(s):
 -o OUTPUT_DIR                         Location for saving output files. Default: Current Directory
 -a ARCHITECTURE                       Architecture to use. Possible options are 'arm' for 'armeabi-v7a'
                                       and 'aarch64' for 'arm64-v8a'. Default is 'aarch64'.
EOF
}

SCRIPT_LOCATION=`dirname $(readlink -f ${0})`
QNN_SDK_ROOT=${SCRIPT_LOCATION}/../../..
OUTPUT_DIR=${SCRIPT_LOCATION}
architecture="aarch64"
while getopts "h?b:o:a:" opt;
do
  case "$opt" in
    h )
      helpMessage;
      exit 1
      ;;
    b )
      backend=$OPTARG
      retVal=0
      ;;
    o )
      OUTPUT_DIR=`readlink -f $OPTARG`
      ;;
    a )
      architecture=$OPTARG
      ;;
    ? )
      helpMessage;
      exit 1
      ;;
  esac
done

if [[ -z "$backend" ]]
then
  echo "ERROR: No Backend Specified.";
  helpMessage;
  retVal=1
fi

if [[ "$backend" != "cpu" && "$backend" != "gpu" && "$backend" != "htp-v68" && "$backend" != "htp-v69" && "$backend" != "htp-v73" && "$backend" != "dsp-v66" && "$backend" != "dsp-v65" && "$backend" != "saver" ]];
then
  echo "ERROR: Invalid Backend Specified."
  helpMessage
  exit 1
fi

if [[ "$architecture" == "aarch64" ]]
then
  arch_variant="aarch64-android"
  cpp_lib="arm64-v8a"
  VENDOR_LIB="/vendor/lib64/"
elif [[ "$architecture" == "arm" ]]
then
  arch_variant="arm-android"
  cpp_lib="armeabi-v7a"
  VENDOR_LIB="/vendor/lib/"
else
  echo "ERROR: Wrong Architecture Provided."
  helpMessage
  exit 1
fi

SCRIPT_LOCATION=`dirname $(readlink -f ${0})`
QNN_SDK_ROOT=${SCRIPT_LOCATION}/../../..

set --
source ${QNN_SDK_ROOT}/target/x86_64-linux-clang/bin/envsetup.sh
unset --

MODEL_ROOT=${QNN_SDK_ROOT}/examples/converter/models
if [[ "$backend" == "htp-v68" ||"$backend" == "htp-v69" || "$backend" == "htp-v73" || "$backend" == "dsp-v66" || "$backend" == "dsp-v65" ]]
then
  MODEL=qnn_model_8bit_quantized
else
  MODEL=qnn_model_float
fi

MODEL_LIBS=${SCRIPT_LOCATION}/model_libs

echo "INFO: Running QNN Model Lib Generator."
export PATH=${ANDROID_NDK_ROOT}:${PATH}
QNN_MODEL_LIB_GENERATOR=${QNN_SDK_ROOT}/target/x86_64-linux-clang/bin/qnn-model-lib-generator
${QNN_MODEL_LIB_GENERATOR} -c ${MODEL_ROOT}/${MODEL}.cpp -b ${MODEL_ROOT}/${MODEL}.bin -o ${MODEL_LIBS} -t ${arch_variant}

TARGET_ROOT=/data/local/tmp/qnn/${MODEL}
# Push inputs, generated model, and executable
echo "INFO: Pushing required files onto device."
adb shell "mkdir -p "${TARGET_ROOT}
adb push ${QNN_SDK_ROOT}/target/${arch_variant}/bin/qnn-net-run ${TARGET_ROOT}
adb push ${ANDROID_NDK_ROOT}/sources/cxx-stl/llvm-libc++/libs/${cpp_lib}/libc++_shared.so ${TARGET_ROOT}
adb push ${MODEL_LIBS}/${arch_variant}/lib${MODEL}.so ${TARGET_ROOT}
adb push ${MODEL_ROOT}/input_list_float.txt ${TARGET_ROOT}
adb push ${MODEL_ROOT}/input_data_float ${TARGET_ROOT}

case "$backend" in
  cpu )
    adb push ${QNN_SDK_ROOT}/target/${arch_variant}/lib/libQnnCpu.so ${TARGET_ROOT}
    QNN_NET_RUN_CMD="./qnn-net-run --model lib"${MODEL}".so --input_list input_list_float.txt --backend libQnnCpu.so"
    ;;
  saver )
    adb push ${QNN_SDK_ROOT}/target/${arch_variant}/lib/libQnnSaver.so ${TARGET_ROOT}
    QNN_NET_RUN_CMD="./qnn-net-run --model lib"${MODEL}".so --input_list input_list_float.txt --backend libQnnSaver.so"
    ;;
  gpu )
    adb push ${QNN_SDK_ROOT}/target/${arch_variant}/lib/libQnnGpu.so ${TARGET_ROOT}
    QNN_NET_RUN_CMD="./qnn-net-run --model lib"${MODEL}".so --input_list input_list_float.txt --backend libQnnGpu.so"
    ;;
  htp-v68 )
    adb push ${QNN_SDK_ROOT}/target/${arch_variant}/lib/libQnnHtp.so ${TARGET_ROOT}
    adb wait-for-device push ${QNN_SDK_ROOT}/target/${arch_variant}/lib/libQnnHtpPrepare.so ${TARGET_ROOT}
    adb wait-for-device push ${QNN_SDK_ROOT}/target/${arch_variant}/lib/libQnnHtpV68Stub.so ${TARGET_ROOT}
    adb wait-for-device push ${QNN_SDK_ROOT}/target/hexagon-v68/lib/unsigned/libQnnHtpV68Skel.so ${TARGET_ROOT}
    ADSP_LIBRARY_PATH="export ADSP_LIBRARY_PATH=\""${TARGET_ROOT}":/vendor/dsp/cdsp:/vendor/lib/rfsa/adsp:/system/lib/rfsa/adsp:/dsp\""
    LD_LIBRARY_PATH="export LD_LIBRARY_PATH=/vendor/dsp/cdsp:"${VENDOR_LIB}":\$LD_LIBRARY_PATH"
    QNN_NET_RUN_CMD="${ADSP_LIBRARY_PATH} && ${LD_LIBRARY_PATH} && ""./qnn-net-run --model lib"${MODEL}".so --input_list input_list_float.txt --backend libQnnHtp.so"
    ;;
  htp-v69 )
    adb push ${QNN_SDK_ROOT}/target/${arch_variant}/lib/libQnnHtp.so ${TARGET_ROOT}
    adb wait-for-device push ${QNN_SDK_ROOT}/target/${arch_variant}/lib/libQnnHtpPrepare.so ${TARGET_ROOT}
    adb wait-for-device push ${QNN_SDK_ROOT}/target/${arch_variant}/lib/libQnnHtpV69Stub.so ${TARGET_ROOT}
    adb wait-for-device push ${QNN_SDK_ROOT}/target/hexagon-v69/lib/unsigned/libQnnHtpV69Skel.so ${TARGET_ROOT}
    ADSP_LIBRARY_PATH="export ADSP_LIBRARY_PATH=\""${TARGET_ROOT}":/vendor/dsp/cdsp:/vendor/lib/rfsa/adsp:/system/lib/rfsa/adsp:/dsp\""
    LD_LIBRARY_PATH="export LD_LIBRARY_PATH="${TARGET_ROOT}":/vendor/dsp/cdsp:"${VENDOR_LIB}":\$LD_LIBRARY_PATH"
    QNN_NET_RUN_CMD="${ADSP_LIBRARY_PATH} && ${LD_LIBRARY_PATH} && ""./qnn-net-run --model lib"${MODEL}".so --input_list input_list_float.txt --backend libQnnHtp.so"
    ;;
  htp-v73 )
    adb push ${QNN_SDK_ROOT}/target/${arch_variant}/lib/libQnnHtp.so ${TARGET_ROOT}
    adb wait-for-device push ${QNN_SDK_ROOT}/target/${arch_variant}/lib/libQnnHtpPrepare.so ${TARGET_ROOT}
    adb wait-for-device push ${QNN_SDK_ROOT}/target/${arch_variant}/lib/libQnnHtpV73Stub.so ${TARGET_ROOT}
    adb wait-for-device push ${QNN_SDK_ROOT}/target/hexagon-v73/lib/unsigned/libQnnHtpV73Skel.so ${TARGET_ROOT}
    ADSP_LIBRARY_PATH="export ADSP_LIBRARY_PATH=\""${TARGET_ROOT}":/vendor/dsp/cdsp:/vendor/lib/rfsa/adsp:/system/lib/rfsa/adsp:/dsp\""
    LD_LIBRARY_PATH="export LD_LIBRARY_PATH="${TARGET_ROOT}":/vendor/dsp/cdsp:"${VENDOR_LIB}":\$LD_LIBRARY_PATH"
    QNN_NET_RUN_CMD="${ADSP_LIBRARY_PATH} && ${LD_LIBRARY_PATH} && ""./qnn-net-run --model lib"${MODEL}".so --input_list input_list_float.txt --backend libQnnHtp.so"
    ;;
  dsp-v66 )
    adb push ${QNN_SDK_ROOT}/target/${arch_variant}/lib/libQnnDspV66Stub.so ${TARGET_ROOT}
    adb wait-for-device push ${QNN_SDK_ROOT}/target/hexagon-v66/lib/unsigned/libQnnDspV66Skel.so ${TARGET_ROOT}
    ADSP_LIBRARY_PATH="export ADSP_LIBRARY_PATH=\""${TARGET_ROOT}";/vendor/dsp/cdsp;/vendor/lib/rfsa/adsp;/system/lib/rfsa/adsp;/dsp\""
    LD_LIBRARY_PATH="export LD_LIBRARY_PATH=/vendor/dsp/cdsp:"${VENDOR_LIB}":\$LD_LIBRARY_PATH"
    QNN_NET_RUN_CMD="${ADSP_LIBRARY_PATH} && ${LD_LIBRARY_PATH} && ./qnn-net-run --model lib"${MODEL}".so --input_list input_list_float.txt --backend libQnnDspV66Stub.so"
    ;;
  dsp-v65 )
    adb push ${QNN_SDK_ROOT}/target/${arch_variant}/lib/libQnnDspV65Stub.so ${TARGET_ROOT}
    adb wait-for-device push ${QNN_SDK_ROOT}/target/hexagon-v65/lib/unsigned/libQnnDspV65Skel.so ${TARGET_ROOT}
    ADSP_LIBRARY_PATH="export ADSP_LIBRARY_PATH=\""${TARGET_ROOT}";/vendor/dsp/cdsp;/vendor/lib/rfsa/adsp;/system/lib/rfsa/adsp;/dsp\""
    LD_LIBRARY_PATH="export LD_LIBRARY_PATH=/vendor/dsp/cdsp:"${VENDOR_LIB}":\$LD_LIBRARY_PATH"
    QNN_NET_RUN_CMD="${ADSP_LIBRARY_PATH} && ${LD_LIBRARY_PATH} && ./qnn-net-run --model lib"${MODEL}".so --input_list input_list_float.txt --backend libQnnDspV65Stub.so"
    ;;
esac

echo "INFO: Executing qnn-net-run."
ADB_SHELL_CMD="export LD_LIBRARY_PATH=/data/local/tmp/qnn/"${MODEL}" && cd /data/local/tmp/qnn/"${MODEL}" && "${QNN_NET_RUN_CMD}
adb shell ${ADB_SHELL_CMD}

echo "INFO: Pulling results from device."
adb pull ${TARGET_ROOT}/output ${SCRIPT_LOCATION}

#move output files to OUTPUT_DIR
if [[ "${OUTPUT_DIR}" != "${SCRIPT_LOCATION}" ]];
then
  if [[ ! -d ${OUTPUT_DIR} ]]
  then
    echo "INFO: Creating Output dir ${OUTPUT_DIR}."
    mkdir -p ${OUTPUT_DIR}
  fi
  mv -f output ${OUTPUT_DIR}/.
fi

#cleaning up generated files
trap "rm -rf ${MODEL_LIBS}" EXIT

echo "INFO: Done."

#set +x
set +e
