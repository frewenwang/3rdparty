//==============================================================================
//
// Copyright (c) 2018 Qualcomm Technologies, Inc.
// All Rights Reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
//
//==============================================================================

#ifndef OP_H
#define OP_H

#include "flags.h"
#include "graph_status.h"
#include "op_def.h"
#include "executable.h"
#include "cost_funcs.h"

#include <set>
#include <vector>

class Graph;
class Tensor;
namespace hnnx {
class OpIoPtrs;
class SimpleOpBase;
} // namespace hnnx
/*
 * What are the fundamentals of an op?
 * 
 * It has an ID to be able to refer to it easily
 * It has zero or more inputs.  Inputs refer to an output of another op.
 * It has zero or more outputs.  Output definitions determine the max size of an op.
 * It can execute.  When an op executes, it uses the inputs to produce the outputs.
 * 
 * There are also, probably some less important aspects to ops:
 * * Constructor / Destructor 
 * * In Hexagon NN V2, we have a hook during graph preparation.  This isn't
 *   always necessary, and maybe we should strive to make it unnecessary?
 * * We sometimes use flags to indicate something about an op
 *  
 * There will be some other aspects to ops eventually, dealing with when
 * to "wake up" and what other ops to notify when finished.  But for now 
 * we can just run one op at a time.
 */

#pragma GCC visibility push(default)

// Flags used to describe the class of checkpoints we have.
enum ChkptStoreType {
    ChkptNormal = 0, // N, M
    ChkptNone = 1, // 0, -1, or -1, -1.
    ChkptNoGate = 2, // 0, N
    ChkptNoDone = 3, // N, -1
    ChkptFlagShift = 2,
    ChkptFlagMask = ((1 << ChkptFlagShift) - 1),
};

/*
 * FIXME: instead of deserialize function, we should have a constructor with arguments (const char **bufp, size_t *)
 */

/**
 * @class Op
 *
 * @brief Basic, minimal Op
 * Ops inherit from this class
 *
 * This is starting out minimal, we will extend this in the future
 *
 * Maybe ID should be here, maybe not.
 */

class Op : public hnnx::Executable {
  protected:
    //! Interface to the external world is 32 bits for an Op ID (0 and above 0xF000_0000 are reserved for internal use).
    //! However, as we break ops down we want to have some semblance of the original op IDs while still maintaining unique IDs.
    //! So we make internal OpIDs 64 bits.
    //! Half of them can be the external OpID, and we can use a counter or something to uniquify in the other bits.
    //! We can accumulate performance information and such to still represent OpIDs on the interface.
    //	uint32_t dependencies;
    //	uint32_t dependencies_left;		// number of dependencies, FIXME: REPLACE WITH ATOMIC<UINT32_T>
    //	std::vector<Op *> consumers;
    Graph *graph_ptr;

  public:
    inline Graph &graph() const { return *graph_ptr; }
    //const unsigned long long int nope_my_id; // move this here so alignment doesn't waste 4 bytes
    Op(Graph &graph_in) : graph_ptr(&graph_in){};
    Op(Graph &graph_in, unsigned long long int my_id_in);
    Op(Graph &graph_in, hnnx::Deserializer &);
    Op(Op const &) = delete;
    Op &operator=(Op const &) = delete;
    virtual ~Op() = default; // virtual destructor
    // virtual GraphStatus execute() const noexcept = 0; in execute
    virtual GraphStatus prepare(hnnx::OpIoPtrs const &, bool tcm_available) = 0;
    virtual GraphStatus allocate() = 0;
    unsigned long long int id() const noexcept;

    ChkptStoreType get_chkpt_store_type() const;
    uint32_t get_chkpt_store_flags() const;
    static uint32_t is_vec_op(uint32_t flags)
    {
        return (flags >> ChkptFlagShift) & 1;
    };

    const std::pair<int, int> &chkpts() const noexcept;
    std::pair<int, int> &chkpts() noexcept;
    void set_chkpts(const std::pair<int, int> chkpts);
    void set_chkpts(int gate, int done);

    const Tensor *get_input(size_t which) const
    {
        return get_input_output(which, true);
    }
    const Tensor *get_output(size_t which) const
    {
        return get_input_output(which, false);
    }

