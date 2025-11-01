//==============================================================================
//
// Copyright (c) 2021 Qualcomm Technologies, Inc.
// All Rights Reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
//
//==============================================================================

#ifndef TYPE_NAME_H
#define TYPE_NAME_H 1

#include <array>
#include <string_view>

template <typename T> constexpr const char *type_name()
{
    return "unknown";
}
#define DEFINE_TYPENAME(TYPE, TYPENAME)                                        \
    template <> constexpr const char *type_name<TYPE>() { return TYPENAME; }

/* use DEFINE_TYPENAME to define the typename for classes
e.g.
DEFINE_TYPENAME(MyTensor8, mt8);
DEFINE_TYPENAME(MyTensor16, mt16);
*/
// DEFINE_TYPENAME(int, int);
// DEFINE_TYPENAME(float, float);

template <typename T> constexpr void AddTypeNameSize(size_t &size)
{
    std::string_view name = type_name<T>();
    size += 1; //add space for "." or "@"
    size += name.size();
}

template <typename... TYPES> constexpr size_t GetTypeNamesTotalSize()
{
    size_t size = 0;
    (AddTypeNameSize<TYPES>(size), ...);
    return size;
}

template <typename T>
constexpr void AppendTypeName(char *des, size_t &offset, size_t &duplicate,
                              size_t &left)
{
    left--;
    std::string_view name = type_name<T>();
    size_t i = offset;
    bool same = false;
    if (offset != 0) { //if not the first name
        same = true;
        des[i++] = '.'; //add delimiter
        size_t len = name.size();
        for (int j = 0; j < len; j++) {
            if (des[offset - 1 - j] != name[len - 1 - j]) {
                same = false;
                break;
            }
        }
        if (same && des[offset - len - 1] != '.' &&
            des[offset - len - 1] != '@') {
            same = false;
        }
        if (same)
            duplicate += 1;
    } else
        des[i++] = '@';
    if (!same) {
        if (offset != 0) {
            if (duplicate != 0) {
                des[i - 1] = '*';
                des[i++] = 49 + duplicate;
                des[i++] = '.';
            }
            duplicate = 0; //add delimiter
        }
        for (auto n : name)
            des[i++] = n;
        des[i] = 0;
        offset = i;
    }
    if (left == 0 && duplicate != 0) {
        des[i - 1] = '*';
        des[i++] = 49 + duplicate;
    }
}

template <typename... TYPES> constexpr auto GetTypeNames()
{
    std::array<char, GetTypeNamesTotalSize<TYPES...>() + 1> result{};
    char *des = result.data();
    size_t offset = 0;
    size_t duplicate = 0;
    size_t left = sizeof...(TYPES);
    (AppendTypeName<TYPES>(des, offset, duplicate, left), ...);
    return result;
}

template <typename TYPESTUPLE, std::size_t... I>
constexpr auto GetTypeNames(std::index_sequence<I...>)
{
    std::array<char,
               GetTypeNamesTotalSize<std::tuple_element_t<I, TYPESTUPLE>...>() +
                       1>
            result{};
    char *des = result.data();
    size_t offset = 0;
    size_t duplicate = 0;
    size_t left = sizeof...(I);
    (AppendTypeName<std::tuple_element_t<I, TYPESTUPLE>>(des, offset, duplicate,
                                                         left),
     ...);
    return result;
}

#endif
