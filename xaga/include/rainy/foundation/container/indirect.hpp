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
#ifndef RAINY_FOUNDATION_CONTAINER_INDIRECT_HPP
#define RAINY_FOUNDATION_CONTAINER_INDIRECT_HPP
#include <rainy/core/core.hpp>

namespace rainy::foundation::container {
    struct defered_init_t {};

    inline constexpr defered_init_t defered_init;

    template <typename Ty, typename Alloc = std::allocator<Ty>>
    class indirect {
    public:
        using value_type = Ty;
        using pointer = Ty *;
        using const_pointer = const Ty *;
        using allocator_type = Alloc;
        using reference = value_type &;
        using const_reference = const value_type &;

        static_assert(type_traits::composite_types::is_object_v<Ty>, "Ty must be a object");
        static_assert(!type_traits::composite_types::is_reference_v<Ty>, "Ty cannot be a reference type");
        static_assert(!(type_traits::type_properties::is_const_v<type_traits::reference_modify::remove_reference_t<Ty>> ||
                        type_traits::type_properties::is_volatile_v<type_traits::reference_modify::remove_reference_t<Ty>>),
                      "Ty cannot be a const/volatile type");
        static_assert(!type_traits::type_relations::is_same_v<Ty, std::in_place_t>, "Ty cannot be std::in_place_t");
        static_assert(!type_traits::primary_types::is_specialization_v<Ty, std::in_place_type_t>,
                      "Ty cannot be a specialization of std::in_place_type_t");

        template <type_traits::other_trans::enable_if_t<type_traits::type_properties::is_default_constructible_v<Ty>, int> = 0>
        RAINY_CONSTEXPR20 indirect() noexcept(type_traits::type_properties::is_nothrow_default_constructible_v<Ty>) :
            pair(allocator_type{}, nullptr) {
            pointer ptr = std::allocator_traits<allocator_type>::allocate(pair.get_first(), 1);
            try {
                std::allocator_traits<allocator_type>::construct(pair.get_first(), ptr);
                pair.get_second() = ptr;
            } catch (...) {
                std::allocator_traits<allocator_type>::deallocate(pair.get_first(), ptr, 1);
                throw;
            }
        }

        template <type_traits::other_trans::enable_if_t<type_traits::type_properties::is_default_constructible_v<Ty>, int> = 0>
        RAINY_CONSTEXPR20 indirect(defered_init_t) noexcept : pair(allocator_type{}, nullptr) {
        }

        template <typename... Args,
                  type_traits::other_trans::enable_if_t<type_traits::type_properties::is_constructible_v<Ty, Args...>, int> = 0>
        RAINY_CONSTEXPR20 indirect(std::in_place_t,
                                   Args &&...args) noexcept(type_traits::type_properties::is_nothrow_default_constructible_v<Ty>) :
            pair(allocator_type{}, nullptr) {
            pointer ptr = std::allocator_traits<allocator_type>::allocate(pair.get_first(), 1);
            try {
                std::allocator_traits<allocator_type>::construct(pair.get_first(), ptr, utility::forward<Args>(args)...);
                pair.get_second() = ptr;
            } catch (...) {
                std::allocator_traits<allocator_type>::deallocate(pair.get_first(), ptr, 1);
                throw;
            }
        }

        template <typename Elem, typename... Args,
                  type_traits::other_trans::enable_if_t<
                      type_traits::type_properties::is_constructible_v<Ty, std::initializer_list<Elem> &, Args...>, int> = 0>
        RAINY_CONSTEXPR20 indirect(std::in_place_t, std::initializer_list<Elem> ilist, Args &&...args) noexcept(
            type_traits::type_properties::is_nothrow_constructible_v<Ty, std::initializer_list<Elem> &, Args...>) :
            pair(allocator_type{}, nullptr) {
            pointer ptr = std::allocator_traits<allocator_type>::allocate(pair.get_first(), 1);
            try {
                std::allocator_traits<allocator_type>::construct(pair.get_first(), ptr, ilist, utility::forward<Args>(args)...);
                pair.get_second() = ptr;
            } catch (...) {
                std::allocator_traits<allocator_type>::deallocate(pair.get_first(), ptr, 1);
                throw;
            }
        }

