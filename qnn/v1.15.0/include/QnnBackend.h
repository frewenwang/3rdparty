//=============================================================================
//
//  Copyright (c) 2019-2021 Qualcomm Technologies, Inc.
//  All Rights Reserved.
//  Confidential and Proprietary - Qualcomm Technologies, Inc.
//
//=============================================================================

/**
 *  @file
 *  @brief  Backend component API.
 *
 *          This is top level QNN API component.
 *          Most of the QNN API requires backend to be initialized first.
 */

#ifndef QNN_BACKEND_H
#define QNN_BACKEND_H

#include "QnnCommon.h"
#include "QnnTypes.h"

#ifdef __cplusplus
extern "C" {
#endif

//=============================================================================
// Macros
//=============================================================================

//=============================================================================
// Data Types
//=============================================================================

/**
 * @brief QNN Backend API result / error codes.
 */
typedef enum {
  QNN_BACKEND_MIN_ERROR = QNN_MIN_ERROR_BACKEND,
  ////////////////////////////////////////////

  /// Qnn Backend success
  QNN_BACKEND_NO_ERROR = QNN_SUCCESS,
  /// Backend hasn't been initialized
  QNN_BACKEND_ERROR_NOT_INITIALIZED = QNN_COMMON_ERROR_BACKEND_NOT_INITIALIZED,
  /// General error relating to memory allocation in Backend API
  QNN_BACKEND_ERROR_MEM_ALLOC = QNN_COMMON_ERROR_MEM_ALLOC,
  /// Backend attempted to be initialized on an unsupported platform
  QNN_BACKEND_ERROR_UNSUPPORTED_PLATFORM = QNN_COMMON_ERROR_PLATFORM_NOT_SUPPORTED,
  /// Backend failed to initialize
  QNN_BACKEND_ERROR_CANNOT_INITIALIZE = QNN_MIN_ERROR_BACKEND + 0,
  /// Backend is already initialized
  QNN_BACKEND_ERROR_ALREADY_INITIALIZED = QNN_MIN_ERROR_BACKEND + 1,
  /// Failed to free allocated resources during termination
  QNN_BACKEND_ERROR_TERMINATE_FAILED = QNN_MIN_ERROR_BACKEND + 2,
  /// Backend does not support requested functionality
  QNN_BACKEND_ERROR_NOT_SUPPORTED = QNN_MIN_ERROR_BACKEND + 3,
  /// Invalid function argument
  QNN_BACKEND_ERROR_INVALID_ARGUMENT = QNN_MIN_ERROR_BACKEND + 4,
  /// Could not find specified op package
  QNN_BACKEND_ERROR_OP_PACKAGE_NOT_FOUND = QNN_MIN_ERROR_BACKEND + 5,
  /// Could not load interface provider from op package library
  QNN_BACKEND_ERROR_OP_PACKAGE_IF_PROVIDER_NOT_FOUND = QNN_MIN_ERROR_BACKEND + 6,
  /// Failed to register op package
  QNN_BACKEND_ERROR_OP_PACKAGE_REGISTRATION_FAILED = QNN_MIN_ERROR_BACKEND + 7,
  /// Backend does not support the op config's interface version
  QNN_BACKEND_ERROR_OP_PACKAGE_UNSUPPORTED_VERSION = QNN_MIN_ERROR_BACKEND + 8,
  /// An Op with the same package name and op name was already registered
  QNN_BACKEND_ERROR_OP_PACKAGE_DUPLICATE = QNN_MIN_ERROR_BACKEND + 9,

  ////////////////////////////////////////////
  QNN_BACKEND_MAX_ERROR = QNN_MAX_ERROR_BACKEND,
  // Unused, present to ensure 32 bits.
  QNN_BACKEND_ERROR_UNDEFINED = 0x7FFFFFFF
} QnnBackend_Error_t;

/**
 * @brief Backend specific opaque performance infrastructure object
 *
 * Please refer to documentation provided by the backend for usage information.
 */
typedef void* QnnBackend_PerfInfrastructure_t;

/**
 * @brief Backend specific object for custom configuration
 *
 * Please refer to documentation provided by the backend for usage information
 */
typedef void* QnnBackend_CustomConfig_t;

/**
 * @brief This enum defines backend config options.
 */
typedef enum {
  /// sets backend custom options via QnnBackend_CustomConfig_t
  QNN_BACKEND_CONFIG_OPTION_CUSTOM = 0,
  /// sets error reporting level
  QNN_BACKEND_CONFIG_OPTION_ERROR_REPORTING = 1,
  // Unused, present to ensure 32 bits.
  QNN_BACKEND_CONFIG_OPTION_UNDEFINED = 0x7FFFFFFF
} QnnBackend_ConfigOption_t;

/**
 * @brief This struct provides backend configuration.
 */
typedef struct {
  QnnBackend_ConfigOption_t option;
  union UNNAMED {
    QnnBackend_CustomConfig_t customConfig;
    /// applies error reporting configuration across backend.
    /// All QNN contexts share this common error configuration
    /// for APIs that are independent of a context.
    Qnn_ErrorReportingConfig_t errorConfig;
  };
} QnnBackend_Config_t;

/// QnnBackend_Config_t initializer macro
#define QNN_BACKEND_CONFIG_INIT                     \
  {                                                 \
    QNN_BACKEND_CONFIG_OPTION_UNDEFINED, /*option*/ \
    {                                               \
      NULL /*customConfig*/                         \
    }                                               \
  }

//=============================================================================
// Public Functions
//=============================================================================

/**
 * @brief Initialize a backend library.
 *        Function is not re-entrant and must be called once when backend library is loaded.
 *        Once successfully initialized, backend library must be terminated before
 *        attempting another initialize call.
 *
 * @param[in] config Pointer to a NULL terminated array of config option pointers.
 *                   NULL is allowed and indicates no config options are provided.
 *                   All config options have default value, in case not provided.
 *                   If same config option type is provided multiple times,
 *                   the last option value will be used.
 *
 * @return Error code:
 *         - QNN_SUCCESS: No error encountered
 *         - QNN_BACKEND_ERROR_UNSUPPORTED_PLATFORM: Backend attempted to be initialized on
 *           unsupported platform
 *         - QNN_BACKEND_ERROR_ALREADY_INITIALIZED: backend was already successfully initialized
 *         - QNN_BACKEND_ERROR_CANNOT_INITIALIZE: backend failed to initialize
 *         - QNN_BACKEND_ERROR_MEM_ALLOC: error related to memory allocation
 *
 * @note DEPRECATED: Use corresponding API through QnnInterface_t.
 */
QNN_API
Qnn_ErrorHandle_t QnnBackend_initialize(const QnnBackend_Config_t** config);

/**
 * @brief Get the QNN API version
 *
 * @note Safe to call any time, backend does not have to be initialized
 *
 * @param[out] pVersion Pointer to version object.
 *
 * @return Error code:
 *         - QNN_SUCCESS: No error encountered
 *         - QNN_BACKEND_ERROR_INVALID_ARGUMENT: if _pVersion_ was NULL
 *
 * @note DEPRECATED: Use corresponding API through QnnInterface_t.
 */
QNN_API
Qnn_ErrorHandle_t QnnBackend_getApiVersion(Qnn_ApiVersion_t* pVersion);

/**
 * @brief Get build id for backend library.
 *
 * @param[out] id Pointer to string containing the build id.
 *
 * @return Error code:
 *         - QNN_SUCCESS: No error encountered
 *         - QNN_BACKEND_ERROR_NOT_SUPPORTED: No build ID is available
 *         - QNN_BACKEND_ERROR_INVALID_ARGUMENT: if _id_ is NULL
 *
 * @note DEPRECATED: Use corresponding API through QnnInterface_t.
 */
QNN_API
Qnn_ErrorHandle_t QnnBackend_getBuildId(const char** id);

/**
 * @brief Get backend's infrastructure interface object for performance settings.
 *
 * This is optional capability, support is advertised via QnnProperty.
 * If supported, please refer to documentation and/or header file provided by the backend for
 * usage information.
 *
 * @note QnnBackend must be initialized (see QnnBackend_initialize).
 *
 * @param[out] pPerfInfra Pointer to performance infrastructure interface object. The
 *                        pointer returned is a backend owned memory.
 *
 * @return Error code:
 *         - QNN_SUCCESS: No error encountered
 *         - QNN_BACKEND_ERROR_NOT_INITIALIZED: QnnBackend not initialized
 *         - QNN_BACKEND_ERROR_NOT_SUPPORTED: Performance infrastructure is not supported
 *         - QNN_BACKEND_ERROR_INVALID_ARGUMENT: if _pPerfInfra_ is NULL
 *
 * @note DEPRECATED: Use corresponding API through QnnInterface_t.
 */
QNN_API
Qnn_ErrorHandle_t QnnBackend_getPerfInfrastructure(
    const QnnBackend_PerfInfrastructure_t* pPerfInfra);

/**
 * @brief register an operation package with the backend library.
 *
 * @param[in] packagePath       Path on disk to the op package library to load.
 *
 * @param[in] interfaceProvider The name of a function in the op package
 *                              library which satisfies the
 *                              QnnOpPackage_InterfaceProvider_t interface.
 *                              The backend will use this function to retrieve
 *                              the op package's interface.
 *
 * @param[in] target            An optional parameter specifying the target platform on which
 *                              the backend must register the op package. Required in scenarios
 *                              where an op package is to be loaded on a processing unit that is
 *                              different from the target on which the backend runs.
 *                              Ex: loading a DSP op package on ARM for optional
 *                              online context caching. Refer to additional documentation for
 *                              a list of permissible target names.
 *
 * @return Error code:
 *         - QNN_SUCCESS: No error encountered
 *         - QNN_BACKEND_ERROR_NOT_INITIALIZED: QnnBackend not initialized
 *         - QNN_BACKEND_ERROR_INVALID_ARGUMENT: if _packagePath_ or _interfaceProvider_ is NULL
 *         - QNN_BACKEND_ERROR_OP_PACKAGE_NOT_FOUND: Could not open _packagePath_
 *         - QNN_BACKEND_ERROR_OP_PACKAGE_IF_PROVIDER_NOT_FOUND: Could not find _interfaceProvider_
 *           symbol in package library
 *         - QNN_BACKEND_ERROR_OP_PACKAGE_REGISTRATION_FAILED: Op package registration failed
 *         - QNN_BACKEND_ERROR_OP_PACKAGE_UNSUPPORTED_VERSION: Op package has interface version not
 *           supported by this backend
 *         - QNN_BACKEND_ERROR_NOT_SUPPORTED: Op package registration is not supported.
 *         - QNN_BACKEND_ERROR_OP_PACKAGE_DUPLICATE: OpPackageName+OpName must be unique.
 *           Op package content information can be be obtained with QnnOpPackage interface.
 *           Indicates that an Op with the same package name and op name was already registered.
 *
 * @note DEPRECATED: Use corresponding API through QnnInterface_t.
 */
QNN_API
Qnn_ErrorHandle_t QnnBackend_registerOpPackage(const char* packagePath,
                                               const char* interfaceProvider,
                                               const char* target);

/**
 * @brief A method to validate op config with an appropriate op package
 *        This is a wrapper API around the actual OpPackage interface method
 *        that performs op validation. Backend may pick an appropriate op package
 *        among ones that are registered with it for validation based on the attributes
 *        of the op configuration.
 *
 * @param[in] opConfig Fully qualified struct containing the configuration of the operation
 *
 * @note  _inputTensors_ and _outputTensors_ inside opConfig must be fully qualified for
 *        complete validation. However, their unique IDs (_id_) are ignored during validation.
 *
 * @return Error code
 *         - QNN_SUCCESS if validation is successful
 *         - QNN_BACKEND_ERROR_NOT_INITIALIZED: QnnBackend not initialized
 *         - QNN_OP_PACKAGE_ERROR_VALIDATION_FAILURE: op config validation failed
 *         - QNN_BACKEND_ERROR_NOT_SUPPORTED: Validation API not supported
 *         - QNN_BACKEND_ERROR_OP_PACKAGE_NOT_FOUND: No op package with matching
 *           op config attributes found.
 *
 * @note DEPRECATED: Use corresponding API through QnnInterface_t.
 */
QNN_API
Qnn_ErrorHandle_t QnnBackend_validateOpConfig(Qnn_OpConfig_t opConfig);

/**
 * @brief Terminate a backend library.
 *        Backend must be successfully initialized.
 *        Terminate call frees all resources and invalidates all handles and pointers
 *        allocated by the library.
 *
 * @return Error code:
 *         - QNN_SUCCESS: No error encountered
 *         - QNN_BACKEND_ERROR_NOT_INITIALIZED: backend has not been initialized
 *           successfully or has already been terminated
 *         - QNN_BACKEND_ERROR_MEM_ALLOC: error related to memory de-allocation
 *         - QNN_BACKEND_ERROR_TERMINATE_FAILED: indicates failure to free
 *           resources or failure to invalidate handles and pointers allocated
 *           by the library
 *
 * @note DEPRECATED: Use corresponding API through QnnInterface_t.
 */
QNN_API
Qnn_ErrorHandle_t QnnBackend_terminate(void);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif
