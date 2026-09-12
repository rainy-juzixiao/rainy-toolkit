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
#include <rainy/core/layer.hpp>
#include <rainy/core/memory/allocator.hpp>
#include <rainy/core/memory/nebula_ptr.hpp>
#include <rainy/core/container/pair.hpp>
#include <rainy/core/typeinfo.hpp>

namespace rainy::core::memory {
    template <typename Ty>
    class shared_ptr;

    template <typename Ty>
    class weak_ptr;
}

#if !RAINY_HAS_MUZIYAN_REACH_FOR_THE_MOON
namespace rainy::core::memory::implements {
    template <typename Ty, typename = void>
    struct can_scalar_del : type_traits::helper::false_type {};
    template <typename Ty>
    struct can_scalar_del<Ty, type_traits::other_trans::void_t<decltype(delete utility::declval<Ty *>())>>
        : type_traits::helper::bool_constant<!type_traits::type_relations::is_void_v<Ty>> {};

    template <typename Ty, typename = void>
    struct can_del_arr : type_traits::helper::false_type {};

    template <typename Ty>
    struct can_del_arr<Ty, type_traits::other_trans::void_t<decltype(delete[] utility::declval<Ty *>())>> : type_traits::helper::true_type {};

    template <typename Fx, typename Arg, typename = void>
    struct is_callable_function_object : type_traits::helper::false_type {};

    template <typename Fx, typename Arg>
    struct is_callable_function_object<Fx, Arg, type_traits::other_trans::void_t<decltype(utility::declval<Fx>()(utility::declval<Arg>()))>>
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
    struct shared_pointer_compatible<UTy[Ext], UTy[]> : type_traits::helper::true_type {};

    template <typename UTy, size_t Ext>
    struct shared_pointer_compatible<UTy[Ext], const UTy[]> : type_traits::helper::true_type {};

    template <typename UTy, size_t Ext>
    struct shared_pointer_compatible<UTy[Ext], volatile UTy[]> : type_traits::helper::true_type {};

    template <typename UTy, size_t Ext>
    struct shared_pointer_compatible<UTy[Ext], const volatile UTy[]> : type_traits::helper::true_type {};
}
#endif

#if !RAINY_HAS_MUZIYAN_REACH_FOR_THE_MOON
namespace rainy::core::memory::implements {
    class ref_count_base {
    public:
        ref_count_base(const ref_count_base &) = delete;
        ref_count_base &operator=(const ref_count_base &) = delete;

        virtual ~ref_count_base() noexcept = default;

        bool incref_non_zero() noexcept {
            rainy_ref volatile_uses = reinterpret_cast<volatile long &>(uses);
            long count = layer::iso_volatile_load32(reinterpret_cast<volatile int *>(&volatile_uses));
            while (count != 0) {
                const long old_value = layer::interlocked_compare_exchange32(
                    reinterpret_cast<volatile std::int32_t *>(&volatile_uses), count + 1, count); // NOLINT
                if (old_value == count) {
                    return true;
                }
                count = old_value;
            }
            return false;
        }

        void inc_ref() noexcept {
            layer::interlocked_increment32(reinterpret_cast<volatile std::int32_t *>(&uses));
        }

        void inc_weak_ref() noexcept {
            layer::interlocked_increment32(reinterpret_cast<volatile std::int32_t *>(&weaks));
        }

        void dref() noexcept {
            if (layer::interlocked_decrement32(reinterpret_cast<volatile std::int32_t *>(&uses)) == 0) {
                destory();
                deweakref();
            }
        }

        void deweakref() noexcept {
            if (layer::interlocked_decrement32(reinterpret_cast<volatile std::int32_t *>(&weaks)) == 0) {
                delete_this();
            }
        }

        RAINY_NODISCARD long use_count() const noexcept {
            return uses;
        }

        RAINY_NODISCARD virtual void *get_deleter(const typeinfo &type) const noexcept {
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

        RAINY_NODISCARD void *get_deleter(const typeinfo &type) const noexcept override {
            if (type == rainy_typeid(Dx)) {
                return const_cast<Dx *>(utility::addressof(pair.first));
            }
            return nullptr;
        }

    private:
        void destory() noexcept override {
            pair.first(pair.second);
        }

        void delete_this() noexcept override {
            delete this;
        }

        container::pair<Dx, Res> pair;
    };

    template <typename Res, typename Dx, typename Alloc>
    class ref_count_resource_alloc : public ref_count_base {
    public:
        ref_count_resource_alloc(Res px, Dx deleter, const Alloc &alloc) :
            ref_count_base(), pair(utility::move(deleter), {alloc, px}) { // call default construct
        }

        ~ref_count_resource_alloc() noexcept override = default;

        RAINY_NODISCARD void *get_deleter(const typeinfo &type) const noexcept override {
            if (type == rainy_typeid(Dx)) {
                return const_cast<Dx *>(utility::addressof(pair.first));
            }
            return nullptr;
        }

    private:
        using allocator_type = typename core::memory::allocator_traits<Alloc>::template rebind_alloc<ref_count_resource_alloc>;

        void destory() noexcept override {
            pair.first(pair.second.second);
        }

        void delete_this() noexcept override {
            allocator_type alloc = pair.second.first;
            this->~ref_count_resource_alloc();
            allocator_traits<allocator_type>::deallocate(alloc, this, 1);
        }

        container::pair<Dx, container::pair<allocator_type, Res>> pair;
    };
}
#endif

#if !RAINY_HAS_MUZIYAN_REACH_FOR_THE_MOON
namespace rainy::core::memory::implements {
    template <typename Ty>
    class shared_ptr_base {
    public:
        using element_type = type_traits::modifers::remove_extent_t<Ty>;

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
                pair.first = right.pair.first;
                pair.second = right.pair.second;
                pair.second->inc_weak_ref();
                if (!pair.second->incref_non_zero()) {
                    pair.first = nullptr;
                } else {
                    pair.second->dref();
                }
            } else {
                assert(!pair.first && !pair.second);
            }
        }

        template <typename UTy>
        void weakly_convert_rvalue_avoiding_expired_conversions(shared_ptr_base<UTy> &&right) noexcept {
            pair.first = right.pair.first;
            pair.second = right.pair.second;
            right.pair.first = nullptr;
            right.pair.second = nullptr;
            if (pair.second && pair.second->incref_non_zero()) {
                pair.second->dref();
            } else {
                pair.first = nullptr;
            }
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

    protected:
        template <typename UTy>
        friend class shared_ptr_base;

        friend class memory::shared_ptr<Ty>;

        template <typename UTy>
        friend struct atomic;

        template <typename Dx, typename UTy>
        friend Dx *get_deleter(const shared_ptr<UTy> &sx) noexcept;

        container::pair<element_type *, ref_count_base *> pair;
    };
}
#endif

#if !RAINY_HAS_MUZIYAN_REACH_FOR_THE_MOON
namespace rainy::core::memory::implements {
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

    template <typename Ty>
    class ref_count_allocated : public ref_count_base {
    public:
        template <typename... Args>
        explicit ref_count_allocated(Args &&...args) {
            ::new (static_cast<void *>(&storage)) Ty(utility::forward<Args>(args)...);
        }

        Ty *get_ptr() noexcept {
            return reinterpret_cast<Ty *>(&storage);
        }

    private:
        void destory() noexcept override {
            get_ptr()->~Ty();
        }

        void delete_this() noexcept override {
            delete this;
        }

        alignas(Ty) core::byte_t storage[sizeof(Ty)]{};
    };

    template <typename Ty>
    class ref_count_allocated_array : public ref_count_base {
    public:
        explicit ref_count_allocated_array(const std::size_t count) : count(count) {
            // NOLINTBEGIN
            try {
                ptr = new Ty[count]();
            } catch (...) {
                delete this;
                throw;
            }
            // NOLINTEND
        }

        ref_count_allocated_array(const std::size_t count, const Ty &u) : count(count) {
            try {
                ptr = new Ty[count];
                for (std::size_t i = 0; i < count; ++i) {
                    ptr[i] = u;
                }
            } catch (...) {
                delete[] ptr;
                delete this;
                throw;
            }
        }

        Ty *get_ptr() noexcept {
            return ptr;
        }

    private:
        void destory() noexcept override {
            delete[] ptr;
        }

        void delete_this() noexcept override {
            delete this;
        }

        Ty *ptr{nullptr};
        std::size_t count;
    };

    template <typename Ty, typename = void>
    struct can_enable_shared : type_traits::helper::false_type {};

    template <typename Ty>
    struct can_enable_shared<Ty, type_traits::other_trans::void_t<typename Ty::rts_enable_shared_type>>
        : type_traits::helper::bool_constant<type_traits::type_relations::is_convertible_v<type_traits::modifers::remove_cv_t<Ty> *, typename Ty::rts_enable_shared_type *>> {};
}
#endif

