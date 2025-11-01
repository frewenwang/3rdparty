//==============================================================================
//
// Copyright (c) 2020 Qualcomm Technologies, Inc.
// All Rights Reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
//
//==============================================================================

#ifndef OPTIMIZE_DEFS_H
#define OPTIMIZE_DEFS_H 1

// this file contains #define that need to be seen by the optimization rule parser,
// in addition to the C++ code. Don't place any #include in here.
//

/**
 * \defgroup OptConstraint
 * @{
 */

#define IS_SCALAR(X)                                                           \
    AND(EQ(DIM_BATCHES(X), 1), EQ(DIM_HEIGHT(X), 1), EQ(DIM_WIDTH(X), 1),      \
        EQ(DIM_DEPTH(X), 1))

#define IS_NOT_SCALAR(X)                                                       \
    OR(NE(DIM_BATCHES(X), 1), NE(DIM_HEIGHT(X), 1), NE(DIM_WIDTH(X), 1),       \
       NE(DIM_DEPTH(X), 1))

#define IS_SHAPE_1x1x1xd(X)                                                    \
    AND(EQ(DIM_BATCHES(X), 1), EQ(DIM_HEIGHT(X), 1), EQ(DIM_WIDTH(X), 1),      \
        NE(RANK_OF(X), 5))

#define IS_SHAPE_1x1x1x1xc(X)                                                  \
    AND(EQ(RANK_OF(X), 5), EQ(DIM_BATCHES(X), 1), EQ(DIM_HEIGHT(X), 1),        \
        EQ(DIM_WIDTH(X), 1), EQ(DIM_DEPTH(X), 1))

/// @brief A constant specifying the standard split for output depth
// NOTE: HEXNNVVV-330 workaround: stay at 32 channels or FP test starts to fail...
#define MIN_CHANNEL_SPLIT_SIZE 32
#define CHANNEL_SPLIT_SIZE 256

// TCM_SIZE and TCM_TOOBIG are renamed to TCM_MAXTENSOR_SIZE and TCM_MAXTENSOR_HALF_SIZE
// to help with tilling on 2x4mb auto hardware
// where total tcm size is multiple of tiling size
//#define TCM_SIZE OPTION_UINT("tcm_size")
//#define TCM_TOOBIG DIV(OPTION_UINT("tcm_size"), 2)

// TCM_MAXTENSOR_HALF_SIZE is one half of the TCM tiling size
#define TCM_MAXTENSOR_SIZE OPTION_UINT("tcm_size_for_tiling")
#define TCM_MAXTENSOR_HALF_SIZE DIV(OPTION_UINT("tcm_size_for_tiling"), 2)

// Used in depth slicing, where height slicing has not been done
// Depth could be 1.
#define ELEMWISE_TILE_SIZE(ACT)                                                \
    MUL(TILE_HEIGHT, ROUNDUP(DIM_WIDTH(ACT), DIV(8, ELEMENTSIZE_OF(ACT))),     \
        ROUNDUP(MIN(CHANNEL_SPLIT_SIZE, DIM_DEPTH(ACT)), 32),                  \
        ELEMENTSIZE_OF(ACT))

#define ELEMWISE_TOOBIG(A, B, OUT)                                             \
    GT(ADD(ELEMWISE_TILE_SIZE(A), ELEMWISE_TILE_SIZE(B),                       \
           ELEMWISE_TILE_SIZE(OUT)),                                           \
       TCM_MAXTENSOR_SIZE)

#define WEIGHT_STORAGE(WEIGHT, SPLIT)                                          \
    MUL(DIM_FILTHEIGHT(WEIGHT), DIM_FILTWIDTH(WEIGHT), DIM_FILTDEPTH(WEIGHT),  \
        MIN(SPLIT, DIM_NFILTS(WEIGHT)))
#define ACT_STORAGE_EST(WEIGHT, ACT, SPLIT)                                    \
    ADD(MUL(ELEMENTSIZE_OF(ACT), ROUNDUP(ADD(7, DIM_FILTHEIGHT(WEIGHT)), 8),   \
            ROUNDUP(DIM_WIDTH(ACT), 8), ROUNDUP(DIM_DEPTH(ACT), 32)),          \
        MUL(ELEMENTSIZE_OF("*"), 8, ROUNDUP(DIM_WIDTH("*"), 8), SPLIT))

#define GOOD_WEIGHTS(WEIGHT, ACT, SPLIT)                                       \
    LT(ADD(WEIGHT_STORAGE(WEIGHT, SPLIT),                                      \
           ACT_STORAGE_EST(WEIGHT, ACT, SPLIT)),                               \
       TCM_MAXTENSOR_HALF_SIZE)

