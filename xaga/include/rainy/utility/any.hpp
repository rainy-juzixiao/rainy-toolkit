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
#include <rainy/foundation/functional/functional.hpp>
#include <rainy/foundation/typeinfo.hpp>
#include <rainy/utility/implements/any_exceptions.hpp>
#include <rainy/utility/implements/cast.hpp>
#include <rainy/utility/tuple_like_traits.hpp>
#include <utility>
#include <variant>

#if RAINY_USING_MSVC
#pragma warning(push)
#pragma warning(disable : 4715 4702 6011 26439 26495)
#endif

#define RAINY_ANY_CAST_TO_POINTER_NODISCARD                                                                                           \
    RAINY_NODISCARD_MSG("Ignoring the return value of cast_to_pointer<Ty>() might be an invalid call.")
#define RAINY_ANY_AS_NODISCARD RAINY_NODISCARD_MSG("Ignoring the return value of as<Ty>() might be an invalid call.")

namespace rainy::utility::implements {
    template <typename Ty, std::size_t Length>
    inline constexpr bool any_is_small = type_traits::type_properties::is_nothrow_move_constructible_v<Ty> && sizeof(Ty) <= Length;

    enum class any_representation : uintptr_t {
        big,
        _small,
        reference
    };

    struct big_any_ctti_manager {
        using destory_fn = void(void *target);
        using copy_fn = void *(const void *source);
        using size_fn = std::size_t() noexcept;

