/*
 * Copyright 2026 rainy-juzixiao
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
#define RAINY_FOUNDATION_TYPEINFO_HPP // NOLINT

#include <functional>
#include <rainy/core/collections/array_view.hpp>
#include <rainy/core/collections/vector.hpp>
#include <rainy/core/platform.hpp>
#include <rainy/core/text/string_view.hpp>
#include <rainy/core/type_traits.hpp>

#if RAINY_USING_CLANG || RAINY_USING_LLVM_GCC // workaround...
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunknown-warning-option"
#pragma GCC diagnostic ignored "-Wenum-constexpr-conversion"
#endif

namespace rainy::core {
    class typeinfo;
}

namespace rainy::core {
    enum class traits {
        /* qualifiers */
        is_lref = 1 << 0,
        is_rref = 1 << 1,
        is_const = 1 << 2,
        is_volatile = 1 << 3,

        /* fundamental types */
        is_void = 1 << 4,
        is_nullptr_t = 1 << 5,
        is_integer = 1 << 6,
        is_floating_point = 1 << 7,
        is_unsigned = 1 << 8,
        is_arithmetic = 1 << 9,
        is_fundamental = 1 << 10,
        is_trivial = 1 << 11,

        /* pointer-related */
        is_pointer = 1 << 12,

        /* compound types */
        is_array = 1 << 13,
        is_class = 1 << 14,
        is_union = 1 << 15,
        is_enum = 1 << 16,
        is_compound = 1 << 17,

        /* function and member pointers */
        is_function = 1 << 18,
        is_function_pointer = 1 << 19,
        is_member_fnptr = 1 << 20,
        is_member_field_ptr = 1 << 21,

        /* class traits */
        is_template = 1 << 22,
        is_polymorphic = 1 << 23,
        is_abstract = 1 << 24,

        /* reflection */
        is_signed = 1 << 25,
        is_sequential_container = 1 << 26,
        is_associative_container = 1 << 27,
    };

    // @NODOCBEGIN
    RAINY_ENABLE_ENUM_CLASS_BITMASK_OPERATORS(traits);
    // @NODOCEND
}

// @NODOCBEGIN
namespace rainy::core::implements {
    using type_name_prober = void;

    constexpr text::string_view type_name_prober_ = "void";

    template <typename Ty>
    constexpr rain_fn wrapped_type_name() -> text::string_view { // NOLINT
#if RAINY_USING_MSVC
        return __FUNCSIG__;
#elif RAINY_USING_CLANG || RAINY_USING_GCC
        return __PRETTY_FUNCTION__;
#else
        static_assert(false, "unsupported compiler");
#endif
    }

    constexpr rain_fn wrapped_type_name_prefix_length() -> std::size_t { // NOLINT
        return wrapped_type_name<type_name_prober>().find(type_name_prober_);
    }

    constexpr rain_fn wrapped_type_name_suffix_length() -> std::size_t { // NOLINT
        return wrapped_type_name<type_name_prober>().length() - wrapped_type_name_prefix_length() - type_name_prober_.length();
    }

    template <auto Variable>
    constexpr rain_fn wrapped_variable_name() -> text::string_view {
#if RAINY_USING_CLANG || RAINY_USING_GCC
        return __PRETTY_FUNCTION__;
#elif RAINY_USING_MSVC
        return __FUNCSIG__;
#else
        static_assert(false, "unsupported compiler");
#endif
    }
}

namespace rainy::core::implements {
    template <typename Ty, typename = void>
    struct is_associative_container : type_traits::helper::false_type {};

    template <typename Ty>
    struct is_associative_container<Ty, type_traits::other_trans::void_t<typename Ty::key_type, typename Ty::mapped_type>>
        : type_traits::helper::true_type {};

    template <typename Ty>
    inline constexpr bool is_associative_container_v = is_associative_container<Ty>::value;

    template <typename Ty, typename = void>
    struct is_sequential_container : type_traits::helper::false_type {};

    template <typename Ty>
    struct is_sequential_container<Ty,
                                   type_traits::other_trans::void_t<typename Ty::value_type, decltype(utility::declval<Ty>().begin())>>
        : type_traits::logical_traits::negation<is_associative_container<Ty>> {};

    template <typename Ty>
    inline constexpr bool is_sequential_container_v = is_sequential_container<Ty>::value;
}

namespace rainy::core::implements {
    template <typename Ty>
    constexpr rain_fn eval_traits_for_properties() noexcept -> traits {
        traits traits_{0};

        if constexpr (type_traits::primary_types::is_lvalue_reference_v<Ty>) {
            traits_ |= traits::is_lref;
        }
        if constexpr (type_traits::primary_types::is_rvalue_reference_v<Ty>) {
            traits_ |= traits::is_rref;
        }

        using base_type = type_traits::modifers::remove_reference_t<Ty>;
        if constexpr (std::is_const_v<base_type>) {
            traits_ |= traits::is_const;
        }
        if constexpr (std::is_volatile_v<base_type>) {
            traits_ |= traits::is_volatile;
        }

        using decay_type = type_traits::other_trans::decay_t<Ty>;
        if constexpr (is_associative_container_v<decay_type>) {
            traits_ |= traits::is_associative_container;
        }
        if constexpr (is_sequential_container_v<decay_type>) {
            traits_ |= traits::is_sequential_container;
        }

        return traits_;
    }