/*
 * "Choose the maximum channel split size that doesn't make the slice of weights too big"
 */
#define SMART_CHANNEL_SIZE(WEIGHT_STR, ACT_STR)                                \
    SELECT(GOOD_WEIGHTS(WEIGHT_STR, ACT_STR, CHANNEL_SPLIT_SIZE),              \
           CHANNEL_SPLIT_SIZE,                                                 \
           SELECT(GOOD_WEIGHTS(WEIGHT_STR, ACT_STR,                            \
                               DIV(CHANNEL_SPLIT_SIZE, 2)),                    \
                  DIV(CHANNEL_SPLIT_SIZE, 2),                                  \
                  SELECT(GOOD_WEIGHTS(WEIGHT_STR, ACT_STR,                     \
                                      DIV(CHANNEL_SPLIT_SIZE, 4)),             \
                         DIV(CHANNEL_SPLIT_SIZE, 4), 32)))

// For HMX DWC
#define DWC_ACT_STORAGE_EST(WEIGHT, ACT, SPLIT)                                \
    ADD(MUL(ELEMENTSIZE_OF(ACT), ROUNDUP(ADD(7, DIM_FILTHEIGHT(WEIGHT)), 8),   \
            ROUNDUP(DIM_WIDTH(ACT), 8), SPLIT),                                \
        MUL(ELEMENTSIZE_OF("*"), 8, ROUNDUP(DIM_WIDTH("*"), 8), SPLIT))

#define DWC_GOOD_WEIGHTS(WEIGHT, ACT, SPLIT)                                   \
    LT(ADD(WEIGHT_STORAGE(WEIGHT, SPLIT),                                      \
           DWC_ACT_STORAGE_EST(WEIGHT, ACT, SPLIT)),                           \
       TCM_MAXTENSOR_HALF_SIZE)

#define DWC_SMART_CHANNEL_SIZE(WEIGHT_STR, ACT_STR)                            \
    SELECT(DWC_GOOD_WEIGHTS(WEIGHT_STR, ACT_STR, CHANNEL_SPLIT_SIZE),          \
           CHANNEL_SPLIT_SIZE,                                                 \
           SELECT(DWC_GOOD_WEIGHTS(WEIGHT_STR, ACT_STR,                        \
                                   DIV(CHANNEL_SPLIT_SIZE, 2)),                \
                  DIV(CHANNEL_SPLIT_SIZE, 2),                                  \
                  SELECT(DWC_GOOD_WEIGHTS(WEIGHT_STR, ACT_STR,                 \
                                          DIV(CHANNEL_SPLIT_SIZE, 4)),         \
                         DIV(CHANNEL_SPLIT_SIZE, 4), 32)))

// For other depthwise ops
#define MAX_CHANNEL_SIZE(ACT)                                                  \
    ROUNDUP(MAX(1, DIV(TCM_MAXTENSOR_HALF_SIZE,                                \
                       MUL(ELEMENTSIZE_OF(ACT), ROUNDUP(DIM_HEIGHT(ACT), 8),   \
                           ROUNDUP(DIM_WIDTH(ACT), 8)))),                      \
            32)

/// @brief SAME_QUANT("A", "B") -> true if the operands have the same stepsize and zero offset
#define SAME_QUANT(OPA, OPB)                                                   \
    OR(AND(EQ(STEPSIZE_OF(OPA), STEPSIZE_OF(OPB)),                             \
           EQ(ZERO_OFFSET_OF(OPA), ZERO_OFFSET_OF(OPB))),                      \
       AND(IS_FLOAT16(OPA), IS_FLOAT16(OPB)),                                  \
       AND(IS_FLOAT32(OPA), IS_FLOAT32(OPB)))

/// @brief SAME_DTYPE_QUANT("A", "B") -> true if the operands have the same dtype, stepsize and zero offset
#define SAME_DTYPE_QUANT(OPA, OPB)                                             \
    AND(EQ(DTYPE_OF(OPA), DTYPE_OF(OPB)),                                      \
        EQ(STEPSIZE_OF(OPA), STEPSIZE_OF(OPB)),                                \
        EQ(ZERO_OFFSET_OF(OPA), ZERO_OFFSET_OF(OPB)))

/// @brief SAME_SHAPE("A", "B") -> true if the operands have the same shape over 4 dimensions.
#define SAME_SHAPE(X, Y)                                                       \
    AND(EQ(DIM_DEPTH(X), DIM_DEPTH(Y)), EQ(DIM_HEIGHT(X), DIM_HEIGHT(Y)),      \
        EQ(DIM_WIDTH(X), DIM_WIDTH(Y)), EQ(DIM_BATCHES(X), DIM_BATCHES(Y)))

