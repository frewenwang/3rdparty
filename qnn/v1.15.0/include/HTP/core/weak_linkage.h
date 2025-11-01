//==============================================================================
//
// Copyright (c) 2021-2022 Qualcomm Technologies, Inc.
// All Rights Reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
//
//==============================================================================

#pragma once

#if defined(IMPORT_SYMBOLS) && defined(ENABLE_WEAK_LINKAGE)
#define API_C_FUNC extern
#define API_FUNC_NAME(N) (*N)
#else
#define API_C_FUNC
#define API_FUNC_NAME(N) N
#endif

// Macro API_FUNC_EXPORT to export symbols
#if defined(_MSC_VER)
#define API_FUNC_EXPORT __declspec(dllexport)
#else
#define API_FUNC_EXPORT __attribute__((visibility("default")))
#endif  // _MSC_VER