        template <type_traits::other_trans::enable_if_t<type_traits::type_properties::is_default_constructible_v<Ty> &&
                                                            type_traits::type_properties::is_copy_constructible_v<allocator_type>,
                                                        int> = 0>
        RAINY_CONSTEXPR20 indirect(std::allocator_arg_t, const allocator_type &allocator) noexcept(
            type_traits::type_properties::is_nothrow_default_constructible_v<Ty> &&
            type_traits::type_properties::is_nothrow_copy_constructible_v<allocator_type>) : pair(allocator, nullptr) {
            pointer ptr = std::allocator_traits<allocator_type>::allocate(pair.get_first(), 1);
            try {
                std::allocator_traits<allocator_type>::construct(pair.get_first(), ptr);
                pair.get_second() = ptr;
            } catch (...) {
                std::allocator_traits<allocator_type>::deallocate(pair.get_first(), ptr, 1);
                throw;
            }
        }

        template <type_traits::other_trans::enable_if_t<type_traits::type_properties::is_default_constructible_v<Ty> &&
                                                            type_traits::type_properties::is_copy_constructible_v<allocator_type>,
                                                        int> = 0>
        RAINY_CONSTEXPR20 indirect(defered_init_t, const allocator_type &allocator) noexcept(
            type_traits::type_properties::is_nothrow_copy_constructible_v<allocator_type>) : pair(allocator, nullptr) {
        }

        template <typename... Args,
                  type_traits::other_trans::enable_if_t<type_traits::type_properties::is_constructible_v<Ty, Args...> &&
                                                            type_traits::type_properties::is_copy_constructible_v<allocator_type>,
                                                        int> = 0>
        RAINY_CONSTEXPR20 indirect(std::in_place_t, const allocator_type &allocator, Args &&...args) noexcept(
            type_traits::type_properties::is_nothrow_default_constructible_v<Ty> &&
            type_traits::type_properties::is_nothrow_copy_constructible_v<allocator_type>) : pair(allocator, nullptr) {
            pointer ptr = std::allocator_traits<allocator_type>::allocate(pair.get_first(), 1);
            try {
                std::allocator_traits<allocator_type>::construct(pair.get_first(), ptr, utility::forward<Args>(args)...);
                pair.get_second() = ptr;
            } catch (...) {
                std::allocator_traits<allocator_type>::deallocate(pair.get_first(), ptr, 1);
                throw;
            }
        }

        RAINY_CONSTEXPR20 indirect(const indirect &right) noexcept(type_traits::type_properties::is_nothrow_copy_constructible_v<Ty>) :
            pair(std::allocator_traits<allocator_type>::select_on_container_copy_construction(right.pair.get_first()), nullptr) {
            if (right.pair.get_second() != nullptr) {
                pointer ptr = std::allocator_traits<allocator_type>::allocate(pair.get_first(), 1);
                try {
                    std::allocator_traits<allocator_type>::construct(pair.get_first(), ptr, *right.pair.get_second());
                    pair.get_second() = ptr;
                } catch (...) {
                    std::allocator_traits<allocator_type>::deallocate(pair.get_first(), ptr, 1);
                    throw;
                }
            }
        }

        template <typename Elem, typename... Args,
                  type_traits::other_trans::enable_if_t<
                      type_traits::type_properties::is_constructible_v<Ty, std::initializer_list<Elem> &, Args...> &&
                          type_traits::type_properties::is_copy_constructible_v<allocator_type>,
                      int> = 0>
        RAINY_CONSTEXPR20 indirect(
            std::allocator_arg_t, const allocator_type &allocator, std::in_place_t, std::initializer_list<Elem> ilist,
            Args &&...args) noexcept(type_traits::type_properties::is_nothrow_constructible_v<Ty, std::initializer_list<Elem> &,
                                                                                              Args...> &&
                                     type_traits::type_properties::is_nothrow_copy_constructible_v<allocator_type>) :
            pair(allocator, nullptr) {
            pointer ptr = std::allocator_traits<allocator_type>::allocate(pair.get_first(), 1);
            try {
                std::allocator_traits<allocator_type>::construct(pair.get_first(), ptr, ilist, utility::forward<Args>(args)...);
                pair.get_second() = ptr;
            } catch (...) {
                std::allocator_traits<allocator_type>::deallocate(pair.get_first(), ptr, 1);
                throw;
            }
        }

