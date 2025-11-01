//==============================================================================
//
// Copyright (c) 2020-2021 Qualcomm Technologies, Inc.
// All Rights Reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
//
//==============================================================================

#ifndef OPTIMIZE_H
#define OPTIMIZE_H 1

/*
 * PLEASE LEAVE graph.h OUT OF THIS FILE
 */

#include "c_tricks.h"
#include "op_def.h"
#include "unique_types.h"

#include <array>
#include <cassert>
#include <functional>
#include <limits>
#include <map>
#include <string>
#include <tuple>
#include <utility>
#include <vector>
#include "optimize_defs.h"
#include "optimize_flags.h"
#include "optim_filter.h"
#include "match_op.h"
#include "oexpr.h"
#include "op_package_name.h"

/*
 * We want Match, Replacement, and Constraint to have mostly their own namespace,
 * so that things like "OP" can mean different things in different places.
 *
 * However, we want to be able to share things like the context
 * We could do this with extra state in each owned class, but that seems wasteful.
 *
 * Instead, we use private class members to give things unique namespaces, but
 * inherit to concatenate classes and values that should be shared.
 *
 * 
 * A note about a trick:
 *  Each class (Match, Constraint, Replacement) has a templated function for
 *  UniqueType that's unused.  That lets us createa arbitrary member functions
 *  later.
 * 
 *  If you're curious, the unique type comes from the current filename and line.
 *
 *  There's also a member function pointer that is used when creating instances
 *  of Match/Replacement/Constraint that we initialize to the arbitrary member
 *  functions that we're creating.
 *
 */

class Replacement;

using ReplFunc = OptFunction<OpRef(Replacement &, OpDef const &)>;

namespace hnnx {
class Match;

typedef std::function<bool(Match &, OpDef const &)> MatchFunc;

template <oExp::OpVnt V, typename T>
ReplFunc wrap_as_replfunc(oExp::opexpr<V, T> &&opr)
{
    return ReplFunc::create(
            [op{std::move(opr)}](Replacement &rpx, OpDef const &) -> OpRef {
                return op.eval(rpx);
            });
}
template <oExp::OpVnt V, typename T>
ReplFunc wrap_as_replfunc(oExp::opexpr<V, T> const &op)
{
    return ReplFunc::create([op](Replacement &rpx, OpDef const &) -> OpRef {
        return op.eval(rpx);
    });
}
inline ReplFunc wrap_as_replfunc(ReplFunc &&rep)
{
    return std::move(rep);
}
inline ReplFunc wrap_as_replfunc(ReplFunc const &rep)
{
    return rep;
}

} // namespace hnnx

#pragma GCC visibility push(default)
namespace gxE {
class GXEngine;
}
#pragma GCC visibility pop

// these are function objects which return the various types,
// and are called with a const reference to 'Constraint'
//
// some of them will cheat and used a static-cast to look at the Split_Context;
// maybe it should be moved to Constraint class. Also, those that implement MESSAGE
// etc will cheat and call a non-const method of the Constraint class.
//

typedef oExp::sFunction<int> ReplFuncInt;
typedef oExp::sFunction<bool> ReplFuncBool;
typedef oExp::sFunction<DType> ReplFuncDType;
typedef oExp::sFunction<float> ReplFuncFloat;

typedef OpRef (*external_replace_funcp)(Replacement &, OpDef const &);

namespace hnnx {

#pragma GCC visibility push(default)

// EJP: FIXME: Instead of separate optim_config things that require several changes in several places,
// we need to plumb through a way to get an option out of graph.options that comes from options.def

// the optim_config struct is visible within the namespace of
// a DEFOPT as 'Config', e.g. Config.tcm_size reads the tcm size.
//
// The actual values are kept in struct optim_config_values, which is instantiated
// within the optimization object.
//
// The struct is actually a static variable which contains instances of optim_configvar;
// each one contains a field pointer into optim_config_values. When these appear
// in an expression, they are converted to an oExp<config,T> containig a copy
// of the struct offset; i.e. the oExp can be built without an instance of optim_config_values existing.
//

struct optim_config_values {
    // values which are not directly available from 'Options'
    size_t tcm_size; // the current tcm_size
    size_t tcm_size_for_tiling; // tcm size to be used for tiling
};

// wrapper functions for graph access
OpRef graph_gen_Const_int32_common_wrapper(Graph &graph_in, const OpDef &old,
                                           const OutputDef &out_def,
                                           const uint8_t *data,
                                           size_t data_len);

template <DType DT>
OpRef graph_gen_Const_scalar_wrapper(
        Graph &graph_in, const OpDef &old,
        typename dtype_traits<DT>::element_type constval);

// these are written as specializations.
template <>
OpRef graph_gen_Const_scalar_wrapper<DType::Int32>(Graph &graph_in,
                                                   const OpDef &old,
                                                   NN_INT32_T constval);
template <>
OpRef graph_gen_Const_scalar_wrapper<DType::Float32>(Graph &graph_in,
                                                     const OpDef &old,
                                                     float constval);
#pragma GCC visibility pop

/* EJP: FIXME: A lot of stuff has accumulated here... const generation, helper functions, etc... */

/*
 * EJP: FIXME: see if we can change some of these functions to just return OpRef instead of 
 * having to return a funcgtion<OpRef(OpDef &)> and all the lambda stuff
 */

template <template <typename... Ts> class C, typename K, typename V>
inline bool exists(C<K, V> const &m, const K &test)
{
    return m.find(test) != m.end();
}

template <template <typename... Ts> class C, typename K, typename V,
          typename V1, typename V2>
inline bool exists(C<K, V, V1, V2> const &m, const K &test)
{
    return m.find(test) != m.end();
}

/*
 * EJP: FIXME: this stuff here at a global level should move somewhere.
 * Maybe even outline the functions...
 */

namespace opt_util {
// map_to_size_t(x)
// maps integer types to size_t;
// passes ReplFuncInt as-is
// This is used to minimize the number of distinct specializations
// of gen_Shape (each having its own lambda).
template <typename T> struct map_to_sizet_helper {
    static_assert(std::numeric_limits<T>::is_integer);
    static inline constexpr size_t convert(T x) { return x; }
};
template <oExp::Variant V, typename T>
struct map_to_sizet_helper<oExp::expr<V, T>> {
    static inline ReplFuncInt convert(oExp::expr<V, T> &&x)
    {
        return oExp::wrap_as_function<int>(std::move(x));
    }
};

template <typename T> inline auto map_to_size_t(T &&x)
{
    return map_to_sizet_helper<T>::convert(std::forward<T>(x));
}

inline size_t eval_size(oExp::ECtx &, size_t size)
{
    return (size_t)size;
}
inline size_t eval_size(oExp::ECtx &e, ReplFuncInt const &f)
{
    return (size_t)(f(e));
}

template <typename... Ts> inline ReplFunc gen_Shape_inner(Ts... sizes)
{
    return ReplFunc::create([=](Replacement &rpx, const OpDef &old) -> OpRef {
        OutputDef out_def = {
                .rank = sizeof...(Ts),
                .dtype = DType::Int32,
                .max_sizes = {eval_size(rpx, sizes)...},
                .zero_offset = 0,
                .stepsize = 0,
        };
        auto &g = old.graph();
        auto newref =
                graph_gen_Const_int32_common_wrapper(g, old, out_def, NULL, 0);
#if 0
        debuglog("Const shape %llx: rank=%zd (%zd,%zd,%zd,%zd...)",
                 newref.input_id, out_def.rank, out_def.max_sizes[0],
                 out_def.max_sizes[1], out_def.max_sizes[2],
                 out_def.max_sizes[3]);
#endif
        return newref;
    });
}

} // namespace opt_util

} // namespace hnnx

// This gen_Shape is intended for use in Replacement rules; the parameters
// are either integer constants or ReplFuncInt 's
// It returns a function.

/// \ingroup OptReplacement
/// @brief gen_Shape(..dims..) - construct an OpDef_Shape of the given dimensions.
///
/// The dimension parameters can be integers, but may also be one of
/// SPLIT_START("tag"), SPLIT_SIZE("tag"), SPLIT_DIM("tag"),  provided the expression
/// appears inside the operand of an AUTOSPLIT which uses the same tag
///
template <typename... Ts> inline ReplFunc gen_Shape(Ts... sizes)
{
    return hnnx::opt_util::gen_Shape_inner(
            hnnx::opt_util::map_to_size_t(std::move(sizes))...);
}

#pragma GCC visibility push(default)

//
// 'QuickShape' can be returned from a SHAPEFN_APPLY function; returning
// a QuickShape is equivalent to returning a gen_Shape() with the same dimensions.
//
struct QuickShape {
    struct empty_rank {
        unsigned r;
    };

    static constexpr unsigned maxdims = 8;
    unsigned rank;
    size_t dims[maxdims];
    // make with specific rank and dimensions, up to 4
    explicit inline constexpr QuickShape(size_t d) : rank(1), dims{d} {}
    inline constexpr QuickShape(size_t d0, size_t d1) : rank(2), dims{d0, d1} {}
    inline constexpr QuickShape(size_t d0, size_t d1, size_t d2)
        : rank(3), dims{d0, d1, d2}
    {
    }
    inline constexpr QuickShape(size_t d0, size_t d1, size_t d2, size_t d3)
        : rank(4), dims{d0, d1, d2, d3}
    {
    }
    // build from an OutputDef's shape info
    QuickShape(OutputDef const &odef)
    {
        int r = std::min((unsigned)odef.rank, maxdims);
        rank = r;
        for (int i = 0; i < r; i++) {
            dims[i] = odef.max_sizes[i];
        }
    }
    // set an output def based on QuickShape. Only useful in implementing modifiers.
    void to_outdef(OutputDef &odef) noexcept;
    explicit inline constexpr QuickShape(empty_rank const &erank)
        : rank(std::min((unsigned)erank.r, maxdims)), dims()
    {
    }
    // build with a given rank, and all zero dims
    static inline constexpr QuickShape make_empty(unsigned r)
    {
        return QuickShape(empty_rank{r});
    }
    // shortcut for make_empty( odef.rank)
    static inline QuickShape make_empty(OutputDef const &odef)
    {
        return QuickShape(empty_rank{unsigned(odef.rank)});
    }
};

// This is an 'immediate' gen_Shape. Shape can be given as vararg ints,
// or as std::vector<size_t>.
//

template <typename... Ts>
OpRef gen_Shape_immed(const OpDef &some_op, Ts... sizes);
OpRef gen_Shape_immed(const OpDef &some_op, std::vector<size_t> const &shape);

#pragma GCC visibility pop

// this is intended to be used with an explicit <DType::Float32> or whatever.
// (gen_Const_scalar must be specialized for the supported types).
template <DType DT>
inline OpRef
gen_ConstScalar_imm(const OpDef &old,
                    typename dtype_traits<DT>::element_type constval)
{
    auto &g = old.graph();
    return hnnx::graph_gen_Const_scalar_wrapper<DT>(g, old, constval);
}
// these are intended to be used in replacement rules, they return ReplFunc.

#pragma GCC visibility push(default)
/// \ingroup OptReplacement
/// @brief gen_ConstScalar_f32(floatval) - Make an Opdef_Const with given scalar float value
ReplFunc gen_ConstScalar_f32(float constval);
ReplFunc gen_ConstScalar_f32_func(ReplFuncFloat &&constval_f);
#pragma GCC visibility pop

template <typename T> inline ReplFunc gen_ConstScalar_f32(T &&expr)
{
    return gen_ConstScalar_f32_func(
            oExp::wrap_as_function<float>(std::forward<T>(expr)));
}