    template <typename Ty>
    constexpr rain_fn eval_traits_for_fundamental() noexcept -> traits {
        traits traits_{0};

        if constexpr (type_traits::type_relations::is_void_v<Ty>) {
            traits_ |= traits::is_void;
        }

        if constexpr (type_traits::primary_types::is_pointer_v<rainy::type_traits::modifers::remove_cvref_t<Ty>> || type_traits::type_relations::is_same_v<Ty, std::nullptr_t>) {
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

        if constexpr (type_traits::properties::is_unsigned_v<Ty>) {
            traits_ |= traits::is_unsigned;
        }
        if constexpr (type_traits::properties::is_signed_v<Ty>) {
            traits_ |= traits::is_signed;
        }

        if constexpr (type_traits::composite_types::is_fundamental_v<Ty>) {
            traits_ |= traits::is_fundamental;
        }

        if constexpr (type_traits::properties::is_trivially_default_constructible_v<Ty> && type_traits::properties::is_trivially_copyable_v<Ty>) {
            traits_ |= traits::is_trivial;
        }

        if constexpr (type_traits::composite_types::is_arithmetic_v<Ty>) {
            traits_ |= traits::is_arithmetic;
        }

        return traits_;
    }

    template <typename Ty>
    constexpr rain_fn eval_traits_for_compound() noexcept -> traits {
        traits traits_{0};

        using decay_type = type_traits::other_trans::decay_t<Ty>;

        if constexpr (type_traits::primary_types::is_class_v<decay_type>) {
            traits_ |= traits::is_class;
        }

        if constexpr (rainy::type_traits::extras::templates::is_template_v<decay_type>) {
            traits_ |= traits::is_template;
        }

        if constexpr (std::is_function_v<Ty>) {
            traits_ |= traits::is_function;
        }

        if constexpr (std::is_pointer_v<Ty> && std::is_function_v<type_traits::modifers::remove_pointer_t<Ty>>) {
            traits_ |= traits::is_function_pointer;
        }

        if constexpr (std::is_member_pointer_v<Ty>) {
            if constexpr (std::is_member_function_pointer_v<Ty>) {
                traits_ |= traits::is_member_fnptr;
            } else {
                traits_ |= traits::is_member_field_ptr;
            }
        }

        if constexpr (type_traits::primary_types::is_array_v<Ty>) {
            traits_ |= traits::is_array;
        }

        if constexpr (std::is_enum_v<Ty>) {
            traits_ |= traits::is_enum;
        }

        if constexpr (std::is_union_v<Ty>) {
            traits_ |= traits::is_union;
        }

        if constexpr (std::is_compound_v<Ty>) {
            traits_ |= traits::is_compound;
        }

        if constexpr (std::is_polymorphic_v<Ty>) {
            traits_ |= traits::is_polymorphic;
        }

        if constexpr (std::is_abstract_v<Ty>) {
            traits_ |= traits::is_abstract;
        }

        return traits_;
    }

    template <typename Ty>
    constexpr rain_fn eval_for_typeinfo() noexcept -> traits {
        traits traits{}; // NOLINT
        traits |= implements::eval_traits_for_properties<Ty>();
        traits |= implements::eval_traits_for_fundamental<Ty>();
        traits |= implements::eval_traits_for_compound<Ty>();
        return traits;
    }

#if RAINY_HAS_CXX26 && RAINY_HAS_CXX26_STATIC_REFLECTION
    template <typename Ty>
    consteval rain_fn generate_type_name() -> text::string_view {
        return std::meta::display_string_of(^^Ty);
    }
#else
    template <typename Ty>
    constexpr rain_fn make_type_name_array() -> auto {
        constexpr auto wrapped_name = implements::wrapped_type_name<Ty>();
        constexpr auto prefix_length = implements::wrapped_type_name_prefix_length();
        constexpr auto suffix_length = implements::wrapped_type_name_suffix_length();
        constexpr auto type_name_length = wrapped_name.length() - prefix_length - suffix_length;
        std::array<char, type_name_length + 1> arr{};
        for (std::size_t i = 0; i < type_name_length; ++i) {
            arr[i] = wrapped_name[prefix_length + i]; // NOLINT
        }
        arr[type_name_length] = '\0'; // NOLINT
        return arr;
    }

    template <typename Ty>
    constexpr auto type_name_array = make_type_name_array<Ty>();

    template <typename Ty>
    constexpr rain_fn generate_type_name() -> text::string_view {
        return {type_name_array<Ty>.data(), type_name_array<Ty>.size() - 1};
    }
#endif

#if RAINY_HAS_CXX26 && RAINY_HAS_CXX26_STATIC_REFLECTION
    template <auto Variable>
    constexpr auto generate_variable_name() -> text::string_view {
        text::string_view raw_name = {};
        if constexpr (constexpr auto r = std::meta::reflect_constant(Variable); std::meta::is_enumerator(r)) {
            raw_name = std::meta::identifier_of(r);
        } else if constexpr (std::meta::has_identifier(r)) {
            raw_name = std::meta::identifier_of(r);
        } else {
            if constexpr (std::is_enum_v<decltype(Variable)>) {
                raw_name = std::meta::display_string_of(r);
            } else {
                constexpr text::string_view full = std::meta::display_string_of(r);
                constexpr auto last_dot = full.rfind('.');
                constexpr text::string_view after_dot = (last_dot != text::string_view::npos && last_dot + 1 < full.size())
                                                            ? full.substr(last_dot + 1)
                                                            : (last_dot != text::string_view::npos ? text::string_view{} : full);
                constexpr auto last_sep = after_dot.rfind("::");
                constexpr text::string_view name = (last_sep != text::string_view::npos && last_sep + 2 <= after_dot.size())
                                                       ? after_dot.substr(last_sep + 2)
                                                       : after_dot;
                raw_name = name;
            }
        }

        bool is_parenthesized = false;

        if (!raw_name.empty() && raw_name[0] == '(') {
            if (auto last_rparen = raw_name.rfind(')'); last_rparen != text::string_view::npos && last_rparen + 1 < raw_name.size()) {
                is_parenthesized = true;
            }
        }

        if (is_parenthesized) {
            return raw_name;
        }

        auto last_dot = raw_name.rfind('.');
        auto last_arrow = raw_name.rfind("->");
        auto last_colon = raw_name.rfind("::");

        if (last_dot == text::string_view::npos) {
            last_dot = 0;
        }

        if (last_arrow == text::string_view::npos) {
            last_arrow = 0;
        }

        if (last_colon == text::string_view::npos) {
            last_colon = 0;
        }

        auto last_sep = (std::max) ({last_dot, last_arrow, last_colon});

        if (last_sep != 0) {
            auto sep_len = (last_sep == last_arrow || last_sep == last_colon) ? 2 : 1;
            if (last_sep + sep_len <= raw_name.size()) {
                return raw_name.substr(last_sep + sep_len);
            }
            return {};
        }

        return raw_name;
    }
#else
    template <auto Variable>
    static constexpr rain_fn make_variable_name_ref() -> text::string_view {
        constexpr text::string_view func_name = wrapped_variable_name<Variable>();
#if RAINY_USING_CLANG || RAINY_USING_LLVM_GCC
        constexpr auto split = func_name.substr(0, func_name.rfind(']'));
        constexpr auto start = split.find("Variable = ") + 11;
        constexpr auto end = split.find(';', start);
        constexpr auto raw_name = split.substr(start, end - start);
        constexpr auto dot_pos = raw_name.rfind('.');
        if constexpr (dot_pos != text::string_view::npos) {
            return raw_name.substr(dot_pos + 1);
        } else {
            return raw_name;
        }
#elif RAINY_USING_GCC
        constexpr auto split = func_name.substr(0, func_name.rfind(']') - 1);
        constexpr auto start = split.find("with auto Variable = ") + 21;
        constexpr auto end = split.find(';', start);
        constexpr auto full = split.substr(start, end - start);
        constexpr auto pos = full.rfind("::");
        constexpr auto rparen = full.rfind(')');

        constexpr auto is_parenthesized = [&]() constexpr {
            if (!full.empty() && full[0] == '(') {
                if constexpr (constexpr auto last_rparen = full.rfind(')');
                              last_rparen != text::string_view::npos && last_rparen + 1 < full.size()) {
                    return true;
                }
            }
            return false;
        }();

        if constexpr (is_parenthesized) {
            return full; // NOLINT
        }

        if constexpr (pos != text::string_view::npos) {
            return full.substr(pos + 2, rparen - (pos + 2));
        }
        return full; // NOLINT
#elif RAINY_USING_MSVC
        auto bracket_start = func_name.rfind('<');
        auto bracket_end = func_name.rfind(">}");
        if (bracket_end == text::string_view::npos) {
            bracket_end = func_name.rfind('}');
        }
        if (bracket_end == text::string_view::npos) {
            bracket_end = func_name.rfind('>');
        }
        if (bracket_start == text::string_view::npos || bracket_end == text::string_view::npos) {
            return "";
        }
        if (bracket_start + 1 >= bracket_end) {
            return "";
        }
        auto content = func_name.substr(bracket_start + 1, bracket_end - bracket_start - 1);
        if (content.empty()) {
            return "";
        }
        bool is_parenthesized = false;
        if (content[0] == '(') {
            auto last_rparen = content.rfind(')');
            if (last_rparen != text::string_view::npos && last_rparen + 1 < content.size()) {
                is_parenthesized = true;
            }
        }
        if (is_parenthesized) {
            return content;
        }
        auto last_dot = content.rfind('.');
        auto last_arrow = content.rfind("->");
        auto last_colon = content.rfind("::");
        auto last_sep = text::string_view::npos;
        if (last_dot != text::string_view::npos) {
            last_sep = last_dot;
        }
        if (last_arrow != text::string_view::npos && last_arrow > last_sep) {
            last_sep = last_arrow;
        }
        if (last_colon != text::string_view::npos && last_colon > last_sep) {
            last_sep = last_colon;
        }
        if (last_sep != text::string_view::npos) {
            auto sep_len = (last_sep == last_arrow || last_sep == last_colon) ? 2 : 1;
            if (last_sep + sep_len <= content.size()) {
                return content.substr(last_sep + sep_len);
            }
            return content;
        }
        return content;
#else
        static_assert(false, "Unsupported compiler");
#endif
    }

    template <auto Variable>
    constexpr rain_fn make_variable_name_array() -> auto {
        constexpr auto name_sv = make_variable_name_ref<Variable>();
#if RAINY_USING_CLANG || RAINY_USING_LLVM_GCC
        if constexpr (constexpr bool has_anonymous = name_sv.find("(anonymous namespace)") != text::string_view::npos) {
            constexpr text::string_view target = "(anonymous namespace)";
            constexpr text::string_view replacement = "<unnamed>";
            constexpr std::size_t target_len = target.size();
            constexpr std::size_t replacement_len = replacement.size();

            constexpr std::size_t count = [&]() constexpr {
                std::size_t c = 0;
                for (std::size_t i = 0; i <= name_sv.size() - target_len;) {
                    if (name_sv.substr(i, target_len) == target) {
                        ++c;
                        i += target_len;
                    } else {
                        ++i;
                    }
                }
                return c;
            }();

            constexpr std::size_t new_size = name_sv.size() + count * (replacement_len - target_len);
            collections::array<char, new_size> arr{};

            std::size_t src_idx = 0;
            std::size_t dst_idx = 0;
            while (src_idx < name_sv.size()) {
                if (src_idx <= name_sv.size() - target_len && name_sv.substr(src_idx, target_len) == target) {
                    for (std::size_t i = 0; i < replacement_len; ++i) {
                        arr[dst_idx++] = replacement[i];
                    }
                    src_idx += target_len;
                } else {
                    arr[dst_idx++] = name_sv[src_idx++];
                }
            }
            return arr;
        } else
#endif
        {
            collections::array<char, name_sv.size()> arr{};
            for (std::size_t i = 0; i < name_sv.size(); ++i) {
                arr[i] = name_sv[i];
            }
            return arr;
        }
    }

    template <auto Variable>
    static constexpr auto variable_name_array = make_variable_name_array<Variable>();

    template <auto Variable>
    constexpr rain_fn generate_variable_name() -> text::string_view {
        return {variable_name_array<Variable>.data(), variable_name_array<Variable>.size()};
    }
#endif

    RAINY_INLINE constexpr std::size_t fnv1a_hash(const text::string_view val) noexcept {
        constexpr std::size_t fnv_offset_basis = 14695981039346656037ULL;
        std::size_t hash = fnv_offset_basis;
        for (const char i: val) {
            constexpr std::size_t fnv_prime = 1099511628211ULL;
            hash ^= static_cast<std::size_t>(static_cast<unsigned char>(i));
            hash *= fnv_prime;
        }
        return hash;
    }
}

// @NODOCEND

// @NODOCBEGIN
namespace rainy::core::implements {
#define raw_type_id(x) typeinfo_component::typehash<x>()

