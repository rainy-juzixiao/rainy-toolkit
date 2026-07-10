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
#ifndef RAINY_CORE_MEMORY_NEBULA_PTR
#define RAINY_CORE_MEMORY_NEBULA_PTR
#include <rainy/core/platform.hpp>
#include <rainy/core/type_traits.hpp>
#include <rainy/core/container/compressed_pair.hpp>
#include <rainy/core/algorithm.hpp>
#include <rainy/core/diagnostics/exceptions.hpp>
#include <rainy/core/memory/allocator.hpp>

namespace rainy::core::memory {
    template <typename Ty>
    struct default_deleter {
        constexpr default_deleter() noexcept = default;

        constexpr default_deleter(std::in_place_t) noexcept {
        }

        template <typename U,
                  typename = type_traits::other_trans::enable_if_t<type_traits::type_relations::is_convertible_v<U *, Ty *>>>
        constexpr explicit default_deleter(const default_deleter<U> &) noexcept {
        }

        RAINY_CONSTEXPR20 rain_fn operator()(const Ty *resource) const noexcept -> void {
            static_assert(0 < sizeof(Ty), "can't delete an incomplete type");
            delete resource;
        }
    };

    template <typename Ty>
    struct default_deleter<Ty[]> {
        constexpr default_deleter() noexcept = default;

        constexpr default_deleter(utility::placeholder_t) noexcept {
        }

        template <typename U,
                  typename = type_traits::other_trans::enable_if_t<
                      type_traits::type_relations::is_convertible_v<U (*)[], Ty (*)[]>>>
        constexpr explicit default_deleter(const default_deleter<U[]> &) noexcept {
        }

        template <typename U,
                  typename = type_traits::other_trans::enable_if_t<type_traits::type_relations::is_convertible_v<U *, Ty *>>>
        constexpr explicit default_deleter(const default_deleter<U> &) noexcept {
        }

        RAINY_CONSTEXPR20 rain_fn operator()(const Ty *resource) const noexcept -> void {
            static_assert(0 < sizeof(Ty), "can't delete an incomplete type");
            delete[] resource;
        }
    };
}

namespace rainy::core::memory::implements {
    template <typename Ty, typename Dx>
    class nebula_ptr_base {
    public:
        using element_type = Ty;
        using pointer = element_type *;
        using const_pointer = const element_type *;
        using deleter_type = Dx;

        template <typename, typename>
        friend class nebula_ptr_base;

        template <typename, typename>
        friend class nebula_ptr;

        constexpr nebula_ptr_base() noexcept : pair(Dx{}, nullptr) {};

        constexpr nebula_ptr_base(std::nullptr_t) noexcept : pair(Dx{}, nullptr) { // NOLINT
        }

        RAINY_CONSTEXPR20 nebula_ptr_base(element_type *pointer) noexcept : pair(Dx{}, pointer) { // NOLINT
        }

        template <
            type_traits::other_trans::enable_if_t<type_traits::properties::is_copy_constructible_v<deleter_type>, int> = 0>
        RAINY_CONSTEXPR20 nebula_ptr_base(element_type *pointer, deleter_type deleter) : pair(deleter, pointer) {
        }

        template <typename UTy,
                  type_traits::other_trans::enable_if_t<type_traits::type_relations::is_convertible_v<UTy *, pointer>, int> = 0>
        RAINY_CONSTEXPR20 nebula_ptr_base(UTy *pointer) noexcept : pair(Dx{}, pointer) { // NOLINT
        }

        template <typename UTy,
                  type_traits::other_trans::enable_if_t<type_traits::type_relations::is_convertible_v<UTy *, pointer> &&
                                                            std::is_copy_constructible_v<deleter_type>,
                                                        int> = 0>
        RAINY_CONSTEXPR20 nebula_ptr_base(UTy *pointer, deleter_type deleter) : pair(deleter, pointer) {
        }

        nebula_ptr_base(const nebula_ptr_base &) = delete;
        nebula_ptr_base(nebula_ptr_base &&right) noexcept : pair{utility::exchange(right.pair, {})} {
        }

        template <typename UTy, typename UDx,
                  type_traits::other_trans::enable_if_t<type_traits::type_relations::is_convertible_v<UTy *, pointer> &&
                                                            type_traits::properties::is_constructible_v<Dx, UDx &&>,
                                                        int> = 0>
        RAINY_CONSTEXPR20 nebula_ptr_base(nebula_ptr_base<UTy, UDx> &&right) noexcept :
            pair(Dx(utility::move(right.pair.get_first())), right.pair.second) {
            right.pair.second = nullptr;
        }

