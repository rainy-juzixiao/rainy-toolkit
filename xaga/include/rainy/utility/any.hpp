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
#ifndef RAINY_UTILITY_ANY_HPP
#define RAINY_UTILITY_ANY_HPP
#include <rainy/core/core.hpp>
#include <rainy/foundation/diagnostics/contract.hpp>
#include <rainy/foundation/functional/function_pointer.hpp>
#include <rainy/foundation/typeinfo.hpp>
#include <rainy/utility/implements/any_bad_cast.hpp>
#include <rainy/utility/implements/cast.hpp>
#include <rainy/utility/tuple_like_traits.hpp>
#include <utility>
#include <variant>

#if RAINY_USING_MSVC
#pragma warning(push)
#pragma warning(disable : 4715 4702)
#endif

namespace rainy::utility::implements {
    template <typename Ty, std::size_t Length>
    inline constexpr bool any_is_small =
        alignof(Ty) <= alignof(max_align_t) && type_traits::type_properties::is_nothrow_move_constructible_v<Ty> &&
        sizeof(Ty) <= Length;

    enum class any_representation : uintptr_t {
        big,
        _small,
        reference
    };

    struct big_any_ctti_manager {
        using destory_fn = void(void *target) noexcept;
        using copy_fn = void *(const void *source);
        using size_fn = std::size_t() noexcept;

        template <typename Ty>
        static void destory(void *const target) noexcept {
            ::delete static_cast<Ty *>(target);
        }

        template <typename Ty>
        RAINY_NODISCARD static void *copy(const void *const source) {
            return ::new Ty(*static_cast<const Ty *>(source));
        }

        destory_fn *destory_;
        copy_fn *copy_;
    };

    struct small_any_ctti_manager {
        using destroy_fn = void(void *target) noexcept;
        using copy_fn = void(void *target, const void *source);
        using move_fn = void(void *target, void *source) noexcept;

        template <typename Ty>
        static void destroy(void *const target) noexcept {
            std::destroy_at(static_cast<Ty *>(target));
        }

        template <typename Ty>
        static void copy(void *const target, const void *const source) {
            ::new (static_cast<Ty *>(target)) Ty(*static_cast<const Ty *>(source));
        }

        template <typename Ty>
        static void move(void *const target, void *const source) noexcept {
            ::new (static_cast<Ty *>(target)) Ty(utility::move(*static_cast<Ty *>(source)));
        }

        destroy_fn *destroy_;
        copy_fn *copy_;
        move_fn *move_;
    };

    template <typename Ty>
    inline constexpr big_any_ctti_manager any_big_ctti_manager_object = {&big_any_ctti_manager::destory<Ty>,
                                                                         &big_any_ctti_manager::copy<Ty>};

    template <typename Ty>
    inline constexpr small_any_ctti_manager any_small_ctti_manager_object = {
        &small_any_ctti_manager::destroy<Ty>, &small_any_ctti_manager::copy<Ty>, &small_any_ctti_manager::move<Ty>};

    struct any_binding_package {
        const void *payload;
        const foundation::ctti::typeinfo *type;
    };
}

namespace rainy::utility {
    template <std::size_t Length = core::small_object_space_size - (sizeof(void*)*2), std::size_t Align = alignof(std::max_align_t)>
    class basic_any;

    template <typename TargetType, typename = void>
    struct any_converter {
        static constexpr bool invalid_mark = true;

        static bool is_convertible(const foundation::ctti::typeinfo &) {
            return false;
        }
    };

    template <typename Type, typename = void>
    RAINY_CONSTEXPR_BOOL is_any_convert_invocable = true;

    template <typename Type>
    RAINY_CONSTEXPR_BOOL
        is_any_convert_invocable<Type, type_traits::other_trans::void_t<decltype(any_converter<Type>::invalid_mark)>> = false;

    template <typename Any>
    struct is_any : type_traits::helper::false_type {};

    template <std::size_t Length, std::size_t Align>
    struct is_any<basic_any<Length, Align>> : type_traits::helper::true_type {};

    template <typename Any>
    RAINY_CONSTEXPR_BOOL is_any_v = is_any<Any>::value;
}

/*
 * any的基础utils和异常实现
 */
namespace rainy::utility::implements {
    struct any_operater_policy {
        enum operation {
            compare_less,
            compare_equal,
            compare_less_equal,
            eval_hash,
            query_for_is_pair_like,
            destructre_this_pack,
            output_any
        };

        using invoke_fn = bool(operation op, void *data) noexcept;

        template <typename Ty, std::size_t Length, std::size_t Align>
        static bool invoke(operation op, void *data);

        invoke_fn *fn;
    };

    template <typename Ty, std::size_t Length, std::size_t Align>
    inline const any_operater_policy any_operater_policy_object = {
        +[](const any_operater_policy::operation op, void *const data) noexcept -> bool {
            return any_operater_policy::invoke<Ty, Length, Align>(op, data);
        }};

    template <typename TargetType, std::size_t Idx, bool IsReference = type_traits::composite_types::is_reference_v<TargetType>>
    struct convert_any_binding_package {
        static decltype(auto) impl(const implements::any_binding_package &pkg) {
            using namespace type_traits;
            if (implements::is_as_runnable<TargetType>(*pkg.type)) {
                return implements::as_impl<TargetType>(pkg.payload, *pkg.type);
            } else {
                if constexpr (is_any_convert_invocable<TargetType>) {
                    if (any_converter<TargetType>::is_convertible(*pkg.type)) {
                        return any_converter<TargetType>::basic_convert(pkg.payload, *pkg.type);
                    }
                }
                std::terminate();
            }
        }
    };

    template <typename TargetType, std::size_t Idx>
    struct convert_any_binding_package<TargetType, Idx, false> {
        static auto impl(const implements::any_binding_package &pkg) {
            using namespace type_traits;
            if (implements::is_as_runnable<TargetType>(*pkg.type)) {
                return implements::as_impl<TargetType>(pkg.payload, *pkg.type);
            } else {
                if constexpr (is_any_convert_invocable<TargetType>) {
                    if (any_converter<TargetType>::is_convertible(*pkg.type)) {
                        return any_converter<TargetType>::basic_convert(pkg.payload, *pkg.type);
                    }
                }
                std::terminate();
            }
        }
    };
}

namespace rainy::utility::implements {
    class any_ostream_converter_register {
    public:
        RAINY_DECLARE_SIGNLE_INSTANCE(any_ostream_converter_register);

        void call_converter(std::ostream &ostream, const foundation::ctti::typeinfo &ctti, const void *res) const {
            const auto find = converters.find(ctti);
            if (find != converters.end()) {
                try {
                    find->second(ostream, ctti, res);
                } catch (std::exception &e) {
                    throw e;
                }
            }
        }

    private:
        std::unordered_map<foundation::ctti::typeinfo, foundation::functional::function_pointer<void (*)(
                                                           std::ostream &, const foundation::ctti::typeinfo &ctti, const void *res)>>
            converters;
    };
}