    enum class type_operation {
        remove_pointer,
        remove_const,
        remove_volatile,
        remove_const_volatile,
        remove_cvref,
        remove_reference,
        decay_type
    };

    template <typename MainTypeInfo>
    struct typeinfo_component {
        using is_comptaible_fnptr_t = bool (*)(const typeinfo_component &);
        using type_modifer = const typeinfo_component *(*) (type_operation);

        constexpr typeinfo_component() = default;

        template <typename TypeList, typename Ty>
        struct template_argument_generater {};

        template <typename... Types, typename Ty>
        struct template_argument_generater<rainy::type_traits::other_trans::type_list<Types...>, Ty> {
            static constexpr std::array<MainTypeInfo, sizeof...(Types)> value = {Ty::template create<Types>()...};
        };

        template <typename Ty>
        static rain_fn constexpr make()->typeinfo_component {
            typeinfo_component raw;
            constexpr text::string_view name = generate_type_name<Ty>();
            constexpr std::size_t eval_hash_code = fnv1a_hash(name);
            constexpr traits traits = implements::eval_for_typeinfo<Ty>();
            raw.name = name;
            raw.hash_code = eval_hash_code;
            raw.type_traits = traits;
            raw.is_comptaible = &is_compatible_impl<Ty>;
            raw.modfier = &type_modfier_impl<Ty>;
            if constexpr (!type_traits::primary_types::is_void_v<Ty> && rainy::type_traits::properties::is_complete_v<Ty>) {
                raw.size_of_the_type = sizeof(Ty);
                raw.align_of_the_type = alignof(Ty);
            }
            if constexpr (rainy::type_traits::extras::templates::template_traits<
                              rainy::type_traits::modifers::remove_cvref_t<Ty>>::value) {
                raw.template_arguemnts = template_argument_generater<typename rainy::type_traits::extras::templates::template_traits<
                                                                         rainy::type_traits::modifers::remove_cvref_t<Ty>>::types,
                                                                     MainTypeInfo>::value;
            }
            return raw;
        }

        template <typename Ty>
        static rain_fn constexpr typehash()->std::size_t {
            constexpr text::string_view name = generate_type_name<Ty>();
            constexpr std::size_t eval_hash_code = fnv1a_hash(name);
            return eval_hash_code;
        }

        template <typename Type>
        static constexpr rain_fn is_compatible_impl(const typeinfo_component &type) -> bool;

        template <typename Ty>
        static constexpr rain_fn type_modfier_impl(type_operation op) -> const typeinfo_component<MainTypeInfo> *; // NOLINT

        static constexpr rain_fn empty_is_compatible(const typeinfo_component &) -> bool;

        static constexpr rain_fn empty_type_modfier(const typeinfo_component &) -> bool;

        text::string_view name{};
        std::size_t hash_code{};
        traits type_traits{}; // NOLINT
        is_comptaible_fnptr_t is_comptaible{};
        type_modifer modfier{};
        rainy::collections::views::array_view<MainTypeInfo> template_arguemnts{};
        std::size_t size_of_the_type{0};
        std::size_t align_of_the_type{0};
    };

