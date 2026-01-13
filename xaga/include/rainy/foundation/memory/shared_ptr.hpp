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
#ifndef RAINY_FOUNDATION_MEMORY_SHARED_PTR_HPP
#define RAINY_FOUNDATION_MEMORY_SHARED_PTR_HPP
#include <rainy/core/core.hpp>
#include <rainy/foundation/memory/allocator.hpp>
#include <rainy/foundation/memory/nebula_ptr.hpp>
#include <rainy/foundation/typeinfo.hpp>
#include <rainy/utility/pair.hpp>

namespace rainy::foundation::memory {
    template <typename Ty>
    class shared_ptr;

    template <typename T>
    class weak_ptr;
}

namespace rainy::foundation::memory::implements {
    template <typename Ty, typename = void>
    struct can_scalar_del : type_traits::helper::false_type {};
    template <typename Ty>
    struct can_scalar_del<Ty, type_traits::other_trans::void_t<decltype(delete utility::declval<Ty *>())>>
        : std::bool_constant<!type_traits::type_relations::is_void_v<Ty>> {};

    template <typename Ty, typename = void>
    struct can_del_arr : type_traits::helper::false_type {};

    template <typename Ty>
    struct can_del_arr<Ty, type_traits::other_trans::void_t<decltype(delete[] utility::declval<Ty *>())>> : std::true_type {};

    template <typename Fx, typename Arg, typename = void>
    struct is_callable_function_object : std::false_type {};

    template <typename Fx, typename Arg>
    struct is_callable_function_object<Fx, Arg, std::void_t<decltype(utility::declval<Fx>()(utility::declval<Arg>()))>>
        : type_traits::helper::true_type {};

    template <typename Ty, typename UTy>
    struct shared_convertible : type_traits::type_relations::is_convertible<Ty *, UTy *>::type {};

    template <typename Ty, typename UTy>
    struct shared_convertible<Ty, UTy[]> : type_traits::type_relations::is_convertible<Ty (*)[], UTy (*)[]>::type {};

    template <typename Ty, typename UTy, size_t Ext>
    struct shared_convertible<Ty, UTy[Ext]> : type_traits::type_relations::is_convertible<Ty (*)[Ext], UTy (*)[Ext]>::type {};

    template <typename Ty, typename UTy>
    struct shared_pointer_compatible : type_traits::type_relations::is_convertible<Ty *, UTy *>::type {};

    template <typename UTy, size_t Ext>
    struct shared_pointer_compatible<UTy[Ext], UTy[]> : std::true_type {};

    template <typename UTy, size_t Ext>
    struct shared_pointer_compatible<UTy[Ext], const UTy[]> : std::true_type {};

    template <typename UTy, size_t Ext>
    struct shared_pointer_compatible<UTy[Ext], volatile UTy[]> : std::true_type {};

    template <typename UTy, size_t Ext>
    struct shared_pointer_compatible<UTy[Ext], const volatile UTy[]> : std::true_type {};
}

namespace rainy::foundation::memory::implements {
    class ref_count_base {
    public:
        ref_count_base(const ref_count_base &) = delete;
        ref_count_base &operator=(const ref_count_base &) = delete;

        virtual ~ref_count_base() noexcept = default;

        bool incref_non_zero() noexcept {
            rainy_ref volatile_uses = reinterpret_cast<volatile long &>(uses);
            long count = core::pal::iso_volatile_load32(reinterpret_cast<volatile int *>(&volatile_uses));
            while (count != 0) {
                const long old_value = core::pal::interlocked_compare_exchange32(
                    reinterpret_cast<volatile std::int32_t *>(&volatile_uses), count + 1, count); // NOLINT
                if (old_value == count) {
                    return true;
                }
                count = old_value;
            }
            return false;
        }

        void inc_ref() noexcept {
            core::pal::interlocked_increment32(reinterpret_cast<volatile std::int32_t *>(&uses));
        }

