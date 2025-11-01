//=============================================================================
//
//  Copyright (c) 2021-2022 Qualcomm Technologies, Inc.
//  All Rights Reserved.
//  Confidential and Proprietary - Qualcomm Technologies, Inc.
//
//=============================================================================

//=============================================================================
// !!! This is an auto-generated file. Do NOT modify manually !!!
//=============================================================================

/**
 *  @file
 *  @brief  QNN Interface API
 *
 *          QNN Interface is an abstraction combining all QNN component APIs.
 *          QNN Interface provides typedef variant of QNN component APIs and
 *          API to get QNN interface object(s).
 *          QNN Interface API can coexist with QNN component APIs. Visibility
 *          of Interface and Component APIs is determined by build configuration,
 *          specifically by QNN_API and QNN_INTERFACE macro definitions.
 */

#ifndef QNN_INTERFACE_H
#define QNN_INTERFACE_H

#include "QnnCommon.h"
#include "QnnTypes.h"

// QNN Component API headers
#include "QnnBackend.h"
#include "QnnContext.h"
#include "QnnGraph.h"
#include "QnnLog.h"
#include "QnnMem.h"
#include "QnnProfile.h"
#include "QnnProperty.h"
#include "QnnTensor.h"

// QNN Op integration headers
#include "QnnOpDef.h"
#include "QnnOpPackage.h"

