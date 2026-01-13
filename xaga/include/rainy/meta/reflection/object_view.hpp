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
#ifndef RAINY_META_REFLECTION_OBJECT_VIEW_HPP
#define RAINY_META_REFLECTION_OBJECT_VIEW_HPP
#include <rainy/core/core.hpp>
#include <rainy/foundation/typeinfo.hpp>
#include <rainy/meta/reflection/refl_impl/type_register.hpp>
#include <rainy/foundation/memory/nebula_ptr.hpp>
#include <rainy/utility/any.hpp>

namespace rainy::meta::reflection {
    class object_view;
}

namespace rainy::meta::reflection::implements {
    struct fake_class {};

    struct static_no_definite_class_type {};

    static const auto static_type = foundation::ctti::typeinfo::create<static_no_definite_class_type>();

    template <typename Class>
    static const foundation::ctti::typeinfo &which_belongs_res() noexcept {
        if constexpr (type_traits::type_relations::is_void_v<Class>) {
            return static_type;
        } else {
            return rainy_typeid(Class);
        }
    }

    struct as_array {};
    struct as_reference {};

    template <typename Type, typename = void>
    RAINY_CONSTEXPR_BOOL has_target_as_void_ptr = false;

    template <typename Type>
    RAINY_CONSTEXPR_BOOL
        has_target_as_void_ptr<Type, type_traits::other_trans::void_t<decltype(utility::declval<Type>().target_as_void_ptr())>> = true;

    template <typename Type, typename = void>
    RAINY_CONSTEXPR_BOOL has_type_query_interface = false;

    template <typename Type>
    RAINY_CONSTEXPR_BOOL has_type_query_interface<Type, type_traits::other_trans::void_t<decltype(utility::declval<Type>().type())>> =
        true;

    template <typename Type>
    RAINY_CONSTEXPR_BOOL is_dynamic_object = has_target_as_void_ptr<Type> && has_type_query_interface<Type>;
}

namespace rainy::meta::reflection::implements {
    template <typename ObjectView = object_view>
    class arg_view {
    public:
        using view = collections::views::array_view<ObjectView>;
        using iterator = typename view::iterator;
        using const_iterator = typename view::const_iterator;
        using reference = ObjectView &;
        using const_reference = const ObjectView &;

        arg_view() : args{} {
        }