namespace rainy::core::memory {
    /**
     * \lang english
     * @brief A shared ownership smart pointer.
     *
     * shared_ptr is a smart pointer that retains shared ownership of an object
     * through a pointer. Multiple shared_ptr objects may own the same object,
     * and the object is destroyed when the last remaining shared_ptr owning it
     * is destroyed.
     *
     * @tparam Ty The type of the managed object
     *
     * \lang simp-chinese
     * @brief 共享所有权智能指针
     *
     * shared_ptr 是通过指针保留对象共享所有权的智能指针。多个 shared_ptr 对象
     * 可以拥有同一对象，当最后一个拥有该对象的 shared_ptr 被销毁时，对象才会被销毁。
     *
     * @tparam Ty 被管理对象的类型
     */
    template <typename Ty>
    class shared_ptr : public implements::shared_ptr_base<Ty> {
    public:
        using element_type = typename implements::shared_ptr_base<Ty>::element_type;
        using pointer = element_type *;

        /**
         * \lang english
         * @brief Constructs a shared_ptr with no managed object.
         *
         * \lang simp-chinese
         * @brief 构造一个不管理任何对象的 shared_ptr
         */
        constexpr shared_ptr() = default;

        /**
         * \lang english
         * @brief Constructs a shared_ptr with no managed object (from nullptr).
         *
         * \lang simp-chinese
         * @brief 由 nullptr 构造一个不管理任何对象的 shared_ptr
         */
        constexpr shared_ptr(std::nullptr_t) {}; // NOLINT

        /**
         * \lang english
         * @brief Constructs a shared_ptr that owns the object pointed to by ptr.
         * @tparam UTy The type of the pointer, convertible to Ty
         * @param ptr The pointer to the object to manage
         *
         * \lang simp-chinese
         * @brief 构造一个拥有 ptr 所指向对象的 shared_ptr
         * @tparam UTy 指针类型，可转换为 Ty
         * @param ptr 指向要管理对象的指针
         */
        template <typename UTy,
                  type_traits::other_trans::enable_if_t<
                      type_traits::logical_traits::conjunction_v<
                          type_traits::other_trans::conditional_t<type_traits::primary_types::is_array_v<Ty>,
                                                                  implements::can_del_arr<UTy>, implements::can_scalar_del<UTy>>,
                          implements::shared_convertible<UTy, Ty>>,
                      int> = 0>
        shared_ptr(UTy *ptr) { // NOLINT
            if constexpr (type_traits::primary_types::is_array_v<Ty>) {
                set_ptr_and_deleter(ptr, default_deleter<Ty[]>{});
            } else {
                implements::temporary_owner<Ty> owner(ptr);
                set_ptr_rep_and_enable_shared(owner.ptr, new implements::ref_count<Ty>(owner.ptr));
                owner.ptr = nullptr;
            }
        }

        /**
         * \lang english
         * @brief Constructs a shared_ptr that owns the object pointed to by ptr with a custom deleter.
         * @tparam UTy The type of the pointer, convertible to Ty
         * @tparam Dx The deleter type
         * @param ptr The pointer to the object to manage
         * @param deleter The deleter used to destroy the object
         *
         * \lang simp-chinese
         * @brief 使用自定义删除器构造一个拥有 ptr 所指向对象的 shared_ptr
         * @tparam UTy 指针类型，可转换为 Ty
         * @tparam Dx 删除器类型
         * @param ptr 指向要管理对象的指针
         * @param deleter 用于销毁对象的删除器
         */
        template <typename UTy, typename Dx,
                  type_traits::other_trans::enable_if_t<
                      type_traits::logical_traits::conjunction_v<type_traits::properties::is_move_constructible<Dx>,
                                                                 implements::is_callable_function_object<Dx &, UTy *&>,
                                                                 implements::shared_convertible<UTy, Ty>>,
                      int> = 0>
        shared_ptr(UTy *ptr, Dx deleter) {
            set_ptr_and_deleter(ptr, utility::move(deleter));
        }

        /**
         * \lang english
         * @brief Constructs a shared_ptr that owns the object pointed to by ptr with a custom deleter and allocator.
         * @tparam UTy The type of the pointer, convertible to Ty
         * @tparam Dx The deleter type
         * @tparam Alloc The allocator type
         * @param ptr The pointer to the object to manage
         * @param deleter The deleter used to destroy the object
         * @param allocator The allocator used to allocate the control block
         *
         * \lang simp-chinese
         * @brief 使用自定义删除器和分配器构造一个拥有 ptr 所指向对象的 shared_ptr
         * @tparam UTy 指针类型，可转换为 Ty
         * @tparam Dx 删除器类型
         * @tparam Alloc 分配器类型
         * @param ptr 指向要管理对象的指针
         * @param deleter 用于销毁对象的删除器
         * @param allocator 用于分配控制块的分配器
         */
        template <typename UTy, typename Dx, typename Alloc,
                  type_traits::other_trans::enable_if_t<
                      type_traits::logical_traits::conjunction_v<type_traits::properties::is_move_constructible<Dx>,
                                                                 implements::is_callable_function_object<Dx &, UTy *&>,
                                                                 implements::shared_convertible<UTy, Ty>>,
                      int> = 0>
        shared_ptr(UTy *ptr, Dx deleter, Alloc allocator) {
            set_ptr_and_deleter_and_also_alloc(ptr, utility::move(deleter), allocator);
        }

        /**
         * \lang english
         * @brief Constructs a shared_ptr with no managed object and a custom deleter.
         * @tparam Dx The deleter type
         * @param deleter The deleter used to destroy the object
         *
         * \lang simp-chinese
         * @brief 使用自定义删除器构造一个不管理任何对象的 shared_ptr
         * @tparam Dx 删除器类型
         * @param deleter 用于销毁对象的删除器
         */
        template <typename Dx,
                  type_traits::other_trans::enable_if_t<
                      type_traits::logical_traits::conjunction_v<type_traits::properties::is_move_constructible<Dx>,
                                                                 implements::is_callable_function_object<Dx &, std::nullptr_t &>,
                                                                 implements::shared_convertible<std::nullptr_t, Ty>>,
                      int> = 0>
        shared_ptr(std::nullptr_t, Dx deleter) {
            set_ptr_and_deleter(nullptr, deleter);
        }

        /**
         * \lang english
         * @brief Constructs a shared_ptr with no managed object, a custom deleter and an allocator.
         * @tparam Dx The deleter type
         * @tparam Alloc The allocator type
         * @param deleter The deleter used to destroy the object
         * @param allocator The allocator used to allocate the control block
         *
         * \lang simp-chinese
         * @brief 使用自定义删除器和分配器构造一个不管理任何对象的 shared_ptr
         * @tparam Dx 删除器类型
         * @tparam Alloc 分配器类型
         * @param deleter 用于销毁对象的删除器
         * @param allocator 用于分配控制块的分配器
         */
        template <typename Dx, typename Alloc,
                  type_traits::other_trans::enable_if_t<
                      type_traits::logical_traits::conjunction_v<type_traits::properties::is_move_constructible<Dx>,
                                                                 implements::is_callable_function_object<Dx &, std::nullptr_t &>,
                                                                 implements::shared_convertible<std::nullptr_t, Ty>>,
                      int> = 0>
        shared_ptr(std::nullptr_t, Dx deleter, Alloc allocator) {
            set_ptr_and_deleter_and_also_alloc(nullptr, deleter, allocator);
        }

        /**
         * \lang english
         * @brief Constructs a shared_ptr that shares ownership of the object managed by right, but stores px as the managed pointer (alias constructor).
         * @tparam UTy The element type of the source shared_ptr
         * @param right The shared_ptr to share ownership with
         * @param px The pointer to be stored as the managed pointer
         *
         * \lang simp-chinese
         * @brief 构造一个与 right 共享所有权、但以 px 作为管理指针的 shared_ptr（别名构造函数）
         * @tparam UTy 源 shared_ptr 的元素类型
         * @param right 与之共享所有权的 shared_ptr
         * @param px 将被存储为管理指针的指针
         */
        template <typename UTy>
        shared_ptr(const shared_ptr<UTy> &right, element_type *px) noexcept {
            this->alias_construct_from(right, px);
        }

        /**
         * \lang english
         * @brief Move alias constructor, shares ownership of the object managed by right, but stores px as the managed pointer.
         * @tparam UTy The element type of the source shared_ptr
         * @param right The shared_ptr to share ownership with
         * @param px The pointer to be stored as the managed pointer
         *
         * \lang simp-chinese
         * @brief 移动别名构造函数，与 right 共享所有权、但以 px 作为管理指针
         * @tparam UTy 源 shared_ptr 的元素类型
         * @param right 与之共享所有权的 shared_ptr
         * @param px 将被存储为管理指针的指针
         */
        template <typename UTy>
        shared_ptr(shared_ptr<UTy> &&right, element_type *px) noexcept {
            this->alias_move_constructFrom(utility::move(right), px);
        }