#ifdef __cplusplus
extern "C" {
#endif

//=============================================================================
// Macros
//=============================================================================

// Macro controlling visibility of QNN Interface API
#ifndef QNN_INTERFACE
#define QNN_INTERFACE
#endif

// Utility macros for version and name construction
#define QNN_INTERFACE_VER_EVAL(major, minor)          QNN_PASTE_THREE(major, _, minor)
#define QNN_INTERFACE_NAME_EVAL(prefix, body, suffix) QNN_PASTE_THREE(prefix, body, suffix)

// Construct interface type name from version, e.g. QnnInterface_ImplementationV0_0_t
#define QNN_INTERFACE_VER_TYPE_EVAL(ver_major, ver_minor) \
  QNN_INTERFACE_NAME_EVAL(                                \
      QnnInterface_ImplementationV, QNN_INTERFACE_VER_EVAL(ver_major, ver_minor), _t)

// Construct interface name from version, e.g. v0_0
#define QNN_INTERFACE_VER_NAME_EVAL(ver_major, ver_minor) \
  QNN_INTERFACE_NAME_EVAL(v, QNN_INTERFACE_VER_EVAL(ver_major, ver_minor), )

// Interface type name for current API version
#define QNN_INTERFACE_VER_TYPE \
  QNN_INTERFACE_VER_TYPE_EVAL(QNN_API_VERSION_MAJOR, QNN_API_VERSION_MINOR)

// Interface name for current API version
#define QNN_INTERFACE_VER_NAME \
  QNN_INTERFACE_VER_NAME_EVAL(QNN_API_VERSION_MAJOR, QNN_API_VERSION_MINOR)

//=============================================================================
// Data Types
//=============================================================================

/**
 * @brief QNN Interface API result / error codes
 */
typedef enum {
  QNN_INTERFACE_MIN_ERROR = QNN_MIN_ERROR_INTERFACE,
  ////////////////////////////////////////

  QNN_INTERFACE_NO_ERROR                = QNN_SUCCESS,
  QNN_INTERFACE_ERROR_NOT_SUPPORTED     = QNN_COMMON_ERROR_NOT_SUPPORTED,
  QNN_INTERFACE_ERROR_INVALID_PARAMETER = QNN_COMMON_ERROR_INVALID_ARGUMENT,

  ////////////////////////////////////////
  QNN_INTERFACE_MAX_ERROR = QNN_MAX_ERROR_INTERFACE
} QnnInterface_Error_t;

//
// From QnnProperty.h
//

/** @brief See QnnProperty_hasCapability()*/
typedef Qnn_ErrorHandle_t (*QnnProperty_HasCapabilityFn_t)(QnnProperty_Key_t key);

/** @brief See QnnProperty_get()*/
typedef Qnn_ErrorHandle_t (*QnnProperty_GetFn_t)(QnnProperty_Key_t key, void** property);

/** @brief See QnnProperty_free()*/
typedef Qnn_ErrorHandle_t (*QnnProperty_FreeFn_t)(void* property);

//
// From QnnBackend.h
//

/** @brief See QnnBackend_initialize()*/
typedef Qnn_ErrorHandle_t (*QnnBackend_InitializeFn_t)(const QnnBackend_Config_t** config);

/** @brief See QnnBackend_getApiVersion()*/
typedef Qnn_ErrorHandle_t (*QnnBackend_GetApiVersionFn_t)(Qnn_ApiVersion_t* pVersion);

/** @brief See QnnBackend_getBuildId()*/
typedef Qnn_ErrorHandle_t (*QnnBackend_GetBuildIdFn_t)(const char** id);

/** @brief See QnnBackend_getPerfInfrastructure()*/
typedef Qnn_ErrorHandle_t (*QnnBackend_GetPerfInfrastructureFn_t)(
    const QnnBackend_PerfInfrastructure_t* pPerfInfra);

/** @brief See QnnBackend_registerOpPackage()*/
typedef Qnn_ErrorHandle_t (*QnnBackend_RegisterOpPackageFn_t)(const char* packagePath,
                                                              const char* interfaceProvider,
                                                              const char* target);

/** @brief See QnnBackend_validateOpConfig()*/
typedef Qnn_ErrorHandle_t (*QnnBackend_ValidateOpConfigFn_t)(Qnn_OpConfig_t opConfig);

/** @brief See QnnBackend_terminate()*/
typedef Qnn_ErrorHandle_t (*QnnBackend_TerminateFn_t)(void);

//
// From QnnContext.h
//

/** @brief See QnnContext_create()*/
typedef Qnn_ErrorHandle_t (*QnnContext_CreateFn_t)(const QnnContext_Config_t** config,
                                                   Qnn_ContextHandle_t* context);

/** @brief See QnnContext_setConfig()*/
typedef Qnn_ErrorHandle_t (*QnnContext_SetConfigFn_t)(Qnn_ContextHandle_t context,
                                                      const QnnContext_Config_t** config);

/** @brief See QnnContext_getBinarySize()*/
typedef Qnn_ErrorHandle_t (*QnnContext_GetBinarySizeFn_t)(
    Qnn_ContextHandle_t context, Qnn_ContextBinarySize_t* binaryBufferSize);

/** @brief See QnnContext_getBinary()*/
typedef Qnn_ErrorHandle_t (*QnnContext_GetBinaryFn_t)(Qnn_ContextHandle_t context,
                                                      void* binaryBuffer,
                                                      Qnn_ContextBinarySize_t binaryBufferSize,
                                                      Qnn_ContextBinarySize_t* writtenBufferSize);

/** @brief See QnnContext_createFromBinary()*/
typedef Qnn_ErrorHandle_t (*QnnContext_CreateFromBinaryFn_t)(
    const void* binaryBuffer,
    Qnn_ContextBinarySize_t binaryBufferSize,
    Qnn_ContextHandle_t* context,
    Qnn_ProfileHandle_t profile);

/** @brief See QnnContext_createFromBinaryWithConfig()*/
typedef Qnn_ErrorHandle_t (*QnnContext_CreateFromBinaryWithConfigFn_t)(
    const QnnContext_Config_t** config,
    const void* binaryBuffer,
    Qnn_ContextBinarySize_t binaryBufferSize,
    Qnn_ContextHandle_t* context,
    Qnn_ProfileHandle_t profile);

/** @brief See QnnContext_free()*/
typedef Qnn_ErrorHandle_t (*QnnContext_FreeFn_t)(Qnn_ContextHandle_t context,
                                                 Qnn_ProfileHandle_t profile);

//
// From QnnGraph.h
//

/** @brief See QnnGraph_create()*/
typedef Qnn_ErrorHandle_t (*QnnGraph_CreateFn_t)(Qnn_ContextHandle_t contextHandle,
                                                 const char* graphName,
                                                 const QnnGraph_Config_t** config,
                                                 Qnn_GraphHandle_t* graphHandle);

/** @brief See QnnGraph_addNode()*/
typedef Qnn_ErrorHandle_t (*QnnGraph_AddNodeFn_t)(Qnn_GraphHandle_t graphHandle,
                                                  Qnn_OpConfig_t opConfig);

/** @brief See QnnGraph_finalize()*/
typedef Qnn_ErrorHandle_t (*QnnGraph_FinalizeFn_t)(Qnn_GraphHandle_t graphHandle,
                                                   Qnn_ProfileHandle_t profileHandle,
                                                   Qnn_SignalHandle_t signalHandle);

/** @brief See QnnGraph_retrieve()*/
typedef Qnn_ErrorHandle_t (*QnnGraph_RetrieveFn_t)(Qnn_ContextHandle_t contextHandle,
                                                   const char* graphName,
                                                   Qnn_GraphHandle_t* graphHandle);

/** @brief See QnnGraph_execute()*/
typedef Qnn_ErrorHandle_t (*QnnGraph_ExecuteFn_t)(Qnn_GraphHandle_t graphHandle,
                                                  const Qnn_Tensor_t* inputs,
                                                  uint32_t numInputs,
                                                  Qnn_Tensor_t* outputs,
                                                  uint32_t numOutputs,
                                                  Qnn_ProfileHandle_t profileHandle,
                                                  Qnn_SignalHandle_t signalHandle);

/** @brief See QnnGraph_executeAsync()*/
typedef Qnn_ErrorHandle_t (*QnnGraph_ExecuteAsyncFn_t)(Qnn_GraphHandle_t graphHandle,
                                                       const Qnn_Tensor_t* inputs,
                                                       uint32_t numInputs,
                                                       Qnn_Tensor_t* outputs,
                                                       uint32_t numOutputs,
                                                       Qnn_ProfileHandle_t profileHandle,
                                                       Qnn_SignalHandle_t signalHandle,
                                                       Qnn_NotifyFn_t notifyFn,
                                                       void* notifyParam);

//
// From QnnTensor.h
//

/** @brief See QnnTensor_createContextTensor()*/
typedef Qnn_ErrorHandle_t (*QnnTensor_CreateContextTensorFn_t)(Qnn_ContextHandle_t context,
                                                               const Qnn_Tensor_t tensor);

/** @brief See QnnTensor_createGraphTensor()*/
typedef Qnn_ErrorHandle_t (*QnnTensor_CreateGraphTensorFn_t)(Qnn_GraphHandle_t graph,
                                                             const Qnn_Tensor_t tensor);

//
// From QnnLog.h
//

/** @brief See QnnLog_initialize()*/
typedef Qnn_ErrorHandle_t (*QnnLog_InitializeFn_t)(QnnLog_Callback_t callback,
                                                   QnnLog_Level_t maxLogLevel);

/** @brief See QnnLog_setLogLevel()*/
typedef Qnn_ErrorHandle_t (*QnnLog_SetLogLevelFn_t)(QnnLog_Level_t maxLogLevel);

/** @brief See QnnLog_terminate()*/
typedef Qnn_ErrorHandle_t (*QnnLog_TerminateFn_t)(void);

//
// From QnnProfile.h
//

/** @brief See QnnProfile_create()*/
typedef Qnn_ErrorHandle_t (*QnnProfile_CreateFn_t)(QnnProfile_Level_t level,
                                                   Qnn_ProfileHandle_t* profile);

/** @brief See QnnProfile_getEvents()*/
typedef Qnn_ErrorHandle_t (*QnnProfile_GetEventsFn_t)(Qnn_ProfileHandle_t profile,
                                                      const QnnProfile_EventId_t** profileEventIds,
                                                      uint32_t* numEvents);

/** @brief See QnnProfile_getSubEvents()*/
typedef Qnn_ErrorHandle_t (*QnnProfile_GetSubEventsFn_t)(QnnProfile_EventId_t eventId,
                                                         const QnnProfile_EventId_t** subEventIds,
                                                         uint32_t* numSubEvents);

/** @brief See QnnProfile_getEventData()*/
typedef Qnn_ErrorHandle_t (*QnnProfile_GetEventDataFn_t)(QnnProfile_EventId_t eventId,
                                                         QnnProfile_EventData_t* eventData);

/** @brief See QnnProfile_free()*/
typedef Qnn_ErrorHandle_t (*QnnProfile_FreeFn_t)(Qnn_ProfileHandle_t profile);

//
// From QnnMem.h
//

/** @brief See QnnMem_register()*/
typedef Qnn_ErrorHandle_t (*QnnMem_RegisterFn_t)(const Qnn_MemDescriptor_t* memDescriptors,
                                                 uint32_t numDescriptors,
                                                 Qnn_MemHandle_t* memHandles);

/** @brief See QnnMem_deRegister()*/
typedef Qnn_ErrorHandle_t (*QnnMem_DeRegisterFn_t)(const Qnn_MemHandle_t* memHandles,
                                                   uint32_t numHandles);

// clang-format off

/**
 * @brief This struct defines Qnn interface specific to version.
 *        Interface functions are allowed to be NULL if not supported/available.
 *
 */
typedef struct {
  QnnProperty_HasCapabilityFn_t              propertyHasCapability;
  QnnProperty_GetFn_t                        propertyGet;
  QnnProperty_FreeFn_t                       propertyFree;

  QnnBackend_InitializeFn_t                  backendInitialize;
  QnnBackend_GetApiVersionFn_t               backendGetApiVersion;
  QnnBackend_GetBuildIdFn_t                  backendGetBuildId;
  QnnBackend_GetPerfInfrastructureFn_t       backendGetPerfInfrastructure;
  QnnBackend_RegisterOpPackageFn_t           backendRegisterOpPackage;
  QnnBackend_ValidateOpConfigFn_t            backendValidateOpConfig;
  QnnBackend_TerminateFn_t                   backendTerminate;

  QnnContext_CreateFn_t                      contextCreate;
  QnnContext_SetConfigFn_t                   contextSetConfig;
  QnnContext_GetBinarySizeFn_t               contextGetBinarySize;
  QnnContext_GetBinaryFn_t                   contextGetBinary;
  QnnContext_CreateFromBinaryFn_t            contextCreateFromBinary;
  QnnContext_CreateFromBinaryWithConfigFn_t  contextCreateFromBinaryWithConfig;
  QnnContext_FreeFn_t                        contextFree;

  QnnGraph_CreateFn_t                        graphCreate;
  QnnGraph_AddNodeFn_t                       graphAddNode;
  QnnGraph_FinalizeFn_t                      graphFinalize;
  QnnGraph_RetrieveFn_t                      graphRetrieve;
  QnnGraph_ExecuteFn_t                       graphExecute;
  QnnGraph_ExecuteAsyncFn_t                  graphExecuteAsync;

  QnnTensor_CreateContextTensorFn_t          tensorCreateContextTensor;
  QnnTensor_CreateGraphTensorFn_t            tensorCreateGraphTensor;

  QnnLog_InitializeFn_t                      logInitialize;
  QnnLog_SetLogLevelFn_t                     logSetLogLevel;
  QnnLog_TerminateFn_t                       logTerminate;

  QnnProfile_CreateFn_t                      profileCreate;
  QnnProfile_GetEventsFn_t                   profileGetEvents;
  QnnProfile_GetSubEventsFn_t                profileGetSubEvents;
  QnnProfile_GetEventDataFn_t                profileGetEventData;
  QnnProfile_FreeFn_t                        profileFree;

  QnnMem_RegisterFn_t                        memRegister;
  QnnMem_DeRegisterFn_t                      memDeRegister;

} QNN_INTERFACE_VER_TYPE;

/// QNN_INTERFACE_VER_TYPE initializer macro
#define QNN_INTERFACE_VER_TYPE_INIT { \
  NULL, /*propertyHasCapability*/ \
  NULL, /*propertyGet*/ \
  NULL, /*propertyFree*/ \
  NULL, /*backendInitialize*/ \
  NULL, /*backendGetApiVersion*/ \
  NULL, /*backendGetBuildId*/ \
  NULL, /*backendGetPerfInfrastructure*/ \
  NULL, /*backendRegisterOpPackage*/ \
  NULL, /*backendValidateOpConfig*/ \
  NULL, /*backendTerminate*/ \
  NULL, /*contextCreate*/ \
  NULL, /*contextSetConfig*/ \
  NULL, /*contextGetBinarySize*/ \
  NULL, /*contextGetBinary*/ \
  NULL, /*contextCreateFromBinary*/ \
  NULL, /*contextCreateFromBinaryWithConfig*/ \
  NULL, /*contextFree*/ \
  NULL, /*graphCreate*/ \
  NULL, /*graphAddNode*/ \
  NULL, /*graphFinalize*/ \
  NULL, /*graphRetrieve*/ \
  NULL, /*graphExecute*/ \
  NULL, /*graphExecuteAsync*/ \
  NULL, /*tensorCreateContextTensor*/ \
  NULL, /*tensorCreateGraphTensor*/ \
  NULL, /*logInitialize*/ \
  NULL, /*logSetLogLevel*/ \
  NULL, /*logTerminate*/ \
  NULL, /*profileCreate*/ \
  NULL, /*profileGetEvents*/ \
  NULL, /*profileGetSubEvents*/ \
  NULL, /*profileGetEventData*/ \
  NULL, /*profileFree*/ \
  NULL, /*memRegister*/ \
  NULL, /*memDeRegister*/ \
}