        template <typename U = Ty,
                  type_traits::other_trans::enable_if_t<
                      type_traits::type_properties::is_constructible_v<Ty, U &&> &&
                          !type_traits::type_relations::is_same_v<type_traits::other_trans::decay_t<U>, indirect> &&
                          !type_traits::type_relations::is_same_v<type_traits::other_trans::decay_t<U>, std::in_place_t>,
                      int> = 0>
        RAINY_CONSTEXPR20 explicit indirect(U &&u) noexcept(type_traits::type_properties::is_nothrow_constructible_v<Ty, U &&>) :
            pair(allocator_type{}, nullptr) {
            pointer ptr = std::allocator_traits<allocator_type>::allocate(pair.get_first(), 1);
            try {
                std::allocator_traits<allocator_type>::construct(pair.get_first(), ptr, utility::forward<U>(u));
                pair.get_second() = ptr;
            } catch (...) {
                std::allocator_traits<allocator_type>::deallocate(pair.get_first(), ptr, 1);
                if constexpr (!type_traits::type_properties::is_nothrow_constructible_v<Ty, U &&>) {
                    throw;
                }
            }
        }

        template <typename U = Ty,
                  type_traits::other_trans::enable_if_t<
                      type_traits::type_properties::is_constructible_v<Ty, U &&> &&
                          type_traits::type_properties::is_copy_constructible_v<allocator_type> &&
                          !type_traits::type_relations::is_same_v<type_traits::other_trans::decay_t<U>, indirect> &&
                          !type_traits::type_relations::is_same_v<type_traits::other_trans::decay_t<U>, std::in_place_t>,
                      int> = 0>
        RAINY_CONSTEXPR20 explicit indirect(std::allocator_arg_t, const allocator_type &allocator, U &&u) noexcept(
            type_traits::type_properties::is_nothrow_constructible_v<Ty, U &&> &&
            type_traits::type_properties::is_nothrow_copy_constructible_v<allocator_type>) : pair(allocator, nullptr) {
            pointer ptr = std::allocator_traits<allocator_type>::allocate(pair.get_first(), 1);
            try {
                std::allocator_traits<allocator_type>::construct(pair.get_first(), ptr, utility::forward<U>(u));
                pair.get_second() = ptr;
            } catch (...) {
                std::allocator_traits<allocator_type>::deallocate(pair.get_first(), ptr, 1);
                throw;
            }
        }

        RAINY_CONSTEXPR20 indirect(indirect &&right) noexcept : pair(std::move(right.pair.get_first()), right.pair.get_second()) {
            right.pair.get_second() = nullptr;
        }

        template <typename Uy, typename UAlloc,
                  type_traits::other_trans::enable_if_t<type_traits::type_properties::is_constructible_v<Ty, const Uy &> &&
                                                            type_traits::type_relations::is_convertible_v<const Uy &, Ty>,
                                                        int> = 0>
        RAINY_CONSTEXPR20 indirect(const indirect<Uy, UAlloc> &right) noexcept(
            type_traits::type_properties::is_nothrow_constructible_v<Ty, const Uy &>) : pair(allocator_type{}, nullptr) {
            if (!right.empty()) {
                pointer ptr = std::allocator_traits<allocator_type>::allocate(pair.first(), 1);
                try {
                    std::allocator_traits<allocator_type>::construct(pair.first(), ptr, *right);
                    pair.get_second() = ptr;
                } catch (...) {
                    std::allocator_traits<allocator_type>::deallocate(pair.first(), ptr, 1);
                    throw;
                }
            }
        }

