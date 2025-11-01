//==============================================================================
//
// Copyright (c) 2020-2022 Qualcomm Technologies, Inc.
// All Rights Reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
//
//==============================================================================

/**
 *  @file
 *  @brief  A header which defines the QNN GPU specialization of the QnnBackend.h interface.
 */

#ifndef QNN_GPU_BACKEND_H
#define QNN_GPU_BACKEND_H

#ifdef __cplusplus
#include <cstdint>
#else
#include <stdint.h>
#endif

#include "QnnBackend.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief A struct which defines the QNN GPU backend custom configuration options.
 *        Objects of this type are to be referenced through QnnBackend_CustomConfig_t.
 *
 *        @note DEPRECATED: Use QnnGpuContext_CustomConfig_t instead.
 */
typedef struct {
  /// The kernel disk cache directory. Must be non-null and is copied into the backend.
  const char* kernelRepoDir;
  /// If non-zero, the kernel disk cache will be ignored when initializing.
  uint8_t invalidateKernelRepo;
} QnnGpuBackend_CustomConfig_t;

// clang-format off
/// QnnGpuBackend_CustomConfig_t initializer macro
#define QNN_GPU_BACKEND_CUSTOM_CONFIG_INIT \
  {                                        \
    NULL,  /*kernelRepoDir*/               \
    0u     /*invalidateKernelRepo*/        \
  }
// clang-format on

#ifdef __cplusplus
}  // extern "C"
#endif

#endif
