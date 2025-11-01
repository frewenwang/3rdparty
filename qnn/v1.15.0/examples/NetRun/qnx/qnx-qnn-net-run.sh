#!/usr/bin/env bash

#
#  Copyright (c) 2021 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#

# This script makes the assumption that there is only one QNX device connected at a time.
# The script assumes adbd is running on QNX to perform adb commands.

set -e
#set -x

helpMessage(){
cat << EOF
Usage: $(basename -- $0) [-h] [-b BACKEND_TO_USE] [-o OUTPUT_DIR]

Required argument(s):
 -b BACKEND_NAME                       Either cpu or saver

optional argument(s):
 -o OUTPUT_DIR                         Location for saving output files. Default: Current Directory

EOF
}

SCRIPT_LOCATION=`dirname $(readlink -f ${0})`
QNN_SDK_ROOT=${SCRIPT_LOCATION}/../../..
OUTPUT_DIR=${SCRIPT_LOCATION}

while getopts "h?b:o:" opt;
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

if [[ "$backend" != "cpu" && "$backend" != "saver" ]]
then
  echo "ERROR: Invalid Backend Specified."
  helpMessage
  exit 1
fi
arch_variant="aarch64-qnx"
VENDOR_LIB="/mnt/lib64/"
SCRIPT_LOCATION=`dirname $(readlink -f ${0})`
QNN_SDK_ROOT=${SCRIPT_LOCATION}/../../..

set --
source ${QNN_SDK_ROOT}/target/x86_64-linux-clang/bin/envsetup.sh
unset --

MODEL_ROOT=${QNN_SDK_ROOT}/examples/converter/models
MODEL=qnn_model_float
MODEL_LIBS=${SCRIPT_LOCATION}/model_libs

echo "INFO: Running QNN Model Lib Generator."
export PATH=${QNX_HOST}:${QNX_TARGET}:${PATH}
QNN_MODEL_LIB_GENERATOR=${QNN_SDK_ROOT}/target/x86_64-linux-clang/bin/qnn-model-lib-generator
${QNN_MODEL_LIB_GENERATOR} -c ${MODEL_ROOT}/${MODEL}.cpp -b ${MODEL_ROOT}/${MODEL}.bin -o ${MODEL_LIBS} -t ${arch_variant}

TARGET_ROOT=/data/local/tmp/qnn/${MODEL}
# Push inputs, generated model, and executable
echo "INFO: Pushing required files onto device."
adb shell "mkdir -p "${TARGET_ROOT}
adb push ${QNN_SDK_ROOT}/target/${arch_variant}/bin/qnn-net-run ${TARGET_ROOT}
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
esac

echo "INFO: Executing qnn-net-run."
ADB_SHELL_CMD="export LD_LIBRARY_PATH=/data/local/tmp/qnn/"${MODEL}":/mnt/lib64/ && cd /data/local/tmp/qnn/"${MODEL}" && "${QNN_NET_RUN_CMD}
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