        template <typename Uy, typename UAlloc,
                  type_traits::other_trans::enable_if_t<type_traits::type_properties::is_constructible_v<Ty, Uy &&> &&
                                                            type_traits::type_relations::is_convertible_v<Uy &&, Ty>,
                                                        int> = 0>
        RAINY_CONSTEXPR20 indirect(indirect<Uy, UAlloc> &&right) noexcept(
            type_traits::type_properties::is_nothrow_constructible_v<Ty, Uy &&>) : pair(allocator_type{}, nullptr) {
            if (!right.empty()) {
                pointer ptr = std::allocator_traits<allocator_type>::allocate(pair.first(), 1);
                try {
                    std::allocator_traits<allocator_type>::construct(pair.first(), ptr, std::move(*right));
                    pair.get_second() = ptr;
                    right.reset();
                } catch (...) {
                    std::allocator_traits<allocator_type>::deallocate(pair.first(), ptr, 1);
                    throw;
                }
            }
        }

        RAINY_CONSTEXPR20 ~indirect() noexcept {
            reset();
        }

        RAINY_CONSTEXPR20 indirect &operator=(const indirect &right) noexcept(
            type_traits::type_properties::is_nothrow_copy_assignable_v<Ty>) {
            if (this != &right) {
                if (right.pair.get_second() == nullptr) {
                    reset();
                } else if (pair.get_second() != nullptr) {
                    *pair.get_second() = *right.pair.get_second();
                } else {
                    pointer ptr = std::allocator_traits<allocator_type>::allocate(pair.get_first(), 1);
                    try {
                        std::allocator_traits<allocator_type>::construct(pair.get_first(), ptr, *right.pair.get_second());
                        pair.get_second() = ptr;
                    } catch (...) {
                        std::allocator_traits<allocator_type>::deallocate(pair.get_first(), ptr, 1);
                        throw;
                    }
                }
            }
            return *this;
        }

        RAINY_CONSTEXPR20 indirect &operator=(indirect &&right) noexcept {
            if (this != &right) {
                reset();
                pair.get_first() = std::move(right.pair.get_first());
                pair.get_second() = right.pair.get_second();
                right.pair.get_second() = nullptr;
            }
            return *this;
        }

        template <typename U = Ty, type_traits::other_trans::enable_if_t<
                                       type_traits::type_properties::is_assignable_v<Ty &, U &&> &&
                                           !type_traits::type_relations::is_same_v<type_traits::other_trans::decay_t<U>, indirect>,
                                       int> = 0>
        RAINY_CONSTEXPR20 indirect &operator=(U &&u) noexcept(type_traits::type_properties::is_nothrow_assignable_v<Ty &, U &&>) {
            if (pair.get_second() != nullptr) {
                *pair.get_second() = utility::forward<U>(u);
            } else {
                pointer ptr = std::allocator_traits<allocator_type>::allocate(pair.get_first(), 1);
                try {
                    std::allocator_traits<allocator_type>::construct(pair.get_first(), ptr, utility::forward<U>(u));
                    pair.get_second() = ptr;
                } catch (...) {
                    std::allocator_traits<allocator_type>::deallocate(pair.get_first(), ptr, 1);
                    throw;
                }
            }
            return *this;
        }

        template <typename Uy, typename UAlloc,
                  type_traits::other_trans::enable_if_t<type_traits::type_properties::is_assignable_v<Ty &, const Uy &>, int> = 0>
        RAINY_CONSTEXPR20 indirect &operator=(const indirect<Uy, UAlloc> &right) noexcept(
            type_traits::type_properties::is_nothrow_assignable_v<Ty &, const Uy &>) {
            if (right.empty()) {
                reset();
            } else if (pair.get_second() != nullptr) {
                *pair.get_second() = *right;
            } else {
                pointer ptr = std::allocator_traits<allocator_type>::allocate(pair.first(), 1);
                try {
                    std::allocator_traits<allocator_type>::construct(pair.first(), ptr, *right);
                    pair.get_second() = ptr;
                } catch (...) {
                    std::allocator_traits<allocator_type>::deallocate(pair.first(), ptr, 1);
                    throw;
                }
            }
            return *this;
        }

