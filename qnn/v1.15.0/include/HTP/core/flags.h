//==============================================================================
//
// Copyright (c) 2020 Qualcomm Technologies, Inc.
// All Rights Reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
//
//==============================================================================

#ifndef HEXNN_FLAGS_H
#define HEXNN_FLAGS_H 1

#include <cstddef>

/*
 * Every flag needs a constant for what it refers to
 */
typedef unsigned long long Flags_word;
enum class Flags : unsigned {
    IS_CONST = 0, // output doesn't change
    INHIBIT_CONST_PROP, // do not const-propagate this Op
    RESOURCE_HVX, // op needs an HVX thread (converted to spawn/validate)
    RESOURCE_HMX, // uses HMX
    IS_DMA, // op issues dma, does not wait (converted to dma_start/dma_sync)
    FOR_HVX, // op is a spawn or validate (must combine with MOVE_*)
    FOR_DMA, // op is a dma_start or dma_sync (must combine with MOVE_*)
    MOVE_EARLY, // scheduler should move early (must combine with FOR_*)
    MOVE_LATE, // scheduler should move late (must combine with FOR_*)
    NULL_EXEC, // exec function does nothing
    IS_SPILL, // Op is a 'Spill', added during schedule/alloc
    IS_FILL, // Op is a 'Fill', added during schedule/alloc
    INPLACE_NOP, // A NULL_EXEC which is just a copy (see below).
    IS_COPY, // An op which is a copy (see below)
    IS_SYNC, // Op is a 'SyncOp'
    IS_PADZAP, // Op is a crouton padzap (same tensor type and shape in and out)
    XXX_LAST_FLAG
};

// INPLACE_NOP is a null_exec op that has 1 input and 1 output, with identical DType and quantization,
// shape, and tensor type; which could just be bypassed. Examples are the 'Padzap' with no work to do,
// and all ForceFormat_flat where the input is already flat (implemented by format_no_translate_flat).

// IS_COPY is a op that has 1 input and 1 output, with the same shape and tensor layout type; the memory
// class and dtype can be different (dtype must be the same #bytes); but the operation must be fulfilled
// by raw copy of the input block(s) to the output block(s). Mainly this is intended to mark const->TCM
// operations that can be replaced by 'const-fill'.

static_assert(static_cast<int>(Flags::XXX_LAST_FLAG) <= 64, "Too many flags");

/**
 * @brief Now we want to add flags to Ops (and maybe other things)
 * The default for all flags is 0.
 * Ideally, every op have one poitner/reference/function pointer/something per class (not per obj)
 * that would get the right flags.
 *
 * We could get that with a virtual function (entry in the vtable)
 * ... the default could be inherited
 * ... But how do we choose whether or not to override that function?
 * ... And how do we override the function conditionally?
 * We could get a static constexpr variable...
 * ... But how would we get at the static constexpr value from a pointer to base?
 *
 * We have get_flag_word(), virtual method of Op, which returns a Flags_word value.
 * And get_flag(Flag f), non-virtual; calls get_flag_word() and then tests the specified bit.
 * You can also call get_flag_word() once and test multiple bits using calls to test_flag_for().
 */

/*
 * We might be able to use bitset here, but bitset has limited constexpr for some reason...
 */

namespace hnnx {

template <Flags... idxs>
constexpr Flags_word flagval_generate =
        ((Flags_word(1) << static_cast<unsigned>(idxs)) | ... | 0);

template <typename T> static constexpr Flags_word flags_for = 0;

static constexpr bool test_flag_for(Flags_word w, Flags which)
{
    return ((w >> static_cast<unsigned>(which)) & 1) != 0;
}
static constexpr bool test_flag_and(Flags_word w, Flags which_a, Flags which_b)
{
    if (((w >> static_cast<unsigned>(which_a)) & 1) == 0)
        return false;
    return ((w >> static_cast<unsigned>(which_b)) & 1) != 0;
}

} // namespace hnnx

#define FLAGS_FOR(T, ...)                                                      \
    template <>                                                                \
    static constexpr Flags_word hnnx::flags_for<T> =                           \
            hnnx::flagval_generate<__VA_ARGS__>;
#define FLAGS_FOR_DT(F, ...) FLAGS_FOR(DerivedType<F>::type, __VA_ARGS__)

#endif