    virtual bool set_input(size_t which, const Tensor *tensor) const
    {
        return false;
    }

    virtual bool
    is_valid() const noexcept = 0; // Is this op valid in this situation?
    void dependence_resolved() noexcept;
    bool is_const() const
            noexcept; // Data for this op always available, execution and dependence tracking not needed.
    virtual std::pair<size_t, size_t> num_inputs_outputs() const = 0;
    inline size_t num_outputs() const { return num_inputs_outputs().second; }
    inline size_t num_inputs() const { return num_inputs_outputs().first; }
    // typeid(*this) is always the true type of this... the more you know ===*
    virtual const char *true_name() const;
    virtual Flags_word get_flag_word() const { return hnnx::flags_for<Op>; }
    // get type, allowing for SimpleOpWrapper to get forwarded type.
    std::type_info const *get_type_extended() const;
    bool get_flag(Flags flag) const
    {
        return hnnx::test_flag_for(get_flag_word(), flag);
    }
    bool get_flag_and(Flags flag0, Flags flag1) const
    {
        return hnnx::test_flag_and(get_flag_word(), flag0, flag1);
    }
    inline hnnx::blockid_set_t input_blocks(int mc_sel = -1) const
    {
        return input_output_blocks(true, mc_sel);
    }
    inline hnnx::blockid_set_t input_blocks(MemoryClass mc) const
    {
        return input_output_blocks(true, int(mc));
    }
    inline hnnx::blockid_set_t output_blocks(int mc_sel = -1) const
    {
        return input_output_blocks(false, mc_sel);
    }
    inline hnnx::blockid_set_t output_blocks(MemoryClass mc) const
    {
        return input_output_blocks(false, int(mc));
    }

    virtual void enumerate_blocks(hnnx::MemBlockEnumerator &en,
                                  bool is_input) const
    {
    }
    inline void enumerate_input_blocks(hnnx::MemBlockEnumerator &en) const
    {
        enumerate_blocks(en, true);
    }
    inline void enumerate_output_blocks(hnnx::MemBlockEnumerator &en) const
    {
        enumerate_blocks(en, false);
    }
    virtual void enumerate_all_blocks(hnnx::MemBlockEnumerator &en,
                                      bool is_input) const
    {
        enumerate_blocks(en, is_input);
    }

    // The 'ef' parameter to these functions is a callable (function, lambda, std::function...)
    // compatible with MemBlockEnumerator::supply_blocks_func
    template <typename ENFUNC>
    inline void enumerate_blocks_withfunc(ENFUNC &&ef, bool is_input) const
    {
        hnnx::MemBlockEnumWrapper<std::remove_reference_t<ENFUNC>> enumer(
                std::forward<ENFUNC>(ef));
        this->enumerate_blocks(enumer, is_input);
    }
    template <typename ENFUNC>
    inline void enumerate_input_blocks_withfunc(ENFUNC &&ef) const
    {
        enumerate_blocks_withfunc(std::forward<ENFUNC>(ef), true);
    }
    template <typename ENFUNC>
    inline void enumerate_output_blocks_withfunc(ENFUNC &&ef) const
    {
        enumerate_blocks_withfunc(std::forward<ENFUNC>(ef), false);
    }

    template <typename ENFUNC>
    inline void enumerate_all_blocks_withfunc(ENFUNC &&ef, bool is_input) const
    {
        hnnx::MemBlockEnumWrapper<std::remove_reference_t<ENFUNC>> enumer(
                std::forward<ENFUNC>(ef));
        this->enumerate_all_blocks(enumer, is_input);
    }
    template <typename ENFUNC>
    inline void enumerate_all_input_blocks_withfunc(ENFUNC &&ef) const
    {
        enumerate_all_blocks_withfunc(std::forward<ENFUNC>(ef), true);
    }
    template <typename ENFUNC>
    inline void enumerate_all_output_blocks_withfunc(ENFUNC &&ef) const
    {
        enumerate_all_blocks_withfunc(std::forward<ENFUNC>(ef), false);
    }

