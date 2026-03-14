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
#ifndef RAINY_FOUNDATION_CONTAINER_POLYMORPHIC_HPP
#define RAINY_FOUNDATION_CONTAINER_POLYMORPHIC_HPP
#include <rainy/core/core.hpp>

namespace rainy::foundation::container {
    template <typename Ty, typename Alloc = std::allocator<Ty>>
    class polymorphic {
    public:
        using value_type = Ty;
        using allocator_type = Alloc;
        using pointer = Ty *;
        using const_pointer = const Ty *;
        using reference = Ty&;
        using const_reference = const Ty&;

        static_assert(type_traits::composite_types::is_object_v<Ty>, "Ty must be an object type");
        static_assert(!type_traits::composite_types::is_reference_v<Ty>, "Ty cannot be a reference type");
        static_assert(!(type_traits::type_properties::is_const_v<Ty> || type_traits::type_properties::is_volatile_v<Ty>),
                      "Ty cannot be const/volatile qualified");
        static_assert(type_traits::type_properties::is_polymorphic_v<Ty>, "Ty must be a polymorphic type");

        template <type_traits::other_trans::enable_if_t<type_traits::type_properties::is_default_constructible_v<Ty>, int> = 0>
        RAINY_CONSTEXPR20 explicit polymorphic() noexcept(type_traits::type_properties::is_nothrow_default_constructible_v<Ty>) :
            pair(allocator_type{}, nullptr) {
            construct_default();
        }

        template <type_traits::other_trans::enable_if_t<type_traits::type_properties::is_default_constructible_v<Ty>, int> = 0>
        RAINY_CONSTEXPR20 explicit polymorphic(std::allocator_arg_t, const Alloc &a) noexcept(
            type_traits::type_properties::is_nothrow_default_constructible_v<Ty> &&
            type_traits::type_properties::is_nothrow_copy_constructible_v<Alloc>) : pair(a, nullptr) {
            construct_default();
        }

        RAINY_CONSTEXPR20 polymorphic(const polymorphic &other) :
            pair(std::allocator_traits<allocator_type>::select_on_container_copy_construction(other.pair.get_first()), nullptr) {
            if (other.pair.get_second() != nullptr) {
                copy_from(other);
            }
        }

        RAINY_CONSTEXPR20 polymorphic(std::allocator_arg_t, const Alloc &a, const polymorphic &other) : pair(a, nullptr) {
            if (other.pair.get_second() != nullptr) {
                copy_from(other);
            }
        }

        RAINY_CONSTEXPR20 polymorphic(polymorphic &&other) noexcept :
            pair(utility::move(other.pair.get_first()), other.pair.get_second()) {
            other.pair.get_second() = nullptr;
        }

        RAINY_CONSTEXPR20 polymorphic(std::allocator_arg_t, const Alloc &a,
                                      polymorphic &&other) noexcept(std::allocator_traits<Alloc>::is_always_equal::value) :
            pair(a, nullptr) {
            if (pair.get_first() == other.pair.get_first()) {
                pair.get_second() = other.pair.get_second();
                other.pair.get_second() = nullptr;
            } else if (other.pair.get_second() != nullptr) {
                copy_from(other);
                other.reset();
            }
        }

        template <typename UTy = Ty,
                  type_traits::other_trans::enable_if_t<
                      type_traits::type_relations::is_base_of_v<Ty, type_traits::other_trans::decay_t<UTy>> &&
                          !type_traits::type_relations::is_same_v<type_traits::other_trans::decay_t<UTy>, polymorphic>,
                      int> = 0>
        RAINY_CONSTEXPR20 explicit polymorphic(UTy &&object) : pair(allocator_type{}, nullptr) { // NOLINT
            using DecayU = type_traits::other_trans::decay_t<UTy>;
            construct_from_value<DecayU>(utility::forward<UTy>(object));
        }

        template <typename UTy = Ty,
                  type_traits::other_trans::enable_if_t<
                      type_traits::type_relations::is_base_of_v<Ty, type_traits::other_trans::decay_t<UTy>> &&
                          !type_traits::type_relations::is_same_v<type_traits::other_trans::decay_t<UTy>, polymorphic>,
                      int> = 0>
        RAINY_CONSTEXPR20 explicit polymorphic(std::allocator_arg_t, const Alloc &a, UTy &&object) : pair(a, nullptr) {
            using DecayU = type_traits::other_trans::decay_t<UTy>;
            construct_from_value<DecayU>(utility::forward<UTy>(object));
        }