        /**
         * \lang english
         * @brief Copy constructor, shares ownership with right.
         * @param right The shared_ptr to share ownership with
         *
         * \lang simp-chinese
         * @brief 拷贝构造函数，与 right 共享所有权
         * @param right 与之共享所有权的 shared_ptr
         */
        shared_ptr(const shared_ptr &right) noexcept {
            this->copy_construct_from(right);
        }

        /**
         * \lang english
         * @brief Converting copy constructor, shares ownership with right.
         * @tparam UTy The element type of the source shared_ptr, convertible to Ty
         * @param right The shared_ptr to share ownership with
         *
         * \lang simp-chinese
         * @brief 转换拷贝构造函数，与 right 共享所有权
         * @tparam UTy 源 shared_ptr 的元素类型，可转换为 Ty
         * @param right 与之共享所有权的 shared_ptr
         */
        template <typename UTy, type_traits::other_trans::enable_if_t<implements::shared_pointer_compatible<UTy, Ty>::value, int> = 0>
        shared_ptr(const shared_ptr<UTy> &right) noexcept { // NOLINT
            this->copy_construct_from(right);
        }

        /**
         * \lang english
         * @brief Move constructor, takes over ownership from right.
         * @param right The shared_ptr to move from
         *
         * \lang simp-chinese
         * @brief 移动构造函数，接管 right 的所有权
         * @param right 要移入的 shared_ptr
         */
        shared_ptr(shared_ptr &&right) noexcept {
            this->move_construct_from(utility::move(right));
        }

        /**
         * \lang english
         * @brief Converting move constructor, takes over ownership from right.
         * @tparam UTy The element type of the source shared_ptr, convertible to Ty
         * @param right The shared_ptr to move from
         *
         * \lang simp-chinese
         * @brief 转换移动构造函数，接管 right 的所有权
         * @tparam UTy 源 shared_ptr 的元素类型，可转换为 Ty
         * @param right 要移入的 shared_ptr
         */
        template <typename UTy, type_traits::other_trans::enable_if_t<implements::shared_pointer_compatible<UTy, Ty>::value, int> = 0>
        shared_ptr(shared_ptr<UTy> &&right) noexcept { // NOLINT
            this->move_construct_from(utility::move(right));
        }

        /**
         * \lang english
         * @brief Internal construction tag constructor, used by make_shared and internal machinery.
         *
         * \lang simp-chinese
         * @brief 内部构造标记构造函数，供 make_shared 及内部机制使用
         */
        shared_ptr(core::internal_construct_tag_t, void *const ptr, implements::ref_count_base *const rx) {
            set_ptr_rep_and_enable_shared(static_cast<element_type *>(ptr), rx);
        }

        /**
         * \lang english
         * @brief Constructs a shared_ptr from a nebula_ptr by taking over its ownership.
         * @tparam UTy The element type of the nebula_ptr
         * @tparam Dx The deleter type of the nebula_ptr
         * @param right The nebula_ptr to take ownership from
         *
         * \lang simp-chinese
         * @brief 从 nebula_ptr 构造 shared_ptr，并接管其所有权
         * @tparam UTy nebula_ptr 的元素类型
         * @tparam Dx nebula_ptr 的删除器类型
         * @param right 要接管所有权的 nebula_ptr
         */
        template <typename UTy, typename Dx,
                  type_traits::other_trans::enable_if_t<
                      type_traits::logical_traits::conjunction_v<
                          implements::shared_pointer_compatible<UTy, Ty>,
                          type_traits::type_relations::is_convertible<typename nebula_ptr<UTy, Dx>::pointer, element_type *>>,
                      int> = 0>
        shared_ptr(nebula_ptr<UTy, Dx> &&right) {
            auto *const ptr = right.get();
            set_ptr_and_deleter(ptr, utility::move(right.get_deleter()));
            static_cast<void>(right.release());
        }

        /**
         * \lang english
         * @brief Constructs a shared_ptr from a weak_ptr, throwing if the weak_ptr has expired.
         * @param right The weak_ptr to lock
         * @throws std::bad_weak_ptr If the weak_ptr is expired
         *
         * \lang simp-chinese
         * @brief 从 weak_ptr 构造 shared_ptr，若 weak_ptr 已过期则抛出异常
         * @param right 要锁定的 weak_ptr
         * @throws std::bad_weak_ptr 如果 weak_ptr 已过期
         */
        explicit shared_ptr(const weak_ptr<Ty> &right) {
            if (!this->construct_from_weak(right)) {
                throw std::bad_weak_ptr{};
            }
        }

        /**
         * \lang english
         * @brief Destructor, releases the ownership of the managed object.
         *
         * \lang simp-chinese
         * @brief 析构函数，释放所管理对象的所有权
         */
        ~shared_ptr() noexcept {
            this->dref();
        }

        /**
         * \lang english
         * @brief Copy assignment operator, shares ownership with right.
         * @param right The shared_ptr to share ownership with
         * @return Reference to this shared_ptr
         *
         * \lang simp-chinese
         * @brief 拷贝赋值运算符，与 right 共享所有权
         * @param right 与之共享所有权的 shared_ptr
         * @return 返回本 shared_ptr 的引用
         */
        shared_ptr &operator=(const shared_ptr &right) noexcept {
            shared_ptr(right).swap(*this);
            return *this;
        }

        /**
         * \lang english
         * @brief Converting copy assignment operator, shares ownership with right.
         * @tparam UTy The element type of the source shared_ptr, convertible to Ty
         * @param right The shared_ptr to share ownership with
         * @return Reference to this shared_ptr
         *
         * \lang simp-chinese
         * @brief 转换拷贝赋值运算符，与 right 共享所有权
         * @tparam UTy 源 shared_ptr 的元素类型，可转换为 Ty
         * @param right 与之共享所有权的 shared_ptr
         * @return 返回本 shared_ptr 的引用
         */
        template <typename UTy, type_traits::other_trans::enable_if_t<implements::shared_pointer_compatible<UTy, Ty>::value, int> = 0>
        shared_ptr &operator=(const shared_ptr<UTy> &right) noexcept {
            shared_ptr(right).swap(*this);
            return *this;
        }

        /**
         * \lang english
         * @brief Move assignment operator, takes over ownership from right.
         * @param right The shared_ptr to move from
         * @return Reference to this shared_ptr
         *
         * \lang simp-chinese
         * @brief 移动赋值运算符，接管 right 的所有权
         * @param right 要移入的 shared_ptr
         * @return 返回本 shared_ptr 的引用
         */
        shared_ptr &operator=(shared_ptr &&right) noexcept {
            shared_ptr(utility::move(right)).swap(*this);
            return *this;
        }

        /**
         * \lang english
         * @brief Converting move assignment operator, takes over ownership from right.
         * @tparam UTy The element type of the source shared_ptr, convertible to Ty
         * @param right The shared_ptr to move from
         * @return Reference to this shared_ptr
         *
         * \lang simp-chinese
         * @brief 转换移动赋值运算符，接管 right 的所有权
         * @tparam UTy 源 shared_ptr 的元素类型，可转换为 Ty
         * @param right 要移入的 shared_ptr
         * @return 返回本 shared_ptr 的引用
         */
        template <typename UTy, type_traits::other_trans::enable_if_t<implements::shared_pointer_compatible<UTy, Ty>::value, int> = 0>
        shared_ptr &operator=(shared_ptr<UTy> &&right) noexcept {
            shared_ptr(utility::move(right)).swap(*this);
            return *this;
        }

        /**
         * \lang english
         * @brief Assigns a nebula_ptr to this shared_ptr by taking over its ownership.
         * @tparam UTy The element type of the nebula_ptr
         * @tparam Dx The deleter type of the nebula_ptr
         * @param right The nebula_ptr to take ownership from
         * @return Reference to this shared_ptr
         *
         * \lang simp-chinese
         * @brief 将 nebula_ptr 赋值给本 shared_ptr，并接管其所有权
         * @tparam UTy nebula_ptr 的元素类型
         * @tparam Dx nebula_ptr 的删除器类型
         * @param right 要接管所有权的 nebula_ptr
         * @return 返回本 shared_ptr 的引用
         */
        template <typename UTy, typename Dx,
                  type_traits::other_trans::enable_if_t<
                      type_traits::logical_traits::conjunction_v<
                          implements::shared_pointer_compatible<UTy, Ty>,
                          type_traits::type_relations::is_convertible<typename nebula_ptr<UTy, Dx>::pointer, element_type *>>,
                      int> = 0>
        shared_ptr &operator=(nebula_ptr<UTy, Dx> &&right) {
            shared_ptr(utility::move(right)).swap(*this);
            return *this;
        }