#pragma GCC visibility push(default)
/// \ingroup OptReplacement
/// @brief gen_ConstScalar_i32(intval) - Make an Opdef_Const with given scalar float value
ReplFunc gen_ConstScalar_i32(int constval);
ReplFunc gen_ConstScalar_i32_func(ReplFuncInt &&constval_f);
#pragma GCC visibility pop

template <typename T> inline ReplFunc gen_ConstScalar_i32(T &&expr)
{
    return gen_ConstScalar_i32_func(
            oExp::wrap_as_function<int>(std::forward<T>(expr)));
}

#pragma GCC visibility push(default)
ReplFunc gen_ConstArr_f32(float constval, size_t n);
ReplFunc gen_ConstArr_f32_func(ReplFuncFloat &&val_func, ReplFuncInt &&n_func);
#pragma GCC visibility pop

template <typename TVAL, typename TN>
inline ReplFunc gen_ConstArr_f32(TVAL &&val, TN &&nn)
{
    return gen_ConstArr_f32_func(
            oExp::wrap_as_function<float>(std::forward<TVAL>(val)),
            oExp::wrap_as_function<int>(std::forward<TN>(nn)));
}

#pragma GCC visibility push(default)
ReplFunc gen_ConstArr_i32(NN_INT32_T constval, size_t n);
ReplFunc gen_ConstArr_i32_func(ReplFuncInt &&val_func, ReplFuncInt &&n_func);
#pragma GCC visibility pop

template <typename TVAL, typename TN>
inline ReplFunc gen_ConstArr_i32(TVAL &&val, TN &&nn)
{
    return gen_ConstArr_i32_func(
            oExp::wrap_as_function<int>(std::forward<TVAL>(val)),
            oExp::wrap_as_function<int>(std::forward<TN>(nn)));
}

#pragma GCC visibility push(default)
//
// gen_ConstArr_vals_i32( ... ) allows creation of a an int32 const array, shape [1,1,1,n],
// with the given set of values in it.

// this implementation only used when all the values are constants
ReplFunc gen_ConstMat_i32__func(std::vector<NN_INT32_T> &&);
// this one is passed a std::vector of ReplFuncInt
ReplFunc gen_ConstMat_i32__func(std::vector<ReplFuncInt> &&);
#pragma GCC visibility pop

namespace hnnx {

// all_are_int<T,T,T,...>()  returns true if all of T,T .. are int,long or unsigned.
// or reference to.
//
template <typename... Ts> struct all_are_int_helper {
    static_assert(sizeof...(Ts) == 0, "template problem");
    static constexpr bool value = true;
};
template <typename T1, typename... Ts> struct all_are_int_helper<T1, Ts...> {
    using TX = std::remove_reference_t<T1>;
    static constexpr bool value =
            (std::is_same_v<TX, int> || std::is_same_v<TX, long> ||
             std::is_same_v<TX, unsigned>)&&all_are_int_helper<Ts...>::value;
};

template <typename... Ts> inline constexpr bool all_are_int()
{
    return all_are_int_helper<Ts...>::value;
}

} // namespace hnnx

//
// gen_ConstMat_i32( wid, ... wid*dep values ... ) -> [1,1,wid,dep] filled
// in with the values. 'wid' can zero, which is treated as 1.

template <typename TW, typename... Ts>
inline ReplFunc gen_ConstMat_i32(TW &&wid, Ts &&...values)
{
    if constexpr (hnnx::all_are_int<TW, Ts...>()) {
        std::vector<NN_INT32_T> parms = {NN_INT32_T(wid),
                                         NN_INT32_T(values)...};
        return gen_ConstMat_i32__func(std::move(parms));
    } else {
        std::vector<ReplFuncInt> parms = {
                oExp::wrap_as_function<int>(std::forward<TW>(wid)),
                oExp::wrap_as_function<int>(std::forward<Ts>(values))...};
        return gen_ConstMat_i32__func(std::move(parms));
    }
}
// gen_ConstArr_vals_i32 is just a special case of gen_ConstMat_i32

template <typename... Ts> inline ReplFunc gen_ConstArr_vals_i32(Ts &&...values)
{
    return gen_ConstMat_i32(0, std::forward<Ts>(values)...);
}

struct Split_Context {
    int start;
    int size;
    int dim;
};

#pragma GCC visibility push(default)

/**
 * \defgroup AutoSplitShapeFnApply  Functions for AUTOSPLIT_SHAPEFN_APPLY
 * \ingroup OptReplacement
 *
 * These are functions which may be used with SHAPEFN_APPLY.
 *
 * The first parameter is always Replacement &; the second is a Split_Context const & (obtained via the 'split_tag' parmeter
 * to the AUTOSPLIT_SHAPEFN_APPLY' and the remaining parameters are obtained from the AUTOSPLIT_SHAPEFN_APPLY, and may be
 * OpRef (mapped from "OperandTag" in the SHAPEFN_APPLY), or scalar values.
 *
 * The return value may be an OpRef representing a new graph object; instead, the function may return a QuickShape object
 * representing a shape, and the framework will convert this to an OpDef_Shape.
 *
 *
 * @{
 */
// :::EXTERNAL_SHAPEFN::: {  qshape simpledim_split_start(split,op,int); }

/// @brief make 'start' shape for 'simple' split (on specific dimension)
///
/// E.g. if dim= 2, and the SPLIT_START is 96, a shape { 0, 0, 96, 0} will be generated.
///
/// This is used within CHANGEDIM_SLICE
///
QuickShape simpledim_split_start(Replacement &rpx,
                                 Split_Context const &splitinfo,
                                 OpRef const &orig, int dim);

// :::EXTERNAL_SHAPEFN::: {  qshape simpledim_split_size(split,op,int); }

/// @brief make 'size' shape for 'simple' split (on specific dimension)
///
/// E.g. if tdim=2, and the SPLIT_START is 30, a shape { b, h, 30, d} will be generated
/// (where b,h,d are the 'default' dims)
///
/// This is used within CHANGEDIM_SLICE
///
QuickShape simpledim_split_size(Replacement &rpx,
                                Split_Context const &splitinfo,
                                OpRef const &orig, int dim);

// :::EXTERNAL_SHAPEFN::: {  qshape simple_split_start(split,op); }

/// @brief make 'start' shape for 'simple' split
///
/// E.g. if SPLIT_DIM=3, and the SPLIT_START is 96, a shape { 0, 0, 0, 96} will be generated.
///
/// This is used within TYPICAL_SLICE
///
QuickShape simple_split_start(Replacement &rpx, Split_Context const &splitinfo,
                              OpRef const &orig);

// :::EXTERNAL_SHAPEFN::: {  qshape simple_split_size(split,op); }

/// @brief make 'size' shape for 'simple' split
///
/// E.g. if SPLIT_DIM=3, and the SPLIT_SIZE is 30, a shape { b, h, w, 30} will be generated.
/// (where b,h,w are the 'default' dims)
///
/// This is used within TYPICAL_SLICE
///
QuickShape simple_split_size(Replacement &rpx, Split_Context const &splitinfo,
                             OpRef const &orig);

// :::EXTERNAL_SHAPEFN::: {  qshape conv_split_start_valid(split,op,op,op); }

/// @brief make 'start' shape for splitting input to 'valid' convolution, where the input is being split along height
///
/// Generates shape {0, SPLIT_START * stride_h, 0, 0 }

QuickShape conv_split_start_valid(Replacement &rpx,
                                  Split_Context const &splitinfo,
                                  OpRef const &Act, OpRef const &weights,
                                  OpRef const &stride);

// :::EXTERNAL_SHAPEFN::: {  qshape conv_split_size_valid(split,op,op,op); }

/// @brief make 'size' shape for splitting input to 'valid' convolution, where the input is being split along height
///
/// Generates shape {0, inrows, 0, 0 }
///
/// where inrows = stride_h * (SPLIT_SIZE-1) + filter_h
///
QuickShape conv_split_size_valid(Replacement &rpx,
                                 Split_Context const &splitinfo,
                                 OpRef const &Act, OpRef const &weights,
                                 OpRef const &stride);

// :::EXTERNAL_SHAPEFN::: {  qshape conv_split_size_valid_dil(split,op,op,op,op); }

/// @brief make 'size' shape for splitting input to 'valid' dilated convolution, where the input is being split along height
///
/// Generates shape {0, inrows, 0, 0 }
///
/// where inrows = stride_h * (SPLIT_SIZE-1) + (filter_h - 1) * dilation + 1
///
QuickShape conv_split_size_valid_dil(Replacement &rpx,
                                     Split_Context const &splitinfo,
                                     OpRef const &Act, OpRef const &weights,
                                     OpRef const &stride,
                                     OpRef const &dilation);

// :::EXTERNAL_SHAPEFN::: {  qshape pool_split_start_valid(split,op,op,op); }

/// @brief make 'start' shape for splitting input to 'valid' Xpool, where the input is being split along height
///
/// Generates shape {0, SPLIT_START * stride_h, 0, 0 }
QuickShape pool_split_start_valid(Replacement &rpx,
                                  Split_Context const &splitinfo,
                                  OpRef const &Act, OpRef const &window,
                                  OpRef const &stride);

// :::EXTERNAL_SHAPEFN::: {  qshape pool_split_size_valid(split,op,op,op); }

/// @brief make 'size' shape for splitting input to 'valid' Xpool, where the input is being split along height
///
/// Generates shape {0, inrows, 0, 0 }
///
/// where inrows = stride_h * (SPLIT_SIZE-1) + window_h
///
QuickShape pool_split_size_valid(Replacement &rpx,
                                 Split_Context const &splitinfo,
                                 OpRef const &Act, OpRef const &window,
                                 OpRef const &stride);

/** @} */

namespace optim_extfunc { // in concat_opt.cc
QuickShape offset_into_concat(Replacement &rpx, Split_Context const &splitinfo,
                              OpRef const &concat, OpRef const &base_shape);
}

/**
 * \defgroup ShapeFnApply  Functions for SHAPEFN_APPLY
 * \ingroup OptReplacement
 *
 * These are functions which may be used with SHAPEFN_APPLY.
 *
 * The first parameter is always Replacement &; the remaining parameters are obtained from the SHAPEFN_APPLY, and may be
 * OpRef (mapped from "OperandTag" in the SHAPEFN_APPLY), or scalar values.
 *
 * The return value may be an OpRef representing a new graph object; instead, the function may return a QuickShape object
 * representing a shape, and the framework will convert this to an OpDef_Shape.
 *
 * @{
 */

// :::EXTERNAL_SHAPEFN::: {  qshape split_merge_start(op,op); }

QuickShape split_merge_start(Replacement &rpx, OpRef const &inner,
                             OpRef const &outer);

//@brief Create shape with extra amount added along some axis
// :::EXTERNAL_SHAPEFN::: { qshape shape_add_on_axis(op,op,int); }
QuickShape shape_add_on_axis(Replacement &rpx, OpRef const &start,
                             OpRef const &amt, int axis);

// :::EXTERNAL_SHAPEFN::: {  qshape conv_same_padded_size(op,op,op); }
///@brief find padded shape for input to 'same' convolution
///
/// For a 'same' convolution, produce a shape the same as 'Act', but expanded in H and W dimensions to allow for the
/// padding needed (as determined by the given filter shape and stride)
///
QuickShape conv_same_padded_size(Replacement &rpx, OpRef const &Act,
                                 OpRef const &weights, OpRef const &stride);

// :::EXTERNAL_SHAPEFN::: {  qshape conv_same_before(op,op,op); }

