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
/**
 * @file typeinfo.hpp
 * @brief 类型信息类
 * @brief 此文件定义了一套类型信息类，用于获取类型信息。rainy-toolkit通过此文件提供的接口，用于实现元编程功能
 * @brief 使用此rtti不会带来性能开销，因为它本质只是做一个静态的编译期计算，仅涉及比较基本的元编程操作
 * @author rainy-juzixiao
*/
#include <rainy/core/core.hpp>
#include <string_view>

#if RAINY_USING_MSVC
#define RAINY_GENERATE_PRETTY_FUNCTION_NAME_IMPL __FUNCSIG__
#elif RAINY_USING_CLANG || RAINY_USING_GCC
#define RAINY_GENERATE_PRETTY_FUNCTION_NAME_IMPL __PRETTY_FUNCTION__
#else
#define RAINY_GENERATE_PRETTY_FUNCTION_NAME_IMPL "unsupported compiler"
#endif

namespace rainy::foundation::rtti {
    struct traits {
        /* qualifiers */
        static RAINY_INLINE_CONSTEXPR int is_lref = 1 << 0;
        static RAINY_INLINE_CONSTEXPR int is_rref = 1 << 1;
        static RAINY_INLINE_CONSTEXPR int is_const = 1 << 2;
        static RAINY_INLINE_CONSTEXPR int is_volatile = 1 << 3;

        /* fundamental types */
        static RAINY_INLINE_CONSTEXPR int is_void = 1 << 4;
        static RAINY_INLINE_CONSTEXPR int is_nullptr_t = 1 << 5;
        static RAINY_INLINE_CONSTEXPR int is_integer = 1 << 6;
        static RAINY_INLINE_CONSTEXPR int is_floating_point = 1 << 7;
        static RAINY_INLINE_CONSTEXPR int is_unsigned = 1 << 8;
        static RAINY_INLINE_CONSTEXPR int is_arithmetic = 1 << 9;
        static RAINY_INLINE_CONSTEXPR int is_fundamental = 1 << 10;
        static RAINY_INLINE_CONSTEXPR int is_trivial = 1 << 11;

        /* pointer-related */
        static RAINY_INLINE_CONSTEXPR int is_pointer = 1 << 12;

        /* compound types */
        static RAINY_INLINE_CONSTEXPR int is_array = 1 << 13;
        static RAINY_INLINE_CONSTEXPR int is_class = 1 << 14;
        static RAINY_INLINE_CONSTEXPR int is_union = 1 << 15;
        static RAINY_INLINE_CONSTEXPR int is_enum = 1 << 16;
        static RAINY_INLINE_CONSTEXPR int is_compound = 1 << 17;

        /* function and member pointers */
        static RAINY_INLINE_CONSTEXPR int is_function = 1 << 18;
        static RAINY_INLINE_CONSTEXPR int is_function_pointer = 1 << 19;
        static RAINY_INLINE_CONSTEXPR int is_member_fnptr = 1 << 20;
        static RAINY_INLINE_CONSTEXPR int is_member_field_ptr = 1 << 21;

        /* class traits */
        static RAINY_INLINE_CONSTEXPR int is_template = 1 << 22;
        static RAINY_INLINE_CONSTEXPR int is_polymorphic = 1 << 23;
        static RAINY_INLINE_CONSTEXPR int is_abstract = 1 << 24;

        /* reflection */
        static RAINY_INLINE_CONSTEXPR int is_reflection_type = 1 << 25;
    };
}

namespace rainy::foundation::rtti::implements {
    using type_name_prober = void;

    constexpr std::string_view type_name_prober_ = "void";

    template <typename Ty>
    constexpr std::string_view wrapped_type_name() { // NOLINT
        return RAINY_GENERATE_PRETTY_FUNCTION_NAME_IMPL;
    }

    static constexpr std::size_t wrapped_type_name_prefix_length() { // NOLINT
        return wrapped_type_name<type_name_prober>().find(type_name_prober_);
    }