        template <typename Uy, typename UAlloc,
                  type_traits::other_trans::enable_if_t<type_traits::type_properties::is_assignable_v<Ty &, Uy &&>, int> = 0>
        RAINY_CONSTEXPR20 indirect &operator=(indirect<Uy, UAlloc> &&right) noexcept(
            type_traits::type_properties::is_nothrow_assignable_v<Ty &, Uy &&>) {
            if (right.empty()) {
                reset();
            } else if (pair.get_second() != nullptr) {
                *pair.get_second() = std::move(*right);
                right.reset();
            } else {
                pointer ptr = std::allocator_traits<allocator_type>::allocate(pair.first(), 1);
                try {
                    std::allocator_traits<allocator_type>::construct(pair.first(), ptr, std::move(*right));
                    pair.get_second() = ptr;
                    right.reset();
                } catch (...) {
                    std::allocator_traits<allocator_type>::deallocate(pair.first(), ptr, 1);
                    throw;
                }
            }
            return *this;
        }

        template <typename... Args,
                  type_traits::other_trans::enable_if_t<type_traits::type_properties::is_constructible_v<Ty, Args...>, int> = 0>
        RAINY_CONSTEXPR20 reference
        emplace(Args &&...args) noexcept(type_traits::type_properties::is_nothrow_constructible_v<Ty, Args...>) {
            reset();
            pointer ptr = std::allocator_traits<allocator_type>::allocate(pair.get_first(), 1);
            try {
                std::allocator_traits<allocator_type>::construct(pair.get_first(), ptr, utility::forward<Args>(args)...);
                pair.get_second() = ptr;
            } catch (...) {
                std::allocator_traits<allocator_type>::deallocate(pair.get_first(), ptr, 1);
                throw;
            }
            return *pair.get_second();
        }

        template <typename Elem, typename... Args,
                  type_traits::other_trans::enable_if_t<
                      type_traits::type_properties::is_constructible_v<Ty, std::initializer_list<Elem> &, Args...>, int> = 0>
        RAINY_CONSTEXPR20 reference emplace(std::initializer_list<Elem> ilist, Args &&...args) noexcept(
            type_traits::type_properties::is_nothrow_constructible_v<Ty, std::initializer_list<Elem> &, Args...>) {
            reset();
            pointer ptr = std::allocator_traits<allocator_type>::allocate(pair.get_first(), 1);
            try {
                std::allocator_traits<allocator_type>::construct(pair.get_first(), ptr, ilist, utility::forward<Args>(args)...);
                pair.get_second() = ptr;
            } catch (...) {
                std::allocator_traits<allocator_type>::deallocate(pair.get_first(), ptr, 1);
                throw;
            }
            return *pair.get_second();
        }

        template <typename... Args,
                  type_traits::other_trans::enable_if_t<type_traits::type_properties::is_constructible_v<Ty, Args...> &&
                                                            type_traits::type_properties::is_copy_constructible_v<allocator_type>,
                                                        int> = 0>
        RAINY_CONSTEXPR20 reference emplace(const allocator_type &allocator, Args &&...args) noexcept(
            type_traits::type_properties::is_nothrow_constructible_v<Ty, Args...> &&
            type_traits::type_properties::is_nothrow_copy_constructible_v<allocator_type>) {
            reset();
            pair.get_first() = allocator;
            pointer ptr = std::allocator_traits<allocator_type>::allocate(pair.get_first(), 1);
            try {
                std::allocator_traits<allocator_type>::construct(pair.get_first(), ptr, utility::forward<Args>(args)...);
                pair.get_second() = ptr;
            } catch (...) {
                std::allocator_traits<allocator_type>::deallocate(pair.get_first(), ptr, 1);
                throw;
            }
            return *pair.get_second();
        }

