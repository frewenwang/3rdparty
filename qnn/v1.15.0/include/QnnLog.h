//=============================================================================
//
//  Copyright (c) 2019-2021 Qualcomm Technologies, Inc.
//  All Rights Reserved.
//  Confidential and Proprietary - Qualcomm Technologies, Inc.
//
//=============================================================================

/**
 *  @file
 *  @brief  Logging component API.
 *
 *          Provides means for QNN backends to output logging data.
 */

#ifndef QNN_LOG_H
#define QNN_LOG_H

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
 * @brief QNN Log API result / error codes.
 */
typedef enum {
  QNN_LOG_MIN_ERROR = QNN_MIN_ERROR_LOG,
  ////////////////////////////////////

  /// Qnn Log success
  QNN_LOG_NO_ERROR = QNN_SUCCESS,
  /// General error relating to memory allocation in Log API
  QNN_LOG_ERROR_MEM_ALLOC = QNN_COMMON_ERROR_MEM_ALLOC,
  /// Function could not be called because logging has already been
  /// successfully initialized.
  QNN_LOG_ERROR_ALREADY_INITIALIZED = QNN_MIN_ERROR_LOG + 0,
  /// Function could not be called because logging has not yet
  /// been initialized.
  QNN_LOG_ERROR_NOT_INITIALIZED = QNN_MIN_ERROR_LOG + 1,
  /// Unable to initialize logging
  QNN_LOG_ERROR_INITIALIZATION = QNN_MIN_ERROR_LOG + 2,
  /// Invalid argument passed
  QNN_LOG_ERROR_INVALID_ARGUMENT = QNN_MIN_ERROR_LOG + 3,

  ////////////////////////////////////
  QNN_LOG_MAX_ERROR = QNN_MAX_ERROR_LOG,
  // Unused, present to ensure 32 bits.
  QNN_LOG_ERROR_UNDEFINED = 0x7FFFFFFF
} QnnLog_Error_t;

typedef enum {
  // Enum Levels must be in ascending order, so that the enum value
  // can be compared with the "maximum" set in QnnLog_initialize().
  QNN_LOG_LEVEL_ERROR   = 1,
  QNN_LOG_LEVEL_WARN    = 2,
  QNN_LOG_LEVEL_INFO    = 3,
  QNN_LOG_LEVEL_VERBOSE = 4,
  /// Reserved for developer debugging
  QNN_LOG_LEVEL_DEBUG = 5,
  // Present to ensure 32 bits
  QNN_LOG_LEVEL_MAX = 0x7fffffff
} QnnLog_Level_t;

/**
 * @brief Signature for user-supplied logging callback.
 *
 * @warning  The backend may call this callback from multiple threads, and expects
 *           that it is re-entrant.
 *
 * @param[in] fmt       printf-style message format specifier.
 *
 * @param[in] level     Log level for the message. Will not be higher than the maximum
 *                      specified in QnnLog_initialize.
 *
 * @param[in] timestamp Backend-generated timestamp which is monotonically increasing,
 *                      but otherwise meaningless.
 *
 * @param[in] args      Message-specific parameters, to be used with fmt.
 */
typedef void (*QnnLog_Callback_t)(const char* fmt,
                                  QnnLog_Level_t level,
                                  uint64_t timestamp,
                                  va_list args);

//=============================================================================
// Public Functions
//=============================================================================

/**
 * @brief Initialize logging for this backend. The backend will generate no logging messages
 *        before this function is called. Once successfully initialized, logging must be terminated
 *        before attempting another initialize call. This function will also configure logging for
 *        each registered op package (see QnnOpPackage_LogInitializeFn_t). This API function can be
 *        called before QnnBackend_initialize().
 *
 * @param[in] callback    Callback to handle backend-generated logging messages.
 *                        NULL indicates backend may direct log messages to the default log stream
 *                        on the target platform when possible (e.g. to logcat in case of Android).
 *
 * @param[in] maxLogLevel Maximum level of messages which the backend will generate.
 *
 * @return Error code:
 *         - QNN_SUCCESS: if logging is successfully initialized.
 *         - QNN_COMMON_ERROR_NOT_SUPPORTED: logging is not supported.
 *         - QNN_LOG_ERROR_INVALID_ARGUMENT: if one or more arguments is invalid.
 *         - QNN_LOG_ERROR_ALREADY_INITIALIZED: logging was already successfully initialized
 *         - QNN_LOG_ERROR_MEM_ALLOC: for memory allocation errors.
 *         - QNN_LOG_ERROR_INITIALIZATION: log init failed.
 *
 * @note DEPRECATED: Use corresponding API through QnnInterface_t.
 */
QNN_API
Qnn_ErrorHandle_t QnnLog_initialize(QnnLog_Callback_t callback, QnnLog_Level_t maxLogLevel);

/**
 * @brief Change the log level for the currently configured logging stream. Also sets the logging
 *        level for each registered op package (see QnnOpPackage_LogSetLevelFn_t).
 *
 * @param[in] maxLogLevel New maximum log level
 *
 * @return Error code:
 *         - QNN_SUCCESS: if the level is changed successfully.
 *         - QNN_LOG_ERROR_NOT_INITIALIZED: if called before logging has been initialized
 *           by QnnLog_initialize.
 *         - QNN_LOG_ERROR_INVALID_ARGUMENT: if maxLogLevel is not a valid QnnLog_Level_t level.
 *
 * @note DEPRECATED: Use corresponding API through QnnInterface_t.
 */
QNN_API
Qnn_ErrorHandle_t QnnLog_setLogLevel(QnnLog_Level_t maxLogLevel);

/**
 * @brief Terminate a logging configuration. Logging must have been successfully initialized.
 *        Terminate call frees all logging resources allocated by the library. This also calls
 *        terminates logging for each registered op package (see QnnOpPackage_LogTerminateFn_t).
 *
 * @return Error code:
 *         - QNN_SUCCESS: indicates logging is terminated.
 *         - QNN_LOG_ERROR_NOT_INITIALIZED: if logging has not been initialized
 *           successfully or has already been terminated.
 *         - QNN_LOG_ERROR_MEM_ALLOC: for memory de-allocation errors.
 *
 * @note DEPRECATED: Use corresponding API through QnnInterface_t.
 */
QNN_API
Qnn_ErrorHandle_t QnnLog_terminate(void);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif
