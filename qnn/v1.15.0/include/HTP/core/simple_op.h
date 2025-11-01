//==============================================================================
//
// Copyright (c) 2021 Qualcomm Technologies, Inc.
// All Rights Reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
//
//==============================================================================

#ifndef SIMPLE_OP_H
#define SIMPLE_OP_H

#include "graph_status.h"
#include "template_help.h"
#include "op_utils.h"
#include "template_help_tensor_ext.h"

namespace hnnx {

// base class of SimpleOp
class SimpleOpBase {
  public:
    SimpleOpBase() {}
    virtual ~SimpleOpBase() = default;
    virtual std::type_info const *true_type() const { return &typeid(*this); }
    virtual size_t get_n_inputs() const = 0;
    virtual size_t get_n_outputs() const = 0;
    virtual uint8_t const *get_input_tensor_types() const = 0;
    virtual uint8_t const *get_output_tensor_types() const = 0;
    virtual bool needs_tcm() const = 0;
    virtual GraphStatus
    execute(const std::vector<Tensor const *> &inputs_in,
            const std::vector<uptr_Tensor> &outputs_in) const noexcept = 0;
};

/*
 * SimpleOp class
 * used by external op packages
 * for the purpose of exposing fewer symbols
 */
template <auto F> class SimpleOp : public SimpleOpBase {
    using Ftype = std::remove_pointer_t<decltype(F)>;

  public:
    // the collection of input types, as pointers
    using input_ptr_tuple_type = typename ArgsTuples<Ftype>::input_ptr_tuple;
    // the collection of output types, as pointers
    using output_ptr_tuple_type = typename ArgsTuples<Ftype>::output_ptr_tuple;
    // the inputs as real types
    using input_tuple_defs = typename ArgsTuples<Ftype>::input_tuple;
    // the outputs as real types
    using output_tuple_defs = typename ArgsTuples<Ftype>::output_tuple;
    // A graph argument is not allowed
    using graph_ptr_tuple_type = typename ArgsTuples<Ftype>::graph_ptr_tuple;

    // numbers of inputs and outputs
    static constexpr size_t n_inputs = ArgsTuples<Ftype>::n_inputs;
    static constexpr size_t n_outputs = ArgsTuples<Ftype>::n_outputs;

    // indices representing input and outputs tensor types
    // only tensor types from AllTensors in template_help_tensor_ext.h are allowed to be used in SimpleOp
    static constexpr std::array<uint8_t, n_inputs> input_tensor_type_indices =
            tensors_to_indices<std::array<uint8_t, n_inputs>,
                               input_tuple_defs>();
    static constexpr std::array<uint8_t, n_outputs> output_tensor_type_indices =
            tensors_to_indices<std::array<uint8_t, n_outputs>,
                               output_tuple_defs>();
    // boolean representing whether all tensor types used in outputs are from AllTensors list
    static constexpr bool are_tensor_types_valid =
            check_tensor_types_valid<output_tuple_defs>();

    // number of graph parameter
    static constexpr size_t n_graphs =
            std::tuple_size<std::decay_t<graph_ptr_tuple_type>>::value;

    SimpleOp() : SimpleOpBase() {}

    ~SimpleOp() override = default;

    size_t get_n_inputs() const override { return n_inputs; }

    size_t get_n_outputs() const override { return n_outputs; }

    uint8_t const *get_input_tensor_types() const override
    {
        return input_tensor_type_indices.data();
    }

    uint8_t const *get_output_tensor_types() const override
    {
        return output_tensor_type_indices.data();
    }

    bool needs_tcm() const override
    {
        // replace with less dependency in the future
        static constexpr bool needs_tcm_t =
                has_memclass<MemoryClass::TCM, output_tuple_defs>::value;
        return needs_tcm_t;
    }

    static inline bool valid_construction(size_t n_inputs_in,
                                          size_t n_outputs_in,
                                          Tensor const *const *inputs_in,
                                          OutputDef const *const *outputs_in,
                                          Graph &graph_in)
    {
        if (n_inputs != n_inputs_in)
            return false;
        if (n_outputs != n_outputs_in)
            return false;
        if (!are_input_tensors_compatible<n_inputs, input_ptr_tuple_type>(
                    graph_in, inputs_in))
            return false;
        if (!are_output_defs_valid<n_outputs, output_tuple_defs>(outputs_in,
                                                                 graph_in))
            return false;
        if (n_graphs)
            return false;
        return true;
    }

    GraphStatus
    execute(const std::vector<Tensor const *> &inputs_in,
            const std::vector<uptr_Tensor> &outputs_in) const noexcept override
    {
        input_ptr_tuple_type typed_inputs;
        tuple_convert(typed_inputs,
                      tuple_from_vec_N<n_inputs>(inputs_in,
                                                 [](auto &x) { return x; }));
        output_ptr_tuple_type typed_outputs;
        tuple_convert(typed_outputs,
                      tuple_from_vec_N<n_outputs>(
                              outputs_in, [](auto &x) { return x.get(); }));
        auto args = std::tuple_cat(typed_outputs, typed_inputs);
        return GraphStatus(tuple_deref_and_apply(F, args));
    }

    static inline std::unique_ptr<SimpleOpBase>
    create(size_t n_inputs_in, size_t n_outputs_in,
           Tensor const *const *inputs_in, OutputDef const *const *outputs_in,
           Graph &graph_in)
    {
        if (SimpleOp::valid_construction(n_inputs_in, n_outputs_in, inputs_in,
                                         outputs_in, graph_in)) {
            return std::move(std::make_unique<SimpleOp>());
        } else {
            return std::unique_ptr<SimpleOp>{};
        }
    }
};

} // namespace hnnx

template <auto F> struct SimpleOpType {
    using type = hnnx::SimpleOp<F>;
};

#endif // SIMPLE_OP_H