    template <typename MainTypeInfo>
    constexpr typeinfo_component<MainTypeInfo> empty_component;

    template <typename MainTypeInfo, typename Ty>
    inline constexpr typeinfo_component<MainTypeInfo> typeinfo = typeinfo_component<MainTypeInfo>::template make<Ty>();

    template <typename MainTypeInfo>
    constexpr rain_fn typeinfo_component<MainTypeInfo>::empty_is_compatible(const typeinfo_component &) -> bool {
        return false;
    }

    template <typename MainTypeInfo>
    constexpr rain_fn typeinfo_component<MainTypeInfo>::empty_type_modfier(const typeinfo_component &) -> bool {
        return false;
    }

    template <typename MainTypeInfo>
    template <typename Ty>
    constexpr rain_fn typeinfo_component<MainTypeInfo>::type_modfier_impl(const type_operation op)
        -> const typeinfo_component<MainTypeInfo> * { // NOLINT
        constexpr bool is_reference_ptr = type_traits::composite_types::is_reference_v<Ty> &&
                                          type_traits::primary_types::is_pointer_v<type_traits::modifers::remove_reference_t<Ty>>;
        if constexpr (!type_traits::type_relations::is_void_v<Ty>) {
            switch (op) {
                case type_operation::remove_pointer: {
                    if constexpr (is_reference_ptr) {
                        using referred_ptr = type_traits::modifers::remove_reference_t<Ty>;
                        using pointer_type = type_traits::modifers::remove_pointer_t<referred_ptr>;
                        if constexpr (type_traits::primary_types::is_lvalue_reference_v<Ty>) {
                            return &typeinfo<MainTypeInfo, type_traits::modifers::add_lvalue_reference<pointer_type>>;
                        } else {
                            return &typeinfo<MainTypeInfo, type_traits::modifers::add_rvalue_reference<pointer_type>>;
                        }
                    } else {
                        return &typeinfo<MainTypeInfo, type_traits::modifers::remove_pointer_t<Ty>>;
                    }
                }
                case type_operation::remove_const: {
                    if constexpr (is_reference_ptr) {
                        using referred_ptr = type_traits::modifers::remove_reference_t<Ty>;
                        using pointer_type = type_traits::modifers::remove_pointer_t<referred_ptr>;
                        using non_const_pointer = type_traits::modifers::remove_const_t<pointer_type>;
                        using non_const_ptr = type_traits::modifers::add_pointer_t<non_const_pointer>;
                        if constexpr (type_traits::primary_types::is_lvalue_reference_v<Ty>) {
                            return &typeinfo<MainTypeInfo, type_traits::modifers::add_lvalue_reference<non_const_ptr>>;
                        } else {
                            return &typeinfo<MainTypeInfo, type_traits::modifers::add_rvalue_reference<non_const_ptr>>;
                        }
                    } else {
                        return &typeinfo<MainTypeInfo, type_traits::modifers::remove_const_t<Ty>>;
                    }
                }
                case type_operation::remove_volatile: {
                    if constexpr (is_reference_ptr) {
                        using referred_ptr = type_traits::modifers::remove_reference_t<Ty>;
                        using pointer_type = type_traits::modifers::remove_pointer_t<referred_ptr>;
                        using non_volatile_pointer = type_traits::modifers::remove_volatile_t<pointer_type>;
                        using non_volatile_ptr = type_traits::modifers::add_pointer_t<non_volatile_pointer>;
                        if constexpr (type_traits::primary_types::is_lvalue_reference_v<Ty>) {
                            return &typeinfo<MainTypeInfo, type_traits::modifers::add_lvalue_reference<non_volatile_ptr>>;
                        } else {
                            return &typeinfo<MainTypeInfo, type_traits::modifers::add_rvalue_reference<non_volatile_ptr>>;
                        }
                    } else {
                        return &typeinfo<MainTypeInfo, type_traits::modifers::remove_volatile_t<Ty>>;
                    }
                }
                case type_operation::remove_const_volatile: {
                    if constexpr (is_reference_ptr) {
                        using referred_ptr = type_traits::modifers::remove_reference_t<Ty>;
                        using pointer_type = type_traits::modifers::remove_pointer_t<referred_ptr>;
                        using non_cv_pointer = type_traits::modifers::remove_cv_t<pointer_type>;
                        using non_cv_ptr = type_traits::modifers::add_pointer_t<non_cv_pointer>;
                        if constexpr (type_traits::primary_types::is_lvalue_reference_v<Ty>) {
                            return &typeinfo<MainTypeInfo, type_traits::modifers::add_lvalue_reference<non_cv_ptr>>;
                        } else {
                            return &typeinfo<MainTypeInfo, type_traits::modifers::add_rvalue_reference<non_cv_ptr>>;
                        }
                    } else {
                        return &typeinfo<MainTypeInfo, type_traits::modifers::remove_cv_t<Ty>>;
                    }
                }
                case type_operation::remove_cvref: {
                    if constexpr (is_reference_ptr) {
                        using referred_ptr = type_traits::modifers::remove_reference_t<Ty>;
                        using pointer_type = type_traits::modifers::remove_pointer_t<referred_ptr>;
                        using non_cv_pointer = type_traits::modifers::remove_cv_t<pointer_type>;
                        using non_cv_ptr = type_traits::modifers::add_pointer_t<non_cv_pointer>;
                        return &typeinfo<MainTypeInfo, non_cv_ptr>;
                    } else {
                        return &typeinfo<MainTypeInfo, type_traits::modifers::remove_cvref_t<Ty>>;
                    }
                }
                case type_operation::remove_reference: {
                    return &typeinfo<MainTypeInfo, type_traits::modifers::remove_reference_t<Ty>>;
                }
                case type_operation::decay_type: {
                    return &typeinfo<MainTypeInfo, type_traits::other_trans::decay_t<Ty>>;
                }
            }
        }
        return &empty_component<MainTypeInfo>;
    }

