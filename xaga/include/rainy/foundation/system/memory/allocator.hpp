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
#ifndef RAINY_FOUNDATION_SYSTEM_MEMORY_ALLCATOR_HPP
#define RAINY_FOUNDATION_SYSTEM_MEMORY_ALLCATOR_HPP
#include <rainy/core/core.hpp>
#include <rainy/foundation/diagnostics/contract.hpp>

namespace rainy::foundation::system::memory {
    enum class allocation_method {
        std_allocator,
        cstd_allocator,
        rainy_allocator
    };

    template <typename Ptr, typename SizeType = std::size_t>
    struct allocation_result {
        constexpr allocation_result() noexcept = default;

        constexpr allocation_result(Ptr ptr, SizeType count) noexcept : ptr(ptr),count(count) {
        }

        Ptr ptr;
        SizeType count;
    };

    template <typename Ty>
    class allocator {
    public:
        using value_type = Ty;
        using pointer = value_type *;
        using size_type = std::size_t;
        using reference = value_type &;
        using const_reference = const value_type &;

        template <typename Other>
        using rebind_alloc = allocator<Other>;

        static_assert(!type_traits::implements::_is_reference_v<value_type>);

        static constexpr std::size_t align = alignof(value_type);
        static constexpr std::size_t element_size = sizeof(value_type);

        RAINY_CONSTEXPR20 allocator() noexcept = default;

        RAINY_CONSTEXPR20 allocator(const allocator &) noexcept = default;

        template <typename U>
        RAINY_CONSTEXPR20 explicit allocator(const allocator<U> &) noexcept {
        }

        RAINY_CONSTEXPR20 ~allocator() = default;

        constexpr allocator(allocator &&) noexcept = default;
        constexpr allocator &operator=(const allocator &) noexcept = default;
        constexpr allocator &operator=(allocator &&) noexcept = default;

        template <typename U>
        RAINY_CONSTEXPR20 explicit allocator(const std::allocator<U>&) noexcept {
        }

        template <allocation_method Method = allocation_method::rainy_allocator>
        RAINY_NODISCARD_RAW_PTR_ALLOC RAINY_CONSTEXPR20 pointer allocate(const size_type count) const {
#if RAINY_HAS_CXX20
            if (std::is_constant_evaluated()) {
                return std::allocator<value_type>{}.allocate(count);
            }
#endif
            using exceptions::runtime::throw_bad_alloc;
            if constexpr (Method == allocation_method::std_allocator) {
                try {
                    rainy_let ret = std::allocator<value_type>{}.allocate(count);
                    return ret;
                } catch (std::bad_alloc &) {
                    throw_bad_alloc();
                }
            } else if constexpr (Method == allocation_method::cstd_allocator) {
                rainy_let allocated_memory =
                    static_cast<pointer>(std::malloc(core::implements::get_size_of_n<value_type>(count)));
                if (!allocated_memory) {
                    throw_bad_alloc();
                }
                return allocated_memory;
            }
            rainy_let allocated_memory = static_cast<value_type *>(
                core::pal::allocate(core::implements::get_size_of_n<value_type>(count), align));
            if (!allocated_memory) {
                throw_bad_alloc();
            }
            return allocated_memory;
        }

        template <allocation_method Method = allocation_method::rainy_allocator>
        RAINY_CONSTEXPR20 void deallocate(value_type *block, const size_type count) const {
            if (!block || count == 0) {
                return;
            }
#if RAINY_HAS_CXX20
            if (std::is_constant_evaluated()) {
                std::allocator<value_type>{}.deallocate(block, count);
            } else
#endif
            {
                if constexpr (Method == allocation_method::std_allocator) {
                    std::allocator<value_type>{}.deallocate(block, count);
                } else if constexpr (Method == allocation_method::cstd_allocator) {
                    std::free(block);
                } else if constexpr (Method == allocation_method::rainy_allocator) {
                    core::pal::deallocate(block, core::implements::get_size_of_n<value_type>(count), align);
                }
            }
        }

        RAINY_NODISCARD_RAW_PTR_ALLOC RAINY_CONSTEXPR20 allocation_result<pointer> allocate_at_least(const size_type count) const {
            return {allocate(count), count};
        }

        template <typename... Args>
        RAINY_CONSTEXPR20 void construct(value_type *const ptr, Args &&...args) const
            noexcept(std::is_nothrow_constructible_v<value_type, Args...>) {
            utility::construct_at(ptr, utility::forward<Args>(args)...);
        }