#if RAINY_USING_MSVC
#pragma warning(push)
#pragma warning(disable : 6011)
#endif

/*
 * any实现
 */
namespace rainy::utility {
    template <std::size_t Length, std::size_t Align>
    class basic_any {
    public:
        static_assert(Length != 0, "What? You serious? Why are you input a 0 for Length, It's illegal!");

        class default_match {
            template <typename...>
            friend class matcher;

            default_match() noexcept = default;
            ~default_match() = default;
            default_match(const default_match &) = default;
            default_match(default_match &&) = default;
        };

        template <typename... Handlers>
        class matcher {
        public:
            matcher(Handlers &&...hs) : handlers(utility::forward<Handlers>(hs)...) {
            }

            basic_any invoke(basic_any const &a) const noexcept {
                return invoke_impl(a, std::make_index_sequence<sizeof...(Handlers)>{});
            }

        private:
            template <std::size_t I>
            bool try_to_invoke(basic_any const &this_, basic_any &result) const noexcept {
                using namespace type_traits;
                using namespace type_traits::primary_types;
                using handler_t = std::tuple_element_t<I, std::tuple<Handlers...>>;
                using argument_t = std::tuple_element_t<0, typename function_traits<handler_t>::tuple_like_type>;
                if constexpr (type_relations::is_same_v<argument_t, default_match>) {
                    return false; // 最后处理
                } else {
                    auto &handler = std::get<I>(handlers);
                    if (this_.is<argument_t>()) {
                        if constexpr (type_relations::is_void_v<function_return_type<handler_t>>) {
                            utility::invoke(handler, this_.as<argument_t>());
                        } else {
                            result = utility::invoke(handler, this_.as<argument_t>());
                        }
                        return true;
                    } else if (this_.is_convertible<argument_t>()) {
                        if constexpr (type_relations::is_void_v<function_return_type<handler_t>>) {
                            utility::invoke(handler, this_.convert<argument_t>());
                        } else {
                            result = utility::invoke(handler, this_.convert<argument_t>());
                        }
                        return true;
                    }
                    return false;
                }
            }

            template <std::size_t... Idx>
            static constexpr std::size_t eval_for_default_match_index(std::index_sequence<Idx...>) {
                using namespace type_traits;
                using namespace type_traits::other_trans;
                using type_list = type_list<typename type_at<
                    0, typename tuple_like_to_type_list<primary_types::param_list_in_tuple<Handlers>>::type>::type...>;
                std::size_t index{};
                ((type_relations::is_same_v<cv_modify::remove_cvref_t<typename type_at<Idx, type_list>::type>, default_match>
                  ? index = Idx,
                  void() : void()),
                 ...);
                return index;
            }

            template <std::size_t... Idx>
            static constexpr std::size_t eval_for_default_match(std::index_sequence<Idx...>) {
                using namespace type_traits;
                using namespace type_traits::other_trans;
                using type_list = type_list<typename type_at<
                    0, typename tuple_like_to_type_list<primary_types::param_list_in_tuple<Handlers>>::type>::type...>;
                std::size_t count{};
                ((count +=
                  type_relations::is_same_v<cv_modify::remove_cvref_t<typename type_at<Idx, type_list>::type>, default_match>),
                 ...);
                return count;
            }

            template <std::size_t... I>
            basic_any invoke_impl(basic_any const &a, std::index_sequence<I...>) const noexcept {
                using namespace type_traits;
                basic_any result{};
                if (!a.has_value()) {
                    return result;
                }
                bool matched = (try_to_invoke<I>(a, result) || ...);
                constexpr std::size_t default_match_count = eval_for_default_match(std::make_index_sequence<sizeof...(Handlers)>{});
                static_assert(default_match_count <= 1, "You can't add 2 and more default_match!");
                if (!matched) {
                    if constexpr (default_match_count == 1) {
                        constexpr std::size_t index = eval_for_default_match_index(std::make_index_sequence<sizeof...(Handlers)>{});
                        auto &handler = std::get<index>(handlers);
                        using handler_t = decltype(handler);
                        static constexpr default_match default_match_obj{};
                        using type_list = other_trans::tuple_like_to_type_list<
                            primary_types::param_list_in_tuple<cv_modify::remove_cvref_t<handler_t>>>::type;
                        constexpr std::size_t arity = other_trans::type_list_size_v<type_list>;
                        if constexpr (arity == 1) {
                            handler(default_match_obj);
                        } else if constexpr (arity == 2) {
                            static_assert(
                                type_traits::type_relations::is_convertible_v<const basic_any &,
                                                                              typename other_trans::type_at<1, type_list>::type>,
                                "param(2) not match");
                            handler(default_match_obj, a);
                        } else {
                            static_assert(type_traits::implements::always_false<handler_t>, "handler param lists not match!");
                        }
                    }
                }
                return result;
            }

            std::tuple<Handlers...> handlers;
        };

        constexpr basic_any() noexcept {
        }

        RAINY_INLINE basic_any(const basic_any &right) {
            using namespace implements;
            storage.type_data = right.storage.type_data;
            storage.executer = right.storage.executer;
            switch (get_representation()) {
                case any_representation::_small:
                    storage.small_any_ctti_manager = right.storage.small_any_ctti_manager;
                    storage.small_any_ctti_manager->copy_(&storage.buffer, &right.storage.buffer);
                    break;
                case any_representation::big:
                    storage.big_any_ctti_manager = right.storage.big_any_ctti_manager;
                    storage.ptr = right.storage.big_any_ctti_manager->copy_(right.storage.ptr);
                    break;
                case any_representation::reference:
                    storage.ptr = right.storage.ptr;
                    break;
                default:
                    break;
            }
        }

        RAINY_INLINE basic_any(basic_any &&right) noexcept {
            move_from(right);
        }

        template <typename ValueType,
                  type_traits::other_trans::enable_if_t<
                      type_traits::logical_traits::conjunction_v<
                          type_traits::logical_traits::negation<
                              type_traits::type_relations::is_same<type_traits::other_trans::decay_t<ValueType>, basic_any>>,
                          type_traits::logical_traits::negation<type_traits::primary_types::is_specialization<
                              type_traits::other_trans::decay_t<ValueType>, std::in_place_type_t>>,
                          type_traits::type_properties::is_copy_constructible<type_traits::other_trans::decay_t<ValueType>>>,
                      int> = 0>
        RAINY_INLINE basic_any(ValueType &&value) {
            emplace_<ValueType>(utility::forward<ValueType>(value));
        }

        template <typename ValueType, typename... Types,
                  type_traits::other_trans::enable_if_t<
                      type_traits::logical_traits::conjunction_v<
                          type_traits::type_properties::is_constructible<type_traits::other_trans::decay_t<ValueType>, Types...>,
                          type_traits::type_properties::is_copy_constructible<type_traits::other_trans::decay_t<ValueType>>>,
                      int> = 0>
        RAINY_INLINE basic_any(std::in_place_type_t<ValueType>, Types &&...args) {
            emplace_<ValueType>(utility::forward<Types>(args)...);
        }