        void inc_weak_ref() noexcept {
            core::pal::interlocked_increment32(reinterpret_cast<volatile std::int32_t *>(&weaks));
        }

        void dref() noexcept {
            if (core::pal::interlocked_decrement32(reinterpret_cast<volatile std::int32_t *>(&uses)) == 0) {
                destory();
                deweakref();
            }
        }

        void deweakref() noexcept {
            if (core::pal::interlocked_decrement32(reinterpret_cast<volatile std::int32_t *>(&weaks)) == 0) {
                delete_this();
            }
        }

        RAINY_NODISCARD long use_count() const noexcept {
            return uses;
        }

        RAINY_NODISCARD virtual void *get_deleter(const ctti::typeinfo &type) const noexcept {
            return nullptr;
        }

    private:
        virtual void destory() noexcept = 0;
        virtual void delete_this() noexcept = 0;
        std::uint32_t uses = 1;
        std::uint32_t weaks = 1;

    protected:
        constexpr ref_count_base() noexcept = default;
    };

    template <typename Ty>
    class ref_count : public ref_count_base {
    public:
        explicit ref_count(Ty *px) : ref_count_base(), ptr(px) {
        }

    private:
        void destory() noexcept override {
            delete ptr;
        }

        void delete_this() noexcept override {
            delete this;
        }

        Ty *ptr;
    };

    template <typename Res, typename Dx>
    class ref_count_resource : public ref_count_base {
    public:
        ref_count_resource(Res px, Dx deleter) : ref_count_base(), pair(utility::move(deleter), px) {
        }

        ~ref_count_resource() noexcept override = default;

        RAINY_NODISCARD void *get_deleter(const ctti::typeinfo &type) const noexcept override {
            if (type == rainy_typeid(Dx)) {
                return const_cast<Dx *>(utility::addressof(pair.second));
            }
            return nullptr;
        }

    private:
        void destory() noexcept override {
            pair.second(pair.second);
        }

        void delete_this() noexcept override {
            delete this;
        }

        utility::pair<Dx, Res> pair;
    };

    template <typename Res, typename Dx, typename Alloc>
    class ref_count_resource_alloc : public ref_count_base {
    public:
        ref_count_resource_alloc(Res px, Dx deleter, const Alloc &alloc) :
            ref_count_base({}), pair(utility::move(deleter), {alloc, px}) {
        }

        ~ref_count_resource_alloc() noexcept override = default;

        RAINY_NODISCARD void *get_deleter(const ctti::typeinfo &type) const noexcept override {
            if (type == rainy_typeid(Dx)) {
                return const_cast<Dx *>(utility::addressof(pair.second));
            }
            return nullptr;
        }

    private:
        using allocator_type = typename foundation::memory::allocator_traits<Alloc>::template rebind_alloc<ref_count_resource_alloc>;

        void destory() noexcept override {
            pair.second(pair.second.second);
        }

        void delete_this() noexcept override {
            allocator_type alloc = pair.second.first;
            delete this;
        }

        utility::pair<Dx, utility::pair<allocator_type, Res>> pair;
    };
}

namespace rainy::foundation::memory::implements {
    template <typename Ty>
    class shared_ptr_base {
    public:
        using element_type = std::remove_extent_t<Ty>;

        RAINY_NODISCARD long use_count() const noexcept {
            return pair.second ? pair.second->use_count() : 0;
        }

        template <typename UTy>
        RAINY_NODISCARD bool owner_before(const shared_ptr_base<UTy> &right) const noexcept { // compare addresses of manager objects
            return pair.second < right.pair.second;
        }

        shared_ptr_base(const shared_ptr_base &) = delete;
        shared_ptr_base &operator=(const shared_ptr_base &) = delete;

    protected:
        constexpr shared_ptr_base() noexcept = default;

        ~shared_ptr_base() = default;

        template <typename UTy>
        void move_construct_from(shared_ptr_base<UTy> &&right) noexcept {
            pair.first = right.pair.first;
            pair.second = right.pair.second;
            right.pair.first = nullptr;
            right.pair.second = nullptr;
        }