        RAINY_CONSTEXPR20 void destory(value_type *const ptr) const noexcept(std::is_nothrow_destructible_v<value_type>) {
            ptr->~value_type();
        }
    };

    template <typename Ty, typename Other>
    RAINY_NODISCARD constexpr bool operator==(const allocator<Ty> &, const allocator<Other> &) noexcept {
        return true;
    }

    template <typename Ty, typename Other>
    RAINY_NODISCARD bool operator!=(const allocator<Ty> &, const allocator<Other> &) noexcept {
        return false;
    }

    namespace pmr {
        class memory_resource : public std::pmr::memory_resource {
        public:
            using std::pmr::memory_resource::memory_resource;

            RAINY_NODISCARD virtual allocation_method current_method() const noexcept = 0;
        };

        class cstd_memory_resource : public memory_resource {
        public:
            RAINY_NODISCARD allocation_method current_method() const noexcept override {
                return allocation_method::cstd_allocator;
            }

            static memory_resource *instance() {
                static cstd_memory_resource instance;
                return &instance;
            }

        private:
            void *do_allocate(std::size_t bytes, std::size_t) override {
                void *ptr = std::malloc(bytes);
                if (!ptr) {
                    exceptions::runtime::throw_bad_alloc();
                }
                return ptr;
            }

            void do_deallocate(void *block, std::size_t bytes, std::size_t alignment) override {
                if (bytes == 0 || !core::implements::is_pow_2(alignment) || !block) {
                    return;
                }
                std::free(block);
            }

            RAINY_NODISCARD bool do_is_equal(const std::pmr::memory_resource &right) const noexcept override {
                return this == utility::addressof(right);
            }
        };

        class std_memory_resource final : public memory_resource {
        public:
            RAINY_NODISCARD allocation_method current_method() const noexcept override {
                return allocation_method::std_allocator;
            }

            static memory_resource *instance() {
                static std_memory_resource instance;
                return &instance;
            }

        private:
            void *do_allocate(std::size_t bytes, std::size_t alignment) override {
#ifdef __cpp_aligned_new
                return ::operator new[](bytes, std::align_val_t{alignment});
#else
                return core::pal::allocate(bytes, alignment);
#endif
            }

            void do_deallocate(void *block, std::size_t bytes, std::size_t alignment) override {
                if (bytes == 0 || !core::implements::is_pow_2(alignment) || !block) {
                    return;
                }
                core::pal::deallocate(block, bytes, alignment);
            }

            RAINY_NODISCARD bool do_is_equal(const std::pmr::memory_resource &right) const noexcept override {
                return this == utility::addressof(right);
            }
        };

        class rainy_memory_resource final : public memory_resource {
        public:
            RAINY_NODISCARD allocation_method current_method() const noexcept override {
                return allocation_method::rainy_allocator;
            }

            static memory_resource *instance() {
                static rainy_memory_resource instance;
                return &instance;
            }

        private:
            void *do_allocate(std::size_t bytes, std::size_t alignment) override {
                if (bytes == 0 || !core::implements::is_pow_2(alignment)) {
                    return nullptr;
                }
                return core::pal::allocate(bytes, alignment);
            }

            void do_deallocate(void *block, std::size_t bytes, std::size_t alignment) override {
                if (bytes == 0 || !core::implements::is_pow_2(alignment) || !block) {
                    return;
                }
                core::pal::deallocate(block, bytes, alignment);
            }

            RAINY_NODISCARD bool do_is_equal(const std::pmr::memory_resource &right) const noexcept override {
                return this == utility::addressof(right);
            }
        };

        RAINY_INLINE memory_resource *get_memory_resource(allocation_method method) noexcept {
            switch (method) {
                case allocation_method::std_allocator:
                    return std_memory_resource::instance();
                case allocation_method::cstd_allocator:
                    return cstd_memory_resource::instance();
                case allocation_method::rainy_allocator:
                    return rainy_memory_resource::instance();
                default:
                    break;
            }
            return nullptr;
        }


#if RAINY_HAS_CXX20
        template <typename Ty = core::byte_t>
#else
        template <typename Ty>
#endif
        class polymorphic_allocator {
        public:
            using value_type = Ty;
            static constexpr std::size_t align = alignof(value_type);
            static constexpr std::size_t element_size = sizeof(value_type);

            polymorphic_allocator() noexcept = default;

            polymorphic_allocator(memory_resource *const resource) noexcept : _resource{resource} {
                utility::expects(static_cast<bool>(resource), "Cannot initialize polymorphic_allocator with null resource");
            }

