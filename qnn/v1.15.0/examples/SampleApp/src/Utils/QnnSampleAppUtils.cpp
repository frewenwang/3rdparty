//==============================================================================
//
//  Copyright (c) 2019-2022 Qualcomm Technologies, Inc.
//  All Rights Reserved.
//  Confidential and Proprietary - Qualcomm Technologies, Inc.
//
//==============================================================================

#include <algorithm>
#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <tuple>

#include "Logger.hpp"
#include "PAL/Directory.hpp"
#include "PAL/FileOp.hpp"
#include "PAL/Path.hpp"
#include "PAL/StringOp.hpp"
#include "QnnSampleAppUtils.hpp"

using namespace qnn;
using namespace qnn::tools;

void sample_app::split(std::vector<std::string> &splitString,
                       const std::string &tokenizedString,
                       const char separator) {
  splitString.clear();
  std::istringstream tokenizedStringStream(tokenizedString);
  while (!tokenizedStringStream.eof()) {
    std::string value;
    getline(tokenizedStringStream, value, separator);
    if (!value.empty()) {
      splitString.push_back(value);
    }
  }
}

void sample_app::parseInputFilePaths(std::vector<std::string> &inputFilePaths,
                                     std::vector<std::string> &paths,
                                     std::string separator) {
  for (auto &inputInfo : inputFilePaths) {
    auto position = inputInfo.find(separator);
    if (position != std::string::npos) {
      auto path = inputInfo.substr(position + separator.size());
      paths.push_back(path);
    } else {
      paths.push_back(inputInfo);
    }
  }
}

sample_app::ReadInputListsRetType_t sample_app::readInputLists(
    std::vector<std::string> inputFileListPaths) {
  std::vector<std::vector<std::queue<std::string>>> filePathsLists;
  for (auto const &path : inputFileListPaths) {
    bool readSuccess;
    std::vector<std::queue<std::string>> filePathList;
    std::tie(filePathList, readSuccess) = readInputList(path);
    if (!readSuccess) {
      filePathsLists.clear();
      return std::make_tuple(filePathsLists, false);
    }
    filePathsLists.push_back(filePathList);
  }
  return std::make_tuple(filePathsLists, true);
}

sample_app::ReadInputListRetType_t sample_app::readInputList(const std::string inputFileListPath) {
  std::queue<std::string> lines;
  std::ifstream fileListStream(inputFileListPath);
  if (!fileListStream) {
    QNN_ERROR("Failed to open input file: %s", inputFileListPath.c_str());
    std::vector<std::queue<std::string>> result;
    return std::make_tuple(result, false);
  }
  std::string fileLine;
  while (std::getline(fileListStream, fileLine)) {
    if (fileLine.empty()) continue;
    lines.push(fileLine);
  }
  if (!lines.empty() && lines.front().compare(0, 1, "#") == 0) {
    lines.pop();
  }
  std::string separator = ":=";
  std::vector<std::queue<std::string>> filePathsList;
  while (!lines.empty()) {
    std::vector<std::string> paths{};
    std::vector<std::string> inputFilePaths;
    split(inputFilePaths, lines.front(), ' ');
    parseInputFilePaths(inputFilePaths, paths, separator);
    filePathsList.reserve(paths.size());
    for (size_t idx = 0; idx < paths.size(); idx++) {
      if (idx >= filePathsList.size()) {
        filePathsList.push_back(std::queue<std::string>());
      }
      filePathsList[idx].push(paths[idx]);
    }
    lines.pop();
  }
  return std::make_tuple(filePathsList, true);
}

sample_app::ProfilingLevel sample_app::parseProfilingLevel(std::string profilingLevelString) {
  std::transform(profilingLevelString.begin(),
                 profilingLevelString.end(),
                 profilingLevelString.begin(),
                 ::tolower);
  ProfilingLevel parsedProfilingLevel = ProfilingLevel::INVALID;
  if (profilingLevelString == "off") {
    parsedProfilingLevel = ProfilingLevel::OFF;
  } else if (profilingLevelString == "basic") {
    parsedProfilingLevel = ProfilingLevel::BASIC;
  } else if (profilingLevelString == "detailed") {
    parsedProfilingLevel = ProfilingLevel::DETAILED;
  }
  return parsedProfilingLevel;
}

