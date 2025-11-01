//==============================================================================
//
//  Copyright (c) 2020-2022 Qualcomm Technologies, Inc.
//  All Rights Reserved.
//  Confidential and Proprietary - Qualcomm Technologies, Inc.
//
//==============================================================================

#pragma once

#include "QnnContext.h"
#include "QnnGraph.h"
#include "QnnTensor.h"
#include "QnnTypes.h"

namespace qnn_wrapper_api {

// macro utils
#define VALIDATE(value, retStatus)                      \
  do {                                                  \
    retStatus = value;                                  \
    if (retStatus != qnn_wrapper_api::MODEL_NO_ERROR) { \
      return retStatus;                                 \
    }                                                   \
  } while (0)

#define STRINGFY(str)      str
#define STRINGFYVALUE(str) STRINGFY(str)

// macros for retrieving binary data
#define BINVARSTART(NAME)                                         \
  ({                                                              \
    extern const uint8_t _binary_obj_binary_##NAME##_raw_start[]; \
    (void *)_binary_obj_binary_##NAME##_raw_start;                \
  })
#define BINVAREND(NAME)                                         \
  ({                                                            \
    extern const uint8_t _binary_obj_binary_##NAME##_raw_end[]; \
    (void *)_binary_obj_binary_##NAME##_raw_end;                \
  })
#define BINLEN(NAME)                                                                             \
  ({                                                                                             \
    extern const uint8_t _binary_obj_binary_##NAME##_raw_start[];                                \
    extern const uint8_t _binary_obj_binary_##NAME##_raw_end[];                                  \
    (uint32_t)((_binary_obj_binary_##NAME##_raw_end) - (_binary_obj_binary_##NAME##_raw_start)); \
  })

typedef enum ModelError {
  MODEL_NO_ERROR         = 0,
  MODEL_TENSOR_ERROR     = 1,
  MODEL_PARAMS_ERROR     = 2,
  MODEL_NODES_ERROR      = 3,
  MODEL_GRAPH_ERROR      = 4,
  MODEL_CONTEXT_ERROR    = 5,
  MODEL_GENERATION_ERROR = 6,
  MODEL_SETUP_ERROR      = 7,
  // Value selected to ensure 32 bits.
  MODEL_UNKNOWN_ERROR = 0x7FFFFFFF
} ModelError_t;

typedef struct QnnTensorWrapper {
  char *name;
  Qnn_Tensor_t tensor;
} Qnn_TensorWrapper_t;

typedef struct GraphInfo {
  Qnn_GraphHandle_t graph;
  char *graphName;
  Qnn_TensorWrapper_t *inputTensors;
  uint32_t numInputTensors;
  Qnn_TensorWrapper_t *outputTensors;
  uint32_t numOutputTensors;
} GraphInfo_t;
typedef GraphInfo_t *GraphInfoPtr_t;

typedef struct GraphConfigInfo {
  char *graphName;
  const QnnGraph_Config_t **graphConfigs;
} GraphConfigInfo_t;

typedef struct QnnParamWrapper {
  /// Type is scalar or tensor
  Qnn_ParamType_t paramType;
  /// Name of the parameter
  char *name;
  union {
    Qnn_Scalar_t scalarParam;
    Qnn_TensorWrapper_t tensorParam;
  };
} Qnn_ParamWrapper_t;

/**
 * @brief Frees all memory allocated tensor attributes.
 *
 * @param[in] tensorWrapper tensor object to free
 *
 * @return Error code
 */
ModelError_t freeQnnTensorWrapper(Qnn_TensorWrapper_t &tensor);

/**
 * @brief Loops through and frees all memory allocated tensor attributes for each tensorWrapper
 * object.
 *
 * @param[in] tensorWrappers array of tensor objects to free
 *
 * @param[in] numTensors length of the above tensorWrappers array
 *
 * @return Error code
 */
ModelError_t freeQnnTensorWrappers(Qnn_TensorWrapper_t *&tensors, uint32_t numTensors);

/**
 * @brief A helper function to free memory malloced for communicating the Graph for a model(s)
 *
 * @param[in] graphsInfo Pointer pointing to location of graph objects
 *
 * @param[in] numGraphs The number of graph objects the above pointer is pointing to
 *
 * @return Error code
 *
 */
ModelError_t freeGraphsInfo(GraphInfoPtr_t **graphsInfo, uint32_t numGraphs);
}  // namespace qnn_wrapper_api