    static constexpr std::size_t wrapped_type_name_suffix_length() { // NOLINT
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

    template <typename Ty>
    static constexpr int eval_traits_for_properties() noexcept {
        int traits_{0};
        if constexpr (type_traits::primary_types::is_lvalue_reference_v<Ty>) {
            traits_ |= traits::is_lref;
        }
        if constexpr (type_traits::primary_types::is_rvalue_reference_v<Ty>) {
            traits_ |= traits::is_rref;
        }
        if constexpr (type_traits::type_properties::is_const_v<type_traits::reference_modify::remove_reference_t<Ty>>) {
            traits_ |= traits::is_const;
        }
        if constexpr (type_traits::type_properties::is_volatile_v<type_traits::reference_modify::remove_reference_t<Ty>>) {
            traits_ |= traits::is_volatile;
        }
        return traits_;
    }

    template <typename Ty>
    static constexpr int eval_traits_for_fundamental() noexcept {
        int traits_{0};
        if constexpr (type_traits::primary_types::is_void_v<Ty>) {
            traits_ |= traits::is_void;
        }
        if constexpr (type_traits::primary_types::is_pointer_v<type_traits::cv_modify::remove_cvref_t<Ty>> || type_traits::type_relations::is_same_v<Ty, std::nullptr_t>) {
            traits_ |= traits::is_pointer;
        }
        if constexpr (type_traits::primary_types::is_integral_v<Ty>) {
            traits_ |= traits::is_integer;
        }
        if constexpr (type_traits::type_relations::is_same_v<Ty, std::nullptr_t>) {
            traits_ |= traits::is_nullptr_t;
        }
        if constexpr (type_traits::primary_types::is_floating_point_v<Ty>) {
            traits_ |= traits::is_floating_point;
        }
        if constexpr (type_traits::type_properties::is_unsigned_v<Ty>) {
            traits_ |= traits::is_unsigned;
        }
        if constexpr (type_traits::composite_types::is_fundamental_v<Ty>) {
            traits_ |= traits::is_fundamental;
        }
        if constexpr (type_traits::type_properties::is_trivial_v<Ty>) {
            traits_ |= traits::is_trivial;
        }
        if constexpr (type_traits::composite_types::is_arithmetic_v<Ty>) {
            traits_ |= traits::is_arithmetic;
        }
        return traits_;
    }

    template <typename Ty>
    static constexpr int eval_traits_for_compound() noexcept {
        int traits_{0};
        if constexpr (type_traits::primary_types::is_class_v<type_traits::other_trans::decay_t<Ty>>) {
            traits_ |= traits::is_class;
        }
        if constexpr (type_traits::primary_types::is_template_v<type_traits::other_trans::decay_t<Ty>>) {
            traits_ |= traits::is_template;
        }
        if constexpr (type_traits::primary_types::is_function_v<Ty> || type_traits::primary_types::is_variadic_function_v<Ty>) {
            traits_ |= traits::is_function;
        }
        if constexpr (type_traits::primary_types::function_traits<Ty>::valid) {
            traits_ |= traits::is_function_pointer;
        }
        if constexpr (type_traits::composite_types::is_member_pointer_v<Ty>) {
            if constexpr (type_traits::primary_types::is_member_function_pointer_v<Ty>) {
                traits_ |= traits::is_member_fnptr;
            }
            if constexpr (!type_traits::primary_types::is_member_function_pointer_v<Ty>) {
                traits_ |= traits::is_member_field_ptr;
            }
        }
        if constexpr (type_traits::primary_types::is_array_v<Ty>) {
            traits_ |= traits::is_array;
        }
        if constexpr (type_traits::primary_types::is_enum_v<Ty>) {
            traits_ |= traits::is_enum;
        }
        if constexpr (type_traits::primary_types::is_union_v<Ty>) {
            traits_ |= traits::is_union;
        }
        if constexpr (type_traits::composite_types::is_compound_v<Ty>) {
            traits_ |= traits::is_compound;
        }
        if constexpr (type_traits::type_properties::is_polymorphic_v<Ty>) {
            traits_ |= traits::is_polymorphic;
        }
        if constexpr (type_traits::type_properties::is_abstract_v<Ty>) {
            traits_ |= traits::is_abstract;
        }
        return traits_;
    }
}
namespace rainy::foundation::rtti {
    template <typename Ty>
    constexpr std::string_view type_name() {
        constexpr auto wrapped_name = implements::wrapped_type_name<Ty>();
        constexpr auto prefix_length = implements::wrapped_type_name_prefix_length();
        constexpr auto suffix_length = implements::wrapped_type_name_suffix_length();
        constexpr auto type_name_length = wrapped_name.length() - prefix_length - suffix_length;
        return wrapped_name.substr(prefix_length, type_name_length);
    }
}

namespace rainy::foundation::rtti {
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
        return split_again.substr(0, split_again.rfind(">(void"));
#else
        static_assert(false, "You are using an unsupported compiler. Please use GCC, Clang or MSVC");
#endif
    }