///@brief find padded offset (top/left margin) for input to 'same' convolution
///
/// For a 'same' convolution, produce a shape which indicates how the input needs to be padded on top and left to
/// be processed as 'valid' - as determined by the given filter shape and stride. The resulting shape will be
///
///   { 0, top_padding,  left_padding, 0 }
///
QuickShape conv_same_before(Replacement &rpx, OpRef const &Act,
                            OpRef const &weights, OpRef const &stride);

// :::EXTERNAL_SHAPEFN::: {  qshape pool_same_padded_size(op,op,op); }

///@brief find padded shape for input to 'same' Xpool
///
/// For a 'same' Xpool, produce a shape the same as 'Act', but expanded in H and W dimensions to allow for the
/// padding needed (as determined by the given window shape and stride)
///
QuickShape pool_same_padded_size(Replacement &rpx, OpRef const &Act,
                                 OpRef const &window, OpRef const &stride);

// :::EXTERNAL_SHAPEFN::: {  qshape pool_same_before(op,op,op); }

///@brief find padded offset (top/left margin) for input to 'same' Xpool
///
/// For a 'same' Xpool, produce a shape which indicates how the input needs to be padded on top and left to
/// be processed as 'valid' - as determined by the given window shape and stride. The resulting shape will be
///
///   { 0, top_padding,  left_padding, 0 }
///
QuickShape pool_same_before(Replacement &rpx, OpRef const &Act,
                            OpRef const &window, OpRef const &stride);

// :::EXTERNAL_SHAPEFN::: {  qshape pad_total_for_qnn(op,op); }

///@brief use input pad_amount to calculate padded offset for input from 'QNN_Conv' to 'valid' convolution
///
/// For a 'QNN_Conv' convolution, produce a shape the same as 'Act', but expanded in H and W dimensions, which
/// is determined by the input pad_amount: [[h_before, h_after], [w_before, w_after]]
///
QuickShape pad_total_for_qnn(Replacement &rpx, OpRef const &Act,
                             OpRef const &pad_amount);

///@brief use input pad_amount to calculate padded offset for input to use 'valid' pooling
///
/// For QNN pool ops, produce a shape the same as 'Act', but expanded in H and W dimensions, which
/// is determined by the input pad_amount: [[h_before, h_after], [w_before, w_after]]
///
QuickShape pad_total_for_qnn_round(Replacement &rpx, OpRef const &Act,
                                   OpRef const &Stride, OpRef const &pad_amount,
                                   OpRef const &rounding_mode);

// :::EXTERNAL_SHAPEFN::: {  qshape pad_before_for_qnn(op,op); }

///@brief use input pad_amount to get the (top/left margin) for input from 'QNN_Conv' to 'valid' convolution
///
/// For a 'QNN_Conv' convolution, produce the result shape of padded shape to
/// be processed as 'valid' - as determined by the given pad_amount. The resulting shape will be
///
///   { 0, top_padding,  left_padding, 0 }
///
QuickShape pad_before_for_qnn(Replacement &rpx, OpRef const &Act,
                              OpRef const &pad_amount);

// :::EXTERNAL_SHAPEFN::: {  qshape explicit_pad_for_qnn(op,op); }

OpRef explicit_pad_for_qnn(Replacement &rpx, OpRef const &output,
                           OpRef const &pad_amount);

// :::EXTERNAL_SHAPEFN::: {  qshape reshape_hw_to_4d(op); }

///@brief given a tensor representing [h, w] expand to [1, h, w, 1]
QuickShape reshape_hw_to_4d(Replacement &rpx, OpRef const &stride);

// :::EXTERNAL_SHAPEFN::: {  qshape reshape_bhw_to_4d(op); }

///@brief given a tensor representing [b, h, w] expand to [b, h, w, 1]
QuickShape reshape_bhw_to_4d(Replacement &rpx, OpRef const &stride);

// :::EXTERNAL_SHAPEFN::: {  qshape shape_after_transpose(op,op); }

///@brief gives the new shape of a tensor after a transpose has been applied to it
QuickShape shape_after_transpose(Replacement &rpx, OpRef const &input,
                                 OpRef const &tx_control);

// :::EXTERNAL_SHAPEFN::: {  qshape shape_after_spaceToBatch(op,op); }
// :::EXTERNAL_SHAPEFN::: {  qshape shape_after_spaceToBatch_w_pad(op,op,op); }

///@brief gives the new shape of a tensor after a SpaceToBatch transformation
QuickShape shape_after_spaceToBatch(Replacement &rpx, OpRef const &input,
                                    OpRef const &block_size);
QuickShape shape_after_spaceToBatch_w_pad(Replacement &rpx, OpRef const &input,
                                          OpRef const &block_size,
                                          OpRef const &pads);

// :::EXTERNAL_SHAPEFN::: {  qshape shape_after_depthToSpace(op,op); }

///@brief gives the new shape after depthToSpace transformation
QuickShape shape_after_depthToSpace(Replacement &rpx, OpRef const &input,
                                    OpRef const &block_size);

// :::EXTERNAL_SHAPEFN::: {  qshape before_pad_shape(op); }

///@brief extracts before pads from pad tensor
QuickShape before_pad_shape(Replacement &rpx, OpRef const &input,
                            OpRef const &padding);
/** @} */

// :::EXTERNAL_SHAPEFN::: {  qshape gen_null_shape(op); }

///@brief generate a shape of all 0s with same rank as input
QuickShape gen_null_shape(Replacement &rpx, OpRef const &input);
/** @} */

// :::EXTERNAL_SHAPEFN::: {  qshape shape_after_pad(op,op); }

///@brief gives the new shape after pad applied
QuickShape shape_after_pad(Replacement &rpx, OpRef const &input,
                           OpRef const &padding);
/** @} */

/////////////////////////////////////////////////////////////
// Given a match rule like
//
//  Op("Add","X","B"),
// or
//  Op("Slice_shape",Op("Slice_shape","Input","inner_start","inner_size"),"outer_start","outer_size"),
//
// .. we make an MatchOp object that can match it and bind the named parameters
//
// This is done in two steps
//  (1) first, we 'parse' the rule, this is done by executing the code in the context of a MatchBuilder
//      member function. Each Op() returns a shared_ptr<MatchAstNode>.
//  (2) we look at that, and based on what it is, we construct something based on MatchOpBase,
//      which has a method in it to do the matching.
//      The return from (1) can then be discarded
//
//
// during matching:
///  - The matcher engine works by first checking all the Op types and parameter counts (in pre-order
//     traversal), and then going back to bind/check the named operands, all  by following a table.
//     There are no "operand_tag_t" involved in this process, since the indices into the output array
//     are baked into the tables in advance.
//   - in the process, all the OpRef  stored in an array of at most [MATCH_MAX_PATTERN], which is
//     stored in the Match object. The first will contain the 'root' Opref, the next 'n' are the matched subops,
//     and the rest are distinct matched parameter names. 'n' could be 0.
//   - Each instance of MatchOpBase has an array std::vector<pair<operand_tag_t,int>> m_operindex, which supplies
//     the 'operand' names for those matched refs, and maps them to indices in the array (the names are in order).
//   - On a complete match we use Match.set_current_matchop() to install a pointer to the MatchOpBase;
//     subsequently, in 'Constraint' and 'Replace' phases, that object's lookup_opertag() method is used to
//     map operand names to OpRef (it maps the m_operindex to an index into the array).
// TODO: we should probably also have a parallel array in Match of the corresponding OpDef pointers, which
// can be filled in lazily (starting with the ones already obtained during matching, with the rest init to NULL).
// This could reduce repeated lookups in op_def_map during Constraint and Replace phases.
//
//

#define OP_CSTR(op) ((op).c_str())

//
// The subclasses of MatchOpBase are declared and implemented in match_op.cc
//
// MatchOpBase
//      +---MatchOpSimple		    // for 1-level pattern with no duplicate operand names
//      +---MatchOpGeneral			// for all other cases.
//
//
namespace hnnx {

class MatchOpBase;
// These are the state vars within Match which belong to MatchOp.
struct MatchOpState {
    MatchOpBase const *
            current_matchop; // after a match, points to the matchop, which does operand lookups.
    // table of OpRef bound by the match; current_matchop->lookup_opertag is used to find the index
    // for a given operand tag.
    // (only the first 'n' are valid, where n is current_matchop->m_matchcount)
    OpRef bound_opref[MATCH_MAX_PATTERN];
    // These are either null or pointing to the OpDef indicated by bound_opref[i]
    // (only the first 'n' are valid, where n is current_matchop->m_matchcount)
    OpDef const *bound_opdef[MATCH_MAX_PATTERN];

    int lookup_opertag(operand_tag_parm_t optag) const;
};

//
class MatchOpBase {
  protected:
    opname_tag_t m_opname0; // name of the root op.
    // 0 <= min <= max
    unsigned short m_min_inputs; // range of input counts on the root op
    unsigned short m_max_inputs;
    unsigned short m_matchcount; // size of table needed for match

    // A fixed list, mapping operand tags to indices in the mapped operands;
    // sorted by operand tag.
    // This is used in lookup_opertag()
    std::vector<std::pair<operand_tag_t, int>> m_operindex;
    //
    // This contains the char const * used for displaying the context
    // (see optim_trace.cc)
    // It may be empty, if this was not enabled in the build.
    std::unique_ptr<const char[]> match_debug_desc;

    virtual bool do_subclass_match(Match &m, OpDef const &op) const = 0;

    MatchOpBase(MatchAstNode const *, int matchcount,
                std::vector<std::pair<operand_tag_t, int>> &&operindex);

    static MatchOpState &matchop_state(Match &m);
    // lookup_ref:   transform an OpRef to OpDef using the methods in Match
    OpDef const &lookup_ref(Match &m, OpRef const &op) const;

    // OpRef to the matched pattern Ops are stored in a linear array,
    // with [0] being the 'base' Op.
    //  For MatchOpSimple, the rest of the array is filled up with the Op's inputs.
    //  For MatchOpGeneral, starting in [1] the array is filled with refs
    //   to all the 'subordinate' Ops (in pre-order). There may be 0 of these.
    //   The rest of the array is filled with OpRef to he named input operands.
    //
    // A table of opdesc is used to match and gather the 'Ops'in the table. Results
    // are stored in order in the match list, starting at [1]
    struct opdesc {
        opname_tag_t opname; // name of the sub op
        unsigned short
                loc_idx; // index of previously matched containing op, in match table
        unsigned short in_idx; // which input do we look at
        unsigned short min_n, max_n; // range of input count
    };
    // parm desc are used to gather the 'named' params
    // results are stored in order in match table.
    // Records with dup_ipx >0 are different: for these, the operand is
    // obtained, and checked to see if it's a dup of the one already at
    // dup_index. If it is not, the match fails; if it is, the matching
    // proceeds, and nothing is added to the output (note, it is not allowed
    // or useful to have an operand aliased to the root op, index 0).
    //
    struct parmdesc {
        unsigned short loc_idx; // index of previously matched op in match table
        unsigned short in_idx; // which input do we look at
        unsigned short dup_idx; // if !=0, must be a dup of dup_index-1
    };

  public:
    // this returns the m_matchcount; can be used for auto-sizing the bind array in match.
    // it needs to be at least as large as the get_mathcount of all the MatchOp.
    int get_matchcount() const { return m_matchcount; }
    // this builds a MatchOp of appropriate class from a MatchAst
    static MatchOp_uptr build_MatchOp(MatchAstNode *);

    virtual ~MatchOpBase();
    bool do_match(Match &m, OpDef const &op) const;
    // lookup an operand tag in m_operindex
    // Returns -1 if not found, or the index (will be in range 0..get_matchcount()-1)
    //
    int lookup_opertag(operand_tag_parm_t optag) const;
    // this is so we can organize rules based on the root opname.
    opname_tag_parm_t get_root_opname() const { return m_opname0; }