        template <typename Ty>
        static void destory(void *const target) {
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
        using destroy_fn = void(void *target);
        using copy_fn = void(void *target, const void *source);
        using move_fn = void(void *target, void *source) noexcept;

        template <typename Ty>
        static void destroy(void *const target) {
            static_cast<Ty *const>(target)->~Ty();
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

    template <typename BasicAny, typename Type>
    struct const_any_proxy_iterator;

    template <typename BasicAny, typename Type>
    struct any_proxy_iterator;
}

namespace rainy::utility {
    template <std::size_t Length = core::small_object_space_size - (sizeof(void *) * 2), std::size_t Align = alignof(std::max_align_t)>
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

    template <typename TargetType>
    RAINY_NODISCARD bool is_any_convertible(const foundation::ctti::typeinfo &source_type) noexcept {
        if constexpr (is_any_convert_invocable<TargetType>) {
            return utility::any_converter<TargetType>::is_convertible(source_type);
        } else {
            return source_type == rainy_typeid(TargetType);
        }
    }

    enum class any_iterator_category : std::int8_t {
        input_iterator,
        output_iterator,
        forward_iterator,
        bidirectional_iterator,
        random_access_iterator,
        contiguous_iterator // for cxx 20
    };
}

namespace rainy::utility::implements {
    enum class any_compare_operation {
        less,
        less_eq,
        eq,
        greater_eq,
        greater
    };

    enum class any_operation {
        compare,
        eval_hash,
        query_for_is_tuple_like,
        destructre_this_pack,
        output_any,
        add,
        subtract,
        incr_prefix,
        decr_prefix,
        incr_postfix,
        decr_postfix,
        multiply,
        divide,
        mod,
        dereference,
        access_element,
        call_begin,
        call_end,
        assign,
        get_reference,
        get_lvalue_reference,
        get_rvalue_reference,
        construct_from,
        swap_value
    };

    struct any_operater_policy {
        using operation = any_operation;

        using invoke_fn = bool(operation op, void *data) noexcept;

        template <typename Ty, std::size_t Length, std::size_t Align>
        static bool invoke_impl(operation op, void *data);

        invoke_fn *invoke;
    };

    template <typename Ty, std::size_t Length, std::size_t Align>
    inline const any_operater_policy any_operater_policy_object = {
        +[](const any_operater_policy::operation op, void *const data) noexcept -> bool {
            return any_operater_policy::invoke_impl<Ty, Length, Align>(op, data);
        }};

    template <typename TargetType, std::size_t Idx, bool IsReference = type_traits::composite_types::is_reference_v<TargetType>>
    struct convert_any_binding_package {
        static decltype(auto) impl(const implements::any_binding_package &pkg) {
            using namespace type_traits;
            if constexpr (type_relations::is_same_v<TargetType, utility::placeholder_t>) {
                return utility::placeholder;
            } else if constexpr (type_relations::is_same_v<TargetType, std::in_place_t>) {
                return std::in_place;
            } else {
                if (implements::is_as_runnable<TargetType>(*pkg.type)) {
                    return implements::as_impl<TargetType>(pkg.payload, *pkg.type);
                } else {
                    if constexpr (is_any_convert_invocable<TargetType>) {
                        if (any_converter<TargetType>::is_convertible(*pkg.type)) {
                            return any_converter<TargetType>::basic_convert(pkg.payload, *pkg.type);
                        }
                    } else if constexpr (is_any_convert_invocable<cv_modify::remove_cv<TargetType>>) {
                        if (any_converter<cv_modify::remove_cv_t<TargetType>>::is_convertible(*pkg.type)) {
                            return any_converter<cv_modify::remove_cv_t<TargetType>>::basic_convert(pkg.payload, *pkg.type);
                        }
                    } else if constexpr (is_any_convert_invocable<cv_modify::remove_cvref_t<TargetType>>) {
                        if (any_converter<cv_modify::remove_cvref_t<TargetType>>::is_convertible(*pkg.type)) {
                            return any_converter<cv_modify::remove_cvref_t<TargetType>>::basic_convert(pkg.payload, *pkg.type);
                        }
                    }
                    std::terminate();
                }
            }
        }
    };

    template <typename TargetType, std::size_t Idx>
    struct convert_any_binding_package<TargetType, Idx, false> {
        static auto impl(const implements::any_binding_package &pkg) {
            using namespace type_traits;
            if constexpr (type_relations::is_same_v<TargetType, utility::placeholder_t>) {
                return utility::placeholder;
            } else if constexpr (type_relations::is_same_v<TargetType, std::in_place_t>) {
                return std::in_place;
            } else {
                if (implements::is_as_runnable<TargetType>(*pkg.type)) {
                    return implements::as_impl<TargetType>(pkg.payload, *pkg.type);
                } else {
                    if constexpr (is_any_convert_invocable<TargetType>) {
                        if (any_converter<TargetType>::is_convertible(*pkg.type)) {
                            return any_converter<TargetType>::basic_convert(pkg.payload, *pkg.type);
                        }
                    } else if constexpr (is_any_convert_invocable<cv_modify::remove_cv<TargetType>>) {
                        if (any_converter<cv_modify::remove_cv_t<TargetType>>::is_convertible(*pkg.type)) {
                            return any_converter<cv_modify::remove_cv_t<TargetType>>::basic_convert(pkg.payload, *pkg.type);
                        }
                    } else if constexpr (is_any_convert_invocable<cv_modify::remove_cvref_t<TargetType>>) {
                        if (any_converter<cv_modify::remove_cvref_t<TargetType>>::is_convertible(*pkg.type)) {
                            return any_converter<cv_modify::remove_cvref_t<TargetType>>::basic_convert(pkg.payload, *pkg.type);
                        }
                    }
                    std::terminate();
                }
            }
        }
    };

    template <typename Ty>
    constexpr std::size_t eval_for_destructure_pack_receiver_size() {
        using implements::any_binding_package;
        using namespace type_traits;
        using namespace type_traits::primary_types;
        if constexpr (function_traits<Ty>::valid && !is_member_object_pointer_v<Ty>) {
            return function_traits<Ty>::arity;
        } else if constexpr (is_pair_v<Ty>) {
            return pair_traits<Ty>::size;
        } else if constexpr (is_tuple_v<Ty>) {
            return tuple_traits<Ty>::size;
        } else if constexpr (constexpr std::size_t size = member_count_v<cv_modify::remove_cvref_t<Ty>>; size != 0) {
            return size;
        } else {
            return 0;
        }
    }

    template <typename BasicAny>
    class any_reference :  public BasicAny {
    public:
        friend struct any_operater_policy;

        using basic_any = BasicAny;

        any_reference() : basic_any{} {
        }

        template <typename ValueType,
                  type_traits::other_trans::enable_if_t<!type_traits::type_relations::is_any_of_v<
                                                            type_traits::other_trans::decay_t<ValueType>, basic_any, any_reference>,
                                                        int> = 0>
        any_reference(ValueType &&value, basic_any *this_pointer = nullptr) :
            basic_any{std::in_place_type<decltype(value)>, utility::forward<ValueType>(value)}, this_pointer{this_pointer},
            the_type{&this_pointer->type()} {
        }

        any_reference(const any_reference &) = default;
        any_reference(any_reference &&) = default;

        template <typename ValueType>
        any_reference &operator=(ValueType &&value) {
#if RAINY_ENABLE_DEBUG
            assert((*this->the_type) == this->this_pointer->type() && "Can't accept a illegal reference to modify");
#else
            if (*this->the_type != this->this_pointer->type()) {
                return *this;
            }
#endif
            auto tuple = std::make_tuple(this, BasicAny{utility::forward<ValueType>(value)});
            this->storage.executer->invoke(any_operation::assign, &tuple);
            return *this;
        }

        any_reference& operator=(const any_reference& right) {
            if (utility::addressof(right) == this) {
                return *this;
            }
            auto tuple = std::make_tuple(this, right.as_value());
            this->storage.executer->invoke(any_operation::assign, &tuple);
            return *this;
        }

        any_reference &operator=(any_reference &&right) {
            if (utility::addressof(right) == this) {
                return *this;
            }
            auto tuple = std::make_tuple(this, right.as_value());
            this->storage.executer->invoke(any_operation::assign, &tuple);
            return *this;
        }

        void swap(any_reference &right) noexcept {
            this->swap_value(right);
        }

        friend void swap(any_reference left, any_reference right) noexcept {
            left.swap(right);
        }

        basic_any &as_value() noexcept {
            return static_cast<basic_any &>(*this);
        }

        const basic_any &as_value() const noexcept {
            return static_cast<const basic_any &>(*this);
        }

        basic_any construct_from_this() {
            basic_any any{};
            auto tuple = std::make_tuple(false, this, &any);
            basic_any::storage.executer->invoke(implements::any_operation::construct_from, &tuple);
            return any;
        }

        basic_any construct_from_this() const {
            basic_any any{};
            auto tuple = std::make_tuple(true, this, &any);
            basic_any::storage.executer->invoke(implements::any_operation::construct_from, &tuple);
            return any;
        }

    private:
        basic_any *this_pointer{nullptr};
        const foundation::ctti::typeinfo *the_type{nullptr};
    };

    template <typename BasicAny>
    class any_iterator_impl {
    public:
        using value_type = BasicAny;
        using reference = any_reference<value_type>;
        using const_reference = any_reference<value_type>;
        using difference_type = std::ptrdiff_t;
        using pointer = value_type;
        using const_pointer = value_type;
        using iterator_category = std::random_access_iterator_tag;

        static constexpr std::size_t length = sizeof(void *) * 5;

        struct iterator_proxy_vtable {
            virtual ~iterator_proxy_vtable() = default;
            virtual void next() = 0;
            virtual void previous() = 0;
            virtual reference dereference() = 0;
            virtual const_reference const_dereference() const = 0;
            virtual foundation::ctti::typeinfo typeinfo() const = 0;
            virtual void destruct(bool is_local) = 0;
            virtual iterator_proxy_vtable *construct_from_this(core::byte_t *soo_buffer) const noexcept = 0;
            virtual bool compare_equal(const iterator_proxy_vtable *right) const = 0;
            virtual bool compare_less(const iterator_proxy_vtable *right) const = 0;
            virtual difference_type subtract(const iterator_proxy_vtable *right) const = 0;
            virtual any_iterator_impl subtract(difference_type diff) const = 0;
            virtual any_iterator_impl add(difference_type diff) const = 0;
            virtual any_iterator_category iterator_category() const noexcept = 0;
        };

        any_iterator_impl() {
        }

        template <typename IterImpl, typename RealIterator>
        any_iterator_impl(std::in_place_type_t<IterImpl>, RealIterator &&iter, value_type* this_pointer) {
            if constexpr (sizeof(IterImpl) >= length) {
                proxy = new IterImpl(utility::forward<RealIterator>(iter), this_pointer);
            } else {
                proxy =
                    utility::construct_at(reinterpret_cast<IterImpl *>(buffer), utility::forward<RealIterator>(iter), this_pointer);
            }
        }

        ~any_iterator_impl() {
            if (proxy) {
                proxy->destruct(is_local());
            }
        }

        any_iterator_impl(const any_iterator_impl &right) {
            copy_from_other(right);
        }

        any_iterator_impl(any_iterator_impl &&right) noexcept {
            move_from_other(utility::move(right));
        }

        any_iterator_impl &operator=(const any_iterator_impl &right) {
            copy_from_other(right);
            return *this;
        }

        any_iterator_impl &operator=(any_iterator_impl &&right) noexcept {
            move_from_other(utility::move(right));
            return *this;
        }

        any_iterator_impl &operator++() {
            proxy->next();
            return *this;
        }

        any_iterator_impl &operator--() {
            proxy->previous();
            return *this;
        }

        reference operator*() {
            return proxy->dereference();
        }

        const_reference operator*() const {
            return proxy->const_dereference();
        }

        friend difference_type operator-(const any_iterator_impl &left, const any_iterator_impl &right) {
            return left.proxy->subtract(right.proxy);
        }

        friend any_iterator_impl operator-(const any_iterator_impl &left, difference_type diff) {
            return left.proxy->subtract(diff);
        }

        friend any_iterator_impl operator+(const any_iterator_impl &left, difference_type diff) {
            return left.proxy->add(diff);
        }

        friend bool operator==(const any_iterator_impl &left, const any_iterator_impl &right) {
            return left.proxy->compare_equal(right.proxy);
        }

        friend bool operator!=(const any_iterator_impl &left, const any_iterator_impl &right) {
            return !left.proxy->compare_equal(right.proxy);
        }

        friend bool operator<(const any_iterator_impl &left, const any_iterator_impl &right) {
            return left.proxy->compare_less(right.proxy);
        }

        friend bool operator>(const any_iterator_impl &left, const any_iterator_impl &right) {
            return right < left;
        }

        friend bool operator<=(const any_iterator_impl &left, const any_iterator_impl &right) {
            return !(right < left);
        }

        friend bool operator>=(const any_iterator_impl &left, const any_iterator_impl &right) {
            return !(left < right);
        }

        bool empty() const noexcept {
            return static_cast<bool>(proxy);
        }

        void copy_from_other(const any_iterator_impl &right) {
            if (this == utility::addressof(right)) {
                return;
            }
            if (proxy) {
                proxy->destruct(is_local());
            }
            proxy = right.proxy->construct_from_this(this->buffer);
        }

        void move_from_other(any_iterator_impl &&right) {
            if (this == utility::addressof(right) || !right.proxy) {
                return;
            }
            if (proxy) { // 防止内存泄漏
                proxy->destruct(is_local());
            }
            if (right.is_local()) {
                proxy = right.proxy->construct_from_this(this->buffer);
                right.proxy = nullptr;
            } else {
                proxy = utility::exchange(right.proxy, nullptr);
            }
        }

        any_iterator_category category() const noexcept {
            return proxy->iterator_category();
        }

        void swap(any_iterator_impl &right) noexcept {
            if (this == utility::addressof(right)) {
                return;
            }
            if (!is_local() && !right.is_local()) {
                utility::swap(proxy, right.proxy);
            } else {
                any_iterator_impl temp;
                temp.move_from_other(utility::move(*this));
                this->move_from_other(utility::move(right));
                right.move_from_other(utility::move(temp));
            }
        }

        friend void swap(any_iterator_impl &left, any_iterator_impl &right) noexcept {
            left.swap(right);
        }

    private:
        bool is_local() const noexcept {
            return proxy == reinterpret_cast<const void *>(buffer);
        }

        iterator_proxy_vtable *proxy{nullptr};
        core::byte_t buffer[length]{};
    };
}

namespace rainy::utility {
    /**
     * @brief 一个混合所有权的动态变量容器
     * @brief
     * 通常用于对不确定的变量或引用进行运算符或访问操作。同时，因其混合所有权的特性，在持有某个对象的时候，basic_any将会采用RAII策略，反之则仅保留使用权，因此，basic_any有大量功能不同于std::any
     * @param Length 指定any小对象的缓冲区长度（一般留空）
     * @param Align 指定any小对象的对齐长度（一般留空）
     */
    template <std::size_t Length, std::size_t Align>
    class basic_any {
    public:
        static_assert(Length != 0, "What? You serious? Why are you input a 0 for Length, It's illegal!");

        struct default_match {
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
                        using type_list = typename other_trans::tuple_like_to_type_list<
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

        friend class implements::any_reference<basic_any>;

        using iterator = implements::any_iterator_impl<basic_any>;
        using const_iterator = iterator;
        using reference = implements::any_reference<basic_any>;
        using const_reference = implements::any_reference<basic_any>;

        constexpr basic_any() noexcept {
        }

        RAINY_INLINE basic_any(const basic_any &right) {
            using namespace implements;
            switch (right.get_representation()) {
                case any_representation::_small:
                    right.storage.small_any_ctti_manager->copy_(&storage.buffer, &right.storage.buffer);
                    storage.small_any_ctti_manager = right.storage.small_any_ctti_manager;
                    break;
                case any_representation::big:
                    storage.ptr = right.storage.big_any_ctti_manager->copy_(right.storage.ptr);
                    storage.big_any_ctti_manager = right.storage.big_any_ctti_manager;
                    break;
                case any_representation::reference:
                    storage.ptr = right.storage.ptr;
                    break;
                default:
                    break;
            }
            storage.executer = right.storage.executer;
            storage.type_data = right.storage.type_data;
        }

        RAINY_INLINE basic_any(basic_any &&right) noexcept {
            move_from(right);
        }

        template <typename ValueType, type_traits::other_trans::enable_if_t<
                                          type_traits::logical_traits::conjunction_v<
                                              type_traits::logical_traits::negation<type_traits::type_relations::is_any_of<
                                                  type_traits::other_trans::decay_t<ValueType>, basic_any, reference>>,
                                              type_traits::logical_traits::negation<type_traits::primary_types::is_specialization<
                                                  type_traits::other_trans::decay_t<ValueType>, std::in_place_type_t>>,
                                              type_traits::type_properties::is_copy_constructible<ValueType>,
                                              type_traits::logical_traits::negation<is_any<ValueType>>>,
                                          int> = 0>
        RAINY_INLINE basic_any(ValueType &&value) {
            emplace_<ValueType>(utility::forward<ValueType>(value));
        }

        template <
            typename ValueType, typename... Types,
                  type_traits::other_trans::enable_if_t<
                      type_traits::type_properties::is_constructible_v<ValueType, Types...> &&
                          !type_traits::type_relations::is_any_of_v<ValueType, basic_any, reference> && !is_any_v<ValueType>,
                      int> = 0>
        RAINY_INLINE basic_any(std::in_place_type_t<ValueType>, Types &&...args) {
            emplace_<ValueType>(utility::forward<Types>(args)...);
        }

        template <typename ValueType, typename Elem, typename... Types,
                  type_traits::other_trans::enable_if_t<
                      type_traits::type_properties::is_constructible_v<ValueType, std::initializer_list<Elem> &, Types...> && 
                        !type_traits::type_relations::is_any_of_v<ValueType, basic_any,reference> && !is_any_v<ValueType>, int> = 0>
        RAINY_INLINE explicit basic_any(std::in_place_type_t<ValueType>, std::initializer_list<Elem> ilist, Types &&...args) {
            emplace_<ValueType>(ilist, utility::forward<Types>(args)...);
        }

        basic_any(reference right) : basic_any{right.construct_from_this()} {
        }

        RAINY_INLINE ~basic_any() {
            reset();
        }

        basic_any &operator=(const basic_any &right) {
            basic_any tmp = right;
            reset_and_move_from(tmp);
            return *this;
        }

        basic_any &operator=(basic_any &&right) noexcept {
            basic_any tmp = utility::move(right);
            reset_and_move_from(tmp);
            return *this;
        }

        template <typename ValueType,
                  type_traits::other_trans::enable_if_t<
                      !type_traits::type_relations::is_any_of_v<type_traits::other_trans::decay_t<ValueType>, basic_any, reference, const_reference>, int> = 0>
        basic_any &operator=(ValueType &&value) {
            basic_any tmp = utility::forward<ValueType>(value);
            reset_and_move_from(tmp);
            return *this;
        }

        template <
            typename ValueType, typename... Types,
            type_traits::other_trans::enable_if_t<type_traits::type_properties::is_constructible_v<ValueType, Types...>, int> = 0>
        decltype(auto) emplace(Types &&...args) {
            reset();
            return emplace_<ValueType>(utility::forward<Types>(args)...);
        }

        template <typename ValueType, typename Elem, typename... Types,
                  type_traits::other_trans::enable_if_t<
                      type_traits::type_properties::is_constructible_v<ValueType, std::initializer_list<Elem> &, Types...>, int> = 0>
        decltype(auto) emplace(std::initializer_list<Elem> ilist, Types &&...args) {
            reset();
            return emplace_<ValueType>(ilist, utility::forward<Types>(args)...);
        }

        void reset() {
            using namespace implements;
            if (!has_value()) {
                return;
            }
            switch (get_representation()) {
                case any_representation::_small:
                    storage.small_any_ctti_manager->destroy_(&storage.buffer);
                    break;
                case any_representation::big:
                    storage.big_any_ctti_manager->destory_(const_cast<void *>(storage.ptr));
                    break;
                case any_representation::reference:
                default:
                    break;
            }
            storage.type_data = 0;
        }

        void swap(basic_any &right) noexcept {
            basic_any tmp = utility::move(*this);
            reset_and_move_from(right);
            right.reset_and_move_from(tmp);
        }

        RAINY_NODISCARD bool has_value() const noexcept {
            return storage.type_data != 0;
        }

        RAINY_NODISCARD const foundation::ctti::typeinfo &type() const noexcept {
            const foundation::ctti::typeinfo *const info = type_info();
            return info != nullptr ? *info : rainy_typeid(void);
        }

        template <typename Type>
        RAINY_ANY_AS_NODISCARD auto as() -> decltype(auto) {
            static constexpr auto location = source_location::current();
            utility::throw_exception_if<foundation::exceptions::cast::bad_any_cast>(implements::is_as_runnable<Type>(type()),
                                                                                    location);
            return implements::as_impl<Type>(target_as_void_ptr(), type());
        }

        template <typename Type>
        RAINY_ANY_AS_NODISCARD auto as() const -> decltype(auto) {
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
        RAINY_ANY_CAST_TO_POINTER_NODISCARD rain_fn cast_to_pointer() const noexcept
            -> type_traits::pointer_modify::add_pointer_t<type_traits::cv_modify::add_const_t<
                type_traits::other_trans::conditional_t<type_traits::composite_types::is_reference_v<Decayed>,
                                                        type_traits::reference_modify::remove_reference_t<Decayed> *, Decayed *>>> {
            using namespace type_traits;
            using type = other_trans::conditional_t<composite_types::is_reference_v<Decayed>,
                                                    reference_modify::remove_reference_t<Decayed>, Decayed>;
            if (!implements::is_as_runnable<type>(this->type())) {
                return nullptr;
            }
            return static_cast<const type *>(target_as_void_ptr());
        }

        template <typename Decayed>
        RAINY_ANY_CAST_TO_POINTER_NODISCARD rain_fn cast_to_pointer() noexcept -> type_traits::pointer_modify::add_pointer_t<
            type_traits::other_trans::conditional_t<type_traits::composite_types::is_reference_v<Decayed>,
                                                    type_traits::reference_modify::remove_reference_t<Decayed>, Decayed>> {
            using namespace type_traits;
            using type = other_trans::conditional_t<composite_types::is_reference_v<Decayed>,
                                                    reference_modify::remove_reference_t<Decayed>, Decayed>;
            if (!implements::is_as_runnable<type>(this->type())) {
                return nullptr;
            }
            return static_cast<type *>(const_cast<void *>(target_as_void_ptr()));
        }

        template <typename TargetType>
        basic_any &transform() {
            if (is<TargetType>()) {
                basic_any(std::in_place_type<TargetType>, this->template as<TargetType>()).swap(*this);
                return *this;
            } else if (is_convertible<TargetType>()) {
                basic_any(std::in_place_type<TargetType>, this->template convert<TargetType>()).swap(*this);
            }
            return *this;
        }

        template <typename Fx>
        basic_any &transform(Fx &&handler) {
            using namespace type_traits;
            using type_list = typename other_trans::tuple_like_to_type_list<primary_types::param_list_in_tuple<Fx>>::type;
            using target_type = typename other_trans::type_at<0, type_list>::type;
            if (is<target_type>()) {
                basic_any(utility::invoke(handler, this->template as<target_type>())).swap(*this);
            } else if (is_convertible<target_type>()) {
                basic_any(utility::invoke(handler, this->template convert<target_type>())).swap(*this);
            }
            return *this;
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
        RAINY_NODISCARD TargetType convert() const {
            return const_cast<basic_any *>(this)->convert<TargetType>();
        }

        template <typename TargetType>
        RAINY_NODISCARD bool is_convertible() const noexcept {
            return is_any_convertible<TargetType>(this->type());
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
            return ptr;
        }

        friend bool operator<(const basic_any &left, const basic_any &right) {
            std::tuple tuple{&left, &right, implements::any_compare_operation::less};
            return left.storage.executer->invoke(implements::any_operation::compare, &tuple);
        }

        friend bool operator<=(const basic_any &left, const basic_any &right) {
            std::tuple tuple{&left, &right, implements::any_compare_operation::less_eq};
            return left.storage.executer->invoke(implements::any_operation::compare, &tuple);
        }

        friend bool operator==(const basic_any &left, const basic_any &right) {
            std::tuple tuple{&left, &right, implements::any_compare_operation::eq};
            return left.storage.executer->invoke(implements::any_operation::compare, &tuple);
        }

        friend bool operator>=(const basic_any &left, const basic_any &right) {
            std::tuple tuple{&left, &right, implements::any_compare_operation::greater_eq};
            return left.storage.executer->invoke(implements::any_operation::compare, &tuple);
        }

        friend bool operator>(const basic_any &left, const basic_any &right) {
            std::tuple tuple{&left, &right, implements::any_compare_operation::greater};
            return left.storage.executer->invoke(implements::any_operation::compare, &tuple);
        }

        friend bool operator!=(const basic_any &left, const basic_any &right) {
            return !(left == right);
        }

        friend basic_any operator+(const basic_any &left, const basic_any &right) {
            basic_any recv;
            std::tuple tuple{&left, &right, &recv};
            left.storage.executer->invoke(implements::any_operation::add, &tuple);
            return recv;
        }

        friend basic_any operator-(const basic_any &left, const basic_any &right) {
            basic_any recv;
            std::tuple tuple{&left, &right, &recv};
            left.storage.executer->invoke(implements::any_operation::subtract, &tuple);
            return recv;
        }

        friend basic_any operator%(const basic_any &left, const basic_any &right) {
            basic_any recv;
            std::tuple tuple{&left, &right, &recv};
            left.storage.executer->invoke(implements::any_operation::mod, &tuple);
            return recv;
        }

        friend basic_any operator*(const basic_any &left, const basic_any &right) {
            basic_any recv;
            std::tuple tuple{&left, &right, &recv};
            left.storage.executer->invoke(implements::any_operation::multiply, &tuple);
            return recv;
        }

        friend basic_any operator/(const basic_any &left, const basic_any &right) {
            basic_any recv;
            std::tuple tuple{&left, &right, &recv};
            left.storage.executer->invoke(implements::any_operation::divide, &tuple);
            return recv;
        }

        basic_any operator--() {
            basic_any recv;
            std::tuple tuple{this, &recv};
            storage.executer->invoke(implements::any_operation::decr_prefix, &tuple);
            return recv;
        }

        basic_any operator++() {
            basic_any recv;
            std::tuple tuple{this, &recv};
            storage.executer->invoke(implements::any_operation::incr_prefix, &tuple);
            return recv;
        }

        basic_any operator++(int) {
            basic_any recv;
            std::tuple tuple{this, &recv};
            storage.executer->invoke(implements::any_operation::incr_postfix, &tuple);
            return recv;
        }

        basic_any operator--(int) {
            basic_any recv;
            std::tuple tuple{this, &recv};
            storage.executer->invoke(implements::any_operation::decr_postfix, &tuple);
            return recv;
        }

        basic_any operator*() const {
            basic_any recv;
            std::tuple tuple{true, this, &recv};
            storage.executer->invoke(implements::any_operation::dereference, &tuple);
            return recv;
        }

        basic_any &operator+=(const basic_any &right) {
            std::tuple tuple{this, &right, this};
            storage.executer->invoke(implements::any_operation::add, &tuple);
            return *this;
        }

        basic_any &operator-=(const basic_any &right) {
            std::tuple tuple{this, &right, this};
            storage.executer->invoke(implements::any_operation::subtract, &tuple);
            return *this;
        }

        basic_any &operator/=(const basic_any &right) {
            std::tuple tuple{this, &right, this};
            storage.executer->invoke(implements::any_operation::divide, &tuple);
            return *this;
        }

        basic_any &operator%=(const basic_any &right) {
            std::tuple tuple{this, &right, this};
            storage.executer->invoke(implements::any_operation::mod, &tuple);
            return *this;
        }

        basic_any &operator*=(const basic_any &right) {
            std::tuple tuple{this, &right, this};
            storage.executer->invoke(implements::any_operation::multiply, &tuple);
            return *this;
        }

        reference operator[](std::size_t index) {
            reference ret;
            basic_any the_index{std::in_place_type<std::size_t>, index};
            std::tuple tuple{false, this, &ret, &the_index};
            storage.executer->invoke(implements::any_operation::access_element, &tuple);
            return ret;
        }

        const_reference operator[](std::size_t index) const {
            const_reference ret;
            basic_any the_index{std::in_place_type<std::size_t>, index};
            std::tuple tuple{true, this, &ret, &the_index};
            storage.executer->invoke(implements::any_operation::access_element, &tuple);
            return ret;
        }

        reference operator[](const basic_any &key) {
            reference ret;
            std::tuple tuple{false, this, &ret, &key};
            storage.executer->invoke(implements::any_operation::access_element, &tuple);
            return ret;
        }

        const_reference operator[](const basic_any &key) const {
            const_reference ret;
            std::tuple tuple{true, this, &ret, &key};
            storage.executer->invoke(implements::any_operation::access_element, &tuple);
            return ret;
        }

        iterator begin() {
            iterator ret{};
            std::tuple tuple{false, this, &ret};
            storage.executer->invoke(implements::any_operation::call_begin, &tuple);
            return ret;
        }

        const_iterator begin() const {
            const_iterator ret{};
            std::tuple tuple{true, this, &ret};
            storage.executer->invoke(implements::any_operation::call_begin, &tuple);
            return ret;
        }

        iterator end() {
            iterator ret{};
            std::tuple tuple{false, this, &ret};
            storage.executer->invoke(implements::any_operation::call_end, &tuple);
            return ret;
        }

        const_iterator end() const {
            const_iterator ret{};
            std::tuple tuple{true, this, &ret};
            storage.executer->invoke(implements::any_operation::call_end, &tuple);
            return ret;
        }

        /**
         * @brief 获取any存储的当前类型变量对应的哈希值
         * @attention 需当前类型支持计算哈希
         * @attention 优先通过std::hash求值，如果std::hash不可用，则使用rainy::utility::hash作为哈希支持
         */
        std::size_t hash_code() const noexcept {
            std::size_t ret{};
            std::tuple tuple{this, &ret};
            if (storage.executer->invoke(implements::any_operation::eval_hash, &tuple)) {
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
            return matcher<Fx...>{utility::forward<Fx>(funcs)...}.invoke(*this).template convert<Rx>();
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

        template <typename Ty>
        bool destructure(Ty &&receiver) {
            return destructure_impl<false>(utility::forward<Ty>(receiver));
        }

        template <typename Ty>
        bool destructure(Ty &&receiver) const {
            return destructure_impl<true>(utility::forward<Ty>(receiver));
        }

        template <typename... Types>
        std::tuple<Types...> destructure() {
            std::tuple<Types...> ret = {};
            this->destructure(ret);
            return ret;
        }

        template <typename... Types>
        std::tuple<Types...> destructure() const {
            std::tuple<Types...> ret = {};
            this->destructure(ret);
            return ret;
        }

        template <typename CharType, typename Any,
                  type_traits::other_trans::enable_if_t<
                      type_traits::type_relations::is_same_v<type_traits::other_trans::decay_t<Any>, basic_any>, int> = 0>
        friend std::basic_ostream<CharType> &operator<<(std::basic_ostream<CharType> &left, const Any &right) {
            if (!right.has_value()) {
                return left;
            }
            constexpr bool is_char = type_traits::type_relations::is_same_v<CharType, char>;
            std::tuple<std::basic_ostream<CharType> * /* ostream */, const basic_any * /* any */> params{&left, &right};
            std::tuple<bool /* is_char/is_wchar_t */, void * /* params */> tuple{is_char, &params};
            bool ok = right.storage.executer->invoke(implements::any_operation::output_any, &tuple);
            if (!ok) {
                left.setstate(std::ios::ios_base::failbit);
            }
            return left;
        }

        template <typename CharType, typename AnyReference,
                  type_traits::other_trans::enable_if_t<
                      type_traits::type_relations::is_any_of_v<type_traits::other_trans::decay_t<AnyReference>, reference, const_reference>, int> = 0>
        friend std::basic_ostream<CharType> &operator<<(std::basic_ostream<CharType> &left, const AnyReference &right) {
            return left << right.as_value();
        }

        template <typename Type>
        bool is() const noexcept {
            return type() == rainy_typeid(Type);
        }

        template <typename... Types>
        bool is_one_of() {
            return (is<Types>() || ...);
        }

        template <typename... Types>
        bool is_one_of_convertible() {
            return (is_convertible<Types>() || ...);
        }

        bool has_ownership() const noexcept {
            return get_representation() != implements::any_representation::reference && has_value();
        }

        reference as_reference() noexcept {
            return as_ref_impl<implements::any_operation::get_reference>();
        }

        const_reference as_reference() const noexcept {
            return as_ref_impl<implements::any_operation::get_reference>();
        }

        const_reference as_const_reference() const noexcept {
            return as_ref_impl<implements::any_operation::get_reference>();
        }

        reference as_lvalue_reference() noexcept {
            return as_ref_impl<implements::any_operation::get_lvalue_reference>();
        }

        const_reference as_lvalue_reference() const noexcept {
            return as_ref_impl<implements::any_operation::get_lvalue_reference>();
        }

        const_reference as_const_lvalue_reference() const noexcept {
            return as_ref_impl<implements::any_operation::get_lvalue_reference>();
        }

        reference as_rvalue_reference() noexcept {
            return as_ref_impl<implements::any_operation::get_rvalue_reference>();
        }

        const_reference as_rvalue_reference() const noexcept {
            return as_ref_impl<implements::any_operation::get_rvalue_reference>();
        }

        const_reference as_const_rvalue_reference() const noexcept {
            return as_ref_impl<implements::any_operation::get_rvalue_reference>();
        }

        void swap_value(basic_any &reference) {
            auto tuple = std::make_tuple(this, &reference);
            storage.executer->invoke(implements::any_operation::swap_value, &tuple);
        }

    private:
        static constexpr std::uintptr_t rep_mask = 3;

        template <implements::any_operation Operation>
        reference as_ref_impl() noexcept {
            reference reference;
            auto tuple = std::make_tuple(true, this, &reference);
            storage.executer->invoke(Operation, &tuple);
            return reference;
        }

        template <implements::any_operation Operation>
        const_reference as_ref_impl() const noexcept {
            const_reference reference;
            auto tuple = std::make_tuple(true, this, &reference);
            storage.executer->invoke(Operation, &tuple);
            return reference;
        }

        template <bool UseConst, typename Ty>
        bool destructure_impl(Ty &&receiver) const {
            using implements::any_binding_package;
            using namespace type_traits;
            using namespace type_traits::primary_types;
            static_assert(!type_traits::type_properties::is_const_v<Ty>);
            constexpr std::size_t size = implements::eval_for_destructure_pack_receiver_size<Ty>();
            static_assert(size != 0, "Cannot process a invalid receiver!");
            collections::array<any_binding_package, size> array;
            std::size_t count{};
            bool ret = storage.executer->invoke(implements::any_operation::query_for_is_tuple_like, &count);
            if (!ret || count != size) {
                return false;
            }
            std::tuple tuple{this, UseConst, collections::views::make_array_view(array)};
            ret = storage.executer->invoke(implements::any_operation::destructre_this_pack, &tuple);
            if (!ret) {
                return false;
            }
            if constexpr (function_traits<Ty>::valid && !is_member_object_pointer_v<Ty>) {
                this->call_handler_with_array<UseConst>(utility::forward<Ty>(receiver), array,
                                                        type_traits::helper::make_index_sequence<size>{});
                return true;
            } else if constexpr (is_pair_v<Ty>) {
                this->fill_pair_with_array<UseConst>(utility::forward<Ty>(receiver), array);
                return true;
            } else if constexpr (is_tuple_v<Ty>) {
                this->fill_tuple_with_array<UseConst>(utility::forward<Ty>(receiver), array,
                                                      type_traits::helper::make_index_sequence<size>{});
                return true;
            } else if constexpr (constexpr std::size_t size = member_count_v<cv_modify::remove_cvref_t<Ty>>; size != 0) {
                auto so_as_tuple = utility::struct_bind_tuple(receiver);
                this->fill_structure_with_array<UseConst>(so_as_tuple, array, type_traits::helper::make_index_sequence<size>{});
                return true;
            }
            return false;
        }

        RAINY_NODISCARD implements::any_representation get_representation() const noexcept {
            return static_cast<implements::any_representation>(storage.type_data & rep_mask);
        }

        RAINY_NODISCARD const foundation::ctti::typeinfo *type_info() const noexcept {
            return reinterpret_cast<const foundation::ctti::typeinfo *>(storage.type_data & ~rep_mask);
        }

        template <bool Const, typename Fx, std::size_t N, std::size_t... Is>
        void call_handler_with_array(Fx &&handler, const collections::array<implements::any_binding_package, N> &array,
                                     type_traits::helper::index_sequence<Is...>) const {
            using namespace type_traits::other_trans;
            using fn_traits = type_traits::primary_types::function_traits<Fx>;
            using type_list = typename tuple_like_to_type_list<typename fn_traits::tuple_like_type>::type;
            utility::invoke(
                utility::forward<Fx>(handler),
                utility::forward<conditional_t<Const, type_traits::cv_modify::add_const_t<typename type_at<Is, type_list>::type>,
                                               typename type_at<Is, type_list>::type>>(
                    implements::convert_any_binding_package<
                        conditional_t<Const, type_traits::cv_modify::add_const_t<typename type_at<Is, type_list>::type>,
                                      typename type_at<Is, type_list>::type>,
                        Is>::impl(array[Is]))...);
        }

        template <bool Const, std::size_t N, typename Tuple, std::size_t... Is>
        void fill_tuple_with_array(Tuple &tuple, const collections::array<implements::any_binding_package, N> &array,
                                   type_traits::helper::index_sequence<Is...>) const {
            using namespace type_traits::other_trans;
            using utility::swap;
            std::destroy_at(&tuple);
            using type_list = typename tuple_like_to_type_list<type_traits::cv_modify::remove_cvref_t<Tuple>>::type;
            utility::construct_in_place(
                tuple,
                utility::forward<conditional_t<Const, type_traits::cv_modify::add_const_t<typename type_at<Is, type_list>::type>,
                                               typename type_at<Is, type_list>::type>>(
                    implements::convert_any_binding_package<
                        conditional_t<Const, type_traits::cv_modify::add_const_t<typename type_at<Is, type_list>::type>,
                                      typename type_at<Is, type_list>::type>,
                        Is>::impl(array[Is]))...);
        }


        template <bool Const, std::size_t N, typename Tuple, std::size_t... Is>
        void fill_structure_with_array(Tuple &so_as_tuple, const collections::array<implements::any_binding_package, N> &array,
                                       type_traits::helper::index_sequence<Is...>) const {
            using namespace type_traits::other_trans;
            using utility::swap;
            Tuple tmp{so_as_tuple};
            (((*std::get<Is>(tmp)) = implements::convert_any_binding_package<
                  type_traits::other_trans::conditional_t<
                      Const, type_traits::cv_modify::add_const_t<std::remove_pointer_t<std::tuple_element_t<Is, Tuple>>>,
                      std::remove_pointer_t<std::tuple_element_t<Is, Tuple>>>,
                  Is>::impl(array[Is])),
             ...);
            swap(tmp, so_as_tuple);
        }

        template <bool Const, typename Pair, std::size_t... Is>
        void fill_pair_with_array(Pair &pair, const collections::array<implements::any_binding_package, 2> &array) const {
            using implements::convert_any_binding_package;
            Pair tmp{};
            auto &[first, second] = tmp; // 从pair中解包
            using first_type = decltype(first);
            using second_type = decltype(second);
            static_assert(type_traits::type_properties::is_copy_assignable_v<first_type>,
                          "The first element of pair-like type is not assignable");
            static_assert(type_traits::type_properties::is_copy_assignable_v<second_type>,
                          "The second element of pair-like type is not assignable");
            first = implements::convert_any_binding_package<
                type_traits::other_trans::conditional_t<Const, type_traits::cv_modify::add_const_t<first_type>, first_type>,
                0>::impl(array[0]);
            second = implements::convert_any_binding_package<
                type_traits::other_trans::conditional_t<Const, type_traits::cv_modify::add_const_t<second_type>, second_type>,
                1>::impl(array[1]);
            std::swap(tmp, pair);
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
            switch (right.get_representation()) {
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

        RAINY_INLINE void reset_and_move_from(basic_any &right) noexcept {
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
            } else if constexpr (implements::any_is_small<decayed, sizeof(storage_t::buffer)>) {
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

        template <typename ReferenceType, typename Type>
        RAINY_INLINE decltype(auto) emplace_ref(Type &&reference) {
            storage.ptr = utility::addressof(reference);
            storage.type_data = reinterpret_cast<std::uintptr_t>(&rainy_typeid(ReferenceType)) |
                                static_cast<std::uintptr_t>(implements::any_representation::reference);
            storage.executer = &implements::any_operater_policy_object<Type, Length, Align>;
            if constexpr (type_traits::primary_types::is_rvalue_reference_v<ReferenceType>) {
                return utility::move(reference);
            } else {
                return reference;
            }
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

    /**
     * @brief 一个混合所有权的动态变量容器
     * @brief
     * 通常用于对不确定的变量或引用进行运算符或访问操作。同时，因其混合所有权的特性，在持有某个对象的时候，basic_any将会采用RAII策略，反之则仅保留使用权，因此，basic_any有大量功能不同于std::any
     */
    using any = basic_any<>;
}

namespace rainy::utility {
    template <typename Ty, typename... Args,
              type_traits::other_trans::enable_if_t<
                  type_traits::type_properties::is_constructible_v<any, std::in_place_type_t<Ty>, Args...>, int> = 0>
    RAINY_NODISCARD any make_any(Args &&...args) {
        return any{std::in_place_type<Ty>, utility::forward<Args>(args)...};
    }

    template <typename Ty, typename U, typename... Args,
              type_traits::other_trans::enable_if_t<
                  type_traits::type_properties::is_constructible_v<any, std::in_place_type_t<Ty>, std::initializer_list<U> &, Args...>,
                  int> = 0>
    RAINY_NODISCARD any make_any(std::initializer_list<U> initializer_list, Args &&...args) {
        return any{std::in_place_type<Ty>, initializer_list, utility::forward<Args>(args)...};
    }

    template <std::size_t Length, std::size_t Align>
    RAINY_INLINE void swap(::rainy::utility::basic_any<Length, Align> &left, ::rainy::utility::basic_any<Length, Align> &right) {
        left.swap(right);
    }
}

namespace std {
    template <std::size_t Length, std::size_t Align>
    RAINY_INLINE void swap(::rainy::utility::basic_any<Length, Align> &left, ::rainy::utility::basic_any<Length, Align> &right) {
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
            switch (target_type.remove_cvref().hash_code()) {
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
            switch (target_type.remove_cvref().hash_code()) {
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
            return basic_convert(any.target_as_void_ptr(), any.type());
        }

        static target_type basic_convert(const void *target_pointer, const foundation::ctti::typeinfo &type) {
            using namespace foundation::ctti;
            using namespace foundation::exceptions::cast;
            using const_pointer = const CharType *;
            using pointer = CharType *;
            using pointer_to_const = const CharType *const;
            using same_type = std::basic_string_view<CharType, Traits>;
            using same_type_with_const = const std::basic_string_view<CharType, Traits>;
            using basic_string_t = std::basic_string<CharType, Traits>;
            using const_basic_string_t = const std::basic_string<CharType, Traits>;
            switch (type.remove_reference().hash_code()) {
                case rainy_typehash(const_pointer):
                case rainy_typehash(pointer):
                case rainy_typehash(pointer_to_const):
                    // return target_type{static_cast<const_pointer>(target_pointer)}; [[deprecated]]
                    return *static_cast<const const_pointer *>(target_pointer);
                case rainy_typehash(same_type):
                case rainy_typehash(same_type_with_const):
                    return *static_cast<const target_type *>(target_pointer);
                case rainy_typehash(basic_string_t):
                    RAINY_FALLTHROUGH;
                case rainy_typehash(const_basic_string_t): {
                    const basic_string_t &str = *static_cast<const basic_string_t *>(target_pointer);
                    return {str.data(), str.size()};
                }
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
                case rainy_typehash(const CharType *const):
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
    template <typename Rx, typename Ty, typename Any1, typename Any2, typename Func>
    Rx any_operator_helper(Any1 &&left, Any2 &&right, Func &&func) {
        auto &left_operand = left.template as<Ty>();
        if (implements::is_as_runnable<Ty>(right.type().remove_cvref())) {
            return utility::invoke(utility::forward<Func>(func), left_operand, right.template as<Ty>());
        } else if (right.template is_convertible<Ty>()) {
            return utility::invoke(utility::forward<Func>(func), left_operand, right.template convert<Ty>());
        }
        return Rx{};
    }
}

namespace rainy::utility {
    template <typename Ty, typename Any = basic_any<>, bool WithEqual = false>
    struct any_operator {
        using any = Any;

        any add(const any &left, const any &right) const {
            if constexpr (type_traits::extras::meta_method::has_operator_add_v<Ty>) {
                return implements::any_operator_helper<any, Ty>(left, right, foundation::functional::plus{});
            }
            return {};
        }

        any subtract(const any &left, const any &right) const {
            if constexpr (type_traits::extras::meta_method::has_operator_sub_v<Ty>) {
                return implements::any_operator_helper<any, Ty>(left, right, foundation::functional::minus{});
            }
            return {};
        }

        any incr_prefix(any &left) const {
            if constexpr (type_traits::extras::meta_method::has_operator_preinc_v<Ty>) {
                auto &left_operand = left.template as<Ty>();
                ++left_operand;
                return any{std::in_place_type<decltype(--left_operand)>, left_operand};
            }
            return {};
        }

        any decr_prefix(any &left) const {
            if constexpr (type_traits::extras::meta_method::has_operator_predec_v<Ty>) {
                auto &left_operand = left.template as<Ty>();
                return any{std::in_place_type<decltype(--left_operand)>, --left_operand};
            }
            return {};
        }

        any incr_postfix(any &left) const {
            if constexpr (type_traits::extras::meta_method::has_operator_postinc_v<Ty>) {
                auto &left_operand = left.template as<Ty>();
                return left_operand++;
            }
            return {};
        }

        any decr_postfix(any &left) const {
            if constexpr (type_traits::extras::meta_method::has_operator_postdec_v<Ty>) {
                auto &left_operand = left.template as<Ty>();
                return left_operand--;
            }
            return {};
        }

        any multiply(const any &left, const any &right) const {
            if constexpr (type_traits::extras::meta_method::has_operator_mul_v<Ty>) {
                return implements::any_operator_helper<any, Ty>(left, right, foundation::functional::multiplies{});
            }
            return {};
        }

        any divide(const any &left, const any &right) const {
            if constexpr (type_traits::extras::meta_method::has_operator_div_v<Ty>) {
                return implements::any_operator_helper<any, Ty>(left, right, foundation::functional::divides{});
            }
            return {};
        }

        any mod(const any &left, const any &right) const {
            if constexpr (type_traits::extras::meta_method::has_operator_mod_v<Ty>) {
                return implements::any_operator_helper<any, Ty>(left, right, foundation::functional::modulus{});
            }
            return {};
        }

        bool compare_less(const any &left, const any &right) const {
            if constexpr (type_traits::extras::meta_method::has_operator_lt_v<Ty>) {
                return implements::any_operator_helper<bool, Ty>(left, right, foundation::functional::less{});
            }
            return false;
        }

        bool compare_less_equal(const any &left, const any &right) const {
            if constexpr (type_traits::extras::meta_method::has_operator_le_v<Ty>) {
                return implements::any_operator_helper<bool, Ty>(left, right, foundation::functional::less_equal{});
            }
            return false;
        }

        bool compare_equal(const any &left, const any &right) const {
            if constexpr (type_traits::extras::meta_method::has_operator_eq_v<Ty>) {
                return implements::any_operator_helper<bool, Ty>(left, right, foundation::functional::equal{});
            }
            return false;
        }

        bool compare_greater_equal(const any &left, const any &right) const {
            if constexpr (type_traits::extras::meta_method::has_operator_gt_v<Ty>) {
                return implements::any_operator_helper<bool, Ty>(left, right, foundation::functional::greater_equal{});
            }
            return false;
        }

        bool compare_greater(const any &left, const any &right) const {
            if constexpr (type_traits::extras::meta_method::has_operator_ge_v<Ty>) {
                return implements::any_operator_helper<bool, Ty>(left, right, foundation::functional::greater{});
            }
            return false;
        }
    };

    template <typename Ty, typename Any>
    struct any_operator<Ty, Any, true> {
        using any = Any;

        any add(any &left, const any &right) const {
            if constexpr (type_traits::extras::meta_method::has_operator_plus_equal_v<Ty>) {
                return implements::any_operator_helper<any, Ty>(left, right, [](auto &&left, auto &&right) { return left += right; });
            }
            return {};
        }

        any subtract(any &left, const any &right) const {
            if constexpr (type_traits::extras::meta_method::has_operator_sub_equal_v<Ty>) {
                return implements::any_operator_helper<any, Ty>(left, right, [](auto &&left, auto &&right) { return left -= right; });
            }
            return {};
        }

        any multiply(any &left, const any &right) const {
            if constexpr (type_traits::extras::meta_method::has_operator_mul_equal_v<Ty>) {
                return implements::any_operator_helper<any, Ty>(left, right, [](auto &&left, auto &&right) { return left *= right; });
            }
            return {};
        }

        any divide(any &left, const any &right) const {
            if constexpr (type_traits::extras::meta_method::has_operator_div_equal_v<Ty>) {
                return implements::any_operator_helper<any, Ty>(left, right, [](auto &&left, auto &&right) { return left *= right; });
            }
            return {};
        }

        any mod(any &left, const any &right) const {
            if constexpr (type_traits::extras::meta_method::has_operator_mod_equal_v<Ty>) {
                return implements::any_operator_helper<any, Ty>(left, right, [](auto &&left, auto &&right) { return left *= right; });
            }
            return {};
        }
    };
}

namespace rainy::utility::implements {
    template <typename Ty, typename Any, bool WithEqual = false, typename = void>
    RAINY_CONSTEXPR_BOOL is_any_addable_v = false;

    template <typename Ty, typename Any, bool WithEqual>
    RAINY_CONSTEXPR_BOOL
        is_any_addable_v<Ty, Any, WithEqual, type_traits::other_trans::void_t<decltype(&any_operator<Ty, Any, WithEqual>::add)>> =
            true;

    template <typename Ty, typename Any, bool WithEqual = false, typename = void>
    RAINY_CONSTEXPR_BOOL has_any_plus_equal_v = false;

    template <typename Ty, typename Any, bool WithEqual>
    RAINY_CONSTEXPR_BOOL has_any_plus_equal_v<
        Ty, Any, WithEqual, type_traits::other_trans::void_t<decltype(&any_operator<Ty, Any, WithEqual>::plus_equal)>> = true;

    template <typename Ty, typename Any, bool WithEqual = false, typename = void>
    RAINY_CONSTEXPR_BOOL is_any_subable_v = false;

    template <typename Ty, typename Any, bool WithEqual>
    RAINY_CONSTEXPR_BOOL
        is_any_subable_v<Ty, Any, WithEqual, type_traits::other_trans::void_t<decltype(&any_operator<Ty, Any, WithEqual>::subtract)>> =
            true;

    template <typename Ty, typename Any, bool WithEqual = false, typename = void>
    RAINY_CONSTEXPR_BOOL is_any_multable_v = false;

    template <typename Ty, typename Any, bool WithEqual>
    RAINY_CONSTEXPR_BOOL is_any_multable_v<Ty, Any, WithEqual,
                                           type_traits::other_trans::void_t<decltype(&any_operator<Ty, Any, WithEqual>::multiply)>> =
        true;

    template <typename Ty, typename Any, bool WithEqual = false, typename = void>
    RAINY_CONSTEXPR_BOOL is_any_divable_v = false;

    template <typename Ty, typename Any, bool WithEqual>
    RAINY_CONSTEXPR_BOOL
        is_any_divable_v<Ty, Any, WithEqual, type_traits::other_trans::void_t<decltype(&any_operator<Ty, Any, WithEqual>::divide)>> =
            true;

    template <typename Ty, typename Any, bool WithEqual = false, typename = void>
    RAINY_CONSTEXPR_BOOL is_any_modable_v = false;

    template <typename Ty, typename Any, bool WithEqual>
    RAINY_CONSTEXPR_BOOL
        is_any_modable_v<Ty, Any, WithEqual, type_traits::other_trans::void_t<decltype(&any_operator<Ty, Any, WithEqual>::mod)>> =
            true;

    template <typename Ty, typename Any, bool WithEqual = false, typename = void>
    RAINY_CONSTEXPR_BOOL is_any_preincable_v = false;

    template <typename Ty, typename Any, bool WithEqual>
    RAINY_CONSTEXPR_BOOL is_any_preincable_v<
        Ty, Any, WithEqual, type_traits::other_trans::void_t<decltype(&any_operator<Ty, Any, WithEqual>::incr_prefix)>> = true;

    template <typename Ty, typename Any, bool WithEqual = false, typename = void>
    RAINY_CONSTEXPR_BOOL is_any_predecable_v = false;

    template <typename Ty, typename Any, bool WithEqual>
    RAINY_CONSTEXPR_BOOL is_any_predecable_v<
        Ty, Any, WithEqual, type_traits::other_trans::void_t<decltype(&any_operator<Ty, Any, WithEqual>::decr_prefix)>> = true;

    template <typename Ty, typename Any, bool WithEqual = false, typename = void>
    RAINY_CONSTEXPR_BOOL is_any_postincable_v = false;

    template <typename Ty, typename Any, bool WithEqual>
    RAINY_CONSTEXPR_BOOL is_any_postincable_v<
        Ty, Any, WithEqual, type_traits::other_trans::void_t<decltype(&any_operator<Ty, Any, WithEqual>::incr_postfix)>> = true;

    template <typename Ty, typename Any, bool WithEqual = false, typename = void>
    RAINY_CONSTEXPR_BOOL is_any_postdecable_v = false;

    template <typename Ty, typename Any, bool WithEqual>
    RAINY_CONSTEXPR_BOOL is_any_postdecable_v<
        Ty, Any, WithEqual, type_traits::other_trans::void_t<decltype(&any_operator<Ty, Any, WithEqual>::decr_postfix)>> = true;

    template <typename Ty, typename Any, bool WithEqual = false, typename = void>
    RAINY_CONSTEXPR_BOOL is_any_less_compareable_v = false;

    template <typename Ty, typename Any, bool WithEqual>
    RAINY_CONSTEXPR_BOOL is_any_less_compareable_v<
        Ty, Any, WithEqual, type_traits::other_trans::void_t<decltype(&any_operator<Ty, Any, WithEqual>::compare_less)>> = true;

    template <typename Ty, typename Any, bool WithEqual = false, typename = void>
    RAINY_CONSTEXPR_BOOL is_any_less_eq_compareable_v = false;

    template <typename Ty, typename Any, bool WithEqual>
    RAINY_CONSTEXPR_BOOL is_any_less_eq_compareable_v<
        Ty, Any, WithEqual, type_traits::other_trans::void_t<decltype(&any_operator<Ty, Any, WithEqual>::compare_less_equal)>> = true;

    template <typename Ty, typename Any, bool WithEqual = false, typename = void>
    RAINY_CONSTEXPR_BOOL is_any_eq_compareable_v = false;

    template <typename Ty, typename Any, bool WithEqual>
    RAINY_CONSTEXPR_BOOL is_any_eq_compareable_v<
        Ty, Any, WithEqual, type_traits::other_trans::void_t<decltype(&any_operator<Ty, Any, WithEqual>::compare_equal)>> = true;

    template <typename Ty, typename Any, bool WithEqual = false, typename = void>
    RAINY_CONSTEXPR_BOOL is_any_gt_compareable_v = false;

    template <typename Ty, typename Any, bool WithEqual>
    RAINY_CONSTEXPR_BOOL is_any_gt_compareable_v<
        Ty, Any, WithEqual, type_traits::other_trans::void_t<decltype(&any_operator<Ty, Any, WithEqual>::compare_greater)>> = true;

    template <typename Ty, typename Any, bool WithEqual = false, typename = void>
    RAINY_CONSTEXPR_BOOL is_any_greater_eq_compareable_v = false;

    template <typename Ty, typename Any, bool WithEqual>
    RAINY_CONSTEXPR_BOOL is_any_greater_eq_compareable_v<
        Ty, Any, WithEqual, type_traits::other_trans::void_t<decltype(&any_operator<Ty, Any, WithEqual>::compare_greater_equal)>> =
        true;
}

namespace rainy::utility::implements {
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

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_index_tuple_v = false;

    template <typename... Args>
    RAINY_CONSTEXPR_BOOL is_index_tuple_v<std::tuple<Args...>> = true;

    template <typename Any, typename Tuple, std::size_t... Is>
    void get_any_from_tuple(Tuple &&tuple, Any &result, std::size_t index, type_traits::helper::index_sequence<Is...>) {
        std::size_t i = 0;
        (
            [&] {
                if (i++ == index && !result.has_value()) {
                    result = std::get<Is>(utility::forward<Tuple>(tuple));
                }
            }(),
            ...);
    }

    template <typename Ty>
    using add_const_helper_for_access_element =
        type_traits::cv_modify::add_const_t<type_traits::reference_modify::remove_reference_t<Ty>>;

    template <typename Ty>
    using access_elements_construct_type = type_traits::other_trans::conditional_t<
        type_traits::composite_types::is_reference_v<Ty>,
        type_traits::other_trans::conditional_t<
            type_traits::primary_types::is_rvalue_reference_v<Ty>,
            type_traits::reference_modify::add_rvalue_reference_t<add_const_helper_for_access_element<Ty>>,
            type_traits::reference_modify::add_lvalue_reference_t<add_const_helper_for_access_element<Ty>>>,
        add_const_helper_for_access_element<Ty>>;

    template <typename Iter>
    constexpr any_iterator_category get_iterator_category() noexcept {
        using namespace type_traits::extras::iterators;
        if (is_contiguous_iterator_v<Iter>) {
            return any_iterator_category::contiguous_iterator;
        } else if constexpr (is_random_access_iterator_v<Iter>) {
            return any_iterator_category::random_access_iterator;
        } else if constexpr (is_bidirectional_iterator_v<Iter>) {
            return any_iterator_category::bidirectional_iterator;
        } else if constexpr (is_bidirectional_iterator_v<Iter>) {
            return any_iterator_category::forward_iterator;
        } else if constexpr (is_output_iterator_v<Iter> && !is_input_iterator_v<Iter>) {
            return any_iterator_category::output_iterator;
        } else {
            static_assert(is_input_iterator_v<Iter> && !is_output_iterator_v<Iter>);
            return any_iterator_category::input_iterator;
        }
    }

    template <typename BasicAny, typename Type>
    struct any_proxy_iterator : BasicAny::iterator::iterator_proxy_vtable {
        using iterator_t = typename Type::iterator;
        using const_iterator_t = typename Type::const_iterator;
        using proxy_t = typename BasicAny::iterator::iterator_proxy_vtable;
        using basic_any = BasicAny;

        using any_iterator_t = typename basic_any::iterator;

        using difference_type = typename any_iterator_t::difference_type;

        any_proxy_iterator(iterator_t iterator, basic_any *this_pointer) : iter{iterator}, this_pointer{this_pointer} {
        }

        void destruct(bool is_local) override {
            if (is_local) {
                std::destroy_at(&iter);
            } else {
                std::destroy_at(&iter);
                delete this;
            }
        }

        void next() override {
            ++iter;
        }

        void previous() override {
            if constexpr (get_iterator_category<iterator_t>() <= any_iterator_category::forward_iterator) {
                foundation::exceptions::logic::throw_any_not_implemented("Current iterator category is not support this operation");
            } else {
                --iter;
            }
        }

        typename BasicAny::reference dereference() override {
            return {*iter, this_pointer};
        }

        typename BasicAny::const_reference const_dereference() const override {
            return {*iter, this_pointer};
        }

        foundation::ctti::typeinfo typeinfo() const override {
            return rainy_typeid(any_proxy_iterator);
        }

        proxy_t *construct_from_this(core::byte_t *soo_buffer) const noexcept override {
            if constexpr (sizeof(type_traits::other_trans::decay_t<decltype(*this)>) >= BasicAny::iterator::length) {
                return ::new any_proxy_iterator(this->iter, this_pointer);
            } else {
                return utility::construct_at(reinterpret_cast<any_proxy_iterator *>(soo_buffer), this->iter, this_pointer);
            }
        }

        bool compare_less(const proxy_t *right) const override {
            if constexpr (type_traits::extras::meta_method::has_operator_le_v<iterator_t>) {
                using cit = const_any_proxy_iterator<BasicAny, Type>;
                if (right->typeinfo() == rainy_typeid(any_proxy_iterator)) {
                    return iter < static_cast<const any_proxy_iterator *>(right)->iter;
                } else if (right->typeinfo() == rainy_typeid(cit)) {
                    return iter < static_cast<const const_any_proxy_iterator<BasicAny, Type> *>(right)->iter;
                }
            }
            return false;
        }

        bool compare_equal(const proxy_t *right) const override {
            using cit = const_any_proxy_iterator<BasicAny, Type>;
            if (right->typeinfo() == rainy_typeid(any_proxy_iterator)) {
                return iter == static_cast<const any_proxy_iterator *>(right)->iter;
            } else if (right->typeinfo() == rainy_typeid(cit)) {
                return iter == static_cast<const const_any_proxy_iterator<BasicAny, Type> *>(right)->iter;
            }
            return false;
        }

        any_iterator_category iterator_category() const noexcept override {
            constexpr any_iterator_category category = get_iterator_category<iterator_t>();
            return category;
        }

        any_iterator_t add(difference_type diff) const override {
            if constexpr (type_traits::extras::meta_method::has_operator_addition_v<iterator_t>) {
                return {std::in_place_type<any_proxy_iterator>, iter + diff, this_pointer};
            }
            return {};
        }

        any_iterator_t subtract(ptrdiff_t diff) const override {
            if constexpr (type_traits::extras::meta_method::has_operator_sub_v<iterator_t>) {
                return any_iterator_t{std::in_place_type<any_proxy_iterator>, iter - diff, this_pointer};
            }
            return {};
        }

        difference_type subtract(const proxy_t *right) const override {
            using cit = any_proxy_iterator<BasicAny, Type>;
            if constexpr (type_traits::extras::meta_method::has_operator_sub_v<iterator_t>) {
                if (right->typeinfo() == rainy_typeid(any_proxy_iterator)) {
                    return iter - static_cast<const any_proxy_iterator *>(right)->iter;
                } else if (right->typeinfo() == rainy_typeid(cit)) {
                    return iter - static_cast<const cit *>(right)->iter;
                }
            }
            return {};
        }

        iterator_t iter;
        basic_any *this_pointer;
    };

    template <typename BasicAny, typename Type>
    struct const_any_proxy_iterator : BasicAny::iterator::iterator_proxy_vtable {
        using iterator_t = typename Type::const_iterator;
        using proxy_t = typename BasicAny::iterator::iterator_proxy_vtable;
        using basic_any = BasicAny;

        using any_iterator_t = typename basic_any::iterator;

        using difference_type = typename any_iterator_t::difference_type;

        const_any_proxy_iterator(const iterator_t &iterator, basic_any *this_pointer) : iter{iterator}, this_pointer{this_pointer} {
        }

        void destruct(bool is_local) override {
            if (is_local) {
                std::destroy_at(&iter);
            } else {
                std::destroy_at(&iter);
                delete this;
            }
        }

        void next() override {
            ++iter;
        }

        void previous() override {
            if constexpr (get_iterator_category<iterator_t>() <= any_iterator_category::forward_iterator) {
                foundation::exceptions::logic::throw_any_not_implemented("Current iterator category is not support this operation");
            } else {
                --iter;
            }
        }

        typename BasicAny::reference dereference() override {
            return {*iter, this_pointer};
        }

        typename BasicAny::const_reference const_dereference() const override {
            return {*iter, this_pointer};
        }

        foundation::ctti::typeinfo typeinfo() const override {
            return rainy_typeid(const_any_proxy_iterator);
        }

        proxy_t *construct_from_this(core::byte_t *soo_buffer) const noexcept override {
            if constexpr (sizeof(type_traits::other_trans::decay_t<decltype(*this)>) >= BasicAny::iterator::length) {
                return ::new const_any_proxy_iterator(this->iter, this_pointer);
            } else {
                return utility::construct_at(reinterpret_cast<const_any_proxy_iterator *>(soo_buffer), this->iter, this_pointer);
            }
        }

        any_iterator_category iterator_category() const noexcept override {
            constexpr any_iterator_category category = get_iterator_category<iterator_t>();
            return category;
        }

        bool compare_equal(const proxy_t *right) const override {
            using it = any_proxy_iterator<BasicAny, Type>;
            if (right->typeinfo() == rainy_typeid(it)) {
                return iter == static_cast<const it *>(right)->iter;
            } else if (right->typeinfo() == rainy_typeid(const_any_proxy_iterator)) {
                return iter == static_cast<const const_any_proxy_iterator<BasicAny, Type> *>(right)->iter;
            }
            return false;
        }

        bool compare_less(const proxy_t *right) const override {
            if constexpr (type_traits::extras::meta_method::has_operator_le_v<iterator_t>) {
                using it = any_proxy_iterator<BasicAny, Type>;
                if (right->typeinfo() == rainy_typeid(it)) {
                    return iter < static_cast<const it *>(right)->iter;
                } else if (right->typeinfo() == rainy_typeid(const_any_proxy_iterator)) {
                    return iter < static_cast<const const_any_proxy_iterator<BasicAny, Type> *>(right)->iter;
                }
            }
            return false;
        }

        any_iterator_t add(difference_type diff) const override {
            if constexpr (type_traits::extras::meta_method::has_operator_addition_v<iterator_t>) {
                return {std::in_place_type<const_any_proxy_iterator>, iter + diff, this_pointer};
            }
            return {};
        }

        any_iterator_t subtract(difference_type diff) const override {
            if constexpr (type_traits::extras::meta_method::has_operator_sub_v<iterator_t>) {
               return {std::in_place_type<const_any_proxy_iterator>, iter - diff, this_pointer};
            }
            return {};
        }
        
        difference_type subtract(const proxy_t *right) const override {
            using it = any_proxy_iterator<BasicAny, Type>;
            if constexpr (type_traits::extras::meta_method::has_operator_sub_v<iterator_t>) {
                if (right->typeinfo() == rainy_typeid(it)) {
                    return iter - static_cast<const it *>(right)->iter;
                } else if (right->typeinfo() == rainy_typeid(const_any_proxy_iterator)) {
                    return iter - static_cast<const it *>(right)->iter;
                }
            }
            return {};
        }

        iterator_t iter;
        basic_any *this_pointer;
    };

    template <typename Ty, std::size_t Length, std::size_t Align>
    bool any_operater_policy::invoke_impl(operation op, void *const data) {
        using namespace type_traits;
        using namespace type_traits::other_trans;
        using namespace type_traits::composite_types;
        using namespace type_traits::primary_types;
        using namespace type_traits::reference_modify;
        using any = basic_any<Length, Align>;
        using remove_cvref_t = type_traits::cv_modify::remove_cvref_t<Ty>;
        switch (op) {
            case operation::compare: {
                using namespace foundation::exceptions::logic;
                using pack = const std::tuple<const any *, const any *, any_compare_operation> *;
                const auto *res = static_cast<pack>(data);
                const any *left = std::get<0>(*res);
                const any *right = std::get<1>(*res);
                any_compare_operation operation = std::get<2>(*res);
                switch (operation) {
                    case any_compare_operation::less: {
                        if constexpr (is_any_less_compareable_v<remove_cvref_t, any>) {
                            return any_operator<remove_cvref_t, any>{}.compare_less(*left, *right);
                        } else {
                            foundation::exceptions::logic::throw_any_not_implemented("Current type not support this operation: less");
                        }
                        break;
                    }
                    case any_compare_operation::less_eq: {
                        if constexpr (is_any_less_eq_compareable_v<remove_cvref_t, any>) {
                            return any_operator<remove_cvref_t, any>{}.compare_less_equal(*left, *right);
                        } else {
                            foundation::exceptions::logic::throw_any_not_implemented(
                                "Current type not support this operation: less_eq");
                        }
                        break;
                    }
                    case any_compare_operation::eq: {
                        if constexpr (is_any_eq_compareable_v<remove_cvref_t, any>) {
                            return any_operator<remove_cvref_t, any>{}.compare_equal(*left, *right);
                        } else {
                            foundation::exceptions::logic::throw_any_not_implemented("Current type not support this operation: eq");
                        }
                        break;
                    }
                    case any_compare_operation::greater_eq: {
                        if constexpr (is_any_greater_eq_compareable_v<remove_cvref_t, any>) {
                            return any_operator<remove_cvref_t, any>{}.compare_greater_equal(*left, *right);
                        } else {
                            foundation::exceptions::logic::throw_any_not_implemented(
                                "Current type not support this operation: greater_eq");
                        }
                        break;
                    }
                    case any_compare_operation::greater: {
                        if constexpr (is_any_gt_compareable_v<remove_cvref_t, any>) {
                            return any_operator<remove_cvref_t, any>{}.compare_greater(*left, *right);
                        } else {
                            foundation::exceptions::logic::throw_any_not_implemented(
                                "Current type not support this operation: greater");
                        }
                        break;
                    }
                }
                break;
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
                break;
            }
            case operation::query_for_is_tuple_like: {
                constexpr std::size_t member_count = member_count_v<type_traits::cv_modify::remove_cvref_t<Ty>>;
                if constexpr (member_count == 0) {
                    return false;
                }
                (*static_cast<std::size_t *>(data)) = member_count;
                return true;
            }
            case operation::destructre_this_pack: {
                using implements::any_binding_package;
                constexpr std::size_t member_count = member_count_v<remove_cvref_t>;
                if constexpr (member_count != 0) {
                    auto *res = static_cast<std::tuple<any *, bool, collections::views::array_view<any_binding_package>> *>(data);
                    any *object = std::get<0>(*res);
                    bool use_const = std::get<1>(*res);
                    auto &view = std::get<2>(*res);
                    if (use_const) {
                        auto tuple_ptr = utility::struct_bind_tuple(std::as_const(object)->template as<remove_cvref_t>());
                        std::apply(
                            [&](auto *...elems) {
                                std::size_t idx = 0;
                                ((view[idx++] =
                                      any_binding_package{static_cast<const void *>(elems), &rainy_typeid(decltype(*elems))}),
                                 ...);
                            },
                            tuple_ptr);
                    } else {
                        auto tuple_ptr = utility::struct_bind_tuple(object->template as<remove_cvref_t>());
                        std::apply(
                            [&](auto *...elems) {
                                std::size_t idx = 0;
                                ((view[idx++] =
                                      any_binding_package{static_cast<const void *>(elems), &rainy_typeid(decltype(*elems))}),
                                 ...);
                            },
                            tuple_ptr);
                    }
                    return true;
                }
                break;
            }
            case operation::output_any: {
                auto *res = static_cast<std::tuple<bool /* is_char/is_wchar_t */, void * /* params */> *>(data);
                bool is_char = std::get<0>(*res);
                void *output_data = std::get<1>(*res);
                if (is_char) {
                    if constexpr (is_char_any_can_output<Ty>) {
                        auto *out =
                            static_cast<std::tuple<std::basic_ostream<char> * /* ostream */, const any * /* any */> *>(output_data);
                        (*std::get<0>(*out)) << std::get<1>(*out)->template as<Ty>();
                        return true;
                    }
                } else {
                    if constexpr (is_wchar_any_can_output<Ty>) {
                        auto *out =
                            static_cast<std::tuple<std::basic_ostream<wchar_t> * /* ostream */, const any * /* any */> *>(output_data);
                        (*std::get<0>(*out)) << std::get<1>(*out)->template as<Ty>();
                        return true;
                    }
                }
                break;
            }
            case operation::add: {
                auto *res = reinterpret_cast<std::tuple<const any * /* left */, const any * /* right */, any * /* recv */

                                                        > *>(data);
                auto &left = *std::get<0>(*res);
                auto &right = *std::get<1>(*res);
                auto &recv = *std::get<2>(*res);
                if (&recv == &left) {
                    if constexpr (is_any_addable_v<remove_cvref_t, any, true>) {
                        any_operator<remove_cvref_t, any, true>{}.add(recv, right);
                    }
                    return true;
                } else if constexpr (is_any_addable_v<remove_cvref_t, any, false>) {
                    recv = any_operator<remove_cvref_t, any, false>{}.add(left, right);
                    return true;
                }
                break;
            }
            case operation::subtract: {
                auto *res = static_cast<std::tuple<const any * /* left */, const any * /* right */, any * /* recv */
                                                   > *>(data);
                auto &left = *std::get<0>(*res);
                auto &right = *std::get<1>(*res);
                auto &recv = *std::get<2>(*res);
                if (&recv == &left) {
                    if constexpr (is_any_subable_v<remove_cvref_t, any, true>) {
                        any_operator<remove_cvref_t, any, true>{}.subtract(recv, right);
                    }
                    return true;
                } else if constexpr (is_any_subable_v<remove_cvref_t, any, false>) {
                    recv = any_operator<remove_cvref_t, any, false>{}.subtract(left, right);
                    return true;
                }
                return true;
                break;
            }
            case operation::incr_prefix: {
                if constexpr (is_any_preincable_v<remove_cvref_t, any>) {
                    auto *res = static_cast<std::tuple<any * /* operand */, any * /* recv */
                                                       > *>(data);
                    auto &left = *std::get<0>(*res);
                    auto &recv = *std::get<1>(*res);
                    recv = any_operator<remove_cvref_t, any>{}.incr_prefix(left);
                    std::cout << "recv.type().name() = " << recv.type().name() << "\n";
                    return true;
                }
                break;
            }
            case operation::decr_prefix: {
                if constexpr (is_any_predecable_v<remove_cvref_t, any>) {
                    auto *res = static_cast<std::tuple<any * /* operand */, any * /* recv */
                                                       > *>(data);
                    auto &left = *std::get<0>(*res);
                    auto &recv = *std::get<1>(*res);
                    recv = any_operator<remove_cvref_t, any>{}.decr_prefix(left);
                    return true;
                }
                break;
            }
            case operation::incr_postfix: {
                if constexpr (is_any_postincable_v<remove_cvref_t, any>) {
                    auto *res = static_cast<std::tuple<any * /* operand */, any * /* recv */
                                                       > *>(data);
                    auto &left = *std::get<0>(*res);
                    auto &recv = *std::get<1>(*res);
                    recv = any_operator<remove_cvref_t, any>{}.incr_postfix(left);
                    return true;
                }
                break;
            }
            case operation::decr_postfix: {
                if constexpr (is_any_postdecable_v<remove_cvref_t, any>) {
                    auto *res = static_cast<std::tuple<any * /* operand */, any * /* recv */
                                                       > *>(data);
                    auto &left = *std::get<0>(*res);
                    auto &recv = *std::get<1>(*res);
                    recv = any_operator<remove_cvref_t, any>{}.decr_postfix(left);
                    return true;
                }
                break;
            }
            case operation::multiply: {
                auto *res = static_cast<std::tuple<const any * /* left */, const any * /* right */, any * /* recv */
                                                   > *>(data);
                auto &left = *std::get<0>(*res);
                auto &right = *std::get<1>(*res);
                auto &recv = *std::get<2>(*res);
                if (&recv == &left) {
                    if constexpr (is_any_multable_v<remove_cvref_t, any, true>) {
                        any_operator<remove_cvref_t, any, true>{}.multiply(recv, right);
                    }
                    return true;
                } else if constexpr (is_any_multable_v<remove_cvref_t, any, false>) {
                    recv = any_operator<remove_cvref_t, any, false>{}.multiply(left, right);
                    return true;
                }
                break;
            }
            case operation::divide: {
                auto *res = static_cast<std::tuple<const any * /* left */, const any * /* right */, any * /* recv */
                                                   > *>(data);
                auto &left = *std::get<0>(*res);
                auto &right = *std::get<1>(*res);
                auto &recv = *std::get<2>(*res);
                if (&recv == &left) {
                    if constexpr (is_any_divable_v<remove_cvref_t, any, true>) {
                        any_operator<remove_cvref_t, any, true>{}.divide(recv, right);
                    }
                    return true;
                } else if constexpr (is_any_divable_v<remove_cvref_t, any, false>) {
                    recv = any_operator<remove_cvref_t, any, false>{}.divide(left, right);
                    return true;
                }
                break;
            }
            case operation::mod: {
                auto *res = static_cast<std::tuple<const any * /* left */, const any * /* right */, any * /* recv */
                                                   > *>(data);
                auto &left = *std::get<0>(*res);
                auto &right = *std::get<1>(*res);
                auto &recv = *std::get<2>(*res);
                if (&recv == &left) {
                    if constexpr (is_any_modable_v<remove_cvref_t, any, true>) {
                        any_operator<remove_cvref_t, any, true>{}.mod(recv, right);
                    }
                    return true;
                } else if constexpr (is_any_modable_v<remove_cvref_t, any, false>) {
                    recv = any_operator<remove_cvref_t, any, false>{}.mod(left, right);
                    return true;
                }
                break;
            }
            case operation::dereference: {
                using namespace type_traits::extras::meta_method;
                auto *res = static_cast<std::tuple<bool /* is_const */, any * /* left */, any * /* recv */
                                                   > *>(data);
                bool is_const = std::get<0>(*res);
                auto &value = (*std::get<1>(*res)).template as<Ty>();
                any *recv = std::get<2>(*res);
                if (is_const) {
                    if constexpr (has_operator_deref_v<type_traits::cv_modify::add_const_t<Ty>>) {
                        recv->template emplace<decltype(*value)>(*value);
                    }
                } else {
                    if constexpr (has_operator_deref_v<Ty>) {
                        recv->template emplace<decltype(*value)>(*value);
                    } else if constexpr (has_operator_deref_v<type_traits::cv_modify::add_const_t<Ty>>) {
                        recv->template emplace<decltype(*value)>(*value);
                    }
                }
                break;
            }
            case operation::access_element: {
                bool has_value{false};
                using namespace type_traits;
                using const_as = type_traits::cv_modify::add_const_t<type_traits::reference_modify::remove_reference_t<Ty>>;
                auto *res = static_cast<std::tuple<bool /* is_const */, any * /* value */, typename any::reference * /* recv */,
                                                   const any * /* index */> *>(data);
                bool is_const = std::get<0>(*res);
                any *value = std::get<1>(*res);
                auto &recv = *std::get<2>(*res);
                auto &key = std::get<3>(*res);
                if constexpr (extras::meta_method::has_operator_index_v<Ty>) {
                    using elem_t = decltype(utility::declval<Ty>()[0]);
                    std::size_t index{};
                    if (key->template is<std::size_t>()) {
                        index = key->template as<std::size_t>();
                    } else if (key->template is_convertible<std::size_t>()) {
                        index = key->template convert<std::size_t>();
                    }
                    if (is_const || value->type().is_const()) {
                        if constexpr (extras::meta_method::has_operator_index_v<const_as>) {
                            const auto &extract = (*std::get<1>(*res)).template as<const_as>();
                            utility::construct_in_place(recv,
                                                        utility::forward<access_elements_construct_type<elem_t>>(extract[index]), value);
                        }
                    } else {
                        auto &extract = (*std::get<1>(*res)).template as<Ty>();
                        utility::construct_in_place(recv, utility::forward<elem_t>(extract[index]), value);
                    }
                    has_value = recv.has_value();
                } else if constexpr (extras::meta_method::has_operator_index_for_key_v<remove_cvref_t>) {
                    using key_type = typename remove_cvref_t::key_type;
                    using elem_t = decltype(utility::declval<remove_cvref_t>()[utility::declval<key_type>()]);
                    key_type key_val{};
                    if (key->template is<key_type>()) {
                        key_val = key->template as<key_type>();
                    } else if (key->template is_convertible<key_type>()) {
                        key_val = key->template convert<key_type>();
                    }
                    if (is_const || value->type().is_const()) {
                        if constexpr (extras::meta_method::has_operator_index_for_key_v<const_as>) {
                            const auto &extract = (*std::get<1>(*res)).template as<const_as>();
                            utility::construct_in_place(
                                recv, utility::forward<access_elements_construct_type<elem_t>>(extract[utility::move(key_val)]),
                                value);
                        }
                    } else {
                        auto &extract = (*std::get<1>(*res)).template as<Ty>();
                        constexpr bool is_const =
                            type_traits::type_properties::is_const_v<type_traits::reference_modify::remove_reference_t<Ty>>;
                        if constexpr (is_const) {
                            constexpr bool has_operator_in_const =
                                extras::meta_method::has_operator_index_for_key_v<type_traits::cv_modify::add_const_t<remove_cvref_t>>;
                            if constexpr (has_operator_in_const) {
                                utility::construct_in_place(
                                    recv, utility::forward<access_elements_construct_type<elem_t>>(extract[utility::move(key_val)]),
                                    value);
                            }
                        } else {
                            utility::construct_in_place(recv, utility::forward<elem_t>(extract[utility::move(key_val)]), value);
                        }
                    }
                    has_value = recv.has_value();
                } else if constexpr (is_index_tuple_v<Ty>) {
                    std::size_t index{0};
                    if (key->template is<std::size_t>()) {
                        index = key->template as<std::size_t>();
                    } else if (key->template is_convertible<std::size_t>()) {
                        index = key->template convert<std::size_t>();
                    }
                    constexpr auto find_fn = [](bool is_const, std::size_t index, auto &&recv, auto &&extract) {
                        // NOLINT BEGIN
                        std::apply(
                            [&](auto &&...elems) {
                                std::size_t i{0};
                                (void) ((i++ == index
                                             ? (is_const
                                                    ? recv.template emplace<access_elements_construct_type<decltype(elems)>>(elems)
                                                    : recv.template emplace<decltype(elems)>(elems),
                                                true)
                                             : false) ||
                                        ...);
                            },
                            extract);
                        // NOLINT END
                    };
                    if (is_const || value->type().is_const()) {
                        find_fn(true, index, recv, value->template as<const_as>());
                    } else {
                        find_fn(false, index, recv, value->template as<Ty>());
                    }
                }
                return has_value;
            }
            case operation::call_begin: {
                if constexpr (type_traits::extras::iterators::has_iterator_v<Ty>) {
                    using tuple_t = std::tuple<bool /* is const */, any * /* value */, typename any::iterator * /* recv */>;
                    using remove_ref_t = type_traits::reference_modify::remove_reference_t<Ty>;
                    using remove_cvref_t = type_traits::cv_modify::remove_cvref_t<Ty>;
                    using const_as = type_traits::cv_modify::add_const_t<type_traits::reference_modify::remove_reference_t<Ty>>;
                    using iterator = any_proxy_iterator<any, remove_cvref_t>;
                    auto *res = static_cast<tuple_t *>(data);
                    bool is_const = std::get<0>(*res);
                    any *value = std::get<1>(*res);
                    auto *recv = std::get<2>(*res);
                    if constexpr (type_traits::extras::iterators::has_const_iterator_v<Ty>) {
                        if (is_const || value->type().is_const()) {
                            using const_iterator = const_any_proxy_iterator<any, remove_cvref_t>;
                            if constexpr (type_traits::extras::meta_method::has_cbegin_v<Ty>) {
                                utility::construct_at(recv, std::in_place_type<const_iterator>,
                                                      value->template as<const_as>().cbegin(), value);
                            } else if constexpr (type_traits::extras::meta_method::has_begin_v<const_as>) {
                                utility::construct_at(recv, std::in_place_type<const_iterator>,
                                                      value->template as<const_as>().begin(), value);
                            }
                        }
                    }
                    if constexpr (type_traits::extras::meta_method::has_begin_v<Ty>) {
                        if (!is_const && !value->type().is_const()) {
                            if constexpr (!type_traits::type_properties::is_const_v<remove_ref_t>) {
                                utility::construct_at(recv, std::in_place_type<iterator>, value->template as<Ty>().begin(), value);
                            }
                        }
                    }
                    return !recv->empty();
                }
                break;
            }
            case operation::call_end: {
                if constexpr (type_traits::extras::iterators::has_iterator_v<Ty>) {
                    using tuple_t = std::tuple<bool /* is const */, any * /* value */, typename any::iterator * /* recv */>;
                    using remove_ref_t = type_traits::reference_modify::remove_reference_t<Ty>;
                    using remove_cvref_t = type_traits::cv_modify::remove_cvref_t<Ty>;
                    using const_as = type_traits::cv_modify::add_const_t<remove_ref_t>;
                    using iterator = any_proxy_iterator<any, remove_cvref_t>;
                    auto *res = static_cast<tuple_t *>(data);
                    bool is_const = std::get<0>(*res);
                    any *value = std::get<1>(*res);
                    auto *recv = std::get<2>(*res);
                    if constexpr (type_traits::extras::iterators::has_const_iterator_v<Ty>) {
                        if (is_const || value->type().is_const()) {
                            using const_iterator = const_any_proxy_iterator<any, remove_cvref_t>;
                            if constexpr (type_traits::extras::meta_method::has_cbegin_v<Ty>) {
                                utility::construct_at(recv, std::in_place_type<const_iterator>, value->template as<const_as>().cend(),
                                                      value);
                            } else if constexpr (type_traits::extras::meta_method::has_begin_v<const_as>) {
                                utility::construct_at(recv, std::in_place_type<const_iterator>, value->template as<const_as>().end(),
                                                      value);
                            }
                        }
                    }
                    if constexpr (type_traits::extras::meta_method::has_end_v<Ty>) {
                        if (!is_const && !value->type().is_const()) {
                            if constexpr (!type_traits::type_properties::is_const_v<remove_ref_t>) {
                                utility::construct_at(recv, std::in_place_type<iterator>, value->template as<Ty>().end(), value);
                            }
                        }
                    }
                    return !recv->empty();
                }
                break;
            }
            case operation::assign: {
                using remove_ref_t = type_traits::reference_modify::remove_reference_t<Ty>;
                constexpr bool can_assign = type_traits::extras::meta_method::has_operator_assign_v<Ty> &&
                                            !type_traits::type_properties::is_const_v<remove_ref_t>;
                if constexpr (can_assign) {
                    using tuple_t = std::tuple<typename any::reference *, any>;
                    auto *res = static_cast<tuple_t *>(data);
                    auto *left = std::get<0>(*res);
                    auto &right = std::get<1>(*res);
                    auto &left_operand = *reinterpret_cast<type_traits::cv_modify::remove_cvref_t<Ty> *>(
                        const_cast<void *>(left->target_as_void_ptr()));
                    if (right.template is<Ty>()) {
                        left_operand = right.template as<Ty>();
                    } else if (right.template is_convertible<Ty>()) {
                        left_operand = right.template convert<Ty>();
                    }
                    return true;
                }
                break;
            }
            case operation::get_reference: {
                using add_const = cv_modify::add_const_t<conditional_t<is_reference_v<Ty>,
                                                conditional_t<is_rvalue_reference_v<Ty>, add_rvalue_reference_t<remove_cvref_t>,
                                                              add_lvalue_reference_t<remove_cvref_t>>,
                                                remove_cvref_t>>;
                using tuple_t = std::tuple<bool, any *, typename any::reference *>;
                auto *res = static_cast<tuple_t *>(data);
                bool is_const = std::get<0>(*res);
                auto *value = std::get<1>(*res);
                auto *reference = std::get<2>(*res);
                if (is_const) {
                    utility::construct_at(reference, value->template as<add_const>(), value);
                } else {
                    utility::construct_at(reference, value->template as<Ty>(), value);
                }
                return true;
            }
            case operation::get_lvalue_reference: {
                using add_const = reference_modify::add_lvalue_reference_t<cv_modify::add_const_t<Ty>>;
                using tuple_t = std::tuple<bool, any *, typename any::reference *>;
                auto *res = static_cast<tuple_t *>(data);
                bool is_const = std::get<0>(*res);
                auto *value = std::get<1>(*res);
                auto *reference = std::get<2>(*res);
                if (is_const) {
                    utility::construct_at(reference, value->template as<add_const>(), value);
                } else {
                    utility::construct_at(reference, value->template as<reference_modify::add_lvalue_reference_t<Ty>>(), value);
                }
                return true;
            }
            case operation::get_rvalue_reference: {
                using add_const = reference_modify::add_rvalue_reference_t<cv_modify::add_const_t<Ty>>;
                using tuple_t = std::tuple<bool, any *, typename any::reference *>;
                auto *res = static_cast<tuple_t *>(data);
                bool is_const = std::get<0>(*res);
                auto *value = std::get<1>(*res);
                auto *reference = std::get<2>(*res);
                if (is_const) {
                    utility::construct_at(reference, utility::move(value->template as<add_const>()), value);
                } else {
                    utility::construct_at(reference, utility::move(value->template as<reference_modify::add_rvalue_reference_t<Ty>>()),
                                          value);
                }
                return true;
            }
            case operation::construct_from: {
                using add_const = cv_modify::add_const_t<
                    conditional_t<is_reference_v<Ty>,
                                  conditional_t<is_rvalue_reference_v<Ty>, add_rvalue_reference_t<remove_cvref_t>,
                                                add_lvalue_reference_t<remove_cvref_t>>,
                                  remove_cvref_t>>;
                using tuple_t = std::tuple<bool, typename any::reference*, any*>;
                auto *res = static_cast<tuple_t *>(data);
                bool is_const = std::get<0>(*res);
                auto *left = std::get<1>(*res);
                auto *value = std::get<2>(*res);
                if (is_const) {
                    value->template emplace<remove_cvref_t>(left->template as<add_const>());
                } else {
                    if (left->type().is_rvalue_reference()) {
                        value->template emplace<remove_cvref_t>(utility::move(left->template as<add_rvalue_reference_t<Ty>>()));
                        if (left->this_pointer) {
                            left->this_pointer->reset();
                        }
                    } else {
                        value->template emplace<remove_cvref_t>(left->template as<Ty>());
                    }
                }
                return value->has_value();
            }
            case operation::swap_value: {
                if constexpr (type_traits::type_properties::is_swappable_v<Ty>) {
                    using tuple_t = std::tuple<any *, any *>;
                    auto *res = static_cast<tuple_t *>(data);
                    auto *left = std::get<0>(*res);
                    auto *value = std::get<1>(*res);
                    if (left->type().is_compatible(value->type())) {
                        using std::swap;
                        swap(left->template as<Ty>(), value->template as<Ty>());
                        return true;
                    }
                    return false;
                }
                return false;
            }
        }
        return false;
    }
}

namespace std {
    template <std::size_t Length, std::size_t Align>
    struct hash<::rainy::utility::basic_any<Length, Align>> {
        RAINY_NODISCARD std::size_t operator()(const ::rainy::utility::any &right) const {
            return right.hash_code();
        }
    };
}

namespace rainy::utility {
    template <std::size_t Length, std::size_t Align>
    struct hash<basic_any<Length, Align>> {
        using result_type = std::size_t;
        using argument_type = basic_any<Length, Align>;

        static std::size_t hash_this_val(const argument_type &val) noexcept {
            return val.hash_code();
        }

        RAINY_NODISCARD std::size_t operator()(const argument_type &right) const {
            return right.hash_code();
        }
    };
}

#if RAINY_USING_MSVC
#pragma warning(pop)
#endif

#undef RAINY_ANY_AS_NODISCARD
#undef RAINY_ANY_CAST_TO_POINTER_NODISCARD

#endif
