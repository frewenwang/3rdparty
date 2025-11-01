//==============================================================================
//
// Copyright (c) 2019-2021 Qualcomm Technologies, Inc.
// All Rights Reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
//
//==============================================================================

/**
 *  @file
 *  @brief  Graph component API
 *
 *          Requires Backend to be initialized.
 *          Provides composable graph API. Graph is created inside Context.
 *          Nodes are added to the graph. Nodes are connected with Tensors.
 *          Once finalized graph can be executed.
 */

#ifndef QNN_GRAPH_H
#define QNN_GRAPH_H

#include "QnnCommon.h"
#include "QnnTypes.h"

#ifdef __cplusplus
extern "C" {
#endif

//=============================================================================
// Data Types
//=============================================================================

/**
 * @brief QNN Graph API result / error codes.
 */
typedef enum {
  QNN_GRAPH_MIN_ERROR = QNN_MIN_ERROR_GRAPH,
  ////////////////////////////////////////

  /// Qnn Graph success
  QNN_GRAPH_NO_ERROR = QNN_SUCCESS,
  /// There is optional API component that is not supported yet. See QnnProperty.
  QNN_GRAPH_ERROR_UNSUPPORTED_FEATURE = QNN_COMMON_ERROR_NOT_SUPPORTED,
  /// General error relating to memory allocation in processing graph API
  QNN_GRAPH_ERROR_MEM_ALLOC = QNN_COMMON_ERROR_MEM_ALLOC,
  /// An argument to QNN API is deemed invalid by a backend
  QNN_GRAPH_ERROR_INVALID_ARGUMENT = QNN_MIN_ERROR_GRAPH + 0,
  /// Invalid graph handle
  QNN_GRAPH_ERROR_INVALID_HANDLE = QNN_MIN_ERROR_GRAPH + 1,
  /// No graph with specified info is registered in the backend
  QNN_GRAPH_ERROR_GRAPH_DOES_NOT_EXIST = QNN_MIN_ERROR_GRAPH + 2,
  /// Invalid or duplicate graph name
  QNN_GRAPH_ERROR_INVALID_NAME = QNN_MIN_ERROR_GRAPH + 3,
  /// Invalid or NULL QNN tensor
  QNN_GRAPH_ERROR_INVALID_TENSOR = QNN_MIN_ERROR_GRAPH + 4,
  /// Some elements in the op config data are invalid
  QNN_GRAPH_ERROR_INVALID_OP_CONFIG = QNN_MIN_ERROR_GRAPH + 5,
  /// Failure to set profile
  QNN_GRAPH_ERROR_SET_PROFILE = QNN_MIN_ERROR_GRAPH + 6,
  /// Node added before its dependent node(s)
  QNN_GRAPH_ERROR_UNCONNECTED_NODE = QNN_MIN_ERROR_GRAPH + 7,

  /// Failure in creating graph with specified configuration
  QNN_GRAPH_ERROR_CREATE_FAILED = QNN_MIN_ERROR_GRAPH + 20,
  /// Graph couldn't be optimized with specified list of ops or config
  QNN_GRAPH_ERROR_OPTIMIZATION_FAILED = QNN_MIN_ERROR_GRAPH + 21,
  /// Graph finalize failed
  QNN_GRAPH_ERROR_FINALIZE_FAILED = QNN_MIN_ERROR_GRAPH + 22,
  /// Graph attempted to be executed before being finalized
  QNN_GRAPH_ERROR_GRAPH_NOT_FINALIZED = QNN_MIN_ERROR_GRAPH + 23,
  /// Graph attempted to be modified after being finalized
  QNN_GRAPH_ERROR_GRAPH_FINALIZED = QNN_MIN_ERROR_GRAPH + 24,
  /// FIFO queue cannot register any more async execution requests
  QNN_GRAPH_ERROR_EXECUTION_ASYNC_FIFO_FULL = QNN_MIN_ERROR_GRAPH + 25,

  /// A control signal object was provided to a call, but that signal object
  /// is already observed by another call.
  QNN_GRAPH_ERROR_SIGNAL_OBSERVED = QNN_MIN_ERROR_GRAPH + 30,
  /// Return when a call is aborted early due to a QnnSignal_abort call issued
  /// to the observed signal object.
  QNN_GRAPH_ERROR_ABORTED = QNN_MIN_ERROR_GRAPH + 31,

  ////////////////////////////////////////
  QNN_GRAPH_MAX_ERROR = QNN_MAX_ERROR_GRAPH,
  // Unused, present to ensure 32 bits.
  QNN_GRAPH_ERROR_UNDEFINED = 0x7FFFFFFF
} QnnGraph_Error_t;

/**
 * @brief This enum defines graph config options.
 */
typedef enum {
  /// sets backend custom configs, see backend specific documentation
  QNN_GRAPH_CONFIG_OPTION_CUSTOM = 0,

  /// sets asyncExeOrder, default is QNN_GRAPH_ASYNC_EXECUTION_ORDER_FIFO
  QNN_GRAPH_CONFIG_OPTION_ASYNC_EXECUTION_ORDER = 1,

  /// Sets async execution queue depth.
  /// Default is QNN_GRAPH_ASYNC_EXECUTION_QUEUE_DEPTH_OPTIMAL.
  /// This option represents number of executions that can be enqueued by
  /// calling QnnGraph_asyncExecute() before getting
  /// QNN_GRAPH_ERROR_EXECUTION_ASYNC_FIFO_FULL error back.
  /// Queue depth is subject to maximum limit determined by the backend and
  /// available system resources.
  QNN_GRAPH_CONFIG_OPTION_ASYNC_EXECUTION_QUEUE_DEPTH = 2,

  // Value selected to ensure 32 bits.
  QNN_GRAPH_CONFIG_OPTION_UNDEFINED = 0x7FFFFFFF
} QnnGraph_ConfigOption_t;

/**
 * @brief Graph specific object for custom configuration
 *
 * Please refer to documentation provided by the backend for usage information
 */
typedef void* QnnGraph_CustomConfig_t;

/**
 * @brief This enum defines graph async execution order.
 */
typedef enum {
  /// Async execution order is FIFO
  QNN_GRAPH_ASYNC_EXECUTION_ORDER_FIFO = 1,

  // Value selected to ensure 32 bits.
  QNN_GRAPH_ASYNC_EXECUTION_ORDER_UNDEFINED = 0x7FFFFFFF
} QnnGraph_AsyncExecutionOrder_t;

/**
 * @brief This enum defines graph async execution queue depth.
 */
typedef enum {
  /// Indicates queue depth is determined by QNN backend for optimal performance.
  QNN_GRAPH_ASYNC_EXECUTION_QUEUE_DEPTH_OPTIMAL = 0,

  /// Indicates queue depth is set to maximum as determined by QNN backend.
  QNN_GRAPH_ASYNC_EXECUTION_QUEUE_DEPTH_MAXIMUM = 1,

  // Value selected to ensure 32 bits.
  QNN_GRAPH_ASYNC_EXECUTION_QUEUE_DEPTH_UNDEFINED = 0x7FFFFFFF
} QnnGraph_AsyncExecutionQueueDepth_t;

/**
 * @brief This struct provides graph configuration.
 */
typedef struct {
  QnnGraph_ConfigOption_t option;
  union UNNAMED {
    QnnGraph_CustomConfig_t customConfig;
    QnnGraph_AsyncExecutionOrder_t asyncExeOrder;
    QnnGraph_AsyncExecutionQueueDepth_t asyncExeQueueDepth;
  };
} QnnGraph_Config_t;

/// QnnGraph_Config_t initializer macro
#define QNN_GRAPH_CONFIG_INIT                     \
  {                                               \
    QNN_GRAPH_CONFIG_OPTION_UNDEFINED, /*option*/ \
    {                                             \
      NULL /*customConfig*/                       \
    }                                             \
  }

/**
 * @brief This struct provides status associated with Qnn_NotifyFn_t() function.
 */
typedef struct {
  Qnn_ErrorHandle_t error;
} Qnn_NotifyStatus_t;

/// Qnn_NotifyStatus_t initializer macro
#define QNN_NOTIFY_STATUS_INIT \
  { 0 /*error*/ }

/**
 * @brief A client-defined callback function.
 *
 * @param[in] notifyParam  Client supplied data object which may be used to identify
 *                         which function this callback applies to.
 *
 * @param[in] notifyStatus Execution status associate with callback.
 *
 * @return None
 *
 */
typedef void (*Qnn_NotifyFn_t)(void* notifyParam, Qnn_NotifyStatus_t notifyStatus);

//=============================================================================
// Public Functions
//=============================================================================

/**
 * @brief A function to create an empty graph.
 *        The function returns an opaque object to be used on all graph APIs
 *        (addNode, finalize, execute, ...)
 *
 * @param[in]  contextHandle A handle to the context in which the graph would be created.
 *
 * @param[in]  graphName A string which identifies the graph.
 *                       Graph name allows retrieval of the graph after creating the
 *                       context from cached binary.
 *                       _graphName_ must be unique within the _context_.
 *
 * @param[in]  config    Pointer to a NULL terminated array of config option pointers.
 *                       NULL is allowed and indicates no config options are provided.
 *                       All config options have default value, in case not provided.
 *                       If same config option type is provided multiple times,
 *                       the last option value will be used.
 *
 * @param[out] graphHandle The created graph handle.
 *
 * @return Error code:
 *         - QNN_SUCCESS: the graph was successfully created
 *         - QNN_COMMON_ERROR_BACKEND_NOT_INITIALIZED: QnnBackend not initialized
 *         - QNN_GRAPH_ERROR_INVALID_ARGUMENT: _graph_ is NULL or at least one config option was
 *           invalid
 *         - QNN_GRAPH_ERROR_INVALID_NAME: _graphName_ is NULL or not unique within the
 *           _context_
 *         - QNN_GRAPH_ERROR_INVALID_HANDLE: _context_ is not a valid handle
 *         - QNN_GRAPH_ERROR_MEM_ALLOC: create failed due to memory/resource allocation
 *         - QNN_GRAPH_ERROR_UNSUPPORTED_FEATURE: some API feature is not supported yet, e.g.
 *           config option
 *         - QNN_GRAPH_ERROR_CREATE_FAILED: create failed due to some other reason
 *         - QNN_COMMON_ERROR_OPERATION_NOT_PERMITTED: create failed when context is
 *           re-created from binary using QnnContext_createFromBinary().
 *
 * @note DEPRECATED: Use corresponding API through QnnInterface_t.
 */
QNN_API
Qnn_ErrorHandle_t QnnGraph_create(Qnn_ContextHandle_t contextHandle,
                                  const char* graphName,
                                  const QnnGraph_Config_t** config,
                                  Qnn_GraphHandle_t* graphHandle);

/**
 * @brief A function to add a node to the graph
 *
 * @param[in]  graphHandle The graph handle to add the node to.
 *
 * @note The following conditions should be honored by tensors specified as
 *       part of opConfig:
 *       1. No tensor in the list opConfig.outputTensors can be of type
 *          QNN_TENSOR_TYPE_APP_WRITE or QNN_TENSOR_TYPE_STATIC.
 *       2. All parameters in the opConfig that happen to be tensors must be
 *          of the type QNN_TENSOR_TYPE_STATIC.
 *       3. Tensors express connectivity between nodes. However, it is permissible
 *          for tensors to remain 'unconsumed' in a graph, i.e.,
 *          not act as inputs to any other node in the graph.
 *
 * @note QnnGraph does not validate opConfig used in creating node beyond checks for basic sanity.
 *       A thorough validation of opConfig for this node defined in a certain op package
 *       has to be done via QnnBackend_validateOpConfig().
 *
 * @note Nodes must be added in dependency order. i.e. all QNN_TENSOR_TYPE_NATIVE inputs to the node
 *       must be outputs of a previously added node.
 *
 * @param[in]  opConfig A struct containing the configuration of the operation
 *                      which should be added as a node in the graph.
 *                      The tensor objects in this structure for inputs and outputs to the node
 *                      must be created with APIs in QnnTensor.h which register them with
 *                      a backend. Unrecognized tensors in the opConfig result in failure.
 *                      Since the tensor ID allows a backend to register a tensor, it is sufficient
 *                      to only specify a valid tensor ID in the Qnn_Tensor_t structures
 *                      associated with the opConfig. All other fields including any static data
 *                      are ignored by the backend when parsing these tensors.
 *
 * @return Error code
 *         - QNN_SUCCESS: the node is successfully added to the graph
 *         - QNN_COMMON_ERROR_BACKEND_NOT_INITIALIZED: QnnBackend not initialized
 *         - QNN_GRAPH_ERROR_INVALID_OP_CONFIG: misconfigured operation - invalid op config
 *           Thrown when a BE cannot match package name and/or op name with any
 *           registered op packages, or when
 *           tensor metadata for tensors in opConfig differs from that used in
 *           registering them with a graph using QnnTensor_createGraphTensor().
 *         - QNN_GRAPH_ERROR_INVALID_TENSOR: when tensor objects within opConfig are invalid
 *         - QNN_GRAPH_ERROR_INVALID_HANDLE: _graph_ is not a valid handle
 *         - QNN_GRAPH_ERROR_GRAPH_FINALIZED: add nodes on a finalized graph
 *         - QNN_GRAPH_ERROR_UNCONNECTED_NODE: node added before its dependent node(s)
 *
 * @note DEPRECATED: Use corresponding API through QnnInterface_t.
 */
QNN_API
Qnn_ErrorHandle_t QnnGraph_addNode(Qnn_GraphHandle_t graphHandle, Qnn_OpConfig_t opConfig);

/**
 * @brief A function to finalize the graph. The runtime will process the
 *        graph, validate that all operations are created successfully and
 *        that connectivity is correct.
 *
 * @param[in] graphHandle  handle to the graph to be finalized.
 *
 * @param[in] profileHandle The profile handle on which metrics is populated and can be queried.
 *                          Use NULL handle to disable profile collection. A handle being re-used
 *                          would reset and is populated with values from the current call.
 *
 * @param[in] signalHandle Signal object to control the execution of the finalize process.
 *                         NULL may be passed to indicate that no execution control is requested,
 *                         and the finalize operation should continue to completion uninterrupted.
 *
 * @note      Graphs that contain zero nodes will fail to finalize.
 *
 * @return Error code:
 *         - QNN_SUCCESS: the graph is finalized successfully
 *         - QNN_COMMON_ERROR_BACKEND_NOT_INITIALIZED: QnnBackend not initialized
 *         - QNN_GRAPH_ERROR_INVALID_HANDLE: _graph_ is not a valid handle
 *         - QNN_GRAPH_ERROR_INVALID_ARGUMENT: invalid param passed in
 *         - QNN_GRAPH_ERROR_CREATE_FAILED: op/kernel creation failed
 *         - QNN_GRAPH_ERROR_OPTIMIZATION_FAILED: graph optimization failed
 *         - QNN_GRAPH_ERROR_UNSUPPORTED_FEATURE: some API feature is not supported yet,
 *           e.g. signal or profile
 *         - QNN_GRAPH_ERROR_SET_PROFILE: set profile failed
 *         - QNN_GRAPH_ERROR_SIGNAL_OBSERVED: the supplied control signal is
 *           already observed by another call.
 *         - QNN_GRAPH_ERROR_ABORTED: the call is aborted before termination
 *         - QNN_GRAPH_ERROR_FINALIZE_FAILED: finalize failed for some other reason
 *
 * @note DEPRECATED: Use corresponding API through QnnInterface_t.
 */
QNN_API
Qnn_ErrorHandle_t QnnGraph_finalize(Qnn_GraphHandle_t graphHandle,
                                    Qnn_ProfileHandle_t profileHandle,
                                    Qnn_SignalHandle_t signalHandle);

/**
 * @brief A function to retrieve a graph based on name.
 *        This function is typically used when a context was created from cached
 *        binary. The re-created context has graph(s) which are also re-created.
 *        The function returns the graph handle to be used for
 *        all graph APIs (addNode, finalize, execute, ...)
 *
 * @param[in]  contextHandle An opaque ID to the context.
 *
 * @param[in]  graphName A string which identifies the graph.
 *
 * @param[out] graphHandle A pointer to the graph handle that is being retrieved.
 *
 * @return Error code:
 *         - QNN_SUCCESS: the graph was successfully retrieved
 *         - QNN_COMMON_ERROR_BACKEND_NOT_INITIALIZED: QnnBackend not initialized
 *         - QNN_GRAPH_ERROR_INVALID_NAME: _graphName_ or _graph_ is NULL
 *         - QNN_GRAPH_ERROR_INVALID_HANDLE: _context_ is not a valid handle
 *         - QNN_GRAPH_ERROR_GRAPH_DOES_NOT_EXIST: graph not found/created
 *
 * @note DEPRECATED: Use corresponding API through QnnInterface_t.
 */
QNN_API
Qnn_ErrorHandle_t QnnGraph_retrieve(Qnn_ContextHandle_t contextHandle,
                                    const char* graphName,
                                    Qnn_GraphHandle_t* graphHandle);

/**
 * @brief Synchronously execute a finalized graph.
 *
 * @param[in]  graphHandle Handle of finalized graph to execute.
 *
 * @param[in]  inputs     Array of tensors with which to populate graph inputs.
 *
 * @param[in]  numInputs  Number of input tensors.
 *
 * @param[out] outputs    Array of output tensors which the graph will populate with output values.
 *
 * @param[in]  numOutputs Number of output tensors.
 *
 * @param[in]  profileHandle The profile handle on which metrics is populated and can be queried.
 *                           Use NULL handle to disable profile collection. A handle being reused
 *                           would reset and is populated with values from the current call.
 *
 * @param[in]  signalHandle Signal object which may be used to control the execution of
 *                          this call. NULL indicates execution should proceed as normal.
 *
 * @note                  Tensors in _inputs_ and _outputs_ must carry the same ID with which
 *                        they were created. Values for all other attributes in Qnn_Tensor_t are
 *                        assumed from the point at which they were registered with a backend
 *                        at the time of tensor creation, with the following exceptions:
 *                        - Tensor data provided by client in structs such as _clientBuf_ can be
 *                        changed between invocations to execute().
 *                        - If valid (non-null) _maxDimensions_ are supplied in the Qnn_Tensor_t
 *                        arguments, all but the batch dimension are expected to match those at
 *                        the time of tensor creation. The batch dimension in _maxDimensions_
 *                        is allowed to be a multiple of the original batch size that the tensor
 *                         was created with.
 *                        _currentDimensions_ in the Qnn_Tensor_t arguments may be ignored,
 *                        depending on op definition and backend support.
 *                        - Other fields like _dataType_ can also be permitted to change between
 *                        invocations to execute() for certain ops that perform data type
 *                        conversions.
 *                        - Some backends may be able to execute a graph with no _inputs_ provided
 *                        the graph has no application-writable tensors.
 *
 *                        - Graph I/O Tensors marked optional (i.e. omitted or marked as
 *                        type=QNN_TENSOR_TYPE_NULL during QnnGraph_addNode()) cannot be supplied to
 *                        QnnGraph_execute(). Clients mark tensors to be of type
 *                        QNN_TENSOR_TYPE_NULL to indicate that they must be ignored when
 *                        constructing a node that lists them as optional.
 *
 * @return Error code:
 *         - QNN_SUCCESS: the graph was successfully executed
 *         - QNN_COMMON_ERROR_BACKEND_NOT_INITIALIZED: QnnBackend not initialized
 *         - QNN_GRAPH_ERROR_INVALID_HANDLE: _graph_ is not a valid handle
 *         - QNN_GRAPH_ERROR_GRAPH_NOT_FINALIZED: graph was not finalized
 *         - QNN_GRAPH_ERROR_INVALID_ARGUMENT:
 *            - _inputs_ or _outputs_ is NULL or ill-formed OR
 *            - _inputs_ is NOT NULL and _numInputs_ is 0 OR
 *            - _outputs_ is NOT NULL and _numOutputs_ is 0 OR
 *            - _profile_ handle is invalid.
 *         - QNN_GRAPH_ERROR_INVALID_TENSOR: one or more tensors in _inputs_ or _outputs_
 *           is invalid or not recognized by graph
 *         - QNN_GRAPH_ERROR_UNSUPPORTED_FEATURE: some API feature is not supported yet, e.g.
 *           signal or profile
 *         - QNN_GRAPH_ERROR_SET_PROFILE: set profile failed
 *         - QNN_GRAPH_ERROR_SIGNAL_OBSERVED: the supplied control signal is already observed by
 *           another call.
 *         - QNN_GRAPH_ERROR_ABORTED: the call is aborted before termination
 *         - QNN_SIGNAL_ERROR_TIMEOUT: the signal object indicates a timeout on the current
 *           invocation
 *
 * @note DEPRECATED: Use corresponding API through QnnInterface_t.
 */
QNN_API
Qnn_ErrorHandle_t QnnGraph_execute(Qnn_GraphHandle_t graphHandle,
                                   const Qnn_Tensor_t* inputs,
                                   uint32_t numInputs,
                                   Qnn_Tensor_t* outputs,
                                   uint32_t numOutputs,
                                   Qnn_ProfileHandle_t profileHandle,
                                   Qnn_SignalHandle_t signalHandle);

/**
 * @brief Asynchronously execute a finalized graph.
 *        This functions enqueues execution request as per configured
 *        QnnGraph_AsyncExecutionOrder_t.
 *
 * @param[in]  graphHandle Handle of finalized graph to execute.
 *
 * @param[in]  inputs      Array of input tensors with which to populate graph inputs.
 *
 * @param[in]  numInputs   Number of input tensors.
 *
 * @param[out] outputs     Array of tensors which the graph will populate with output values.
 *
 * @param[in]  numOutputs  Number of output tensors.
 *
 * @param[in]  profileHandle The profile handle on which metrics is populated and can be queried.
 *                           Use NULL handle to disable profile collection. A handle being reused
 *                           would reset and is populated with values from the enqueued execute
 *                           call. Profile handle management/reuse across asynchronous calls is
 *                           client's responsibility. Behavior is undefined if same profile handle
 *                           is used by two enqueued execute instances at the same time.
 *
 * @param[in]  signalHandle Signal object which may be used to control the execution of
 *                          this call. NULL indicates execution should proceed as normal.
 *                          All pending executions in the queue are affected by Signal control.
 *                          Instance executing when Signal control is issued may not be affected.
 *
 * @param[in]  notifyFn    Pointer to notification function, called when execution is finished.
 *                         NULL indicates no notification is requested. _notifyFn_ will be called
 *                         in context of backend owned thread, with priority equal or lower than
 *                         client's calling thread.
 *
 * @param[in]  notifyParam Client-supplied data object which will be passed back via _notifyFn_ and
 *                         can be used to identify asynchronous execution instance. Can be NULL.
 *
 * @note                  Tensors in _inputs_ and _outputs_ must carry the same ID with which
 *                        they were created. Values for all other attributes in Qnn_Tensor_t are
 *                        assumed from the point at which they were registered with a backend
 *                        at the time of tensor creation, with the following exceptions:
 *                        - Tensor data provided by client in structs such as _clientBuf_ can be
 *                        changed between invocations to execute().
 *                        - If valid (non-null) _maxDimensions_ are supplied in the Qnn_Tensor_t
 *                        arguments, all but the batch dimension are expected to match those at
 *                        the time of tensor creation. The batch dimension in _maxDimensions_
 *                        is allowed to be a multiple of the original batch size that the tensor
 *                         was created with.
 *                        _currentDimensions_ in the Qnn_Tensor_t arguments may be ignored,
 *                        depending on op definition and backend support.
 *                        - Other fields like _dataType_ can also be permitted to change between
 *                        invocations to execute() for certain ops that perform data type
 *                        conversions.
 *                        - Some backends may be able to execute a graph with no _inputs_ provided
 *                        the graph has no application-writable tensors.
 *
 *                        - Graph I/O Tensors marked optional (type=QNN_TENSOR_TYPE_NULL) cannot
 *                        be supplied to QnnGraph_executeAsync(). Clients mark tensors to be of type
 *                        QNN_TENSOR_TYPE_NULL to indicate that they must be ignored when
 *                        constructing a node that lists them as optional.
 *
 * @return Error code:
 *         - QNN_SUCCESS: the graph was successfully executed
 *         - QNN_COMMON_ERROR_BACKEND_NOT_INITIALIZED: QnnBackend not initialized
 *         - QNN_GRAPH_ERROR_INVALID_HANDLE: _graph_ is not a valid handle
 *         - QNN_GRAPH_ERROR_GRAPH_NOT_FINALIZED: graph was not finalized
 *         - QNN_GRAPH_ERROR_INVALID_ARGUMENT:
 *            - _inputs_ or _outputs_ is NULL or ill-formed OR
 *            - _inputs_ is NOT NULL and _numInputs_ is 0 OR
 *            - _outputs_ is NOT NULL and _numOutputs_ is 0 OR
 *            - _profile_ handle is invalid.
 *         - QNN_GRAPH_ERROR_INVALID_TENSOR: one or more tensors in _inputs_ or _outputs_
 *           is invalid or not recognized by graph
 *         - QNN_GRAPH_ERROR_EXECUTION_ASYNC_FIFO_FULL: could not enqueue execution because fifo
 *           is full
 *         - QNN_GRAPH_ERROR_UNSUPPORTED_FEATURE: some API feature is not supported yet, e.g.
 *           signal or profile
 *         - QNN_GRAPH_ERROR_SIGNAL_OBSERVED: the supplied control signal is already observed by
 *           another call.
 *         - QNN_GRAPH_ERROR_ABORTED: the call is aborted before termination
 *         - QNN_SIGNAL_ERROR_TIMEOUT: the signal object indicates a timeout on the current
 *           invocation
 *
 * @note DEPRECATED: Use corresponding API through QnnInterface_t.
 */
QNN_API
Qnn_ErrorHandle_t QnnGraph_executeAsync(Qnn_GraphHandle_t graphHandle,
                                        const Qnn_Tensor_t* inputs,
                                        uint32_t numInputs,
                                        Qnn_Tensor_t* outputs,
                                        uint32_t numOutputs,
                                        Qnn_ProfileHandle_t profileHandle,
                                        Qnn_SignalHandle_t signalHandle,
                                        Qnn_NotifyFn_t notifyFn,
                                        void* notifyParam);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // QNN_GRAPH_H