    // these are used for WITH_OPT_DEBUG. When it is not defined. they return nullptr and empty-map.
    char const *get_debug_desc() const
    {
        return match_debug_desc.get();
    } // may return nullptr
    std::map<OpId, operand_tag_parm_t>
    get_inverse_map(MatchOpState const &m) const;

    const std::vector<std::pair<operand_tag_t, int>> &get_operindex() const
    {
        return m_operindex;
    };
};

static void fail_lookup(operand_tag_parm_t optag)
{
    errlog("Parameter %s not found", optag.c_str());
    throw std::runtime_error("match parm not found");
}

inline int MatchOpState::lookup_opertag(operand_tag_parm_t optag) const
{
    int idx = current_matchop->lookup_opertag(optag);
    if (idx < 0)
        fail_lookup(optag);
    return idx;
}

/////////////////////////////////////////////////////////////

/** \defgroup OptMatch Match-Pattern Expressions for Optimization Rules
 * \ingroup OptimizationFuncs
 *
 * These are the operations available for writing 'Match Pattern' expressions.
 */

/////////////////////////////////////////////////////////////

/** Base Class for Graph Optimization Context
 * This has the shared data elements and functionality, available to all parts of the optimization
 */

class GraphOptContext_Base : public RefersToGraph {
  protected:
    GraphOptContext_Base(Graph &g) : RefersToGraph(g) {}
};

// this is a virtual base class which is used to implement MESSAGE dumps
// while running optimization; it abstracts away the difference between
// 'built-in' optimizations, and externally generated, via two different
// subclasses

class OptDebugBase {
  protected:
    Graph &m_graph;
    uint32_t m_saved_opid;
    OptDebugBase(Graph &g) : m_graph(g), m_saved_opid(0) {}
    OptDebugBase(Graph &g, uint32_t saved_opid)
        : m_graph(g), m_saved_opid(saved_opid)
    {
    }

  public:
    Graph &graph() const { return m_graph; }
    // these are stubs unless WITH_OPT_DEBUG is #defined
    void show_optim(FILE *f, int indent); // show what a rule has matched
    void show_optim_replace(FILE *f, OpId opid, int indent);
    virtual ~OptDebugBase();

  protected:
    // these are used by show_optim, show_optim_replace to access the match context

    virtual char const *get_debug_desc()
            const = 0; // get the 'matchdesc' string for current optimization
    // get an OpRef of an op which is in the pattern at 'idx'
    virtual OpRef get_bound_opref(unsigned idx) const = 0;
    // get an OpDef * to to an op which is in the pattern at 'idx'
    virtual OpDef const *get_bound_opdef(unsigned idx) const = 0;

  public:
    virtual uint32_t saved_opid() const { return m_saved_opid; }

    // get mapping from OpId->parm for all OpId in the match pattern; this is used
    // to show the replacement pattern.
    using id_to_parmname_map = std::map<OpId, operand_tag_parm_t>;
    virtual id_to_parmname_map get_id_to_parmname_map() const = 0;
    virtual std::string get_debug_filepos() const = 0;
};

/*
 * The Match class contains the functionality for the match functions
 * to implement pattern matching
 *
 * We want to write something like:
 *  Op("Relu",Op("ConvLayer","Act","Weights","Bias","Stride"))
 * Where the first parameter is the name of an operation
 * And the rest of the strings are names that match an input that we can use to
 * refer to the input
 *
 * We need to refer to inputs again even in matching: if we see the same string
 * twice it needs to be the same thing in both places.
 * 
 * But primarily we will need to use these strings while during extra constraints
 * and replacement.
 */

class GraphOptInfo;

class Match : public GraphOptContext_Base {
    friend class GraphOptInfo;
    friend class MatchOpBase;
    friend class OptDebugForMatch;

  protected:
    OptimFilter optim_filter; // used for WITH_OPT_DEBUG; empty otherwise
    MatchOpState matchop_state;
    bool pending_show_replacement;
    GraphOptInfo const *curr_rule_info = 0; // only used in WITH_OPT_DEBUG

    // op_id_counter is saved here before 'replace'; after replace, any
    // OpId which are >= this in the upper 32 bits are 'new'.
    uint32_t save_op_id_counter;

    // optim config vars are set here.
    optim_config_values config_vars;

    Match(Graph &g) : GraphOptContext_Base(g), optim_filter(g)
    {
        set_config_vars();
    }
    void set_config_vars();

    // these are debug hooks; they are defined later as inlines
    void constraint_begin(GraphOptInfo const &);
    void replacement_fail();
    void replacement_succeed(OpId newop);

  public:
    // this can be used to test whether an OpId was created since the replacement
    // rule started (though, not at all reliable for 'OpDef_ConstBase' ops).
    inline bool opid_is_new(OpId op) const
    {
        return uint32_t(op >> 32) >= save_op_id_counter;
    }
    //template<typename UniqueType> bool match(OpDef &base);
    typedef MatchAst_uptr (*matchbuilder_type)();
    optim_config_values const &get_config() const { return config_vars; }
    void show_debug_message(char const *why, char const *str)
#ifndef WITH_OPT_DEBUG
    {
    }
#else
            ; // defined in optimize.cc
#endif
};

// these need to be defined after MatchOpBase and Match.
inline MatchOpState &MatchOpBase::matchop_state(Match &m)
{
    return m.matchop_state;
}
inline bool MatchOpBase::do_match(Match &m, OpDef const &op) const
{
    if (op.opstr != m_opname0)
        return false;
    int nin = op.n_inputs();
    if (nin < m_min_inputs || nin > m_max_inputs)
        return false;
    bool res = do_subclass_match(m, op);
    m.matchop_state.current_matchop = res ? this : nullptr;
    return res;
}

// Subclass of OptDebugBase for use with Match
class OptDebugForMatch : public OptDebugBase {
  protected:
    Match const &m_match;

  public:
    OptDebugForMatch(Match const &m)
        : OptDebugBase(m.graph(), m.save_op_id_counter), m_match(m)
    {
    }
    virtual ~OptDebugForMatch() override;
    virtual std::string get_debug_filepos() const override;

  protected:
    virtual char const *get_debug_desc() const override;
    // get an OpRef of an op which is in the pattern at 'idx'
    virtual OpRef get_bound_opref(unsigned idx) const override;
    // get an OpDef * to to an op which is in the pattern at 'idx'
    virtual OpDef const *get_bound_opdef(unsigned idx) const override;
    virtual id_to_parmname_map get_id_to_parmname_map() const override;
};

// define these debug hooks
#ifndef WITH_OPT_DEBUG
inline void Match::constraint_begin(GraphOptInfo const &) {}
inline void Match::replacement_fail() {}
inline void Match::replacement_succeed(OpId newop) {}

#else

inline void Match::constraint_begin(GraphOptInfo const &grinfo)
{
    pending_show_replacement = false;
    curr_rule_info = &grinfo;
}
inline void Match::replacement_fail() {}
// Match::replacement_succeed(OpId newop) is in optimize.cc
#endif

} // namespace hnnx

namespace oExp {
class opdef_accessor;
}
/*
 * Constraints are an expression that can inspect a matched pattern
 * to see if the situation is actually valid
 * 
 * EXTERNAL_CONSTRAINT is a hook that can be used to write your own constraint functions.
 */

namespace constraint_lib {

class Constraint : public hnnx::Match {
    friend class oExp::opdef_accessor;

  protected:
    Constraint(Graph &g) : Match(g) {}
    /* We can put arithmetic functions in a separate library, but we want the namespace here. */
    /* Functions that need things like the context to evaluate should probably go here */
    OpRef get_opref(hnnx::operand_tag_parm_t param_name) const
    {
        int idx = matchop_state.lookup_opertag(param_name);
        return matchop_state.bound_opref[idx];
    }

  private:
    const OpDef &get_opdef_from_idx(int idx)
    {
        OpDef const *odp = matchop_state.bound_opdef[idx];
        if (odp == nullptr) {
            odp = &matchop_state.bound_opref[idx].dereference(this);
            matchop_state.bound_opdef[idx] = odp;
        }
        return *odp;
    }
    const OpDef &get_opdef(hnnx::operand_tag_parm_t param_name)
    {
        return get_opdef_from_idx(matchop_state.lookup_opertag(param_name));
    }
    const OutputDef &get_outdef(hnnx::operand_tag_parm_t param_name)
    {
        int idx = matchop_state.lookup_opertag(param_name);
        OpDef const &def = get_opdef_from_idx(idx);
        return def.get_outputdef();
    }
    // this method is used by oExp::opdef_accessor; the definition
    // is in oexpr.cc (it can't be inlined here because it needs Graph).
    OpDef const &lookup_opdef(OpId oid) const;

  public:
    template <typename UniqueType> static ReplFuncBool constraint();
    typedef ReplFuncBool (*constraintfn_type)();
};

} // namespace constraint_lib

using Constraint = constraint_lib::Constraint;

/** \defgroup OptReplacement Replacement-Rule Expressions for Optimization Rules
 * \ingroup OptimizationFuncs
 *
 * These are the operations available for writing 'Replacement Rule' expressions. Certain of these
 * accept scalar inputs; for these you can use constant values, or 'constraint' expressions.
 *
 * Note: the operations in this group which appear to return a graph element ( Op, gen_Shape, etc)
 * actually return a ReplFunc, which is a std::function that is called to generate the graph element.
 *
 * Likewise, SPLIT_START, SPLIT_SIZE, SPLIT_DIM actually return ReplFuncInt, a std::function which is called
 * to generate the integer result, which changes as the autosplit is iterated.
 */

/*
 * The Replacement generates the new pattern.
 * 
 * EJP: FIXME: maybe we can make things simpler here....
 * 
 * Once we've passed the Match and Constraint phase, we want to generate a new 
 * set of Ops to replace the sequence.
 *
 * We use the same Op() syntax to generate new things, we use "strings" to 
 * refer to matched items, and things typically work nicely.
 *
 * Well, sometimes anyway.
 * 
 * It's common to want to do things like slicing, where we want to generate 
 * lots of ops... so adding some extra things to be able to slice into multiple
 * things and concatenate them is helpful. 
 * 
 * But when we try to do that, we run into problems where the items in the 
 * dictionary are evaluated before we put them in.  So we do a lot of work 
 * with these deferred std::function returns.  Then we just copy what woks
 * to do it again... but I think it might be wasteful.
 *
 * As we're generating these new ops, we start off with the output definition
 * of the thing we're replacing.  That works fine for doing a simple substitution
 * like Op(Relu,Op(ConvLayer,Act,W,B,S)) --> Op(ConvLayer_relu,Act,W,B,S)
 * But if you want to (for example) split weights or pad activations, you need
 * to change the sizes of inputs, not just keep inheriting the output's output def.
 *
 * So we have this WITH_SIZE and friends, but there's probably a better 
 * system that we could concieve of.
 * 
 * Beyond that, it seems like a lot of the size / quant parameter / slicing 
 * code might be kind of common, so maybe some more library code that hides the
 * ugliness is good enough to make the common cases simple.
 *
 */
class Replacement : public Constraint {
    friend class gxE::GXEngine;

  protected:
    OpDef const *m_curr_op; // used as id reference in 'APPLY'
    static std::string pkg_flag;
    Replacement(Graph &g) : Constraint(g), m_curr_op(NULL) {}

  public:
    OpRef do_replacement(const OpDef &oldop, ReplFunc const &replace_func)
    {
        return replace_func(*this, oldop);
    }
    Split_Context const &
    lookup_split(hnnx::split_context_tag_t const &tag) const
    {
        return split_context.at(tag);
    }

