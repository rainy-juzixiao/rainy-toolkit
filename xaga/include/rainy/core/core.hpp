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
/**
 * @file core.hpp
 * @brief 此头文件用于存放核心代码实现。这是rainy-toolkit向库模块，用户提供核心功能的模块。
 * @brief 它将建立一个基本的命名空间结构，并提供基础函数和元编程工具
 * @brief 在此，值得注意的是，rainy-toolkit采用的命名空间结构与C++标准库的命名空间结构不同
 * @brief rainy-toolkit的命名空间及其复杂且规整化。体现如下：
 * @brief 实用工具：rainy::utility
 * @brief 核心：rainy::core
 * @brief 核心平台相关：rainy::core::pal
 * @brief 内部实现：rainy::core::implements
 * @brief 库上层基础设施：rainy::foundation
 * @brief 平台抽象层基础设施相关：rainy::foundation::pal
 * @brief 因此，若无法适应。rainy-toolkit并不会适合你使用
 * @brief 另外。此头文件不会包含具体实现代码。关于平台相关的实现
 * @author rainy-juzixiao
 *
 * @date 2/24/2024 5:50:35 PM 在此进行添加注释（由rainy-juzixiao添加）
 */
/*-------------------------------
文件名： core.hpp
--------------------------------*/
#ifndef RAINY_CORE_CORE_HPP
#define RAINY_CORE_CORE_HPP

// NOLINTBEGIN

// clang-format off

#include <rainy/core/platform.hpp>
#include <rainy/core/type_traits.hpp>
#include <rainy/core/layer.hpp>
#include <rainy/core/implements/bit.hpp>
#include <rainy/core/implements/raw_stringview.hpp>
#include <rainy/core/implements/collections/array.hpp>
#include <rainy/core/implements/collections/array_view.hpp>
#include <rainy/core/implements/source_location.hpp>
#include <rainy/core/implements/views/views_interface.hpp>
#include <rainy/core/lifetime_annotation.hpp>

// clang-format on

// NOLINTEND

namespace rainy::core {
    static constexpr implements::raw_string_view<char> libray_name("rainy's toolkit");
    static constexpr implements::raw_string_view<char> creator_name("rainy-juzixiao");
    static constexpr implements::raw_string_view<char> current_version("0.1");
    static constexpr implements::raw_string_view<char> code_name("xaga");
}

namespace rainy::core::implements {
    constexpr bool is_pow_2(const std::size_t val) noexcept {
        return val != 0 && (val & (val - 1)) == 0;
    }

    template <typename Ty>
    constexpr std::size_t get_size_of_n(const std::size_t count) noexcept {
        constexpr std::size_t type_size = sizeof(Ty);
        if constexpr (constexpr bool overflow_is_possible = type_size > 1; overflow_is_possible) {
            if (constexpr std::size_t max_possible = static_cast<std::size_t>(-1) / type_size; count > max_possible) {
                std::terminate(); // multiply overflow
            }
        }
        return type_size * count;
    }

    template <typename Integral>
    RAINY_INLINE constexpr bool in_range(Integral start, Integral end, Integral wait_for_check) noexcept {
        if (start > end) {
            return false;
        }
        return wait_for_check >= start && wait_for_check <= end;
    }
}

namespace rainy::utility::implements {
    inline constexpr std::size_t fnv_offset_basis = static_cast<std::size_t>(14695981039346656037ULL);
    inline constexpr std::size_t fnv_prime = static_cast<std::size_t>(1099511628211ULL);

    RAINY_INLINE_NODISCARD std::size_t fnv1a_append_bytes(const std::size_t offset_basis, const unsigned char *const first,
                                                          const std::size_t count) noexcept {
        std::size_t hash = offset_basis;
        for (std::size_t i = 0; i < count; ++i) {
            hash ^= static_cast<std::size_t>(first[i]);
            hash *= fnv_prime;
        }
        return hash;
    }
}

namespace rainy::core::implements {
    RAINY_TOOLKIT_API void stl_internal_check(bool result);
}

namespace rainy::core {
    /**
     * @brief A temporary buffer for POD types with automatic management.
     *        Provides RAII-style management of temporary memory buffers.
     *
     *        用于POD类型的临时缓冲区，支持自动管理。
     *        提供RAII风格的临时内存缓冲区管理。
     *
     * @tparam Ty The element type, must be a plain old data type
     *            元素类型，必须是POD类型
     */
    template <typename Ty>
    class temporary_buffer {
    public:
        static_assert(type_traits::type_properties::is_pod_v<Ty>, "Ty must be a plain old data type!");

        using value_type = Ty;
        using pointer = value_type *;
        using const_pointer = const value_type *;
        using iterator = pointer;
        using const_iterator = const_pointer;
        using difference_type = std::ptrdiff_t;

        /**
         * @brief Constructs a temporary buffer from an existing buffer.
         *        从现有缓冲区构造临时缓冲区。
         *
         * @param buffer Pointer to the memory buffer
         *               指向内存缓冲区的指针
         * @param count Number of initialized elements
         *              已初始化元素的数量
         * @param capacity Total capacity of the buffer in elements
         *                 缓冲区的总容量（以元素为单位）
         */
        temporary_buffer(Ty *buffer, difference_type count, difference_type capacity) :
            buffer_(buffer), count_(count), capacity_(capacity) {
            implements::stl_internal_check(buffer);
        }

        /**
         * @brief Move constructor.
         *        移动构造函数。
         *
         * @param right The temporary_buffer to move from
         *              要移动的 temporary_buffer
         */
        temporary_buffer(temporary_buffer &&right) noexcept :
            buffer_{utility::exchange(right.buffer_, nullptr)}, count_{utility::exchange(right.count_, 0)},
            capacity_{utility::exchange(right.capacity_, 0)} {
        }

        /**
         * @brief Destructor that automatically returns the buffer.
         *        析构函数，自动归还缓冲区。
         */
        ~temporary_buffer() {
            return_buffer();
        }

        /**
         * @brief Move assignment operator.
         *        移动赋值运算符。
         *
         * @param right The temporary_buffer to move from
         *              要移动的 temporary_buffer
         * @return Reference to this object
         *         此对象的引用
         */
        rain_fn operator=(temporary_buffer &&right) noexcept -> temporary_buffer & {
            this->buffer_ = utility::exchange(right.buffer_, nullptr);
            this->count_ = utility::exchange(right.count_, 0);
            this->capacity_ = utility::exchange(right.capacity_, 0);
            return *this;
        }

        temporary_buffer(const temporary_buffer &) = delete;
        temporary_buffer &operator=(const temporary_buffer &) = delete;

        /**
         * @brief Returns an iterator to the beginning.
         *        返回指向起始的迭代器。
         *
         * @return Iterator to the first element
         *         指向第一个元素的迭代器
         */
        rain_fn begin() noexcept -> iterator {
            return buffer_;
        }

        /**
         * @brief Returns a const iterator to the beginning.
         *        返回指向起始的常量迭代器。
         *
         * @return Const iterator to the first element
         *         指向第一个元素的常量迭代器
         */
        rain_fn begin() const noexcept -> const_iterator {
            return buffer_;
        }

        /**
         * @brief Returns an iterator to the end.
         *        返回指向末尾的迭代器。
         *
         * @return Iterator to one past the last element
         *         指向最后一个元素之后位置的迭代器
         */
        rain_fn end() noexcept -> iterator {
            return buffer_ + count_;
        }

        /**
         * @brief Returns a const iterator to the end.
         *        返回指向末尾的常量迭代器。
         *
         * @return Const iterator to one past the last element
         *         指向最后一个元素之后位置的常量迭代器
         */
        rain_fn end() const noexcept -> const_iterator {
            return buffer_ + count_;
        }

        /**
         * @brief Returns the buffer to the system.
         *        将缓冲区归还给系统。
         */
        rain_fn return_buffer() -> void {
            if (!buffer_) {
                return;
            }
            pal::deallocate(buffer_, sizeof(Ty) * count_, alignof(Ty));
            buffer_ = nullptr;
            count_ = capacity_ = 0;
        }

