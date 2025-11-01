//==============================================================================
//
//  Copyright (c) 2019-2021 Qualcomm Technologies, Inc.
//  All Rights Reserved.
//  Confidential and Proprietary - Qualcomm Technologies, Inc.
//
//==============================================================================

/*
 * This file is a prototype model file that is here to mimic what the Qnn Converter will
 * generate. This file along with the WrapperApis can be used to build the qnn_model.so
 */

#include "QnnModel.hpp"
#include "QnnOpDef.h"

// Flag to determine if Backend should node validation for each opNode added
#define DO_GRAPH_NODE_VALIDATIONS 1

using namespace qnn_wrapper_api;
extern "C" {
QNN_API
ModelError_t QnnModel_composeGraphs(void* qnnBackendHandle,
                                    Qnn_ContextHandle_t context,
                                    const GraphConfigInfo_t** graphsConfigInfo,
                                    const uint32_t numGraphsConfigInfo,
                                    GraphInfoPtr_t** graphsInfo,
                                    uint32_t* numGraphsInfo,
                                    bool debug) {
  ModelError_t err = MODEL_NO_ERROR;

  /* model/graph for convReluModel*/
  QnnModel convReluModel;
  const QnnGraph_Config_t** graphConfigs = NULL;
  VALIDATE(convReluModel.setup(qnnBackendHandle), err);
  VALIDATE(getQnnGraphConfigFromInfo(
               "convReluModel", graphsConfigInfo, numGraphsConfigInfo, graphConfigs),
           err);
  VALIDATE(convReluModel.initialize(
               context, "convReluModel", debug, DO_GRAPH_NODE_VALIDATIONS, false, graphConfigs),
           err);
  uint32_t curDims_input_0[] = {1, 299, 299, 3};
  uint32_t maxDims_input_0[] = {1, 299, 299, 3};
  VALIDATE(
      convReluModel.addTensor(
          "input_0",  // Node Name
          (Qnn_TensorWrapper_t){
              .name = "input_0",
              (Qnn_Tensor_t){
                  .id                = 1839211098,
                  .type              = QNN_TENSOR_TYPE_APP_WRITE,
                  .dataFormat        = QNN_TENSOR_DATA_FORMAT_FLAT_BUFFER,
                  .dataType          = QNN_DATATYPE_UFIXED_POINT_8,
                  .quantizeParams    = {.encodingDefinition   = QNN_DEFINITION_DEFINED,
                                     .quantizationEncoding = QNN_QUANTIZATION_ENCODING_SCALE_OFFSET,
                                     {.scaleOffsetEncoding = {.scale = 0.007812, .offset = -128}}},
                  .rank              = 4,
                  .maxDimensions     = maxDims_input_0,
                  .currentDimensions = curDims_input_0,
                  .memType           = QNN_TENSORMEMTYPE_RAW,
                  {.clientBuf = {.data = nullptr, .dataSize = 0}}}}),
      err);
  uint32_t curDims_InceptionV3_InceptionV3_Conv2d_1a_3x3_Conv2D_weight[] = {3, 3, 3, 32};
  uint32_t maxDims_InceptionV3_InceptionV3_Conv2d_1a_3x3_Conv2D_weight[] = {3, 3, 3, 32};
  VALIDATE(
      convReluModel.addTensor(
          "InceptionV3_InceptionV3_Conv2d_1a_3x3_Conv2D",  // Node Name
          (Qnn_TensorWrapper_t){
              .name = "InceptionV3_InceptionV3_Conv2d_1a_3x3_Conv2D_weight",
              (Qnn_Tensor_t){
                  .id                = 3250563203,
                  .type              = QNN_TENSOR_TYPE_STATIC,
                  .dataFormat        = QNN_TENSOR_DATA_FORMAT_FLAT_BUFFER,
                  .dataType          = QNN_DATATYPE_UFIXED_POINT_8,
                  .quantizeParams    = {.encodingDefinition   = QNN_DEFINITION_DEFINED,
                                     .quantizationEncoding = QNN_QUANTIZATION_ENCODING_SCALE_OFFSET,
                                     {.scaleOffsetEncoding = {.scale = 0.037750, .offset = -118}}},
                  .rank              = 4,
                  .maxDimensions     = maxDims_InceptionV3_InceptionV3_Conv2d_1a_3x3_Conv2D_weight,
                  .currentDimensions = curDims_InceptionV3_InceptionV3_Conv2d_1a_3x3_Conv2D_weight,
                  .memType           = QNN_TENSORMEMTYPE_RAW,
                  {.clientBuf = {.data = BINVARSTART(
                                     InceptionV3_InceptionV3_Conv2d_1a_3x3_Conv2D_weight),
                                 .dataSize = BINLEN(
                                     InceptionV3_InceptionV3_Conv2d_1a_3x3_Conv2D_weight)}}}}),
      err);
  uint32_t curDims_InceptionV3_InceptionV3_Conv2d_1a_3x3_Conv2D_bias[] = {32};
  uint32_t maxDims_InceptionV3_InceptionV3_Conv2d_1a_3x3_Conv2D_bias[] = {32};
  VALIDATE(
      convReluModel.addTensor(
          "InceptionV3_InceptionV3_Conv2d_1a_3x3_Conv2D",  // Node Name
          (Qnn_TensorWrapper_t){
              .name = "InceptionV3_InceptionV3_Conv2d_1a_3x3_Conv2D_bias",
              (Qnn_Tensor_t){
                  .id                = 38271498,
                  .type              = QNN_TENSOR_TYPE_STATIC,
                  .dataFormat        = QNN_TENSOR_DATA_FORMAT_FLAT_BUFFER,
                  .dataType          = QNN_DATATYPE_UFIXED_POINT_8,
                  .quantizeParams    = {.encodingDefinition   = QNN_DEFINITION_DEFINED,
                                     .quantizationEncoding = QNN_QUANTIZATION_ENCODING_SCALE_OFFSET,
                                     {.scaleOffsetEncoding = {.scale = 0.021825, .offset = -70}}},
                  .rank              = 1,
                  .maxDimensions     = maxDims_InceptionV3_InceptionV3_Conv2d_1a_3x3_Conv2D_bias,
                  .currentDimensions = curDims_InceptionV3_InceptionV3_Conv2d_1a_3x3_Conv2D_bias,
                  .memType           = QNN_TENSORMEMTYPE_RAW,
                  {.clientBuf =
                       {.data     = BINVARSTART(InceptionV3_InceptionV3_Conv2d_1a_3x3_Conv2D_bias),
                        .dataSize = BINLEN(InceptionV3_InceptionV3_Conv2d_1a_3x3_Conv2D_bias)}}}}),
      err);

  /* ADDING NODE FOR InceptionV3_InceptionV3_Conv2d_1a_3x3_Conv2D */
  uint32_t InceptionV3_InceptionV3_Conv2d_1a_3x3_Conv2D_dilation[]           = {1, 1};
  uint32_t curDims_InceptionV3_InceptionV3_Conv2d_1a_3x3_Conv2D_dilation[]   = {2};
  uint32_t maxDims_InceptionV3_InceptionV3_Conv2d_1a_3x3_Conv2D_dilation[]   = {2};
  uint32_t InceptionV3_InceptionV3_Conv2d_1a_3x3_Conv2D_pad_amount[]         = {0, 0, 0, 0};
  uint32_t curDims_InceptionV3_InceptionV3_Conv2d_1a_3x3_Conv2D_pad_amount[] = {2, 2};
  uint32_t maxDims_InceptionV3_InceptionV3_Conv2d_1a_3x3_Conv2D_pad_amount[] = {2, 2};
  uint32_t InceptionV3_InceptionV3_Conv2d_1a_3x3_Conv2D_stride[]             = {2, 2};
  uint32_t curDims_InceptionV3_InceptionV3_Conv2d_1a_3x3_Conv2D_stride[]     = {2};
  uint32_t maxDims_InceptionV3_InceptionV3_Conv2d_1a_3x3_Conv2D_stride[]     = {2};
  Qnn_ParamWrapper_t params_InceptionV3_InceptionV3_Conv2d_1a_3x3_Conv2D[]   = {
      {.paramType = QNN_PARAMTYPE_TENSOR,
       .name      = "dilation",
       {.tensorParam =
            (Qnn_TensorWrapper_t){
                .name = "InceptionV3_InceptionV3_Conv2d_1a_3x3_Conv2D_dilation",
                (Qnn_Tensor_t){
                    .id             = 3662017328,
                    .type           = QNN_TENSOR_TYPE_STATIC,
                    .dataFormat     = QNN_TENSOR_DATA_FORMAT_FLAT_BUFFER,
                    .dataType       = QNN_DATATYPE_UINT_32,
                    .quantizeParams = {.encodingDefinition   = QNN_DEFINITION_UNDEFINED,
                                       .quantizationEncoding = QNN_QUANTIZATION_ENCODING_UNDEFINED,
                                       {.scaleOffsetEncoding = {.scale  = 0.000000000000,
                                                                .offset = 0}}},
                    .rank           = 1,
                    .maxDimensions  = maxDims_InceptionV3_InceptionV3_Conv2d_1a_3x3_Conv2D_dilation,
                    .currentDimensions =
                        curDims_InceptionV3_InceptionV3_Conv2d_1a_3x3_Conv2D_dilation,
                    .memType = QNN_TENSORMEMTYPE_RAW,
                    {.clientBuf = {.data = (uint8_t*)
                                       InceptionV3_InceptionV3_Conv2d_1a_3x3_Conv2D_dilation,
                                   .dataSize = 8}}}}}},
      {.paramType = QNN_PARAMTYPE_TENSOR,
       .name      = "pad_amount",
       {.tensorParam =
            (Qnn_TensorWrapper_t){
                .name = "InceptionV3_InceptionV3_Conv2d_1a_3x3_Conv2D_pad_amount",
                (Qnn_Tensor_t){
                    .id             = 3044353595,
                    .type           = QNN_TENSOR_TYPE_STATIC,
                    .dataFormat     = QNN_TENSOR_DATA_FORMAT_FLAT_BUFFER,
                    .dataType       = QNN_DATATYPE_UINT_32,
                    .quantizeParams = {.encodingDefinition   = QNN_DEFINITION_UNDEFINED,
                                       .quantizationEncoding = QNN_QUANTIZATION_ENCODING_UNDEFINED,
                                       {.scaleOffsetEncoding = {.scale  = 0.000000000000,
                                                                .offset = 0}}},
                    .rank           = 2,
                    .maxDimensions =
                        maxDims_InceptionV3_InceptionV3_Conv2d_1a_3x3_Conv2D_pad_amount,
                    .currentDimensions =
                        curDims_InceptionV3_InceptionV3_Conv2d_1a_3x3_Conv2D_pad_amount,
                    .memType = QNN_TENSORMEMTYPE_RAW,
                    {.clientBuf =
                         {.data = (uint8_t*)InceptionV3_InceptionV3_Conv2d_1a_3x3_Conv2D_pad_amount,
                          .dataSize = 16}}}}}},
      {.paramType = QNN_PARAMTYPE_TENSOR,
       .name      = "stride",
       {.tensorParam =
            (Qnn_TensorWrapper_t){
                .name = "InceptionV3_InceptionV3_Conv2d_1a_3x3_Conv2D_stride",
                (Qnn_Tensor_t){
                    .id             = 1373508289,
                    .type           = QNN_TENSOR_TYPE_STATIC,
                    .dataFormat     = QNN_TENSOR_DATA_FORMAT_FLAT_BUFFER,
                    .dataType       = QNN_DATATYPE_UINT_32,
                    .quantizeParams = {.encodingDefinition   = QNN_DEFINITION_UNDEFINED,
                                       .quantizationEncoding = QNN_QUANTIZATION_ENCODING_UNDEFINED,
                                       {.scaleOffsetEncoding = {.scale  = 0.000000000000,
                                                                .offset = 0}}},
                    .rank           = 1,
                    .maxDimensions  = maxDims_InceptionV3_InceptionV3_Conv2d_1a_3x3_Conv2D_stride,
                    .currentDimensions =
                        curDims_InceptionV3_InceptionV3_Conv2d_1a_3x3_Conv2D_stride,
                    .memType = QNN_TENSORMEMTYPE_RAW,
                    {.clientBuf =
                         {.data     = (uint8_t*)InceptionV3_InceptionV3_Conv2d_1a_3x3_Conv2D_stride,
                          .dataSize = 8}}}}}},
      {.paramType = QNN_PARAMTYPE_SCALAR,
       .name      = "group",
       {.scalarParam = (Qnn_Scalar_t){.dataType = QNN_DATATYPE_UINT_32, {.uint32Value = 1}}}}};
  char* inputs_InceptionV3_InceptionV3_Conv2d_1a_3x3_Conv2D[] = {
      "input_0",
      "InceptionV3_InceptionV3_Conv2d_1a_3x3_Conv2D_weight",
      "InceptionV3_InceptionV3_Conv2d_1a_3x3_Conv2D_bias"};
  uint32_t curDims_InceptionV3_InceptionV3_Conv2d_1a_3x3_BatchNorm_FusedBatchNorm_0[] = {
      1, 149, 149, 32};
  uint32_t maxDims_InceptionV3_InceptionV3_Conv2d_1a_3x3_BatchNorm_FusedBatchNorm_0[] = {
      1, 149, 149, 32};
  Qnn_TensorWrapper_t outputs_InceptionV3_InceptionV3_Conv2d_1a_3x3_Conv2D[] = {(
      Qnn_TensorWrapper_t){
      .name = "InceptionV3_InceptionV3_Conv2d_1a_3x3_BatchNorm_FusedBatchNorm_0",
      (Qnn_Tensor_t){
          .id             = 3087073075,
          .type           = QNN_TENSOR_TYPE_NATIVE,
          .dataFormat     = QNN_TENSOR_DATA_FORMAT_FLAT_BUFFER,
          .dataType       = QNN_DATATYPE_UFIXED_POINT_8,
          .quantizeParams = {.encodingDefinition   = QNN_DEFINITION_DEFINED,
                             .quantizationEncoding = QNN_QUANTIZATION_ENCODING_SCALE_OFFSET,
                             {.scaleOffsetEncoding = {.scale = 0.104550, .offset = -125}}},
          .rank           = 4,
          .maxDimensions = maxDims_InceptionV3_InceptionV3_Conv2d_1a_3x3_BatchNorm_FusedBatchNorm_0,
          .currentDimensions =
              curDims_InceptionV3_InceptionV3_Conv2d_1a_3x3_BatchNorm_FusedBatchNorm_0,
          .memType = QNN_TENSORMEMTYPE_RAW,
          {.clientBuf = {.data = nullptr, .dataSize = 0}}}}};
  VALIDATE(convReluModel.addNode(
               "InceptionV3_InceptionV3_Conv2d_1a_3x3_Conv2D",        // Node Name
               "qti.aisw",                                            // Package Name
               "Conv2d",                                              // Qnn Node Type
               params_InceptionV3_InceptionV3_Conv2d_1a_3x3_Conv2D,   // Node Params
               4,                                                     // Num Node Params
               inputs_InceptionV3_InceptionV3_Conv2d_1a_3x3_Conv2D,   // Input Tensor Names
               3,                                                     // Num Input Tensor Names
               outputs_InceptionV3_InceptionV3_Conv2d_1a_3x3_Conv2D,  // Output Tensors
               1                                                      // Num Output Tensors
               ),
           err);

  /* ADDING NODE FOR InceptionV3_InceptionV3_Conv2d_1a_3x3_Relu */
  char* inputs_InceptionV3_InceptionV3_Conv2d_1a_3x3_Relu[] = {
      "InceptionV3_InceptionV3_Conv2d_1a_3x3_BatchNorm_FusedBatchNorm_0"};
  uint32_t curDims_InceptionV3_InceptionV3_Conv2d_1a_3x3_Relu_0[]          = {1, 149, 149, 32};
  uint32_t maxDims_InceptionV3_InceptionV3_Conv2d_1a_3x3_Relu_0[]          = {1, 149, 149, 32};
  Qnn_TensorWrapper_t outputs_InceptionV3_InceptionV3_Conv2d_1a_3x3_Relu[] = {(Qnn_TensorWrapper_t){
      .name = "InceptionV3_InceptionV3_Conv2d_1a_3x3_Relu_0",
      (Qnn_Tensor_t){
          .id                = 3774485837,
          .type              = QNN_TENSOR_TYPE_APP_READ,
          .dataFormat        = QNN_TENSOR_DATA_FORMAT_FLAT_BUFFER,
          .dataType          = QNN_DATATYPE_UFIXED_POINT_8,
          .quantizeParams    = {.encodingDefinition   = QNN_DEFINITION_DEFINED,
                             .quantizationEncoding = QNN_QUANTIZATION_ENCODING_SCALE_OFFSET,
                             {.scaleOffsetEncoding = {.scale = 0.0547085, .offset = 0}}},
          .rank              = 4,
          .maxDimensions     = maxDims_InceptionV3_InceptionV3_Conv2d_1a_3x3_Relu_0,
          .currentDimensions = curDims_InceptionV3_InceptionV3_Conv2d_1a_3x3_Relu_0,
          .memType           = QNN_TENSORMEMTYPE_RAW,
          {.clientBuf = {.data = nullptr, .dataSize = 0}}}}};
  VALIDATE(convReluModel.addNode(
               "InceptionV3_InceptionV3_Conv2d_1a_3x3_Relu",        // Node Name
               "qti.aisw",                                          // Package Name
               "Relu",                                              // Qnn Node Type
               NULL,                                                // Node Params
               0,                                                   // Num Node Params
               inputs_InceptionV3_InceptionV3_Conv2d_1a_3x3_Relu,   // Input Tensor Names
               1,                                                   // Num Input Tensor Names
               outputs_InceptionV3_InceptionV3_Conv2d_1a_3x3_Relu,  // Output Tensors
               1                                                    // Num Output Tensors
               ),
           err);

  // Add all models to array to get graphsInfo
  QnnModel* models[] = {&convReluModel};
  uint32_t numModels = 1;

  // Populate the constructed graphs in provided output variables
  VALIDATE(getGraphInfoFromModels(*models, numModels, graphsInfo), err);
  *numGraphsInfo = numModels;

  return err;

}  // PREPARE_GRAPHS

QNN_API
ModelError_t QnnModel_freeGraphsInfo(GraphInfoPtr_t** graphs, uint32_t numGraphsInfo) {
  return qnn_wrapper_api::freeGraphsInfo(graphs, numGraphsInfo);
}  // FREEGRAPHINFO
}