        /**
         * \lang english
         * @brief Assigns a raw pointer to this shared_ptr.
         * @tparam UTy The type of the pointer, convertible to Ty
         * @param ptr The pointer to the object to manage
         * @return Reference to this shared_ptr
         *
         * \lang simp-chinese
         * @brief 将原始指针赋值给本 shared_ptr
         * @tparam UTy 指针类型，可转换为 Ty
         * @param ptr 指向要管理对象的指针
         * @return 返回本 shared_ptr 的引用
         */
        template <typename UTy,
                  type_traits::other_trans::enable_if_t<
                      type_traits::logical_traits::conjunction_v<
                          type_traits::other_trans::conditional_t<type_traits::primary_types::is_array_v<Ty>,
                                                                  implements::can_del_arr<UTy>, implements::can_scalar_del<UTy>>,
                          implements::shared_convertible<UTy, Ty>>,
                      int> = 0>
        shared_ptr &operator=(UTy *ptr) {
            shared_ptr(ptr).swap(*this);
            return *this;
        }

        /**
         * \lang english
         * @brief Swaps the managed objects and ownerships of two shared_ptr objects.
         * @param right The shared_ptr to swap with
         *
         * \lang simp-chinese
         * @brief 交换两个 shared_ptr 对象所管理的对象和所有权
         * @param right 要交换的 shared_ptr
         */
        void swap(shared_ptr &right) noexcept {
            this->swap_(right);
        }

        /**
         * \lang english
         * @brief Releases the ownership of the managed object.
         *
         * \lang simp-chinese
         * @brief 释放所管理对象的所有权
         */
        void reset() noexcept {
            shared_ptr{}.swap(*this);
        }

        /**
         * \lang english
         * @brief Replaces the managed object with the object pointed to by ptr.
         * @tparam UTy The type of the pointer, convertible to Ty
         * @param ptr The pointer to the new object to manage
         *
         * \lang simp-chinese
         * @brief 用 ptr 所指向的对象替换被管理对象
         * @tparam UTy 指针类型，可转换为 Ty
         * @param ptr 指向新的要管理对象的指针
         */
        template <typename UTy,
                  type_traits::other_trans::enable_if_t<
                      type_traits::logical_traits::conjunction_v<
                          type_traits::other_trans::conditional_t<type_traits::primary_types::is_array_v<Ty>,
                                                                  implements::can_del_arr<UTy>, implements::can_scalar_del<UTy>>,
                          implements::shared_convertible<UTy, Ty>>,
                      int> = 0>
        void reset(UTy *ptr) {
            shared_ptr(ptr).swap(*this);
        }

        /**
         * \lang english
         * @brief Replaces the managed object with the object pointed to by ptr and a custom deleter.
         * @tparam UTy The type of the pointer, convertible to Ty
         * @tparam Dx The deleter type
         * @param ptr The pointer to the new object to manage
         * @param deleter The deleter used to destroy the object
         *
         * \lang simp-chinese
         * @brief 用 ptr 所指向的对象和自定义删除器替换被管理对象
         * @tparam UTy 指针类型，可转换为 Ty
         * @tparam Dx 删除器类型
         * @param ptr 指向新的要管理对象的指针
         * @param deleter 用于销毁对象的删除器
         */
        template <typename UTy, typename Dx,
                  type_traits::other_trans::enable_if_t<
                      type_traits::logical_traits::conjunction_v<type_traits::properties::is_move_constructible<Dx>,
                                                                 implements::is_callable_function_object<Dx &, UTy *&>,
                                                                 implements::shared_convertible<UTy, Ty>>,
                      int> = 0>
        void reset(UTy *ptr, Dx deleter) {
            shared_ptr(ptr, deleter).swap(*this);
        }

        /**
         * \lang english
         * @brief Replaces the managed object with the object pointed to by ptr, a custom deleter and an allocator.
         * @tparam UTy The type of the pointer, convertible to Ty
         * @tparam Dx The deleter type
         * @tparam Alloc The allocator type
         * @param ptr The pointer to the new object to manage
         * @param deleter The deleter used to destroy the object
         * @param allocator The allocator used to allocate the control block
         *
         * \lang simp-chinese
         * @brief 用 ptr 所指向的对象、自定义删除器和分配器替换被管理对象
         * @tparam UTy 指针类型，可转换为 Ty
         * @tparam Dx 删除器类型
         * @tparam Alloc 分配器类型
         * @param ptr 指向新的要管理对象的指针
         * @param deleter 用于销毁对象的删除器
         * @param allocator 用于分配控制块的分配器
         */
        template <typename UTy, typename Dx, typename Alloc,
                  type_traits::other_trans::enable_if_t<
                      type_traits::logical_traits::conjunction_v<type_traits::properties::is_move_constructible<Dx>,
                                                                 implements::is_callable_function_object<Dx &, UTy *&>,
                                                                 implements::shared_convertible<UTy, Ty>>,
                      int> = 0>
        void reset(UTy *ptr, Dx deleter, Alloc allocator) {
            shared_ptr(ptr, deleter, allocator).swap(*this);
        }

        /**
         * \lang english
         * @brief Dereferences the managed object.
         * @return Reference to the managed object
         *
         * \lang simp-chinese
         * @brief 解引用被管理对象
         * @return 返回被管理对象的引用
         */
        inline element_type &operator*() const noexcept {
            return *get();
        }

        /**
         * \lang english
         * @brief Provides access to the managed object's members.
         * @return Pointer to the managed object
         *
         * \lang simp-chinese
         * @brief 提供对被管理对象成员的访问
         * @return 返回指向被管理对象的指针
         */
        inline pointer operator->() const noexcept {
            return get();
        }

        /**
         * \lang english
         * @brief Provides indexed access to the managed array.
         * @tparam U The element type of the managed array
         * @param index The index of the element to access
         * @return Reference to the element at the given index
         *
         * \lang simp-chinese
         * @brief 提供对被管理数组的索引访问
         * @tparam U 被管理数组的元素类型
         * @param index 要访问的元素下标
         * @return 返回指定下标元素的引用
         */
        template <typename U = Ty, type_traits::other_trans::enable_if_t<type_traits::primary_types::is_array_v<U>, int> = 0>
        element_type &operator[](const std::size_t index) const noexcept {
            return get()[index];
        }

        /**
         * \lang english
         * @brief Gets the managed pointer.
         * @return The managed pointer
         *
         * \lang simp-chinese
         * @brief 获取被管理指针
         * @return 返回被管理指针
         */
        RAINY_NODISCARD pointer get() const noexcept {
            return this->pair.first;
        }

        /**
         * \lang english
         * @brief Checks whether this shared_ptr owns a non-null pointer.
         * @return true if the managed pointer is non-null, otherwise false
         *
         * \lang simp-chinese
         * @brief 检查本 shared_ptr 是否拥有非空指针
         * @return 如果被管理指针非空返回 true，否则返回 false
         */
        inline explicit operator bool() const noexcept {
            return static_cast<bool>(this->pair.first);
        }

    private:
        template <typename UptrOrNullptr, typename Dx>
        void set_ptr_and_deleter(const UptrOrNullptr px, Dx deleter) {
            implements::temporary_owner_del<UptrOrNullptr, Dx> owner(px, deleter);
            set_ptr_rep_and_enable_shared(
                owner.ptr, new implements::ref_count_resource<UptrOrNullptr, Dx>(owner.ptr, utility::move(deleter)));
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
            allocator_traits<ref_count_res_alloc>::construct(alloc, constructor.ptr, owner.ptr, utility::move(deleter), ax);
            set_ptr_rep_and_enable_shared(owner.ptr, constructor.ptr);
            constructor.release();
            owner.need_release = false;
        }

        template <typename UTy>
        void set_ptr_rep_and_enable_shared(UTy *const px, implements::ref_count_base *const rx) noexcept {
            this->pair.first = px;
            this->pair.second = rx;
            if constexpr (!type_traits::primary_types::is_array_v<Ty> && !type_traits::properties::is_volatile_v<UTy> &&
                          implements::can_enable_shared<UTy>::value) {
                if (px && px->wptr.expired()) {
                    px->wptr = shared_ptr<type_traits::modifers::remove_cv_t<UTy>>(
                        *this, const_cast<type_traits::modifers::remove_cv_t<UTy> *>(px));
                }
            }
        }

        void set_ptr_rep_and_enable_shared(std::nullptr_t, implements::ref_count_base *const rx) noexcept {
            this->pair.first = nullptr;
            this->pair.second = rx;
        }
    };

    /**
     * \lang english
     * @brief Deduction guide, deduces the element type from a weak_ptr.
     * @tparam Ty The element type of the weak_ptr
     *
     * \lang simp-chinese
     * @brief 推导指引，从 weak_ptr 推导元素类型
     * @tparam Ty weak_ptr 的元素类型
     */
    template <typename Ty>
    shared_ptr(weak_ptr<Ty>) -> shared_ptr<Ty>;