  private:
    std::map<hnnx::split_context_tag_t, Split_Context> split_context;

    // apply_param_adapter is a gasket for parameters to SHAPEFN_APPLY and similar:
    //   int, size_t, float, dtype -> same
    //   OpRef -> same;
    //   operand_tag -> lookup OpRef;
    //   ReplFunc -> call it to get OpRef.
    inline int apply_param_adapter(const OpDef &base, int val) { return val; }
    inline size_t apply_param_adapter(const OpDef &base, size_t val)
    {
        return val;
    }
    inline float apply_param_adapter(const OpDef &base, float val)
    {
        return val;
    }
    inline DType apply_param_adapter(const OpDef &base, DType val)
    {
        return val;
    }
    inline OpRef apply_param_adapter(const OpDef &base,
                                     hnnx::operand_tag_parm_t str)
    {
        return get_opref(str);
    }
    inline OpRef apply_param_adapter(const OpDef &base, OpRef ref)
    {
        return ref;
    }
    inline OpRef apply_param_adapter(const OpDef &base, ReplFunc const &f)
    {
        return f(*this, base);
    }

    template <oExp::Variant V, typename T>
    inline auto apply_param_adapter(const OpDef &base,
                                    oExp::expr<V, T> const &expn)
    {
        return expn.eval(*this);
    }
    template <oExp::OpVnt V, typename T>
    inline OpRef apply_param_adapter(const OpDef &base,
                                     oExp::opexpr<V, T> const &expn)
    {
        return expn.eval(*this);
    }

    // 'runtime' of ResizeDim
    OpRef do_ResizeDim(OpDef const &old, int dim, int size, ReplFunc const &f,
                       bool reduce_dim = false,
                       hnnx::splithist_t const *new_splithist = nullptr);

    // A thin subclass of ReplFunc, which can be constructed from a ReplFunc, but also
    // from an opexpr<V,T>
    struct ReplFunc_general : ReplFunc {
        ReplFunc_general(ReplFunc &&f) : ReplFunc(std::move(f)) {}
        ReplFunc_general(ReplFunc_general &&src) = default;
        ReplFunc_general(ReplFunc_general const &) = default;
        template <oExp::OpVnt V, typename T>
        ReplFunc_general(oExp::opexpr<V, T> &&op)
            : ReplFunc(hnnx::wrap_as_replfunc(op))
        {
        }
        template <oExp::OpVnt V, typename T>
        ReplFunc_general(oExp::opexpr<V, T> const &op)
            : ReplFunc(hnnx::wrap_as_replfunc(op))
        {
        }
    };
    // A thin subclass of ReplFunc, which can be constructed from a ReplFunc, but also
    // from an operand tag, or string (via Operand()), or a fixed OpRef (this is to support OUTPUT_OF and similar)
    struct ReplFunc_or_Operand : ReplFunc {
        ReplFunc_or_Operand(ReplFunc &&f) : ReplFunc(std::move(f)) {}
        ReplFunc_or_Operand(ReplFunc_or_Operand &&src) = default;
        ReplFunc_or_Operand(ReplFunc_or_Operand const &) = default;
        ReplFunc_or_Operand(hnnx::operand_tag_parm_t str)
            : ReplFunc(Operand(str))
        {
        }
        ReplFunc_or_Operand(char const *str) : ReplFunc(Operand(str)) {}
        ReplFunc_or_Operand(OpRef const &);

        template <oExp::OpVnt V, typename T>
        ReplFunc_or_Operand(oExp::opexpr<V, T> &&op)
            : ReplFunc(hnnx::wrap_as_replfunc(op))
        {
        }
        template <oExp::OpVnt V, typename T>
        ReplFunc_or_Operand(oExp::opexpr<V, T> const &op)
            : ReplFunc(hnnx::wrap_as_replfunc(op))
        {
        }
    };

    /// \ingroup OptReplacement
    /// @brief ResizeDim(dim,size, expr) - evaluate 'expr' with a modification of reference shape
    ///
    /// The reference shape used to evaluate 'expr' is modified from the default by changing dimension 'dim' to size'
    ///
    static ReplFunc ResizeDim(int dim, int size, ReplFunc_general &&f);
    //
    // Modifiers (e.g. WITH_SIZE( ref, target )
    //  work like this:
    //     (a) evaluate the 'ref' subtree using the current reference OpDef as reference;
    //     (b) execute the modifier. This creates a temporary OpDef object, which combines
    //         attributes of the original ref object, and the one constructed from ref;
    //         e.g. WITH_SIZE takes rank and shape from 'ref' and the dtype etc from previoud ref
    //     (c) now, execute the 'target' subtree using this temporary object as the reference.
    //         The result of that is the result of the modifier. The temporary OpDef is discarded.
    //
    // this does WITH_SIZE, WITH_TYPE, WITH_SAME_OUTPUT
    static const int mode_with_size = 1;
    static const int mode_with_type = 2;
    static const int mode_with_same_output = mode_with_size | mode_with_type;

    // immed_modifier does step (b) above; it makes the temp object from the ref result and the current opdef
    // The lambda inside WITH_output_like does steps (a) and (c).
    //
    OpDef immed_modifier(OpRef const &ref, OpDef const &old, int mode);

    static ReplFunc WITH_output_like(ReplFunc_or_Operand &&ref, ReplFunc &&f,
                                     int mode);

    OpDef immed_modifier_OPID(OpRef const &ref, OpDef const &old);

    /// \ingroup OptReplacement
    /// @brief WITH_SAME_ID(refexp, expr) - evaluate 'expr' using 'refexp' for the reference opid
    inline static ReplFunc __attribute__((visibility("hidden")))
    WITH_SAME_ID(ReplFunc_or_Operand &&ref, ReplFunc_general &&f)
    {
        return ReplFunc::create(
                [=](Replacement &rpx, const OpDef &old) -> OpRef {
                    OpDef new_def = rpx.immed_modifier_OPID(ref(rpx, old), old);
                    return f(rpx, new_def);
                });
    }

    
    /// \ingroup OptReplacement
    /// @brief WITH_SPLIT_HISTORY(refexp, expr) - evaluate 'expr' using 'refexp' for the split history
    inline static ReplFunc __attribute__((visibility("hidden")))
    WITH_SPLIT_HISTORY(ReplFunc_or_Operand &&ref, ReplFunc_general &&f)
    {
        return ReplFunc::create(
                [=](Replacement &rpx, const OpDef &old) -> OpRef {
                    OpRef new_id = f(rpx, old);
                    OpDef & new_def = new_id.dereference(rpx.graph());
                    OpRef ref_id = ref(rpx,old);
                    new_def.set_splithist(ref_id.dereference(rpx.graph()).get_splithist());
                    return new_id;
                });
    }

    void do_SPLIT_HISTORY(const OpDef & Src, int dim, OpDef & expr);

    /// \ingroup OptReplacement
    /// @brief WITH_SPLIT_HISTORY(refexp, dim, chunksize, expr) - evaluate 'expr' using 'refexp' for the split history
    // 
    // Add a new entry to split history table using refexp as the parent, dim as dimension.
    // expr is expected to be a Concat or a InstanceNorm.SumAndSquares_TileReduce.
    // The number of splits is determined by the number of children of expr
    // The chunksize is determined by the first non-constant child
    inline static ReplFunc __attribute__((visibility("hidden")))
    WITH_SPLIT_HISTORY(ReplFunc_or_Operand &&ref, int dim, ReplFunc_general &&f)
    {
        return ReplFunc::create(
                [=](Replacement &rpx, const OpDef &old) -> OpRef {
                    OpRef ref_id = ref(rpx,old);
                    OpRef new_id = f(rpx, old);
                    rpx.do_SPLIT_HISTORY(ref_id.dereference(rpx), dim,  new_id.dereference(rpx));
                    return new_id;
                });
    }

    /// \ingroup OptReplacement
    /// @brief WITH_SIZE(refexp, expr) - evaluate 'expr' using 'refexp' for the reference output size
    inline static ReplFunc __attribute__((visibility("hidden")))
    WITH_SIZE(ReplFunc_or_Operand &&shape, ReplFunc_general &&f)
    {
        return WITH_output_like(std::move(shape), std::move(f), mode_with_size);
    }
    /// \ingroup OptReplacement
    /// @brief WITH_TYPE(refexp, expr) - evaluate 'expr' using 'refexp' for the reference output type
    inline static ReplFunc __attribute__((visibility("hidden")))
    WITH_TYPE(ReplFunc_or_Operand &&type, ReplFunc_general &&f)
    {
        return WITH_output_like(std::move(type), std::move(f), mode_with_type);
    }
    /// \ingroup OptReplacement
    /// @brief WITH_SAME_OUTPUT(refexp, expr) - evaluate 'expr' using 'refexp' for the reference output type and size
    inline static ReplFunc __attribute__((visibility("hidden")))
    WITH_SAME_OUTPUT(ReplFunc_or_Operand &&ref, ReplFunc_general &&f)
    {
        return WITH_output_like(std::move(ref), std::move(f),
                                mode_with_type | mode_with_size);
    }
    OpRef immed_gen_ShapeOf(OpRef const &shaperef, OpDef const &old);
    /// \ingroup OptReplacement
    /// @brief gen_ShapeOf(any_oper) - Construct an OpDef_Shape with the shape taken from the given graph operation.
    static ReplFunc gen_ShapeOf(ReplFunc_or_Operand &&shape);

    static inline OpDef immed_modifier_OUTPUT_TYPE(OpDef const &old,
                                                   DType dtype,
                                                   int32_t zero_offset,
                                                   float stepsize)
    {
        OutputDef temp{};
        temp.dtype = dtype;
        temp.zero_offset = zero_offset;
        temp.stepsize = stepsize;
        return old.make_output_exemplar(nullptr, &temp);
    }

    /// \ingroup OptReplacement
    /// @brief WITH_SIZE(dtype,zero_offset,stepsize, expr) - evaluate 'expr' but using the specified output type.
    ///
    /// A temporary reference is created which specifies the given dtype, step, and offset instead of the
    /// default; this is used to evaluate 'expr'. If the dtype is not quantized, use 0 and 1.0f for zero_offset and stepsize.
    static ReplFunc WITH_OUTPUT_TYPE(DType dtype, int32_t zero_offset,
                                     float stepsize, ReplFunc_general &&f);

    // same thing, where the inputs are all function objects...

    static ReplFunc WITH_OUTPUT_TYPE_func(ReplFuncDType &&dtype,
                                          ReplFuncInt &&zero_offset,
                                          ReplFuncFloat &&stepsize,
                                          ReplFunc &&f);

    // adapter to allow  WITH_OUTPUT_TYPE to be called with some mixture of literals and oExp, and have them all converted to
    // function objects, which are forwarded to WITH_OUTPUT_TYPE_func
    template <typename TDT, typename TZO, typename TSS>
    static inline ReplFunc WITH_OUTPUT_TYPE(TDT &&dtype, TZO &&zero_offset,
                                            TSS &&stepsize,
                                            ReplFunc_general &&f)
    {
        return WITH_OUTPUT_TYPE_func(
                oExp::wrap_as_function<DType>(std::forward<TDT>(dtype)),
                oExp::wrap_as_function<int>(std::forward<TZO>(zero_offset)),
                oExp::wrap_as_function<float>(std::forward<TSS>(stepsize)),
                std::move(f));
    }