        RAINY_CONSTEXPR20 void reset() noexcept {
            if (pair.get_second() != nullptr) {
                std::allocator_traits<allocator_type>::destroy(pair.get_first(), pair.get_second());
                std::allocator_traits<allocator_type>::deallocate(pair.get_first(), pair.get_second(), 1);
                pair.get_second() = nullptr;
            }
        }

        RAINY_CONSTEXPR20 pointer operator->() noexcept {
            return pair.get_second();
        }

        RAINY_CONSTEXPR20 const_pointer operator->() const noexcept {
            return pair.get_second();
        }

        RAINY_CONSTEXPR20 reference operator*() noexcept {
            return *pair.get_second();
        }

        RAINY_CONSTEXPR20 const_reference operator*() const noexcept {
            return *pair.get_second();
        }

        RAINY_CONSTEXPR20 bool empty() const noexcept {
            return pair.get_second() == nullptr;
        }

        RAINY_CONSTEXPR20 allocator_type get_allocator() const noexcept {
            return pair.get_first();
        }

        RAINY_CONSTEXPR20 pointer get() noexcept {
            return pair.get_second();
        }

        RAINY_CONSTEXPR20 const_pointer get() const noexcept {
            return pair.get_second();
        }

        RAINY_CONSTEXPR20 pointer begin() noexcept {
            return empty() ? nullptr : get();
        }

        RAINY_CONSTEXPR20 const_pointer begin() const noexcept {
            return empty() ? nullptr : get();
        }

        RAINY_CONSTEXPR20 const_pointer cbegin() const noexcept {
            return empty() ? nullptr : get();
        }

        RAINY_CONSTEXPR20 pointer end() noexcept {
            return empty() ? nullptr : get() + 1;
        }

        RAINY_CONSTEXPR20 const_pointer end() const noexcept {
            return empty() ? nullptr : get() + 1;
        }

        RAINY_CONSTEXPR20 const_pointer cend() const noexcept {
            return empty() ? nullptr : get() + 1;
        }

        RAINY_CONSTEXPR20 void swap(indirect &right) noexcept {
            if (this != &right) {
                pointer temp_ptr = pair.get_second();
                pair.get_second() = right.pair.get_second();
                right.pair.get_second() = temp_ptr;
                allocator_type temp_alloc = std::move(pair.get_first());
                pair.get_first() = std::move(right.pair.get_first());
                right.pair.get_first() = std::move(temp_alloc);
            }
        }

        RAINY_CONSTEXPR20 bool valueless_after_move() const noexcept {
            return empty();
        }

    private:
        utility::compressed_pair<Alloc, Ty *> pair;
    };

    template <typename Value>
    indirect(Value) -> indirect<Value>;

    template <typename Allocator, typename Value>
    indirect(std::allocator_arg_t, Allocator, Value)
        -> indirect<Value, typename std::allocator_traits<Allocator>::template rebind_alloc<Value>>;
}

namespace rainy::foundation::container {
    template <typename Ty, typename Alloc, typename Uy, typename UAlloc>
    RAINY_CONSTEXPR20 bool operator==(const indirect<Ty, Alloc> &left,
                                      const indirect<Uy, UAlloc> &right) noexcept(noexcept(*left == *right)) {
        if (left.empty() != right.empty()) {
            return false;
        }
        if (left.empty()) {
            return true;
        }
        return *left == *right;
    }

    template <typename Ty, typename Alloc, typename Uy, typename UAlloc>
    RAINY_CONSTEXPR20 bool operator!=(const indirect<Ty, Alloc> &left,
                                      const indirect<Uy, UAlloc> &right) noexcept(noexcept(left == right)) {
        return !(left == right);
    }

    template <typename Ty, typename Alloc, typename Uy, typename UAlloc>
    RAINY_CONSTEXPR20 bool operator<(const indirect<Ty, Alloc> &left,
                                     const indirect<Uy, UAlloc> &right) noexcept(noexcept(*left < *right)) {
        if (right.empty()) {
            return false;
        }
        if (left.empty()) {
            return true;
        }
        return *left < *right;
    }