bool sample_app::populateTensorNamesFromMetadata(
    std::unordered_map<std::string, std::unordered_map<uint32_t, std::string>>
        &graphTensorIdToNamesMap,
    qnn_wrapper_api::GraphInfo_t **&graphsInfo,
    const uint32_t graphsCount) {
  for (uint32_t gIdx = 0; gIdx < graphsCount; gIdx++) {
    std::string graphName = std::string((*graphsInfo)[gIdx].graphName);
    if (graphTensorIdToNamesMap.find(graphName) == graphTensorIdToNamesMap.end()) {
      QNN_ERROR("Graph [%s] not found in metadata.", graphName.c_str());
      return false;
    }
    for (uint32_t tIdx = 0; tIdx < (*graphsInfo)[gIdx].numInputTensors; tIdx++) {
      auto tensorId = (*graphsInfo)[gIdx].inputTensors[tIdx].tensor.id;
      if (graphTensorIdToNamesMap[graphName].find(tensorId) ==
          graphTensorIdToNamesMap[graphName].end()) {
        QNN_ERROR("Input tensor name for [%u] in graph [%s] not found in metadata.",
                  tensorId,
                  graphName.c_str());
        return false;
      }
      (*graphsInfo)[gIdx].inputTensors[tIdx].name =
          pal::StringOp::strndup(graphTensorIdToNamesMap[graphName][tensorId].c_str(),
                                 strlen(graphTensorIdToNamesMap[graphName][tensorId].c_str()));
    }
    for (uint32_t tIdx = 0; tIdx < (*graphsInfo)[gIdx].numOutputTensors; tIdx++) {
      auto tensorId = (*graphsInfo)[gIdx].outputTensors[tIdx].tensor.id;
      if (graphTensorIdToNamesMap[graphName].find(tensorId) ==
          graphTensorIdToNamesMap[graphName].end()) {
        QNN_ERROR("Output tensor name for [%u] in graph [%s] not found in metadata.",
                  tensorId,
                  graphName.c_str());
        return false;
      }
      (*graphsInfo)[gIdx].outputTensors[tIdx].name =
          pal::StringOp::strndup(graphTensorIdToNamesMap[graphName][tensorId].c_str(),
                                 strlen(graphTensorIdToNamesMap[graphName][tensorId].c_str()));
    }
  }
  return true;
}