        template <typename Dx2 = Dx,
                  type_traits::other_trans::enable_if_t<
                      type_traits::logical_traits::conjunction_v<std::is_reference<Dx2>,
                                                                  std::is_constructible<Dx2, std::remove_reference_t<Dx2>>>,
                      int> = 0>
        nebula_ptr_base(pointer, std::remove_reference_t<Dx> &&) = delete;

        RAINY_CONSTEXPR20 ~nebula_ptr_base() {
            reset(nullptr);
        };

        nebula_ptr_base &operator=(const nebula_ptr_base &) = delete;

        RAINY_NODISCARD constexpr bool empty() const noexcept {
            return !static_cast<bool>(pair.second);
        }

        RAINY_NODISCARD constexpr operator bool() const noexcept {
            return static_cast<bool>(pair.second);
        }

        RAINY_CONSTEXPR20 void reset(element_type *pointer = nullptr) {
            if (!empty()) {
                pair.get_first()(pair.second);
            }
            pair.second = pointer;
        }

        RAINY_CONSTEXPR20 nebula_ptr_base &operator=(std::nullptr_t) {
            reset(nullptr);
            return *this;
        }

        RAINY_CONSTEXPR20 nebula_ptr_base &operator=(element_type *pointer) {
            reset(pointer);
            return *this;
        }

        RAINY_CONSTEXPR20 nebula_ptr_base &operator=(nebula_ptr_base &&right) noexcept {
            this->reset();
            pair = right.pair;
            right.pair.second = nullptr;
            return *this;
        }

        template <typename UTy, typename UDx,
                  type_traits::other_trans::enable_if_t<type_traits::type_relations::is_convertible_v<UTy *, pointer> &&
                                                            type_traits::properties::is_constructible_v<Dx, UDx &&>,
                                                        int> = 0>
        RAINY_CONSTEXPR20 nebula_ptr_base &operator=(nebula_ptr_base<UTy, UDx> &&right) noexcept {
            this->reset();
            pair = right.pair;
            right.pair.second = nullptr;
            return *this;
        }

        template <typename Reint, typename Dx_ = default_deleter<Reint>>
        nebula_ptr_base<Reint, Dx_> &reinterpret() noexcept {
            return reinterpret_cast<nebula_ptr_base<Reint, Dx_> &>(*this);
        }

        template <typename Reint, typename Dx_ = default_deleter<Reint>>
        const nebula_ptr_base<Reint, Dx_> &reinterpret() const noexcept {
            return reinterpret_cast<nebula_ptr_base<Reint, Dx_> &>(*this);
        }

        template <typename Cast, typename Dx_ = default_deleter<Cast>,
                  type_traits::other_trans::enable_if_t<type_traits::type_relations::is_convertible_v<Ty, Cast>, int> = 0>
        constexpr nebula_ptr_base<Cast, Dx_> &cast() noexcept {
            return static_cast<nebula_ptr_base<Cast, Dx_> &>(*this);
        }

        template <typename Cast, typename Dx_ = default_deleter<Cast>,
                  type_traits::other_trans::enable_if_t<type_traits::type_relations::is_convertible_v<Ty, Cast>, int> = 0>
        constexpr const nebula_ptr_base<Cast, Dx_> &cast() const noexcept {
            return static_cast<const nebula_ptr_base<Cast, Dx> &>(*this);
        }

        template <typename Base, type_traits::other_trans::enable_if_t<std::is_base_of_v<Base, Ty>, int> = 0>
        nebula_ptr_base<Base, Dx> &upcast() noexcept {
            return reinterpret_cast<nebula_ptr_base<Base, Dx> &>(*this);
        }

        template <typename Base, type_traits::other_trans::enable_if_t<std::is_base_of_v<Base, Ty>, int> = 0>
        const nebula_ptr_base<Base, Dx> &upcast() const noexcept {
            return reinterpret_cast<nebula_ptr_base<Base, Dx> &>(*this);
        }

        RAINY_NODISCARD constexpr pointer *operator&() noexcept { // NOLINT
            return utility::addressof(pair.second);
        }

        RAINY_NODISCARD constexpr const pointer *operator&() const noexcept { // NOLINT
            return utility::addressof(pair.second);
        }

        RAINY_NODISCARD constexpr pointer release() noexcept {
            return utility::exchange(pair.second, nullptr);
        }