    template <typename Ty, typename Alloc, typename Uy, typename UAlloc>
    RAINY_CONSTEXPR20 bool operator<=(const indirect<Ty, Alloc> &left,
                                      const indirect<Uy, UAlloc> &right) noexcept(noexcept(right < left)) {
        return !(right < left);
    }

    template <typename Ty, typename Alloc, typename Uy, typename UAlloc>
    RAINY_CONSTEXPR20 bool operator>(const indirect<Ty, Alloc> &left,
                                     const indirect<Uy, UAlloc> &right) noexcept(noexcept(right < left)) {
        return right < left;
    }

    template <typename Ty, typename Alloc, typename Uy, typename UAlloc>
    RAINY_CONSTEXPR20 bool operator>=(const indirect<Ty, Alloc> &left,
                                      const indirect<Uy, UAlloc> &right) noexcept(noexcept(left < right)) {
        return !(left < right);
    }

    template <typename Ty, typename Alloc>
    RAINY_CONSTEXPR20 bool operator==(const indirect<Ty, Alloc> &left, std::nullptr_t) noexcept {
        return left.empty();
    }

    template <typename Ty, typename Alloc>
    RAINY_CONSTEXPR20 bool operator==(std::nullptr_t, const indirect<Ty, Alloc> &right) noexcept {
        return right.empty();
    }

    template <typename Ty, typename Alloc>
    RAINY_CONSTEXPR20 bool operator!=(const indirect<Ty, Alloc> &left, std::nullptr_t) noexcept {
        return !left.empty();
    }

    template <typename Ty, typename Alloc>
    RAINY_CONSTEXPR20 bool operator!=(std::nullptr_t, const indirect<Ty, Alloc> &right) noexcept {
        return !right.empty();
    }

    template <typename Ty, typename Alloc>
    RAINY_CONSTEXPR20 bool operator<(const indirect<Ty, Alloc> &, std::nullptr_t) noexcept {
        return false;
    }

    template <typename Ty, typename Alloc>
    RAINY_CONSTEXPR20 bool operator<(std::nullptr_t, const indirect<Ty, Alloc> &right) noexcept {
        return !right.empty();
    }

    template <typename Ty, typename Alloc>
    RAINY_CONSTEXPR20 bool operator<=(const indirect<Ty, Alloc> &left, std::nullptr_t) noexcept {
        return left.empty();
    }

    template <typename Ty, typename Alloc>
    RAINY_CONSTEXPR20 bool operator<=(std::nullptr_t, const indirect<Ty, Alloc> &) noexcept {
        return true;
    }

    template <typename Ty, typename Alloc>
    RAINY_CONSTEXPR20 bool operator>(const indirect<Ty, Alloc> &left, std::nullptr_t) noexcept {
        return !left.empty();
    }

    template <typename Ty, typename Alloc>
    RAINY_CONSTEXPR20 bool operator>(std::nullptr_t, const indirect<Ty, Alloc> &) noexcept {
        return false;
    }

    template <typename Ty, typename Alloc>
    RAINY_CONSTEXPR20 bool operator>=(const indirect<Ty, Alloc> &, std::nullptr_t) noexcept {
        return true;
    }

    template <typename Ty, typename Alloc>
    RAINY_CONSTEXPR20 bool operator>=(std::nullptr_t, const indirect<Ty, Alloc> &right) noexcept {
        return right.empty();
    }

    template <typename Ty, typename Alloc, typename Uy>
    RAINY_CONSTEXPR20 bool operator==(const indirect<Ty, Alloc> &left, const Uy &right) noexcept(noexcept(*left == right)) {
        return !left.empty() && *left == right;
    }

    template <typename Ty, typename Alloc, typename Uy>
    RAINY_CONSTEXPR20 bool operator==(const Uy &left, const indirect<Ty, Alloc> &right) noexcept(noexcept(left == *right)) {
        return !right.empty() && left == *right;
    }

