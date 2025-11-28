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
#ifndef RAINY_META_REFL_IMPL_INVOKER_ACCESSOR_HPP
#define RAINY_META_REFL_IMPL_INVOKER_ACCESSOR_HPP
#include <rainy/meta/reflection/refl_impl/invoker.hpp>

namespace rainy::meta::reflection::implements {
    template <typename Fx, bool IsFnObj = is_fnobj<Fx>::value>
    struct function_signature {
        using type = Fx;
    };

    template <typename Fx>
    struct function_signature<Fx, true> {
        using traits = type_traits::primary_types::function_traits<Fx>;
        using return_type = typename traits::return_type;
        using type_list = typename type_traits::other_trans::tuple_like_to_type_list<typename traits::tuple_like_type>::type;

        template <typename TypeList>
        struct extractor {};

        template <typename... Args>
        struct extractor<type_traits::other_trans::type_list<Args...>> {
            using type =
                type_traits::other_trans::conditional_t<traits::is_noexcept, return_type(Args...) noexcept, return_type(Args...)>;
        };

        using type = typename extractor<type_list>::type;
    };

    /**
     * @brief function使用的接口抽象层。用于统一访问函数对象，非库开发者不需要直接引用
     */
    struct invoker_accessor {
        virtual ~invoker_accessor() = default;
        RAINY_NODISCARD virtual std::uintptr_t target(const foundation::ctti::typeinfo &fx_sign) const noexcept = 0;
        RAINY_NODISCARD virtual method_flags type() const noexcept = 0;
        virtual utility::any invoke(object_view &object) = 0;
        virtual utility::any invoke(object_view &object, arg_view arg_view) = 0;
        virtual utility::any dynamic_invoke(object_view &object, collections::views::array_view<utility::any> arg_view) = 0;
        RAINY_NODISCARD virtual const foundation::ctti::typeinfo &which_belongs() const noexcept = 0;
        RAINY_NODISCARD virtual const foundation::ctti::typeinfo &return_type() const noexcept = 0;
        RAINY_NODISCARD virtual const foundation::ctti::typeinfo &function_signature() const noexcept = 0;
        virtual invoker_accessor *construct_from_this(core::byte_t *soo_buffer) const noexcept = 0;
        RAINY_NODISCARD virtual const collections::views::array_view<foundation::ctti::typeinfo> &paramlists() const noexcept = 0;
        virtual bool equal_with(const invoker_accessor *impl) const noexcept = 0;
        RAINY_NODISCARD virtual bool is_invocable(
            collections::views::array_view<foundation::ctti::typeinfo> paramlist) const noexcept = 0;
        RAINY_NODISCARD virtual bool is_invocable_with(collections::views::array_view<utility::any> paramlist) const noexcept = 0;
        virtual void destruct(bool local) noexcept = 0;
    };

    template <typename Fx, typename Class, typename ReturnType, typename DefaultArguments, typename... Args>
    struct invoker_accessor_impl final : invoker_accessor {
        using function_signature_t = typename implements::function_signature<Fx>::type;
        using storage_t = invoker<Fx, DefaultArguments, Args...>;
        using typelist = type_traits::other_trans::type_list<Args...>;

        template <typename Functor, typename... UAx>
        explicit invoker_accessor_impl(Functor &&method, UAx &&...args) noexcept {
            utility::construct_at(&this->storage, utility::forward<Functor>(method), utility::forward<UAx>(args)...);
        }

        invoker_accessor_impl(const storage_t &right) : storage(right) {
        }

        invoker_accessor_impl(storage_t &&right) : storage(utility::move(right)) {
        }

        ~invoker_accessor_impl() {
        
        }

        RAINY_NODISCARD const foundation::ctti::typeinfo &return_type() const noexcept override {
            return implements::return_type_res<ReturnType>();
        }

        RAINY_NODISCARD const collections::views::array_view<foundation::ctti::typeinfo> &paramlists() const noexcept override {
            static const collections::views::array_view<foundation::ctti::typeinfo> paramlist = implements::param_types_res<Args...>();
            return paramlist;
        }