        template <std::size_t N>
        arg_view(collections::array<ObjectView, N> &list) : args(list) { // NOLINT
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

    template <std::size_t N, typename ObjectView = object_view>
    class arg_store {
    public:
        using object_view = ObjectView;

        template <typename... Args>
        explicit arg_store([[maybe_unused]] Args &&...args_in) noexcept :
            args(std::in_place, make_object_view_helper(utility::forward<Args>(args_in))...) {
            static_assert(sizeof...(Args) == N, "Argument count mismatch with N");
        }

        arg_view<> to_argview() noexcept {
            return arg_view<>{args};
        }

        operator arg_view<>() && noexcept { // NOLINT
            return arg_view{args};
        }

    private:
        template <typename Ty>
        static object_view make_object_view_helper(Ty &&arg) noexcept {
            using namespace rainy::type_traits;
            if constexpr (primary_types::is_array_v<reference_modify::remove_reference_t<Ty>>) {
                volatile auto *ptr = &arg;
                void *addr = const_cast<void *>(static_cast<const volatile void *>(ptr));
                return object_view{implements::as_array{}, addr, foundation::ctti::typeinfo::of<other_trans::decay_t<Ty>>()};
            } else if constexpr (primary_types::is_pointer_reference_v<Ty>) { // NOLINT
                return object_view{implements::as_reference{}, const_cast<void *>(static_cast<const void *>(&arg)),
                                   foundation::ctti::typeinfo::of<Ty>()};
            } else if constexpr (primary_types::is_pointer_v<Ty>) {
                return object_view{const_cast<void *>(static_cast<const void *>(&arg)), foundation::ctti::typeinfo::of<Ty>()};
            } else if constexpr (type_relations::is_same_v<other_trans::decay_t<Ty>, object_view>) {
                return object_view{arg};
            } else if constexpr (is_dynamic_object<Ty>) {
                return object_view{const_cast<void *>(arg.target_as_void_ptr()), arg.type()};
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
            } else if constexpr (implements::is_dynamic_object<Ty>) {
                return arg.type();
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
        default_arguments_store(UArgs &&...args) : store(utility::forward<UArgs>(args)...) {
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
        using param_type = typename type_traits::other_trans::type_at<Index, ParamList>::type;
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

namespace rainy::meta::reflection {
    // 用于表示不存在的实例
    struct non_exists_instance_t {};

    static constexpr inline non_exists_instance_t non_exists_instance;

    class shared_object;

    class object_view {
    public:
        template <typename Ty>
        using enable_if_t =
            type_traits::other_trans::enable_if_t<!type_traits::type_relations::is_same_v<Ty, object_view> &&
                                                      !type_traits::type_relations::is_same_v<Ty, non_exists_instance_t>,
                                                  int>;

        object_view() = default;
        object_view(std::nullptr_t) = delete;
        object_view &operator=(std::nullptr_t) = delete;

        template <typename Ty, enable_if_t<Ty> = 0>
        object_view(Ty &object) noexcept : // NOLINT
            object_{const_cast<void *>(static_cast<const void *>(utility::addressof(object)))}, ctti_{&rainy_typeid(Ty)} {
        }

        template <typename Ty, std::enable_if_t<!type_traits::type_relations::is_same_v<std::decay_t<Ty>, object_view> &&
                                                    !type_traits::type_relations::is_same_v<std::decay_t<Ty>, non_exists_instance_t> &&
                                                    std::is_rvalue_reference_v<Ty &&> && !std::is_lvalue_reference_v<Ty>,
                                                int> = 0>
        object_view(Ty &&object) : // NOLINT
            object_{const_cast<void *>(static_cast<const void *>(utility::addressof(object)))}, ctti_{&rainy_typeid(Ty &&)} {
        }

        object_view(void *const object, const foundation::ctti::typeinfo &ctti) noexcept : object_{object}, ctti_{&ctti} {
        }

        object_view(implements::as_array, void *const object, const foundation::ctti::typeinfo &ctti) noexcept :
            object_{nullptr}, ctti_{&ctti} {
            object_holder_ = object;
            object_ = static_cast<void *>(&object_holder_);
        }

        object_view(implements::as_reference, void *const object, const foundation::ctti::typeinfo &ctti) noexcept :
            object_{object}, ctti_{&ctti} {
        }

        object_view(non_exists_instance_t) noexcept : object_(nullptr), ctti_(&rainy_typeid(void)) {
        }

        object_view(object_view &&other) noexcept : object_(other.object_), ctti_(other.ctti_), object_holder_(other.object_holder_) {
            if (other.object_ == &other.object_holder_) {
                object_ = &object_holder_;
            }
        }

        object_view &operator=(object_view &&other) noexcept {
            object_ = other.object_;
            ctti_ = other.ctti_;
            object_holder_ = other.object_holder_;
            if (other.object_ == &other.object_holder_) {
                object_ = &object_holder_;
            }
            return *this;
        }

        object_view(const object_view &other) : object_(other.object_), ctti_(other.ctti_), object_holder_(other.object_holder_) {
            if (other.object_ == &other.object_holder_) {
                object_ = &object_holder_;
            }
        }

        object_view &operator=(const object_view &other) {
            object_ = other.object_;
            ctti_ = other.ctti_;
            object_holder_ = other.object_holder_;
            if (other.object_ == &other.object_holder_) {
                object_ = &object_holder_;
            }
            return *this;
        }

        template <typename Decayed, enable_if_t<Decayed> = 0>
        RAINY_NODISCARD Decayed *cast_to_pointer() noexcept {
            using remove_ref_t = type_traits::reference_modify::remove_reference_t<Decayed>;
            return const_cast<remove_ref_t *>(static_cast<const object_view *>(this)->cast_to_pointer<Decayed>());
        }

        template <typename Decayed, enable_if_t<Decayed> = 0>
        RAINY_NODISCARD const Decayed *cast_to_pointer() const noexcept {
            using namespace foundation::ctti;
            static constexpr typeinfo target_type = typeinfo::create<Decayed>();
            return type().is_compatible(target_type) ? reinterpret_cast<const Decayed *>(target_as_void_ptr()) : nullptr;
        }

        template <typename Type>
        RAINY_NODISCARD auto as() noexcept -> decltype(auto) {
            return utility::implements::as_impl<Type>(target_as_void_ptr(), type());
        }

        template <typename Type, enable_if_t<Type> = 0>
        RAINY_NODISCARD auto as() const -> decltype(auto) {
            using namespace type_traits::cv_modify;
            using ret_type = decltype(utility::declval<object_view &>().template as<Type>());
            rainy_let nonconst = const_cast<object_view *>(this);
            if constexpr (type_traits::primary_types::is_rvalue_reference_v<ret_type>) {
                return nonconst->as<type_traits::reference_modify::add_const_rvalue_ref_t<Type>>();
            } else {
                return nonconst->as<type_traits::reference_modify::add_const_lvalue_ref_t<Type>>();
            }
        }

        RAINY_NODISCARD explicit operator bool() const noexcept {
            return valid();
        }

        RAINY_NODISCARD const foundation::ctti::typeinfo &type() const noexcept {
            rainy_assume(ctti_ != nullptr);
            return *ctti_;
        }

        RAINY_NODISCARD bool valid() const noexcept {
            rainy_assume(ctti_ != nullptr);
            return !ctti_->is_same(rainy_typeid(void));
        }

        RAINY_NODISCARD void *target_as_void_ptr() noexcept {
            return object_;
        }

        RAINY_NODISCARD RAINY_INLINE const void *target_as_void_ptr() const noexcept {
            return object_;
        }

        template <typename TargetType>
        RAINY_NODISCARD TargetType *try_dynamic_cast() noexcept {
            return const_cast<TargetType *>(static_cast<const object_view *>(this)->try_dynamic_cast<TargetType>());
        }

        template <typename TargetType>
        RAINY_NODISCARD const TargetType *try_dynamic_cast() const noexcept {
            using namespace foundation::ctti;
            static constexpr typeinfo target_type = typeinfo::create<TargetType>();
            auto *result =
                static_cast<const TargetType *>(foundation::ctti::apply_offset(const_cast<void *>(object_), type(), target_type));
            if (result) {
                return result;
            }
            return static_cast<const TargetType *>(
                foundation::ctti::apply_offset(const_cast<void *>(object_), type().remove_cvref(), target_type));
        }

        template <typename... Args, typename Type = reflection::type>
        utility::any invoke(std::string_view name, Args &&...args) {
            if (!impl_) {
                impl_ = implements::register_table::get_accessor(this->type());
                if (!impl_) {
                    return {}; // 未注册
                }
            }
            auto mytype = Type(core::internal_construct_tag, static_cast<implements::type_accessor *>(impl_));
            return mytype.invoke_method(name, *this, utility::forward<Args>(args)...);
        }

        template <typename SharedObject = shared_object, typename Type = reflection::type>
        SharedObject create_shared() {
            if (!impl_) {
                impl_ = implements::register_table::get_accessor(this->type());
                if (!impl_) {
                    return {}; // 未注册
                }
            }
            auto mytype = Type(core::internal_construct_tag, static_cast<implements::type_accessor *>(impl_));
            return mytype.create(*this);
        }

    private:
        void *object_{};
        const foundation::ctti::typeinfo *ctti_{&rainy_typeid(void)};
        void *object_holder_{};
        void *impl_{};
    };

    template <typename Any,
              type_traits::other_trans::enable_if_t<
                  type_traits::type_relations::is_same_v<type_traits::cv_modify::remove_cvref_t<Any>, utility::any>, int> = 0>
    RAINY_INLINE object_view as_object_view(Any &&any) {
        return object_view{const_cast<void *>(any.target_as_void_ptr()), any.type()};
    }

    template <typename Any,
              type_traits::other_trans::enable_if_t<type_traits::type_relations::is_same_v<type_traits::cv_modify::remove_cvref_t<Any>,
                                                                                           typename utility::any::reference>,
                                                    int> = 0>
    RAINY_INLINE object_view as_object_view(Any &&any) {
        return object_view{const_cast<void *>(any.target_as_void_ptr()), any.type()};
    }
}

#endif