            polymorphic_allocator(allocation_method method) : _resource{get_memory_resource(method)} {};

            polymorphic_allocator(const polymorphic_allocator &) = default;

            template <typename Uty>
            polymorphic_allocator(const polymorphic_allocator<Uty> &that) noexcept : _resource{that._resource} {
            }

            polymorphic_allocator &operator=(const polymorphic_allocator &) = delete;

            RAINY_NODISCARD_RAW_PTR_ALLOC value_type *allocate(const size_t count) {
                void *const resource = _resource->allocate(core::implements::get_size_of_n<Ty>(count), align);
                return static_cast<value_type *>(resource);
            }

            void deallocate(Ty *const ptr, const size_t count) noexcept {
                _resource->deallocate(ptr, core::implements::get_size_of_n<Ty>(count), align);
            }

            RAINY_NODISCARD_RAW_PTR_ALLOC void *allocate_bytes(const size_t bytes, const size_t align_ = alignof(std::max_align_t)) {
                return _resource->allocate(bytes, align_);
            }

            void deallocate_bytes(void *const ptr, const size_t bytes, const size_t align_ = alignof(std::max_align_t)) noexcept {
                _resource->deallocate(ptr, bytes, align_);
            }

            RAINY_NODISCARD_RAW_PTR_ALLOC value_type *allocate_object(const size_t count = 1) {
                void *const resource = allocate_bytes(core::implements::get_size_of_n<value_type>(count), align);
                return static_cast<value_type *>(resource);
            }

            void deallocate_object(value_type *const ptr, const size_t count = 1) noexcept {
                deallocate_bytes(ptr, core::implements::get_size_of_n<value_type>(count), align);
            }

            template <typename... Args>
            RAINY_NODISCARD_RAW_PTR_ALLOC value_type *new_object(Args &&...args) {
                value_type *const ptr = allocate_object();
                struct deallocate_bytes_guard {
                    ~deallocate_bytes_guard() noexcept {
                        if (mem_res) {
                            mem_res->deallocate(pointer, element_size, align);
                        }
                    }

                    deallocate_bytes_guard &operator=(const deallocate_bytes_guard &) = delete;
                    deallocate_bytes_guard &operator=(deallocate_bytes_guard &&) = delete;

                    memory_resource *mem_res;
                    void *pointer;
                };

                deallocate_bytes_guard guard{_resource, ptr};
                construct(ptr, utility::forward<Args>(args)...);
                guard.mem_res = nullptr;
                return ptr;
            }

            void delete_object(value_type *const ptr) noexcept {
                ptr->~value_type();
                deallocate_object(ptr);
            }

            template <typename... Args>
            void construct(value_type *const ptr, Args &&...args) {
                utility::construct_at(ptr, utility::forward<Args>(args)...);
            }

            void destroy(value_type *const ptr) noexcept {
                ptr->~value_type();
            }

            RAINY_NODISCARD polymorphic_allocator select_on_container_copy_construction() const noexcept {
                return {};
            }

            RAINY_NODISCARD memory_resource *resource() const noexcept {
                return _resource;
            }

            RAINY_NODISCARD_FRIEND bool operator==(const polymorphic_allocator &left, const polymorphic_allocator &right) noexcept {
                return *left._resource == *right._resource;
            }

        private:
            memory_resource *_resource = get_memory_resource(allocation_method::std_allocator);
        };
    }
}

namespace rainy::foundation::system::memory::implements {
    template <typename Alloc>
    RAINY_CONSTEXPR_BOOL is_std_allocator = false;

    template <typename Elem>
    RAINY_CONSTEXPR_BOOL is_std_allocator<std::allocator<Elem>> = true;
}

namespace rainy::foundation::system::memory::implements {
    template <typename Alloc>
    struct std_allocator_traits {
        using allocator_type = Alloc;
        using value_type = typename allocator_type::value_type;

        using pointer = value_type *;
        using const_pointer = const value_type *;
        using void_pointer = void *;
        using const_void_pointer = const void *;

        using size_type = size_t;
        using difference_type = ptrdiff_t;

        using propagate_on_container_copy_assignment = std::false_type;
        using propagate_on_container_move_assignment = std::true_type;
        using propagate_on_container_swap = std::false_type;
        using is_always_equal = std::true_type;

        template <typename Other>
        using rebind_alloc = std::allocator<Other>;

        template <typename Other>
        using rebind_traits = std_allocator_traits<std::allocator<Other>>;