        template <typename ValueType, typename Elem, typename... Types,
                  type_traits::other_trans::enable_if_t<
                      type_traits::logical_traits::conjunction_v<
                          type_traits::type_properties::is_constructible<type_traits::other_trans::decay_t<ValueType>,
                                                                         std::initializer_list<Elem> &, Types...>,
                          type_traits::type_properties::is_copy_constructible<type_traits::other_trans::decay_t<ValueType>>>,
                      int> = 0>
        RAINY_INLINE explicit basic_any(std::in_place_type_t<ValueType>, std::initializer_list<Elem> ilist, Types &&...args) {
            emplace_<ValueType>(ilist, utility::forward<Types>(args)...);
        }

        RAINY_INLINE ~basic_any() noexcept {
            reset();
        }

        basic_any &operator=(const basic_any &right) {
            assign_(right);
            return *this;
        }

        basic_any &operator=(basic_any &&right) noexcept {
            assign_(utility::move(right));
            return *this;
        }

        template <typename ValueType,
                  type_traits::other_trans::enable_if_t<
                      type_traits::logical_traits::conjunction_v<
                          type_traits::logical_traits::negation<
                              type_traits::type_relations::is_same<type_traits::other_trans::decay_t<ValueType>, basic_any>>,
                          type_traits::type_properties::is_copy_constructible<type_traits::other_trans::decay_t<ValueType>>>,
                      int> = 0>
        basic_any &operator=(ValueType &&value) {
            assign_(utility::forward<ValueType>(value));
            return *this;
        }

        template <typename ValueType, typename... Types,
                  type_traits::other_trans::enable_if_t<
                      type_traits::logical_traits::conjunction_v<
                          type_traits::type_properties::is_constructible<type_traits::other_trans::decay_t<ValueType>, Types...>,
                          type_traits::type_properties::is_copy_constructible<type_traits::other_trans::decay_t<ValueType>>>,
                      int> = 0>
        type_traits::other_trans::decay_t<ValueType> &emplace(Types &&...args) {
            reset();
            return emplace_<type_traits::other_trans::decay_t<ValueType>>(utility::forward<Types>(args)...);
        }

        template <typename ValueType, typename Elem, typename... Types,
                  type_traits::other_trans::enable_if_t<
                      type_traits::logical_traits::conjunction_v<
                          type_traits::type_properties::is_constructible<type_traits::other_trans::decay_t<ValueType>,
                                                                         std::initializer_list<Elem> &, Types...>,
                          type_traits::type_properties::is_copy_constructible<type_traits::other_trans::decay_t<ValueType>>>,
                      int> = 0>
        type_traits::other_trans::decay_t<ValueType> &emplace(std::initializer_list<Elem> ilist, Types &&...args) {
            reset();
            return emplace_<type_traits::other_trans::decay_t<ValueType>>(ilist, utility::forward<Types>(args)...);
        }

        void reset() noexcept {
            using namespace implements;
            if (!has_value()) {
                return;
            }
            switch (get_representation()) {
                case any_representation::_small:
                    storage.small_any_ctti_manager->destroy_(&storage.buffer);
                    break;
                case any_representation::big:
                    storage.big_any_ctti_manager->destory_(const_cast<void*>(storage.ptr));
                    break;
                case any_representation::reference:
                default:
                    break;
            }
            storage.type_data = 0;
        }

        void swap(basic_any &right) noexcept {
            right = utility::exchange(*this, utility::move(right));
        }

        RAINY_NODISCARD bool has_value() const noexcept {
            return storage.type_data != 0;
        }

        RAINY_NODISCARD const foundation::ctti::typeinfo &type() const noexcept {
            const foundation::ctti::typeinfo *const info = type_info();
            if (info) {
                return *info;
            }
            return rainy_typeid(void);
        }

        template <typename Type>
        RAINY_NODISCARD auto as() noexcept -> decltype(auto) {
            return implements::as_impl<Type>(target_as_void_ptr(), type());
        }

        template <typename Type>
        RAINY_NODISCARD auto as() const noexcept -> decltype(auto) {
            using namespace type_traits::cv_modify;
            using ret_type = decltype(utility::declval<basic_any &>().template as<Type>());
            rainy_let nonconst = const_cast<basic_any *>(this);
            if constexpr (type_traits::primary_types::is_rvalue_reference_v<ret_type>) {
                return nonconst->template as<type_traits::reference_modify::add_const_rvalue_ref_t<Type>>();
            } else {
                return nonconst->template as<type_traits::reference_modify::add_const_lvalue_ref_t<Type>>();
            }
        }

        template <typename Decayed>
        RAINY_NODISCARD const Decayed *cast_to_pointer() const noexcept {
            using namespace foundation::ctti;
            static constexpr std::size_t target_hashcode = foundation::ctti::typeinfo::create<Decayed>().hash_code();
            const foundation::ctti::typeinfo *const info = type_info();
            if (!info) {
                return nullptr;
            }
            if (info->has_traits(traits::is_lref)) {
                static constexpr std::size_t add_ref_hash =
                    foundation::ctti::typeinfo::create<type_traits::reference_modify::add_lvalue_reference_t<Decayed>>().hash_code();
                return info->hash_code() == add_ref_hash ? static_cast<const Decayed *>(target_as_void_ptr()) : nullptr;
            } else if (info->has_traits(traits::is_rref)) {
                static constexpr std::size_t add_ref_hash =
                    foundation::ctti::typeinfo::create<type_traits::reference_modify::add_rvalue_reference_t<Decayed>>().hash_code();
                return info->hash_code() == add_ref_hash ? static_cast<const Decayed *>(target_as_void_ptr()) : nullptr;
            }
            return info->hash_code() == target_hashcode ? static_cast<const Decayed *>(target_as_void_ptr()) : nullptr;
        }

        template <typename Decayed>
        RAINY_NODISCARD Decayed *cast_to_pointer() noexcept {
            using remove_ref_t = type_traits::reference_modify::remove_reference_t<Decayed>;
            return const_cast<remove_ref_t *>(static_cast<const basic_any *>(this)->cast_to_pointer<Decayed>());
        }

        template <typename TargetType>
        basic_any &transform() {
            basic_any(std::in_place_type<TargetType>, this->convert<TargetType>()).swap(*this);
            return *this;
        }

        template <typename Fx>
        basic_any &transform(Fx &&handler) {
            using namespace type_traits;
            using type_list = typename other_trans::type_list<primary_types::param_list_in_tuple<Fx>>::type;
            using target_type = typename other_trans::type_at<0, type_list>::type;
            basic_any(std::in_place_type<target_type>, handler(this->convert<target_type>())).swap(*this);
        }