    template <typename MainTypeInfo>
    template <typename Type>
    constexpr rain_fn typeinfo_component<MainTypeInfo>::is_compatible_impl(const typeinfo_component<MainTypeInfo> &type)
        -> bool { // NOLINT
        using match_t = rainy::type_traits::modifers::remove_cvref_t<Type>;
        using real_convert_type = type_traits::other_trans::conditional_t<type_traits::composite_types::is_reference_v<Type>, Type,
                                                                          type_traits::modifers::add_lvalue_reference<Type>>;
        if constexpr (type_traits::primary_types::is_void_v<rainy::type_traits::modifers::remove_cvref_t<Type>>) {
            return false;
        } else if constexpr (type_traits::primary_types::is_null_pointer_v<match_t>) {
            return type.hash_code == raw_type_id(std::nullptr_t);
        } else {
            using ftraits = type_traits::primary_types::function_traits<match_t>;
            if constexpr (ftraits::valid) {
                return raw_type_id(match_t) == type.hash_code;
            } else {
                switch (type.hash_code) {
                    case raw_type_id(match_t):
                        return true;
                    case raw_type_id(match_t &):
                        return type_traits::type_relations::is_convertible_v<match_t &, real_convert_type>;
                    case raw_type_id(match_t &&):
                        return type_traits::type_relations::is_convertible_v<match_t &&, real_convert_type>;
                    case raw_type_id(const match_t):
                        return true;
                    case raw_type_id(const match_t &):
                        return type_traits::type_relations::is_convertible_v<const match_t &, real_convert_type>;
                    case raw_type_id(const match_t &&):
                        return type_traits::type_relations::is_convertible_v<const match_t &&, real_convert_type>;
                    case raw_type_id(const volatile match_t):
                        return true;
                    case raw_type_id(const volatile match_t &):
                        return type_traits::type_relations::is_convertible_v<const volatile match_t &, real_convert_type>;
                    case raw_type_id(const volatile match_t &&):
                        return type_traits::type_relations::is_convertible_v<const volatile match_t &&, real_convert_type>;
                    default:
                        return false;
                }
            }
        }
    }
#undef raw_type_id
}

// @NODOCEND

namespace rainy::core {
    /**
     * \lang english
     * @brief Gets a name from a type.
     * @tparam Ty The type to obtain
     * @returns A string view representing the name of the compile-time variable
     *
     * \lang simp-chinese
     * @brief 通过类型获取一个名称
     * @tparam Ty 要获取的类型
     * @returns 一个字符串视图，表示对编译时变量的名称
     */
    template <typename Ty>
    constexpr text::string_view type_name() {
        return implements::generate_type_name<Ty>();
    }

    /**
     * \lang english
     * @brief Gets a variable name that can be determined at compile time.
     * @tparam Variable A constant variable value
     * @returns A string view representing the name of the compile-time variable
     *
     * \lang simp-chinese
     * @brief 获取一个可在编译时确定的变量名
     * @tparam Variable 一个常量变量值
     * @returns 一个字符串视图，表示对编译时变量的名称
     */
    template <auto Variable>
    constexpr text::string_view variable_name() noexcept {
        constexpr auto name = implements::generate_variable_name<Variable>();
        return name;
    }
}

#define RAINY_GENERATE_TYPEINFO_MODIFY_METHOD_HELPER(function_name, operation)                                                        \
    RAINY_NODISCARD constexpr rain_fn function_name() const noexcept -> typeinfo {                                                    \
        typeinfo ret;                                                                                                                 \
        ret.internal_type = internal_type->modfier(operation);                                                                        \
        ret.cache_name = ret.internal_type->name;                                                                                     \
        ret.cache_hash_code = ret.internal_type->hash_code;                                                                           \
        return ret;                                                                                                                   \
    }

#define RAINY_GENERATE_TYPEINFO_TYPEINSPECT_METHOD_HELPER(function_name, traits)                                                      \
    RAINY_NODISCARD constexpr rain_fn function_name() const noexcept -> bool {                                                        \
        return has_traits(traits);                                                                                                    \
    }

namespace rainy::core {
    /**
     * \lang english
     * @brief
     * An abstract type information supporting compile-time evaluation.
     * Can be used to query type information.
     *
     * \lang simp-chinese
     * @brief
     * 支持编译期计算的类型信息抽象
     * 可用于查询类型信息
     */
    class typeinfo {
    public:
        constexpr typeinfo() noexcept = default;

        constexpr typeinfo(const typeinfo &) noexcept = default;

        constexpr typeinfo(typeinfo &&) noexcept = default;

        constexpr typeinfo &operator=(const typeinfo &) noexcept = default;

        constexpr typeinfo &operator=(typeinfo &&) noexcept = default;

        /**
         * \lang english
         * @brief Constructs a typeinfo object from the Ty type information.
         * @tparam Ty The type whose type information is to be obtained
         * @return Returns the type information object corresponding to Ty
         *
         * \lang simp-chinese
         * @brief 从Ty类型信息中构造typeinfo对象
         * @tparam Ty 要获取类型信息的类型
         * @return 返回Ty对应的类型信息对象
         */
        template <typename Ty>
        static constexpr rain_fn create() noexcept -> typeinfo {
            typeinfo type;
            type.internal_type = &implements::typeinfo<typeinfo, Ty>;
            type.cache_name = type.internal_type->name;
            type.cache_hash_code = type.internal_type->hash_code;
            return type;
        }

        /**
         * \lang english
         * @brief Gets the hash value of the Ty type information.
         * @tparam Ty The type whose type information is to be obtained
         * @return Returns the hash value of the type information
         *
         * \lang simp-chinese
         * @brief 获取Ty类型信息的哈希值
         * @tparam Ty 要获取类型信息的类型
         * @return 返回该类型信息的哈希值
         */
        template <typename Ty>
        static constexpr rain_fn get_type_hash() noexcept -> std::size_t {
            return implements::fnv1a_hash(type_name<Ty>());
        }

        /**
         * \lang english
         * @brief Gets a constant reference to the static instance of the Ty type information, usually used by the rainy_typeid macro (recommended).
         * @tparam Ty The type whose type information is to be obtained
         * @return A reference to a static read-only lifetime object pointing to the corresponding type information
         *
         * \lang simp-chinese
         * @brief 获取Ty类型信息的静态实例的常量引用，通常由rainy_typeid宏使用（推荐）
         * @tparam Ty 要获取类型信息的类型
         * @return 一个指向对应类型信息的static只读生命周期对象的引用
         */
        template <typename Ty>
        static rain_fn of() noexcept -> const typeinfo & {
            static typeinfo instance = create<Ty>();
            return instance;
        }

        /**
         * \lang english
         * @brief Gets the name corresponding to the type information.
         *
         * \lang simp-chinese
         * @brief 获取类型信息中对应的名称
         */
        RAINY_NODISCARD constexpr rain_fn name() const noexcept -> text::string_view {
            return cache_name;
        }

        /**
         * \lang english
         * @brief Gets the hash value corresponding to the type information.
         *
         * \lang simp-chinese
         * @brief 获取类型信息中对应的哈希值
         */
        RAINY_NODISCARD constexpr rain_fn hash_code() const noexcept -> std::size_t {
            return cache_hash_code;
        }

        /**
         * \lang english
         * @brief Checks whether two type information objects are the same.
         * @param right Another type information object
         * @return Returns true if the type information is the same, otherwise false
         *
         * \lang simp-chinese
         * @brief 检查两个类型信息是否相同
         * @param right 另一个类型信息对象
         * @return 如果类型信息相同，返回true，否则返回false
         */
        RAINY_NODISCARD constexpr rain_fn is_same(const typeinfo &right) const noexcept -> bool {
            return hash_code() == right.hash_code();
        }

        /**
         * \lang english
         * @brief Checks whether the type information is void.
         * @return Returns true if the type information is void, otherwise false
         *
         * \lang simp-chinese
         * @brief 检查两个类型信息是否为void
         * @return 如果类型信息是void，返回true，否则返回false
         */
        RAINY_NODISCARD constexpr rain_fn is_void() const noexcept -> bool {
            return hash_code() == get_type_hash<void>();
        }

