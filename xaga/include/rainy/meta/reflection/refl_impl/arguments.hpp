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
#ifndef RAINY_META_REFL_IMPL_ARGUMENTS_HPP
#define RAINY_META_REFL_IMPL_ARGUMENTS_HPP
#include <rainy/core/core.hpp>
#include <rainy/meta/reflection/refl_impl/object_view.hpp>
#include <rainy/utility/any.hpp>

namespace rainy::meta::reflection::implements {
    class arg_view {
    public:
        using view = collections::views::array_view<object_view>;
        using iterator = typename view::iterator;
        using const_iterator = typename view::const_iterator;
        using reference = object_view &;
        using const_reference = const object_view &;

        arg_view() : args{} {
        }

        template <std::size_t N>
        arg_view(collections::array<object_view, N> &list) : args(list) { // NOLINT
        }

        iterator begin() noexcept {
            return args.begin();
        }

        iterator end() noexcept {
            return args.end();
        }

        RAINY_INLINE reference operator[](const std::size_t idx) {
            return args.operator[](static_cast<std::ptrdiff_t>(idx));
        }

        RAINY_INLINE const_reference &operator[](const std::size_t idx) const {
            return args.operator[](static_cast<std::ptrdiff_t>(idx));
        }

        reference at(const std::size_t idx) {
            return args.operator[](static_cast<std::ptrdiff_t>(idx));
        }

        RAINY_NODISCARD const_reference at(const std::size_t idx) const {
            return args.operator[](static_cast<std::ptrdiff_t>(idx));
        }

        RAINY_NODISCARD std::size_t size() const noexcept {
            return args.size();
        }

    private:
        view args;
    };

    template <std::size_t N>
    class arg_store {
    public:
        template <typename... Args>
        explicit arg_store([[maybe_unused]] Args &&...args_in) noexcept :
            args{make_object_view_helper(utility::forward<Args>(args_in))...} {
            static_assert(sizeof...(Args) == N, "Argument count mismatch with N");
        }

        arg_view to_argview() noexcept {
            return arg_view{args};
        }

        operator arg_view() && noexcept { // NOLINT
            return arg_view{args};
        }

    private:
        template <typename Ty>
        static object_view make_object_view_helper(Ty &&arg) noexcept {
            using namespace rainy::type_traits;
            if constexpr (primary_types::is_array_v<reference_modify::remove_reference_t<Ty>>) {
                return object_view{const_cast<void *>(static_cast<const void *>(arg)),
                                   foundation::ctti::typeinfo::of<other_trans::decay_t<Ty>>()};
            } else if constexpr (primary_types::is_pointer_reference_v<Ty>) { // NOLINT
                return object_view{const_cast<void *>(static_cast<const void *>(&arg)), foundation::ctti::typeinfo::of<Ty>()};
            } else if constexpr (primary_types::is_pointer_v<Ty>) {
                return object_view{const_cast<void *>(static_cast<const void *>(arg)), foundation::ctti::typeinfo::of<Ty>()};
            } else if constexpr (type_relations::is_same_v<other_trans::decay_t<Ty>, utility::any>) {
                return object_view{const_cast<void *>(arg.target_as_void_ptr()), arg.type()};
            } else if constexpr (type_relations::is_same_v<other_trans::decay_t<Ty>, object_view>) {
                return object_view{arg};
            } else {
                return object_view{const_cast<void *>(static_cast<const void *>(&arg)), foundation::ctti::typeinfo::of<Ty>()};
            }
        }

        collections::array<object_view, N> args;
    };

    template <typename... Args>
    arg_store(Args...) -> arg_store<sizeof...(Args)>;

    template <std::size_t N>
    class make_paramlist {
    public:
        template <typename... Args>
        explicit make_paramlist(Args &&...args_in) : types{make_paramlist_helper(utility::forward<Args>(args_in))...} {
        }

        RAINY_NODISCARD collections::views::array_view<foundation::ctti::typeinfo> get() const noexcept {
            return types;
        }