        RAINY_NODISCARD constexpr pointer get() noexcept {
            return pair.second;
        }

        RAINY_NODISCARD constexpr const_pointer get() const noexcept {
            return pair.second;
        }

        RAINY_NODISCARD constexpr deleter_type &get_deleter() noexcept {
            return pair.get_first();
        }

        RAINY_NODISCARD constexpr const deleter_type &get_deleter() const noexcept {
            return pair.get_first();
        }

        constexpr pointer operator->() noexcept {
            return pair.second;
        }

        constexpr const_pointer operator->() const noexcept {
            return pair.second;
        }

        constexpr type_traits::modifers::add_lvalue_reference_t<element_type> operator*() {
            return as_reference();
        }

        constexpr type_traits::modifers::add_lvalue_reference_t<const element_type> operator*() const {
            return as_reference();
        }

        constexpr type_traits::modifers::add_lvalue_reference_t<element_type> as_reference() {
            return *get();
        }

        RAINY_NODISCARD constexpr type_traits::modifers::add_lvalue_reference_t<const element_type> as_reference() const {
            return *get();
        }

        constexpr void swap(nebula_ptr_base &right) noexcept {
            pair.swap(right.pair);
        }

    protected:
        foundation::container::compressed_pair<deleter_type, pointer> pair;
    };
}

namespace rainy::core::memory {
    template <typename Ty, typename Dx = default_deleter<Ty>>
    class nebula_ptr : public implements::nebula_ptr_base<Ty, Dx> {
    public:
        using nebula_base = implements::nebula_ptr_base<Ty, Dx>;
        using nebula_base::nebula_base;
    };

    template <typename Ty, typename Dx>
    class nebula_ptr<Ty[], Dx> : public implements::nebula_ptr_base<Ty, Dx> {
    public:
        using base = implements::nebula_ptr_base<Ty, Dx>;
        using element_type = typename base::element_type;
        using size_type = std::size_t;
        using pointer = typename base::pointer;
        using base::base;

        nebula_ptr(element_type *ptr, const size_type length) : base(ptr), length_(length) {
        }

        RAINY_CONSTEXPR20 ~nebula_ptr() {
            reset(nullptr);
        }

        element_type &operator[](const size_type idx) noexcept {
            return this->pair.second[idx];
        }

        const element_type &operator[](const size_type idx) const noexcept {
            return this->pair.second[idx];
        }

        element_type &at(const size_type idx) {
            range_check(idx);
            return this->pair.second[idx];
        }

        const element_type &at(const size_type idx) const {
            range_check(idx);
            return this->pair.second[idx];
        }

        constexpr pointer begin() noexcept {
            return this->pair.second;
        }

        constexpr pointer end() noexcept {
            return this->pair.second + length();
        }

        constexpr pointer begin() const noexcept {
            return this->pair.second;
        }

        constexpr pointer end() const noexcept {
            return this->pair.second + length();
        }

        RAINY_CONSTEXPR20 void fill(const Ty &val) noexcept(
            type_traits::properties::is_nothrow_constructible_v<Ty>) {
            core::algorithm::fill_n(this->pair.second, length(), val);
        }

        RAINY_CONSTEXPR20 void fill_with_ilist(std::initializer_list<Ty> ilist) noexcept(
            type_traits::properties::is_nothrow_constructible_v<Ty>) {
            size_type count = ilist.size() > length() ? length() : ilist.size();
            core::algorithm::copy_n(ilist.begin(), count, this->pair.second);
        }

        RAINY_CONSTEXPR20 void reset(element_type *pointer, size_type new_size = 0) {
            base::reset(pointer);
            this->length_ = new_size;
        }

        RAINY_CONSTEXPR20 nebula_ptr &operator=(std::nullptr_t) {
                base::reset();
            this->pair.second = nullptr;
            this->length_ = 0;
            return *this;
        }

        RAINY_CONSTEXPR20 nebula_ptr &operator=(element_type *pointer) {
            reset(pointer);
            return *this;
        }

        template <size_type N>
        RAINY_CONSTEXPR20 nebula_ptr &operator=(element_type (*arr)[N]) {
            reset<N>(*arr);
            return *this;
        }

        constexpr nebula_ptr &operator=(nebula_ptr &&right) noexcept {
            static_cast<base &>(*this) = utility::move(right);
            this->length_ = right.length_;
            return *this;
        }

        RAINY_NODISCARD constexpr size_type has_range() const noexcept {
            return length() == 0;
        }