/// @brief OPCONST(X) enforces that op X is a Const during pattern matching
#define OPCONST(X) LET(X, Op("$Const"))

// How wide should the output tile be?
// Well,
// * We have HEX_VTCM_MB - WEIGHT_STORAGE available from VTCM
// * Input is roughly (input height * input depth) * (1+filter-related-value) * WIDTH
// * Output is output depth * 8 * WIDTH
// So we should take (HEX_VTCM_MB-WEIGHT_STORAGE) and divide by
//     (input height * input depth) * (1+filter-related-value) + (OUTPUT DEPTH*8)
// And then round down to a multiple of 8 probably
// But we need to do at least 8 wide

// need "Too big" indication for constraint (which might be "big total width"), then
// need to tile into at least ~4 chunks to actually shrink size.

#define MIN_WIDTH 8

#define ESTIMATE_TENSOR_SIZE(T)                                                \
    MUL(ELEMENTSIZE_OF(T), DIM_BATCHES(T),                                     \
        ROUNDUP(DIM_HEIGHT(T), TILE_HEIGHT),                                   \
        ROUNDUP(DIM_WIDTH(T), DIV(TILE_HEIGHT, ELEMENTSIZE_OF(T))),            \
        ROUNDUP(DIM_DEPTH(T), 32))

#define ESTIMATE_SIZE(ACT, WEIGHTS, OUT)                                       \
    ADD(ROUNDUP(WEIGHT_STORAGE(WEIGHTS, DIM_NFILTS(WEIGHTS)), 2048),           \
        ESTIMATE_TENSOR_SIZE(ACT), ESTIMATE_TENSOR_SIZE(OUT),                  \
        ROUNDUP(MUL(8, ROUNDUP(DIM_DEPTH(OUT), 32)), 2048))

#define MAX_GOOD_WIDTH_DILATION(ACT_STR, WEIGHT_STR, OUT_STR, DILATION,        \
                                TCMSIZE)                                       \
    MAX(MIN_WIDTH,                                                             \
        ROUNDUP(DIV(SUB(DIV(MUL(TCMSIZE, 7), 8),                               \
                        ADD(ROUNDUP(WEIGHT_STORAGE(WEIGHT_STR,                 \
                                                   DIM_NFILTS(WEIGHT_STR)),    \
                                    2048),                                     \
                            MUL(ROUNDUP(DIM_HEIGHT(ACT_STR), 8),               \
                                DIM_DEPTH(ACT_STR),                            \
                                ROUNDUP(ADD(1,                                 \
                                            MUL(SUB(DIM_FILTWIDTH(WEIGHT_STR), \
                                                    1),                        \
                                                DILATION)),                    \
                                        8)))),                                 \
                    MUL(4, ADD(MUL(ELEMENTSIZE_OF(OUT_STR),                    \
                                   ROUNDUP(DIM_DEPTH(OUT_STR), 32),            \
                                   ROUNDUP(DIM_HEIGHT(OUT_STR), 8)),           \
                               MUL(ELEMENTSIZE_OF(OUT_STR),                    \
                                   ROUNDUP(DIM_HEIGHT(ACT_STR), 8),            \
                                   ROUNDUP(DIM_DEPTH(ACT_STR), 32))))),        \
                8))

#define MAX_GOOD_WIDTH(ACT_STR, WEIGHT_STR, OUT_STR, TCMSIZE)                  \
    MAX_GOOD_WIDTH_DILATION(ACT_STR, WEIGHT_STR, OUT_STR, 1, TCMSIZE)

#define FLAT_TENSOR_SIZE(T)                                                    \
    MUL(ELEMENTSIZE_OF(T), DIM_BATCHES(T), DIM_HEIGHT(T), DIM_WIDTH(T),        \
        DIM_DEPTH(T))

// How many bytes do we have left for our S2D data?  Well, we have to take our total TCM size
// Then we subtract off storage for the weights
// Then we subtract off the scale/bias values
// Finally we subtract off some small amount of space for output data

#define S2S2D_BYTESLEFT(WSHAPE, ASHAPE, TCMSIZE)                               \
    SUB(TCMSIZE,                                                               \
        ADD(ROUNDUP(WEIGHT_STORAGE(WSHAPE, DIM_NFILTS(WSHAPE)), 2048),         \
            MUL(ROUNDUP(DIM_NFILTS(WSHAPE), 32), 8), MUL(1, 8, 64, 64)))