        template <typename TargetType>
        RAINY_NODISCARD TargetType convert() {
            using foundation::ctti::traits;
            static foundation::ctti::typeinfo target_type = foundation::ctti::typeinfo::create<TargetType>();
            if constexpr (is_any_convert_invocable<TargetType>) {
                return utility::any_converter<TargetType>::convert(*this);
            } else {
                foundation::exceptions::cast::throw_bad_any_cast();
            }
            std::terminate();
        }

        template <typename TargetType>
        RAINY_NODISCARD bool is_convertible() const noexcept {
            if constexpr (is_any_convert_invocable<TargetType>) {
                return utility::any_converter<TargetType>::is_convertible(this->type());
            }
            return false;
        }

        template <typename TargetType>
        RAINY_NODISCARD TargetType convert() const {
            return const_cast<basic_any *>(this)->convert<TargetType>();
        }

        const void *target_as_void_ptr() const {
            const void *ptr = nullptr;
            switch (get_representation()) {
                case implements::any_representation::big:
                case implements::any_representation::reference:
                    ptr = reinterpret_cast<const void *>(storage.ptr);
                    break;
                case implements::any_representation::_small:
                    ptr = reinterpret_cast<const void *const>(&storage.buffer);
                    break;
            }
            if (type().is_pointer() && !type().is_reference()) {
                ptr = *static_cast<const void *const *>(ptr);
            }
            return ptr;
        }

        bool operator<(const basic_any &right) const {
            errno = 0;
            std::tuple tuple{this, right};
            bool res = storage.executer->fn(implements::any_operater_policy::compare_less, &tuple);
            if (errno == EINVAL) {
                return false;
            }
            return res;
        }

        bool operator>(const basic_any& right) const {
            errno = 0;
            std::tuple tuple{this, right};
            bool res = (!storage.executer->fn(implements::any_operater_policy::compare_less, &tuple) &&
                        !storage.executer->fn(implements::any_operater_policy::compare_equal, &tuple));
            if (errno == EINVAL) {
                return false;
            }
            return res;
        }

        bool operator>=(const basic_any &right) const {
            errno = 0;
            std::tuple tuple{this, right};
            bool res = ((storage.executer->fn(implements::any_operater_policy::compare_equal, &tuple)) ||
                        (!storage.executer->fn(implements::any_operater_policy::compare_less, &tuple)));
            if (errno == EINVAL) {
                return false;
            }
            return res;
        }

        bool operator<=(const basic_any &right) const {
            errno = 0;
            std::tuple tuple{this, right};
            bool res = (storage.executer->fn(implements::any_operater_policy::compare_equal, &tuple) ||
                        storage.executer->fn(implements::any_operater_policy::compare_less, &tuple));
            if (errno == EINVAL) {
                return false;
            }
            return res;
        }

        bool operator==(const utility::basic_any<Length, Align> &right) const {
            errno = 0;
            std::tuple tuple{this, &right};
            bool res = storage.executer->fn(implements::any_operater_policy::compare_equal, &tuple);
            if (errno == EINVAL) {
                return false;
            }
            return res;
        }

        bool operator!=(const utility::basic_any<Length, Align> &right) const {
            return !(*this == right);
        }

        /**
         * @brief 获取any存储的当前类型变量对应的哈希值
         * @attention 需当前类型支持计算哈希
         * @attention 优先通过std::hash求值，如果std::hash不可用，则使用rainy::utility::hash作为哈希支持
         */
        std::size_t hash_code() const noexcept {
            std::size_t ret{};
            std::tuple tuple{this, &ret};
            if (storage.executer->fn(implements::any_operater_policy::eval_hash, &tuple)) {
                return ret;
            }
            std::terminate();
        }

        template <typename... Fx>
        basic_any match(Fx &&...funcs) const noexcept((type_traits::primary_types::function_traits<Fx>::is_noexcept && ...)) {
            return matcher<Fx...>{utility::forward<Fx>(funcs)...}.invoke(*this);
        }

        template <typename Rx, typename... Fx>
        Rx match(Fx &&...funcs) const {
            return matcher<Fx...>{utility::forward<Fx>(funcs)...}.invoke(*this).convert<Rx>();
        }

        template <typename... Types, typename... Fx>
        std::variant<Types...> match_for(Fx &&...funcs) const {
            auto res = match(utility::forward<Fx>(funcs)...);
            return match_variant_helper<0, std::variant<Types...>, type_traits::other_trans::type_list<Types...>>(res);
        }

        template <typename... Fx>
        auto match_for(auto_deduce_t, Fx &&...funcs) const -> std::variant<type_traits::primary_types::function_return_type<Fx>...> {
            using namespace type_traits::other_trans;
            using namespace type_traits::primary_types;
            static_assert((!type_traits::type_relations::is_void_v<function_return_type<Fx>> && ...),
                          "Cannot accept a void-ret type functions.");
            using auto_deduce_type_list = type_list<function_return_type<Fx>...>;
            using variant_type = typename type_list_to_tuple_like<auto_deduce_type_list, std::variant>::type;
            auto res = match(utility::forward<Fx>(funcs)...);
            return match_variant_helper<0, variant_type, auto_deduce_type_list>(res);
        }

        template <typename Fx, type_traits::other_trans::enable_if_t<type_traits::primary_types::function_traits<Fx>::valid &&
                                                                         !type_traits::primary_types::is_member_object_pointer_v<Fx>,
                                                                     int> = 0>
        bool destructure(Fx &&handler) const {
            using implements::any_binding_package;
            using namespace type_traits;
            using namespace type_traits::primary_types;
            using fn_traits = function_traits<Fx>;
            constexpr std::size_t arity = fn_traits::arity;
            static_assert(arity != 0, "Cannot process a empty paramlist handler!");
            std::size_t count{};
            bool ret = storage.executer->fn(implements::any_operater_policy::query_for_is_pair_like, &count);
            if (ret && count == arity) {
                collections::array<any_binding_package, arity> array;
                std::tuple tuple{this, collections::views::make_array_view(array)};
                ret = storage.executer->fn(implements::any_operater_policy::destructre_this_pack, &tuple);
                if (!ret) {
                    return false;
                }
                this->call_handler_with_array(utility::forward<Fx>(handler), array, std::make_index_sequence<arity>{});
                return true;
            }
            return false;
        }

        template <typename... Types>
        bool destructure(std::tuple<Types...> &ref_tuple) const {
            using implements::any_binding_package;
            using namespace type_traits;
            using namespace type_traits::primary_types;
            constexpr std::size_t size = sizeof...(Types);
            static_assert(size != 0, "Cannot process a empty tuple!");
            static_assert(type_traits::type_properties::is_constructible_v<std::tuple<Types...>, Types...>,
                          "Cannot construct a tuple with you giving types of tuple");
            std::size_t count{};
            bool ret = storage.executer->fn(implements::any_operater_policy::query_for_is_pair_like, &count);
            if (ret && count == size) {
                collections::array<any_binding_package, size> array;
                std::tuple tuple{this, collections::views::make_array_view(array)};
                ret = storage.executer->fn(implements::any_operater_policy::destructre_this_pack, &tuple);
                if (!ret) {
                    return false;
                }
                this->fill_tuple_with_array(ref_tuple, array, std::make_index_sequence<size>{});
                return true;
            }
            return false;
        }