    class typeinfo {
    public:
        constexpr typeinfo() = default;
        constexpr typeinfo(const typeinfo&) = default;
        constexpr typeinfo(typeinfo &&) = default;

        typeinfo &operator=(const typeinfo &) = default;
        typeinfo &operator=(typeinfo &&) = default;

        /**
         * @brief 从Ty类型信息中构造typeinfo对象
         * @tparam Ty 要获取类型信息的类型
         * @return 返回Ty对应的类型信息对象
         */
        template <typename Ty>
        static constexpr typeinfo create() {
            typeinfo info{};
            info._name = type_name<Ty>();
            info._hash_code = fnv1a_hash(info._name);
            info.traits_ |= implements::eval_traits_for_properties<Ty>();
            info.traits_ |= implements::eval_traits_for_fundamental<Ty>();
            info.traits_ |= implements::eval_traits_for_compound<Ty>();
            return info;
        }

        template <typename Ty>
        static constexpr std::size_t get_type_hash() {
            return fnv1a_hash(type_name<Ty>());
        }

        /**
         * @brief 获取Ty类型信息的静态实例的常量引用，通常由rainy_typeid宏使用（推荐）
         * @tparam Ty 要获取类型信息的类型
         */
        template <typename Ty>
        static const typeinfo &of() {
            static typeinfo instance = create<Ty>();
            return instance;
        }

        /**
         * @brief 获取类型信息中对应的名称
         */
        RAINY_NODISCARD constexpr std::string_view name() const noexcept {
            return _name;
        }

        /**
         * @brief 获取类型信息中对应的哈希值
         */
        RAINY_NODISCARD constexpr std::size_t hash_code() const noexcept {
            return _hash_code;
        }

        /**
         * @brief 检查两个类型信息是否相同
         * @param right 另一个类型信息对象
         * @return 如果类型信息相同，返回true，否则返回false
         */
        RAINY_NODISCARD constexpr bool is_same(const typeinfo &right) const noexcept {
            return hash_code() == right.hash_code();
        }

        /**
         * @brief 运算符重载，检查两个类型信息是否相同。等效于is_same()
         * @param left 要比较的左边的类型信息对象
         * @param right 要比较的右边的类型信息对象
         * @return 如果类型信息相同，返回true，否则返回false
         */
        constexpr friend bool operator==(const typeinfo &left, const typeinfo &right) noexcept {
            return left.is_same(right);
        }

        /**
         * @brief 运算符重载，检查两个类型信息是否不同。等效于!is_same()
         * @param left 要比较的左边的类型信息对象
         * @param right 要比较的右边的类型信息对象
         * @return 如果类型信息不同，返回true，否则返回false
         */
        constexpr friend bool operator!=(const typeinfo &left, const typeinfo &right) noexcept {
            return !left.is_same(right);
        }

