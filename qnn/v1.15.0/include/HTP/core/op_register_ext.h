//==============================================================================
//
// Copyright (c) 2021 Qualcomm Technologies, Inc.
// All Rights Reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
//
//==============================================================================

#ifndef OP_REGISTER_EXT_H
#define OP_REGISTER_EXT_H

#include "graph_status.h"
#include "template_help.h"
#include "op_utils.h"
#include "op_info.h"
#include "op_registry.h"
#include "template_help_tensor_ext.h"
#include "serialize_register.h"
#include "simple_op.h"

#pragma GCC visibility push(default)

namespace hnnx {

class PackageOpStorageBase {
  public:
    const std::string op_name;
    const SimpleOpFactory simpop;
    const std::type_info &type_info;
    const Op::tensor_deserializer_register_func deserializer_reg_func;
    const deserialize_op_func deserialize_func;
    cost_function_t cost_f;
    const Flags_word flags;

    PackageOpStorageBase(const std::string_view pkg_name_in,
                         const std::string_view op_name_in,
                         const SimpleOpFactory simpop_in,
                         std::type_info const &tinf,
                         const Op::tensor_deserializer_register_func
                                 deserializer_reg_func_in,
                         const deserialize_op_func deserialize_func_in,
                         const cost_function_t cost_f_in, Flags_word flags_in);

    PackageOpStorageBase(const std::string_view pkg_name_in,
                         const std::string_view op_name_in,
                         const SimpleOpFactory simpop_in,
                         std::type_info const &tinf,
                         const Op::tensor_deserializer_register_func
                                 deserializer_reg_func_in,
                         const deserialize_op_func deserialize_func_in,
                         std::string_view cost_str_in, Flags_word flags_in);

    const OpFactory make_op_wrapper() const;
};

std::map<std::string, std::vector<std::unique_ptr<PackageOpStorageBase>> *> &
get_pkg_op_tmp_map();

int tensor_deserializer_register_ext(size_t n_out, uint8_t const *out_indices);

template <typename Derived> int tensor_deserializer_register_ext_so()
{
    return tensor_deserializer_register_ext(
            Derived::n_outputs, (Derived::output_tensor_type_indices).data());
}

#undef PACKAGEOPSTORAGE_DERIVED_PARAMS_EXT
#define PACKAGEOPSTORAGE_DERIVED_PARAMS_EXT                                    \
    typeid(Derived), tensor_deserializer_register_ext_so<Derived>,             \
            deserialize_op_using_constructor_ext<Derived>::deserialize

template <auto F, typename Derived = typename SimpleOpType<F>::type>
void add_package_op_ext(std::vector<std::unique_ptr<PackageOpStorageBase>> &ops,
                        const std::string_view op_name_in)
{
    const SimpleOpFactory so_create =
            Derived::are_tensor_types_valid ? Derived::create : nullptr;
    ops.push_back(std::make_unique<PackageOpStorageBase>(
            THIS_PKG_NAME_STR, op_name_in, so_create,
            PACKAGEOPSTORAGE_DERIVED_PARAMS_EXT, get_costf<Derived>(),
            flags_for<Derived>));
}

template <auto F, typename Derived = typename SimpleOpType<F>::type>
void add_package_op_ext(std::vector<std::unique_ptr<PackageOpStorageBase>> &ops,
                        const std::string_view op_name_in,
                        std::string_view cost_str_in, Flags_word flags_in)
{
    const SimpleOpFactory so_create =
            Derived::are_tensor_types_valid ? Derived::create : nullptr;
    ops.push_back(std::make_unique<PackageOpStorageBase>(
            THIS_PKG_NAME_STR, op_name_in, so_create,
            PACKAGEOPSTORAGE_DERIVED_PARAMS_EXT, cost_str_in, flags_in));
}

template <auto F, typename Derived = typename SimpleOpType<F>::type>
void add_package_op_ext(std::vector<std::unique_ptr<PackageOpStorageBase>> &ops,
                        const std::string_view op_name_in,
                        const cost_function_t cost_f_in, Flags_word flags_in)
{
    const SimpleOpFactory so_create =
            Derived::are_tensor_types_valid ? Derived::create : nullptr;
    ops.push_back(std::make_unique<PackageOpStorageBase>(
            THIS_PKG_NAME_STR, op_name_in, so_create,
            PACKAGEOPSTORAGE_DERIVED_PARAMS_EXT, cost_f_in, flags_in));
}

} // namespace hnnx