        /**
         * @brief Reallocates the buffer to a new size.
         *        重新分配缓冲区到新大小。
         *
         * @param realloc The requested number of elements
         *                请求的元素数量
         */
        rain_fn reallocate(const std::size_t realloc) -> void {
            if (buffer_) {
                if (realloc <= count_ || realloc == 0 || implements::in_range<std::size_t>(count_, capacity_, realloc)) {
                    count_ = static_cast<std::ptrdiff_t>(realloc);
                    return;
                }
            }
            rainy_const reallocated_size = static_cast<std::size_t>(realloc * 1.2f);
            auto new_buffer = static_cast<pointer>(pal::allocate(reallocated_size, alignof(Ty)));
            builtin::copy_memory(new_buffer, buffer_, capacity_);
            return_buffer();
            implements::stl_internal_check(buffer_);
            buffer_ = new_buffer;
            capacity_ = reallocated_size;
            count_ = static_cast<std::ptrdiff_t>(realloc);
        }

        /**
         * @brief Gets the raw buffer pointer.
         *        获取原始缓冲区指针。
         *
         * @return Pointer to the buffer
         *         指向缓冲区的指针
         */
        rain_fn get_buffer() noexcept -> pointer {
            return buffer_;
        }

        /**
         * @brief Gets the raw buffer pointer (const version).
         *        获取原始缓冲区指针（常量版本）。
         *
         * @return Const pointer to the buffer
         *         指向缓冲区的常量指针
         */
        rain_fn get_buffer() const noexcept -> const_pointer {
            return buffer_;
        }

        /**
         * @brief Checks if the buffer is valid.
         *        检查缓冲区是否有效。
         *
         * @return true if buffer is valid, false otherwise
         *         如果缓冲区有效则为true，否则为false
         */
        explicit operator bool() const noexcept {
            return static_cast<bool>(buffer_);
        }

        /**
         * @brief Implicit conversion to pointer.
         *        到指针的隐式转换。
         *
         * @return Pointer to the buffer
         *         指向缓冲区的指针
         */
        operator pointer() noexcept {
            return buffer_;
        }

        /**
         * @brief Implicit conversion to const pointer.
         *        到常量指针的隐式转换。
         *
         * @return Const pointer to the buffer
         *         指向缓冲区的常量指针
         */
        operator const_pointer() const noexcept {
            return buffer_;
        }

    private:
        Ty *buffer_;
        std::ptrdiff_t count_;
        std::ptrdiff_t capacity_;
    };

    /**
     * @brief Obtains a temporary buffer for a given number of elements.
     *        获取用于指定数量元素的临时缓冲区。
     *
     * @tparam Ty The element type (must be POD)
     *            元素类型（必须是POD）
     * @param count The requested number of elements
     *              请求的元素数量
     * @return A temporary_buffer instance managing the allocated memory
     *         管理已分配内存的 temporary_buffer 实例
     */
    template <typename Ty>
    rain_fn get_temporary_buffer(std::ptrdiff_t count) noexcept -> temporary_buffer<Ty> {
        rainy_const new_size = static_cast<std::ptrdiff_t>(count * 1.2f);
        return {static_cast<Ty *>(pal::allocate(implements::get_size_of_n<Ty>(new_size), alignof(Ty))), count, new_size};
    }

    /**
     * @brief Explicitly returns a temporary buffer.
     *        显式归还临时缓冲区。
     *
     * @tparam Ty The element type
     *            元素类型
     * @param buffer The buffer to return
     *               要归还的缓冲区
     */
    template <typename Ty>
    rain_fn return_temporary_buffer(temporary_buffer<Ty> &buffer) -> void {
        buffer.return_buffer();
    }
}

namespace rainy::core {
    template <typename... Test>
    RAINY_CONSTEXPR_BOOL check_format_type =
        (type_traits::type_relations::is_any_convertible_v<type_traits::other_trans::decay_t<Test>, char, int, double, void *, float,
                                                           long, long long, unsigned int, unsigned long, unsigned long long,
                                                           const char *> ||
         ...);

    template <typename... Test>
    RAINY_CONSTEXPR_BOOL check_wformat_type =
        (type_traits::type_relations::is_any_convertible_v<type_traits::other_trans::decay_t<Test>, char, int, double, void *, float,
                                                           long, long long, unsigned int, unsigned long, unsigned long long,
                                                           const wchar_t *> ||
         ...);
}

#if RAINY_USING_GCC
#endif

// clang-format off

#ifndef RAINY_NODISCARD_RAW_PTR_ALLOC
#define RAINY_NODISCARD_RAW_PTR_ALLOC                                                                                                   \
    RAINY_NODISCARD_MSG("This function allocates memory and returns a raw pointer. "                                                    \
                        "Discarding the return value will cause a memory leak.")
#endif

#define RAINY_DECLARE_SIGNLE_INSTANCE(CLASSNAME) static CLASSNAME &instance() noexcept { static CLASSNAME instance;return instance;}

// clang-format on

namespace rainy::core::implements {
    constexpr static raw_string_view<char> token_charset("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789");
}

#include <thread>

namespace rainy::core::implements {
    static const std::thread::id main_thread_id = std::this_thread::get_id();
}

namespace rainy::core {
    RAINY_INLINE bool is_main_thread() noexcept {
        return implements::main_thread_id == std::this_thread::get_id();
    }

    static const std::size_t max_threads = std::thread::hardware_concurrency();
}

namespace rainy::foundation::memory {
    /**
     * @brief Default deleter for unique_ptr and similar smart pointers.
     *        Uses delete operator for single objects.
     *
     *        用于 unique_ptr 等智能指针的默认删除器。
     *        对单个对象使用 delete 操作符。
     *
     * @tparam Ty The type of object to delete (non-array version)
     *            要删除的对象类型（非数组版本）
     */
    template <typename Ty>
    struct default_deleter {
        constexpr default_deleter() noexcept = default;

        /**
         * @brief Constructs a default_deleter from a placeholder.
         *        从占位符构造 default_deleter。
         *
         * @param  placeholder_t placeholder value
         *         占位符值
         */
        constexpr default_deleter(utility::placeholder_t) noexcept {
        }

        /**
         * @brief Converting constructor from another default_deleter.
         *        从另一个 default_deleter 的转换构造函数。
         *
         * @tparam U The other type, must be convertible to Ty*
         *           另一个类型，必须可转换为 Ty*
         */
        template <typename U,
                  typename = type_traits::other_trans::enable_if_t<type_traits::type_relations::is_convertible_v<U *, Ty *>>>
        constexpr explicit default_deleter(const default_deleter &) noexcept {
        }

        /**
         * @brief Function call operator that deletes the pointer.
         *        删除指针的函数调用运算符。
         *
         * @param resource Pointer to delete
         *                 要删除的指针
         */
        RAINY_CONSTEXPR20 rain_fn operator()(const Ty *resource) const noexcept -> void {
            static_assert(0 < sizeof(Ty), "can't delete an incomplete type");
            delete resource;
        }
    };

    /**
     * @brief Default deleter specialization for array types.
     *        Uses delete[] operator for arrays.
     *
     *        数组类型的默认删除器特化。
     *        对数组使用 delete[] 操作符。
     *
     * @tparam Ty The element type of the array
     *            数组的元素类型
     */
    template <typename Ty>
    struct default_deleter<Ty[]> {
        constexpr default_deleter() = default;

        /**
         * @brief Constructs a default_deleter from a placeholder.
         *        从占位符构造 default_deleter。
         *
         * @param  placeholder_t placeholder value
         *         占位符值
         */
        constexpr default_deleter(utility::placeholder_t) noexcept {
        }

        /**
         * @brief Converting constructor from another default_deleter for arrays.
         *        从另一个数组 default_deleter 的转换构造函数。
         *
         * @tparam U The other element type, must have convertible array types
         *           另一个元素类型，数组类型必须可转换
         */
        template <typename U,
                  type_traits::other_trans::enable_if_t<type_traits::type_relations::is_convertible_v<U (*)[], Ty (*)[]>, int> = 0>
        constexpr explicit default_deleter(const default_deleter &) noexcept {
        }

