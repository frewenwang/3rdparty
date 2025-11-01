//==============================================================================
//
//  Copyright (c) 2022 Qualcomm Technologies, Inc.
//  All Rights Reserved.
//  Confidential and Proprietary - Qualcomm Technologies, Inc.
//
//==============================================================================

#include "QnnTypes.h"

// Below are macros duplicated from Core to support OpPackage Example

inline const char* getQnnGpuOpConfigName(const Qnn_OpConfig_t opConfig) { return opConfig.name; }

inline const char* getQnnGpuOpConfigTypeName(const Qnn_OpConfig_t opConfig) {
  return opConfig.typeName;
}

inline uint32_t getQnnGpuOpConfigNumInputs(const Qnn_OpConfig_t opConfig) {
  return opConfig.numOfInputs;
}

inline Qnn_Tensor_t getQnnGpuOpConfigInputTensors(const Qnn_OpConfig_t opConfig, uint32_t index) {
  return opConfig.inputTensors[index];
}

inline uint32_t getQnnGpuOpConfigNumOutputs(const Qnn_OpConfig_t opConfig) {
  return opConfig.numOfOutputs;
}

inline Qnn_Tensor_t getQnnGpuOpConfigOutputTensors(const Qnn_OpConfig_t opConfig, uint32_t index) {
  return opConfig.outputTensors[index];
}

inline uint32_t getQnnGpuTensorRank(const Qnn_Tensor_t tensor) { return tensor.rank; }

inline uint32_t* getQnnGpuTensorDimensions(const Qnn_Tensor_t tensor) {
  return tensor.currentDimensions;
}

// Accessors for QNN Op Config
#define QNN_GPU_OP_CFG_GET_NAME(opConfig)       (getQnnGpuOpConfigName(opConfig))
#define QNN_GPU_OP_CFG_GET_TYPE_NAME(opConfig)  getQnnGpuOpConfigTypeName(opConfig)
#define QNN_GPU_OP_CFG_GET_NUM_INPUTS(opConfig) getQnnGpuOpConfigNumInputs(opConfig)
#define QNN_GPU_OP_CFG_GET_INPUT_TENSOR(opConfig, index) \
  getQnnGpuOpConfigInputTensors(opConfig, index)
#define QNN_GPU_OP_CFG_GET_NUM_OUTPUTS(opConfig) getQnnGpuOpConfigNumOutputs(opConfig)
#define QNN_GPU_OP_CFG_GET_OUTPUT_TENSOR(opConfig, index) \
  getQnnGpuOpConfigOutputTensors(opConfig, index)
#define QNN_GPU_TENSOR_GET_RANK(tensor)       getQnnGpuTensorRank(tensor)
#define QNN_GPU_TENSOR_GET_DIMENSIONS(tensor) getQnnGpuTensorDimensions(tensor)