        /**
         * @brief 解构当前any对象为一个pair
         * @param pair 应仅能绑定两个成员
         * @attention 需当前any存储的对象可以进行映射。即只有is_bindable返回true才可行。并且目标可解构的数量必须为2
         * @return 如果成功，返回true，反之false
         */
        template <template <typename,typename> typename PairTemplate,typename Ty1, typename Ty2>
        bool destructure(PairTemplate<Ty1, Ty2> &pair) const {
            using implements::any_binding_package;
            using namespace type_traits;
            using namespace type_traits::primary_types;
            constexpr std::size_t size = 2;
            std::size_t count{};
            bool ret = storage.executer->fn(implements::any_operater_policy::query_for_is_pair_like, &count);
            if (count == size) {
                collections::array<any_binding_package, size> array{};
                std::tuple tuple{this, collections::views::make_array_view(array)};
                ret = storage.executer->fn(implements::any_operater_policy::destructre_this_pack, &tuple);
                if (!ret) {
                    return false;
                }
                this->fill_pair_with_array(pair, array);
                return true;
            }
            return false;
        }

        template <typename Structure>
        bool destructure(Structure &structure_object) const {
            using implements::any_binding_package;
            using namespace type_traits;
            using namespace type_traits::primary_types;
            constexpr std::size_t size = member_count_v<type_traits::cv_modify::remove_cvref_t<Structure>>;
            static_assert(size != 0, "Cannot process this struct type. try to add this type as a specialization of "
                                     "rainy::type_traits::extras::tuple::reflectet_for_type.");
            std::size_t count{};
            bool ret = storage.executer->fn(implements::any_operater_policy::query_for_is_pair_like, &count);
            if (count == size) {
                collections::array<any_binding_package, size> array{};
                std::tuple tuple{this, collections::views::make_array_view(array)};
                ret = storage.executer->fn(implements::any_operater_policy::destructre_this_pack, &tuple);
                if (!ret) {
                    return false;
                }
                auto so_as_tuple = utility::struct_bind_tuple(structure_object);
                fill_structure_with_array(so_as_tuple, array, std::make_index_sequence<size>{});
                return true;
            }
            return false;
        }

        template <
            typename CharType, typename Any,
            type_traits::other_trans::enable_if_t<type_traits::type_relations::is_same_v<Any, basic_any>, int> = 0>
        friend std::basic_ostream<CharType> &operator<<(std::basic_ostream<CharType> &left, const Any &right) {
            if (!right.has_value()) {
                return left;
            }
            constexpr bool is_char = type_traits::type_relations::is_same_v<CharType, char>;
            std::tuple<std::basic_ostream<CharType> * /* ostream */, const basic_any * /* any */> params{&left, &right};
            std::tuple<bool /* is_char/is_wchar_t */, void * /* params */> tuple{is_char, &params};
            bool ok = right.storage.executer->fn(implements::any_operater_policy::output_any, &tuple);
            if (!ok) {
                left.setstate(std::ios::ios_base::failbit);
            }
            return left;
        }

        template <typename Type>
        bool is() const noexcept {
            return type() == rainy_typeid(Type);
        }

    private:
        static constexpr std::uintptr_t rep_mask = 3;

        RAINY_NODISCARD implements::any_representation get_representation() const noexcept {
            return static_cast<implements::any_representation>(storage.type_data & rep_mask);
        }

        RAINY_NODISCARD const foundation::ctti::typeinfo *type_info() const noexcept {
            return reinterpret_cast<const foundation::ctti::typeinfo *>(storage.type_data & ~rep_mask);
        }

        template <typename Fx, std::size_t N,std::size_t... Is>
        void call_handler_with_array(Fx &&handler, const collections::array<implements::any_binding_package, N> &array,
                                     std::index_sequence<Is...>) const {
            using namespace type_traits::other_trans;
            using fn_traits = type_traits::primary_types::function_traits<Fx>;
            using type_list = typename tuple_like_to_type_list<typename fn_traits::tuple_like_type>::type;
            utility::invoke(utility::forward<Fx>(handler),
                            implements::convert_any_binding_package<typename type_at<Is, type_list>::type, Is>::impl(array[Is])...);
        }

        template <std::size_t N,typename Tuple, std::size_t... Is>
        void fill_tuple_with_array(Tuple &tuple, const collections::array<implements::any_binding_package, N> &array,
                                   std::index_sequence<Is...>) const {
            tuple = {};
            utility::construct_in_place(
                tuple, implements::convert_any_binding_package<std::tuple_element_t<Is, Tuple>, Is>::impl(array[Is])...);
        }

        
        template <std::size_t N, typename Tuple, std::size_t... Is>
        void fill_structure_with_array(Tuple &so_as_tuple, const collections::array<implements::any_binding_package, N> &array,
                                   std::index_sequence<Is...>) const {
            (((*std::get<Is>(so_as_tuple)) =
                  implements::convert_any_binding_package<std::remove_pointer_t<std::tuple_element_t<Is, Tuple>>, Is>::impl(
                      array[Is])),
             ...);
        }

        template <typename Pair, std::size_t... Is>
        void fill_pair_with_array(Pair &pair, const collections::array<implements::any_binding_package, 2> &array) const {
            using implements::convert_any_binding_package;
            auto &[first, second] = pair; // 从pair中解包
            using first_type = decltype(first);
            using second_type = decltype(second);
            static_assert(type_traits::type_properties::is_copy_assignable_v<first_type>,
                          "The first element of pair-like type is not assignable");
            static_assert(type_traits::type_properties::is_copy_assignable_v<second_type>,
                          "The second element of pair-like type is not assignable");
            first = implements::convert_any_binding_package<first_type, 0>::impl(array[0]);
            second = implements::convert_any_binding_package<second_type, 1>::impl(array[1]);
        }

        template <std::size_t Idx = 0, typename Variant, typename TypeList>
        auto match_variant_helper(const basic_any &res) const {
            if constexpr (Idx < type_traits::other_trans::type_list_size_v<TypeList>) {
                using type = typename type_traits::other_trans::type_at<Idx, TypeList>::type;
                if (res.template is<type>()) {
                    return Variant{res.template as<type>()};
                } else if (res.template is_convertible<type>()) {
                    return Variant{res.template convert<type>()};
                } else {
                    return match_variant_helper<Idx + 1, Variant, TypeList>(res);
                }
            } else {
                return Variant{};
            }
        }

