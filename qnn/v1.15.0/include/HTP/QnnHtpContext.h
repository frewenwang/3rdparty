//=============================================================================
//
//  Copyright (c) 2022 Qualcomm Technologies, Inc.
//  All Rights Reserved.
//  Confidential and Proprietary - Qualcomm Technologies, Inc.
//
//=============================================================================

/** @file
 *  @brief QNN HTP component Context API.
 *
 *         The interfaces in this file work with the top level QNN
 *         API and supplements QnnContext.h for HTP context
 */

#ifndef QNN_HTP_CONTEXT_H
#define QNN_HTP_CONTEXT_H

#ifdef __cplusplus
extern "C" {
#endif

//=============================================================================
// Macros
//=============================================================================

//=============================================================================
// Data Types
//=============================================================================

//=============================================================================
// Public Functions
//=============================================================================

//------------------------------------------------------------------------------
//   Implementation Definition
//------------------------------------------------------------------------------

// clang-format off

/* @brief Enum describing the set of custom configs supported by HTP context.
*/
typedef enum {
  ///  option to select which device (htp instance) to use for context
  QNN_HTP_CONTEXT_CONFIG_OPTION_DEVICE_ID = 0,
  /// UNKNOWN enum option that must not be used
  QNN_HTP_CONTEXT_CONFIG_OPTION_UNKNOWN = 0x7fffffff
} QnnHtpContext_ConfigOption_t;

/**
 * @brief Structure describing the set of configurations supported by the context.
 *        Objects of this type are to be referenced through QnnContext_CustomConfig_t.
 */
typedef struct QnnHtpContext_CustomConfig {
  QnnHtpContext_ConfigOption_t option;
  union UNNAMED {
    // Only supported values are 0 and 1.
    uint32_t deviceId;
  };
} QnnHtpContext_CustomConfig_t ;

/// QnnHtpcontext_CustomConfig_t initializer macro
#define QNN_HTP_CONTEXT_CUSTOM_CONFIG_INIT \
  {                                                   \
    QNN_HTP_CONTEXT_CONFIG_OPTION_UNKNOWN, /*option*/ \
    {                                                 \
      0 /*deviceId*/                                  \
    }                                                 \
  }

// clang-format on
#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // QNN_HTP_CONTEXT_H