        /**
         * @brief 运算符重载，检查left类型的兼容性。等效于is_compatible()
         * @param right 要检查的右边的类型信息对象
         * @remarks 之所以使用 <= 作为比较符号，可以这么理解。right表示它可以内化为left，就这样
         * @return 如果right的类型信息与this存储的类型信息具有兼容性，返回true，否则返回false
         */
        constexpr bool operator<=(const typeinfo &right) const noexcept {
            return is_compatible(right);
        }

        /**
         * @brief 用于比较两个类型信息是否兼容
         * @param right 要比较的右边的类型信息对象
         * @attention 在实现细节上。它并非万能的，不过此处将提及它的规则
         * @attention 首先，它将检查非const是否可以转换为const
         * @attention 接着，继续检查数值类型是否可以相互转换
         * @attention 然后，进行指针类型兼容性规则检查
         * @attention 再之后，进行nullptr_t兼容性规则检查
         * @attention 最后，进行名称字符串的比较，如果相同，则认为是“兼容”的
         * @return 如果right的类型信息与this存储的类型信息具有兼容性，返回true，否则返回false
         */
        constexpr bool is_compatible(const typeinfo &right) const noexcept {
            if (is_same(right)) {
                return true;
            }
            if (right.has_traits(traits::is_const) && !has_traits(traits::is_const)) {
                typeinfo stripped_target = right;
                stripped_target.traits_ &= ~traits::is_const;
                if (is_compatible(stripped_target)) {
                    return true;
                }
            }
            if (has_traits(traits::is_pointer) && right.has_traits(traits::is_pointer)) {
                if (has_traits(traits::is_lref) || has_traits(traits::is_rref)) {
                    foundation::rtti::typeinfo target = right;
                    if (target.has_traits(traits::is_lref) || target.has_traits(traits::is_rref)) {
                        target = target.remove_reference();
                    }
                    return this->remove_reference() == target;
                }
                std::string_view tmp = right._name;
                normalize_name(tmp);
                if (tmp == "void*") {
                    return true;
                }
            }
            if ((has_traits(traits::is_array) || right.has_traits(traits::is_array)) &&
                (has_traits(traits::is_pointer) || right.has_traits(traits::is_pointer))) {
                return true;
            }
            if (has_traits(traits::is_nullptr_t) && right.has_traits(traits::is_pointer)) {
                return true;
            }
            return this->remove_all_qualifier() == right.remove_all_qualifier();
        }

        /**
         * @brief 检查此类型信息是否具有特定的traits
         * @param traits 要检查的traits，从traits枚举中获取
         * @return 如果类型信息具有traits，返回true，否则返回false
         */
        constexpr bool has_traits(const int traits) const noexcept {
            return traits_ & traits;
        }

        /**
         * @brief 获取类型信息去除引用的类型信息
         * @return 如果类型不是引用类型，将返回自身的类型信息对象。否则，将返回引用指向的类型信息对象
         */
        RAINY_NODISCARD constexpr typeinfo remove_reference() const {
            if (!has_traits(traits::is_lref) && !has_traits(traits::is_rref)) {
                return *this;
            }
            typeinfo result = *this;
            result.traits_ &= ~(traits::is_lref | traits::is_rref);
            std::string_view name = _name;
            if (has_traits(traits::is_lref)) {
                remove_suffix(name, "&");
            } else if (has_traits(traits::is_rref)) {
                remove_suffix(name, "&&");
            }
            result._hash_code = fnv1a_hash(name);
            result._name = name;
            return result;
        }

        RAINY_NODISCARD static typeinfo create_typeinfo_by_name(std::string_view name) {
            typeinfo result;
            result._name = name;
            result._hash_code = fnv1a_hash(name);
            result.traits_ |= traits::is_reflection_type;
            return result;
        }

