//=============================================================================
//
//  Copyright (c) 2020-2022 Qualcomm Technologies, Inc.
//  All Rights Reserved.
//  Confidential and Proprietary - Qualcomm Technologies, Inc.
//
//=============================================================================

/** @file
 *  @brief QNN Saver component API.
 *
 *         Provides an interface to the client to allow configuration
 *         of settings that are specific to the Saver Backend
 */

#ifndef QNN_SAVER_H
#define QNN_SAVER_H

#include "QnnTypes.h"

#ifdef __cplusplus
extern "C" {
#endif

//=============================================================================
// Macros
//=============================================================================

// Macro controlling visibility of Saver API
#ifndef QNN_SAVER_API
#define QNN_SAVER_API
#endif

//=============================================================================
// Data Types
//=============================================================================

/**
 * @brief QNN Saver API result / error codes.
 */
typedef enum {
  QNN_SAVER_MIN_ERROR = QNN_MIN_ERROR_BACKEND_SAVER,
  ////////////////////////////////////////

  /// The API has been recorded by Saver, however the return value is fake and should not be used.
  /// This error code is generally returned from get() APIs where Saver has no capability to
  /// actually fulfill the request, but can still record the API in saver_output.c.
  /// Saver will return this error code from the following QNN APIs:
  ///   - QnnBackend_getPerfInfrastructure()
  ///   - QnnContext_getBinarySize()
  ///   - QnnContext_getBinary()
  ///   - QnnProperty_hasCapability()
  ///   - QnnProperty_get()
  ///   - QnnProfile_getEvents()
  ///   - QnnProfile_getSubEvents()
  ///   - QnnProfile_getEventData()
  QNN_SAVER_ERROR_DUMMY_RETVALUE = QNN_MIN_ERROR_BACKEND_SAVER + 0,
  /// The API must be called before any others, but  backend instance has already been instantiated.
  QNN_SAVER_ERROR_ALREADY_INSTANTIATED = QNN_MIN_ERROR_BACKEND_SAVER + 1,

  ////////////////////////////////////////
  QNN_SAVER_MAX_ERROR = QNN_MAX_ERROR_BACKEND_SAVER
} QnnSaver_Error_t;

/**
 * @brief This enum contains the supported config options for Saver
 */
typedef enum {
  /// Configuration of the location Saver outputs
  QNN_SAVER_CONFIG_OPTION_OUTPUT_DIRECTORY = 0,
  /// Configuration of timestamp appended to Saver outputs
  QNN_SAVER_CONFIG_OPTION_APPEND_TIMESTAMP = 1,
  /// Configuration indicating to Saver which backend to interpret custom configs as.
  /// This option should only be used if you are providing custom configs to QNN APIs
  /// that support them (e.g. QnnBackend_Initialize()) and you want these custom
  /// configs to be recorded by Saver.
  QNN_SAVER_CONFIG_OPTION_BACKEND_ID = 2,
  // Unused, present to ensure 32 bits.
  QNN_SAVER_CONFIG_OPTION_UNDEFINED = 0x7FFFFFFF
} QnnSaver_ConfigOption_t;

/**
 * @brief A struct that provides configuration for Saver
 */
typedef struct {
  /// Type of Saver configuration option
  QnnSaver_ConfigOption_t option;
  /// Union of mutually exclusive config values based on
  /// the type specified by 'option'.
  union UNNAMED {
    /// Path to a directory where Saver output should be stored. The directory will
    /// be created if it doesn't exist already. If a relative filepath is given, the location is
    /// relative to the current working directory. Defaults to "./saver_output/" if not provided.
    const char* outputDirectory;
    /// Boolean flag to indicate if a timestamp should be appended to the
    /// filename of Saver outputs to prevent them from being overwritten during
    /// consecutive uses of Saver. Note that all input tensor data is dumped into params.bin, so
    /// this setting may use lots of storage over time. Any nonzero value will enable the timestamp.
    /// Defaults to 0 (false) if not provided.
    uint8_t appendTimestamp;
    /// Backend identifier indicating which backend to interpret custom configs as.
    /// These identifiers are defined by each backend in a Qnn<Backend>Common.h file
    /// included with the SDK.
    uint32_t backendId;
  };
} QnnSaver_Config_t;

// clang-format off
/// QnnSaver_Config_t initializer macro
#define QNN_SAVER_CONFIG_INIT                     \
  {                                               \
    QNN_SAVER_CONFIG_OPTION_UNDEFINED, /*option*/ \
    {                                             \
      NULL /*outputDirectory*/                    \
    }                                             \
  }
// clang-format on

//=============================================================================
// API Methods
//=============================================================================

/**
 * @brief Supply the Saver backend with configuration options.
 *        Must be called before QnnBackend_Initialize().
 *        This function only needs to be called if you are providing configs to Saver.
 *        If no configuration is needed, you may simply call QnnBackend_Initialize()
 *        to initialize the Saver.
 *
 * @param[in] config Pointer to a NULL terminated array of config option pointers.
 *                   NULL is allowed and indicates no config options are provided,
 *                   however this function only serves to supply configs, so it
 *                   is unnecessary to call if no configuration is desired.
 *                   All config options have a default value, in case not provided.
 *                   If the same config option type is provided multiple times,
 *                   the last option value will be used.
 *
 * @return Error code:
 *         - QNN_SUCCESS: No error encountered
 *         - QNN_COMMON_ERROR_INVALID_ARGUMENT: A config was supplied incorrectly
 *         - QNN_SAVER_ERROR_ALREADY_INSTANTIATED: Saver backend was already initialized
 */

QNN_SAVER_API
Qnn_ErrorHandle_t QnnSaver_initialize(const QnnSaver_Config_t** config);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // QNN_SAVER_H