        RAINY_CONSTEXPR20 static pointer allocate(allocator_type al, size_type count) {
            return al.allocate(count);
        }

        RAINY_CONSTEXPR20 static pointer allocate(allocator_type al, size_type count, const_void_pointer *hint) {
            return al.allocate(count);
        }

        RAINY_CONSTEXPR20 static void deallocate(allocator_type al, pointer ptr, size_type count) {
            return al.deallocate(ptr, count);
        }

        template <typename Uty, typename... Args>
        RAINY_CONSTEXPR20 static void construct(allocator_type, Uty *ptr, Args &&...args) noexcept(
            noexcept(utility::construct_at(ptr, utility::forward<Args>(args)...))) {
            return utility::construct_at(ptr, utility::forward<Args>(args)...);
        }

        template <typename Uty>
        RAINY_CONSTEXPR20 static void destory(allocator_type, Uty* ptr) {
            ptr->~value_type();
        }
    };
}

namespace rainy::foundation::system::memory {
    template <typename Alloc>
    struct allocator_traits {
        using allocator_type = Alloc;
    };
}

namespace rainy::foundation::system::memory {
    /**
     * @brief 一个具有静态存储资源的分配器
     * @tparam Ty 要分配资源的类型
     * @tparam N 希望分配的数量
     */
    template <typename Ty, std::size_t N>
    class block_allocator {
    public:
        using value_type = Ty;
        using pointer = value_type *;
        using size_type = std::size_t;
        using reference = value_type &;
        using const_reference = const value_type &;

        static constexpr std::size_t align = alignof(value_type);
        static constexpr std::size_t element_size = sizeof(value_type);

        RAINY_CONSTEXPR20 block_allocator() noexcept = default;

        RAINY_CONSTEXPR20 block_allocator(const block_allocator &) noexcept = default;

        template <typename U, std::size_t N_>
        RAINY_CONSTEXPR20 explicit block_allocator(const block_allocator<U, N_> &) noexcept {
        }

        RAINY_CONSTEXPR20 ~block_allocator() = default;

        constexpr block_allocator(block_allocator &&) noexcept = default;
        constexpr block_allocator &operator=(const block_allocator &) noexcept = default;
        constexpr block_allocator &operator=(block_allocator &&) noexcept = default;

        template <allocation_method Method = allocation_method::rainy_allocator>
        RAINY_NODISCARD_RAW_PTR_ALLOC RAINY_CONSTEXPR20 pointer allocate(const size_type count) const {
#if RAINY_HAS_CXX20
            if (std::is_constant_evaluated()) {
                return std::allocator<Ty>{}.allocate(count); // 因为我们无法直接返回一个数组地址，因此使用标准库分配模式
            } else
#endif
            {
                if (count > N || allocated_) {
                    return allocator<Ty>{}.template allocate<Method>(count);
                }
                allocated_ = true;
                return reinterpret_cast<pointer>(const_cast<core::byte_t *>(resources));
            }
        }

        template <allocation_method Method = allocation_method::rainy_allocator>
        RAINY_CONSTEXPR20 void deallocate(value_type *block, const size_type count) const {
#if RAINY_HAS_CXX20
            if (std::is_constant_evaluated()) {
                return std::allocator<Ty>{}.deallocate(block, count);
            } else
#endif
            {

                if (!block || count == 0) {
                    return;
                }
                if (block != static_cast<const void *>(utility::addressof(resources))) {
                    allocator<Ty>{}.template deallocate<Method>(block, count);
                } else {
                    allocated_ = false;
                }
            }
        }

        RAINY_NODISCARD_RAW_PTR_ALLOC constexpr allocation_result<pointer> allocate_at_least(const size_type count) const {
            return {allocate(count), count};
        }

        template <typename... Args>
        RAINY_CONSTEXPR20 void construct(value_type *const ptr, Args &&...args) const
            noexcept(std::is_nothrow_constructible_v<value_type, Args...>) {
            utility::construct_at(ptr, utility::forward<Args>(args)...);
        }

        RAINY_CONSTEXPR20 void destroy(value_type *const ptr) const noexcept(std::is_nothrow_destructible_v<value_type>) {
            ptr->~value_type();
        }

    private:
        mutable std::atomic_bool allocated_{false};
        alignas(Ty) core::byte_t resources[N == 0 ? 1 : element_size * N]{};
        // 这是类型无关的一个字节数组，它没有实际内容，确保实际行为与标准库一致
    };
}

#endif