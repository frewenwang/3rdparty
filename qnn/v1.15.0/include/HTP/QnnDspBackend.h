//=============================================================================
//
//  Copyright (c) 2022 Qualcomm Technologies, Inc.
//  All Rights Reserved.
//  Confidential and Proprietary - Qualcomm Technologies, Inc.
//
//=============================================================================

/** @file
 *  @brief QNN DSP component Backend API.
 *
 *         The interfaces in this file work with the top level QNN
 *         API and supplements QnnBackend.h for DSP backend
 */

#ifndef QNN_DSP_BACKEND_H
#define QNN_DSP_BACKEND_H

#include "QnnBackend.h"
#include "QnnDspPerfInfrastructure.h"

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

/**
 * @brief QNN DSP backend PerfInfrastructure specialization structure.
 *        Objects of this type are to be referenced through QnnBackend_PerfInfrastructure_t.
 *
 *        Contains function pointers for each interface method for
 *        Dsp PerfInfrastructure.
 */
typedef struct {
    QnnDspPerfInfrastructure_CreatePowerConfigIdFn_t  createPowerConfigId;
    QnnDspPerfInfrastructure_DestroyPowerConfigIdFn_t destroyPowerConfigId;
    QnnDspPerfInfrastructure_SetPowerConfigFn_t       setPowerConfig;
    QnnDspPerfInfrastructure_SetMemoryConfigFn_t      setMemoryConfig;
    QnnDspPerfInfrastructure_SetThreadConfigFn_t      setThreadConfig;
    // v2 apis for device support
    QnnDspPerfInfrastructure_CreatePowerConfigIdFn_v2_t createPowerConfigIdV2;
    QnnDspPerfInfrastructure_SetMemoryConfigFn_v2_t     setMemoryConfigV2;
} QnnDspBackend_PerfInfrastructure_t ;

/// QnnDspBackend_PerfInfrastructure_t initializer macro
#define QNN_DSP_BACKEND_PERF_INFRASTRUCTURE_INIT \
  {                                              \
    NULL,     /*createPowerConfigId*/            \
    NULL,     /*destroyPowerConfigId*/           \
    NULL,     /*setPowerConfig*/                 \
    NULL,     /*setMemoryConfig*/                \
    NULL,     /*setThreadConfig*/                \
    NULL,     /*createPowerConfigIdV2*/          \
    NULL      /*setMemoryConfigV2*/              \
  }

/* @brief Enum describing the set of custom configs supported by DSP backend.
*/
typedef enum {
  ///  The accelerator will always attempt to fold relu activation
  ///  into the immediate preceding convolution operation. This optimization
  ///  is correct when quantization ranges for convolution are equal or
  ///  subset of the Relu operation. For graphs, where this cannot be
  ///  guaranteed, the client should set this option to true
  QNN_DSP_BACKEND_CONFIG_OPTION_FOLD_RELU_ACTIVATION_INTO_CONV_OFF = 0,
  ///  The accelerator will always attempt to all Convolution
  ///  operation using HMX instructions. Convolution that have
  ///  short depth and/or weights that are not symmetric could
  ///  exhibit inaccurate results. In such cases, clients must
  ///  set this option to true to guarantee correctness of the operation
  QNN_DSP_BACKEND_CONFIG_OPTION_SHORT_DEPTH_CONV_ON_HMX_OFF = 1,
  ///  Every APP side user process that uses a DSP via FastRPC
  ///  has a corresponding dynamic user process domain on the DSP side.
  ///  QNN by default opens RPC session as unsigned PD,
  ///  in case this option is set to true,
  ///  RPC session will be opened as signed PD (requires signed .so).
  QNN_DSP_BACKEND_CONFIG_OPTION_USE_SIGNED_PROCESS_DOMAIN = 2,
  /// set QnnDspBackend_DspArch_t for offline prepare mode
  QNN_DSP_BACKEND_CONFIG_OPTION_ARCH = 3,
  /// set socId for offline prepare mode inside QnnDspBackend_CustomConfig
  QNN_DSP_BACKEND_CONFIG_OPTION_SOC = 4,
  /// UNKNOWN enum option that must not be used
  QNN_DSP_BACKEND_CONFIG_OPTION_UNKNOWN = 0x7fffffff
} QnnDspBackend_ConfigOption_t;

typedef enum {
  QNN_DSP_BACKEND_DSP_ARCH_NONE = 0,
  QNN_DSP_BACKEND_DSP_ARCH_V68 = 68,
  QNN_DSP_BACKEND_DSP_ARCH_V69 = 69,
  QNN_DSP_BACKEND_DSP_ARCH_V73 = 73,
  QNN_DSP_BACKEND_DSP_ARCH_UNKNOWN = 0x7fffffff
} QnnDspBackend_DspArch_t;

/**
 * @brief Structure describing the set of configurations supported by the backend.
 *        Objects of this type are to be referenced through QnnBackend_CustomConfig_t.
 */
typedef struct QnnDspBackend_CustomConfig {
  QnnDspBackend_ConfigOption_t option;
  union UNNAMED {
    bool foldReluActivationIntoConvOff;
    bool shortDepthConvOnHmxOff;
    bool useSignedProcessDomain;
    QnnDspBackend_DspArch_t arch;
    const char* socId;
  };
} QnnDspBackend_CustomConfig_t ;

/// QnnDspBackend_CustomConfig_t initializer macro
#define QNN_DSP_BACKEND_CUSTOM_CONFIG_INIT \
  {                                                   \
    QNN_DSP_BACKEND_CONFIG_OPTION_UNKNOWN, /*option*/ \
    {                                                 \
      false /*foldReluActivationIntoConvOff*/         \
    }                                                 \
  }

// clang-format on
#ifdef __cplusplus
}  // extern "C"
#endif

#endif