bool sample_app::copyTensorsInfo(const Qnn_Tensor_t *tensorsInfoSrc,
                                 qnn_wrapper_api::Qnn_TensorWrapper_t *&tensorWrappers,
                                 uint32_t tensorsCount) {
  QNN_FUNCTION_ENTRY_LOG;
  auto returnStatus = true;
  tensorWrappers    = (qnn_wrapper_api::Qnn_TensorWrapper_t *)calloc(
      tensorsCount, sizeof(qnn_wrapper_api::Qnn_TensorWrapper_t));
  if (nullptr == tensorWrappers) {
    QNN_ERROR("Failed to allocate memory for tensorWrappers.");
    return false;
  }
  if (returnStatus) {
    for (size_t tIdx = 0; tIdx < tensorsCount; tIdx++) {
      QNN_DEBUG("Extracting tensorInfo for tensor Idx: %d", tIdx);
      tensorWrappers[tIdx].name              = nullptr;
      tensorWrappers[tIdx].tensor.id         = tensorsInfoSrc[tIdx].id;
      tensorWrappers[tIdx].tensor.type       = tensorsInfoSrc[tIdx].type;
      tensorWrappers[tIdx].tensor.dataFormat = tensorsInfoSrc[tIdx].dataFormat;
      tensorWrappers[tIdx].tensor.dataType   = tensorsInfoSrc[tIdx].dataType;
      tensorWrappers[tIdx].tensor.quantizeParams.quantizationEncoding =
          QNN_QUANTIZATION_ENCODING_UNDEFINED;
      if (tensorsInfoSrc[tIdx].quantizeParams.quantizationEncoding ==
          QNN_QUANTIZATION_ENCODING_SCALE_OFFSET) {
        tensorWrappers[tIdx].tensor.quantizeParams.quantizationEncoding =
            tensorsInfoSrc[tIdx].quantizeParams.quantizationEncoding;
        tensorWrappers[tIdx].tensor.quantizeParams.scaleOffsetEncoding =
            tensorsInfoSrc[tIdx].quantizeParams.scaleOffsetEncoding;
      } else if (tensorsInfoSrc[tIdx].quantizeParams.quantizationEncoding ==
                 QNN_QUANTIZATION_ENCODING_AXIS_SCALE_OFFSET) {
        tensorWrappers[tIdx].tensor.quantizeParams.quantizationEncoding =
            tensorsInfoSrc[tIdx].quantizeParams.quantizationEncoding;
        tensorWrappers[tIdx].tensor.quantizeParams.axisScaleOffsetEncoding.axis =
            tensorsInfoSrc[tIdx].quantizeParams.axisScaleOffsetEncoding.axis;
        tensorWrappers[tIdx].tensor.quantizeParams.axisScaleOffsetEncoding.numScaleOffsets =
            tensorsInfoSrc[tIdx].quantizeParams.axisScaleOffsetEncoding.numScaleOffsets;
        if (tensorsInfoSrc[tIdx].quantizeParams.axisScaleOffsetEncoding.numScaleOffsets > 0) {
          tensorWrappers[tIdx].tensor.quantizeParams.axisScaleOffsetEncoding.scaleOffset =
              (Qnn_ScaleOffset_t *)malloc(
                  tensorsInfoSrc[tIdx].quantizeParams.axisScaleOffsetEncoding.numScaleOffsets *
                  sizeof(Qnn_ScaleOffset_t));
          if (tensorWrappers[tIdx].tensor.quantizeParams.axisScaleOffsetEncoding.scaleOffset) {
            for (size_t idx = 0;
                 idx < tensorsInfoSrc[tIdx].quantizeParams.axisScaleOffsetEncoding.numScaleOffsets;
                 idx++) {
              tensorWrappers[tIdx]
                  .tensor.quantizeParams.axisScaleOffsetEncoding.scaleOffset[idx]
                  .scale = tensorsInfoSrc[tIdx]
                               .quantizeParams.axisScaleOffsetEncoding.scaleOffset[idx]
                               .scale;
              tensorWrappers[tIdx]
                  .tensor.quantizeParams.axisScaleOffsetEncoding.scaleOffset[idx]
                  .offset = tensorsInfoSrc[tIdx]
                                .quantizeParams.axisScaleOffsetEncoding.scaleOffset[idx]
                                .offset;
            }
          }
        }
      }
      tensorWrappers[tIdx].tensor.rank              = tensorsInfoSrc[tIdx].rank;
      tensorWrappers[tIdx].tensor.maxDimensions     = nullptr;
      tensorWrappers[tIdx].tensor.currentDimensions = nullptr;
      if (tensorWrappers[tIdx].tensor.rank > 0) {
        tensorWrappers[tIdx].tensor.maxDimensions =
            (uint32_t *)malloc(tensorsInfoSrc->rank * sizeof(uint32_t));
        if (tensorWrappers[tIdx].tensor.maxDimensions) {
          pal::StringOp::memscpy(tensorWrappers[tIdx].tensor.maxDimensions,
                                 tensorWrappers[tIdx].tensor.rank * sizeof(uint32_t),
                                 tensorsInfoSrc[tIdx].maxDimensions,
                                 tensorsInfoSrc[tIdx].rank * sizeof(uint32_t));
        }
        tensorWrappers[tIdx].tensor.currentDimensions =
            (uint32_t *)malloc(tensorsInfoSrc->rank * sizeof(uint32_t));
        if (tensorWrappers[tIdx].tensor.currentDimensions) {
          pal::StringOp::memscpy(tensorWrappers[tIdx].tensor.currentDimensions,
                                 tensorWrappers[tIdx].tensor.rank * sizeof(uint32_t),
                                 tensorsInfoSrc[tIdx].currentDimensions,
                                 tensorsInfoSrc[tIdx].rank * sizeof(uint32_t));
        }
      }
    }
  }
  QNN_FUNCTION_EXIT_LOG;
  return returnStatus;
}

bool sample_app::copyGraphsInfoV1(const QnnSystemContext_GraphInfoV1_t *graphInfoSrc,
                                  qnn_wrapper_api::GraphInfo_t *graphInfoDst) {
  graphInfoDst->graphName = nullptr;
  if (graphInfoSrc->graphName) {
    graphInfoDst->graphName =
        pal::StringOp::strndup(graphInfoSrc->graphName, strlen(graphInfoSrc->graphName));
  }
  graphInfoDst->inputTensors    = nullptr;
  graphInfoDst->numInputTensors = 0;
  if (graphInfoSrc->graphInputs) {
    if (!copyTensorsInfo(
            graphInfoSrc->graphInputs, graphInfoDst->inputTensors, graphInfoSrc->numGraphInputs)) {
      return false;
    }
    graphInfoDst->numInputTensors = graphInfoSrc->numGraphInputs;
  }
  graphInfoDst->outputTensors    = nullptr;
  graphInfoDst->numOutputTensors = 0;
  if (graphInfoSrc->graphOutputs) {
    if (!copyTensorsInfo(graphInfoSrc->graphOutputs,
                         graphInfoDst->outputTensors,
                         graphInfoSrc->numGraphOutputs)) {
      return false;
    }
    graphInfoDst->numOutputTensors = graphInfoSrc->numGraphOutputs;
  }
  return true;
}