        RAINY_NODISCARD const foundation::ctti::typeinfo &function_signature() const noexcept override {
            return implements::function_signature_res<function_signature_t>();
        }

        RAINY_NODISCARD const foundation::ctti::typeinfo &which_belongs() const noexcept override {
            return implements::which_belongs_res<Class>();
        }

        RAINY_NODISCARD invoker_accessor *construct_from_this(core::byte_t *soo_buffer) const noexcept override {
            if constexpr (sizeof(type_traits::other_trans::decay_t<decltype(*this)>) >= core::fn_obj_soo_buffer_size) {
                return ::new invoker_accessor_impl(storage);
            } else {
                return utility::construct_at(reinterpret_cast<invoker_accessor_impl *>(soo_buffer), storage);
            }
        }

        RAINY_NODISCARD method_flags type() const noexcept override {
            return storage.type;
        }

        RAINY_NODISCARD bool equal_with(const invoker_accessor *impl) const noexcept override {
            if (function_signature() != impl->function_signature()) {
                return false;
            }
            if (impl->type() != type()) {
                return false;
            }
            auto cast_impl = static_cast<const invoker_accessor_impl *>(impl);
            if constexpr (is_fnobj<Fx>::value) {
                return std::memcmp(reinterpret_cast<const void *>(&storage.fn), reinterpret_cast<const void *>(&cast_impl->storage.fn),
                                   sizeof(storage.fn)) == 0;
            } else {
                return storage.fn == cast_impl->storage.fn;
            }
        }

        RAINY_NODISCARD std::uintptr_t target(const foundation::ctti::typeinfo &fx_sign) const noexcept override {
            if (fx_sign == rainy_typeid(function *)) {
                return reinterpret_cast<std::uintptr_t>(const_cast<type_traits::other_trans::decay_t<Fx> *>(&storage.fn));
            }
            if constexpr (is_fnobj<Fx>::value) {
                if (fx_sign != rainy_typeid(Fx)) {
                    return 0;
                }
            } else {
                if (fx_sign != function_signature()) {
                    return 0;
                }
            }
            return reinterpret_cast<std::uintptr_t>(const_cast<type_traits::other_trans::decay_t<Fx> *>(&storage.fn));
        }

        RAINY_NODISCARD utility::any invoke(object_view &object) override {
            if constexpr (storage_t::arity == 0) {
                if constexpr (type_traits::type_properties::is_polymorphic_v<Class>) {
                    auto *ptr = object.template try_dynamic_cast<Class>();
#if RAINY_ENABLE_DEBUG
                    utility::expects(ptr != nullptr, "Failure to convert the instance to the target pointer type during runtime");
#endif
                    return access_invoke(utility::forward<Fx>(storage.fn), ptr);
                } else {
#if RAINY_ENABLE_DEBUG
                    utility::expects(object.type().is_compatible(rainy_typeid(Class)) || object.type().is_void());
#endif
                    return access_invoke(utility::forward<Fx>(storage.fn), object.target_as_void_ptr());
                }
            } else {
                if constexpr (!type_traits::type_relations::is_same_v<decltype(storage.arguments.store), std::tuple<>>) {
                    return invoke(object, {});
                }
                return {};
            }
        }

        RAINY_NODISCARD utility::any invoke(object_view &object, arg_view arg_view) override {
            const std::size_t size = arg_view.size();
            static constexpr std::size_t arity = storage_t::arity;
            static constexpr std::size_t least = arity - storage_t::default_arity;
            void *ptr = nullptr;
            if constexpr (type_traits::type_properties::is_polymorphic_v<Class>) {
                ptr = static_cast<void *>(object.template try_dynamic_cast<Class>());
#if RAINY_ENABLE_DEBUG
                utility::expects(ptr != nullptr, "Failure to convert the instance to the target pointer type during runtime");
#endif
            } else {
#if RAINY_ENABLE_DEBUG
                utility::expects(object.type().is_compatible(rainy_typeid(Class)) || object.type().is_void());
#endif
                ptr = object.target_as_void_ptr();
            }
            if (size == arity) {
                const std::size_t args_hash =
                    core::accumulate(arg_view.begin(), arg_view.end(), std::size_t{0},
                                     [right = std::size_t{1}](const std::size_t acc, const object_view &item) mutable {
                                         return acc + static_cast<std::size_t>(item.type().hash_code() * right++);
                                     });
                /*
                在此处，我们考虑了参数校验的开销影响。例如，基本的遍历，占据n次比较开销用于检查遍历条件。
                因此，为了减少不必要的比较开销，我们设计了一个优化算法。
                这个算法通过一个基本的离散数学理论进行证明。可减少比较开销次数到n+1次。下面是证明。
                定义：
                  设序列 S = {item_i}_{i=1}^n
                  设旧哈希: H_old(S) = ∑_{i=1}^n hash(item_i)
                  设新哈希: H_new(S) = ∑_{i=1}^n i × hash(item_i)

                其中，旧哈希是对序列元素的简单哈希求和，新哈希则引入了元素的位置权重比较。

                证明：
                  ∀S, T: (H_old(S) = H_old(T)) ⇒ (∃ i,j: i ≠ j ∧ hash(item_i^S) = hash(item_j^T) ∧ item_i^S ≠ item_j^T ∨
                顺序不同且H_old相同) ⇏ ∀S, T: (H_new(S) = H_new(T)) ⇒ (∀ i: item_i^S = item_i^T)

                解释：
                  - H_old不唯一对应序列元素及顺序（因为交换律导致碰撞）
                  - H_new引入权重i破坏交换律，使哈希值唯一对应序列元素及位置
                  - 故H_new能检测元素及位置，一次比较即可判定序列一致性，减少多次比较
                结论：
                  H_new(S) = ∑ i × hash(item_i) 同时保证元素及其顺序的唯一性，改良了H_old(S)
                */
                if (args_hash == storage_t::param_hash) {
                    return storage.invoke_impl(ptr, arg_view, type_traits::helper::make_index_sequence<arity>{});
                } else {
                    // 如果参数一致，我们或许可以尝试转换参数类型来进行fallback处理
                    return storage.invoke_with_conv_impl(ptr, arg_view,
                                                         type_traits::helper::make_index_sequence<arity>{});
                }
            }
            if (size < least || size > arity) {
                errno = EINVAL;
                return {};
            }
            // 这里可能会涉及默认参数，不过，可能有些参数是需要转换的，因此，这个路径可能会处理需要转换的参数
            return storage.invoke_with_defaults(ptr, arg_view);
        }

        utility::any dynamic_invoke(object_view &object, collections::views::array_view<utility::any> arg_view) {
            const std::size_t size = arg_view.size();
            static constexpr std::size_t arity = storage_t::arity;
            static constexpr std::size_t least = arity - storage_t::default_arity;
            void *ptr = nullptr;
            if constexpr (type_traits::type_properties::is_polymorphic_v<Class>) {
                ptr = static_cast<void *>(object.template try_dynamic_cast<Class>());
#if RAINY_ENABLE_DEBUG
                utility::expects(ptr != nullptr, "Failure to convert the instance to the target pointer type during runtime");
#endif
            } else {
#if RAINY_ENABLE_DEBUG
                utility::expects(object.type().is_compatible(rainy_typeid(Class)) || object.type().is_void());
#endif
                ptr = object.target_as_void_ptr();
            }
            if (size == arity) {
                const std::size_t args_hash =
                    core::accumulate(arg_view.begin(), arg_view.end(), std::size_t{0},
                                     [right = std::size_t{1}](const std::size_t acc, const utility::any &item) mutable {
                                         return acc + static_cast<std::size_t>(item.type().hash_code() * right++);
                                     });
                if (args_hash == storage_t::param_hash) {
                    return storage.invoke_impl(ptr, arg_view, type_traits::helper::make_index_sequence<arity>{});
                } else {
                    return storage.invoke_with_conv_impl(ptr, arg_view, type_traits::helper::make_index_sequence<arity>{});
                }
            }
            if (size < least || size > arity) {
                errno = EINVAL;
                return {};
            }
            return storage.invoke_with_defaults(ptr, arg_view);
        }

