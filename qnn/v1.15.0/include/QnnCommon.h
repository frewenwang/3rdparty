//==============================================================================
//
// Copyright (c) 2019-2022 Qualcomm Technologies, Inc.
// All Rights Reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
//
//==============================================================================

/**
 * @file
 * @brief   Common API components
 *
 *          A header which contains common components shared between different
 *          parts of the API, for example, definition of "context" type. This
 *          simplifies the cross-inclusion of headers.
 */

#ifndef QNN_COMMON_H
#define QNN_COMMON_H

#ifdef __cplusplus
extern "C" {
#endif

//=============================================================================
// Macros
//=============================================================================

//! \cond
// Macro to enable processing unnamed unions under struct for documentation purposes
#define UNNAMED
//! \endcond

// Provide values to use for API version.
#define QNN_API_VERSION_MAJOR 1
#define QNN_API_VERSION_MINOR 12
#define QNN_API_VERSION_PATCH 0

/// NULL backend identifier.
#define QNN_BACKEND_ID_NULL 0

/*
 * Identifiers for known backends that may be included into the SDK.
 * These identifiers are defined by each backend in Qnn<backend>Common.h.
 * Identifiers must be unique per backend.
 *
 * - QNN_BACKEND_ID_NULL      0
 * - QNN_BACKEND_ID_REFERENCE 1
 * - QNN_BACKEND_ID_SAVER     2
 * - QNN_BACKEND_ID_CPU       3
 * - QNN_BACKEND_ID_GPU       4
 * - QNN_BACKEND_ID_DSP       5
 * - QNN_BACKEND_ID_HTP       6
 */

/// Global value indicating success
#define QNN_SUCCESS 0

// Error code space assigned to API components
#define QNN_MIN_ERROR_COMMON              1000
#define QNN_MAX_ERROR_COMMON              1999
#define QNN_MIN_ERROR_PROPERTY            2000
#define QNN_MAX_ERROR_PROPERTY            2999
#define QNN_MIN_ERROR_OP_PACKAGE          3000
#define QNN_MAX_ERROR_OP_PACKAGE          3999
#define QNN_MIN_ERROR_BACKEND             4000
#define QNN_MIN_ERROR_BACKEND_SAVER       4950
#define QNN_MAX_ERROR_BACKEND_SAVER       4998
#define QNN_MAX_ERROR_BACKEND             4999
#define QNN_MIN_ERROR_CONTEXT             5000
#define QNN_MAX_ERROR_CONTEXT             5999
#define QNN_MIN_ERROR_GRAPH               6000
#define QNN_MAX_ERROR_GRAPH               6999
#define QNN_MIN_ERROR_TENSOR              7000
#define QNN_MAX_ERROR_TENSOR              7999
#define QNN_MIN_ERROR_MEM                 8000
#define QNN_MAX_ERROR_MEM                 8999
#define QNN_MIN_ERROR_SIGNAL              9000
#define QNN_MAX_ERROR_SIGNAL              9999
#define QNN_MIN_ERROR_ERROR               10000
#define QNN_MAX_ERROR_ERROR               10999
#define QNN_MIN_ERROR_LOG                 11000
#define QNN_MAX_ERROR_LOG                 11999
#define QNN_MIN_ERROR_PROFILE             12000
#define QNN_MAX_ERROR_PROFILE             12999
#define QNN_MIN_ERROR_PERF_INFRASTRUCTURE 13000
#define QNN_MAX_ERROR_PERF_INFRASTRUCTURE 13999
// Reserved range for QNN system APIs: 30000-50000
#define QNN_MIN_ERROR_SYSTEM    30000
#define QNN_MAX_ERROR_SYSTEM    49999
#define QNN_MIN_ERROR_INTERFACE 60000
#define QNN_MAX_ERROR_INTERFACE 60999

// Utility macros
#define QNN_BIT(x)               (1 << (x))
#define QNN_PASTE_THREE(a, b, c) a##b##c

//=============================================================================
// Data Types
//=============================================================================

// clang-format off

/**
 * @brief Definition of shared opaque objects. These objects are managed by QNN
 * API, that is they are handles allocated and freed through the API.
 */
typedef void* Qnn_ContextHandle_t;
typedef void* Qnn_GraphHandle_t;
typedef void* Qnn_ProfileHandle_t;

/**
 * @brief An opaque control object which may be used to control the execution
 *        behavior of various QNN functions. A signal object may only be observed
 *        by one call at a time; if the same signal object is supplied to a second
 *        call before the first has terminated, the second call will immediately
 *        fail with an error. When the call observing a signal returns gracefully,
 *        the signal object is made observable again.
 */
typedef void* Qnn_SignalHandle_t;

/**
 * @brief A typedef to indicate a QNN memory handle
 */
typedef void* Qnn_MemHandle_t;

/**
 * @brief A typedef to indicate a QNN context utility handle
 *        This type is deprecated.
 */
typedef void* Qnn_ContextUtilsHandle_t;

// clang-format on

/**
 * @brief An enum which defines error codes commonly used across API components.
 */
typedef enum {
  QNN_COMMON_MIN_ERROR = QNN_MIN_ERROR_COMMON,
  //////////////////////////////////////////

  /// Indicates an API or a feature is not supported by implementation.
  /// Generally applicable to optional elements of the API.
  QNN_COMMON_ERROR_NOT_SUPPORTED = QNN_MIN_ERROR_COMMON + 0,
  /// Indicates Backend library has not been initialized (see QnnBackend_initialize).
  QNN_COMMON_ERROR_BACKEND_NOT_INITIALIZED = QNN_MIN_ERROR_COMMON + 1,
  /// Indicates memory allocation related error.
  QNN_COMMON_ERROR_MEM_ALLOC = QNN_MIN_ERROR_COMMON + 2,
  /// Indicates system level error, such as related to platform / OS services
  QNN_COMMON_ERROR_SYSTEM = QNN_MIN_ERROR_COMMON + 3,
  /// Indicates invalid function argument
  QNN_COMMON_ERROR_INVALID_ARGUMENT = QNN_MIN_ERROR_COMMON + 4,
  /// Indicates an illegal operation or sequence of operations
  QNN_COMMON_ERROR_OPERATION_NOT_PERMITTED = QNN_MIN_ERROR_COMMON + 5,
  /// Indicates failure in attempting to use QNN API on an unsupported platform
  QNN_COMMON_ERROR_PLATFORM_NOT_SUPPORTED = QNN_MIN_ERROR_COMMON + 6,
  /// Communication errors with platform / OS service
  QNN_COMMON_ERROR_SYSTEM_COMMUNICATION = QNN_MIN_ERROR_COMMON + 7,
  /// Indicates general type of error, which has not been identified as any other error type.
  /// In general, this error should rarely be used.
  QNN_COMMON_ERROR_GENERAL = QNN_MIN_ERROR_COMMON + 100,

  //////////////////////////////////////////
  QNN_COMMON_MAX_ERROR = QNN_MAX_ERROR_COMMON,
  // Unused, present to ensure 32 bits.
  QNN_COMMON_ERROR_UNDEFINED = 0x7FFFFFFF
} QnnCommon_Error_t;

//=============================================================================
// Public Functions
//=============================================================================

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // QNN_COMMON_H