    /**
     * \lang english
     * @brief Deduction guide, deduces the element type from a nebula_ptr.
     * @tparam Ty The element type of the nebula_ptr
     * @tparam D The deleter type of the nebula_ptr
     *
     * \lang simp-chinese
     * @brief 推导指引，从 nebula_ptr 推导元素类型
     * @tparam Ty nebula_ptr 的元素类型
     * @tparam D nebula_ptr 的删除器类型
     */
    template <typename Ty, typename D>
    shared_ptr(nebula_ptr<Ty, D>) -> shared_ptr<Ty>;

    /**
     * \lang english
     * @brief Compares two shared_ptr objects for equality.
     * @tparam Ty The element type of the left shared_ptr
     * @tparam UTy The element type of the right shared_ptr
     * @param left The left shared_ptr
     * @param right The right shared_ptr
     * @return true if both manage the same pointer, otherwise false
     *
     * \lang simp-chinese
     * @brief 比较两个 shared_ptr 对象是否相等
     * @tparam Ty 左 shared_ptr 的元素类型
     * @tparam UTy 右 shared_ptr 的元素类型
     * @param left 左侧 shared_ptr
     * @param right 右侧 shared_ptr
     * @return 如果两者管理相同指针返回 true，否则返回 false
     */
    template <typename Ty, typename UTy>
    bool operator==(const shared_ptr<Ty> &left, const shared_ptr<UTy> &right) noexcept {
        return left.get() == right.get();
    }

    /**
     * \lang english
     * @brief Compares two shared_ptr objects for inequality.
     * @tparam Ty The element type of the left shared_ptr
     * @tparam UTy The element type of the right shared_ptr
     * @param left The left shared_ptr
     * @param right The right shared_ptr
     * @return true if both manage different pointers, otherwise false
     *
     * \lang simp-chinese
     * @brief 比较两个 shared_ptr 对象是否不相等
     * @tparam Ty 左 shared_ptr 的元素类型
     * @tparam UTy 右 shared_ptr 的元素类型
     * @param left 左侧 shared_ptr
     * @param right 右侧 shared_ptr
     * @return 如果两者管理不同指针返回 true，否则返回 false
     */
    template <typename Ty, typename UTy>
    bool operator!=(const shared_ptr<Ty> &left, const shared_ptr<UTy> &right) noexcept {
        return !(left == right);
    }

    /**
     * \lang english
     * @brief Compares two shared_ptr objects using less-than on the managed pointers.
     * @tparam Ty The element type of the left shared_ptr
     * @tparam UTy The element type of the right shared_ptr
     * @param left The left shared_ptr
     * @param right The right shared_ptr
     * @return true if left's managed pointer is less than right's
     *
     * \lang simp-chinese
     * @brief 按被管理指针比较两个 shared_ptr 对象是否小于
     * @tparam Ty 左 shared_ptr 的元素类型
     * @tparam UTy 右 shared_ptr 的元素类型
     * @param left 左侧 shared_ptr
     * @param right 右侧 shared_ptr
     * @return 如果 left 的被管理指针小于 right 的返回 true
     */
    template <typename Ty, typename UTy>
    bool operator<(const shared_ptr<Ty> &left, const shared_ptr<UTy> &right) noexcept {
        return left.get() < right.get();
    }

    /**
     * \lang english
     * @brief Compares two shared_ptr objects using greater-than on the managed pointers.
     * @tparam Ty The element type of the left shared_ptr
     * @tparam UTy The element type of the right shared_ptr
     * @param left The left shared_ptr
     * @param right The right shared_ptr
     * @return true if left's managed pointer is greater than right's
     *
     * \lang simp-chinese
     * @brief 按被管理指针比较两个 shared_ptr 对象是否大于
     * @tparam Ty 左 shared_ptr 的元素类型
     * @tparam UTy 右 shared_ptr 的元素类型
     * @param left 左侧 shared_ptr
     * @param right 右侧 shared_ptr
     * @return 如果 left 的被管理指针大于 right 的返回 true
     */
    template <typename Ty, typename UTy>
    bool operator>(const shared_ptr<Ty> &left, const shared_ptr<UTy> &right) noexcept {
        return right < left;
    }

    /**
     * \lang english
     * @brief Compares two shared_ptr objects using less-than-or-equal on the managed pointers.
     * @tparam Ty The element type of the left shared_ptr
     * @tparam UTy The element type of the right shared_ptr
     * @param left The left shared_ptr
     * @param right The right shared_ptr
     * @return true if left's managed pointer is less than or equal to right's
     *
     * \lang simp-chinese
     * @brief 按被管理指针比较两个 shared_ptr 对象是否小于等于
     * @tparam Ty 左 shared_ptr 的元素类型
     * @tparam UTy 右 shared_ptr 的元素类型
     * @param left 左侧 shared_ptr
     * @param right 右侧 shared_ptr
     * @return 如果 left 的被管理指针小于等于 right 的返回 true
     */
    template <typename Ty, typename UTy>
    bool operator<=(const shared_ptr<Ty> &left, const shared_ptr<UTy> &right) noexcept {
        return !(right < left);
    }

    /**
     * \lang english
     * @brief Compares two shared_ptr objects using greater-than-or-equal on the managed pointers.
     * @tparam Ty The element type of the left shared_ptr
     * @tparam UTy The element type of the right shared_ptr
     * @param left The left shared_ptr
     * @param right The right shared_ptr
     * @return true if left's managed pointer is greater than or equal to right's
     *
     * \lang simp-chinese
     * @brief 按被管理指针比较两个 shared_ptr 对象是否大于等于
     * @tparam Ty 左 shared_ptr 的元素类型
     * @tparam UTy 右 shared_ptr 的元素类型
     * @param left 左侧 shared_ptr
     * @param right 右侧 shared_ptr
     * @return 如果 left 的被管理指针大于等于 right 的返回 true
     */
    template <typename Ty, typename UTy>
    bool operator>=(const shared_ptr<Ty> &left, const shared_ptr<UTy> &right) noexcept {
        return !(left < right);
    }

    /**
     * \lang english
     * @brief Compares a shared_ptr with nullptr for equality.
     * @tparam Ty The element type of the shared_ptr
     * @param left The shared_ptr
     * @return true if left manages no object, otherwise false
     *
     * \lang simp-chinese
     * @brief 比较 shared_ptr 与 nullptr 是否相等
     * @tparam Ty shared_ptr 的元素类型
     * @param left shared_ptr
     * @return 如果 left 不管理任何对象返回 true，否则返回 false
     */
    template <typename Ty>
    bool operator==(const shared_ptr<Ty> &left, std::nullptr_t) noexcept {
        return left.get() == nullptr;
    }

    /**
     * \lang english
     * @brief Compares nullptr with a shared_ptr for equality.
     * @tparam Ty The element type of the shared_ptr
     * @param right The shared_ptr
     * @return true if right manages no object, otherwise false
     *
     * \lang simp-chinese
     * @brief 比较 nullptr 与 shared_ptr 是否相等
     * @tparam Ty shared_ptr 的元素类型
     * @param right shared_ptr
     * @return 如果 right 不管理任何对象返回 true，否则返回 false
     */
    template <typename Ty>
    bool operator==(std::nullptr_t, const shared_ptr<Ty> &right) noexcept {
        return nullptr == right.get();
    }

    /**
     * \lang english
     * @brief Compares a shared_ptr with nullptr for inequality.
     * @tparam Ty The element type of the shared_ptr
     * @param left The shared_ptr
     * @return true if left manages an object, otherwise false
     *
     * \lang simp-chinese
     * @brief 比较 shared_ptr 与 nullptr 是否不相等
     * @tparam Ty shared_ptr 的元素类型
     * @param left shared_ptr
     * @return 如果 left 管理对象返回 true，否则返回 false
     */
    template <typename Ty>
    bool operator!=(const shared_ptr<Ty> &left, std::nullptr_t) noexcept {
        return !(left == nullptr);
    }

    /**
     * \lang english
     * @brief Compares nullptr with a shared_ptr for inequality.
     * @tparam Ty The element type of the shared_ptr
     * @param right The shared_ptr
     * @return true if right manages an object, otherwise false
     *
     * \lang simp-chinese
     * @brief 比较 nullptr 与 shared_ptr 是否不相等
     * @tparam Ty shared_ptr 的元素类型
     * @param right shared_ptr
     * @return 如果 right 管理对象返回 true，否则返回 false
     */
    template <typename Ty>
    bool operator!=(std::nullptr_t, const shared_ptr<Ty> &right) noexcept {
        return !(nullptr == right);
    }