    template <typename Ty, typename Alloc, typename Uy>
    RAINY_CONSTEXPR20 bool operator!=(const indirect<Ty, Alloc> &left, const Uy &right) noexcept(noexcept(left == right)) {
        return !(left == right);
    }

    template <typename Ty, typename Alloc, typename Uy>
    RAINY_CONSTEXPR20 bool operator!=(const Uy &left, const indirect<Ty, Alloc> &right) noexcept(noexcept(left == right)) {
        return !(left == right);
    }

    template <typename Ty, typename Alloc, typename Uy>
    RAINY_CONSTEXPR20 bool operator<(const indirect<Ty, Alloc> &left, const Uy &right) noexcept(noexcept(*left < right)) {
        return left.empty() || *left < right;
    }

    template <typename Ty, typename Alloc, typename Uy>
    RAINY_CONSTEXPR20 bool operator<(const Uy &left, const indirect<Ty, Alloc> &right) noexcept(noexcept(left < *right)) {
        return !right.empty() && left < *right;
    }

    template <typename Ty, typename Alloc, typename Uy>
    RAINY_CONSTEXPR20 bool operator<=(const indirect<Ty, Alloc> &left, const Uy &right) noexcept(noexcept(right < left)) {
        return !(right < left);
    }

    template <typename Ty, typename Alloc, typename Uy>
    RAINY_CONSTEXPR20 bool operator<=(const Uy &left, const indirect<Ty, Alloc> &right) noexcept(noexcept(right < left)) {
        return !(right < left);
    }

    template <typename Ty, typename Alloc, typename Uy>
    RAINY_CONSTEXPR20 bool operator>(const indirect<Ty, Alloc> &left, const Uy &right) noexcept(noexcept(right < left)) {
        return right < left;
    }

    template <typename Ty, typename Alloc, typename Uy>
    RAINY_CONSTEXPR20 bool operator>(const Uy &left, const indirect<Ty, Alloc> &right) noexcept(noexcept(right < left)) {
        return right < left;
    }

    template <typename Ty, typename Alloc, typename Uy>
    RAINY_CONSTEXPR20 bool operator>=(const indirect<Ty, Alloc> &left, const Uy &right) noexcept(noexcept(left < right)) {
        return !(left < right);
    }

    template <typename Ty, typename Alloc, typename Uy>
    RAINY_CONSTEXPR20 bool operator>=(const Uy &left, const indirect<Ty, Alloc> &right) noexcept(noexcept(left < right)) {
        return !(left < right);
    }

    template <typename Ty, typename Alloc>
    RAINY_CONSTEXPR20 void swap(indirect<Ty, Alloc> &left, indirect<Ty, Alloc> &right) noexcept {
        left.swap(right);
    }

    template <typename Ty, typename... Args>
    RAINY_CONSTEXPR20 indirect<Ty> make_indirect(Args &&...args) noexcept(
        type_traits::type_properties::is_nothrow_constructible_v<Ty, Args...>) {
        return indirect<Ty>(std::in_place, utility::forward<Args>(args)...);
    }

    template <typename Ty, typename Alloc, typename... Args>
    RAINY_CONSTEXPR20 indirect<Ty, Alloc> make_indirect(const Alloc &alloc, Args &&...args) noexcept(
        type_traits::type_properties::is_nothrow_constructible_v<Ty, Args...> &&
        type_traits::type_properties::is_nothrow_copy_constructible_v<Alloc>) {
        return indirect<Ty, Alloc>(std::in_place, alloc, utility::forward<Args>(args)...);
    }

    template <typename Ty, typename Elem, typename... Args>
    RAINY_CONSTEXPR20 indirect<Ty> make_indirect(std::initializer_list<Elem> ilist, Args &&...args) noexcept(
        type_traits::type_properties::is_nothrow_constructible_v<Ty, std::initializer_list<Elem> &, Args...>) {
        return indirect<Ty>(std::in_place, ilist, utility::forward<Args>(args)...);
    }
}

#endif