#pragma GCC visibility pop

#define INIT_PACKAGE_OP_DEF()                                                  \
    __attribute__((visibility("hidden")))                                      \
            std::vector<std::unique_ptr<hnnx::PackageOpStorageBase>>           \
                    &current_package_ops_storage_vec_func()                    \
    {                                                                          \
        static std::vector<std::unique_ptr<hnnx::PackageOpStorageBase>> opv;   \
        return opv;                                                            \
    }                                                                          \
    extern "C" {                                                               \
    void clear_package_ops_storage_vec_func()                                  \
    {                                                                          \
        current_package_ops_storage_vec_func().clear();                        \
    }                                                                          \
    }

#define DECLARE_PACKAGE_OP_DEF()                                               \
    __attribute__((visibility("hidden")))                                      \
            std::vector<std::unique_ptr<hnnx::PackageOpStorageBase>>           \
                    &current_package_ops_storage_vec_func();

#define REGISTER_PACKAGE_OPS()                                                 \
    if (hnnx::get_pkg_op_tmp_map().find(std::string(THIS_PKG_NAME_STR)) ==     \
        hnnx::get_pkg_op_tmp_map().end())                                      \
        hnnx::get_pkg_op_tmp_map()[std::string(THIS_PKG_NAME_STR)] =           \
                &current_package_ops_storage_vec_func();

#define DEF_PACKAGE_OP(F, OP)                                                  \
    [[maybe_unused]] static bool CTRICKS_PASTER(_PKG_OP_REG_, __LINE__) =      \
            (hnnx::add_package_op_ext<F>(                                      \
                     current_package_ops_storage_vec_func(), OP),              \
             true);

using package_cost_function_t = float (*)(Op const *);
inline float call_cost_func(package_cost_function_t func, const Op *op)
{
    return (func)(op);
}
inline float call_cost_func(std::string_view, const Op *op)
{
    return 0.0;
}
namespace hnnx {
template <auto F>
void add_package_op_ext(std::vector<std::unique_ptr<PackageOpStorageBase>> &ops,
                        const std::string_view op_name_in,
                        const package_cost_function_t cost_f_in,
                        Flags_word flags_in);
}

#define DEF_PACKAGE_OP_AND_COST_AND_FLAGS(F, OP, COST, ...)                    \
    namespace hnnx {                                                           \
    template <>                                                                \
    void add_package_op_ext<F>(                                                \
            std::vector<std::unique_ptr<PackageOpStorageBase>> & ops,          \
            const std::string_view op_name_in,                                 \
            const package_cost_function_t cost_f_in, Flags_word flags_in)      \
    {                                                                          \
        auto wrapper = [](hnnx::cost_function_t const &,                       \
                          const Op *op) -> float {                             \
            return call_cost_func(COST, op);                                   \
        };                                                                     \
        hnnx::add_package_op_ext<F>(ops, op_name_in,                           \
                                    hnnx::cost_function_t(wrapper, 1.0),       \
                                    flags_in);                                 \
    }                                                                          \
    }                                                                          \
    [[maybe_unused]] static bool CTRICKS_PASTER(_PKG_OP_REG_, __LINE__) =      \
            (hnnx::add_package_op_ext<F>(                                      \
                     current_package_ops_storage_vec_func(), OP, COST,         \
                     (hnnx::flagval_generate<__VA_ARGS__>)),                   \
             true);

DECLARE_PACKAGE_OP_DEF()

#endif // OP_REGISTER_EXT_H
