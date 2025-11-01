//==============================================================================
//
// Copyright (c) 2020 Qualcomm Technologies, Inc.
// All Rights Reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
//
//==============================================================================

#ifndef SERIALIZE_REGISTER_H
#define SERIALIZE_REGISTER_H 1

#include "crate.h"
#include "simple_op.h"
#include "op_info.h"

namespace hnnx {

class SimpleOpWrapper;

template <typename T> struct deserialize_tensor_using_constructor {
    static uptr_Tensor deserialize(const Op *producer, Deserializer &dctx,
                                   Graph &graph_in)
    {
        // If the graph has a crate, put the deserialized
        // Tensor into the crate, using a 'Tensor_Deleter' which won't actually try to delete it.
        Crate *cp = graph_crate(graph_in);
        if (cp == nullptr) {
            std::unique_ptr<Tensor> optr = std::make_unique<T>(producer, dctx);
            return optr;
        } else {
            Tensor *op_ptr = cp->emplace<T>(producer, dctx);
            return std::unique_ptr<Tensor, Tensor_Deleter>(
                    op_ptr, Tensor_Deleter(true));
        }
    }
};

using deserialize_op_func = uptr_Op (*)(Deserializer &dctx, Graph &graph_in);

template <typename T> struct deserialize_op_using_constructor {
    static uptr_Op deserialize(Deserializer &dctx, Graph &graph_in)
    {
        // If the graph has a crate, put the deserialized
        // op into the crate, using an 'Op_Deleter' which won't actually try to delete it.
        Crate *cp = graph_crate(graph_in);
        assert(cp != nullptr);
        Op *op_ptr = cp->emplace<T>(graph_in, dctx);
        return std::unique_ptr<Op, Op_Deleter>(op_ptr, Op_Deleter(true));
    }
};

__attribute__((visibility("default"))) uptr_Op
deserialize_simple_op_wrapper(Graph &graph_in, Deserializer &dctx,
                              std::unique_ptr<SimpleOpBase> sop_in, Crate *cp);

// used in serialization/deserialization of SimpleOpWrapper
template <typename T> struct deserialize_op_using_constructor_ext {
    static uptr_Op deserialize(Deserializer &dctx, Graph &graph_in)
    {
        // If the graph has a crate, put the deserialized
        // op into the crate, using an 'Op_Deleter' which won't actually try to delete it.
        Crate *cp = graph_crate(graph_in);
        // creating SimpleOp
        std::unique_ptr<T> so = std::make_unique<T>();
        return deserialize_simple_op_wrapper(graph_in, dctx, std::move(so), cp);
    }
};
template <typename OPTYPE> inline void register_framework_op(char const *opname)
{
    register_op_info(typeid(OPTYPE), hnnx::cost_function_t(StandardCosts::FAST),
                     0, (SimpleOpFactory) nullptr, false, opname);
    deserialize_op_register(
            typeid(OPTYPE), opname,
            deserialize_op_using_constructor<OPTYPE>::deserialize);
}

} // namespace hnnx
#endif // SERIALIZE_REGISTER_H