        /**
         * \lang english
         * @brief Operator overload to check whether two type information objects are the same. Equivalent to is_same().
         * @param left The left type information object to compare
         * @param right The right type information object to compare
         * @return Returns true if the type information is the same, otherwise false
         *
         * \lang simp-chinese
         * @brief 运算符重载，检查两个类型信息是否相同。等效于is_same()
         * @param left 要比较的左边的类型信息对象
         * @param right 要比较的右边的类型信息对象
         * @return 如果类型信息相同，返回true，否则返回false
         */
        constexpr friend rain_fn operator==(const typeinfo &left, const typeinfo &right) noexcept -> bool {
            return left.is_same(right);
        }

        /**
         * \lang english
         * @brief Operator overload to check whether two type information objects are different. Equivalent to !is_same().
         * @param left The left type information object to compare
         * @param right The right type information object to compare
         * @return Returns true if the type information is different, otherwise false
         *
         * \lang simp-chinese
         * @brief 运算符重载，检查两个类型信息是否不同。等效于!is_same()
         * @param left 要比较的左边的类型信息对象
         * @param right 要比较的右边的类型信息对象
         * @return 如果类型信息不同，返回true，否则返回false
         */
        constexpr friend rain_fn operator!=(const typeinfo &left, const typeinfo &right) noexcept -> bool {
            return !left.is_same(right);
        }

        /**
         * \lang english
         * @brief Performs a remove-const operation on the current type.
         * @return Returns the processed type information
         *
         * \lang simp-chinese
         * @brief 对当前类型执行移除const属性操作.
         * @return 返回被处理后的类型信息
         */
        RAINY_GENERATE_TYPEINFO_MODIFY_METHOD_HELPER(remove_const, implements::type_operation::remove_const);

        /**
         * \lang english
         * @brief Performs a remove-volatile operation on the current type.
         * @return Returns the processed type information
         *
         * \lang simp-chinese
         * @brief 对当前类型执行移除volatile属性操作.
         * @return 返回被处理后的类型信息
         */
        RAINY_GENERATE_TYPEINFO_MODIFY_METHOD_HELPER(remove_volatile, implements::type_operation::remove_volatile);

        /**
         * \lang english
         * @brief Performs a remove-const-and-volatile operation on the current type.
         * @return Returns the processed type information
         *
         * \lang simp-chinese
         * @brief 对当前类型执行移除const和volatile属性操作.
         * @return 返回被处理后的类型信息
         */
        RAINY_GENERATE_TYPEINFO_MODIFY_METHOD_HELPER(remove_cv, implements::type_operation::remove_const_volatile);

        /**
         * \lang english
         * @brief Performs a remove-reference operation on the current type.
         * @return Returns the processed type information
         *
         * \lang simp-chinese
         * @brief 对当前类型执行移除reference属性操作.
         * @return 返回被处理后的类型信息
         */
        RAINY_GENERATE_TYPEINFO_MODIFY_METHOD_HELPER(remove_reference, implements::type_operation::remove_reference);

        /**
         * \lang english
         * @brief Performs a remove-const-volatile-and-reference operation on the current type.
         * @return Returns the processed type information
         *
         * \lang simp-chinese
         * @brief 对当前类型执行移除const、volatile以及reference属性操作.
         * @return 返回被处理后的类型信息
         */
        RAINY_GENERATE_TYPEINFO_MODIFY_METHOD_HELPER(remove_cvref, implements::type_operation::remove_cvref);

        /**
         * \lang english
         * @brief Performs a remove-pointer operation on the current type.
         * @return Returns the processed type information
         *
         * \lang simp-chinese
         * @brief 对当前类型执行移除指针属性操作.
         * @return 返回被处理后的类型信息
         */
        RAINY_GENERATE_TYPEINFO_MODIFY_METHOD_HELPER(remove_pointer, implements::type_operation::remove_pointer);

        /**
         * \lang english
         * @brief Performs a decay operation on the current type.
         * @return Returns the processed type information
         *
         * \lang simp-chinese
         * @brief 对当前类型执行decay操作.
         * @return 返回被处理后的类型信息
         */
        RAINY_GENERATE_TYPEINFO_MODIFY_METHOD_HELPER(decay, implements::type_operation::decay_type);

        /**
         * \lang english
         * @brief Checks whether the current type information is compatible with another type information.
         * @param right The other type information to compare
         * @return Returns true if the type information is compatible, otherwise false
         *
         * \lang simp-chinese
         * @brief 检查当前类型信息是否与另一个类型信息兼容
         * @param right 要比较的另一个类型信息
         * @return 如果类型信息兼容，返回true，否则返回false
         */
        RAINY_NODISCARD constexpr rain_fn is_compatible(const typeinfo &right) const noexcept -> bool {
            return internal_type->is_comptaible(*right.internal_type);
        }

        /**
         * \lang english
         * @brief Checks whether this type information has specific traits.
         * @param traits The traits to check, obtained from the traits enum
         * @return Returns true if the type information has the traits, otherwise false
         *
         * \lang simp-chinese
         * @brief 检查此类型信息是否具有特定的traits
         * @param traits 要检查的traits，从traits枚举中获取
         * @return 如果类型信息具有traits，返回true，否则返回false
         */
        RAINY_NODISCARD constexpr rain_fn has_traits(const traits &traits) const noexcept -> bool {
            return static_cast<bool>(internal_type->type_traits & traits);
        }

        /**
         * \lang english
         * @brief Gets the sizeof size of the current type.
         * @return Returns the sizeof size of the current type
         *
         * \lang simp-chinese
         * @brief 获取当前类型的sizeof大小
         * @return 返回当前类型的sizeof大小
         */
        RAINY_NODISCARD constexpr rain_fn sizeof_the_type() const noexcept -> std::size_t {
            return internal_type->size_of_the_type;
        }

        /**
         * \lang english
         * @brief Attempts to get the template instantiation arguments of the current type.
         * @attention For templates with NTTP parameters such as template <std::size_t>, they cannot be obtained; only pure type templates are supported
         * @return Returns a list view of the template instantiation argument types
         *
         * \lang simp-chinese
         * @brief 尝试获取当前类型的模板实例化参数
         * @attention 对于 template <std::size_t> 这类带有NTTP参数的模板，则无法获取，仅支持纯类型的模板
         * @return 返回模板实例化参数类型的列表视图
         */
        RAINY_NODISCARD constexpr rain_fn template_arguments() const noexcept -> rainy::collections::views::array_view<typeinfo> {
            return internal_type->template_arguemnts;
        }

        /**
         * \lang english
         * @brief Used to adapt into switch(x) to implement compile-time type matching.
         *
         * \lang simp-chinese
         * @brief 用于适配到switch(x)中，实现编译时类型匹配
         */
        operator std::size_t() const noexcept { // NOLINT
            return cache_hash_code;
        }

        /**
         * \lang english
         * @brief Checks whether the current type is an arithmetic type.
         * @return Returns true if it is, otherwise false
         *
         * \lang simp-chinese
         * @brief 检查当前是否为算数类型.
         * @return 如果是返回true，否则false
         */
        RAINY_GENERATE_TYPEINFO_TYPEINSPECT_METHOD_HELPER(is_arithmetic, traits::is_arithmetic);