        template <typename UTy>
        void copy_construct_from(const shared_ptr<UTy> &right) noexcept {
            right.inc_ref();
            pair.first = right.pair.first;
            pair.second = right.pair.second;
        }

        template <typename UTy>
        void alias_construct_from(const shared_ptr<UTy> &right, element_type *px) noexcept {
            right.inc_ref();
            pair.first = px;
            pair.second = right.pair.second;
        }

        template <typename UTy>
        void alias_move_constructFrom(shared_ptr<UTy> &&right, element_type *px) noexcept {
            pair.first = px;
            pair.second = right.pair.second;
            right.pair.first = nullptr;
            right.pair.second = nullptr;
        }

        template <typename UTy>
        friend class memory::weak_ptr;

        template <typename UTy>
        bool construct_from_weak(const weak_ptr<UTy> &right) noexcept {
            if (right.pair.second && right.pair.second->incref_non_zero()) {
                pair.first = right.pair.first;
                pair.second = right.pair.second;
                return true;
            }
            return false;
        }

        void inc_ref() const noexcept {
            if (pair.second) {
                pair.second->inc_ref();
            }
        }

        void dref() noexcept { // decrement reference count
            if (pair.second) {
                pair.second->dref();
            }
        }

        void swap_(shared_ptr_base &right) noexcept {
            std::swap(pair.first, right.pair.first);
            std::swap(pair.second, right.pair.second);
        }

        template <typename UTy>
        void weakly_construct_from(const shared_ptr_base<UTy> &right) noexcept {
            if (right.pair.second) {
                pair.first = right.pair.first;
                pair.second = right.pair.second;
                pair.second->inc_weak_ref();
            } else {
                assert(!pair.first && !pair.second);
            }
        }

        template <typename UTy>
        void weakly_convert_lvalue_avoiding_expired_conversions(const shared_ptr_base<UTy> &right) noexcept {
            if (right.pair.second) {
                pair.second = right.pair.second;
                pair.second->inc_weak_ref();
                if (pair.second->incref_non_zero()) {
                    pair.second = right.pair.second;
                    pair.second->dref();
                } else {
                    assert(!pair.first);
                }
            } else {
                assert(!pair.first && !pair.second);
            }
        }

        template <typename UTy>
        void weakly_convert_rvalue_avoiding_expired_conversions(shared_ptr_base<UTy> &&right) noexcept {
            pair.second = right.second;
            right.second = nullptr;
            if (pair.second && pair.second->incRef_nz()) {
                pair.second = right.pair.second;
                pair.second->dref();
            } else {
                assert(!pair.first);
            }
            right.pair.first = nullptr;
        }

        void inc_weak_ref() const noexcept {
            if (pair.second) {
                pair.second->inc_weak_ref();
            }
        }

        void deweakref() noexcept {
            if (pair.second) {
                pair.second->deweakref();
            }
        }

    private:
        template <typename UTy>
        friend class shared_ptr_base;

        friend class memory::shared_ptr<Ty>;

        template <typename UTy>
        friend struct atomic;

        template <typename Dx, typename UTy>
        friend Dx *get_deleter(const shared_ptr<UTy> &sx) noexcept;

        utility::pair<element_type *, ref_count_base *> pair;
    };
}

namespace rainy::foundation::memory::implements {
    template <typename UTy>
    struct temporary_owner {
        explicit temporary_owner(UTy *const ptr) noexcept : ptr(ptr) {
        }

        temporary_owner(const temporary_owner &) = delete;

        temporary_owner &operator=(const temporary_owner &) = delete;

        ~temporary_owner() {
            delete ptr;
        }

        UTy *ptr;
    };

    template <typename UptrOrNullptr, typename Dx>
    struct temporary_owner_del {
        explicit temporary_owner_del(const UptrOrNullptr ptr, Dx &deleter_) noexcept : ptr(ptr), deleter(deleter_) {
        }

