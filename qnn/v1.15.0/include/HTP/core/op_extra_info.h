//==============================================================================
//
// Copyright (c) 2020 Qualcomm Technologies, Inc.
// All Rights Reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
//
//==============================================================================

#pragma once

#include <utility>

namespace hnnx {

// map Op* to a few properties, to avoid the need to keep them in the Op
// object. Currently contains the ID and the gate/done checkpoint indices.
struct OpExtraInfo {
    unsigned long long id;
    std::pair<int, int> chkpts;
    explicit OpExtraInfo(unsigned long long id_in) : id(id_in), chkpts(-1, -1)
    {
    }
    OpExtraInfo(unsigned long long id_in, int cg, int dc)
        : id(id_in), chkpts(cg, dc)
    {
    }
    OpExtraInfo() : OpExtraInfo(0) {}

    bool valid() const { return id != 0; };
    void clear() { id = 0; };
};

} // namespace hnnx