typedef struct {
  /// Backend identifier. See QnnCommon.h for details.
  /// Allowed to be QNN_BACKEND_ID_NULL in case of single backend library, in which case
  /// clients can deduce backend identifier based on library being loaded.
  uint32_t backendId;
  /// Interface provider name. Allowed to be NULL.
  const char* providerName;
  // API version for provided interface
  Qnn_ApiVersion_t apiVersion;
  union UNNAMED {
    // Core interface type and name: e.g. QnnInterface_ImplementationV0_0_t v0_0;
    QNN_INTERFACE_VER_TYPE  QNN_INTERFACE_VER_NAME;
  };
} QnnInterface_t;

/// QnnInterface_t initializer macro
#define QNN_INTERFACE_INIT                                   \
  {                                                          \
    QNN_BACKEND_ID_NULL,      /*backendId*/                  \
    NULL,                     /*providerName*/               \
    QNN_API_VERSION_INIT,     /*apiVersion*/                 \
    {                                                        \
      QNN_INTERFACE_VER_TYPE_INIT /*QNN_INTERFACE_VER_NAME*/ \
    }                                                        \
  }

// clang-format on

//=============================================================================
// Public Functions
//=============================================================================

/**
 * @brief Get list of available interface providers.
 *
 * @param[out] providerList Array containing available interface providers.
 *                          The lifetime of returned interface object pointers
 *                          corresponds to the lifetime of the provider library.
 *                          Contents are to be considered invalid if the provider
 *                          library is terminated/unloaded.
 *                          This function can be called immediately after provider
 *                          library has been loaded.
 * @param[out] numProviders Number of available interface objects in _providerList_.
 *
 * @return Error code:
 *         - QNN_SUCCESS: No error.
 *         - QNN_INTERFACE_INVALID_PARAMETER: Invalid parameter was provided.
 *           Either _providerList_ or _numProviders_ was NULL.
 *         - QNN_INTERFACE_ERROR_NOT_SUPPORTED: API not supported.
 */
QNN_INTERFACE
Qnn_ErrorHandle_t QnnInterface_getProviders(const QnnInterface_t** providerList,
                                            uint32_t* numProviders);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // QNN_INTERFACE_H