    virtual void serialize(hnnx::Serializer &) const = 0;
    void serialize_common(hnnx::Serializer &) const;
    void serialize_common(hnnx::Serializer &,
                          std::type_info const &actual_type) const;
    using tensor_deserializer_register_func = int (*)();

    // there are fewer combinations of true_output_tuple_type than there are
    // TypicalOpIO, so it's better to return a function here than to make one.
    //
    static inline tensor_deserializer_register_func
    get_tensor_deserializer_register_func()
    {
        return hnnx::deserialize_tensor_tuple<std::tuple<>, false>::f_ptr();
    }
    float cost() const;

    // 'clone_mode' for Op::clone
    enum op_clonemode {
        opclone_auto, // opclone_dup if op has NULL_EXEC, otherwise opclone_realloc
        opclone_realloc, // when duplicating the output tensors, zero all block ids and reallocate
        opclone_dup // duplicate output with same block ids; and suppress ctor hooks.
    };
    //
    //
    // Clone an Op.
    // This makes an op with the same input tensors as the current Op, and the specified
    // OpId. The new op has new output tensors which are 'duplicate_clone' of the output
    // tensors of the existing Op.
    //
    // Caveats:
    //  - ALWAYS CHECK FOR NULL RETURN VALUE. There is no errlog if the clone fails, just a null return.
    //  - Not all Op can be cloned in this way; it applies only to Ops which can be created from OpDef.generate().
    //    So, no things like SpillOp or ValidateOp.
    //  - The Op's 'constructor hooks' are only called if 'opclone_realloc' mode is specified (or selected via opclone_auto)
    //  - 'prepare' is called with tcm_available = true; it is assumed that if new Op needs that, the original
    //    op needed it too.
    //  - You can pass an alternate Op type ('clone X but as Y'... ); use extreme caution, will only work if the
    //    number and types of input and output tensors are supported by Y.
    //
    hnnx::uptr_Op clone(OpId, op_clonemode opclonemode = opclone_auto,
                        std::type_info const *as_type = nullptr) const;

    // these are not virtual, but are thin wrappers of swap_output so they act as if virtual.
    /// @brief remove output tensor from an op
    /// returns empty pointer on failure. Always fails on Op types which don't overload swap_output.
    hnnx::uptr_Tensor steal_output(size_t which);
    /// @brief attach an output tensors to an Op.
    /// succeeds (and returns true) if val is not empty, 'which' is in range and the Op doesn't already have that
    //  output set; otherwise it returns false and val is unchanged.
    //  Always fails on Op types which don't overload swap_output.
    bool install_output(size_t which, hnnx::uptr_Tensor &&val);

  protected:
    virtual Tensor const *get_input_output(size_t which,
                                           bool is_input) const = 0;
    // swap_output underpins steal_output and install_output:
    // it should:
    //    return false, if these operations are not supported, or if the index is too large;
    //    otherwise:
    //       - if the incoming val is empty, treat it as 'steal_input'; if ok, swap and return true;
    //         perform any other side-effects which may be needed.
    //       - otherwise it's a 'set_output'. return false if the output is already set; otherwise
    //         swap and return true (and perform any side-effects).
    //
    virtual bool swap_output(size_t which, hnnx::uptr_Tensor &val);

    // (GLS note: clang allows derived classes to call Op::get_memory_blocks
    // even if it's marked private. But not a derived of a derived. I have no idea why).
    //
    // this is instantiated twice in each Op subclass, but only for a small number of iterator types,
    // so it should not be inlined.
    template <typename TensorIter>
    void get_memory_blocks
            [[gnu::noinline]] (hnnx::MemBlockEnumerator &en, TensorIter begin,
                               TensorIter end) const
    {
        for (TensorIter it = begin; it != end; ++it) {
            (*it)->enum_memory_blocks(en);
        }
    }
    // essentially the same thing, but only used twice in VariadicOpBase, we want it inlined.
    template <typename TensorPtr>
    void get_memory_blocks
            [[gnu::always_inline]] (hnnx::MemBlockEnumerator &en,
                                    std::vector<TensorPtr> const &ports) const
    {
        for (TensorPtr const &tptr : ports) {
            tptr->enum_memory_blocks(en);
        }
    }