    /// \ingroup OptReplacement
    /// @brief WITH_MULT_OUT(int num_outputs, expr) - evaluate 'expr' with 'DType::Multi' for 'num_outputs' outputs
    ///
    /// A temporary reference is created with OutputDef configured to make an Multi-Output op with the given number
    /// of outputs. This is used to evaluate 'expr'. num_outputs must be >=2.
    ///
    static ReplFunc WITH_MULTI_OUT(unsigned num_outputs, ReplFunc_general &&f);

    /// immed_WITH_MULTI_OUT makes the OpDef used in WITH_MULTI_OUT.
    static OpDef immed_WITH_MULTI_OUT(OpDef const &old, unsigned num_outputs);

    static OpRef shapefn_adapt_result(const OpDef &old, OpRef const &inp)
    {
        return inp;
    };
    static OpRef shapefn_adapt_result(const OpDef &old, QuickShape const &inp);

    template <typename F_T, typename... Arg_Ts>
    OpRef __attribute__((visibility("hidden")))
    immed_SHAPEFN_APPLY(const OpDef &old, F_T f, Arg_Ts &&...args)
    {
        OpDef const *keep = m_curr_op;
        m_curr_op = &old;
        OpRef result = shapefn_adapt_result(
                old, f(*this, std::forward<Arg_Ts>(args)...));
        m_curr_op = keep;
        return result;
    }
    /// \ingroup OptReplacement
    /// @brief SHAPEFN_APPLY(function,parms...) - generate a shape object by calling a specified function.
    ///
    /// The named function is called, with specified parameters. These can be strings (assumed to be be operand
    /// references, and converted to OpRef), or scalar expressions.
    ///
    /// See also: \ref ShapeFnApply
    template <typename F_T, typename... Arg_Ts>
    static ReplFunc __attribute__((visibility("hidden")))
    SHAPEFN_APPLY(F_T f, Arg_Ts... args)
    {
        return ReplFunc::create(
                [=](Replacement &rpx, const OpDef &old) -> OpRef {
                    /* Call f(rpx,args...) */
                    return rpx.immed_SHAPEFN_APPLY(
                            old, f, rpx.apply_param_adapter(old, args)...);
                });
    }

    /// \ingroup OptReplacement
    /// @brief AUTOSPLIT_SHAPEFN_APPLY(function, "split_tag", parms...) - generate a shape object by calling a specified function.
    ///
    /// The named function is called, with specified parameters. These can be strings (assumed to be be operand
    /// references, and converted to OpRef), or scalar expressions.
    /// The 'split_tag' parameter is converted to a reference to a Split_Context
    ///
    /// See also: \ref AutoSplitShapeFnApply
    template <typename F_T, typename... Arg_Ts>
    static ReplFunc __attribute__((visibility("hidden")))
    AUTOSPLIT_SHAPEFN_APPLY(F_T f, hnnx::split_context_tag_t whatsplit,
                            Arg_Ts... args)
    {
        return ReplFunc::create(
                [=](Replacement &rpx, const OpDef &old) -> OpRef {
                    /* Call f(rpx,split_context.at(whatsplit),args...) */
                    return rpx.immed_SHAPEFN_APPLY(
                            old, f, rpx.split_context.at(whatsplit),
                            rpx.apply_param_adapter(old, args)...);
                });
    }
    // AUTOSPLIT_SLICE, TYPICAL_SLICE, CHANGEDIM_SLICE
    // are 'macro' operations - same effect as inserting a more
    // complex expression in the source rule.

    /// \ingroup OptReplacement
    /// @brief AUTOSPLIT_SLICE(in, start, size ) -> WITH_SIZE( size, WITH_TYPE( in, Op("Slice_shape", in, start, size)))
    ///
    /// This generates a "Slice_shape" op applied to the given input 'in', with the given 'start' and 'size' shapes. The
    /// output shape is configured to match 'size', and the output type is always the same as 'in'
    ///
    static ReplFunc AUTOSPLIT_SLICE(ReplFunc_or_Operand &&in,
                                    ReplFunc_or_Operand &&start,
                                    ReplFunc_or_Operand &&size);

    /// \ingroup OptReplacement
    /// @brief Create a slice of an autosplit via a simple split along a dimension.
    ///
    /// This does an 'AUTOSPLIT_SLICE' where the size and start are calculated by
    /// simple_split_start, and simple_split_size, i.e. the split is done exactly as the output split,
    /// in the same dimension, with no overlap.
    ///
    /// Equivalent to the following:
    ///
    ///     TYPICAL_SLICE(in, "tag" ) ->
    ///       AUTOSPLIT_SLICE( in,
    ///          AUTOSPLIT_SHAPEFN_APPLY( simple_split_start, tag, in ),
    ///          AUTOSPLIT_SHAPEFN_APPLY( simple_split_size, tag, in ))
    static ReplFunc TYPICAL_SLICE(ReplFunc_or_Operand &&in,
                                  hnnx::split_context_tag_t whatsplit);

    /// \ingroup OptReplacement
    /// @brief Create a slice of an autosplit
    ///
    /// This does an 'AUTOSPLIT_SLICE' where the size and start are calculated by
    /// simpledim_split_start, and simpledim_split_size, i.e. the split is done as the output split
    /// with no overlap, but it may be applied to a different axis than that specified
    /// in the AUTOSPLIT.
    ///
    /// Equivalent to the following:
    ///
    ///     CHANGEDIM_SLICE(in, "tag", int newdim ) ->
    ///       AUTOSPLIT_SLICE( in,
    ///          AUTOSPLIT_SHAPEFN_APPLY( simpledim_split_start, tag, in, newdim ),
    ///          AUTOSPLIT_SHAPEFN_APPLY( simpledim_split_size, tag, in, newdim ))
    static ReplFunc CHANGEDIM_SLICE(ReplFunc_or_Operand &&in,
                                    hnnx::split_context_tag_t whatsplit,
                                    int newdim);

    // Pretty much all TYPICAL_SLICE and CHANGEDIM_SLICE are just ("string", "string") .. so this wrapper
    // will save some code space at the call sites.
    static ReplFunc CHANGEDIM_SLICE(char const *in_parm, char const *whatsplit,
                                    int newdim);
    static ReplFunc TYPICAL_SLICE(char const *in_parm, char const *whatsplit)
    {
        return CHANGEDIM_SLICE(in_parm, whatsplit, -1);
    }

    // this actually implements TYPICAL_SLICE (with newdim=-1) and CHANGEDIM_SLICE (with newdim >=0)
    OpRef do_TYPICAL_SLICE(OpDef const &old, OpRef input_op,
                           hnnx::split_context_tag_t whatsplit, int newdim,
                           bool reduce_dim = false);

    OpRef do_AUTOSPLIT(OpDef const &old, int dim, Split_Context &splitinfo,
                       int chunksize, ReplFunc const &f,
                       bool reduce_dim = false);

    /// \ingroup OptReplacement
    /// @brief Expand an expression by splitting on some dimension.
    ///
    /// AUTOSPLIT( dim, "tag", size,  <repl_expression> ) causes the operation to be split into
    /// slices along dimension dim, with each slice being of 'size' (or possibly smaller, for the last one).
    ///
    /// This done by
    ///
    ///   * Repeatedly evaluating the 'repl_expression', once for each slice
    ///   * Using a 'Concat' on the specified dimension to join the results.
    ///   * Within each iteration, SPLIT_START("tag") and SPLIT_SIZE"tag"), when evaluated within <repl_expression>,
    ///    will reflect the extent of the current split in the output, and thus can be used to construct the corresponding
    ///    slices of the input. SLICE_DIM("tag") will always give the value supplied to the AUTOSPLIT as 'dim'. Normally, this is all done within
    ///    functions invoked via AUTOSPLIT_SHAPEFN_APPLY.
    ///
    /// Rules with AUTOSPLIT should have a constraint to prevent them from being applied where the split dimension does not exceed size.
    ///
    /// @param dim        Dimension on which to split
    /// @param varname    A string indentifying the split context
    /// @param chunksize  The size of each slice of the output
    /// @param f          The subexpression to generate each part of the split
    static ReplFunc AUTOSPLIT(int dim, hnnx::split_context_tag_t varname,
                              int chunksize, ReplFunc_general &&f);

    static ReplFunc AUTOSPLIT_func(ReplFuncInt &&dim,
                                   hnnx::split_context_tag_t varname,
                                   ReplFuncInt &&chunksize, ReplFunc &&f);

    template <oExp::Variant V1, typename T1, oExp::Variant V2, typename T2>
    inline static ReplFunc __attribute__((visibility("hidden")))
    AUTOSPLIT(oExp::expr<V1, T1> &&dim, hnnx::split_context_tag_t varname,
              oExp::expr<V2, T2> &&chunksize, ReplFunc_general &&f)
    {
        return AUTOSPLIT_func(oExp::wrap_as_function<int>(std::move(dim)),
                              varname,
                              oExp::wrap_as_function<int>(std::move(chunksize)),
                              std::move(f));
    }
    // TODO: need a better way to do this (map 'int' or oExp which is int, to ReplFuncInt).
    template <oExp::Variant V2, typename T2>
    inline static ReplFunc __attribute__((visibility("hidden")))
    AUTOSPLIT(int dim, hnnx::split_context_tag_t varname,
              oExp::expr<V2, T2> &&chunksize, ReplFunc_general &&f)
    {
        return AUTOSPLIT_func(oExp::make_literal_sfunction<int>(dim), varname,
                              oExp::wrap_as_function<int>(std::move(chunksize)),
                              std::move(f));
    }
    template <oExp::Variant V1, typename T1>
    inline static ReplFunc __attribute__((visibility("hidden")))
    AUTOSPLIT(oExp::expr<V1, T1> &&dim, hnnx::split_context_tag_t varname,
              int chunksize, ReplFunc_general &&f)
    {
        return AUTOSPLIT_func(
                oExp::wrap_as_function<int>(std::move(dim)), varname,
                oExp::make_literal_sfunction<int>(chunksize), std::move(f));
    }

    /// AUTOSPLIT and reduce the dim
    static ReplFunc AUTOSPLIT_REDUCE(int dim, hnnx::split_context_tag_t varname,
                                     ReplFunc_general &&f);

    static ReplFunc TYPICAL_SLICE_REDUCE(ReplFunc_or_Operand &&in,
                                         hnnx::split_context_tag_t whatsplit);

    /// \ingroup OptReplacement
    /// @brief Create a multi-output Op by iterating over expression
    ///
    /// OP_ITER( op_base, "tag", lo_index, hi_index, <repl_expression> )
    ///
    /// The operation will iterate for "I" >= lo_index, < hi_index; for each value, the repl_expression
    /// is evaluated, and a new Op is created which
    ///
    ///  - has the same opstr as op_base, and the same inputs, plus additions inputs generated by
    ///    the iteration
    ///  - the OutputDef of the new op is defined by the context of the ITER_OP, and may be different
    ///    from that of the op_base.
    ///
    /// if lo_index <= hi_index, no iteration is done, and the built Op has the same inputs
    /// as op_base. Rules with OP_ITER should have a constraint to prevent them from being
    /// applied where this could be incorrect.
    ///
    /// @param op_base    'Reference' Op supplying the name and fixed inputs
    /// @param varname    A string indentifying the split context
    /// @param lo_index   the first input index
    /// @param hi_index   the last input index+1
    /// @param f          The subexpression to iterate.
    ///
    static ReplFunc OP_ITER(ReplFunc &&op_base,
                            hnnx::split_context_tag_t varname, int lo_index,
                            int hi_index, ReplFunc_general &&f);

    // same with ReplFuncInt for the index values, so they can be expressions
    static ReplFunc OP_ITER_func(ReplFunc &&op_base,
                                 hnnx::split_context_tag_t const &varname,
                                 ReplFuncInt &&lo_index, ReplFuncInt &&hi_index,
                                 ReplFunc &&f);