        temporary_owner_del(const temporary_owner_del &) = delete;
        temporary_owner_del &operator=(const temporary_owner_del &) = delete;
        ~temporary_owner_del() {
            if (need_release) {
                deleter(ptr);
            }
        }

        UptrOrNullptr ptr;
        Dx &deleter;
        bool need_release{true};
    };

    template <typename Ty, typename = void>
    struct can_enable_shared : type_traits::helper::false_type {};

    template <typename Ty>
    struct can_enable_shared<Ty, type_traits::other_trans::void_t<typename Ty::rts_enable_shared_type>>
        : type_traits::type_relations::is_convertible<type_traits::cv_modify::remove_cv_t<Ty> *,
                                                      typename Ty::rts_enable_shared_type *>::type {};
}

namespace rainy::foundation::memory {
    template <typename Ty>
    class shared_ptr : public implements::shared_ptr_base<Ty> {
    public:
        using element_type = implements::shared_ptr_base<Ty>::element_type;
        using pointer = element_type *;

        constexpr shared_ptr() = default;

        constexpr shared_ptr(std::nullptr_t) {}; // NOLINT

        template <typename UTy>
            requires std::conjunction_v<
                std::conditional_t<std::is_array_v<Ty>, implements::can_del_arr<UTy>, implements::can_scalar_del<UTy>>,
                implements::shared_convertible<UTy, Ty>>
        shared_ptr(UTy *ptr) { // NOLINT
            if constexpr (std::is_array_v<Ty>) {
                set_ptr_and_deleter(ptr, default_deleter<Ty[]>{});
            } else {
                implements::temporary_owner<Ty> owner(ptr);
                set_ptr_rep_and_enable_shared(owner.ptr, new implements::ref_count<Ty>(owner.ptr));
                owner.ptr = nullptr;
            }
        }

        template <typename UTy, typename Dx>
            requires std::conjunction_v<std::is_move_constructible<Dx>, implements::is_callable_function_object<Dx &, UTy *&>,
                                        implements::shared_convertible<UTy, Ty>>
        shared_ptr(UTy *ptr, Dx deleter) {
            set_ptr_and_deleter(ptr, utility::move(deleter));
        }

        template <typename UTy, typename Dx, typename Alloc>
            requires std::conjunction_v<std::is_move_constructible<Dx>, implements::is_callable_function_object<Dx &, UTy *&>,
                                        implements::shared_convertible<UTy, Ty>>
        shared_ptr(UTy *ptr, Dx deleter, Alloc allocator) {
            set_ptr_and_deleter_and_also_alloc(ptr, utility::move(deleter), allocator);
        }

        template <typename Dx>
            requires std::conjunction_v<std::is_move_constructible<Dx>,
                                        implements::is_callable_function_object<Dx &, std::nullptr_t &>,
                                        implements::shared_convertible<std::nullptr_t, Ty>>
        shared_ptr(std::nullptr_t, Dx deleter) {
            set_ptr_and_deleter(nullptr, deleter);
        }

        template <typename Dx, typename Alloc>
            requires std::conjunction_v<std::is_move_constructible<Dx>,
                                        implements::is_callable_function_object<Dx &, std::nullptr_t &>,
                                        implements::shared_convertible<std::nullptr_t, Ty>>
        shared_ptr(std::nullptr_t, Dx deleter, Alloc allocator) {
            set_ptr_and_deleter_and_also_alloc(nullptr, deleter, allocator);
        }

        template <typename UTy>
        shared_ptr(const shared_ptr<UTy> &right, element_type *px) noexcept {
            this->alias_construct_from(right, px);
        }

        template <typename UTy>
        shared_ptr(shared_ptr<UTy> &&right, element_type *px) noexcept {
            this->alias_move_constructFrom(utility::move(right), px);
        }

        shared_ptr(const shared_ptr &right) noexcept {
            this->copy_construct_from(right);
        }