    private:
        template <typename Ty>
        static foundation::ctti::typeinfo make_paramlist_helper(Ty &&arg) noexcept {
            using namespace rainy::type_traits;
            if constexpr (primary_types::is_array_v<reference_modify::remove_reference_t<Ty>>) {
                return foundation::ctti::typeinfo::of<other_trans::decay_t<Ty>>();
            } else if constexpr (primary_types::is_pointer_reference_v<Ty>) { // NOLINT
                return foundation::ctti::typeinfo::of<Ty>();
            } else if constexpr (primary_types::is_pointer_v<Ty>) {
                return foundation::ctti::typeinfo::of<Ty>();
            } else if constexpr (type_relations::is_same_v<other_trans::decay_t<Ty>, utility::any>) {
                return arg.type();
            } else if constexpr (type_relations::is_same_v<other_trans::decay_t<Ty>, utility::any>) {
                return arg.ctti();
            } else {
                return foundation::ctti::typeinfo::of<Ty>();
            }
        }

        collections::array<foundation::ctti::typeinfo, N> types;
    };

    template <typename... Args>
    make_paramlist(Args...) -> make_paramlist<sizeof...(Args)>;

    template <typename... Args>
    class make_nondynamic_paramlist {
    public:
        make_nondynamic_paramlist() : types{make_paramlist_helper<Args>()...} {
        }

        RAINY_NODISCARD collections::views::array_view<foundation::ctti::typeinfo> get() const noexcept {
            return types;
        }

    private:
        template <typename Ty>
        static foundation::ctti::typeinfo make_paramlist_helper() noexcept {
            using namespace rainy::type_traits;
            if constexpr (primary_types::is_array_v<reference_modify::remove_reference_t<Ty>>) {
                return foundation::ctti::typeinfo::of<other_trans::decay_t<Ty>>();
            } else if constexpr (primary_types::is_pointer_reference_v<Ty>) { // NOLINT
                return foundation::ctti::typeinfo::of<Ty>();
            } else if constexpr (primary_types::is_pointer_v<Ty>) {
                return foundation::ctti::typeinfo::of<Ty>();
            } else {
                return foundation::ctti::typeinfo::of<Ty>();
            }
        }

        collections::array<foundation::ctti::typeinfo, sizeof...(Args)> types;
    };

    template <typename... Types>
    struct default_arguments_store {
        default_arguments_store() {
        }

        default_arguments_store(const default_arguments_store &) = default;
        default_arguments_store(default_arguments_store &&) = default;

        template <typename... UArgs>
        default_arguments_store(UArgs&&... args) : store(utility::forward<UArgs>(args)...) {
        }

        template <std::size_t Index>
        decltype(auto) get() noexcept {
            return std::get<Index>(store);
        }

        std::tuple<type_traits::other_trans::decay_t<Types>...> store;
    };

    template <>
    struct default_arguments_store<> {
        default_arguments_store() {
        }

        default_arguments_store(const default_arguments_store &) = default;
        default_arguments_store(default_arguments_store &&) = default;

        std::tuple<> store;
    };

    template <typename ParamList, std::size_t Index, typename FirstArg, typename... RestArgs>
    constexpr bool check_args_at_index() {
        using param_type = typename type_traits::other_trans::type_at<Index,ParamList>::type;
        constexpr bool current_compatible = std::is_convertible_v<FirstArg, param_type>;
        if constexpr (sizeof...(RestArgs) == 0) {
            return current_compatible;
        } else {
            return current_compatible && check_args_at_index<ParamList, Index + 1, RestArgs...>();
        }
    }

    template <typename ParamList, std::size_t Index>
    constexpr bool check_args_at_index() {
        return true;
    }

    template <typename ParamList, std::size_t StartIndex, typename... args>
    constexpr bool check_default_args_compatibility() {
        if constexpr (sizeof...(args) == 0) {
            return true;
        } else {
            return check_args_at_index<ParamList, StartIndex, args...>();
        }
    }
}

#endif