    // template to map expressions to ReplFuncInt
    template <typename TLO, typename THI>
    inline static ReplFunc __attribute__((visibility("hidden")))
    OP_ITER(ReplFunc &&op_base, hnnx::split_context_tag_t varname,
            TLO &&lo_index, THI &&hi_index, ReplFunc_general &&f)
    {
        return OP_ITER_func(
                std::move(op_base), varname,
                oExp::wrap_as_function<int>(std::forward<TLO>(lo_index)),
                oExp::wrap_as_function<int>(std::forward<THI>(hi_index)),
                std::move(f));
    }

    OpRef do_OP_ITER(OpDef const &old, OpDef const &base_op,
                     Split_Context &splitinfo, int lo_index, int hi_index,
                     ReplFunc const &f);

    // this is basically a SHAPEFN_APPLY for a function with no other inputs.
    // We still want to bind it into a std::function.

    /// \ingroup OptReplacement
    /// @brief Generate replacement by calling an external function
    ///
    /// The function must be: OpRef function( Replacement &, OpDef const &op);
    ///
    /// ... where 'op' is the OpDef being replaced.
    /// The return value is the OpRef of the replacement. If it's the same as the OpRef of 'op', it is assumed
    /// that the rule has no effect in this situation.
    ///
    static ReplFunc __attribute__((visibility("hidden")))
    EXTERNAL_REPLACE(external_replace_funcp f)
    {
        return ReplFunc(ReplFunc::FunctionWrapper, (void *)f);
    }

    /// \ingroup OptReplacement
    /// @brief Define a new mult-output Op, with one of its outputs
    ///
    ///  OpMultiOut( n_out, outno, "opstr", ...inputs... )
    ///  is equivalent to
    ///     Op( "$Out",  WITH_MULTI_OUT( n_out, Op("opstr", ... inputs...),
    ///        gen_Shape(0,0,n_out,outno))
    ///
    /// If any of the inputs have Op in them, they will need to have WITH_ modifiers
    /// for shape and type enclosing them.
    ///
    template <typename... Ts>
    inline static ReplFunc __attribute__((visibility("hidden")))
    OpMultiOut(unsigned n_out, unsigned outno, char const *opstr, Ts &&...ts)
    {
        assert(n_out >= 2 && outno < n_out);
        return Op("$Out",
                  WITH_MULTI_OUT(n_out, Op(opstr, std::forward<Ts>(ts)...)),
                  gen_Shape(0, 0, size_t(n_out), size_t(outno)));
    }
    /// \ingroup OptReplacement
    /// @brief Generate reference to an operand in the match rule: Operand("opname")
    ///
    /// This need not be written in rules; if "X" appears in any part of a replacement rule
    /// where an 'Op' expression is needed, it will be interpreted as Operand("X"). Including
    /// the case where the entire replacement rule is just "X" (i.e. the rule 'bypasses' input X to
    /// the output).
    ///
    static ReplFunc Operand(hnnx::operand_tag_parm_t str)
    {
        return ReplFunc::create(
                [=](Replacement &rpx, const OpDef &old) -> OpRef {
                    return rpx.get_opref(str);
                });
    }
    static ReplFunc Operand(ReplFunc const &opf) { return opf; }
    static ReplFunc Operand(ReplFunc &&opf) { return std::move(opf); }
    template <oExp::OpVnt V, typename T>
    static ReplFunc Operand(oExp::opexpr<V, T> &&op)
    {
        return hnnx::wrap_as_replfunc(op);
    }
    template <oExp::OpVnt V, typename T>
    static ReplFunc Operand(oExp::opexpr<V, T> const &op)
    {
        return hnnx::wrap_as_replfunc(op);
    }

    static ReplFunc Op_inner(char const *str, char const *package, int n_in,
                             ReplFunc const *ifuncs);

    // all of the Ts for Op should be either an operand_tag_t (or convertible to one)
    // or should be a ReplFunc
    // This Op() just maps all the operand tags to  ReplFunc
    // (by passing the them all through Operand(), which has no effect on functions)
    // They are placed in an array, passed to Op_inner.
    //
    /// \ingroup OptReplacement
    /// @brief Generate a new Op in a replacement rule: Op("opname", ... inputs ... )
    ///
    /// The inputs can be any 'replacement' expressions, or operand tags; the shape and type of the Op output
    /// are inherited from the replaced Op -- or from the innermost modifier, if the Op appears
    /// within a modifier.
    ///

    template <typename... Ts>
    static ReplFunc __attribute__((visibility("hidden")))
    Op(char const *str, Ts... ts)
    {
        std::array<ReplFunc, sizeof...(Ts)> input_funcs = {
                Replacement::Operand(ts)...};
        return Op_inner(str, pkg_flag.c_str(), sizeof...(Ts),
                        input_funcs.data());
    }
    // this is to include oExp::SELECT, on an equal namespace footing with these other select.
    template <typename TS, typename TA, typename TB>
    static inline auto SELECT(TS &&sel, TA &&a, TB &&b)
    {
        // compiler wants to use this for Repl inputs, too...
        // send those to SELECT_func
        if constexpr (std::is_constructible<ReplFunc_or_Operand, TA>::value ||
                      std::is_constructible<ReplFunc_or_Operand, TB>::value) {
            return SELECT_func(
                    oExp::wrap_as_function<bool>(std::forward<TS>(sel)),
                    ReplFunc_or_Operand(std::forward<TA>(a)),
                    ReplFunc_or_Operand(std::forward<TB>(b)));
        } else {
            return oExp::SELECT(std::forward<TS>(sel), std::forward<TA>(a),
                                std::forward<TB>(b));
        }
    }

    // this is to implement all of the select cases where ?: works, where the result is not ReplFunc
    /* removed - I doubt this is safe
	template <typename TA, typename TB>
	static auto SELECT( bool sel, TA &&iftrue, TB &&iffalse) -> decltype(sel?iftrue:iffalse) {
		return sel? std::forward<TA>(iftrue): std::forward<TB>(iffalse);
	} */
    // SELECT ReplFunc with immediate execution
    // The second one allows "Parmname" as one operand, third allows two.
    static ReplFunc SELECT(bool sel, ReplFunc_general &&iftrue,
                           ReplFunc_general &&iffalse);
    static ReplFunc SELECT(bool sel, ReplFunc_or_Operand &&iftrue,
                           ReplFunc_or_Operand &&iffalse);
    static ReplFunc SELECT(bool sel, char const *iftrue, char const *iffalse);
    // SELECT ReplFunc with deferred execution (returned function will call sel(), and then one
    // of the functions).
    static ReplFunc SELECT_func(ReplFuncBool &&sel,
                                ReplFunc_or_Operand &&iftrue,
                                ReplFunc_or_Operand &&iffalse);

    template <oExp::Variant V, typename T>
    static ReplFunc SELECT(oExp::expr<V, T> &&condn,
                           ReplFunc_or_Operand &&iftrue,
                           ReplFunc_or_Operand &&iffalse)
    {
        return SELECT_func(oExp::wrap_as_function<bool>(std::move(condn)),
                           std::move(iftrue), std::move(iffalse));
    }

    /*
	OpRef do_replacement(const OpDef & oldop, ReplFunc const & f)
	{
		return f(*this,oldop);
	}
	OpRef do_replacement(const OpDef & oldop, hnnx::operand_tag_parm_t str)
	{
		return get_opref(str);
	}*/
  public:
    OpDef const &curr_op() const { return *m_curr_op; }

    static OpRef gen_node(const hnnx::opname_tag_t str,
                          std::vector<OpRef> const &inputs, const OpDef &old,
                          char const *package_name = THIS_PKG_NAME_STR,
                          const OpDef *model = nullptr);
    static OpRef gen_node(const hnnx::opname_tag_t str, size_t n_in,
                          OpRef const *inputs, const OpDef &old,
                          char const *package_name = THIS_PKG_NAME_STR);
    template <size_t N>
    static inline OpRef
    gen_node(const hnnx::opname_tag_t str, std::array<OpRef, N> const &inputs,
             const OpDef &old, char const *package_name = THIS_PKG_NAME_STR)
    {
        return gen_node(str, N, &inputs[0], old, package_name);
    }

    OpRef gen_Shape_in_graph(const OpDef &old, int rank, size_t const *sizes);

    template <DType DT>
    OpRef gen_Const_scalar(const OpDef &old,
                           typename dtype_traits<DT>::element_type constval);

    template <DType DT>
    OpRef
    gen_Const_1D_array(const OpDef &old,
                       typename dtype_traits<DT>::element_type const *vals,
                       size_t n);

    template <DType DT>
    OpRef
    gen_Const_4D_array(const OpDef &old,
                       typename dtype_traits<DT>::element_type const *vals,
                       size_t n);

    OpRef gen_Const_int32_common(const OpDef &old, const OutputDef &out_def,
                                 const uint8_t *data, size_t data_len);

    OpRef gen_Const_float_common(const OpDef &old, const OutputDef &out_def,
                                 const uint8_t *data, size_t data_len);

    template <typename UniqueType> static ReplFunc replacement();
    //typedef OpRef (Replacement::*replacementfn_type)(const OpDef &oldop);
    typedef ReplFunc (*replacementfn_type)();

    template <typename T> static constexpr bool is_Op_type()
    {
        return std::is_constructible<ReplFunc_or_Operand, T>::value;
    }
};

namespace hnnx {

//
// These implement CONSTVAL_INT, CONSTVAL_INT_VALID
// and GETCONST_FLOAT, CONSTVAL_FLOAT_VALID
// The first part of the return value is the result from CONSTVAL_INT(op,idx)
// The second part is the return from CONSTVAL_INT_VALID
std::pair<NN_INT32_T, bool> getconst_int_impl(Graph &g, OpDef const &opdef,
                                              int index);
std::pair<NN_INT32_T, bool> getconst_int_impl(Graph &g, OpDef const &opdef,
                                              int index, int index2);
std::pair<float, bool> getconst_float_impl(Graph &g, OpDef const &opdef,
                                           int index);
std::pair<float, bool> getconst_float_impl(Graph &g, OpDef const &opdef,
                                           int index, int index2);

class GraphOptInfo;
/*
 * A GraphOptContext ties these all together, along with the 'attempt' method
 */
class GraphOptContext : public Replacement {
  public:
    GraphOptContext(Graph &g) : Replacement(g) {}
    bool attempt(GraphOptInfo const &, OpDef &oldop);
};

class GraphOptPass;

// GraphOpInfo: contains pointers to all the specialized methods.
// These are all created as global variables, and they are linked together
// in a linked list; optimization_passes will be populated with pointers
// to them.

class GraphOptInfo {
    friend class GraphOptContext;

    int priority;
    OptimFlags::flags_t flags;
    //bool (Match::*)(OpRef base) match;
    Match::matchbuilder_type matchbld;
    //bool (Constraint::*)() constrain;
    Constraint::constraintfn_type constrain;
    //OpRef (Replacement::*)() replace;
    Replacement::replacementfn_type replace;

    MatchOp_uptr matchop_ptr; //stores the built matchop.
    ReplFuncBool constraint_func; // function object for the constraint.
    ReplFunc replace_func; // fucntion object for replacement.
    GraphOptInfo const
            *next_in_pass; // next opt for the same opstr in the same pass.

    // note, WITH OPT_DEBUG must be consistent across a build now, otherwise you
    // should get a link error (at least on "add_package_opt").
#ifdef WITH_OPT_DEBUG
    char const *debug_filename = nullptr;
    int debug_lineno = 0;
#endif