        /**
         * @brief Function call operator that deletes the array pointer.
         *        删除数组指针的函数调用运算符。
         *
         * @tparam U The element type (deduced)
         *           元素类型（推导）
         * @param resource Pointer to array to delete
         *                 要删除的数组指针
         */
        template <typename U,
                  type_traits::other_trans::enable_if_t<type_traits::type_relations::is_convertible_v<U (*)[], Ty (*)[]>, int> = 0>
        RAINY_CONSTEXPR20 rain_fn operator()(const U *resource) const noexcept -> void {
            static_assert(!type_traits::type_properties::is_abstract_v<U>, "can't delete an incomplete type");
            delete[] resource;
        }
    };

    /**
     * @brief No-op deleter that does nothing.
     *        Useful for smart pointers that don't own the resource.
     *
     *        空操作删除器，什么都不做。
     *        用于不拥有资源的智能指针。
     *
     * @tparam Ty The type of object (non-array version)
     *            对象类型（非数组版本）
     */
    template <typename Ty>
    struct no_delete {
        constexpr no_delete() noexcept = default;

        /**
         * @brief Constructs a no_delete from a placeholder.
         *        从占位符构造 no_delete。
         *
         * @param  placeholder_t placeholder value
         *         占位符值
         */
        constexpr no_delete(utility::placeholder_t) noexcept {
        }

        /**
         * @brief Converting constructor from another no_delete.
         *        从另一个 no_delete 的转换构造函数。
         *
         * @tparam U The other type, must be convertible to Ty*
         *           另一个类型，必须可转换为 Ty*
         */
        template <typename U,
                  typename = type_traits::other_trans::enable_if_t<type_traits::type_relations::is_convertible_v<U *, Ty *>>>
        constexpr explicit no_delete(const no_delete &) noexcept {
        }

        /**
         * @brief Function call operator that does nothing.
         *        什么都不做的函数调用运算符。
         *
         * @param resource Pointer (ignored)
         *                 指针（忽略）
         */
        RAINY_CONSTEXPR20 rain_fn operator()(const Ty *) const noexcept -> void {
            static_assert(0 < sizeof(Ty), "can't delete an incomplete type");
        }
    };

    /**
     * @brief No-op deleter specialization for array types.
     *        Does nothing with the pointer.
     *
     *        数组类型的空操作删除器特化。
     *        对指针不进行任何操作。
     *
     * @tparam Ty The element type of the array
     *            数组的元素类型
     */
    template <typename Ty>
    struct no_delete<Ty[]> {
        constexpr no_delete() = default;

        /**
         * @brief Constructs a no_delete from a placeholder.
         *        从占位符构造 no_delete。
         *
         * @param  placeholder_t placeholder value
         *         占位符值
         */
        constexpr no_delete(utility::placeholder_t) noexcept {
        }

        /**
         * @brief Converting constructor from another no_delete for arrays.
         *        从另一个数组 no_delete 的转换构造函数。
         *
         * @tparam U The other element type, must have convertible array types
         *           另一个元素类型，数组类型必须可转换
         */
        template <typename U,
                  type_traits::other_trans::enable_if_t<type_traits::type_relations::is_convertible_v<U (*)[], Ty (*)[]>, int> = 0>
        constexpr explicit no_delete(const no_delete &) noexcept {
        }

        /**
         * @brief Function call operator that does nothing for array pointers.
         *        对数组指针什么都不做的函数调用运算符。
         *
         * @tparam U The element type (deduced)
         *           元素类型（推导）
         * @param resource Pointer to array (ignored)
         *                 数组指针（忽略）
         */
        template <typename U,
                  type_traits::other_trans::enable_if_t<type_traits::type_relations::is_convertible_v<U (*)[], Ty (*)[]>, int> = 0>
        RAINY_CONSTEXPR20 rain_fn operator()(const U *) const noexcept -> void {
            static_assert(!type_traits::type_properties::is_abstract_v<U>, "can't delete an incomplete type");
        }
    };
}

namespace rainy::utility::implements {
    template <typename Dx, typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL is_deleter_invocable_v = false;

    template <typename Dx, typename Ty>
    RAINY_CONSTEXPR_BOOL
        is_deleter_invocable_v<Dx, Ty, type_traits::other_trans::void_t<decltype(utility::declval<Dx>()(utility::declval<Ty *>()))>> =
            true;
}

namespace rainy::utility {
    template <typename Ty, typename Dx = foundation::memory::default_deleter<Ty>>
    class resource_guard {
    public:
        using value_type = Ty;
        using pointer = value_type *;
        using const_pointer = const value_type *;

        static_assert(!type_traits::type_relations::is_same_v<Ty, std::nullptr_t>, "Ty should not be nullptr_t");

        RAINY_CONSTEXPR20 resource_guard() = default;

        resource_guard(const resource_guard &) = delete;
        resource_guard(resource_guard &&) = delete;
        resource_guard &operator=(const resource_guard &) = delete;
        resource_guard &operator=(resource_guard &&) = delete;
        resource_guard(std::nullptr_t) = delete;

        RAINY_CONSTEXPR20 resource_guard(pointer data) : data_({}, data) {
            check_null(data);
        }

        RAINY_CONSTEXPR20 resource_guard(pointer data, Dx deleter) : data_(deleter, data) {
            check_null(data);
        }

        template <typename Uty, type_traits::other_trans::enable_if_t<type_traits::type_relations::is_convertible_v<Uty, value_type> &&
                                                                          implements::is_deleter_invocable_v<Dx, Ty>,
                                                                      int> = 0>
        RAINY_CONSTEXPR20 resource_guard(Uty *data) : data_({}, data) {
            check_null(data);
        }

        template <typename Uty, type_traits::other_trans::enable_if_t<type_traits::type_relations::is_convertible_v<Uty, value_type> &&
                                                                          implements::is_deleter_invocable_v<Dx, Ty>,
                                                                      int> = 0>
        RAINY_CONSTEXPR20 resource_guard(Uty *data, Dx deleter) : data_(deleter, data) {
            check_null(data);
        }

        RAINY_CONSTEXPR20 ~resource_guard() {
            reset();
        }

        template <typename Uty, type_traits::other_trans::enable_if_t<type_traits::type_relations::is_convertible_v<Uty, value_type> &&
                                                                          implements::is_deleter_invocable_v<Dx, Uty>,
                                                                      int> = 0>
        RAINY_CONSTEXPR20 void reset(Uty *new_data) {
            if (pointer release_data = get(); release_data != nullptr) {
                data_.get_first()(release_data);
            }
            data_.get_second() = new_data;
        }

        template <type_traits::other_trans::enable_if_t<implements::is_deleter_invocable_v<Dx, Ty>, int> = 0>
        RAINY_CONSTEXPR20 void reset(Ty *new_data = nullptr) {
            if (pointer release_data = get(); release_data != nullptr) {
                data_.get_first()(release_data);
            }
            data_.get_second() = new_data;
        }

        RAINY_CONSTEXPR20 pointer release() {
            return utility::exchange(data_.get_second(), nullptr);
        }

        RAINY_CONSTEXPR20 pointer get() {
            return data_.get_second();
        }

        RAINY_CONSTEXPR20 const_pointer get() const {
            return data_.get_second();
        }

        auto &get_deleter() noexcept {
            return data_.get_first();
        }

        const auto &get_deleter() const noexcept {
            return data_.get_first();
        }

    private:
        template <typename Type>
        RAINY_CONSTEXPR20 static void check_null(Type *data) {
            if (!data) {
                core::pal::debug_break();
            }
        }

        utility::compressed_pair<Dx, pointer> data_{};
    };

    template <typename F>
    class scope_guard {
    public:
        explicit scope_guard(F &&f) : func(utility::forward<F>(f)), active(true) {
        }

        scope_guard(const scope_guard &) = delete;
        scope_guard &operator=(const scope_guard &) = delete;

        scope_guard(scope_guard &&other) noexcept : func(utility::move(other.func)), active(other.active) {
            other.active = false;
        }

        ~scope_guard() {
            if (active) {
                func();
            }
        }

        void dismiss() noexcept {
            active = false;
        }

    private:
        F func;
        bool active;
    };

    template <typename F>
    scope_guard<F> make_scope_guard(F &&f) {
        return scope_guard<F>(utility::forward<F>(f));
    }
}