        void move_from(basic_any &right) noexcept {
            using namespace implements;
            storage.type_data = right.storage.type_data;
            storage.executer = right.storage.executer;
            switch (get_representation()) {
                case any_representation::_small:
                    storage.small_any_ctti_manager = right.storage.small_any_ctti_manager;
                    storage.small_any_ctti_manager->move_(&storage.buffer, &right.storage.buffer);
                    break;
                case any_representation::big:
                    storage.big_any_ctti_manager = right.storage.big_any_ctti_manager;
                    storage.ptr = right.storage.ptr;
                    right.storage.type_data = 0;
                    break;
                case any_representation::reference:
                    storage.ptr = right.storage.ptr;
                    right.storage.type_data = 0;
                    break;
                default:
                    break;
            }
        }

        RAINY_INLINE void assign_(basic_any right) noexcept {
            reset();
            move_from(right);
        }

        template <typename Decayed, typename... Types>
        RAINY_INLINE decltype(auto) emplace_(Types &&...args) {
            using namespace type_traits;
            using decayed = other_trans::decay_t<Decayed>;
            if constexpr (composite_types::is_reference_v<Decayed>) {
                using remove_reference = reference_modify::remove_reference_t<Decayed>;
                if constexpr (!primary_types::is_array_v<remove_reference>) {
                    return emplace_ref<Decayed>(utility::forward<Types>(args)...);
                } else {
                    return emplace_<decayed>(utility::forward<Types>(args)...);
                }
            } else if constexpr (implements::any_is_small<decayed, Length>) {
                auto &object = reinterpret_cast<decayed &>(storage.buffer);
                storage.small_any_ctti_manager = &implements::any_small_ctti_manager_object<decayed>;
                ::new (utility::addressof(object)) decayed(utility::forward<Types>(args)...);
                storage.type_data = reinterpret_cast<std::uintptr_t>(&rainy_typeid(decayed)) |
                                    static_cast<std::uintptr_t>(implements::any_representation::_small);
                storage.executer = &implements::any_operater_policy_object<decayed, Length, Align>;
                return object;
            } else {
                decayed *const new_alloc_ptr = ::new decayed(utility::forward<Types>(args)...);
                storage.ptr = new_alloc_ptr;
                storage.big_any_ctti_manager = &implements::any_big_ctti_manager_object<decayed>;
                storage.type_data = reinterpret_cast<std::uintptr_t>(&rainy_typeid(decayed)) |
                                    static_cast<std::uintptr_t>(implements::any_representation::big);
                storage.executer = &implements::any_operater_policy_object<decayed, Length, Align>;
                return *new_alloc_ptr;
            }
        }

        template <typename Decayed, typename Type>
        RAINY_INLINE decltype(auto) emplace_ref(Type &&reference) {
            storage.ptr = utility::addressof(reference);
            storage.type_data = reinterpret_cast<std::uintptr_t>(&rainy_typeid(Decayed)) |
                                static_cast<std::uintptr_t>(implements::any_representation::reference);
            storage.executer = &implements::any_operater_policy_object<Decayed, Length, Align>;
            return reference;
        }

        struct storage_t {
            union {
                const void *ptr;
                alignas(Align) core::byte_t buffer[Length];
            };
            union {
                const implements::big_any_ctti_manager *big_any_ctti_manager;
                const implements::small_any_ctti_manager *small_any_ctti_manager;
            };
            const implements::any_operater_policy *executer;
            std::uintptr_t type_data;
        };

        union {
            std::max_align_t dummy;
            storage_t storage{};
        };
    };

    using any = basic_any<>;
}

#if RAINY_USING_MSVC
#pragma warning(pop)
#endif
#include <iostream>
namespace rainy::utility {
    template <typename Ty, typename... Args,
              typename = type_traits::other_trans::enable_if_t<
                  type_traits::type_properties::is_constructible_v<any, std::in_place_type_t<Ty>, Args...>, int>>
    RAINY_NODISCARD any make_any(Args &&...args) {
        return any{std::in_place_type<Ty>, utility::forward<Args>(args)...};
    }

    template <typename Ty, typename U, typename... Args,
              typename = type_traits::other_trans::enable_if_t<type_traits::type_properties::is_constructible_v<
                  any, std::in_place_type_t<Ty>, std::initializer_list<U> &, Args...>>>
    RAINY_NODISCARD any make_any(std::initializer_list<U> initializer_list, Args &&...args) {
        return any{std::in_place_type<Ty>, initializer_list, utility::forward<Args>(args)...};
    }

    RAINY_INLINE void swap(utility::any &left, utility::any &right) {
        left.swap(right);
    }
}

namespace rainy::utility {
    template <typename TargetType>
    struct any_converter<TargetType,
                         type_traits::other_trans::enable_if_t<type_traits::composite_types::is_reference_v<TargetType> ||
                                                               type_traits::primary_types::is_pointer_reference_v<TargetType>>> {
        template <typename Any, type_traits::other_trans::enable_if_t<is_any_v<type_traits::cv_modify::remove_cvref_t<Any>>, int> = 0>
        static decltype(auto) convert(Any &&any) {
            using namespace foundation::ctti;
            return basic_convert(any.target_as_void_ptr(), any.type());
        }

        static decltype(auto) basic_convert(const void *target_pointer, const foundation::ctti::typeinfo &type) {
            return implements::as_impl<TargetType>(target_pointer, type);
        }

        static bool is_convertible(const foundation::ctti::typeinfo &type) {
            return implements::is_as_runnable<TargetType>(type);
        }
    };

