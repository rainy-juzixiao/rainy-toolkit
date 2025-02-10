/*
 * Copyright 2025 rainy-juzixiao
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef RAINY_FOUNDATION_TYPEINFO_HPP
#define RAINY_FOUNDATION_TYPEINFO_HPP

#include <rainy/core/core.hpp>
#include <rainy/text/format_wrapper.hpp>
#include <string_view>

#if RAINY_USING_MSVC
#define RAINY_GENERATE_PRETTY_FUNCTION_NAME_IMPL __FUNCSIG__
#elif RAINY_USING_CLANG || RAINY_USING_GCC
#define RAINY_GENERATE_PRETTY_FUNCTION_NAME_IMPL __PRETTY_FUNCTION__
#else
#define RAINY_GENERATE_PRETTY_FUNCTION_NAME_IMPL "unsupported compiler"
#endif

namespace rainy::foundation::rtti::internals {
    using type_name_prober = void;

    constexpr std::string_view type_name_prober_ = "void";

    template <typename Ty>
    constexpr std::string_view wrapped_type_name() {
        return RAINY_GENERATE_PRETTY_FUNCTION_NAME_IMPL;
    }

    static constexpr std::size_t wrapped_type_name_prefix_length() {
        return wrapped_type_name<type_name_prober>().find(type_name_prober_);
    }

    static constexpr std::size_t wrapped_type_name_suffix_length() {
        return wrapped_type_name<type_name_prober>().length() - wrapped_type_name_prefix_length() - type_name_prober_.length();
    }

    template <auto Variable>
    constexpr std::string_view wrapped_variable_name() {
#if RAINY_USING_CLANG || RAINY_USING_GCC
        return __PRETTY_FUNCTION__;
#elif RAINY_USING_MSVC
        return __FUNCSIG__;
#else
        static_assert(false, "unsupported compiler");
#endif
        // 仅支持Clang、MSVC、GCC编译器。因为其它的编译器不在本库的支持范围内。考虑其它实现
    }
}

namespace rainy::foundation::rtti {
    template <typename Ty>
    constexpr std::string_view type_name() {
        constexpr auto wrapped_name = internals::wrapped_type_name<Ty>();
        constexpr auto prefix_length = internals::wrapped_type_name_prefix_length();
        constexpr auto suffix_length = internals::wrapped_type_name_suffix_length();
        constexpr auto type_name_length = wrapped_name.length() - prefix_length - suffix_length;
        return wrapped_name.substr(prefix_length, type_name_length);
    }

    template <auto Variable>
    inline constexpr std::string_view variable_name() {
#if RAINY_USING_MSVC
        constexpr std::string_view func_name = __FUNCSIG__;
#else
        constexpr std::string_view func_name = __PRETTY_FUNCTION__;
#endif
#if RAINY_USING_CLANG
        auto split = func_name.substr(0, func_name.size() - 2);
        return split.substr(split.find_last_of(":.") + 1);
#elif RAINY_USING_GCC
        auto split = func_name.substr(0, func_name.rfind(")}"));
        return split.substr(split.find_last_of(':') + 1);
#elif RAINY_USING_MSVC
        auto split = func_name.substr(func_name.rfind("variable_name<") + 13);
        auto split_again = split.substr(split.rfind("->") + 2);
        return split_again.substr(0, split_again.rfind(">(void)"));
#else
        static_assert(false, "You are using an unsupported compiler. Please use GCC, Clang or MSVC");
#endif
    }

    class typeinfo {
    public:
        template <typename Ty>
        static constexpr typeinfo create() {
            typeinfo info{};
            info._name = type_name<Ty>();
            info._hash_code = fnv1a_hash(info._name);
            return info;
        }

        template <typename Ty>
        static const typeinfo &of() {
            static const typeinfo instance = create<Ty>();
            return instance;
        }

        RAINY_NODISCARD constexpr std::string_view name() const noexcept {
            return _name;
        }

        RAINY_NODISCARD constexpr std::size_t hash_code() const noexcept {
            return _hash_code;
        }

        RAINY_NODISCARD constexpr bool is_same(const typeinfo &right) const noexcept {
            return hash_code() == right.hash_code();
        }

        constexpr friend bool operator==(const typeinfo &left, const typeinfo &right) noexcept {
            return left.is_same(right);
        }

        constexpr friend bool operator!=(const typeinfo &left, const typeinfo &right) noexcept {
            return !left.is_same(right);
        }

    private:
        constexpr typeinfo() = default;

        static constexpr std::size_t fnv1a_hash(std::string_view val,
                                                std::size_t offset_basis = rainy::utility::internals::fnv_offset_basis) noexcept {
            std::size_t hash = offset_basis;
            for (char i : val) {
                hash ^= static_cast<std::size_t>(static_cast<unsigned char>(i));
                hash *= rainy::utility::internals::fnv_prime;
            }
            return hash;
        }

        std::string_view _name{};
        std::size_t _hash_code{};
    };
}

template <>
struct rainy::utility::hash<rainy::foundation::rtti::typeinfo> {
    using argument_type = foundation::rtti::typeinfo;
    using result_type = std::size_t;

    static size_t hash_this_val(const argument_type &val) noexcept {
        return val.hash_code();
    }
};

#define rainy_typeid(x) ::rainy::foundation::rtti::typeinfo::of<x>()

template <>
struct std::hash<rainy::foundation::rtti::typeinfo> {
    RAINY_NODISCARD std::size_t operator()(const rainy::foundation::rtti::typeinfo &val) const noexcept {
        return val.hash_code();
    }
};

#endif