namespace rainy::utility {
    /**
     * @brief A template for hash function object.
     *        Provides hash computation for various types.
     *
     *        哈希函数对象的模板。
     *        为各种类型提供哈希计算。
     *
     * @tparam key The type to compute hash for
     *             要计算哈希的类型
     */
    template <typename Key>
    struct hash;
}

namespace rainy::utility::implements {
    template <typename Key>
    RAINY_AINLINE_NODISCARD std::size_t fnv1a_append_value(const std::size_t offset_basis, const Key &keyval) noexcept { // NOLINT
        static_assert(type_traits::type_properties::is_trivial_v<Key>, "Only trivial types can be directly hashed.");
        return fnv1a_append_bytes(offset_basis, &reinterpret_cast<const unsigned char &>(keyval), sizeof(Key));
    }

    template <typename Ty>
    RAINY_AINLINE_NODISCARD std::size_t fnv1a_append_range(const std::size_t offset_basis, const Ty *const first, // NOLINT
                                                           const Ty *const last) {
        static_assert(type_traits::type_properties::is_trivial_v<Ty>, "Only trivial types can be directly hashed.");
        const auto *const first_binary = reinterpret_cast<const unsigned char *>(first);
        const auto *const last_binary = reinterpret_cast<const unsigned char *>(last);
        return fnv1a_append_bytes(offset_basis, first_binary, static_cast<std::size_t>(last_binary - first_binary));
    }

    template <typename Key>
    RAINY_AINLINE_NODISCARD std::size_t hash_representation(const Key &keyval) noexcept {
        return fnv1a_append_value(fnv_offset_basis, keyval);
    }

    template <typename Key>
    RAINY_AINLINE_NODISCARD std::size_t hash_array_representation(const Key *const first, const std::size_t count) noexcept {
        static_assert(type_traits::type_properties::is_trivial_v<Key>, "Only trivial types can be directly hashed.");
        return fnv1a_append_bytes(fnv_offset_basis, reinterpret_cast<const unsigned char *>(first), count * sizeof(Key));
    }

    /**
     * @tparam key
     * @tparam check 如果为真，此模板将启用
     */
    template <typename key, bool check>
    struct hash_enable_if {
        using argument_type = key;
        using result_type = std::size_t;

        RAINY_AINLINE_NODISCARD result_type operator()(const argument_type &val) const
            noexcept(noexcept(hash<key>::hash_this_val(val))) {
            return hash<key>::hash_this_val(val);
        }
    };

    template <typename key>
    struct hash_enable_if<key, false> {
        hash_enable_if() = delete;
        hash_enable_if(const hash_enable_if &) = delete;
        hash_enable_if(hash_enable_if &&) = delete;
        hash_enable_if &operator=(const hash_enable_if &) = delete;
        hash_enable_if &operator=(hash_enable_if &&) = delete;
    };
}

namespace rainy::utility {
    /**
     * @brief Primary template for hash function object.
     *        Provides hash computation for various types.
     *
     *        哈希函数对象的主模板。
     *        为各种类型提供哈希计算。
     *
     * @tparam key The type to compute hash for
     *             要计算哈希的类型
     */
    template <typename key>
    struct hash : implements::hash_enable_if<
                      key, !type_traits::type_properties::is_const_v<key> && !type_traits::type_properties::is_volatile_v<key> &&
                               (type_traits::primary_types::is_enum_v<key> || type_traits::primary_types::is_integral_v<key> ||
                                type_traits::primary_types::is_pointer_v<key>)> {
        /**
         * @brief Computes hash value for the given key.
         *        计算给定键的哈希值。
         *
         * @param keyval The value to hash
         *               要哈希的值
         * @return Hash value
         *         哈希值
         */
        static std::size_t hash_this_val(const key &keyval) noexcept {
            return implements::hash_representation(keyval);
        }
    };

    /**
     * @brief Specialization for float type.
     *        float 类型的特化。
     */
    template <>
    struct hash<float> {
        using argument_type = float;
        using result_type = std::size_t;

        /**
         * @brief Computes hash value for float, treating -0.0f as 0.0f.
         *        计算 float 的哈希值，将 -0.0f 视为 0.0f。
         *
         * @param val The float value to hash
         *            要哈希的 float 值
         * @return Hash value
         *         哈希值
         */
        RAINY_AINLINE_NODISCARD rain_fn operator()(argument_type val) const->result_type {
            return implements::hash_representation(val == 0.0f ? 0.0f : val);
        }
    };

    /**
     * @brief Specialization for double type.
     *        double 类型的特化。
     */
    template <>
    struct hash<double> {
        using argument_type = double;
        using result_type = std::size_t;

        /**
         * @brief Computes hash value for double, treating -0.0 as 0.0.
         *        计算 double 的哈希值，将 -0.0 视为 0.0。
         *
         * @param val The double value to hash
         *            要哈希的 double 值
         * @return Hash value
         *         哈希值
         */
        RAINY_AINLINE_NODISCARD rain_fn operator()(argument_type val) const->result_type {
            return implements::hash_representation(val == 0.0 ? 0.0 : val);
        }
    };

    /**
     * @brief Specialization for long double type.
     *        long double 类型的特化。
     */
    template <>
    struct hash<long double> {
        using argument_type = long double;
        using result_type = std::size_t;

        /**
         * @brief Computes hash value for long double, treating -0.0L as 0.0L.
         *        计算 long double 的哈希值，将 -0.0L 视为 0.0L。
         *
         * @param val The long double value to hash
         *            要哈希的 long double 值
         * @return Hash value
         *         哈希值
         */
        RAINY_AINLINE_NODISCARD rain_fn operator()(argument_type val) const->result_type {
            return implements::hash_representation(val == 0.0L ? 0.0L : val);
        }
    };

    /**
     * @brief Specialization for nullptr_t type.
     *        nullptr_t 类型的特化。
     */
    template <>
    struct hash<std::nullptr_t> {
        using argument_type = std::nullptr_t;
        using result_type = std::size_t;

        /**
         * @brief Computes hash value for nullptr.
         *        计算 nullptr 的哈希值。
         *
         * @param  nullptr_t
         * @return Hash value (hash of null pointer)
         *         哈希值（空指针的哈希）
         */
        static rain_fn hash_this_val(std::nullptr_t) noexcept -> std::size_t {
            void *null_pointer{};
            return implements::hash_representation(null_pointer);
        }

        /**
         * @brief Function call operator for nullptr.
         *        nullptr 的函数调用运算符。
         *
         * @param  nullptr_t
         * @return Hash value (hash of null pointer)
         *         哈希值（空指针的哈希）
         */
        RAINY_AINLINE_NODISCARD rain_fn operator()(std::nullptr_t) const->result_type {
            void *null_pointer{};
            return implements::hash_representation(null_pointer);
        }
    };

    /**
     * @brief Specialization for basic_string_view types.
     *        basic_string_view 类型的特化。
     *
     * @tparam CharType Character type
     *                  字符类型
     * @tparam Traits String traits type
     *                字符串特性类型
     */
    template <typename CharType, typename Traits>
    struct hash<std::basic_string_view<CharType, Traits>> {
        using argument_type = std::basic_string_view<CharType, Traits>;
        using result_type = std::size_t;

        /**
         * @brief Computes hash value for a string view.
         *        计算字符串视图的哈希值。
         *
         * @param val The string view to hash
         *            要哈希的字符串视图
         * @return Hash value based on the string's characters
         *         基于字符串字符的哈希值
         */
        static rain_fn hash_this_val(const argument_type &val) noexcept -> std::size_t {
            return implements::hash_array_representation(val.data(), val.size());
        }

        /**
         * @brief Function call operator for string view.
         *        字符串视图的函数调用运算符。
         *
         * @param val The string view to hash
         *            要哈希的字符串视图
         * @return Hash value
         *         哈希值
         */
        RAINY_AINLINE_NODISCARD rain_fn operator()(argument_type val) const->result_type {
            return hash_this_val(val);
        }
    };

    /**
     * @brief Specialization for basic_string types.
     *        basic_string 类型的特化。
     *
     * @tparam CharType Character type
     *                  字符类型
     * @tparam Traits String traits type
     *                字符串特性类型
     * @tparam Alloc Allocator type
     *               分配器类型
     */
    template <typename CharType, typename Traits, typename Alloc>
    struct hash<std::basic_string<CharType, Traits, Alloc>> {
        using argument_type = std::basic_string<CharType, Traits, Alloc>;
        using result_type = std::size_t;

        /**
         * @brief Computes hash value for a string.
         *        计算字符串的哈希值。
         *
         * @param val The string to hash
         *            要哈希的字符串
         * @return Hash value based on the string's characters
         *         基于字符串字符的哈希值
         */
        static rain_fn hash_this_val(const argument_type &val) noexcept -> std::size_t {
            return implements::hash_array_representation(val.data(), val.size());
        }

        /**
         * @brief Function call operator for string.
         *        字符串的函数调用运算符。
         *
         * @param val The string to hash
         *            要哈希的字符串
         * @return Hash value
         *         哈希值
         */
        RAINY_AINLINE_NODISCARD rain_fn operator()(const argument_type &val) const->result_type {
            return hash_this_val(val);
        }
    };

    /**
     * @brief Type trait to check if standard hasher is available for a type.
     *        检查类型的标准哈希器是否可用的类型特性。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename Ty, typename = void>
    struct is_support_standard_hasher_available : type_traits::helper::false_type {};

    /**
     * @brief Specialization that detects if std::hash<Ty> is callable.
     *        检测 std::hash<Ty> 是否可调用的特化。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename Ty>
    struct is_support_standard_hasher_available<
        Ty, type_traits::other_trans::void_t<decltype(utility::declval<std::hash<Ty>>()(utility::declval<Ty>()))>>
        : type_traits::helper::true_type {};

    /**
     * @brief Type trait to check if Rainy Toolkit hasher is available for a type.
     *        检查类型的 Rainy Toolkit 哈希器是否可用的类型特性。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename Ty, typename = void>
    struct is_support_rainytoolkit_hasher_available : type_traits::helper::false_type {};

    /**
     * @brief Specialization that detects if rainy::utility::hash<Ty> is callable.
     *        检测 rainy::utility::hash<Ty> 是否可调用的特化。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename Ty>
    struct is_support_rainytoolkit_hasher_available<
        Ty, type_traits::other_trans::void_t<decltype(utility::declval<std::hash<Ty>>()(utility::declval<Ty>()))>>
        : type_traits::helper::true_type {};
}

namespace rainy::utility {
    /**
     * @brief Empty monostate type for use in variants and similar contexts.
     *        用于变体等场景的空monostate类型。
     */
    struct monostate {};
}

namespace rainy::core::algorithm {
    /**
     * @brief Copies elements from a range to another range.
     *        将元素从一个范围复制到另一个范围。
     *
     * @tparam InputIter Input iterator type
     *                   输入迭代器类型
     * @tparam OutIter Output iterator type
     *                 输出迭代器类型
     * @param begin Iterator to the beginning of the source range
     *              指向源范围起始的迭代器
     * @param end Iterator to the end of the source range
     *            指向源范围末尾的迭代器
     * @param dest Iterator to the beginning of the destination range
     *             指向目标范围起始的迭代器
     * @return Iterator to the end of the destination range
     *         指向目标范围末尾的迭代器
     */
    template <typename InputIter, typename OutIter>
    RAINY_CONSTEXPR20 rain_fn copy(InputIter begin, InputIter end, OutIter dest) noexcept(
        type_traits::type_properties::is_nothrow_copy_constructible_v<type_traits::other_trans::conditional_t<
            type_traits::implements::_is_pointer_v<InputIter>, type_traits::pointer_modify::remove_pointer_t<InputIter>,
            typename utility::iterator_traits<InputIter>::value_type>>) -> OutIter {
        using value_type = typename utility::iterator_traits<InputIter>::value_type;
        if (begin == end || (end - 1) == begin) {
            return dest;
        }
#if RAINY_HAS_CXX20
        if (std::is_constant_evaluated()) {
            auto input_begin = utility::addressof(*begin);
            auto input_end = utility::addressof(*(end - 1)) + 1;
            auto out_dest = utility::addressof(*dest);
            for (auto i = input_begin; i != input_end; ++i) {
                *out_dest = *i;
            }
            return out_dest;
        }
#endif
        if constexpr (type_traits::type_properties::is_standard_layout_v<value_type> &&
                      type_traits::type_properties::is_trivial_v<value_type>) {
            const auto input_begin = utility::addressof(*begin);
            const auto input_end = utility::addressof(*(end - 1)) + 1;
            auto out_dest = utility::addressof(*dest);
            std::memcpy(out_dest, input_begin, sizeof(value_type) * utility::distance(input_begin, input_end));
        } else {
            for (InputIter i = begin; begin != end; ++i, ++dest) {
                *dest = *i;
            }
        }
        return dest;
    }

    /**
     * @brief Copies exactly n elements from a range to another range.
     *        从一个范围精确复制n个元素到另一个范围。
     *
     * @tparam InputIter Input iterator type
     *                   输入迭代器类型
     * @tparam OutIter Output iterator type
     *                 输出迭代器类型
     * @param begin Iterator to the beginning of the source range
     *              指向源范围起始的迭代器
     * @param count Number of elements to copy
     *              要复制的元素数量
     * @param dest Iterator to the beginning of the destination range
     *             指向目标范围起始的迭代器
     * @return Iterator to the end of the destination range
     *         指向目标范围末尾的迭代器
     */
    template <typename InputIter, typename OutIter>
    constexpr rain_fn copy_n(InputIter begin, const std::size_t count, OutIter dest) noexcept(
        type_traits::type_properties::is_nothrow_copy_constructible_v<type_traits::other_trans::conditional_t<
            type_traits::implements::_is_pointer_v<InputIter>, type_traits::pointer_modify::remove_pointer_t<InputIter>,
            typename utility::iterator_traits<InputIter>::value_type>>) -> OutIter {
        using value_type = typename utility::iterator_traits<InputIter>::value_type;
        if (count == 0) {
            return dest;
        }
#if RAINY_HAS_CXX20
        if (std::is_constant_evaluated()) {
            auto input_begin = utility::addressof(*begin);
            for (std::size_t i = 0; i < count; ++i, ++input_begin, ++dest) {
                *dest = *input_begin;
            }
        } else
#endif
        {
            if constexpr (type_traits::type_properties::is_standard_layout_v<value_type> &&
                          type_traits::type_properties::is_trivial_v<value_type>) {
                const auto input_begin = utility::addressof(*begin);
                auto out_dest = utility::addressof(*dest);
                std::memcpy(out_dest, input_begin, sizeof(value_type) * count);
            } else {
                for (std::size_t i = 0; i < count; ++i, ++begin, ++dest) {
                    *dest = *begin;
                }
            }
        }
        return dest;
    }

    /**
     * @brief Applies a function to each element in a range and stores the results.
     *        对范围内的每个元素应用函数并存储结果。
     *
     * @tparam InputIter Input iterator type
     *                   输入迭代器类型
     * @tparam OutIter Output iterator type
     *                 输出迭代器类型
     * @tparam Fx Unary function type
     *            一元函数类型
     * @param begin Iterator to the beginning of the source range
     *              指向源范围起始的迭代器
     * @param end Iterator to the end of the source range
     *            指向源范围末尾的迭代器
     * @param dest Iterator to the beginning of the destination range
     *             指向目标范围起始的迭代器
     * @param func Function to apply to each element
     *             应用于每个元素的函数
     * @return Iterator to the end of the destination range
     *         指向目标范围末尾的迭代器
     */
    template <typename InputIter, typename OutIter, typename Fx>
    constexpr rain_fn transform(InputIter begin, InputIter end, OutIter dest, Fx func) noexcept(
        type_traits::type_properties::is_nothrow_copy_assignable_v<type_traits::other_trans::conditional_t<
            type_traits::implements::_is_pointer_v<InputIter>, type_traits::pointer_modify::remove_pointer_t<InputIter>,
            typename utility::iterator_traits<InputIter>::value_type>>) -> OutIter {
        for (InputIter iter = begin; iter != end; ++iter, ++dest) {
            *dest = func(*iter);
        }
        return dest;
    }