    /**
     * \lang english
     * @brief Compares a shared_ptr with nullptr using less-than on the managed pointer.
     * @tparam Ty The element type of the shared_ptr
     * @param left The shared_ptr
     * @return true if left's managed pointer is less than nullptr
     *
     * \lang simp-chinese
     * @brief 按被管理指针比较 shared_ptr 与 nullptr 是否小于
     * @tparam Ty shared_ptr 的元素类型
     * @param left shared_ptr
     * @return 如果 left 的被管理指针小于 nullptr 返回 true
     */
    template <typename Ty>
    bool operator<(const shared_ptr<Ty> &left, std::nullptr_t) noexcept {
        return left.get() < static_cast<typename shared_ptr<Ty>::pointer>(nullptr);
    }

    /**
     * \lang english
     * @brief Compares nullptr with a shared_ptr using less-than on the managed pointer.
     * @tparam Ty The element type of the shared_ptr
     * @param right The shared_ptr
     * @return true if nullptr is less than right's managed pointer
     *
     * \lang simp-chinese
     * @brief 按被管理指针比较 nullptr 与 shared_ptr 是否小于
     * @tparam Ty shared_ptr 的元素类型
     * @param right shared_ptr
     * @return 如果 nullptr 小于 right 的被管理指针返回 true
     */
    template <typename Ty>
    bool operator<(std::nullptr_t, const shared_ptr<Ty> &right) noexcept {
        return static_cast<typename shared_ptr<Ty>::pointer>(nullptr) < right.get();
    }

    /**
     * \lang english
     * @brief Compares a shared_ptr with nullptr using greater-than on the managed pointer.
     * @tparam Ty The element type of the shared_ptr
     * @param left The shared_ptr
     * @return true if left's managed pointer is greater than nullptr
     *
     * \lang simp-chinese
     * @brief 按被管理指针比较 shared_ptr 与 nullptr 是否大于
     * @tparam Ty shared_ptr 的元素类型
     * @param left shared_ptr
     * @return 如果 left 的被管理指针大于 nullptr 返回 true
     */
    template <typename Ty>
    bool operator>(const shared_ptr<Ty> &left, std::nullptr_t) noexcept {
        return nullptr < left;
    }

    /**
     * \lang english
     * @brief Compares nullptr with a shared_ptr using greater-than on the managed pointer.
     * @tparam Ty The element type of the shared_ptr
     * @param right The shared_ptr
     * @return true if nullptr is greater than right's managed pointer
     *
     * \lang simp-chinese
     * @brief 按被管理指针比较 nullptr 与 shared_ptr 是否大于
     * @tparam Ty shared_ptr 的元素类型
     * @param right shared_ptr
     * @return 如果 nullptr 大于 right 的被管理指针返回 true
     */
    template <typename Ty>
    bool operator>(std::nullptr_t, const shared_ptr<Ty> &right) noexcept {
        return right < nullptr;
    }

    /**
     * \lang english
     * @brief Compares a shared_ptr with nullptr using less-than-or-equal on the managed pointer.
     * @tparam Ty The element type of the shared_ptr
     * @param left The shared_ptr
     * @return true if left's managed pointer is less than or equal to nullptr
     *
     * \lang simp-chinese
     * @brief 按被管理指针比较 shared_ptr 与 nullptr 是否小于等于
     * @tparam Ty shared_ptr 的元素类型
     * @param left shared_ptr
     * @return 如果 left 的被管理指针小于等于 nullptr 返回 true
     */
    template <typename Ty>
    bool operator<=(const shared_ptr<Ty> &left, std::nullptr_t) noexcept {
        return !(nullptr < left);
    }

    /**
     * \lang english
     * @brief Compares nullptr with a shared_ptr using less-than-or-equal on the managed pointer.
     * @tparam Ty The element type of the shared_ptr
     * @param right The shared_ptr
     * @return true if nullptr is less than or equal to right's managed pointer
     *
     * \lang simp-chinese
     * @brief 按被管理指针比较 nullptr 与 shared_ptr 是否小于等于
     * @tparam Ty shared_ptr 的元素类型
     * @param right shared_ptr
     * @return 如果 nullptr 小于等于 right 的被管理指针返回 true
     */
    template <typename Ty>
    bool operator<=(std::nullptr_t, const shared_ptr<Ty> &right) noexcept {
        return !(right < nullptr);
    }

    /**
     * \lang english
     * @brief Compares a shared_ptr with nullptr using greater-than-or-equal on the managed pointer.
     * @tparam Ty The element type of the shared_ptr
     * @param left The shared_ptr
     * @return true if left's managed pointer is greater than or equal to nullptr
     *
     * \lang simp-chinese
     * @brief 按被管理指针比较 shared_ptr 与 nullptr 是否大于等于
     * @tparam Ty shared_ptr 的元素类型
     * @param left shared_ptr
     * @return 如果 left 的被管理指针大于等于 nullptr 返回 true
     */
    template <typename Ty>
    bool operator>=(const shared_ptr<Ty> &left, std::nullptr_t) noexcept {
        return !(left < nullptr);
    }

    /**
     * \lang english
     * @brief Compares nullptr with a shared_ptr using greater-than-or-equal on the managed pointer.
     * @tparam Ty The element type of the shared_ptr
     * @param right The shared_ptr
     * @return true if nullptr is greater than or equal to right's managed pointer
     *
     * \lang simp-chinese
     * @brief 按被管理指针比较 nullptr 与 shared_ptr 是否大于等于
     * @tparam Ty shared_ptr 的元素类型
     * @param right shared_ptr
     * @return 如果 nullptr 大于等于 right 的被管理指针返回 true
     */
    template <typename Ty>
    bool operator>=(std::nullptr_t, const shared_ptr<Ty> &right) noexcept {
        return !(nullptr < right);
    }

    /**
     * \lang english
     * @brief Creates a shared_ptr that manages a newly constructed object of type Ty.
     * @tparam Ty The type of the object to create
     * @tparam Args The argument types passed to the constructor of Ty
     * @param args The arguments forwarded to the constructor of Ty
     * @return A shared_ptr managing the newly constructed object
     *
     * \lang simp-chinese
     * @brief 创建一个管理新建 Ty 类型对象的 shared_ptr
     * @tparam Ty 要创建的对象的类型
     * @tparam Args 传递给 Ty 构造函数的参数类型
     * @param args 转发给 Ty 构造函数的参数
     * @return 一个管理新建对象的 shared_ptr
     */
    template <typename Ty, typename... Args,
              type_traits::other_trans::enable_if_t<!type_traits::primary_types::is_array_v<Ty>, int> = 0>
    shared_ptr<Ty> make_shared(Args &&...args) {
        auto *control_block = new implements::ref_count_allocated<Ty>(utility::forward<Args>(args)...);
        shared_ptr<Ty> result(core::internal_construct_tag, control_block->get_ptr(), control_block);
        return result;
    }

    /**
     * \lang english
     * @brief Creates a shared_ptr that manages a newly constructed unbounded array.
     * @tparam Ty The unbounded array type, e.g. int[]
     * @param count The number of elements in the array
     * @return A shared_ptr managing the newly constructed array
     *
     * \lang simp-chinese
     * @brief 创建一个管理新建无界数组的 shared_ptr
     * @tparam Ty 无界数组类型，如 int[]
     * @param count 数组的元素个数
     * @return 一个管理新建数组的 shared_ptr
     */
    template <typename Ty, type_traits::other_trans::enable_if_t<type_traits::primary_types::is_unbounded_array_v<Ty>, int> = 0>
    shared_ptr<Ty> make_shared(std::size_t count) {
        using element_type = type_traits::modifers::remove_extent_t<Ty>;
        implements::ref_count_allocated_array<element_type> *control_block = nullptr;
        // NOLINTBEGIN
        try {
            control_block = new implements::ref_count_allocated_array<element_type>(count);
            shared_ptr<Ty> result(core::internal_construct_tag, control_block->get_ptr(), control_block);
            return result;
        } catch (...) {
            delete control_block;
            throw;
        }
        // NOLINTEND
    }

    /**
     * \lang english
     * @brief Creates a shared_ptr that manages a newly constructed bounded array.
     * @tparam Ty The bounded array type, e.g. int[3]
     * @return A shared_ptr managing the newly constructed array
     *
     * \lang simp-chinese
     * @brief 创建一个管理新建有界数组的 shared_ptr
     * @tparam Ty 有界数组类型，如 int[3]
     * @return 一个管理新建数组的 shared_ptr
     */
    template <typename Ty, type_traits::other_trans::enable_if_t<type_traits::primary_types::is_bounded_array_v<Ty>, int> = 0>
    shared_ptr<Ty> make_shared() {
        constexpr std::size_t count = type_traits::modifers::extent_v<Ty>;
        using element_type = type_traits::modifers::remove_extent_t<Ty>;
        implements::ref_count_allocated_array<element_type> *control_block = nullptr;
        // NOLINTBEGIN
        try {
            control_block = new implements::ref_count_allocated_array<element_type>(count);
            shared_ptr<Ty> result(core::internal_construct_tag, control_block->get_ptr(), control_block);
            return result;
        } catch (...) {
            delete control_block;
            throw;
        }
        // NOLINTEND
    }

