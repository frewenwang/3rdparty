//==============================================================================
//
//  Copyright (c) 2020-2022 Qualcomm Technologies, Inc.
//  All Rights Reserved.
//  Confidential and Proprietary - Qualcomm Technologies, Inc.
//
//==============================================================================
#pragma once

#include <memory>
#include <queue>

#include "IOTensor.hpp"
#include "SampleApp.hpp"

namespace qnn {
namespace tools {
namespace sample_app {

enum class StatusCode { SUCCESS, FAILURE, FAILURE_INPUT_LIST_EXHAUSTED };
enum class ProfilingLevel { OFF, BASIC, DETAILED, INVALID };

class QnnSampleApp {
 public:
  QnnSampleApp(QnnFunctionPointers qnnFunctionPointers,
               std::string inputListPaths,
               std::string opPackagePaths,
               void *backendHandle,
               std::string outputPath                  = s_defaultOutputPath,
               bool debug                              = false,
               iotensor::OutputDataType outputDataType = iotensor::OutputDataType::FLOAT_ONLY,
               iotensor::InputDataType inputDataType   = iotensor::InputDataType::FLOAT,
               ProfilingLevel profilingLevel           = ProfilingLevel::OFF,
               bool dumpOutputs                        = false,
               std::string cachedBinaryPath            = "",
               std::string saveBinaryName              = "");

  // @brief Print a message to STDERR then return a nonzero
  //  exit status.
  int32_t reportError(const std::string &err);

  StatusCode initialize();

  StatusCode initializeBackend();

  StatusCode createContext();

  StatusCode composeGraphs();

  StatusCode finalizeGraphs();

  StatusCode executeGraphs();

  StatusCode registerOpPackages();

  StatusCode createFromBinary();

  StatusCode saveBinary();

  StatusCode freeContext();

  StatusCode terminateBackend();

  StatusCode freeGraphs();

  Qnn_ContextHandle_t getContext();

  StatusCode initializeProfiling();

  std::string getBackendBuildId();

  virtual ~QnnSampleApp();

 private:
  StatusCode extractBackendProfilingInfo(Qnn_ProfileHandle_t profileHandle);

  StatusCode extractProfilingSubEvents(QnnProfile_EventId_t profileEventId);

  StatusCode extractProfilingEvent(QnnProfile_EventId_t profileEventId);

  static const std::string s_defaultOutputPath;

  QnnFunctionPointers m_qnnFunctionPointers;
  std::vector<std::string> m_inputListPaths;
  std::vector<std::vector<std::queue<std::string>>> m_inputFileLists;
  std::vector<std::string> m_opPackagePaths;
  std::string m_outputPath;
  std::string m_saveBinaryName;
  std::string m_cachedBinaryPath;
  QnnBackend_Config_t **m_backendConfig;
  Qnn_ContextHandle_t m_context;
  QnnContext_Config_t **m_contextConfig;
  bool m_debug;
  iotensor::OutputDataType m_outputDataType;
  iotensor::InputDataType m_inputDataType;
  ProfilingLevel m_profilingLevel;
  bool m_dumpOutputs;
  qnn_wrapper_api::GraphInfo_t **m_graphsInfo;
  uint32_t m_graphsCount;
  void *m_backendHandle;
  iotensor::IOTensor m_ioTensor;
  bool m_isBackendInitialized;
  bool m_isContextCreated;
  Qnn_ProfileHandle_t m_profileBackendHandle;
  qnn_wrapper_api::GraphConfigInfo_t **m_graphConfigsInfo;
  uint32_t m_graphConfigsInfoCount;
};
}  // namespace sample_app
}  // namespace tools
}  // namespace qnn