        /**
         * \lang english
         * @brief Checks whether the current type is a floating-point type.
         * @return Returns true if it is, otherwise false
         *
         * \lang simp-chinese
         * @brief 检查当前是否为浮点类型.
         * @return 如果是返回true，否则false
         */
        RAINY_GENERATE_TYPEINFO_TYPEINSPECT_METHOD_HELPER(is_floating_point, traits::is_floating_point);

        /**
         * \lang english
         * @brief Checks whether the current type is an integer type.
         * @return Returns true if it is, otherwise false
         *
         * \lang simp-chinese
         * @brief 检查当前是否为整型类型.
         * @return 如果是返回true，否则false
         */
        RAINY_GENERATE_TYPEINFO_TYPEINSPECT_METHOD_HELPER(is_integer, traits::is_integer);

        /**
         * \lang english
         * @brief Checks whether the current type is a null pointer type.
         * @return Returns true if it is, otherwise false
         *
         * \lang simp-chinese
         * @brief 检查当前是否为空指针类型.
         * @return 如果是返回true，否则false
         */
        RAINY_GENERATE_TYPEINFO_TYPEINSPECT_METHOD_HELPER(is_nullptr, traits::is_nullptr_t);

        /**
         * \lang english
         * @brief Checks whether the current type is a reference type (either lvalue or rvalue).
         * @return Returns true if it is, otherwise false
         *
         * \lang simp-chinese
         * @brief 检查当前是否为引用类型（包括左或右）.
         * @return 如果是返回true，否则false
         */
        RAINY_GENERATE_TYPEINFO_TYPEINSPECT_METHOD_HELPER(is_reference, traits::is_lref | traits::is_rref);

        /**
         * \lang english
         * @brief Checks whether the current type is a pointer type.
         * @return Returns true if it is, otherwise false
         *
         * \lang simp-chinese
         * @brief 检查当前是否为指针类型.
         * @return 如果是返回true，否则false
         */
        RAINY_GENERATE_TYPEINFO_TYPEINSPECT_METHOD_HELPER(is_pointer, traits::is_pointer);

        /**
         * \lang english
         * @brief Checks whether the current type is an lvalue reference type.
         * @return Returns true if it is, otherwise false
         *
         * \lang simp-chinese
         * @brief 检查当前是否为左引用类型.
         * @return 如果是返回true，否则false
         */
        RAINY_GENERATE_TYPEINFO_TYPEINSPECT_METHOD_HELPER(is_lvalue_reference, traits::is_lref);

        /**
         * \lang english
         * @brief Checks whether the current type is an rvalue reference type.
         * @return Returns true if it is, otherwise false
         *
         * \lang simp-chinese
         * @brief 检查当前是否为右引用类型.
         * @return 如果是返回true，否则false
         */
        RAINY_GENERATE_TYPEINFO_TYPEINSPECT_METHOD_HELPER(is_rvalue_reference, traits::is_rref);

        /**
         * \lang english
         * @brief Checks whether the current type is a const type.
         * @return Returns true if it is, otherwise false
         *
         * \lang simp-chinese
         * @brief 检查当前是否为const类型.
         * @return 如果是返回true，否则false
         */
        RAINY_GENERATE_TYPEINFO_TYPEINSPECT_METHOD_HELPER(is_const, traits::is_const);

        /**
         * \lang english
         * @brief Checks whether the current type is a volatile type.
         * @return Returns true if it is, otherwise false
         *
         * \lang simp-chinese
         * @brief 检查当前是否为volatile类型.
         * @return 如果是返回true，否则false
         */
        RAINY_GENERATE_TYPEINFO_TYPEINSPECT_METHOD_HELPER(is_volatile, traits::is_volatile);

        /**
         * \lang english
         * @brief Checks whether the current type is a mixed const volatile type.
         * @return Returns true if it is, otherwise false
         *
         * \lang simp-chinese
         * @brief 检查当前是否为const volatile混合的类型.
         * @return 如果是返回true，否则false
         */
        RAINY_GENERATE_TYPEINFO_TYPEINSPECT_METHOD_HELPER(is_const_volatile, traits::is_const | traits::is_volatile);

        /**
         * \lang english
         * @brief Checks whether the current type is an associative container type.
         * @return Returns true if it is, otherwise false
         *
         * \lang simp-chinese
         * @brief 检查当前是否为关联容器类型.
         * @return 如果是返回true，否则false
         */
        RAINY_GENERATE_TYPEINFO_TYPEINSPECT_METHOD_HELPER(is_associative_container, traits::is_associative_container);

        /**
         * \lang english
         * @brief Checks whether the current type is a sequential container type.
         * @return Returns true if it is, otherwise false
         *
         * \lang simp-chinese
         * @brief 检查当前是否为序列容器类型.
         * @return 如果是返回true，否则false
         */
        RAINY_GENERATE_TYPEINFO_TYPEINSPECT_METHOD_HELPER(is_sequential_container, traits::is_sequential_container);

        /**
         * \lang english
         * @brief Checks whether the current type is a class type.
         * @return Returns true if it is, otherwise false
         *
         * \lang simp-chinese
         * @brief 检查当前是否为类类型.
         * @return 如果是返回true，否则false
         */
        RAINY_GENERATE_TYPEINFO_TYPEINSPECT_METHOD_HELPER(is_class, traits::is_class);

    private:
        text::string_view cache_name{};
        std::size_t cache_hash_code{};
        const implements::typeinfo_component<typeinfo> *internal_type{&implements::empty_component<typeinfo>};
    };
}

#undef RAINY_GENERATE_TYPEINFO_MODIFY_METHOD_HELPER
#undef RAINY_GENERATE_TYPEINFO_TYPEINSPECT_METHOD_HELPER

/**
 * \lang english
 * @brief A macro used to obtain type information, taking into account the usage habits of traditional RTTI.
 *
 * \lang simp-chinese
 * @brief 用于获取类型信息的宏，考虑到使用传统rtti的使用习惯
 */
#define rainy_typeid(x) ::rainy::core::typeinfo::of<x>()

/**
 * \lang english
 * @brief Used for type switch matching.
 * @param x The type branch to match
 *
 * \lang simp-chinese
 * @brief 用于类型switch匹配
 * @param x 待匹配的类型分支
 */
#define rainy_typehash(x) ::rainy::core::typeinfo::get_type_hash<x>()

/**
 * \lang english
 * @brief A specialization template defined for std::hash.
 * @spec_template rainy::core::typeinfo
 *
 * \lang simp-chinese
 * @brief 为std::hash定义的特化模板
 * @spec_template rainy::core::typeinfo
 */
template <>
struct std::hash<rainy::core::typeinfo> {
    RAINY_NODISCARD std::size_t operator()(const rainy::core::typeinfo &val) const noexcept {
        return val.hash_code();
    }
};


namespace rainy::core::implements {
    template <typename Type>
    struct derived_info {
        void *ptr;
        Type type;
    };

    template <typename Type>
    using derived_func = derived_info<Type> (*)(void *);

    using converter_func = void* (*)(void *);

    struct conversion_key {
        bool operator==(const conversion_key &right) const {
            return source_hash == right.source_hash && target_hash == right.target_hash;
        }

        std::size_t source_hash;
        std::size_t target_hash;
    };