        RAINY_NODISCARD constexpr typeinfo remove_cvref() const noexcept {
            return remove_cv().remove_reference();
        }

        RAINY_NODISCARD constexpr typeinfo remove_cv() const noexcept {
            typeinfo result = *this;
#if RAINY_USING_CLANG || RAINY_USING_GCC
            remove_prefix(result._name, "const ");
            remove_prefix(result._name, "volatile ");
#else
            remove_prefix(result._name, "volatile ");
            remove_prefix(result._name, "const ");
#endif
            result._hash_code = fnv1a_hash(result._name);
            result.traits_ &= ~(traits::is_const);
            result.traits_ &= ~(traits::is_volatile);
            return result;
        }

        RAINY_NODISCARD constexpr typeinfo remove_pointer() const noexcept {
            typeinfo result = *this;
            if (is_pointer()) {
                remove_suffix(result._name, "*");
            }
            result._hash_code = fnv1a_hash(result._name);
            result.traits_ &= ~(traits::is_pointer);
            return result;
        }

        RAINY_NODISCARD constexpr typeinfo remove_all_qualifier() const noexcept {
            return this->remove_cvref().remove_pointer();
        }

        RAINY_NODISCARD constexpr bool is_arithmetic() const noexcept {
            return has_traits(traits::is_arithmetic);
        }

        RAINY_NODISCARD constexpr bool is_floating_point() const noexcept {
            return has_traits(traits::is_floating_point);
        }

        RAINY_NODISCARD constexpr bool is_integer() const noexcept {
            return has_traits(traits::is_integer);
        }

        RAINY_NODISCARD constexpr bool is_nullptr() const noexcept {
            return has_traits(traits::is_nullptr_t);
        }

        RAINY_NODISCARD constexpr bool is_reference() const noexcept {
            return has_traits(traits::is_lref) || has_traits(traits::is_rref);
        }

        RAINY_NODISCARD constexpr bool is_pointer() const noexcept {
            return has_traits(traits::is_pointer);
        }

    private:
        static constexpr std::size_t fnv1a_hash(std::string_view val,
                                                std::size_t offset_basis = rainy::utility::implements::fnv_offset_basis) noexcept {
            std::size_t hash = offset_basis;
            for (char i: val) {
                hash ^= static_cast<std::size_t>(static_cast<unsigned char>(i));
                hash *= rainy::utility::implements::fnv_prime;
            }
            return hash;
        }

        static constexpr void remove_prefix(std::string_view &str, std::string_view prefix) {
            if (str.size() >= prefix.size() && str.substr(0, prefix.size()) == prefix) {
                str.remove_prefix(prefix.size());
            }
        }

        static constexpr void remove_suffix(std::string_view &str, std::string_view suffix) {
            if (str.size() >= suffix.size() && str.substr(str.size() - suffix.size()) == suffix) {
                str.remove_suffix(suffix.size());
            }
        }

        static constexpr void normalize_name(std::string_view &name) {
#if RAINY_USING_CLANG || RAINY_USING_GCC
            remove_prefix(name, "const ");
            remove_prefix(name, "volatile ");
#else
            remove_prefix(name, "volatile ");
            remove_prefix(name, "const ");
#endif
            remove_suffix(name, "&");
            remove_suffix(name, "&");
            remove_suffix(name, "*");
            remove_suffix(name, " ");
        }
        
        std::string_view _name{};
        std::size_t _hash_code{};
        int traits_{};
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

// 用于获取类型信息的宏，考虑到使用传统rtti的使用习惯
#define rainy_typeid(x) ::rainy::foundation::rtti::typeinfo::of<x>()
#define rainy_typehash(x) ::rainy::foundation::rtti::typeinfo::get_type_hash<x>()

template <>
struct std::hash<rainy::foundation::rtti::typeinfo> {
    RAINY_NODISCARD std::size_t operator()(const rainy::foundation::rtti::typeinfo &val) const noexcept {
        return val.hash_code();
    }
};

#endif