bool sample_app::copyGraphsInfo(const QnnSystemContext_GraphInfo_t *graphsInput,
                                const uint32_t numGraphs,
                                qnn_wrapper_api::GraphInfo_t **&graphsInfo) {
  QNN_FUNCTION_ENTRY_LOG;
  if (!graphsInput) {
    QNN_ERROR("Received nullptr for graphsInput.");
    return false;
  }
  auto returnStatus = true;
  graphsInfo =
      (qnn_wrapper_api::GraphInfo_t **)calloc(numGraphs, sizeof(qnn_wrapper_api::GraphInfo_t *));
  qnn_wrapper_api::GraphInfo_t *graphInfoArr =
      (qnn_wrapper_api::GraphInfo_t *)calloc(numGraphs, sizeof(qnn_wrapper_api::GraphInfo_t));
  if (nullptr == graphsInfo || nullptr == graphInfoArr) {
    QNN_ERROR("Failure to allocate memory for *graphInfo");
    returnStatus = false;
  }
  if (true == returnStatus) {
    for (size_t gIdx = 0; gIdx < numGraphs; gIdx++) {
      QNN_DEBUG("Extracting graphsInfo for graph Idx: %d", gIdx);
      if (graphsInput[gIdx].version == QNN_SYSTEM_CONTEXT_GRAPH_INFO_VERSION_1) {
        copyGraphsInfoV1(&graphsInput[gIdx].graphInfoV1, &graphInfoArr[gIdx]);
      }
      graphsInfo[gIdx] = graphInfoArr + gIdx;
    }
  }
  if (true != returnStatus) {
    QNN_ERROR("Received an ERROR during extractGraphsInfo. Freeing resources.");
    if (graphsInfo) {
      for (uint32_t gIdx = 0; gIdx < numGraphs; gIdx++) {
        if (graphsInfo[gIdx]) {
          if (nullptr != graphsInfo[gIdx]->graphName) {
            free(graphsInfo[gIdx]->graphName);
            graphsInfo[gIdx]->graphName = nullptr;
          }
          qnn_wrapper_api::freeQnnTensorWrappers(graphsInfo[gIdx]->inputTensors,
                                                 graphsInfo[gIdx]->numInputTensors);
          qnn_wrapper_api::freeQnnTensorWrappers(graphsInfo[gIdx]->outputTensors,
                                                 graphsInfo[gIdx]->numOutputTensors);
        }
      }
      free(*graphsInfo);
    }
    free(graphsInfo);
    graphsInfo = nullptr;
  }
  QNN_FUNCTION_EXIT_LOG;
  return true;
}

bool sample_app::copyMetadataToGraphsInfo(const QnnSystemContext_BinaryInfo_t *binaryInfo,
                                          qnn_wrapper_api::GraphInfo_t **&graphsInfo,
                                          uint32_t &graphsCount) {
  if (nullptr == binaryInfo) {
    QNN_ERROR("binaryInfo is nullptr.");
    return false;
  }
  graphsCount = 0;
  if (binaryInfo->version == QNN_SYSTEM_CONTEXT_BINARY_INFO_VERSION_1) {
    if (binaryInfo->contextBinaryInfoV1.graphs) {
      if (!copyGraphsInfo(binaryInfo->contextBinaryInfoV1.graphs,
                          binaryInfo->contextBinaryInfoV1.numGraphs,
                          graphsInfo)) {
        QNN_ERROR("Failed while copying graphs Info.");
        return false;
      }
      graphsCount = binaryInfo->contextBinaryInfoV1.numGraphs;
      return true;
    }
  }
  QNN_ERROR("Unrecognized system context binary info version.");
  return false;
}

QnnLog_Level_t sample_app::parseLogLevel(std::string logLevelString) {
  QNN_FUNCTION_ENTRY_LOG;
  std::transform(logLevelString.begin(), logLevelString.end(), logLevelString.begin(), ::tolower);
  QnnLog_Level_t parsedLogLevel = QNN_LOG_LEVEL_MAX;
  if (logLevelString == "error") {
    parsedLogLevel = QNN_LOG_LEVEL_ERROR;
  } else if (logLevelString == "warn") {
    parsedLogLevel = QNN_LOG_LEVEL_WARN;
  } else if (logLevelString == "info") {
    parsedLogLevel = QNN_LOG_LEVEL_INFO;
  } else if (logLevelString == "verbose") {
    parsedLogLevel = QNN_LOG_LEVEL_VERBOSE;
  } else if (logLevelString == "debug") {
    parsedLogLevel = QNN_LOG_LEVEL_DEBUG;
  }
  QNN_FUNCTION_EXIT_LOG;
  return parsedLogLevel;
}