    struct conversion_key_hash {
        std::size_t operator()(const conversion_key &k) const {
            return k.source_hash ^ (k.target_hash + 0x9e3779b9 + (k.source_hash << 6) + (k.source_hash >> 2));
        }
    };

    struct type_upcast_info {
        collections::vector<std::size_t> base_hashes;
        collections::vector<converter_func> converters;
    };

    RAINY_TOOLKIT_API void register_direct_base(const std::size_t derived, const std::size_t base, converter_func fn);

    template <typename Derived, typename Base>
    void register_base() {
        static_assert(type_traits::type_relations::is_base_of_v<Base, Derived>);
        converter_func up_fn = [](void *ptr) -> void * { return static_cast<Base *>(static_cast<Derived *>(ptr)); };
        register_direct_base(typeinfo::get_type_hash<Derived>(), typeinfo::get_type_hash<Base>(), up_fn);
    }

    RAINY_TOOLKIT_API bool is_upcast_reachable(std::size_t from_hash, std::size_t to_hash);

    RAINY_TOOLKIT_API void *apply_offset(void *ptr, const rainy::core::typeinfo &source, const rainy::core::typeinfo &target);
}

namespace rainy::core::implements {
    using converter_fn = bool (*)(void *dest, const void *source, const class typeinfo &type);

    /// @hide
    class RAINY_TOOLKIT_API dynamic_converter_registry {
    public:
        static dynamic_converter_registry &instance();

        void register_converter(const class typeinfo &to_type, converter_fn fn); // NOLINT

        converter_fn find(const class typeinfo &) const; // NOLINT

    private:
        /// @hide
        class impl;

        impl *global_ptr{nullptr};
    };
}

namespace rainy::utility {
    template <typename TargetType, typename = void>
    struct any_converter {
        static constexpr bool invalid_mark = true;

        static bool is_convertible(const core::typeinfo &) {
            return false;
        }
    };
}

namespace rainy::utility::implements {
    /// @hide
    template <typename>
    struct get_any_converter_target_type {
        using type = void;
    };

    /// @hide
    template <typename TargetType, typename Void>
    struct get_any_converter_target_type<any_converter<TargetType, Void>> {
        using type = TargetType;
    };
}

namespace rainy::core {
    /**
     * \lang english
     * @brief Enables a class to register a converter for a target type.
     * @tparam ConverterClass The static type description class
     *
     * \lang simp-chinese
     * @brief 使一个类能够为目标类型注册转换器
     * @tparam ConverterClass 静态类型描述类
     */
    template <typename ConverterClass>
    class enable_for_type_convert {
    public:
        virtual ~enable_for_type_convert() = default;

        enable_for_type_convert() = default;

    private:
        using converter_func = bool (*)(void *dest, const void *source, const typeinfo &type);
        using target_type = typename utility::implements::get_any_converter_target_type<ConverterClass>::type;

        template <typename TargetType = target_type>
        static bool get_converter_func_invoker(void *dest, const void *source, const typeinfo &type) {
            if (!ConverterClass::is_convertible(type)) {
                return false;
            }
            if constexpr (!type_traits::composite_types::is_reference_v<target_type>) {
                if (dest && source) {
                    *static_cast<type_traits::modifers::remove_const_t<TargetType> *>(dest) =
                        ConverterClass::basic_convert(source, type);
                }
            }
            return true;
        }

        struct inject {
            inject() {
                constexpr converter_func func = &get_converter_func_invoker;
                implements::dynamic_converter_registry::instance().register_converter(rainy_typeid(target_type), func);
            }
        };

#if RAINY_USING_MSVC
        virtual void *rainy_toolkit_touch_register() {
            return &inject_;
        }
#endif

#if RAINY_USING_GCC || RAINY_USING_CLANG
        inline static inject inject_ __attribute__((used));
#else
        inline static inject inject_;
#endif
    };
}

namespace rainy::core {
    /**
     * \lang english
     * @brief Dynamically converts a value from one type to another.
     * @tparam Target The target type to convert to
     * @param src Pointer to the source data
     * @param src_type Type information of the source data
     * @return Returns the converted value or reference
     *
     * \lang simp-chinese
     * @brief 将一个值从一种类型动态转换为另一种类型
     * @tparam Target 要转换到的目标类型
     * @param src 指向源数据的指针
     * @param src_type 源数据的类型信息
     * @return 返回转换后的值或引用
     */
    template <typename Target>
    rain_fn dynamic_convert(const void *src, const typeinfo &src_type) -> decltype(auto) {
        if constexpr (type_traits::composite_types::is_reference_v<std::remove_cv_t<Target>>) {
            rainy_let ptr = const_cast<void *>(src);
            if constexpr (type_traits::primary_types::is_lvalue_reference_v<Target>) {
                if constexpr (type_traits::properties::is_const_v<type_traits::modifers::remove_reference_t<Target>>) {
                    return *static_cast<const type_traits::modifers::remove_reference_t<Target> *>(ptr);
                } else {
                    return *static_cast<type_traits::modifers::remove_reference_t<Target> *>(ptr);
                }
            } else if constexpr (type_traits::primary_types::is_rvalue_reference_v<Target>) {
                if constexpr (type_traits::properties::is_const_v<type_traits::modifers::remove_reference_t<Target>>) {
                    return utility::move(*static_cast<const type_traits::modifers::remove_reference_t<Target> *>(ptr));
                } else {
                    return utility::move(*static_cast<type_traits::modifers::remove_reference_t<Target> *>(ptr));
                }
            } else {
                return *static_cast<Target *>(ptr);
            }
        } else {
            auto fn = implements::dynamic_converter_registry::instance().find(rainy_typeid(Target));
            if (!fn) {
                std::terminate();
            }
            rainy::type_traits::modifers::remove_cvref_t<Target> result{};
            fn(&result, src, src_type);
            return result;
        }
    }

    /**
     * \lang english
     * @brief Checks whether a value can be converted from one type information to another.
     * @param from Type information of the source type
     * @param to Type information of the target type
     * @return Returns true if convertible, otherwise false
     *
     * \lang simp-chinese
     * @brief 检查一个值是否可以由一种类型信息转换为另一种类型信息
     * @param from 源类型的类型信息
     * @param to 目标类型的类型信息
     * @return 如果可以转换，返回true，否则返回false
     */
    RAINY_INLINE bool is_convertible_to(const typeinfo &from, const typeinfo &to) noexcept {
        const auto &reg = implements::dynamic_converter_registry::instance();
        if (from.remove_cvref() == to.remove_cvref()) {
            return true;
        }
        if (const auto p = reg.find(to); p != nullptr) {
            return p(nullptr, nullptr, from);
        }
        return false;
    }
}

/**
 * \lang english
 *
 * @brief Used to quickly register the inheritance relationship of a class.
 * @param Derived The derived class type
 * @param Base The base class type
 *
 * \lang simp-chinese
 *
 * @brief 用于快捷注册一个类的继承关系
 * @param Derived 派生类类型
 * @param Base 基类类型
 */
#define RAINY_REGISTER_BASE(Derived, Base) ::rainy::core::register_base<Derived, Base>()

#if RAINY_USING_CLANG || RAINY_USING_LLVM_GCC
#pragma GCC diagnostic pop
#endif

#endif