        template <typename UTy, type_traits::other_trans::enable_if_t<implements::shared_pointer_compatible<UTy, Ty>::value, int> = 0>
        shared_ptr(const shared_ptr<UTy> &right) noexcept { // NOLINT
            this->copy_construct_from(right);
        }

        shared_ptr(shared_ptr &&right) noexcept {
            this->move_construct_from(utility::move(right));
        }

        template <typename UTy, type_traits::other_trans::enable_if_t<implements::shared_pointer_compatible<UTy, Ty>::value, int> = 0>
        shared_ptr(shared_ptr<UTy> &&right) noexcept { // NOLINT
            this->move_construct_from(utility::move(right));
        }

        ~shared_ptr() noexcept {
            this->dref();
        }

        shared_ptr &operator=(const shared_ptr &right) noexcept {
            shared_ptr(right).swap(*this);
            return *this;
        }

        template <typename UTy>
            requires implements::shared_pointer_compatible<UTy, Ty>::value
        shared_ptr &operator=(const shared_ptr<UTy> &right) noexcept {
            shared_ptr(right).swap(*this);
            return *this;
        }

        shared_ptr &operator=(shared_ptr &&right) noexcept {
            shared_ptr(utility::move(right)).swap(*this);
            return *this;
        }

        template <typename UTy>
            requires implements::shared_pointer_compatible<UTy, Ty>::value
        shared_ptr &operator=(shared_ptr<UTy> &&right) noexcept {
            shared_ptr(utility::move(right)).swap(*this);
            return *this;
        }

        template <typename UTy, typename Dx>
            requires std::conjunction_v<implements::shared_pointer_compatible<UTy, Ty>,
                                        std::is_convertible<typename nebula_ptr<UTy, Dx>::pointer, element_type *>>
        shared_ptr &operator=(nebula_ptr<UTy, Dx> &&right) {
            shared_ptr(utility::move(right)).swap(*this);
            return *this;
        }

        template <typename UTy>
            requires std::conjunction_v<
                std::conditional_t<std::is_array_v<Ty>, implements::can_del_arr<UTy>, implements::can_scalar_del<UTy>>,
                implements::shared_convertible<UTy, Ty>>
        shared_ptr &operator=(UTy *ptr) {
            shared_ptr(ptr).swap(*this);
            return *this;
        }

        void swap(shared_ptr &right) noexcept {
            this->swap_(right);
        }

        void reset() noexcept {
            shared_ptr{}.swap(*this);
        }

        template <typename UTy>
            requires std::conjunction_v<
                std::conditional_t<std::is_array_v<Ty>, implements::can_del_arr<UTy>, implements::can_scalar_del<UTy>>,
                implements::shared_convertible<UTy, Ty>>
        void reset(UTy *ptr) {
            shared_ptr(ptr).swap(*this);
        }

        template <typename UTy, typename Dx>
            requires std::conjunction_v<std::is_move_constructible<Dx>, implements::is_callable_function_object<Dx &, UTy *&>,
                                        implements::shared_convertible<UTy, Ty>>
        void reset(UTy *ptr, Dx deleter) {
            shared_ptr(ptr, deleter).swap(*this);
        }

        template <typename UTy, typename Dx, typename Alloc>
            requires std::conjunction_v<std::is_move_constructible<Dx>, implements::is_callable_function_object<Dx &, UTy *&>,
                                        implements::shared_convertible<UTy, Ty>>
        void reset(UTy *ptr, Dx deleter, Alloc allocator) {
            shared_ptr(ptr, deleter, allocator).swap(*this);
        }

        inline element_type &operator*() const noexcept {
            return *get();
        }

        inline pointer operator->() const noexcept {
            return get();
        }

        RAINY_NODISCARD pointer get() const noexcept {
            return this->pair.first;
        }

        inline explicit operator bool() const noexcept {
            return static_cast<bool>(this->pair.first);
        }