    // legacy interface, implemented via enumerate_blocks
    hnnx::blockid_set_t input_output_blocks(bool is_input, int mc_sel) const;

    // subclasses can forward enumerate_blocks to this method to reduce copy-pasta -
    // it just traverses the inputs (or outputs) using the virtual API and calls
    // enum_memory_blocks on all the tensors it discovers.
    void enumerate_blocks_generic(hnnx::MemBlockEnumerator &en,
                                  bool is_input) const;

    // subclasses can forward 'allocate' to this method to reduce copy-pasta.
    // it just calls allocate on all of the outputs it discovers using the
    // virtual function API.
    // If allocator is null, it uses the alloc in the graph.
    GraphStatus allocate_generic(hnnx::Allocator *alloc = nullptr);

    void serialize_internal(hnnx::Serializer &sctx, ChkptStoreType st) const;
    uint32_t get_serialize_flags(ChkptStoreType t) const;
};

// only needed because we're using Const and Shape for ConstWrapperOp and ShapeWrapperOp; otherwise true_name could be non-virtual.
#define TRUE_NAME_METHODS_K(NAME)                                              \
    virtual const char *true_name() const override { return NAME; }

#define TRUE_NAME_METHODS /* don't override true_name() in base */

/**
 * @brief Op Cost return types
 * As of now we support 3 types of cost for Ops
 */

struct StandardCosts {
    static constexpr float GLACIAL = 0x1.0p48; // 2**48 cycles
    static constexpr float SNAIL = 0x1.0p32; // 2**32 cycles
    static constexpr float FAST = 0x1.0p8; // 256 cycles
    static constexpr float FREE = 0x1.0p-64;
    static constexpr float DISABLE =
            0x1.0p50; // 2**50 cycles, worse than GLACIAL, don't select this.
};

/*
 * EJP: FIXME: Cost here is a simple fixed cost.
 * Having simple costs available and a slow fixed cost available by default is great.
 * 
 * But to accurately reflect cost, we need be able to inspect the details of the op definition.
 * For example, a const of a convolution will depend on the types and shapes of
 * weights and activations.
 *
 */

namespace hnnx {

/**
 * Return the cost_function_t object for the Op.
 * The Ops need to specialize this class
 * if its cost differs from the default one.
 */

template <typename ConcreteOp> inline hnnx::cost_function_t get_costf()
{
    return hnnx::cost_function_t(StandardCosts::GLACIAL);
}

/*
 * For concrete version of an op, see typical_op.h
 */

/*
 * Not the typical Const op, but a wrapper around a Tensor that someone has formed...
 */

class ConstWrapperOp : public Op {
    uptr_Tensor owned_tensor;
    const Tensor *tensor_ptr;

  public:
    ConstWrapperOp(Graph &graph_in, OpId my_id_in, const OpDef *op_def_in);
    ConstWrapperOp(Graph &graph_in, OpId my_id_in, uptr_Tensor owned_tensor_in);
    ConstWrapperOp(Graph &graph_in, hnnx::Deserializer &dctx);
    // make a persistent Flat tensor with the given type, shape, data,
    // and wrap it in a ConstWrapperOp. May not support all DTtype, but definitely
    // Float32 and Int32, and QUint8. See implementaton in op.cc
    ConstWrapperOp(Graph &graph_in, OpId my_id_in, const OutputDef &def,
                   void const *data_in);

    virtual ~ConstWrapperOp();
    virtual GraphStatus execute() const noexcept override
    {
        return GraphStatus::Success;
    }
    virtual hnnx::Executable::ItemType compile() const noexcept override
    {
        return hnnx::Executable::null_item();
    }
    virtual GraphStatus prepare(hnnx::OpIoPtrs const &,
                                bool tcm_available) override
    {
        return GraphStatus::Success;
    }
    virtual GraphStatus allocate() override { return GraphStatus::Success; }
    virtual std::pair<size_t, size_t> num_inputs_outputs() const override
    {
        return {0, 1};
    }
    virtual bool is_valid() const noexcept override { return true; }
    TRUE_NAME_METHODS_K("Const")