        template <typename UTy, typename... Ts,
                  type_traits::other_trans::enable_if_t<type_traits::type_relations::is_base_of_v<Ty, UTy> &&
                                                            type_traits::type_properties::is_constructible_v<UTy, Ts...>,
                                                        int> = 0>
        RAINY_CONSTEXPR20 explicit polymorphic(std::in_place_type_t<UTy>, Ts &&...ts) : pair(allocator_type{}, nullptr) {
            construct_inplace<UTy>(utility::forward<Ts>(ts)...);
        }

        template <typename UTy, typename... Ts,
                  type_traits::other_trans::enable_if_t<type_traits::type_relations::is_base_of_v<Ty, UTy> &&
                                                            type_traits::type_properties::is_constructible_v<UTy, Ts...>,
                                                        int> = 0>
        RAINY_CONSTEXPR20 explicit polymorphic(std::allocator_arg_t, const Alloc &a, std::in_place_type_t<UTy>, Ts &&...ts) :
            pair(a, nullptr) {
            construct_inplace<UTy>(utility::forward<Ts>(ts)...);
        }

        template <typename UTy, typename I, typename... Us,
                  type_traits::other_trans::enable_if_t<
                      type_traits::type_relations::is_base_of_v<Ty, UTy> &&
                          type_traits::type_properties::is_constructible_v<UTy, std::initializer_list<I> &, Us...>,
                      int> = 0>
        RAINY_CONSTEXPR20 explicit polymorphic(std::in_place_type_t<UTy>, std::initializer_list<I> ilist, Us &&...us) :
            pair(allocator_type{}, nullptr) {
            construct_inplace<UTy>(ilist, utility::forward<Us>(us)...);
        }

        template <typename UTy, typename I, typename... Us,
                  type_traits::other_trans::enable_if_t<
                      type_traits::type_relations::is_base_of_v<Ty, UTy> &&
                          type_traits::type_properties::is_constructible_v<UTy, std::initializer_list<I> &, Us...>,
                      int> = 0>
        RAINY_CONSTEXPR20 explicit polymorphic(std::allocator_arg_t, const Alloc &a, std::in_place_type_t<UTy>,
                                               std::initializer_list<I> ilist, Us &&...us) : pair(a, nullptr) {
            construct_inplace<UTy>(ilist, utility::forward<Us>(us)...);
        }

        RAINY_CONSTEXPR20 ~polymorphic() noexcept {
            reset();
        }

        RAINY_CONSTEXPR20 polymorphic &operator=(const polymorphic &other) {
            if (this != &other) {
                reset();
                if (other.pair.get_second() != nullptr) {
                    copy_from(other);
                }
            }
            return *this;
        }

        RAINY_CONSTEXPR20 polymorphic &operator=(polymorphic &&other) noexcept(std::allocator_traits<Alloc>::is_always_equal::value) {
            if (this != &other) {
                reset();
                if constexpr (std::allocator_traits<Alloc>::is_always_equal::value) {
                    pair.get_first() = utility::move(other.pair.get_first());
                    pair.get_second() = other.pair.get_second();
                    other.pair.get_second() = nullptr;
                } else {
                    if (pair.get_first() == other.pair.get_first()) {
                        pair.get_second() = other.pair.get_second();
                        other.pair.get_second() = nullptr;
                    } else if (other.pair.get_second() != nullptr) {
                        copy_from(other);
                        other.reset();
                    }
                }
            }
            return *this;
        }

        RAINY_CONSTEXPR20 const_reference operator*() const noexcept {
            auto* block = static_cast<control_block_base*>(pair.get_second());
            return *block->get_value_ptr();
        }

        RAINY_CONSTEXPR20 reference operator*() noexcept {
            auto* block = static_cast<control_block_base*>(pair.get_second());
            return *block->get_value_ptr();
        }

        RAINY_CONSTEXPR20 const_pointer operator->() const noexcept {
            auto* block = static_cast<const control_block_base*>(pair.get_second());
            return static_cast<const_pointer>(block->get_value_ptr());
        }

        RAINY_CONSTEXPR20 pointer operator->() noexcept {
            auto* block = static_cast<control_block_base*>(pair.get_second());
            return static_cast<pointer>(block->get_value_ptr());
        }

        RAINY_NODISCARD RAINY_CONSTEXPR20 bool valueless_after_move() const noexcept {
            return pair.get_second() == nullptr;
        }

        RAINY_CONSTEXPR20 allocator_type get_allocator() const noexcept {
            return pair.get_first();
        }

        RAINY_CONSTEXPR20 void swap(polymorphic &other) noexcept(std::allocator_traits<Alloc>::is_always_equal::value) {
            if (this != &other) {
                void *temp_ptr = pair.get_second();
                pair.get_second() = other.pair.get_second();
                other.pair.get_second() = temp_ptr;

                if constexpr (!std::allocator_traits<Alloc>::is_always_equal::value) {
                    allocator_type temp_alloc = utility::move(pair.get_first());
                    pair.get_first() = utility::move(other.pair.get_first());
                    other.pair.get_first() = utility::move(temp_alloc);
                }
            }
        }