    /**
     * \lang english
     * @brief Creates a shared_ptr that manages a newly constructed unbounded array, initialized with the given value.
     * @tparam Ty The unbounded array type, e.g. int[]
     * @param count The number of elements in the array
     * @param u The value used to initialize each element
     * @return A shared_ptr managing the newly constructed array
     *
     * \lang simp-chinese
     * @brief 创建一个管理新建无界数组的 shared_ptr，并使用给定值初始化每个元素
     * @tparam Ty 无界数组类型，如 int[]
     * @param count 数组的元素个数
     * @param u 用于初始化每个元素的值
     * @return 一个管理新建数组的 shared_ptr
     */
    template <typename Ty, type_traits::other_trans::enable_if_t<type_traits::primary_types::is_unbounded_array_v<Ty>, int> = 0>
    shared_ptr<Ty> make_shared(std::size_t count, const type_traits::modifers::remove_extent_t<Ty> &u) {
        using element_type = type_traits::modifers::remove_extent_t<Ty>;
        implements::ref_count_allocated_array<element_type> *control_block = nullptr;
        // NOLINTBEGIN
        try {
            control_block = new implements::ref_count_allocated_array<element_type>(count, u);
            shared_ptr<Ty> result(core::internal_construct_tag, control_block->get_ptr(), control_block);
            return result;
        } catch (...) {
            delete control_block;
            throw;
        }
        // NOLINTEND
    }

    /**
     * \lang english
     * @brief Creates a shared_ptr that manages a newly constructed bounded array, initialized with the given value.
     * @tparam Ty The bounded array type, e.g. int[3]
     * @param u The value used to initialize each element
     * @return A shared_ptr managing the newly constructed array
     *
     * \lang simp-chinese
     * @brief 创建一个管理新建有界数组的 shared_ptr，并使用给定值初始化每个元素
     * @tparam Ty 有界数组类型，如 int[3]
     * @param u 用于初始化每个元素的值
     * @return 一个管理新建数组的 shared_ptr
     */
    template <typename Ty, type_traits::other_trans::enable_if_t<type_traits::primary_types::is_bounded_array_v<Ty>, int> = 0>
    shared_ptr<Ty> make_shared(const type_traits::modifers::remove_extent_t<Ty> &u) {
        constexpr std::size_t count = type_traits::modifers::extent_v<Ty>;
        using element_type = type_traits::modifers::remove_extent_t<Ty>;
        implements::ref_count_allocated_array<element_type> *control_block = nullptr;
        // NOLINTBEGIN
        try {
            control_block = new implements::ref_count_allocated_array<element_type>(count, u);
            shared_ptr<Ty> result(core::internal_construct_tag, control_block->get_ptr(), control_block);
            return result;
        } catch (...) {
            delete control_block;
            throw;
        }
        // NOLINTEND
    }
}

#if !RAINY_HAS_MUZIYAN_REACH_FOR_THE_MOON
namespace rainy::core::memory::implements {
    template <typename Ty, typename UTy, typename = void>
    static constexpr bool must_avoid_expired_conversions_from = true;

    template <typename Ty, typename UTy>
    static constexpr bool must_avoid_expired_conversions_from<
        Ty, UTy, type_traits::other_trans::void_t<decltype(static_cast<const UTy *>(static_cast<Ty *>(nullptr)))>> = false;
}
#endif

namespace rainy::core::memory {
    /**
     * \lang english
     * @brief A weak reference to an object managed by a shared_ptr.
     *
     * weak_ptr is a smart pointer that holds a non-owning ("weak") reference to an
     * object that is managed by a shared_ptr. To access the referenced object, it
     * must be converted to a shared_ptr through lock().
     *
     * @tparam Ty The type of the managed object
     *
     * \lang simp-chinese
     * @brief 对由 shared_ptr 管理的对象的弱引用
     *
     * weak_ptr 是持有对由 shared_ptr 管理的对象的非拥有（"弱"）引用的智能指针。
     * 要访问被引用的对象，必须通过 lock() 将其转换为 shared_ptr。
     *
     * @tparam Ty 被管理对象的类型
     */
    template <typename Ty>
    class weak_ptr : public implements::shared_ptr_base<Ty> {
    public:
        template <typename UTy>
        static constexpr bool must_avoid_expired_conversions_from = implements::must_avoid_expired_conversions_from<Ty, UTy>;

        /**
         * \lang english
         * @brief Constructs a weak_ptr with no managed object.
         *
         * \lang simp-chinese
         * @brief 构造一个不管理任何对象的 weak_ptr
         */
        constexpr weak_ptr() noexcept = default;

        /**
         * \lang english
         * @brief Copy constructor, shares the weak reference with right.
         * @param right The weak_ptr to copy
         *
         * \lang simp-chinese
         * @brief 拷贝构造函数，与 right 共享弱引用
         * @param right 要拷贝的 weak_ptr
         */
        weak_ptr(const weak_ptr &right) noexcept {
            this->weakly_construct_from(right);
        }

        /**
         * \lang english
         * @brief Constructs a weak_ptr that shares ownership with the shared_ptr right.
         * @tparam UTy The element type of the shared_ptr, convertible to Ty
         * @param right The shared_ptr to share ownership with
         *
         * \lang simp-chinese
         * @brief 构造一个与 shared_ptr right 共享所有权的 weak_ptr
         * @tparam UTy shared_ptr 的元素类型，可转换为 Ty
         * @param right 与之共享所有权的 shared_ptr
         */
        template <typename UTy, type_traits::other_trans::enable_if_t<implements::shared_pointer_compatible<UTy, Ty>::value, int> = 0>
        weak_ptr(const shared_ptr<UTy> &right) noexcept {
            this->weakly_construct_from(right);
        }

        /**
         * \lang english
         * @brief Converting copy constructor, shares the weak reference with right.
         * @tparam UTy The element type of the source weak_ptr, convertible to Ty
         * @param right The weak_ptr to share the reference with
         *
         * \lang simp-chinese
         * @brief 转换拷贝构造函数，与 right 共享弱引用
         * @tparam UTy 源 weak_ptr 的元素类型，可转换为 Ty
         * @param right 与之共享弱引用的 weak_ptr
         */
        template <typename UTy, type_traits::other_trans::enable_if_t<implements::shared_pointer_compatible<UTy, Ty>::value, int> = 0>
        weak_ptr(const weak_ptr<UTy> &right) noexcept {
            if constexpr (constexpr bool avoid_expired_conversions = must_avoid_expired_conversions_from<UTy>) {
                this->weakly_convert_lvalue_avoiding_expired_conversions(right);
            } else {
                this->weakly_construct_from(right);
            }
        }

        /**
         * \lang english
         * @brief Move constructor, takes over the weak reference from right.
         * @param right The weak_ptr to move from
         *
         * \lang simp-chinese
         * @brief 移动构造函数，接管 right 的弱引用
         * @param right 要移入的 weak_ptr
         */
        weak_ptr(weak_ptr &&right) noexcept {
            this->move_construct_from(utility::move(right));
        }

        /**
         * \lang english
         * @brief Converting move constructor, takes over the weak reference from right.
         * @tparam UTy The element type of the source weak_ptr, convertible to Ty
         * @param right The weak_ptr to move from
         *
         * \lang simp-chinese
         * @brief 转换移动构造函数，接管 right 的弱引用
         * @tparam UTy 源 weak_ptr 的元素类型，可转换为 Ty
         * @param right 要移入的 weak_ptr
         */
        template <typename UTy, type_traits::other_trans::enable_if_t<implements::shared_pointer_compatible<UTy, Ty>::value, int> = 0>
        explicit weak_ptr(weak_ptr<UTy> &&right) noexcept {
            if constexpr (constexpr bool avoid_expired_conversions = must_avoid_expired_conversions_from<UTy>;
                          avoid_expired_conversions) {
                this->weakly_convert_rvalue_avoiding_expired_conversions(utility::move(right));
            } else {
                this->move_construct_from(utility::move(right));
            }
        }

        /**
         * \lang english
         * @brief Destructor, releases the weak reference.
         *
         * \lang simp-chinese
         * @brief 析构函数，释放弱引用
         */
        ~weak_ptr() noexcept {
            this->deweakref();
        }

        /**
         * \lang english
         * @brief Copy assignment operator, shares the weak reference with right.
         * @param right The weak_ptr to copy
         * @return Reference to this weak_ptr
         *
         * \lang simp-chinese
         * @brief 拷贝赋值运算符，与 right 共享弱引用
         * @param right 要拷贝的 weak_ptr
         * @return 返回本 weak_ptr 的引用
         */
        weak_ptr &operator=(const weak_ptr &right) noexcept {
            weak_ptr(right).swap(*this);
            return *this;
        }