    virtual void serialize(hnnx::Serializer &sctx) const override
    {
        Op::serialize_common(sctx);
        tensor_ptr->serialize(sctx);
        //sctx.serialize_uint64(unsigned long)tensor_ptr);
    }

  protected:
    virtual const Tensor *get_input_output(size_t which,
                                           bool is_input) const override
    {
        return is_input ? nullptr : tensor_ptr;
    }
    virtual bool set_input(size_t which, const Tensor *tensor) const override
    {
        return false;
    }
};

class ShapeWrapperOp : public Op {
    uptr_Tensor shape; // must actually be a TensorShape
  public:
    ShapeWrapperOp(Graph &graph_in, OpId my_id_in, const OpDef *op_def_in);
    ShapeWrapperOp(Graph &graph_in, OpId my_id_in, uptr_Tensor owned_tensor_in);
    ShapeWrapperOp(Graph &graph_in, hnnx::Deserializer &);
    virtual GraphStatus execute() const noexcept override
    {
        return GraphStatus::Success;
    }
    virtual hnnx::Executable::ItemType compile() const noexcept override
    {
        return hnnx::Executable::null_item();
    }
    virtual GraphStatus prepare(hnnx::OpIoPtrs const &,
                                bool tcm_available) override
    {
        return GraphStatus::Success;
    }
    virtual GraphStatus allocate() override { return GraphStatus::Success; }
    virtual std::pair<size_t, size_t> num_inputs_outputs() const override
    {
        return {0, 1};
    }
    virtual bool is_valid() const noexcept override { return true; }
    TRUE_NAME_METHODS_K("Shape")

    virtual void serialize(hnnx::Serializer &sctx) const override
    {
        Op::serialize_common(sctx);
        shape->serialize(sctx);
    }

  protected:
    virtual const Tensor *get_input_output(size_t which,
                                           bool is_input) const override
    {
        return is_input ? nullptr : shape.get();
    }
    virtual bool set_input(size_t which, const Tensor *tensor) const override
    {
        return false;
    }
};

// MetaOpBase is a shim which provides empty defs for all of the =0 virtual methods,
// so that internal Ops (e.g. PreloadOp) can be based on this and not need to define any they don't need
//
class MetaOpBase : public Op {
  public:
    MetaOpBase(Graph &graph_in) : Op(graph_in){};
    MetaOpBase(Graph &graph_in, unsigned long long int my_id_in)
        : Op(graph_in, my_id_in)
    {
    }
    MetaOpBase(Graph &graph_in, hnnx::Deserializer &dctx) : Op(graph_in, dctx)
    {
    }

    virtual GraphStatus
    prepare(hnnx::OpIoPtrs const &,
            bool tcm_available) override; //{ return GraphStatus::Success;}
    virtual GraphStatus allocate() override; // { return GraphStatus::Success;}
    virtual bool is_valid() const noexcept override; // {return false;}
    virtual std::pair<size_t, size_t>
    num_inputs_outputs() const override; //{ return {0,0};}
    virtual Tensor const *
    get_input_output(size_t which,
                     bool is_input) const override; // {return nullptr;}
    virtual void serialize(hnnx::Serializer &) const override; // {}
    virtual uptr_Op
    clone_meta(OpId new_opid) const; // {return uptr_Op(nullptr);}
};

// this is a base class for adding hooks on construction of Ops.
// May not have data members or dtor - so it's just a vtable pointer, and is constexpr constructable
// All methods must be const, and return GraphStatus; the 'default' methods do nothing and return GraphNotApplicable.
// So, we can allow two or more hooks to be attached to an Op; when calling a method,
// we will call it on the first one, and if it returns NotApplicable, we will try the next
// one, etc (so they are 'layered', in effect).
//
class OpHookBase {
  public:
    virtual GraphStatus pre_output_prep(OpIoPtrs const &, Op &) const;
    virtual GraphStatus pre_allocate(OpIoPtrs const &, Op &) const;
};

using SimpleOpFactory = std::unique_ptr<SimpleOpBase> (*)(
        size_t n_inputs_in, size_t n_outputs_in, Tensor const *const *inputs_in,
        OutputDef const *const *outputs_in, Graph &graph_in);

} // namespace hnnx

#pragma GCC visibility pop

#endif /*OP_H*/