    private:
        template <typename UptrOrNullptr, typename Dx>
        void set_ptr_and_deleter(const UptrOrNullptr px, Dx deleter) {
            implements::temporary_owner_del<UptrOrNullptr, Dx> owner(px, deleter);
            set_ptr_rep_and_enable_shared(
                owner.pair.first, new implements::ref_count_resource<UptrOrNullptr, Dx>(owner.pair.first, utility::move(deleter)));
            owner.need_release = false;
        }

        template <typename UptrOrNullptr, typename Dx, typename Alloc>
        void set_ptr_and_deleter_and_also_alloc(const UptrOrNullptr px, Dx deleter, Alloc ax) {
            using ref_count_res_alloc = typename allocator_traits<Alloc>::template rebind_alloc<
                implements::ref_count_resource_alloc<UptrOrNullptr, Dx, Alloc>>;
            implements::temporary_owner_del<UptrOrNullptr, Dx> owner(px, deleter);
            ref_count_res_alloc alloc(ax);
            implements::alloc_construct_ptr<ref_count_res_alloc> constructor(alloc);
            constructor.allocate();
            utility::construct_in_place(*constructor.pair.first, owner.ptr, utility::move(deleter), ax);
            set_ptr_rep_and_enable_shared(owner.pair.first, constructor.ptr);
            constructor.release();
            owner.need_release = false;
        }

        template <typename UTy>
        void set_ptr_rep_and_enable_shared(UTy *const px, implements::ref_count_base *const rx) noexcept {
            this->pair.first = px;
            this->pair.second = rx;
            if constexpr (!std::is_array_v<Ty> && !std::is_volatile_v<UTy> && implements::can_enable_shared<UTy>::value) {
                if (px && px->wptr.expired()) {
                    px->wptr = shared_ptr<std::remove_cv_t<UTy>>(*this, const_cast<std::remove_cv_t<UTy> *>(px));
                }
            }
        }

        void set_ptr_rep_and_enable_shared(nullptr_t, implements::ref_count_base *const rx) noexcept {
            this->pair.first = nullptr;
            this->pair.second = rx;
        }
    };

    template <typename T>
    shared_ptr(weak_ptr<T>) -> shared_ptr<T>;
    template <typename T, typename D>
    shared_ptr(nebula_ptr<T, D>) -> shared_ptr<T>;
}

namespace rainy::foundation::memory {
    template <typename Ty>
    class weak_ptr : public implements::shared_ptr_base<Ty> {
    public:
        template <typename UTy, typename = const UTy *>
        static constexpr bool must_avoid_expired_conversions_from = true;

        template <typename UTy>
        static constexpr bool
            must_avoid_expired_conversions_from<UTy, decltype(static_cast<const UTy *>(static_cast<Ty *>(nullptr)))> = false;

        constexpr weak_ptr() noexcept = default;

        weak_ptr(const weak_ptr &other) noexcept {
            this->weakly_construct_from(other);
        }

        template <typename UTy>
            requires implements::shared_pointer_compatible<UTy, Ty>::value
        weak_ptr(const shared_ptr<UTy> &other) noexcept {
            this->weakly_construct_from(other);
        }

        template <typename UTy, type_traits::other_trans::enable_if_t<implements::shared_pointer_compatible<UTy, Ty>::value, int> = 0>
        weak_ptr(const weak_ptr<UTy> &other) noexcept {
            constexpr bool avoid_expired_conversions = must_avoid_expired_conversions_from<UTy>;
            if constexpr (avoid_expired_conversions) {
                this->weakly_convert_lvalue_avoiding_expired_conversions(other);
            } else {
                this->weakly_construct_from(other);
            }
        }

        weak_ptr(weak_ptr &&other) noexcept {
            this->move_construct_from(utility::move(other));
        }

        template <typename UTy, type_traits::other_trans::enable_if_t<implements::shared_pointer_compatible<UTy, Ty>::value, int> = 0>
        explicit weak_ptr(weak_ptr<UTy> &&other) noexcept {
            if constexpr (constexpr bool avoid_expired_conversions = must_avoid_expired_conversions_from<UTy>; avoid_expired_conversions) {
                this->weakly_convert_rvalue_avoiding_expired_conversions(utility::move(other));
            } else {
                this->move_construct_from(utility::move(other));
            }
        }

