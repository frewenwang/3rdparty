//=============================================================================
//
//  Copyright (c) 2021 Qualcomm Technologies, Inc.
//  All Rights Reserved.
//  Confidential and Proprietary - Qualcomm Technologies, Inc.
//
//============================================================================

#include <cmath>

#include "HTP/core/constraints.h"
#include "HTP/core/op_package_feature_support.h"
#include "HTP/core/op_register_ext.h"
#include "HTP/core/optimize.h"
#include "QnnOpPackage.h"

static constexpr auto sg_opNameRelu  = "Relu";
static constexpr auto sg_opNameReluX = "ReluX";
static constexpr auto sg_opNameRelu1 = "Relu1";

// op execute function declarations
// op 1
template <typename T_Ttype>
int reluImpl(T_Ttype &out, const T_Ttype &in);

// op 2
template <typename T_TtypeI, typename T_TtypeX>
int reluXImpl(T_TtypeI &out, const T_TtypeI &in, const T_TtypeX &inX);

// op 3
template <typename T_TtypeI, typename T_TtypeX>
int relu1Impl(T_TtypeI &out, const T_TtypeI &in);

/*
 * op definitions
 * need to be global in the package
 * one definition per op
 */

/*
 * method 1 for defining op, using default cost value (i.e. GLACIAL) and no flag
 * syntax: DEF_PACKAGE_OP(F,OP)
 * e.g. DEF_PACKAGE_OP((reluImpl<Tensor>), sg_opNameRelu)
 */
DEF_PACKAGE_OP((reluImpl<Tensor>), sg_opNameRelu)
DEF_PACKAGE_OP((reluXImpl<Tensor, Tensor>), sg_opNameReluX)
DEF_PACKAGE_OP((relu1Impl<Tensor, Tensor>), sg_opNameRelu1)

/*
 * method 2 for defining op with specified cost value (one of "GLACIAL", "SNAIL", "FAST", "FREE")
 * and provided flags
 * syntax: DEF_PACKAGE_OP_AND_COST_AND_FLAGS(F,OP,COST,...)
 * can use zero (default) or more flags, FLAG options are IS_CONST, INHIBIT_CONST_PROP,
 * RESOURCE_HVX, RESOURCE_HMX(not supported in external op packages)
 * e.g. DEF_PACKAGE_OP_AND_COST_AND_FLAGS((reluImpl<PlainFloatTensor>), sg_opNameRelu, "SNAIL")
 */
DEF_PACKAGE_OP_AND_COST_AND_FLAGS((reluImpl<PlainFloatTensor>), sg_opNameRelu, "SNAIL")

DEF_PACKAGE_OP_AND_COST_AND_FLAGS((reluXImpl<PlainFloatTensor, PlainFloatTensor>),
                                  sg_opNameReluX,
                                  "SNAIL")
DEF_PACKAGE_OP_AND_COST_AND_FLAGS((relu1Impl<PlainFloatTensor, PlainFloatTensor>),
                                  sg_opNameRelu1,
                                  "SNAIL")

/*
 * method 3 for defining op with cost function pointer and provided flags
 * cost function pointer type: typedef float (*cost_function) (const Op * op);
 * syntax: DEF_PACKAGE_OP_AND_COST_AND_FLAGS(F,OP,COST_F,...)
 * e.g. DEF_PACKAGE_OP_AND_COST_AND_FLAGS((reluImpl<PlainFloatTensor>),
 * sg_opNameRelu, reluCostFunc, Flags::RESOURCE_HVX)
 */

/*
 * optimization definitions
 * need to be global in the package
 * one definition per optimization
 * syntax: DEF_PACKAGE_OPTIMIZATION(PRIORITY,MATCHCODE,CONSTRAINTCODE,REPLACECODE)
 * PRIORITY predefined values include EARLY(2000), MIDDLE(3000), LATE(4000)
 * HTP core provides some replacement functions for op package to use
 * for more information about optimization rules, please refer to HTP core documentations
 */

/* execute functions for ops */

// op 1 Relu
template <typename T_Ttype>
int reluImpl(T_Ttype &out, const T_Ttype &in) {
  debuglog("relu execute... dims=(%zdx%zdx%zdx%zd)", in.dim(0), in.dim(1), in.dim(2), in.dim(3));
  debuglog("in=%p out=%p", &in, &out);
  out.set_dims(in);
  float fyiMax = 0.0f;
  for (Idx b = 0; b < in.dim(0); b++) {
    for (Idx h = 0; h < in.dim(1); h++) {
      for (Idx w = 0; w < in.dim(2); w++) {
        for (Idx d = 0; d < in.dim(3); d++) {
          float inval     = in(b, h, w, d);
          fyiMax          = fmaxf(fyiMax, inval);
          out(b, h, w, d) = fmaxf(inval, 0.0f);
        }
      }
    }
  }
  return GraphStatus::Success;
}

// op 2 ReluX has range [0.0f, X]
template <typename T_TtypeI, typename T_TtypeX>
int reluXImpl(T_TtypeI &out, const T_TtypeI &in, const T_TtypeX &inX) {
  debuglog("relux execute... dims=(%zdx%zdx%zdx%zd)", in.dim(0), in.dim(1), in.dim(2), in.dim(3));
  debuglog("in=%p out=%p", &in, &out);
  float x = inX(0, 0, 0, 0);

  if (!(x > 0.0f)) {
    errlog("reluX limit %f not > 0", x);
    return GraphStatus::ErrorFatal;
  }

  out.set_dims(in);
  for (Idx b = 0; b < in.dim(0); b++) {
    for (Idx h = 0; h < in.dim(1); h++) {
      for (Idx w = 0; w < in.dim(2); w++) {
        for (Idx d = 0; d < in.dim(3); d++) {
          float inval     = in(b, h, w, d);
          out(b, h, w, d) = fminf(fmaxf(inval, 0.0f), x);
        }
      }
    }
  }
  return GraphStatus::Success;
}

// op 3 Relu1 has range [-1.0f, 1.0f]
template <typename T_TtypeI, typename T_TtypeX>
int relu1Impl(T_TtypeI &out, const T_TtypeI &in) {
  debuglog("relux execute... dims=(%zdx%zdx%zdx%zd)", in.dim(0), in.dim(1), in.dim(2), in.dim(3));
  debuglog("in=%p out=%p", &in, &out);

  out.set_dims(in);
  for (Idx b = 0; b < in.dim(0); b++) {
    for (Idx h = 0; h < in.dim(1); h++) {
      for (Idx w = 0; w < in.dim(2); w++) {
        for (Idx d = 0; d < in.dim(3); d++) {
          float inval     = in(b, h, w, d);
          out(b, h, w, d) = fminf(fmaxf(inval, -1.0f), 1.0f);
        }
      }
    }
  }
  return GraphStatus::Success;
}