    // this is done in populate_optimization_map, for all optims.
    void build_matchop()
    {
        matchop_ptr = MatchBuilder::build_matcher((*matchbld)());
        // build the constraint function too
        // If the actual constraint function is detected to be the 'always true'
        // function, we leave constraint_func empty.
        ReplFuncBool cfunc = (*constrain)();
        int check = oExp::check_sfunction_bool(cfunc);
        if (check != 1)
            constraint_func = cfunc;
        replace_func = (*replace)();
    }

  public:
    GraphOptInfo(int priority, OptimFlags::flags_t flags_in,
                 Match::matchbuilder_type matchbld_in,
                 Constraint::constraintfn_type constrain_in,
                 Replacement::replacementfn_type replace_in);

    // This fills in the optimization map.
    static void insert_optimization(std::map<int, GraphOptPass> &opt_passes,
                                    GraphOptInfo *p);
    static void populate_package_optimization_map(
            std::vector<std::unique_ptr<GraphOptInfo>> &opts);

    inline bool test_constraint(Constraint &cst) const
    {
        // an empty constraint_func means 'always'
        return constraint_func ? constraint_func(cst) : true;
    }
    GraphOptInfo const *next_optim() const { return next_in_pass; }

    MatchOpBase &get_matchop() const { return *matchop_ptr.get(); }

    OptimFlags::flags_t get_flags() const { return flags; }
    bool has_flags(OptimFlags::flags_t v) const { return (flags & v) != 0; }
#if defined(WITH_OPT_DEBUG)
    inline void add_debug_info(char const *filename, int lineno)
    {
        debug_filename = filename;
        debug_lineno = lineno;
    }
    inline char const *get_filename() const { return debug_filename; }
#else
    inline char const *get_filename() const { return ""; }
#endif
    // get the filename.cc:lineo as a string
    std::string get_debug_filepos() const;
};

//
// optimization_passes is a vector of GraphOptPass;
// each GraphOptPass contains a map of opstr => GraphOptInfo*, which
// is the *first* rule to be applied to the given opstr, within that pass.
// if there are more rules, they are chained via their 'next_in_pass' fields.

class GraphOptPass {
  public:
    int priority;
    OptimFlags::flags_t flags; // 'or' of certain flags in the whole pass
    int nrules; // total # of rules the pass.
    // use a minhash_noerase for the rules, if we can:
    using rules_map_t = std::conditional_t<
            std::is_same_v<opname_tag_t, string_tag_t>,
            minihash_noerase<opname_tag_t, GraphOptInfo const *>,
            std::map<opname_tag_t, GraphOptInfo const *>>;

    rules_map_t rules;
    // for each name in the rules, bit  find_opname_hash(name)&63
    // is set in set_bitmap, so we don't even need to probe the map
    // unless we see that bit.
    uint64_t set_bitmap;

    explicit GraphOptPass(int pri)
        : priority(pri), flags(0), nrules(0), set_bitmap(0)
    {
    }
    GraphOptPass(GraphOptPass &&) = default;

    // get the first GraphOptInfo for a given OpDef (or null if none)
    GraphOptInfo const *first_optim(OpDef const *opdef) const
    {
        int h = opdef->get_opstr_hash() & 63;
        if ((set_bitmap & (uint64_t(1) << h)) == 0)
            return nullptr;
        auto found = rules.find(opdef->opstr);
        return (found == rules.end()) ? nullptr : found->second;
    }
};

} // namespace hnnx

#pragma GCC visibility pop

#include "oexpr_post.h"

//
//

#pragma GCC visibility push(default)

namespace hnnx {

std::map<int, GraphOptPass> &get_optimization_passes();

std::map<std::string, std::vector<std::unique_ptr<GraphOptInfo>> *> &
get_pkg_opt_tmp_map();

// There is a mechanism here to force a bad link unless all of the DEF_OPT are compiled
// with the same value of WITH_OPT_DEBUG : "add_package_op" is defined as a real function
// and it has two extra parms when defined(WITH_OPT_DEBUG).
// When !defined(WITH_OPT_DEBUG), the version with two extra parms is an inline which
// calls the other one, dropping the extra paramaters.
void add_package_opt(std::vector<std::unique_ptr<GraphOptInfo>> &opts,
                     int priority, OptimFlags::flags_t flags_in,
                     Match::matchbuilder_type matchbld_in,
                     Constraint::constraintfn_type constrain_in,
                     Replacement::replacementfn_type replace_in
#if defined(WITH_OPT_DEBUG)
                     ,
                     char const *fname, int lineno
#endif
);
#if !defined(WITH_OPT_DEBUG)
// just forward this to the other one and drop the extra parms.
__attribute__((always_inline)) inline void
add_package_opt(std::vector<std::unique_ptr<GraphOptInfo>> &opts, int priority,
                OptimFlags::flags_t flags_in,
                Match::matchbuilder_type matchbld_in,
                Constraint::constraintfn_type constrain_in,
                Replacement::replacementfn_type replace_in, char const *fname,
                int lineno)
{
    add_package_opt(opts, priority, flags_in, matchbld_in, constrain_in,
                    replace_in);
}
#endif

std::string get_opname_with_default_pkg_prefix(char const *opname);

} // namespace hnnx

#pragma GCC visibility pop

#define INIT_PACKAGE_OPTIMIZATION_DEF()                                        \
    __attribute__((visibility("hidden")))                                      \
            std::vector<std::unique_ptr<hnnx::GraphOptInfo>>                   \
                    &current_package_opts_storage_vec_func()                   \
    {                                                                          \
        static std::vector<std::unique_ptr<hnnx::GraphOptInfo>> optv;          \
        return optv;                                                           \
    }                                                                          \
    extern "C" {                                                               \
    void clear_package_opts_storage_vec_func()                                 \
    {                                                                          \
        current_package_opts_storage_vec_func().clear();                       \
    }                                                                          \
    }

#define DECLARE_PACKAGE_OPTIMIZATION_DEF()                                     \
    __attribute__((visibility("hidden")))                                      \
            std::vector<std::unique_ptr<hnnx::GraphOptInfo>>                   \
                    &current_package_opts_storage_vec_func();

#define REGISTER_PACKAGE_OPT(PRIORITY, FLAGS, MATCHFN, CONSTRAINTFN,           \
                             REPLACEFN)                                        \
    [[maybe_unused]] static bool CTRICKS_PASTER(_PKG_OPT_REG_, __LINE__) =     \
            (hnnx::add_package_opt(current_package_opts_storage_vec_func(),    \
                                   PRIORITY, FLAGS, MATCHFN, CONSTRAINTFN,     \
                                   REPLACEFN, __FILE__, __LINE__),             \
             true);

#define DEF_PACKAGE_OPTIMIZATION(PRIORITY, MATCHCODE, CONSTRAINTCODE,          \
                                 REPLACECODE)                                  \
    DEF_PACKAGE_OPTIMIZATION_COMMON(PRIORITY, 0, MATCHCODE, CONSTRAINTCODE,    \
                                    REPLACECODE)

#define DEF_PACKAGE_OPTIMIZATION_WITH_FLAGS(PRIORITY, FLAGS, MATCHCODE,        \
                                            CONSTRAINTCODE, REPLACECODE)       \
    DEF_PACKAGE_OPTIMIZATION_COMMON(PRIORITY, FLAGS, MATCHCODE,                \
                                    CONSTRAINTCODE, REPLACECODE)

#define DEF_PACKAGE_OPTIMIZATION_COMMON(PRIORITY, FLAGS, MATCHCODE,            \
                                        CONSTRAINTCODE, REPLACECODE)           \
    template <> hnnx::MatchAst_uptr MatchBuilder::matcher<UNIQUE_TYPE>()       \
    {                                                                          \
        return MATCHCODE;                                                      \
    };                                                                         \
    template <> ReplFuncBool Constraint::constraint<UNIQUE_TYPE>()             \
    {                                                                          \
        using namespace oExp_for_cst;                                          \
        using oExp::INT;                                                       \
        using oExp::UINT;                                                      \
        auto result = oExp::wrap_param_to<bool>(CONSTRAINTCODE);               \
        return oExp::wrap_as_function<bool>(result);                           \
    }                                                                          \
    template <> ReplFunc Replacement::replacement<UNIQUE_TYPE>()               \
    {                                                                          \
        using namespace oExp_for_repl;                                         \
        using oExp::INT;                                                       \
        using oExp::UINT;                                                      \
        pkg_flag = THIS_PKG_NAME_STR;                                          \
        return Operand(REPLACECODE);                                           \
    }                                                                          \
    template <>                                                                \
    inline constexpr hnnx::OptimFlags::flags_t                                 \
    hnnx::OptimFlags::flag_evaluate<UNIQUE_TYPE>()                             \
    {                                                                          \
        return any_rule | (FLAGS);                                             \
    }                                                                          \
    REGISTER_PACKAGE_OPT((PRIORITY),                                           \
                         hnnx::OptimFlags::flag_evaluate<UNIQUE_TYPE>(),       \
                         &MatchBuilder::matcher<UNIQUE_TYPE>,                  \
                         &Constraint::constraint<UNIQUE_TYPE>,                 \
                         &Replacement::replacement<UNIQUE_TYPE>);

#define REGISTER_PACKAGE_OPTIMIZATIONS()                                       \
    {                                                                          \
        auto &pkg_opt_map =                                                    \
                hnnx::get_pkg_opt_tmp_map(); /* package registration map */    \
        auto [iter, ok] = pkg_opt_map.try_emplace(                             \
                std::string(THIS_PKG_NAME_STR),                                \
                nullptr); /*see if we can insert an empty one */               \
        if (ok)                                                                \
            iter->second = &current_package_opts_storage_vec_func();           \
    } /* if so, replace it with this */

#ifndef PREPARE_DISABLED
#define DEF_OPTIM(PRIORITY, FLAGS, MATCHCODE, CONSTRAINTCODE, REPLACECODE)     \
    DEF_PACKAGE_OPTIMIZATION_WITH_FLAGS(PRIORITY, FLAGS, MATCHCODE,            \
                                        CONSTRAINTCODE, REPLACECODE)
#else
#define DEF_OPTIM(PRIORITY, FLAGS, MATCHCODE, CONSTRAINTCODE, REPLACECODE)
#endif

#ifndef PREPARE_DISABLED
#define DEF_OPT(PRIORITY, MATCHCODE, CONSTRAINTCODE, REPLACECODE)              \
    DEF_PACKAGE_OPTIMIZATION(PRIORITY, MATCHCODE, CONSTRAINTCODE, REPLACECODE)
#else
#define DEF_OPT(PRIORITY, MATCHCODE, CONSTRAINTCODE, REPLACECODE)
#endif

#define FROM_DEFAULT_PACKAGE(OP)                                               \
    hnnx::get_opname_with_default_pkg_prefix(OP).c_str()

DECLARE_PACKAGE_OPTIMIZATION_DEF()

#define COMPILER_FOR(XXF, FUNC, PARA)                                          \
    template <> constexpr bool has_compile_method<XXF> = true;                 \
    template <> struct OpaqueT_FOR<XXF> {                                      \
        using type = PARA;                                                     \
    };                                                                         \
    template <>                                                                \
    hnnx::Executable::ItemType                                                 \
    hnnx::TypicalOpWithCompiler<XXF, PARA>::compile() const                    \
    {                                                                          \
        return FUNC(this);                                                     \
    }

#define GRAPH_CLEANUP 0
#define PRE_QNN 500
#define QNN 1000
#define EARLY 2000
#define MIDDLE 3000
#define LATE 4000

#endif