    template <typename TargetType>
    struct any_converter<TargetType,
                         type_traits::other_trans::enable_if_t<type_traits::composite_types::is_arithmetic_v<TargetType>>> {
        template <typename Any, type_traits::other_trans::enable_if_t<is_any_v<type_traits::cv_modify::remove_cvref_t<Any>>, int> = 0>
        static TargetType convert(Any &&any) {
            return basic_convert(any.target_as_void_ptr(), any.type());
        }

        static TargetType basic_convert(const void *target_pointer, const foundation::ctti::typeinfo &type) {
            using namespace foundation::ctti;
            foundation::ctti::typeinfo target_type = type;
            if (target_type.has_traits(traits::is_const) || target_type.has_traits(traits::is_volatile)) {
                target_type = target_type.remove_cv();
            }
            if (target_type.has_traits(traits::is_lref) || target_type.has_traits(traits::is_rref)) {
                target_type = target_type.remove_reference();
            }
            switch (target_type.hash_code()) {
                case rainy_typehash(int):
                    return static_cast<TargetType>(*static_cast<const int *>(target_pointer));
                case rainy_typehash(char):
                    return static_cast<TargetType>(*static_cast<const char *>(target_pointer));
                case rainy_typehash(float):
                    return static_cast<TargetType>(*static_cast<const float *>(target_pointer));
                case rainy_typehash(double):
                    return static_cast<TargetType>(*static_cast<const double *>(target_pointer));
                case rainy_typehash(long):
                    return static_cast<TargetType>(*static_cast<const long *>(target_pointer));
                case rainy_typehash(bool):
                    return static_cast<TargetType>(*static_cast<const bool *>(target_pointer));
                case rainy_typehash(long long):
                    return static_cast<TargetType>(*static_cast<const long long *>(target_pointer));
                case rainy_typehash(short):
                    return static_cast<TargetType>(*static_cast<const short *>(target_pointer));
                case rainy_typehash(std::int8_t):
                    return static_cast<TargetType>(*static_cast<const std::int8_t *>(target_pointer));
                case rainy_typehash(std::uint8_t):
                    return static_cast<TargetType>(*static_cast<const std::uint8_t *>(target_pointer));
                case rainy_typehash(std::uint16_t):
                    return static_cast<TargetType>(*static_cast<const std::uint16_t *>(target_pointer));
                case rainy_typehash(std::uint32_t):
                    return static_cast<TargetType>(*static_cast<const std::uint32_t *>(target_pointer));
                case rainy_typehash(std::uint64_t):
                    return static_cast<TargetType>(*static_cast<const std::uint64_t *>(target_pointer));
                default:
                    break;
            }
            foundation::exceptions::cast::throw_bad_any_cast();
            std::terminate();
        }

        static bool is_convertible(const foundation::ctti::typeinfo &type) {
            using namespace foundation::ctti;
            foundation::ctti::typeinfo target_type = type;
            if (target_type.has_traits(traits::is_const) || target_type.has_traits(traits::is_volatile)) {
                target_type = target_type.remove_cv();
            }
            if (target_type.has_traits(traits::is_lref) || target_type.has_traits(traits::is_rref)) {
                target_type = target_type.remove_reference();
            }
            switch (target_type.hash_code()) {
                case rainy_typehash(int):
                case rainy_typehash(char):
                case rainy_typehash(float):
                case rainy_typehash(double):
                case rainy_typehash(long):
                case rainy_typehash(bool):
                case rainy_typehash(long long):
                case rainy_typehash(short):
                case rainy_typehash(std::int8_t):
                case rainy_typehash(std::uint8_t):
                case rainy_typehash(std::uint16_t):
                case rainy_typehash(std::uint32_t):
                case rainy_typehash(std::uint64_t):
                    return true;
                default:
                    return false;
            }
        }
    };

    template <typename CharType, typename Traits>
    struct any_converter<std::basic_string_view<CharType, Traits>> {
        using target_type = std::basic_string_view<CharType, Traits>;

        template <typename Any, type_traits::other_trans::enable_if_t<is_any_v<type_traits::cv_modify::remove_cvref_t<Any>>, int> = 0>
        static target_type convert(Any &&any) {
            const auto &type = any.type();
            auto target_pointer = any.target_as_void_ptr();
            if (type.is_pointer() && type.is_reference()) {
                // 需检测const char_type*&这一情况，由于basic_convert是直接从值转换的，因此，必须保证地址指向适当的值
                target_pointer = *static_cast<const void *const *>(target_pointer);
            }
            return basic_convert(target_pointer, type);
        }

        static target_type basic_convert(const void *target_pointer, const foundation::ctti::typeinfo &type) {
            using namespace foundation::ctti;
            using namespace foundation::exceptions::cast;
            using const_pointer = const CharType *;
            using pointer = CharType *;
            using pointer_to_const = const CharType * const;
            using same_type = std::basic_string_view<CharType, Traits>;
            using same_type_with_const = const std::basic_string_view<CharType, Traits>;
            using basic_string_t = std::basic_string<CharType, Traits>;
            using const_basic_string_t = const std::basic_string<CharType, Traits>;
            switch (type.remove_reference().hash_code()) {
                case rainy_typehash(const_pointer):
                case rainy_typehash(pointer):
                    return target_type{static_cast<const_pointer>(target_pointer)};
                case rainy_typehash(pointer_to_const):
                    return *static_cast<const const_pointer *>(target_pointer);
                case rainy_typehash(same_type):
                case rainy_typehash(same_type_with_const):
                    return *static_cast<const target_type *>(target_pointer);
                case rainy_typehash(basic_string_t):
                case rainy_typehash(const_basic_string_t):
                    return static_cast<target_type>(*static_cast<const basic_string_t *>(target_pointer));
                default:
                    break;
            }
            throw_bad_any_cast();
            std::terminate();
        }

        static bool is_convertible(const foundation::ctti::typeinfo &type) {
            using namespace foundation::ctti;
            switch (type.remove_reference().hash_code()) {
                case rainy_typehash(const CharType *):
                case rainy_typehash(CharType *):
                case rainy_typehash(const CharType * const):
                case rainy_typehash(std::basic_string_view<CharType>):
                case rainy_typehash(std::basic_string<CharType>):
                case rainy_typehash(const std::basic_string_view<CharType>):
                case rainy_typehash(const std::basic_string<CharType>):
                    return true;
                default:
                    break;
            }
            return false;
        }
    };
}

namespace rainy::utility::implements {
    RAINY_INLINE bool compare_equal_helpr(const utility::any &left, const utility::any &right) {
        if (left.type().is_floating_point() || right.type().is_floating_point()) {
            return core::builtin::almost_equal(left.convert<double>(), right.convert<double>());
        } else {
            if (any_converter<std::int64_t>::is_convertible(left.type()) &&
                any_converter<std::int64_t>::is_convertible(right.type())) {
                return left.convert<std::int64_t>() == right.convert<std::int64_t>();
            }
            return false;
        }
    }

    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL is_char_any_can_output = false;

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_char_any_can_output<
        Ty, type_traits::other_trans::void_t<decltype(utility::declval<std::basic_ostream<char>>() << utility::declval<Ty>())>> = true;

    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL is_wchar_any_can_output = false;

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_wchar_any_can_output<
        Ty, type_traits::other_trans::void_t<decltype(utility::declval<std::basic_ostream<wchar_t>>() << utility::declval<Ty>())>> =
        true;