        /**
         * \lang english
         * @brief Converting copy assignment operator, shares the weak reference with right.
         * @tparam UTy The element type of the source weak_ptr, convertible to Ty
         * @param right The weak_ptr to share the reference with
         * @return Reference to this weak_ptr
         *
         * \lang simp-chinese
         * @brief 转换拷贝赋值运算符，与 right 共享弱引用
         * @tparam UTy 源 weak_ptr 的元素类型，可转换为 Ty
         * @param right 与之共享弱引用的 weak_ptr
         * @return 返回本 weak_ptr 的引用
         */
        template <typename UTy, type_traits::other_trans::enable_if_t<implements::shared_pointer_compatible<UTy, Ty>::value, int> = 0>
        weak_ptr &operator=(const weak_ptr<UTy> &right) noexcept {
            weak_ptr(right).swap(*this);
            return *this;
        }

        /**
         * \lang english
         * @brief Move assignment operator, takes over the weak reference from right.
         * @param right The weak_ptr to move from
         * @return Reference to this weak_ptr
         *
         * \lang simp-chinese
         * @brief 移动赋值运算符，接管 right 的弱引用
         * @param right 要移入的 weak_ptr
         * @return 返回本 weak_ptr 的引用
         */
        weak_ptr &operator=(weak_ptr &&right) noexcept {
            weak_ptr(utility::move(right)).swap(*this);
            return *this;
        }

        /**
         * \lang english
         * @brief Converting move assignment operator, takes over the weak reference from right.
         * @tparam UTy The element type of the source weak_ptr, convertible to Ty
         * @param right The weak_ptr to move from
         * @return Reference to this weak_ptr
         *
         * \lang simp-chinese
         * @brief 转换移动赋值运算符，接管 right 的弱引用
         * @tparam UTy 源 weak_ptr 的元素类型，可转换为 Ty
         * @param right 要移入的 weak_ptr
         * @return 返回本 weak_ptr 的引用
         */
        template <typename UTy, type_traits::other_trans::enable_if_t<implements::shared_pointer_compatible<UTy, Ty>::value, int> = 0>
        weak_ptr &operator=(weak_ptr<UTy> &&right) noexcept {
            weak_ptr(utility::move(right)).swap(*this);
            return *this;
        }

        /**
         * \lang english
         * @brief Assigns the shared ownership of the shared_ptr right to this weak_ptr.
         * @tparam UTy The element type of the shared_ptr, convertible to Ty
         * @param right The shared_ptr to share ownership with
         * @return Reference to this weak_ptr
         *
         * \lang simp-chinese
         * @brief 将 shared_ptr right 的共享所有权赋值给本 weak_ptr
         * @tparam UTy shared_ptr 的元素类型，可转换为 Ty
         * @param right 与之共享所有权的 shared_ptr
         * @return 返回本 weak_ptr 的引用
         */
        template <typename UTy, type_traits::other_trans::enable_if_t<implements::shared_pointer_compatible<UTy, Ty>::value, int> = 0>
        weak_ptr &operator=(const shared_ptr<UTy> &right) noexcept {
            weak_ptr(right).swap(*this);
            return *this;
        }

        /**
         * \lang english
         * @brief Releases the weak reference to the managed object.
         *
         * \lang simp-chinese
         * @brief 释放对被管理对象的弱引用
         */
        void reset() noexcept {
            weak_ptr{}.swap(*this);
        }

        /**
         * \lang english
         * @brief Swaps the weak references of two weak_ptr objects.
         * @param right The weak_ptr to swap with
         *
         * \lang simp-chinese
         * @brief 交换两个 weak_ptr 对象的弱引用
         * @param right 要交换的 weak_ptr
         */
        void swap(weak_ptr &right) noexcept {
            this->swap_(right);
        }

        /**
         * \lang english
         * @brief Checks whether the referenced object has already been destroyed.
         * @return true if the referenced object was already destroyed, otherwise false
         *
         * \lang simp-chinese
         * @brief 检查所引用的对象是否已被销毁
         * @return 如果被引用对象已被销毁返回 true，否则返回 false
         */
        RAINY_NODISCARD bool expired() const noexcept {
            return this->use_count() == 0;
        }

        /**
         * \lang english
         * @brief Creates a shared_ptr that manages the referenced object.
         * @return A shared_ptr that manages the referenced object, or an empty shared_ptr if it has expired
         *
         * \lang simp-chinese
         * @brief 创建一个管理被引用对象的 shared_ptr
         * @return 一个管理被引用对象的 shared_ptr；若对象已过期，则返回空的 shared_ptr
         */
        RAINY_NODISCARD shared_ptr<Ty> lock() const noexcept {
            shared_ptr<Ty> ret;
            ret.construct_from_weak(*this);
            return ret;
        }
    };
}

namespace rainy::core::memory {
    /**
     * \lang english
     * @brief A base class that allows an object to create shared_ptr instances referring to itself.
     *
     * enable_shared_from_this provides the member functions shared_from_this() and
     * weak_from_this(), which allow an object that is already managed by a shared_ptr
     * to safely obtain additional shared_ptr or weak_ptr instances referring to itself.
     *
     * @tparam Ty The type of the derived class
     *
     * \lang simp-chinese
     * @brief 允许对象创建指向自身的 shared_ptr 的基类
     *
     * enable_shared_from_this 提供 shared_from_this() 和 weak_from_this() 成员函数，
     * 使已被 shared_ptr 管理的对象能够安全地获得指向自身的额外 shared_ptr 或 weak_ptr 实例。
     *
     * @tparam Ty 派生类的类型
     */
    template <typename Ty>
    class enable_shared_from_this {
    public:
        using rts_enable_shared_type = enable_shared_from_this;

        /**
         * \lang english
         * @brief Returns a shared_ptr that shares ownership of this.
         * @return A shared_ptr that shares ownership of *this
         * @throws std::bad_weak_ptr If no shared_ptr owns *this
         *
         * \lang simp-chinese
         * @brief 返回一个与 this 共享所有权的 shared_ptr
         * @return 一个与 *this 共享所有权的 shared_ptr
         * @throws std::bad_weak_ptr 如果没有 shared_ptr 拥有 *this
         */
        RAINY_NODISCARD shared_ptr<Ty> shared_from_this() {
            return shared_ptr<Ty>(wptr);
        }

        /**
         * \lang english
         * @brief Returns a shared_ptr to const that shares ownership of this.
         * @return A shared_ptr to const that shares ownership of *this
         * @throws std::bad_weak_ptr If no shared_ptr owns *this
         *
         * \lang simp-chinese
         * @brief 返回一个与 this 共享所有权的指向 const 的 shared_ptr
         * @return 一个与 *this 共享所有权的指向 const 的 shared_ptr
         * @throws std::bad_weak_ptr 如果没有 shared_ptr 拥有 *this
         */
        RAINY_NODISCARD shared_ptr<const Ty> shared_from_this() const {
            return shared_ptr<const Ty>(wptr);
        }

        /**
         * \lang english
         * @brief Returns a weak_ptr that references this.
         * @return A weak_ptr that references *this
         *
         * \lang simp-chinese
         * @brief 返回一个引用 this 的 weak_ptr
         * @return 一个引用 *this 的 weak_ptr
         */
        RAINY_NODISCARD weak_ptr<Ty> weak_from_this() noexcept {
            return wptr;
        }

        /**
         * \lang english
         * @brief Returns a weak_ptr to const that references this.
         * @return A weak_ptr to const that references *this
         *
         * \lang simp-chinese
         * @brief 返回一个引用 this 的指向 const 的 weak_ptr
         * @return 一个引用 *this 的指向 const 的 weak_ptr
         */
        RAINY_NODISCARD weak_ptr<const Ty> weak_from_this() const noexcept {
            return wptr;
        }

    protected:
        /**
         * \lang english
         * @brief Default constructor.
         *
         * \lang simp-chinese
         * @brief 默认构造函数
         */
        constexpr enable_shared_from_this() noexcept : wptr() {
        }

        /**
         * \lang english
         * @brief Copy constructor, initializes the weak reference as empty.
         *
         * \lang simp-chinese
         * @brief 拷贝构造函数，将弱引用初始化为空
         */
        enable_shared_from_this(const enable_shared_from_this &) noexcept : wptr() {
        }

        /**
         * \lang english
         * @brief Copy assignment operator, does not copy the weak reference.
         * @return Reference to this enable_shared_from_this
         *
         * \lang simp-chinese
         * @brief 拷贝赋值运算符，不拷贝弱引用
         * @return 返回本 enable_shared_from_this 的引用
         */
        enable_shared_from_this &operator=(const enable_shared_from_this &) noexcept {
            return *this;
        }

        /**
         * \lang english
         * @brief Destructor.
         *
         * \lang simp-chinese
         * @brief 析构函数
         */
        ~enable_shared_from_this() = default;

    private:
        template <typename UTy>
        friend class shared_ptr;

        mutable weak_ptr<Ty> wptr;
    };
}

#endif