        RAINY_NODISCARD constexpr size_type size() const noexcept {
            return length_;
        }

        RAINY_NODISCARD constexpr size_type length() const noexcept {
            return length_;
        }

    private:
        constexpr void range_check(const size_type idx) const {
            if (length() == 0) {
                return;
            }
            if (idx >= length()) {
                exceptions::logic::throw_out_of_range("Invalid array subscript");
            }
        }

        size_type length_{0};
    };

    template <typename Ty, typename... Args,
              type_traits::other_trans::enable_if_t<!type_traits::primary_types::is_array_v<Ty>, int> = 0>
    nebula_ptr<Ty> make_nebula(Args &&...args) noexcept(
        type_traits::properties::is_nothrow_constructible_v<Ty, Args...>) {
        return nebula_ptr<Ty>(new Ty(utility::forward<Args>(args)...));
    }

    template <typename Ty, typename... Args,
              type_traits::other_trans::enable_if_t<type_traits::primary_types::is_array_v<Ty> && type_traits::modifers::extent_v<Ty> == 0, int> = 0>
    nebula_ptr<Ty> make_nebula(const std::size_t num,
                               Args... args) noexcept(type_traits::properties::is_nothrow_constructible_v<Ty>) {
        using elem = type_traits::modifers::remove_extent_t<Ty>;
        std::allocator<elem> alloc;
        auto *data = alloc.allocate(num);

        std::size_t constructed = 0;
        try {
            for (; constructed < num; ++constructed) {
                memory::allocator_traits<std::allocator<elem>>::construct(alloc, &data[constructed], args...);
            }
            return nebula_ptr<Ty>(data, num);
        } catch (...) {
            for (std::size_t i = 0; i < constructed; ++i) {
                memory::allocator_traits<std::allocator<elem>>::destroy(alloc, &data[i]);
            }
            alloc.deallocate(data, num);
            throw;
        }
    }

    template <typename Ty, typename... Args,
              type_traits::other_trans::enable_if_t<type_traits::primary_types::is_array_v<Ty> && type_traits::modifers::extent_v<Ty> == 0, int> = 0>
    nebula_ptr<Ty> make_nebula(const std::size_t num,
                               std::initializer_list<type_traits::modifers::remove_extent_t<Ty>> ilist) noexcept(
        type_traits::properties::is_nothrow_constructible_v<Ty>) {
        using elem = type_traits::modifers::remove_extent_t<Ty>;
        auto data = new elem[num]();
        std::size_t count = num > ilist.size() ? ilist.size() : num;
        core::algorithm::copy_n(ilist.begin(), count, data);
        return nebula_ptr<Ty>(data, num);
    }

    template <typename Ty, typename Dx = default_deleter<Ty>>
    using unique_ptr = nebula_ptr<Ty, Dx>;

    template <typename Ty, typename... Args,
              type_traits::other_trans::enable_if_t<!type_traits::primary_types::is_array_v<Ty> , int> = 0>
    unique_ptr<Ty> make_unique(Args &&...args) noexcept(
        type_traits::properties::is_nothrow_constructible_v<Ty, Args...>) {
        return unique_ptr<Ty>(new Ty(utility::forward<Args>(args)...));
    }

    template <typename Ty, typename... Args,
              type_traits::other_trans::enable_if_t<type_traits::primary_types::is_array_v<Ty> && type_traits::modifers::extent_v<Ty> == 0, int> = 0>
    unique_ptr<Ty> make_unique(const std::size_t num) noexcept(
        type_traits::properties::is_nothrow_constructible_v<Ty>) {
        using elem = type_traits::modifers::remove_extent_t<Ty>;
        return unique_ptr<Ty>(new elem[num], num);
    }

    template <typename Ty, typename... Args,
              type_traits::other_trans::enable_if_t<type_traits::primary_types::is_array_v<Ty> && type_traits::modifers::extent_v<Ty> == 0, int> = 0>
    unique_ptr<Ty> make_unique(const std::size_t num,
                               std::initializer_list<type_traits::modifers::remove_extent_t<Ty>> ilist) noexcept(
        type_traits::properties::is_nothrow_constructible_v<Ty>) {
        using elem = type_traits::modifers::remove_extent_t<Ty>;
        auto data = new elem[num]();
        std::size_t count = num > ilist.size() ? ilist.size() : num;
        core::algorithm::copy_n(ilist.begin(), count, data);
        return unique_ptr<Ty>(data, num);
    }
}

#endif