// What's the maximum width?
// Well, start with the number of bytes left
// The expected size of the input at the conv, is 4*orig_depth * width * height
// Height is probably 16 (8 + overlap)
// so divide by 4*depth*16 and if it's more than that, your width is too big.

#define S2S2D_MAXWIDTH(WSHAPE, ASHAPE, TCMSIZE)                                \
    DIV(S2S2D_BYTESLEFT(WSHAPE, ASHAPE, TCMSIZE),                              \
        MUL(/* Don't forget element size */ ELEMENTSIZE_OF("*"),               \
            /* Height, roughly */ 16,                                          \
            /* Depth, roughly */ MUL(4, ROUNDUP(DIM_DEPTH(ASHAPE), 32))))

// If your width is too big, how much do you want in each chunk?
// We don't want it to be too big, because we want to hold decent amounts of things in TCM
// And not lose too much from overlap and stuff
// On the other hand, you don't want too small of a tile size because of overhead
#define S2S2D_GOODWIDTH(WSHAPE, ASHAPE, TCMSIZE)                               \
    MAX(MIN_WIDTH, ROUNDUP(DIV(S2S2D_MAXWIDTH(WSHAPE, ASHAPE, TCMSIZE), 6), 8))

#define MAX_GOOD_WIDTH_S2D(ACT_STR, ORIG_WEIGHTS, OUT_STR, TCMSIZE)            \
    MAX(MIN_WIDTH,                                                             \
        ROUNDUP(DIV(SUB(DIV(MUL(TCMSIZE, 6), 8),                               \
                        ROUNDUP(WEIGHT_STORAGE(ORIG_WEIGHTS,                   \
                                               DIM_NFILTS(ORIG_WEIGHTS)),      \
                                2048)),                                        \
                    MUL(4, ADD(MUL(DIM_DEPTH(OUT_STR),                         \
                                   ROUNDUP(DIM_HEIGHT(OUT_STR), 8)),           \
                               MUL(ELEMENTSIZE_OF(OUT_STR), 2,                 \
                                   ROUNDUP(DIM_HEIGHT(OUT_STR), 8),            \
                                   ROUNDUP(DIM_DEPTH(ACT_STR), 32))))),        \
                8))

// Tile the width based on the input and output size
// for channel_shuffle op.
#define MAX_GOOD_WIDTH_CHANSHUF(ACT_STR, OUT_STR, TCMSIZE)                     \
    MAX(MIN_WIDTH,                                                             \
        ROUNDUP(DIV(DIV(MUL(TCMSIZE, 6), 8),                                   \
                    MUL(4, ADD(MUL(ELEMENTSIZE_OF(ACT_STR),                    \
                                   ROUNDUP(DIM_HEIGHT(ACT_STR), 8),            \
                                   ROUNDUP(DIM_DEPTH(ACT_STR), 32)),           \
                               MUL(ELEMENTSIZE_OF(OUT_STR),                    \
                                   ROUNDUP(DIM_HEIGHT(OUT_STR), 8),            \
                                   ROUNDUP(DIM_DEPTH(ACT_STR), 32))))),        \
                8))

#define MAX_GOOD_WIDTH_ELEMWISE(FIRST_IN_STR, SECOND_IN_STR, OUT_STR, TCMSIZE) \
    MAX(MIN_WIDTH,                                                             \
        ROUNDUP(DIV(DIV(MUL(TCMSIZE, 6), 8),                                   \
                    MUL(4, ADD(MUL(ELEMENTSIZE_OF(FIRST_IN_STR),               \
                                   ROUNDUP(DIM_HEIGHT(FIRST_IN_STR), 8),       \
                                   ROUNDUP(DIM_DEPTH(FIRST_IN_STR), 32)),      \
                               MUL(ELEMENTSIZE_OF(SECOND_IN_STR),              \
                                   ROUNDUP(DIM_HEIGHT(SECOND_IN_STR), 8),      \
                                   ROUNDUP(DIM_DEPTH(SECOND_IN_STR), 32)),     \
                               MUL(ELEMENTSIZE_OF(OUT_STR),                    \
                                   ROUNDUP(DIM_HEIGHT(OUT_STR), 8),            \
                                   ROUNDUP(DIM_DEPTH(OUT_STR), 32))))),        \
                8))

/** @} */

#define CONST_ZERO_OFF(OPERAND) gen_ConstScalar_i32(ZERO_OFFSET_OF(OPERAND))

// wrap tile_height option for better usability

#define TILE_HEIGHT OPTION_UINT("tile_height")

