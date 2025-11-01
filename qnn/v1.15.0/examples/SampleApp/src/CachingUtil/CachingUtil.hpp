//==============================================================================
//
//  Copyright (c) 2021-2022 Qualcomm Technologies, Inc.
//  All Rights Reserved.
//  Confidential and Proprietary - Qualcomm Technologies, Inc.
//
//==============================================================================

#include <fstream>
#include <unordered_map>

#include "DataUtil.hpp"
#include "Logger.hpp"
#include "PAL/StringOp.hpp"
#include "QnnWrapperUtils.hpp"
#include "SampleAppContextCaching_generated.h"

//
// This is a utility for caching QNN contexts in an application.
// QNN backends have the ability to serialize a context for saving
// and load a saved context for execution.
//
// With the QnnSystem APIs, it is possible retrieve information needed to
// context/graph(s) in the context. It is self-contained for clients who do not
// need or use Tensor names. For clients who need to store input and output tensor
// names, qnn-sample-app provides a utility to save this additional metadata
// along with cached context into a single file. This utility can be used to
// serialize metadata and binary context as well as deserialize the same for execution.
//
// This utility makes use of Flatbuffers for this purpose. The structure
// of the data stored is captured in the schema SampleAppContextCaching.fbs.
// A rough description of the root type in the schema:
//    ContextCaching {
//        Metadata related to graphs;
//        Graphs count;
//        Binary blob size;
//        Binary context blob;
//    }
//

