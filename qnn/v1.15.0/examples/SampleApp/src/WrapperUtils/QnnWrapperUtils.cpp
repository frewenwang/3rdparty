//==============================================================================
//
//  Copyright (c) 2020, 2022 Qualcomm Technologies, Inc.
//  All Rights Reserved.
//  Confidential and Proprietary - Qualcomm Technologies, Inc.
//
//==============================================================================

#include <stdlib.h>

#include "QnnWrapperUtils.hpp"

qnn_wrapper_api::ModelError_t qnn_wrapper_api::freeQnnTensorWrapper(Qnn_TensorWrapper_t &tensor) {
  // free all pointer allocations in struct
  free(tensor.name);
  free(tensor.tensor.maxDimensions);
  free(tensor.tensor.currentDimensions);
  return MODEL_NO_ERROR;
}

qnn_wrapper_api::ModelError_t qnn_wrapper_api::freeQnnTensorWrappers(Qnn_TensorWrapper_t *&tensors,
                                                                     uint32_t numTensors) {
  // free all pointer allocations in struct
  for (size_t i = 0; i < numTensors; i++) {
    freeQnnTensorWrapper(tensors[i]);
  }
  free(tensors);

  return MODEL_NO_ERROR;
}

qnn_wrapper_api::ModelError_t qnn_wrapper_api::freeGraphsInfo(GraphInfoPtr_t **graphsInfo,
                                                              uint32_t numGraphs) {
  if (graphsInfo == nullptr || *graphsInfo == nullptr) {
    return MODEL_TENSOR_ERROR;
  }
  for (uint32_t i = 0; i < numGraphs; i++) {
    free((*graphsInfo)[i]->graphName);
    freeQnnTensorWrappers((*graphsInfo)[i]->inputTensors, (*graphsInfo)[i]->numInputTensors);
    freeQnnTensorWrappers((*graphsInfo)[i]->outputTensors, (*graphsInfo)[i]->numOutputTensors);
  }
  free(**graphsInfo);
  free(*graphsInfo);
  *graphsInfo = nullptr;

  return MODEL_NO_ERROR;
}