    /**
     * @brief Applies a binary function to elements from two ranges and stores the results.
     *        对两个范围的元素应用二元函数并存储结果。
     *
     * @tparam InputIter Input iterator type
     *                   输入迭代器类型
     * @tparam OutIter Output iterator type
     *                 输出迭代器类型
     * @tparam Fx Binary function type
     *            二元函数类型
     * @param begin1 Iterator to the beginning of the first source range
     *               指向第一个源范围起始的迭代器
     * @param end1 Iterator to the end of the first source range
     *             指向第一个源范围末尾的迭代器
     * @param begin2 Iterator to the beginning of the second source range
     *               指向第二个源范围起始的迭代器
     * @param dest Iterator to the beginning of the destination range
     *             指向目标范围起始的迭代器
     * @param func Binary function to apply to each pair of elements
     *             应用于每对元素的二元函数
     * @return Iterator to the end of the destination range
     *         指向目标范围末尾的迭代器
     */
    template <typename InputIter, typename OutIter, typename Fx>
    constexpr rain_fn transform(InputIter begin1, InputIter end1, InputIter begin2, OutIter dest, Fx func) noexcept(
        type_traits::type_properties::is_nothrow_copy_assignable_v<type_traits::other_trans::conditional_t<
            type_traits::implements::_is_pointer_v<InputIter>, type_traits::pointer_modify::remove_pointer_t<InputIter>,
            typename utility::iterator_traits<InputIter>::value_type>>) -> OutIter {
        if (begin1 == end1 || (end1 - 1) == begin1) {
            return dest;
        }
        for (InputIter iter = begin1; iter != end1; ++iter, ++dest, ++begin2) {
            *dest = func(*iter, *begin2);
        }
        return dest;
    }
}

namespace rainy::collections::views::implements {
    template <typename Iter>
    class iterator_range_iterator {
    public:
        using iterator_category = typename utility::iterator_traits<Iter>::iterator_category;
        using value_type = typename utility::iterator_traits<Iter>::value_type;
        using difference_type = typename utility::iterator_traits<Iter>::difference_type;
        using pointer = typename utility::iterator_traits<Iter>::pointer;
        using reference = typename utility::iterator_traits<Iter>::reference;

        /**
         * @brief Constructs an iterator_range_iterator from an underlying iterator.
         *        从底层迭代器构造iterator_range_iterator。
         *
         * @param iter The underlying iterator to wrap
         *             要包装的底层迭代器
         */
        iterator_range_iterator(Iter iter) : iter_{iter} {
        }

        /**
         * @brief Dereference operator.
         *        解引用运算符。
         *
         * @return Reference to the element pointed to by the underlying iterator
         *         底层迭代器所指向元素的引用
         */
        decltype(auto) operator*() const {
            return (*iter_);
        }

        /**
         * @brief Arrow operator.
         *        箭头运算符。
         *
         * @return Pointer to the element pointed to by the underlying iterator
         *         指向底层迭代器所指向元素的指针
         */
        pointer operator->() const {
            return utility::addressof(*iter_);
        }

        /**
         * @brief Prefix increment operator.
         *        前置自增运算符。
         *
         * @return Reference to this iterator after increment
         *         自增后此迭代器的引用
         */
        rain_fn operator++()->iterator_range_iterator & {
            ++iter_;
            return *this;
        }

        /**
         * @brief Postfix increment operator.
         *        后置自增运算符。
         *
         * @return Copy of this iterator before increment
         *         自增前此迭代器的副本
         */
        rain_fn operator++(int) {
            iterator_range_iterator temp = *this;
            ++iter_;
            return temp;
        }

        /**
         * @brief Prefix decrement operator.
         *        前置自减运算符。
         *
         * @return Reference to this iterator after decrement
         *         自减后此迭代器的引用
         */
        rain_fn operator--() {
            --iter_;
            return *this;
        }

        /**
         * @brief Postfix decrement operator.
         *        后置自减运算符。
         *
         * @return Copy of this iterator before decrement
         *         自减前此迭代器的副本
         */
        rain_fn operator--(int) {
            iterator_range_iterator temp = *this;
            --iter_;
            return temp;
        }

        /**
         * @brief Addition operator.
         *        加法运算符。
         *
         * @param n Number of positions to advance
         *          前进的位置数
         * @return New iterator advanced by n positions
         *         前进n个位置后的新迭代器
         */
        rain_fn operator+(difference_type n) const->iterator_range_iterator {
            return iterator_range_iterator(iter_ + n);
        }

        /**
         * @brief Subtraction operator.
         *        减法运算符。
         *
         * @param n Number of positions to move back
         *          后退的位置数
         * @return New iterator moved back by n positions
         *         后退n个位置后的新迭代器
         */
        rain_fn operator-(difference_type n) const->iterator_range_iterator {
            return iterator_range_iterator(iter_ - n);
        }

        /**
         * @brief Addition assignment operator.
         *        加法赋值运算符。
         *
         * @param n Number of positions to advance
         *          前进的位置数
         * @return Reference to this iterator after advancement
         *         前进后此迭代器的引用
         */
        rain_fn &operator+=(difference_type n) {
            iter_ += n;
            return *this;
        }

        /**
         * @brief Subtraction assignment operator.
         *        减法赋值运算符。
         *
         * @param n Number of positions to move back
         *          后退的位置数
         * @return Reference to this iterator after moving back
         *         后退后此迭代器的引用
         */
        rain_fn &operator-=(difference_type n) {
            iter_ -= n;
            return *this;
        }

        /**
         * @brief Difference operator.
         *        差运算符。
         *
         * @param other Another iterator to compare with
         *              要比较的另一个迭代器
         * @return Distance between this iterator and other
         *         此迭代器与other之间的距离
         */
        rain_fn operator-(const iterator_range_iterator &other) const->difference_type {
            return iter_ - other.iter_;
        }

        /**
         * @brief Equality comparison operator.
         *        相等比较运算符。
         *
         * @param left Left-hand side iterator
         *             左侧迭代器
         * @param right Right-hand side iterator
         *              右侧迭代器
         * @return true if iterators are equal, false otherwise
         *         如果迭代器相等则为true，否则为false
         */
        friend bool operator==(const iterator_range_iterator &left, const iterator_range_iterator &right) noexcept {
            return left.iter_ == right.iter_;
        }

        /**
         * @brief Inequality comparison operator.
         *        不等比较运算符。
         *
         * @param left Left-hand side iterator
         *             左侧迭代器
         * @param right Right-hand side iterator
         *              右侧迭代器
         * @return true if iterators are not equal, false otherwise
         *         如果迭代器不相等则为true，否则为false
         */
        friend bool operator!=(const iterator_range_iterator &left, const iterator_range_iterator &right) noexcept {
            return !(left.iter_ == right.iter_);
        }

    private:
        Iter iter_;
    };

    template <typename Iter>
    class adapter_iterator_range : public views::view_interface<adapter_iterator_range<Iter>> {
    public:
        using iterator = iterator_range_iterator<Iter>;
        using const_iterator = const iterator_range_iterator<const Iter>;
        using reference = type_traits::extras::iterators::iterator_reference_t<iterator>;
        using const_reference = type_traits::cv_modify::add_const_t<reference>;
        using difference_type = type_traits::extras::iterators::iterator_difference_t<const_iterator>;
        using value_type = type_traits::extras::iterators::iter_value_t<const_iterator>;

        /**
         * @brief Default constructor.
         *        默认构造函数。
         */
        adapter_iterator_range() : begin_{}, end_{} {
        }

        /**
         * @brief Constructs an adapter_iterator_range from begin and end iterators.
         *        从开始和结束迭代器构造adapter_iterator_range。
         *
         * @param begin Iterator to the beginning of the range
         *              指向范围起始的迭代器
         * @param end Iterator to the end of the range
         *            指向范围末尾的迭代器
         */
        adapter_iterator_range(Iter begin, Iter end) : begin_{begin}, end_{end} {
        }

        /**
         * @brief Returns a const reference to the base range.
         *        返回基范围的常量引用。
         *
         * @return Const reference to this adapter_iterator_range
         *         此adapter_iterator_range的常量引用
         */
        RAINY_NODISCARD constexpr rain_fn base() const & noexcept -> const adapter_iterator_range & {
            return *this;
        }

        /**
         * @brief Returns an rvalue reference to the base range.
         *        返回基范围的右值引用。
         *
         * @return Rvalue reference to this adapter_iterator_range
         *         此adapter_iterator_range的右值引用
         */
        RAINY_NODISCARD constexpr rain_fn base() && noexcept -> adapter_iterator_range {
            return utility::move(*this);
        }

        /**
         * @brief Returns an iterator to the beginning.
         *        返回指向起始的迭代器。
         *
         * @return Iterator to the first element
         *         指向第一个元素的迭代器
         */
        rain_fn begin() noexcept -> iterator {
            return iterator{begin_};
        }

        /**
         * @brief Returns an iterator to the end.
         *        返回指向末尾的迭代器。
         *
         * @return Iterator to one past the last element
         *         指向最后一个元素之后位置的迭代器
         */
        rain_fn end() noexcept -> iterator {
            return iterator{end_};
        }

        /**
         * @brief Returns a const iterator to the beginning.
         *        返回指向起始的常量迭代器。
         *
         * @return Const iterator to the first element
         *         指向第一个元素的常量迭代器
         */
        rain_fn begin() const noexcept -> const_iterator {
            return const_iterator{begin_};
        }

        /**
         * @brief Returns a const iterator to the end.
         *        返回指向末尾的常量迭代器。
         *
         * @return Const iterator to one past the last element
         *         指向最后一个元素之后位置的常量迭代器
         */
        rain_fn end() const noexcept -> const_iterator {
            return const_iterator{end_};
        }

        /**
         * @brief Returns a const iterator to the beginning (explicit).
         *        返回指向起始的常量迭代器（显式）。
         *
         * @return Const iterator to the first element
         *         指向第一个元素的常量迭代器
         */
        rain_fn cbegin() const noexcept -> const_iterator {
            return const_iterator{begin_};
        }

        /**
         * @brief Returns a const iterator to the end (explicit).
         *        返回指向末尾的常量迭代器（显式）。
         *
         * @return Const iterator to one past the last element
         *         指向最后一个元素之后位置的常量迭代器
         */
        rain_fn cend() const noexcept -> const_iterator {
            return const_iterator{end_};
        }

    private:
        Iter begin_;
        Iter end_;
    };
}

namespace rainy::collections::views {
    /**
     * @brief A range view over an iterator pair.
     *        基于迭代器对的区间视图。
     *
     * This class provides a view over a range defined by a pair of iterators,
     * inheriting all functionality from adapter_iterator_range.
     *
     * 此类提供由一对迭代器定义的区间的视图，
     * 继承自 adapter_iterator_range 的所有功能。
     *
     * @tparam Iter The underlying iterator type
     *              底层迭代器类型
     */
    template <typename Iter>
    class iterator_range : public implements::adapter_iterator_range<Iter> {
    public:
        using base = implements::adapter_iterator_range<Iter>;

        /**
         * @brief Constructs an iterator_range from begin and end iterators.
         *        从开始和结束迭代器构造 iterator_range。
         *
         * @param begin Iterator to the beginning of the range
         *              指向范围起始的迭代器
         * @param end Iterator to the end of the range
         *            指向范围末尾的迭代器
         */
        iterator_range(Iter begin, Iter end) : base(begin, end) {
        }
    };
}

namespace rainy::core::implements {
    struct poly_inspector {
        template <typename Type>
        operator Type &&() const;

        template <std::size_t Member, typename... Args>
        RAINY_NODISCARD poly_inspector invoke(Args &&...args) const;

        template <std::size_t Member, typename... Args>
        RAINY_NODISCARD poly_inspector invoke(Args &&...args);
    };

    template <typename Concept>
    struct make_vtable {
    public:
        using inspector = typename Concept::template type<implements::poly_inspector>;

        template <auto... Candidate>
        static auto make(type_traits::other_trans::value_list<Candidate...>) noexcept
            -> decltype(std::make_tuple(vtable_entry(Candidate)...));

        template <typename... Func>
        RAINY_NODISCARD static constexpr auto make(type_traits::other_trans::type_list<Func...>) noexcept {
            if constexpr (sizeof...(Func) == 0u) {
                return decltype(make_with_vl(typename Concept::template impl<inspector>{}))();
            } else if constexpr ((type_traits::primary_types::is_function_v<Func> && ...)) {
                return decltype(std::make_tuple(vtable_entry(std::declval<Func>())...))();
            }
        }

        template <auto... V>
        RAINY_NODISCARD static constexpr auto make_with_vl(type_traits::other_trans::value_list<V...>) noexcept {
            return decltype(std::make_tuple(vtable_entry(V)...))();
        }

        template <typename Func>
        static auto vtable_entry(Func) noexcept {
            using namespace type_traits;
            using namespace type_traits::primary_types;
            using Traits = function_traits<Func>;
            using ret = typename Traits::return_type;
            if constexpr (Traits::is_const_member_function) {
                return static_cast<pointer_modify::add_pointer_t<
                    typename make_normalfx_type_with_pl<ret, Func, other_trans::type_list<const void *>>::type>>(nullptr);
            } else if constexpr (Traits::is_member_function_pointer) {
                return static_cast<pointer_modify::add_pointer_t<
                    typename make_normalfx_type_with_pl<ret, Func, other_trans::type_list<void *>>::type>>(nullptr);
            } else {
                return static_cast<Func>(nullptr);
            }
        }

        template <typename Type, auto Candidate, typename Ret, typename PtrType, typename... Args>
        static void fill_vtable_entry(Ret (*&entry)(PtrType, Args...)) noexcept {
            if constexpr (type_traits::type_properties::is_invocable_r_v<Ret, decltype(Candidate), Args...>) {
                entry = +[](PtrType, Args... args) -> Ret { return utility::invoke(Candidate, utility::forward<Args>(args)...); };
            } else {
                entry = +[](PtrType instance_ptr, Args... args) -> Ret {
                    return static_cast<Ret>(utility::invoke(
                        Candidate,
                        *static_cast<type_traits::cv_modify::constness_as_t<Type, std::remove_pointer_t<PtrType>> *>(instance_ptr),
                        utility::forward<Args>(args)...));
                };
            }
        }

        template <typename VtableType, typename Type, auto... Index>
        RAINY_NODISCARD static auto fill_vtable(std::index_sequence<Index...>) noexcept {
            VtableType impl{};
            (fill_vtable_entry<Type, type_traits::other_trans::value_at<Index, typename Concept::template impl<Type>>::value>(
                 std::get<Index>(impl)),
             ...);
            return impl;
        }
    };
}

namespace rainy::core {
    /**
     * @brief Virtual table provider for polymorphic concepts.
     *        Generates and stores the vtable for a given concept.
     *
     *        多态概念的虚表提供者。
     *        为给定概念生成并存储虚表。
     *
     * @tparam Concept The concept that defines the polymorphic interface
     *                 定义多态接口的概念
     */
    template <typename Concept>
    class poly_vtable {
    public:
        using inspector = typename Concept::template type<implements::poly_inspector>;
        using vtable_type = decltype(implements::make_vtable<Concept>::make_with_vl(typename Concept::template impl<inspector>{}));
        static constexpr bool is_mono = std::tuple_size_v<vtable_type> == 1u;

        using type = std::conditional_t<is_mono, std::tuple_element_t<0u, vtable_type>, const vtable_type *>;

        /**
         * @brief Gets the vtable instance for a specific type.
         *        获取特定类型的虚表实例。
         *
         * @tparam Type The concrete type to get the vtable for
         *              要获取虚表的具体类型
         * @return The vtable (direct value if mono, pointer otherwise)
         *         虚表（如果是单函数则为直接值，否则为指针）
         */
        template <typename Type>
        RAINY_NODISCARD static rain_fn instance() noexcept -> type {
            static_assert(type_traits::type_relations::is_same_v<Type, type_traits::other_trans::decay_t<Type>>,
                          "Type differs from its decayed form");
            static const vtable_type vtable = implements::make_vtable<Concept>::template fill_vtable<vtable_type, Type>(
                std::make_index_sequence<type_traits::other_trans::value_list_size_v<typename Concept::template impl<Type>>>{});
            if constexpr (is_mono) {
                return std::get<0>(vtable);
            } else {
                return &vtable;
            }
        }
    };

    /**
     * @brief Base class for polymorphic objects providing virtual call dispatching.
     *        为多态对象提供虚调用分派的基类。
     *
     * @tparam Poly The derived polymorphic type (CRTP)
     *              派生的多态类型（CRTP）
     */
    template <typename Poly>
    struct poly_base {
        /**
         * @brief Invokes a member function on a const polymorphic object.
         *        在常量多态对象上调用成员函数。
         *
         * @tparam Member The index of the member function to invoke
         *                要调用的成员函数索引
         * @tparam Args The argument types
         *              参数类型
         * @param self The const polymorphic object
         *             常量多态对象
         * @param args The arguments to forward
         *             要转发的参数
         * @return The result of the function call
         *         函数调用的结果
         */
        template <std::size_t Member, typename... Args>
        rain_fn invoke(const poly_base &self, Args &&...args) const -> decltype(auto) {
            const auto &poly = static_cast<const Poly &>(self);
            if constexpr (Poly::vtable_info::is_mono) {
                return poly.vtable(poly._ptr, utility::forward<Args>(args)...);
            } else {
                return std::get<Member>(*poly.vtable)(poly._ptr, utility::forward<Args>(args)...);
            }
        }

        /**
         * @brief Invokes a member function on a mutable polymorphic object.
         *        在可变多态对象上调用成员函数。
         *
         * @tparam Member The index of the member function to invoke
         *                要调用的成员函数索引
         * @tparam Args The argument types
         *              参数类型
         * @param self The mutable polymorphic object
         *             可变多态对象
         * @param args The arguments to forward
         *             要转发的参数
         * @return The result of the function call
         *         函数调用的结果
         */
        template <std::size_t Member, typename... Args>
        rain_fn invoke(poly_base &self, Args &&...args) -> decltype(auto) {
            auto &poly = static_cast<Poly &>(self);
            if constexpr (Poly::vtable_info::is_mono) {
                static_assert(Member == 0, "Unknown member");
                return poly.vtable(poly._ptr, utility::forward<Args>(args)...);
            } else {
                return std::get<Member>(*poly.vtable)(poly._ptr, utility::forward<Args>(args)...);
            }
        }
    };

    /**
     * @brief Helper function to make polymorphic calls.
     *        进行多态调用的辅助函数。
     *
     * @tparam Member The index of the member function to invoke
     *                要调用的成员函数索引
     * @tparam Poly The polymorphic type
     *              多态类型
     * @tparam Args The argument types
     *              参数类型
     * @param self The polymorphic object
     *             多态对象
     * @param args The arguments to forward
     *             要转发的参数
     * @return The result of the function call
     *         函数调用的结果
     */
    template <std::size_t Member, typename Poly, typename... Args>
    rain_fn poly_call(Poly &&self, Args &&...args) -> decltype(auto) {
        return utility::forward<Poly>(self).template invoke<Member>(self, utility::forward<Args>(args)...);
    }

    /**
     * @brief Basic polymorphic wrapper that stores a void pointer and vtable.
     *        存储void指针和虚表的基本多态包装器。
     *
     * @tparam AbstractBody The abstract concept body that defines the interface
     *                      定义接口的抽象概念主体
     */
    template <typename AbstractBody>
    class basic_poly : AbstractBody::template type<poly_base<basic_poly<AbstractBody>>> {
    public:
        friend struct poly_base<basic_poly>;

        using abstract_type = typename AbstractBody::template type<poly_base<basic_poly>>;
        using vtable_info = poly_vtable<AbstractBody>;
        using vtable_type = typename vtable_info::type;

        /**
         * @brief Default constructor
         *        默认构造函数
         */
        basic_poly() noexcept = default;

        /**
         * @brief Constructs from a pointer to a concrete type.
         *        从指向具体类型的指针构造。
         *
         * @tparam Type The concrete type
         *              具体类型
         * @param ptr Pointer to the object to wrap
         *            要包装的对象的指针
         */
        template <typename Type>
        basic_poly(Type *ptr) noexcept : // NOLINT
            _ptr(static_cast<void *>(ptr)), vtable{vtable_info::template instance<std::remove_cv_t<std::remove_pointer_t<Type>>>()} {
        }

        /**
         * @brief Constructs a null polymorphic object.
         *        构造空多态对象。
         *
         * @param  nullptr_t
         */
        basic_poly(std::nullptr_t) { // NOLINT
        }

        /**
         * @brief Move constructor
         *        移动构造函数
         *
         * @param other The other polymorphic object to move from
         *              要移动的另一个多态对象
         */
        basic_poly(basic_poly &&other) noexcept : _ptr(other._ptr), vtable(other.vtable) {
            other._ptr = nullptr;
            other.vtable = {};
        }

        /**
         * @brief Move assignment operator
         *        移动赋值运算符
         *
         * @param other The other polymorphic object to move from
         *              要移动的另一个多态对象
         * @return Reference to this object
         *         此对象的引用
         */
        basic_poly &operator=(basic_poly &&other) noexcept {
            if (this != &other) {
                _ptr = other._ptr;
                vtable = other.vtable;
                other._ptr = nullptr;
                other.vtable = {};
            }
            return *this;
        }

        /**
         * @brief Assigns a null pointer.
         *        赋值为空指针。
         *
         * @param  nullptr_t
         * @return Reference to this object
         *         此对象的引用
         */
        basic_poly &operator=(std::nullptr_t) noexcept {
            _ptr = nullptr;
            return *this;
        }

        /**
         * @brief Copy constructor
         *        拷贝构造函数
         */
        basic_poly(const basic_poly &) = default;

        /**
         * @brief Copy assignment operator
         *        拷贝赋值运算符
         */
        basic_poly &operator=(const basic_poly &) = default;

        /**
         * @brief Resets to null state.
         *        重置为空状态。
         */
        rain_fn reset() noexcept -> void {
            _ptr = nullptr;
            vtable = {};
        }

        /**
         * @brief Resets to point to a new object.
         *        重置为指向新对象。
         *
         * @tparam Type The concrete type
         *              具体类型
         * @param ptr Pointer to the new object
         *            新对象的指针
         */
        template <typename Type>
        rain_fn reset(Type *ptr) noexcept -> void {
            _ptr = static_cast<void *>(ptr);
            vtable = vtable_info::template instance<std::remove_cv_t<std::remove_pointer_t<Type>>>();
        }

        /**
         * @brief Checks if the object is empty.
         *        检查对象是否为空。
         *
         * @return true if empty, false otherwise
         *         如果为空则为true，否则为false
         */
        rain_fn empty() const noexcept -> bool {
            return vtable == nullptr;
        }

        /**
         * @brief Checks if the object holds a non-null pointer.
         *        检查对象是否持有非空指针。
         *
         * @return true if pointer is non-null, false otherwise
         *         如果指针非空则为true，否则为false
         */
        RAINY_NODISCARD explicit operator bool() const noexcept {
            return _ptr != nullptr;
        }

        /**
         * @brief Arrow operator to access the abstract interface.
         *        箭头运算符，用于访问抽象接口。
         *
         * @return Pointer to the abstract interface
         *         指向抽象接口的指针
         */
        RAINY_NODISCARD rain_fn operator->() noexcept -> abstract_type * {
            return this;
        }

        /**
         * @brief Const arrow operator to access the abstract interface.
         *        常量箭头运算符，用于访问抽象接口。
         *
         * @return Const pointer to the abstract interface
         *         指向抽象接口的常量指针
         */
        RAINY_NODISCARD rain_fn operator->() const noexcept -> const abstract_type * {
            return this;
        }

        /**
         * @brief Gets the stored pointer as void*.
         *        获取存储的指针作为void*。
         *
         * @return The stored void pointer
         *         存储的void指针
         */
        rain_fn target_as_void_ptr() const noexcept -> void * {
            return _ptr;
        }

    protected:
        rain_fn reset_ptr(void *ptr) noexcept -> void {
            _ptr = ptr;
        }

    private:
        void *_ptr{};
        vtable_type vtable{};
    };
}

#endif