namespace qnn {
namespace tools {
namespace sample_app {
namespace caching {
enum class StatusCode { SUCCESS, FAILURE };

// Function to help serialize tensorInfos. Creates all necessary
// flatbuffers data structures related to tensors which are used in
// buildGraphsInfo().
StatusCode buildTensorsInfo(flatbuffers::FlatBufferBuilder &fbb,
                            std::vector<flatbuffers::Offset<QnnTensorInfo>> &tensorsInfoVector,
                            qnn_wrapper_api::Qnn_TensorWrapper_t *tensorWrappers,
                            const uint32_t tensorCount) {
  QNN_FUNCTION_ENTRY_LOG;
  for (size_t tIdx = 0; tIdx < tensorCount; tIdx++) {
    QNN_DEBUG("Building tensorInfo for tensor Idx: %d", tIdx);
    auto fbTensorname = fbb.CreateString(tensorWrappers[tIdx].name);
    auto fbTensorInfo = CreateQnnTensorInfo(fbb, tensorWrappers[tIdx].tensor.id, fbTensorname);
    tensorsInfoVector.push_back(fbTensorInfo);
  }
  QNN_FUNCTION_EXIT_LOG;
  return StatusCode::SUCCESS;
}

// Function to help serialize graphsInfo. Creates all necessary
// flatbuffers data structures which will be serialized in serializeData().
StatusCode buildGraphsInfo(flatbuffers::FlatBufferBuilder &fbb,
                           std::vector<flatbuffers::Offset<QnnGraphInfo>> &graphsInfoVector,
                           qnn_wrapper_api::GraphInfo_t **graphsInfo,
                           const uint32_t &graphsCount) {
  QNN_FUNCTION_ENTRY_LOG;
  for (size_t gIdx = 0; gIdx < graphsCount; gIdx++) {
    QNN_DEBUG("Building graphsInfo for graph Idx: %d", gIdx);
    std::vector<flatbuffers::Offset<QnnTensorInfo>> inputTensorsInfoVector;
    size_t inputTensorCount = (*graphsInfo)[gIdx].numInputTensors;
    buildTensorsInfo(
        fbb, inputTensorsInfoVector, (*graphsInfo)[gIdx].inputTensors, inputTensorCount);
    std::vector<flatbuffers::Offset<QnnTensorInfo>> outputTensorsInfoVector;
    size_t outputTensorCount = (*graphsInfo)[gIdx].numOutputTensors;
    buildTensorsInfo(
        fbb, outputTensorsInfoVector, (*graphsInfo)[gIdx].outputTensors, outputTensorCount);
    auto graphName           = fbb.CreateString((*graphsInfo)[gIdx].graphName);
    auto fbInputTensorsInfo  = fbb.CreateVector(inputTensorsInfoVector);
    auto fbOutputTensorsInfo = fbb.CreateVector(outputTensorsInfoVector);
    auto graphInfo           = CreateQnnGraphInfo(fbb,
                                        graphName,
                                        inputTensorCount,
                                        fbInputTensorsInfo,
                                        outputTensorCount,
                                        fbOutputTensorsInfo);
    graphsInfoVector.push_back(graphInfo);
  }
  QNN_FUNCTION_EXIT_LOG;
  return StatusCode::SUCCESS;
}

// Function to serialize data for caching a context and metadata associated
// with graphs in the context. Serialization is based on flatbuffers and the
// schema SampleAppContextCaching.fbs. The structure of schema goes like this:
//    ContextCaching {
//        Metadata related to graphs;
//        Binary Context Blob;
//    }
// Metadata about graphs is received in the list qnn_wrapper_api::GraphInfo_t.
// Binary blob is a uint8_t*.
StatusCode serializeData(std::string fileDir,
                         std::string fileName,
                         qnn_wrapper_api::GraphInfo_t **graphsInfo,
                         uint32_t graphsCount,
                         uint8_t *binaryCache,
                         uint32_t binaryCacheSize) {
  QNN_FUNCTION_ENTRY_LOG;
  flatbuffers::FlatBufferBuilder fbb;
  std::vector<flatbuffers::Offset<QnnGraphInfo>> graphsInfoVector;
  buildGraphsInfo(fbb, graphsInfoVector, graphsInfo, graphsCount);
  auto fbGraphsInfo    = fbb.CreateVector(graphsInfoVector);
  uint8_t *cacheBuffer = nullptr;
  auto fbCacheBuffer   = fbb.CreateUninitializedVector<uint8_t>(binaryCacheSize, &cacheBuffer);
  pal::StringOp::memscpy(cacheBuffer, binaryCacheSize, binaryCache, binaryCacheSize);
  auto contextCache =
      CreateContextCache(fbb, graphsCount, fbGraphsInfo, binaryCacheSize, fbCacheBuffer);
  fbb.Finish(contextCache);
  uint8_t *fbBuf = fbb.GetBufferPointer();
  int fbBufsize  = fbb.GetSize();
  if (tools::datautil::StatusCode::SUCCESS !=
      tools::datautil::writeBinaryToFile(fileDir, fileName + ".bin", fbBuf, fbBufsize)) {
    QNN_ERROR("Error while writing binary to file.");
  }
  QNN_FUNCTION_EXIT_LOG;
  return StatusCode::SUCCESS;
}

// Function to help extract tensorsInfos from flatbuffers structures.
StatusCode extractTensorsInfo(
    const flatbuffers::Vector<flatbuffers::Offset<QnnTensorInfo>> *fbTensorInfosVector,
    std::string graphName,
    std::unordered_map<std::string, std::unordered_map<uint32_t, std::string>>
        &graphTensorIdToNamesMap,
    uint32_t tensorsCount) {
  QNN_FUNCTION_ENTRY_LOG;
  auto returnStatus = StatusCode::SUCCESS;
  if (StatusCode::SUCCESS == returnStatus) {
    for (size_t tIdx = 0; tIdx < tensorsCount; tIdx++) {
      QNN_DEBUG("Extracting tensorInfo for tensor Idx: %d", tIdx);
      if (graphTensorIdToNamesMap.find(graphName) == graphTensorIdToNamesMap.end()) {
        graphTensorIdToNamesMap[graphName] = std::unordered_map<uint32_t, std::string>();
      }
      auto fbTensorInfo = fbTensorInfosVector->Get(tIdx);
      if (fbTensorInfo->name() != nullptr) {
        graphTensorIdToNamesMap[graphName][fbTensorInfo->id()] = fbTensorInfo->name()->str();
      } else {
        QNN_DEBUG("fbTensorInfo->name() is nullptr for graph [%s] and tensorId [%d].",
                  graphName.c_str(),
                  fbTensorInfo->id());
        graphTensorIdToNamesMap[graphName][fbTensorInfo->id()] = "";
      }
    }
  }
  QNN_FUNCTION_EXIT_LOG;
  return returnStatus;
}

// Function to help extract graphs' metadata from loaded flatbuffers structure
// in deserializeData().
StatusCode extractGraphsInfo(
    const sample_app::ContextCache *contextCache,
    std::unordered_map<std::string, std::unordered_map<uint32_t, std::string>>
        &graphTensorIdToNamesMap,
    uint32_t *graphsCount) {
  QNN_FUNCTION_ENTRY_LOG;
  auto returnStatus   = StatusCode::SUCCESS;
  *graphsCount        = contextCache->graphsCount();
  auto fbGraphsVector = contextCache->graphsInfo();
  if (returnStatus == StatusCode::SUCCESS) {
    for (size_t gIdx = 0; gIdx < *graphsCount; gIdx++) {
      QNN_DEBUG("Extracting graphsInfo for graph Idx: %d", gIdx);
      auto fbGraph = fbGraphsVector->Get(gIdx);
      if (StatusCode::SUCCESS != extractTensorsInfo(fbGraph->inputTensorsInfo(),
                                                    fbGraph->name()->str(),
                                                    graphTensorIdToNamesMap,
                                                    fbGraph->inputTensorsCount())) {
        returnStatus = StatusCode::FAILURE;
        break;
      }
      if (StatusCode::SUCCESS != extractTensorsInfo(fbGraph->outputTensorsInfo(),
                                                    fbGraph->name()->str(),
                                                    graphTensorIdToNamesMap,
                                                    fbGraph->outputTensorsCount())) {
        returnStatus = StatusCode::FAILURE;
        break;
      }
    }
  }
  QNN_FUNCTION_EXIT_LOG;
  return StatusCode::SUCCESS;
}

// Function to deserialize flatbuffers related to caching.
//  1. Flatbuffers are loaded from a binary buffer.
//  2. Metadata containing a map of tenor id to names is populated.
//  3. Binary blob is retrieved and copied into a uint8_t buffer.
StatusCode deserializeData(
    std::string filePath,
    std::unordered_map<std::string, std::unordered_map<uint32_t, std::string>>
        &graphTensorIdToNamesMap,
    uint32_t *graphsCount,
    std::shared_ptr<uint8_t> &binaryCache,
    uint64_t &binaryCacheSize) {
  QNN_FUNCTION_ENTRY_LOG;
  size_t fileSize{0};
  tools::datautil::StatusCode fileSizeReadStatus{tools::datautil::StatusCode::SUCCESS};
  std::tie(fileSizeReadStatus, fileSize) = tools::datautil::getFileSize(filePath);
  if (0 == fileSize) {
    QNN_ERROR("Received path to an empty file. Nothing to deserialize.");
    return StatusCode::FAILURE;
  }
  std::unique_ptr<char> buffer(new char[fileSize]);
  if (!buffer) {
    QNN_ERROR("Failed to allocate memory.");
    return StatusCode::FAILURE;
  }
  if (tools::datautil::StatusCode::SUCCESS !=
      tools::datautil::readBinaryFromFile(
          filePath, reinterpret_cast<uint8_t *>(buffer.get()), fileSize)) {
    QNN_ERROR("Failed to read binary data.");
    return StatusCode::FAILURE;
  }
  // Verify the buffer is well-formed
  flatbuffers::Verifier verifier(reinterpret_cast<uint8_t *>(buffer.get()), fileSize);
  if (!VerifyContextCacheBuffer(verifier)) {
    QNN_ERROR("Invalid flatbuffer binary: %s", filePath.c_str());
    return StatusCode::FAILURE;
  }
  auto contextCache = GetContextCache(buffer.get());
  binaryCacheSize   = contextCache->binaryCacheSize();
  binaryCache       = std::shared_ptr<uint8_t>(
      static_cast<uint8_t *>(malloc(binaryCacheSize * sizeof(uint8_t))), free);
  if (nullptr == binaryCache) {
    QNN_ERROR("Failed to allocate memory for binaryCache");
    return StatusCode::FAILURE;
  }
  pal::StringOp::memscpy(
      binaryCache.get(), binaryCacheSize, contextCache->binaryCache()->Data(), binaryCacheSize);

  if (StatusCode::SUCCESS !=
      extractGraphsInfo(contextCache, graphTensorIdToNamesMap, graphsCount)) {
    QNN_ERROR("Failed to extract graphsInfo.");
    return StatusCode::FAILURE;
  }
  QNN_FUNCTION_EXIT_LOG;
  return StatusCode::SUCCESS;
}

}  // namespace caching
}  // namespace sample_app
}  // namespace tools
}  // namespace qnn