    template <typename Ty, std::size_t Length, std::size_t Align>
    bool any_operater_policy::invoke(operation op, void *const data) {
        using any = utility::basic_any<Length, Align>;
        switch (op) {
            case operation::compare_less: {
                const auto *res = static_cast<const std::tuple<const any *, const any &> *const>(data);
                const any *left = std::get<0>(*res);
                const any &right = std::get<1>(*res);
                static constexpr auto left_type = foundation::ctti::typeinfo::create<Ty>();
                if constexpr (type_traits::composite_types::is_arithmetic_v<Ty>) {
                    if (right.type().is_arithmetic()) {
                        if (left_type == right.type()) {
                            return (left->template as<Ty>() < right.template as<Ty>());
                        }
                        if (left_type.is_floating_point() || right.type().is_floating_point()) {
                            return (left->template convert<double>() < right.template convert<double>());
                        }
                        return (left->template convert<std::int64_t>() < right.template convert<std::int64_t>());
                    }
                }
                if (!left_type.is_nullptr() && right.type().is_nullptr()) {
                    errno = EINVAL;
                    return false;
                }
                if constexpr (type_traits::type_relations::is_same_v<Ty, std::string_view>) {
                    if (left_type == right.type()) {
                        return (left->template as<std::string_view>() < right.template as<std::string_view>());
                    }
                    if (any_converter<std::string_view>::is_convertible(right.type())) {
                        return (left->template as<std::string_view>() < any_converter<std::string_view>::convert(right));
                    }
                } else {
                    if (any_converter<std::string_view>::is_convertible(left->type()) &&
                        any_converter<std::string_view>::is_convertible(right.type())) {
                        return left->template convert<std::string_view>() < right.template convert<std::string_view>();
                    }
                }
                errno = EINVAL;
                return false;
            }
            case operation::compare_equal: {
                const auto *res = static_cast<const std::tuple<const any *, const any *> *const>(data);
                const any *left = std::get<0>(*res);
                const any *right = std::get<1>(*res);
                static constexpr auto left_type = foundation::ctti::typeinfo::create<Ty>();
                if constexpr (type_traits::composite_types::is_arithmetic_v<Ty>) {
                    if (right->type().is_arithmetic()) {
                        if (left_type == right->type()) {
                            return (left->template as<Ty>() == right->template as<Ty>());
                        }
                        return compare_equal_helpr(left, right);
                    }
                }
                if (!left_type.is_nullptr() && right->type().is_nullptr()) {
                    errno = EINVAL;
                    return false;
                }
                if constexpr (type_traits::type_relations::is_same_v<Ty, std::string_view>) {
                    if (left_type == right->type()) {
                        return (left->template as<std::string_view>() == right->template as<std::string_view>());
                    }
                    if (any_converter<std::string_view>::is_convertible(right->type())) {
                        return (left->template as<std::string_view>() == any_converter<std::string_view>::convert(*right));
                    }
                } else {
                    if (any_converter<std::string_view>::is_convertible(left->type()) &&
                        any_converter<std::string_view>::is_convertible(right->type())) {
                        return left->template convert<std::string_view>() == right->template convert<std::string_view>();
                    }
                }
                errno = EINVAL;
                return false;
            }
            case operation::compare_less_equal: {
                const auto *res = static_cast<const std::tuple<const any *, const any &> *const>(data);
                const any *left = std::get<0>(*res);
                const any &right = std::get<1>(*res);
                static constexpr auto left_type = foundation::ctti::typeinfo::create<Ty>();
                if constexpr (type_traits::composite_types::is_arithmetic_v<Ty>) {
                    if (right.type().is_arithmetic()) {
                        if (left_type <= right.type()) {
                            return (left->template as<Ty>() <= right.template as<Ty>());
                        }
                        if (left_type.is_floating_point() || right.type().is_floating_point()) {
                            return (left->template convert<double>() <= right.template convert<double>());
                        }
                        return (left->template convert<std::int64_t>() <= right.template convert<std::int64_t>());
                    }
                }
                if (!left_type.is_nullptr() && right.type().is_nullptr()) {
                    errno = EINVAL;
                    return false;
                }
                if constexpr (type_traits::type_relations::is_same_v<Ty, std::string_view>) {
                    if (left_type == right.type()) {
                        return (left->template as<std::string_view>() <= right.template as<std::string_view>());
                    }
                    if (any_converter<std::string_view>::is_convertible(right.type())) {
                        return (left->template as<std::string_view>() <= any_converter<std::string_view>::convert(right));
                    }
                } else {
                    if (any_converter<std::string_view>::is_convertible(left->type()) &&
                        any_converter<std::string_view>::is_convertible(right.type())) {
                        return left->template convert<std::string_view>() <= right.template convert<std::string_view>();
                    }
                }
                errno = EINVAL;
                return false;
            }
            case operation::eval_hash: {
                auto *res = static_cast<std::tuple<const any *, std::size_t *> *>(data);
                const any *object = std::get<0>(*res);
                std::size_t *hashcode = std::get<1>(*res);
                if constexpr (is_support_standard_hasher_available<Ty>::value) {
                    static std::hash<Ty> hasher;
                    *hashcode = hasher(object->template as<Ty>());
                    return true;
                } else if constexpr (is_support_rainytoolkit_hasher_available<Ty>::value) {
                    static utility::hash<Ty> hasher;
                    *hashcode = hasher(object->template as<Ty>());
                    return true;
                } else {
                    return false;
                }
            }
            case operation::query_for_is_pair_like: {
                constexpr std::size_t member_count = member_count_v<Ty>;
                if constexpr (member_count == 0) {
                    return false;
                }
                (*static_cast<std::size_t *>(data)) = member_count;
                return true;
            }
            case operation::destructre_this_pack: {
                using implements::any_binding_package;
                constexpr std::size_t member_count = member_count_v<Ty>;
                if constexpr (member_count != 0) {
                    auto *res = static_cast<std::tuple<const any *, collections::views::array_view<any_binding_package>> *>(data);
                    const any *object = std::get<0>(*res);
                    auto &view = std::get<1>(*res);
                    auto tuple_ptr = utility::struct_bind_tuple(object->template as<Ty>());
                    std::apply(
                        [&](auto *...elems) {
                            std::size_t idx = 0;
                            ((view[idx++] = any_binding_package{static_cast<const void *>(elems), &rainy_typeid(decltype(*elems))}),
                             ...);
                        },
                        tuple_ptr);
                    return true;
                }
                return false;
            }
            case operation::output_any: {
                auto *res = static_cast<std::tuple<bool /* is_char/is_wchar_t */, void * /* params */> *>(data);
                bool is_char = std::get<0>(*res);
                void *output_data = std::get<1>(*res);
                if (is_char) {
                    if constexpr (is_char_any_can_output<Ty>) {
                        auto *out =
                            static_cast<std::tuple<std::basic_ostream<char> * /* ostream */, const any * /* any */> *>(output_data);
                        (*std::get<0>(*out)) << std::get<1>(*out)->as<Ty>();
                        return true;
                    }
                } else {
                    if constexpr (is_wchar_any_can_output<Ty>) {
                        auto *out =
                            static_cast<std::tuple<std::basic_ostream<wchar_t> * /* ostream */, const any * /* any */> *>(output_data);
                        (*std::get<0>(*out)) << std::get<1>(*out)->as<Ty>();
                        return true;
                    }
                }
                return false;
            }
        }
        return false;
    }
}

namespace std {
    template <std::size_t Length, std::size_t Align>
    struct hash<rainy::utility::basic_any<Length, Align>> {
        RAINY_NODISCARD std::size_t operator()(const rainy::utility::any& right) const {
            return right.hash_code();
        }
    };
}

namespace rainy::utility {
    template <std::size_t Length, std::size_t Align>
    struct hash<basic_any<Length, Align>> {
        using result_type = std::size_t;
        using argument_type = basic_any<Length, Align>;

        RAINY_NODISCARD std::size_t operator()(const argument_type &right) const {
            return right.hash_code();
        }
    };
}

#if RAINY_USING_MSVC
#pragma warning(pop)
#endif

#endif
