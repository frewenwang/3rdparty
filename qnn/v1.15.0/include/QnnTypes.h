//==============================================================================
//
// Copyright (c) 2019-2022 Qualcomm Technologies, Inc.
// All Rights Reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
//
//==============================================================================

/**
 *  @file
 *  @brief  A header which contains the base types required by the API.
 *          Strings are expected to be UTF-8 encoded and NULL terminated.
 */

#ifndef QNN_TYPES_H
#define QNN_TYPES_H

#ifdef __cplusplus
#include <cstdarg>
#include <cstddef>
#include <cstdint>
#else
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#endif

#include "QnnCommon.h"

#ifdef __cplusplus
extern "C" {
#endif

//=============================================================================
// Macros
//=============================================================================

// Macro controlling visibility of QNN API
#ifndef QNN_API
#define QNN_API
#endif

//=============================================================================
// Data Types
//=============================================================================

/**
 * @brief An enum which defines various data types.
 *        FIXED_XX types are targeted for data in tensors.
 *        UINT / INT types are targeted for scalar params.
 */
typedef enum {
  // Signed Int: 0x00XX

  /// 8-bit integer type
  QNN_DATATYPE_INT_8 = 0x0008,
  /// 16-bit integer type
  QNN_DATATYPE_INT_16 = 0x0016,
  /// 32-bit integer type
  QNN_DATATYPE_INT_32 = 0x0032,
  /// 64-bit integer type
  QNN_DATATYPE_INT_64 = 0x0064,

  // Unsigned Int: 0x01XX
  QNN_DATATYPE_UINT_8  = 0x0108,
  QNN_DATATYPE_UINT_16 = 0x0116,
  QNN_DATATYPE_UINT_32 = 0x0132,
  QNN_DATATYPE_UINT_64 = 0x0164,

  // Float: 0x02XX
  QNN_DATATYPE_FLOAT_16 = 0x0216,
  QNN_DATATYPE_FLOAT_32 = 0x0232,

  // Signed Fixed Point: 0x03XX
  QNN_DATATYPE_SFIXED_POINT_8  = 0x0308,
  QNN_DATATYPE_SFIXED_POINT_16 = 0x0316,
  QNN_DATATYPE_SFIXED_POINT_32 = 0x0332,

  // Unsigned Fixed Point: 0x04XX
  QNN_DATATYPE_UFIXED_POINT_8  = 0x0408,
  QNN_DATATYPE_UFIXED_POINT_16 = 0x0416,
  QNN_DATATYPE_UFIXED_POINT_32 = 0x0432,

  // Bool: 0x05XX
  /// 8-bit boolean type, 0 = false, any non-zero value = true
  QNN_DATATYPE_BOOL_8 = 0x0508,

  // Unused, present to ensure 32 bits.
  QNN_DATATYPE_UNDEFINED = 0x7FFFFFFF
} Qnn_DataType_t;

/**
 * @brief An enum which defines the different precision modes supported by QNN backends.
 *        A precision mode may be used to express the math type used in the implementation
 *        of an operation.
 */
typedef enum {
  // FLOATING POINT REPRESENTATIONS

  /// 32-bit Floating point precision. The format of the floating point
  /// value is left to backends to choose.
  QNN_PRECISION_FLOAT32 = 0,
  /// 16-bit Floating point precision. The format of the floating point
  /// value is left to backends to choose.
  QNN_PRECISION_FLOAT16 = 1,

  // Unused, present to ensure 32 bits.
  QNN_PRECISION_UNDEFINED = 0x7FFFFFFF
} Qnn_Precision_t;

/**
 * @brief An enum to specify the tensor type, application accessible or native to QNN
 *
 * Data on APP tensors are accessible to a client.
 * Data on NATIVE tensors are not accessible to a client.
 * Native tensors on a Qnn_GraphHandle_t are accessible only within
 * a graph and could potentially be optimized by the backend.
 * Native tensors on a Qnn_ContextHandle_t are accessible by nodes
 * in any graph in the Context.
 */
typedef enum {
  /// Application writeable tensor
  QNN_TENSOR_TYPE_APP_WRITE = 0,
  /// Application readable tensor
  QNN_TENSOR_TYPE_APP_READ = 1,
  /// Tensor that can both be read and written by an application
  /// Used in scenarios that may include supplying an output
  /// tensor from one graph as the input to another graph.
  QNN_TENSOR_TYPE_APP_READWRITE = 2,
  /// Tensor native to a Graph
  QNN_TENSOR_TYPE_NATIVE = 3,
  /// Static data (doesn't change during execution)
  QNN_TENSOR_TYPE_STATIC = 4,
  /// Tensor type NULL. Qnn_Tensor_t objects specified with this type
  /// will not carry any other information.
  /// Clients can set optional tensors with this type to avoid
  /// populating Qnn_Tensor_t objects with valid data.
  QNN_TENSOR_TYPE_NULL = 5,
  // Unused, present to ensure 32 bits.
  QNN_TENSOR_TYPE_UNDEFINED = 0x7FFFFFFF
} Qnn_TensorType_t;

/**
 * @brief An enum to specify the parameter type : Scalar or Tensor
 */
typedef enum {
  QNN_PARAMTYPE_SCALAR = 0,
  QNN_PARAMTYPE_TENSOR = 1,
  // Unused, present to ensure 32 bits.
  QNN_PARAMTYPE_UNDEFINED = 0xFFFFFFFF
} Qnn_ParamType_t;

/**
 * @brief An enum to specify definition source for field(s) following this enum
 */
typedef enum {
  /// Indicates backend implementation to update or decide
  QNN_DEFINITION_IMPL_GENERATED = 0,
  /// Indicates that provided definition needs to be used
  QNN_DEFINITION_DEFINED = 1,
  // Unused, present to ensure 32 bits.
  QNN_DEFINITION_UNDEFINED = 0x7FFFFFFF
} Qnn_Definition_t;

/**
 * @brief An enum to specify a priority.
 */
typedef enum {
  QNN_PRIORITY_LOW       = 0,
  QNN_PRIORITY_NORMAL    = 100,
  QNN_PRIORITY_DEFAULT   = QNN_PRIORITY_NORMAL,
  QNN_PRIORITY_HIGH      = 200,
  QNN_PRIORITY_UNDEFINED = 0x7FFFFFFF
} Qnn_Priority_t;

/**
 * @brief A typedef to indicate QNN API return handle.
 * Return error codes from APIs are to be read out from the
 * least significant 16 bits of the field. The higher order bits are
 * reserved for internal tracking purposes.
 */
typedef uint64_t Qnn_ErrorHandle_t;

/**
 * @brief A typedef to indicate context binary size.
 */
typedef uint32_t Qnn_ContextBinarySize_t;

/// simple utility to extract 16-bit error code from 64-bit Qnn_ErrorHandle_t
#define QNN_GET_ERROR_CODE(errorHandle) (errorHandle & 0xFFFF)

/**
 * @brief An enum to describe reporting levels for the error handling API
 * QNN_ERROR_REPORTING_LEVEL_BRIEF: get basic information about an error
 * QNN_ERROR_REPORTING_LEVEL_DETAILED: get detailed information about an error
 * in memory-based object forms
 */
typedef enum {
  QNN_ERROR_REPORTING_LEVEL_BRIEF    = 0,
  QNN_ERROR_REPORTING_LEVEL_DETAILED = 1,
  // Unused, present to ensure 32 bits.
  QNN_ERROR_REPORTING_LEVEL_UNDEFINED = 0x7FFFFFFF
} Qnn_ErrorReportingLevel_t;

/**
 * @brief A typedef describing error reporting configuration
 */
typedef struct {
  /// Error reporting level
  Qnn_ErrorReportingLevel_t reportingLevel;
  /// Amount of memory to be reserved for error information. Specified in KB
  uint32_t storageLimit;
} Qnn_ErrorReportingConfig_t;

// clang-format off
/// Qnn_ErrorReportingConfig_t initializer macro
#define QNN_ERROR_REPORTING_CONFIG_INIT                     \
  {                                                         \
    QNN_ERROR_REPORTING_LEVEL_UNDEFINED, /*reportingLevel*/ \
    0                                    /*storageLimit*/   \
  }
// clang-format on

/**
 * @brief A struct which is used to provide a version number using 3 values:
 * major, minor, patch
 */
typedef struct {
  uint32_t major;
  uint32_t minor;
  uint32_t patch;
} Qnn_Version_t;

// clang-format off
/// Qnn_Version_t initializer macro
#define QNN_VERSION_INIT \
  {                      \
    0,     /*major*/     \
    0,     /*minor*/     \
    0      /*patch*/     \
  }
// clang-format on

/**
 * @brief A struct used to provide the versions of both the core QNN API
 * and any Backend Specific API
 */
typedef struct {
  /// Version of the QNN core API common to all backends
  Qnn_Version_t coreApiVersion;
  /// Version of the backend-specific API
  Qnn_Version_t backendApiVersion;
} Qnn_ApiVersion_t;

/// Qnn_ApiVersion_t initializer macro
#define QNN_API_VERSION_INIT                            \
  {                                                     \
    {                                                   \
        QNN_API_VERSION_MAJOR, /*coreApiVersion.major*/ \
        QNN_API_VERSION_MINOR, /*coreApiVersion.minor*/ \
        QNN_API_VERSION_PATCH  /*coreApiVersion.patch*/ \
    },                                                  \
        QNN_VERSION_INIT /*backendApiVersion*/          \
  }

/**
 * @brief A value representing an immutable value which configures a node.
 */
typedef struct {
  Qnn_DataType_t dataType;
  union UNNAMED {
    float floatValue;
    uint32_t uint32Value;
    int32_t int32Value;
    uint16_t uint16Value;
    int16_t int16Value;
    uint8_t uint8Value;
    int8_t int8Value;
    uint8_t bool8Value;
  };
} Qnn_Scalar_t;

/// Qnn_Scalar_t initializer macro
#define QNN_SCALAR_INIT                  \
  {                                      \
    QNN_DATATYPE_UNDEFINED, /*dataType*/ \
    {                                    \
      0.0f /*floatValue*/                \
    }                                    \
  }

/**
 * @brief An enum to specify quantization encoding type structure
 *
 */
typedef enum {
  /// Indicates Qnn_ScaleOffset_t encoding type
  QNN_QUANTIZATION_ENCODING_SCALE_OFFSET = 0,
  /// Indicates Qnn_AxisScaleOffset_t encoding type
  QNN_QUANTIZATION_ENCODING_AXIS_SCALE_OFFSET = 1,
  // Unused, present to ensure 32 bits.
  QNN_QUANTIZATION_ENCODING_UNDEFINED = 0x7FFFFFFF
} Qnn_QuantizationEncoding_t;

/**
 * @brief A struct to express quantization parameters as a positive scale
 * with a zero offset.
 *
 * float_value = (quantized_value + offset) * scale
 */
typedef struct {
  float scale;  // needs to be strictly positive.
  int32_t offset;
} Qnn_ScaleOffset_t;

// clang-format off
/// Qnn_ScaleOffset_t initializer macro
#define QNN_SCALE_OFFSET_INIT \
  {                           \
    0.0f, /*scale*/           \
    0     /*offset*/          \
  }
// clang-format on

/**
 * @brief A struct to express per-axis quantization parameters as
 * a scale with a zero offset
 */
typedef struct {
  int32_t axis;
  uint32_t numScaleOffsets;
  Qnn_ScaleOffset_t* scaleOffset;
} Qnn_AxisScaleOffset_t;

// clang-format off
/// Qnn_AxisScaleOffset_t initializer macro
#define QNN_AXIS_SCALE_OFFSET_INIT \
  {                                \
    0,       /*axis*/              \
    0,       /*numScaleOffsets*/   \
    NULL     /*scaleOffset*/       \
  }
// clang-format on

/**
 * @brief A struct which defines the quantization parameters, and union of supported quantization
 * encoding structs.
 */
typedef struct {
  Qnn_Definition_t encodingDefinition;
  /// Quantization encoding type identifying quantization encoding structure to use
  Qnn_QuantizationEncoding_t quantizationEncoding;
  union UNNAMED {
    Qnn_ScaleOffset_t scaleOffsetEncoding;
    Qnn_AxisScaleOffset_t axisScaleOffsetEncoding;
  };
} Qnn_QuantizeParams_t;

// clang-format off
/// Qnn_QuantizeParams_t initializer macro
#define QNN_QUANTIZE_PARAMS_INIT                                      \
  {                                                                   \
    QNN_DEFINITION_UNDEFINED,                /*encodingDefinition*/   \
    QNN_QUANTIZATION_ENCODING_UNDEFINED,     /*quantizationEncoding*/ \
    {                                                                 \
      QNN_SCALE_OFFSET_INIT /*scaleOffsetEncoding*/                   \
    }                                                                 \
  }
// clang-format on

/**
 * @brief An n-dimensional tensor formatted in memory as flat buffer where the last dimension varies
 *        the fastest
 */
#define QNN_TENSOR_DATA_FORMAT_FLAT_BUFFER 0

/**
 * @brief Implementation-defined data format identifier for tensors.
 *        Legal values and semantics are defined by QNN backends, the default format
 *        QNN_TENSOR_DATA_FORMAT_FLAT_BUFFER is supported by all backends
 * @note  Data format for intermediate tensors, i.e ones of type QNN_TENSOR_TYPE_NATIVE
 *        may not be honored by a backend, because it can choose to pick a data format that is
 *        more conducive for its execution.
 */
typedef uint32_t Qnn_TensorDataFormat_t;

/**
 * @brief An enum specifying memory types of tensor data
 */
typedef enum {
  /// Raw memory pointer
  QNN_TENSORMEMTYPE_RAW = 0,
  /// Memory object, provide capability for memory sharing in between QNN accelerator backends.
  QNN_TENSORMEMTYPE_MEMHANDLE = 1,
  // Unused, present to ensure 32 bits.
  QNN_TENSORMEMTYPE_UNDEFINED = 0x7FFFFFFF
} Qnn_TensorMemType_t;

/**
 * @brief A struct which defines a memory buffer
 *
 */
typedef struct {
  /// app-accessible data pointer, provided by app.
  void* data;
  /// size of buffer, in bytes, pointed to by data.
  uint32_t dataSize;
} Qnn_ClientBuffer_t;

// clang-format off
/// Qnn_ClientBuffer_t initializer macro
#define QNN_CLIENT_BUFFER_INIT \
  {                            \
    NULL, /*data*/             \
    0     /*dataSize*/         \
  }
// clang-format on

/**
 * @brief A struct which describes the properties of a tensor.
 *
 */
typedef struct {
  /// Unique integer identifier for a tensor
  uint32_t id;
  /// Tensor type (Activation vs Static; User-provided vs native, etc)
  Qnn_TensorType_t type;
  /// Tensor data formatting in memory (refer to definition type for info)
  Qnn_TensorDataFormat_t dataFormat;
  /// Tensor data type (e.g: UINT8, FLOAT)
  Qnn_DataType_t dataType;
  /// Tensor quantization params
  Qnn_QuantizeParams_t quantizeParams;
  /// Rank of both max and current dimensions
  uint32_t rank;
  /// Maximum tensor dimensions; Size is rank
  uint32_t* maxDimensions;
  /// Current tensor dimensions; Size is rank
  uint32_t* currentDimensions;
  /// Tensor memory type.
  Qnn_TensorMemType_t memType;

  /// Actual data contained in the tensor
  union UNNAMED {
    /// Tensor data provided by client as a pointer to raw memory
    Qnn_ClientBuffer_t clientBuf;
    /// Tensor data shared via a memory handle
    Qnn_MemHandle_t memHandle;
  };
} Qnn_Tensor_t;

// clang-format off
/// Qnn_Tensor_t initializer macro
#define QNN_TENSOR_INIT                                           \
  {                                                               \
    0,                                      /*id*/                \
    QNN_TENSOR_TYPE_UNDEFINED,              /*type*/              \
    QNN_TENSOR_DATA_FORMAT_FLAT_BUFFER,     /*dataFormat*/        \
    QNN_DATATYPE_UNDEFINED,                 /*dataType*/          \
    QNN_QUANTIZE_PARAMS_INIT,               /*quantizeParams*/    \
    0,                                      /*rank*/              \
    NULL,                                   /*maxDimensions*/     \
    NULL,                                   /*currentDimensions*/ \
    QNN_TENSORMEMTYPE_UNDEFINED,            /*memType*/           \
    {                                                             \
      QNN_CLIENT_BUFFER_INIT /*clientBuf*/                        \
    }                                                             \
  }
// clang-format on

/**
 * @brief A struct which defines a  parameter -
 *        a union of scalar and tensor parameters
 *
 */
typedef struct {
  /// Parameter type: scalar or tensor
  Qnn_ParamType_t paramType;
  /// Name of the parameter
  const char* name;

  union UNNAMED {
    /// Scalar parameter specification
    Qnn_Scalar_t scalarParam;
    /// Tensor parameter specification; tensors referred to must be STATIC.
    Qnn_Tensor_t tensorParam;
  };
} Qnn_Param_t;

// clang-format off
/// Qnn_Param_t initializer macro
#define QNN_PARAM_INIT                     \
  {                                        \
    QNN_PARAMTYPE_UNDEFINED, /*paramType*/ \
    NULL,                    /*name*/      \
    {                                      \
      QNN_SCALAR_INIT /*scalarParam*/      \
    }                                      \
  }
// clang-format on

/**
 * @brief This struct defines the configuration for a single operation
 */
typedef struct {
  /// A human-readable name for the operation instance.
  const char* name;
  /// The name of the operation package to which this operation's type belongs
  const char* packageName;
  /// The name of operation type, e.g. Conv2D
  const char* typeName;
  /// The number of static parameters provided in the params array.
  uint32_t numOfParams;
  /// Array of operation parameters (each can be scalar or tensor)
  Qnn_Param_t* params;
  /// The number of input tensors.
  uint32_t numOfInputs;
  /// Array of input tensors
  Qnn_Tensor_t* inputTensors;
  /// The number of output tensors.
  uint32_t numOfOutputs;
  /// Array of output tensors
  Qnn_Tensor_t* outputTensors;
} Qnn_OpConfig_t;

// clang-format off
/// Qnn_OpConfig_t initializer macro
#define QNN_OPCONFIG_INIT       \
  {                             \
    NULL,     /*name*/          \
    NULL,     /*packageName*/   \
    NULL,     /*typeName*/      \
    0,        /*numOfParams*/   \
    NULL,     /*params*/        \
    0,        /*numOfInputs*/   \
    NULL,     /*inputTensors*/  \
    0,        /*numOfOutputs*/  \
    NULL      /*outputTensors*/ \
  }
// clang-format on
#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // QNN_TYPES_H