        RAINY_NODISCARD bool is_invocable(
            collections::views::array_view<foundation::ctti::typeinfo> paramlist) const noexcept override {
            if (storage_t::arity != paramlist.size()) {
                return false;
            }
            std::size_t paramhash =
                core::accumulate(paramlist.begin(), paramlist.end(), std::size_t{0},
                                 [right = std::size_t{1}](const std::size_t acc, const foundation::ctti::typeinfo &item) mutable {
                                     return acc + (item.hash_code() * right++);
                                 });
            if (paramhash == storage.param_hash) {
                return true;
            }
            if (storage.is_compatible(paramlist)) {
                return true;
            }
            return is_invocable_helper(paramlist, type_traits::helper::make_index_sequence<storage_t::arity>{});
        }

        RAINY_NODISCARD bool is_invocable_with(collections::views::array_view<utility::any> paramlist) const noexcept override {
            if (storage_t::arity != paramlist.size()) {
                return false;
            }
            std::size_t paramhash =
                core::accumulate(paramlist.begin(), paramlist.end(), std::size_t{0},
                                 [right = std::size_t{1}](const std::size_t acc, const utility::any &item) mutable {
                                     return acc + (item.type().hash_code() * right++);
                                 });
            if (paramhash == storage.param_hash) {
                return true;
            }
            if (storage.is_compatible(paramlist)) {
                return true;
            }
            return is_invocable_helper(paramlist, type_traits::helper::make_index_sequence<storage_t::arity>{});
        }

        template <std::size_t... I>
        bool is_invocable_helper(collections::views::array_view<foundation::ctti::typeinfo> paramlist,
                                 type_traits::helper::index_sequence<I...>) const noexcept {
            return (... && utility::any_converter<typename type_traits::other_trans::type_at<I, typelist>::type>::is_convertible(
                               paramlist[I]));
        }

        template <std::size_t... I>
        bool is_invocable_helper(collections::views::array_view<utility::any> paramlist,
                                 type_traits::helper::index_sequence<I...>) const noexcept {
            return (... && utility::any_converter<typename type_traits::other_trans::type_at<I, typelist>::type>::is_convertible(
                               paramlist[I].type()));
        }

        void destruct(const bool local) noexcept override {
            if (local) {
            } else {
                delete this;
            }
        }

        union {
            storage_t storage;
            std::max_align_t dummy{};
        };
    };

    template <typename Fx, typename DefaultArguments, typename Traits,
              typename TypeList = typename type_traits::other_trans::tuple_like_to_type_list<typename Traits::tuple_like_type>::type>
    struct get_ia_implement_type {
        using unused_type1 = Fx;
        using unused_type2 = DefaultArguments;
        using unused_type3 = TypeList;
    };

    template <typename Fx, typename Traits, typename... DArgs, typename... Args>
    struct get_ia_implement_type<Fx, default_arguments_store<DArgs...>, Traits, type_traits::other_trans::type_list<Args...>> {
        using memptr_traits = type_traits::primary_types::member_pointer_traits<Fx>;

        template <typename FxTraits, bool IsMemptr = FxTraits::valid>
        struct decl_class {
            using type = void;
            static RAINY_CONSTEXPR_BOOL unused = IsMemptr;
        };

        template <typename FxTraits>
        struct decl_class<FxTraits, true> {
            using type = typename FxTraits::class_type;
        };

        using type = invoker_accessor_impl<Fx, typename decl_class<memptr_traits>::type, typename Traits::return_type,
                                           implements::default_arguments_store<DArgs...>, Args...>;
    };
}

#endif