        friend RAINY_CONSTEXPR20 void swap(polymorphic &lhs, polymorphic &rhs) noexcept(noexcept(lhs.swap(rhs))) {
            lhs.swap(rhs);
        }

    private:
        struct control_block_base {
            virtual ~control_block_base() = default;
            virtual void destroy(allocator_type &alloc) noexcept = 0;
            virtual control_block_base *clone(allocator_type &alloc) const = 0;
            virtual Ty *get_value_ptr() noexcept = 0;
            virtual const Ty *get_value_ptr() const noexcept = 0;
        };

        template <typename U>
        struct control_block : control_block_base {
            template <typename... Args>
            control_block(Args &&...args) : value(utility::forward<Args>(args)...) { // NOLINT
            }

            void destroy(allocator_type &alloc) noexcept override {
                using BlockAlloc = typename std::allocator_traits<allocator_type>::template rebind_alloc<control_block>;
                BlockAlloc block_alloc(alloc);
                auto *derived_this = static_cast<control_block *>(this);
                std::allocator_traits<BlockAlloc>::destroy(block_alloc, derived_this);
                std::allocator_traits<BlockAlloc>::deallocate(block_alloc, derived_this, 1);
            }

            control_block_base *clone(allocator_type &alloc) const override {
                using BlockAlloc = typename std::allocator_traits<allocator_type>::template rebind_alloc<control_block>;
                BlockAlloc block_alloc(alloc);
                auto *ptr = std::allocator_traits<BlockAlloc>::allocate(block_alloc, 1);
                try {
                    std::allocator_traits<BlockAlloc>::construct(block_alloc, ptr, value);
                    return ptr;
                } catch (...) {
                    std::allocator_traits<BlockAlloc>::deallocate(block_alloc, ptr, 1);
                    throw;
                }
            }

            Ty *get_value_ptr() noexcept override {
                return &value;
            }

            const Ty *get_value_ptr() const noexcept override {
                return &value;
            }

            U value;
        };

        void construct_default() {
            using BlockAlloc = typename std::allocator_traits<allocator_type>::template rebind_alloc<control_block<Ty>>; // NOLINT
            BlockAlloc block_alloc(pair.get_first());
            auto *ptr = std::allocator_traits<BlockAlloc>::allocate(block_alloc, 1);
            try {
                std::allocator_traits<BlockAlloc>::construct(block_alloc, ptr);
                pair.get_second() = ptr;
            } catch (...) {
                std::allocator_traits<BlockAlloc>::deallocate(block_alloc, ptr, 1);
                throw;
            }
        }

        template <typename UTy, typename... Args>
        void construct_inplace(Args &&...args) {
            using BlockAlloc = typename std::allocator_traits<allocator_type>::template rebind_alloc<control_block<UTy>>; // NOLINT
            BlockAlloc block_alloc(pair.get_first());
            auto *ptr = std::allocator_traits<BlockAlloc>::allocate(block_alloc, 1);
            try {
                std::allocator_traits<BlockAlloc>::construct(block_alloc, ptr, utility::forward<Args>(args)...);
                pair.get_second() = ptr;
            } catch (...) {
                std::allocator_traits<BlockAlloc>::deallocate(block_alloc, ptr, 1);
                throw;
            }
        }

        template <typename UTy, typename V>
        void construct_from_value(V &&v) {
            using BlockAlloc = typename std::allocator_traits<allocator_type>::template rebind_alloc<control_block<UTy>>; // NOLINT
            BlockAlloc block_alloc(pair.get_first());
            auto *ptr = std::allocator_traits<BlockAlloc>::allocate(block_alloc, 1);
            try {
                std::allocator_traits<BlockAlloc>::construct(block_alloc, ptr, utility::forward<V>(v));
                pair.get_second() = ptr;
            } catch (...) {
                std::allocator_traits<BlockAlloc>::deallocate(block_alloc, ptr, 1);
                throw;
            }
        }

        void copy_from(const polymorphic &other) {
            auto *block = static_cast<control_block_base *>(other.pair.get_second());
            pair.get_second() = block->clone(pair.get_first());
        }

        void reset() noexcept {
            if (pair.get_second() != nullptr) {
                auto *block = static_cast<control_block_base *>(pair.get_second());
                block->destroy(pair.get_first());
                pair.get_second() = nullptr;
            }
        }

        compressed_pair<Alloc, void *> pair;
    };
}

#endif