        template <typename UTy, type_traits::other_trans::enable_if_t<
                                    type_traits::other_trans::conditional_t<std::is_array_v<Ty>, implements::can_del_arr<UTy>,
                                                                            implements::can_scalar_del<UTy>>::value &&
                                        implements::shared_convertible<UTy, Ty>::value,
                                    int> = 0>
        weak_ptr(UTy *ptr) { // NOLINT
            this->construct_from_weak(ptr);
        }

        ~weak_ptr() noexcept {
            this->deweakref();
        }

        weak_ptr &operator=(const weak_ptr &right) noexcept {
            weak_ptr(right).swap(*this);
            return *this;
        }

        template <typename UTy, type_traits::other_trans::enable_if_t<implements::shared_pointer_compatible<UTy, Ty>::value, int> = 0>
        weak_ptr &operator=(const weak_ptr<UTy> &right) noexcept {
            weak_ptr(right).swap(*this);
            return *this;
        }

        weak_ptr &operator=(weak_ptr &&right) noexcept {
            weak_ptr(utility::move(right)).swap(*this);
            return *this;
        }

        template <typename UTy, type_traits::other_trans::enable_if_t<implements::shared_pointer_compatible<UTy, Ty>::value, int> = 0>
        weak_ptr &operator=(weak_ptr<UTy> &&right) noexcept {
            weak_ptr(utility::move(right)).swap(*this);
            return *this;
        }

        template <typename UTy, type_traits::other_trans::enable_if_t<implements::shared_pointer_compatible<UTy, Ty>::value, int> = 0>
        weak_ptr &operator=(const shared_ptr<UTy> &right) noexcept {
            weak_ptr(right).swap(*this);
            return *this;
        }

        void reset() noexcept {
            weak_ptr{}.swap(*this);
        }

        void swap(weak_ptr &other) noexcept {
            this->swap_(other);
        }

        template <typename UTy,
                  type_traits::other_trans::enable_if_t<
                      std::conditional_t<std::is_array_v<Ty>, implements::can_del_arr<UTy>, implements::can_scalar_del<UTy>>::value &&
                          implements::shared_convertible<UTy, Ty>::value,
                      int> = 0>
        void reset(UTy *ptr) {
            shared_ptr(ptr).swap(*this);
        }

        RAINY_NODISCARD bool expired() const noexcept {
            return this->use_count() == 0;
        }

        RAINY_NODISCARD shared_ptr<Ty> lock() const noexcept {
            shared_ptr<Ty> ret;
            ret.construct_from_weak(*this);
            return ret;
        }
    };
}

namespace rainy::foundation::memory {
    template <typename Ty>
    class enable_shared_from_this { // provide member functions that create shared_ptr to this
    public:
        using rts_enable_shared_type = enable_shared_from_this;

        RAINY_NODISCARD shared_ptr<Ty> shared_from_this() {
            return shared_ptr<Ty>(wptr);
        }

        RAINY_NODISCARD shared_ptr<const Ty> shared_from_this() const {
            return shared_ptr<const Ty>(wptr);
        }

        RAINY_NODISCARD weak_ptr<Ty> weak_from_this() noexcept {
            return wptr;
        }

        RAINY_NODISCARD weak_ptr<const Ty> weak_from_this() const noexcept {
            return wptr;
        }

    protected:
        constexpr enable_shared_from_this() noexcept : wptr() {
        }

        enable_shared_from_this(const enable_shared_from_this &) noexcept : wptr() {
        }

        enable_shared_from_this &operator=(const enable_shared_from_this &) noexcept {
            return *this;
        }

        ~enable_shared_from_this() = default;

    private:
        template <typename UTy>
        friend class shared_ptr;

        mutable weak_ptr<Ty> wptr;
    };
}

#endif