// These are used to help optimize graphs when the relaxed_precision_flag is set
#define CAST_TO_DTYPE(X, DTYPE)                                                \
    WITH_OUTPUT_TYPE(DTYPE, 0, 1.0f, Op(FROM_DEFAULT_PACKAGE("QNN_Cast"), X))

#define CAST_TO_FP16(X) WITH_SIZE(X, CAST_TO_DTYPE(X, DType::Float16))

#define CAST_TO_FP32(X) CAST_TO_DTYPE(X, DType::Float32)

#define MAKE_OP_FP16_AND_INSERT_CAST(OP)                                       \
    CAST_TO_FP32(WITH_SIZE("*", WITH_OUTPUT_TYPE(DType::Float16, 0, 1.0f, OP)))

#define IS_BINARY_FP16(A, B, Out)                                              \
    AND(IS_FLOAT16(A), IS_FLOAT16(B), IS_FLOAT16(Out))

#define IS_BINARY_FP32(A, B, Out)                                              \
    AND(IS_FLOAT32(A), IS_FLOAT32(B), IS_FLOAT32(Out))

#define FP16_CONST_CAST(X, Y)                                                  \
    LET(X, Op(FROM_DEFAULT_PACKAGE("Cast_fp32_to_fp16_plain"), Y))

#define FP16_CONST_CASTSLICE(X, Y, Z)                                          \
    LET(X, Op(FROM_DEFAULT_PACKAGE("SlicePad_shape_inplace"),                  \
              LET(Y, Op(FROM_DEFAULT_PACKAGE("Cast_fp32_to_fp16_plain"), Z)),  \
              "Before", "Start", "Out", "Zero"))

#define CONVERT_BINARY_OP_TO_FP16(OP, A, B)                                    \
    DEF_OPTIM(GRAPH_CLEANUP, relaxed_precision_flag, Op(OP, A, B),             \
              IS_BINARY_FP32(A, B, "*"),                                       \
              MAKE_OP_FP16_AND_INSERT_CAST(                                    \
                      Op(OP, CAST_TO_FP16(A), CAST_TO_FP16(B))))

// These are used to reshape 1xHx1xD or 1x1xWxD QUint8CroutonTensor
#define SHAPE_FROM_W1(A)                                                       \
    gen_Shape(DIM_BATCHES(A), DIV(ADD(DIM_HEIGHT(A), 7), 8), 8, DIM_DEPTH(A))

#define REARRANGE_FROM_W1(A)                                                   \
    WITH_SIZE(SHAPE_FROM_W1(A), WITH_TYPE(A, Op("space_rearrange", A)))

#define REARRANGE_TO_W1(OP)                                                    \
    Op("space_rearrange", WITH_SIZE(SHAPE_FROM_W1("*"), WITH_TYPE("*", OP)))

#define SHAPE_FROM_H1(A)                                                       \
    gen_Shape(DIM_BATCHES(A), MIN(8, DIV(ADD(DIM_WIDTH(A), 7), 8)),            \
              MUL(DIV(ADD(DIM_WIDTH(A), 63), 64), 8), DIM_DEPTH(A))

#define REARRANGE_FROM_H1(A)                                                   \
    WITH_SIZE(SHAPE_FROM_H1(A), WITH_TYPE(A, Op("space_rearrange", A)))

#define REARRANGE_TO_H1(OP)                                                    \
    Op("space_rearrange", WITH_SIZE(SHAPE_FROM_H1("*"), WITH_TYPE("*", OP)))

// This is intended to be seen only by the external parser, not by the C++ compiler.
// DEF_OPTIM is mapped to DEF_OPTIM_PARSE(...), with the PRIO and FLAGS parameter both
// string-quoted -- this allows these to have non-conformant (non-lispy) syntax,
// without complicating the parser;
// and DEF_OPT(PRIO,PAT...) is just DEF_OPRIM_PARSE("prio","0"...)

#ifdef EXTERNAL_DEFOPT_PARSER
#define DEF_OPTIM(PRIO, FLAGS, PAT, CST, REP)                                  \
    DEF_OPTIM_PARSE(#PRIO, #FLAGS, PAT, CST, REP)
#define DEF_OPT(PRIO, PAT, CST, REP) DEF_OPTIM_PARSE(#PRIO, "0", PAT, CST, REP)

// Some DEF_OPT use this
#define MAX_DIMENSIONS 8

// FIXME - maybe the parser should understand FROM_DEFAULT_PACKAGE("opname") directly
#define FROM_DEFAULT_PACKAGE(name) name

#endif

#endif
