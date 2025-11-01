//=============================================================================
//
//  Copyright (c) 2019-2022 Qualcomm Technologies, Inc.
//  All Rights Reserved.
//  Confidential and Proprietary - Qualcomm Technologies, Inc.
//
//=============================================================================

/**
 *  @file
 *  @brief  Property component API.
 *
 *          Provides means for client to discover capabilities of a backend.
 */

#ifndef QNN_PROPERTY_H
#define QNN_PROPERTY_H

#include "QnnCommon.h"
#include "QnnTypes.h"

#ifdef __cplusplus
extern "C" {
#endif

//=============================================================================
// Macros
//=============================================================================
///
/// Definition of QNN_PROPERTY_GROUP_CORE property group.
///

/**
 * @brief Property group for the QNN core property group.
 */
#define QNN_PROPERTY_GROUP_CORE 0x00000001

/**
 * @brief Property key for retrieving the list of operations supported by
 *        a backend. This is a variable-length property, which should be
 *        interpreted as a pointer to QnnProperty_SupportedOperations_t.
 */
#define QNN_PROPERTY_CORE_SUPPORTED_OPERATIONS QNN_PROPERTY_GROUP_CORE + 1

///
/// Definition of QNN_PROPERTY_GROUP_BACKEND property group. This group is Core (non-optional) API.
///

/**
 * @brief Property group for the QNN Backend API property group. This is a non-optional API
 *        component and cannot be used as a property key.
 */
#define QNN_PROPERTY_GROUP_BACKEND QNN_PROPERTY_GROUP_CORE + 100

/**
 * @brief Property key for determining if a backend supports build ID. This is a capability.
 */
#define QNN_PROPERTY_BACKEND_SUPPORT_BUILD_ID QNN_PROPERTY_GROUP_BACKEND + 1

/**
 * @brief Property key for determining if a backend supports performance infrastructure.
 *        This is a capability.
 */
#define QNN_PROPERTY_BACKEND_SUPPORT_PERF_INFRASTRUCTURE QNN_PROPERTY_GROUP_BACKEND + 2

/**
 * @brief Property key for determining if a backend supports op config validation.
 *        This is a capability.
 */
#define QNN_PROPERTY_BACKEND_SUPPORT_OP_VALIDATION QNN_PROPERTY_GROUP_BACKEND + 3

/**
 * @brief Property key for determining if a backend supports op package registration.
 *        This is a capability.
 */
#define QNN_PROPERTY_BACKEND_SUPPORT_OP_PACKAGE QNN_PROPERTY_GROUP_BACKEND + 4

///
/// Definition of QNN_PROPERTY_GROUP_CONTEXT property group. This group is Core (non-optional) API.
///

/**
 * @brief Property group for the QNN Context API property group. This is a non-optional API
 *        component and cannot be used as a property key.
 */
#define QNN_PROPERTY_GROUP_CONTEXT QNN_PROPERTY_GROUP_CORE + 200

/**
 * @brief Property key for determining whether or not a backend supports
 *        get binary context. This is a capability.
 */
#define QNN_PROPERTY_CONTEXT_SUPPORT_GET_BINARY QNN_PROPERTY_GROUP_CONTEXT + 1

/**
 * @brief Property key for determining whether or not a backend supports
 *        get binary context size. This is a capability.
 */
#define QNN_PROPERTY_CONTEXT_SUPPORT_GET_BINARY_SIZE QNN_PROPERTY_GROUP_CONTEXT + 2

/**
 * @brief Property key for determining whether or not a backend supports
 *        creation of context from binary. This is a capability.
 */
#define QNN_PROPERTY_CONTEXT_SUPPORT_CREATE_BINARY QNN_PROPERTY_GROUP_CONTEXT + 3

/**
 * @brief Property key for determining whether or not a backend supports
 *        context configurations. This is a capability.
 */
#define QNN_PROPERTY_CONTEXT_SUPPORT_CONFIGURATION QNN_PROPERTY_GROUP_CONTEXT + 4

/**
 * @brief Property key for determining whether or not a backend supports
 *        create from binary with configurations. This is a capability.
 */
#define QNN_PROPERTY_CONTEXT_SUPPORT_CREATE_FROM_BINARY_WITH_CONFIG QNN_PROPERTY_GROUP_CONTEXT + 5

///
/// Definition of QNN_PROPERTY_GROUP_GRAPH property group. This group is Core (non-optional) API.
///

/**
 * @brief Property group for the QNN Graph API property group. This is a non-optional API
 *        component and cannot be used as a property key.
 */
#define QNN_PROPERTY_GROUP_GRAPH QNN_PROPERTY_GROUP_CORE + 300

/**
 * @brief Property key for determining whether or not a backend supports
 *        graph configuration. This is a capability.
 */
#define QNN_PROPERTY_GRAPH_SUPPORT_CONFIG QNN_PROPERTY_GROUP_GRAPH + 1

/**
 * @brief Property key for determining whether or not a backend supports
 *        signals. This is a capability.
 */
#define QNN_PROPERTY_GRAPH_SUPPORT_SIGNALS QNN_PROPERTY_GROUP_GRAPH + 2

/**
 * @brief Property key for determining whether or not a backend supports
 *        asynchronous graph execution. This is a capability.
 */
#define QNN_PROPERTY_GRAPH_SUPPORT_ASYNC_EXECUTION QNN_PROPERTY_GROUP_GRAPH + 3

/**
 * @brief Property key for determining whether or not a backend supports
 *        execution of graphs will null inputs. This implies that the graph
 *        will contain no APP_WRITE tensors.
 */
#define QNN_PROPERTY_GRAPH_SUPPORT_NULL_INPUTS QNN_PROPERTY_GROUP_GRAPH + 4

///
/// Definition of QNN_PROPERTY_GROUP_OP_PACKAGE property group. This group is Optional portion of
/// API.
///

/**
 * @brief Property group for the QNN Op Package API property group.This can be used as a key to
 *        check if Op Package API is supported by a backend.
 */
#define QNN_PROPERTY_GROUP_OP_PACKAGE QNN_PROPERTY_GROUP_CORE + 400

/**
 * @brief Property key for determining whether or not an op package supports validation.
 *        This is a capability.
 */
#define QNN_PROPERTY_OP_PACKAGE_SUPPORTS_VALIDATION QNN_PROPERTY_GROUP_OP_PACKAGE + 1

/**
 * @brief Property key for determining whether or not an op package supports op implementation
 *         creation and freeing. This is a capability.
 */
#define QNN_PROPERTY_OP_PACKAGE_SUPPORTS_OP_IMPLS QNN_PROPERTY_GROUP_OP_PACKAGE + 2

/**
 * @brief Property key for determining whether or not an op package supports duplication of
 *        operation names, such that there are duplicated op_package_name::op_name combinations.
 *        This is a capability.
 */
#define QNN_PROPERTY_OP_PACKAGE_SUPPORTS_DUPLICATE_NAMES QNN_PROPERTY_GROUP_OP_PACKAGE + 3

///
/// Definition of QNN_PROPERTY_GROUP_TENSOR property group. This group is Core (non-optional) API.
///

/**
 * @brief Property group for the QNN Tensor API property group. This is a non-optional API
 *        component and cannot be used as a property key.
 *
 * @note Currently all Tensor APIs are mandatory.
 */
#define QNN_PROPERTY_GROUP_TENSOR QNN_PROPERTY_GROUP_CORE + 500

/**
 * @brief Property key to determine whether or not a backend supports memhandle type tensors.
 *        This is a capability.
 */
#define QNN_PROPERTY_TENSOR_SUPPORT_MEMHANDLE_TYPE QNN_PROPERTY_GROUP_TENSOR + 1

///
/// Definition of QNN_PROPERTY_GROUP_ERROR property group. This group is Optional portion of API.
///

/**
 * @brief Property key for the QNN Error API property group. This can be used as a key to
 *        check if Error API is supported by a backend.
 */
#define QNN_PROPERTY_GROUP_ERROR QNN_PROPERTY_GROUP_CORE + 1000

///
/// Definition of QNN_PROPERTY_GROUP_MEMORY property group. This group is Optional API.
///

/**
 * @brief Property group for the QNN Memory API property group. This can be used as a key to
 *        check if Memory API is supported by a backend.
 */
#define QNN_PROPERTY_GROUP_MEMORY QNN_PROPERTY_GROUP_CORE + 1100

///
/// Definition of QNN_PROPERTY_GROUP_SIGNAL property group. This group is Optional API.
///

/**
 * @brief Property group for signal support. This can be used as a key to
 *        check if Signal API is supported by a backend.
 */
#define QNN_PROPERTY_GROUP_SIGNAL QNN_PROPERTY_GROUP_CORE + 1200

///
/// Definition of QNN_PROPERTY_GROUP_LOG property group. This group is Optional API.
///

/**
 * @brief Property group for log support. This can be used as a key to
 *        check if Log API is supported by a backend.
 */
#define QNN_PROPERTY_GROUP_LOG QNN_PROPERTY_GROUP_CORE + 1300

/**
 * @brief Property key for determining whether a backend supports logging with the
 *        system's default stream (callback=NULL). This is a capability.
 */
#define QNN_PROPERTY_LOG_SUPPORTS_DEFAULT_STREAM QNN_PROPERTY_GROUP_LOG + 1

///
/// Definition of QNN_PROPERTY_GROUP_PROFILE property group. This group is Optional API.
///

/**
 * @brief Property group for profile support. This can be used as a key to
 *        check if Profile API is supported by a backend.
 */
#define QNN_PROPERTY_GROUP_PROFILE QNN_PROPERTY_GROUP_CORE + 1400

///
/// Definition of QNN_PROPERTY_GROUP_CUSTOM property group. This group represents backend defined
/// properties and capabilities. These may include hardware capabilities and properties.
///

/**
 * @brief Property group for custom backend capabilities and properties.
 */
#define QNN_PROPERTY_GROUP_CUSTOM QNN_PROPERTY_GROUP_CORE + 2000

//=============================================================================
// Data Types
//=============================================================================

/**
 * @brief Type used for unique property identifiers for both capabilities
 *        and properties with value. Valid property values are determined by
 *        property documentation.
 */
typedef uint32_t QnnProperty_Key_t;

/**
 * @brief QNN Property API result / error codes.
 */
typedef enum {
  QNN_PROPERTY_MIN_ERROR = QNN_MIN_ERROR_PROPERTY,
  //////////////////////////////////////////////

  QNN_PROPERTY_NO_ERROR = QNN_SUCCESS,
  /// Property in question is supported
  QNN_PROPERTY_SUPPORTED = QNN_SUCCESS,
  /// Property in question not supported.
  QNN_PROPERTY_NOT_SUPPORTED = QNN_COMMON_ERROR_NOT_SUPPORTED,

  // Remaining values signal errors.

  /// The the property key was not known to a backend.
  QNN_PROPERTY_ERROR_UNKNOWN_KEY = QNN_MIN_ERROR_PROPERTY + 0,
  /// The property key refers to a different property type (capability vs property)
  /// than is queried via the called function.
  QNN_PROPERTY_ERROR_WRONG_KEY_TYPE = QNN_MIN_ERROR_PROPERTY + 1,
  /// An unknown error occurred while attempting to process the property
  /// query. This likely indicates the backend is non-functional.
  QNN_PROPERTY_ERROR_GENERAL = QNN_MIN_ERROR_PROPERTY + 2,
  /// Attempt to free a pointer which is not recognized as a value pointer
  /// previously returned by a QnnProperty_get call on this backend.
  QNN_PROPERTY_ERROR_BAD_FREE = QNN_MIN_ERROR_PROPERTY + 3,

  //////////////////////////////////////////////
  QNN_PROPERTY_MAX_ERROR = QNN_MAX_ERROR_PROPERTY,
  // Unused, present to ensure 32 bits.
  QNN_PROPERTY_ERROR_UNDEFINED = 0x7FFFFFFF
} QnnProperty_Error_t;

/**
 * @brief Struct which encapsulates the fully-qualified name of an operation.
 *        _packageName_ is the op package to which the operation belongs,
 *        _name_ is the type name of the operation, and _target_ is the intended target platform for
 *       the combination of domain and operation name. Target may be unused (NULL) by some backends.
 */
typedef struct {
  const char* packageName;
  const char* name;
  const char* target;
} QnnProperty_OperationName_t;

// clang-format off
/// QnnProperty_OperationName_t initializer macro
#define QNN_PROPERTY_OPERATION_NAME_INIT \
  {                                      \
    NULL,     /*packageName*/            \
    NULL,     /*name*/                   \
    NULL      /*target*/                 \
  }
// clang-format on

/**
 * @brief Structure describing the set of operations supported by a backend.
 */
typedef struct {
  uint32_t numOperations;
  QnnProperty_OperationName_t* operations;
} QnnProperty_SupportedOperations_t;

// clang-format off
/// QnnProperty_SupportedOperations_t initializer macro
#define QNN_PROPERTY_SUPPORTED_OPERATIONS_INIT \
  {                                            \
    0,       /*numOperations*/                 \
    NULL     /*operations*/                    \
  }
// clang-format on

/**
 * @brief Structure defined by backend to capture custom backend properties.
 */
typedef void* QnnProperty_Custom_t;

//=============================================================================
// Public Functions
//=============================================================================

/**
 * @brief Queries a capability of the backend.
 *
 * @param[in] key   Key which identifies the capability within group.
 *
 * @return Error code:
 *         - QNN_PROPERTY_SUPPORTED: if the backend supports capability.
 *         - QNN_PROPERTY_ERROR_UNKNOWN_KEY: The provided key is not valid.
 *         - QNN_PROPERTY_NOT_SUPPORTED: if the backend does not support capability.
 *         - QNN_PROPERTY_ERROR_WRONG_KEY_TYPE: if the key refers to a property within the group.
 *         - QNN_PROPERTY_ERROR_GENERAL: general other failure
 *
 * @note DEPRECATED: Use corresponding API through QnnInterface_t.
 */
QNN_API
Qnn_ErrorHandle_t QnnProperty_hasCapability(QnnProperty_Key_t key);

/**
 * @brief Query a property of the backend.
 *
 *
 * @param[in]  key Key which identifies the property.
 *
 * @param[out] property Pointer which will be populated with the property value.
 *                      This pointer will be allocated by the backend in an
 *                      implementation-defined way, and may be de-allocated by the caller
 *                      at any time with QnnProperty_free().
 * @return Error code:
 *         - QNN_SUCCESS: on success
 *         - QNN_PROPERTY_ERROR_UNKNOWN_KEY: The provided key is not valid.
 *         - QNN_PROPERTY_ERROR_WRONG_KEY_TYPE: if the key refers to a capability within the group.
 *         - QNN_PROPERTY_ERROR_GENERAL: general other failure (e.g. NULL property pointer)
 *
 * @note DEPRECATED: Use corresponding API through QnnInterface_t.
 */
QNN_API
Qnn_ErrorHandle_t QnnProperty_get(QnnProperty_Key_t key, void** property);

/**
 * @brief Deallocate the memory of a property.
 *
 * @param[in] property Pointer to a property value returned by QnnProperty_get().
 *
 * @return Error code:
 *         - QNN_SUCCESS: success
 *         - QNN_PROPERTY_ERROR_BAD_FREE: failed to free property value
 *         - QNN_PROPERTY_ERROR_GENERAL: general other failure
 *
 * @note DEPRECATED: Use corresponding API through QnnInterface_t.
 */
QNN_API
Qnn_ErrorHandle_t QnnProperty_free(void* property);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif
