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
#ifndef RAINY_CORE_MEMORY_TEMPORARY_BUFFER_HPP
#define RAINY_CORE_MEMORY_TEMPORARY_BUFFER_HPP
#include <atomic>
#include <new>
#include <rainy/core/platform.hpp>
#include <rainy/core/type_traits.hpp>

namespace rainy::core::memory {
    /**
     * \lang english
     * @brief A temporary buffer for POD types with automatic management.
     *         Provides RAII-style management of temporary memory buffers.
     *
     * @tparam Ty The element type, must be a plain old data type
     *
     * \lang simp-chinese
     * @brief 用于POD类型的临时缓冲区，支持自动管理。
     *         提供RAII风格的临时内存缓冲区管理。
     *
     * @tparam Ty 元素类型，必须是POD类型
     */
    template <typename Ty>
    class temporary_buffer {
    public:
        static_assert(type_traits::properties::is_pod_v<Ty>, "Ty must be a plain old data type!");

        using value_type = Ty;
        using pointer = value_type *;
        using const_pointer = const value_type *;
        using iterator = pointer;
        using const_iterator = const_pointer;
        using difference_type = std::ptrdiff_t;

        template <typename UTy>
        friend rain_fn get_temporary_buffer(std::ptrdiff_t count) noexcept -> temporary_buffer<UTy>;

        /**
         * \lang english
         * @brief Move constructor.
         *
         * @param right The temporary_buffer to move from
         *
         * \lang simp-chinese
         * @brief 移动构造函数。
         *
         * @param right 要移动的 temporary_buffer
         */
        temporary_buffer(temporary_buffer &&right) noexcept :
            buffer_{utility::exchange(right.buffer_, nullptr)}, count_{utility::exchange(right.count_, 0)},
            capacity_{utility::exchange(right.capacity_, 0)} {
        }

        /**
         * \lang english
         * @brief Destructor that automatically returns the buffer.
         *
         * \lang simp-chinese
         * @brief 析构函数，自动归还缓冲区。
         */
        ~temporary_buffer() {
            return_buffer();
        }

        /**
         * \lang english
         * @brief Move assignment operator.
         *
         * @param right The temporary_buffer to move from
         * @return Reference to this object
         *
         * \lang simp-chinese
         * @brief 移动赋值运算符。
         *
         * @param right 要移动的 temporary_buffer
         * @return 此对象的引用
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
         * \lang english
         * @brief Returns an iterator to the beginning.
         *
         * @return Iterator to the first element
         *
         * \lang simp-chinese
         * @brief 返回指向起始的迭代器。
         *
         * @return 指向第一个元素的迭代器
         */
        rain_fn begin() noexcept -> iterator {
            return buffer_;
        }

        /**
         * \lang english
         * @brief Returns a const iterator to the beginning.
         *
         * @return Const iterator to the first element
         *
         * \lang simp-chinese
         * @brief 返回指向起始的常量迭代器。
         *
         * @return 指向第一个元素的常量迭代器
         */
        rain_fn begin() const noexcept -> const_iterator {
            return buffer_;
        }

        /**
         * \lang english
         * @brief Returns an iterator to the end.
         *
         * @return Iterator to one past the last element
         *
         * \lang simp-chinese
         * @brief 返回指向末尾的迭代器。
         *
         * @return 指向最后一个元素之后位置的迭代器
         */
        rain_fn end() noexcept -> iterator {
            return buffer_ + count_;
        }

        /**
         * \lang english
         * @brief Returns a const iterator to the end.
         *
         * @return Const iterator to one past the last element
         *
         * \lang simp-chinese
         * @brief 返回指向末尾的常量迭代器。
         *
         * @return 指向最后一个元素之后位置的常量迭代器
         */
        rain_fn end() const noexcept -> const_iterator {
            return buffer_ + count_;
        }

        /**
         * \lang english
         * @brief Returns the buffer to the system.
         *
         * \lang simp-chinese
         * @brief 将缓冲区归还给系统。
         */
        rain_fn return_buffer() -> void {
            if (!buffer_) {
                return;
            }
            layer::deallocate(buffer_, sizeof(Ty) * count_, alignof(Ty));
            buffer_ = nullptr;
            count_ = capacity_ = 0;
        }

        /**
         * \lang english
         * @brief Reallocates the buffer to a new size.
         *
         * @param realloc The requested number of elements
         *
         * \lang simp-chinese
         * @brief 重新分配缓冲区到新大小。
         *
         * @param realloc 请求的元素数量
         */
        rain_fn reallocate(const std::size_t realloc) -> void {
            if (buffer_) {
                if (realloc <= count_ || realloc == 0 || core::implements::in_range<std::size_t>(count_, capacity_, realloc)) {
                    count_ = static_cast<std::ptrdiff_t>(realloc);
                    return;
                }
            }
            rainy_const reallocated_size = static_cast<std::size_t>(realloc);
            auto new_buffer = static_cast<pointer>(layer::allocate(reallocated_size, alignof(Ty)));
            builtin::copy_memory(new_buffer, buffer_, core::implements::get_size_of_n<Ty>(count_));
            return_buffer();
            core::implements::stl_internal_check(new_buffer);
            buffer_ = new_buffer;
            capacity_ = reallocated_size;
            count_ = static_cast<std::ptrdiff_t>(realloc);
        }

        /**
         * \lang english
         * @brief Gets the raw buffer pointer.
         *
         * @return Pointer to the buffer
         *
         * \lang simp-chinese
         * @brief 获取原始缓冲区指针。
         *
         * @return 指向缓冲区的指针
         */
        rain_fn get_buffer() noexcept -> pointer {
            return buffer_;
        }

        /**
         * \lang english
         * @brief Gets the raw buffer pointer (const version).
         *
         * @return Const pointer to the buffer
         *
         * \lang simp-chinese
         * @brief 获取原始缓冲区指针（常量版本）。
         *
         * @return 指向缓冲区的常量指针
         */
        rain_fn get_buffer() const noexcept -> const_pointer {
            return buffer_;
        }

        /**
         * \lang english
         * @brief Checks if the buffer is valid.
         *
         * @return true if buffer is valid, false otherwise
         *
         * \lang simp-chinese
         * @brief 检查缓冲区是否有效。
         *
         * @return 如果缓冲区有效则为true，否则为false
         */
        explicit operator bool() const noexcept {
            return static_cast<bool>(buffer_);
        }

        /**
         * \lang english
         * @brief Implicit conversion to pointer.
         *
         * @return Pointer to the buffer
         *
         * \lang simp-chinese
         * @brief 到指针的隐式转换。
         *
         * @return 指向缓冲区的指针
         */
        operator pointer() noexcept {
            return buffer_;
        }

        /**
         * \lang english
         * @brief Implicit conversion to const pointer.
         *
         * @return Const pointer to the buffer
         *
         * \lang simp-chinese
         * @brief 到常量指针的隐式转换。
         *
         * @return 指向缓冲区的常量指针
         */
        operator const_pointer() const noexcept {
            return buffer_;
        }

    private:
        temporary_buffer(Ty *buffer, difference_type count, difference_type capacity) :
            buffer_(buffer), count_(count), capacity_(capacity) {
            core::implements::stl_internal_check(buffer);
        }

        Ty *buffer_;
        std::ptrdiff_t count_;
        std::ptrdiff_t capacity_;
    };

    /**
     * \lang english
     * @brief Obtains a temporary buffer for a given number of elements.
     *
     * @tparam Ty The element type (must be POD)
     * @param count The requested number of elements
     * @return A temporary_buffer instance managing the allocated memory
     *
     * \lang simp-chinese
     * @brief 获取用于指定数量元素的临时缓冲区。
     *
     * @tparam Ty 元素类型（必须是POD）
     * @param count 请求的元素数量
     * @return 管理已分配内存的 temporary_buffer 实例
     */
    template <typename Ty>
    rain_fn get_temporary_buffer(std::ptrdiff_t count) noexcept -> temporary_buffer<Ty> {
        rainy_const new_size = static_cast<std::ptrdiff_t>(count);
        return {static_cast<Ty *>(layer::allocate(core::implements::get_size_of_n<Ty>(new_size), alignof(Ty))), count, new_size};
    }

    /**
     * \lang english
     * @brief Explicitly returns a temporary buffer.
     *
     * @tparam Ty The element type
     * @param buffer The buffer to return
     *
     * \lang simp-chinese
     * @brief 显式归还临时缓冲区。
     *
     * @tparam Ty 元素类型
     * @param buffer 要归还的缓冲区
     */
    template <typename Ty>
    rain_fn return_temporary_buffer(temporary_buffer<Ty> &buffer) -> void {
        buffer.return_buffer();
    }
}

#endif
