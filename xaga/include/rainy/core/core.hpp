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
#ifndef RAINY_CORE_HPP
#define RAINY_CORE_HPP
#include <rainy/core/platform.hpp>
#include <rainy/core/type_traits.hpp>
#include <rainy/core/lifetime_annotation.hpp>
#include <rainy/core/implements/bit.hpp>
#include <rainy/core/implements/raw_stringview.hpp>
#include <rainy/core/implements/collections/array.hpp>
#include <rainy/core/implements/collections/array_view.hpp>
#include <rainy/core/implements/exceptions.hpp>
#include <rainy/core/implements/source_location.hpp>
#include <rainy/core/expected.hpp>
#include <rainy/core/layer.hpp>

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
    RAINY_TOOLKIT_API void stl_internal_check(bool result, const internal_source_location &source_location = internal_source_location::current());
}

namespace rainy::core {
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

        temporary_buffer(Ty *buffer, difference_type count, difference_type capacity) :
            buffer_(buffer), count_(count), capacity_(capacity) {
            implements::stl_internal_check(buffer);
        }

        temporary_buffer(temporary_buffer &&right) noexcept :
            buffer_{utility::exchange(right.buffer_, nullptr)}, count_{utility::exchange(right.count_, 0)},
            capacity_{utility::exchange(right.capacity_, 0)} {
        }

        ~temporary_buffer() {
            return_buffer();
        }

        temporary_buffer &operator=(temporary_buffer &&right) noexcept {
            this->buffer_ = utility::exchange(right.buffer_, nullptr);
            this->count_ = utility::exchange(right.count_, 0);
            this->capacity_ = utility::exchange(right.capacity_, 0);
            return *this;
        }

        temporary_buffer(const temporary_buffer &) = delete;
        temporary_buffer &operator=(const temporary_buffer &) = delete;

        iterator begin() noexcept {
            return buffer_;
        }

        const_iterator begin() const noexcept {
            return buffer_;
        }

        iterator end() noexcept {
            return buffer_ + count_;
        }

        const_iterator end() const noexcept {
            return buffer_ + count_;
        }

        void return_buffer() {
            if (!buffer_) {
                return;
            }
            pal::deallocate(buffer_, sizeof(Ty) * count_, alignof(Ty));
            // 在释放后，对状态进行重置
            buffer_ = nullptr;
            count_ = capacity_ = 0;
        }

        void reallocate(const std::size_t realloc) {
            // 永远不可能保证线程安全
            if (buffer_) {
                if (realloc <= count_ || realloc == 0 || implements::in_range<std::size_t>(count_, capacity_, realloc)) {
                    count_ = static_cast<std::ptrdiff_t>(realloc);
                    return;
                }
            }
            rainy_const reallocated_size = static_cast<std::size_t>(realloc * 1.2f); // NOLINT
            auto new_buffer = static_cast<pointer>(pal::allocate(reallocated_size, alignof(Ty)));
            builtin::copy_memory(new_buffer, buffer_, capacity_);
            return_buffer(); // 需要归还
            implements::stl_internal_check(buffer_);
            buffer_ = new_buffer;
            capacity_ = reallocated_size; // NOLINT
            count_ = static_cast<std::ptrdiff_t>(realloc);
        }

        pointer get_buffer() noexcept {
            return buffer_;
        }

        const_pointer get_buffer() const noexcept {
            return buffer_;
        }

        explicit operator bool() const noexcept {
            return static_cast<bool>(buffer_);
        }

        operator pointer() noexcept { // NOLINT
            return buffer_;
        }

        operator const_pointer() const noexcept { // NOLINT
            return buffer_;
        }

    private:
        Ty *buffer_;
        std::ptrdiff_t count_;
        std::ptrdiff_t capacity_;
    };

    template <typename Ty>
    temporary_buffer<Ty> get_temporary_buffer(std::ptrdiff_t count) noexcept {
        rainy_const new_size = static_cast<std::ptrdiff_t>(count * 1.2f); // NOLINT
        return {static_cast<Ty *>(pal::allocate(implements::get_size_of_n<Ty>(new_size), alignof(Ty))), count, new_size};
    }

    template <typename Ty>
    void return_temporary_buffer(temporary_buffer<Ty> &buffer) {
        buffer.return_buffer();
    }
}

namespace rainy::core {
    template <typename... Test>
    RAINY_CONSTEXPR_BOOL check_format_type =
        (type_traits::type_relations::is_any_convertible_v<type_traits::other_trans::decay_t<Test>, char, int,
                                                                       double, void *, float, long, long long, unsigned int,
                                                                       unsigned long, unsigned long long, const char *> ||
         ...);

    template <typename... Test>
    RAINY_CONSTEXPR_BOOL check_wformat_type =
        (type_traits::type_relations::is_any_convertible_v<type_traits::other_trans::decay_t<Test>, char, int,
                                                                       double, void *, float, long, long long, unsigned int,
                                                                       unsigned long, unsigned long long, const wchar_t *> ||
         ...);
}

#if RAINY_USING_GCC
#include <rainy/core/gnu/source_location.hpp>
#endif

#ifndef RAINY_NODISCARD_RAW_PTR_ALLOC
#define RAINY_NODISCARD_RAW_PTR_ALLOC                                                                                                   \
    RAINY_NODISCARD_MSG("This function allocates memory and returns a raw pointer. "                                                    \
                           "Discarding the return value will cause a memory leak.")
#endif

/* 此部分宏由ChatGPT生成 */
#define RAINY_TO_TUPLE_EXPAND_ARGS(N) RAINY_TO_TUPLE_EXPAND_##N
#define RAINY_TO_TUPLE_EXPAND_1 _1
#define RAINY_TO_TUPLE_EXPAND_2 _1, _2
#define RAINY_TO_TUPLE_EXPAND_3 _1, _2, _3
#define RAINY_TO_TUPLE_EXPAND_4 _1, _2, _3, _4
#define RAINY_TO_TUPLE_EXPAND_5 _1, _2, _3, _4, _5
#define RAINY_TO_TUPLE_EXPAND_6 _1, _2, _3, _4, _5, _6
#define RAINY_TO_TUPLE_EXPAND_7 _1, _2, _3, _4, _5, _6, _7
#define RAINY_TO_TUPLE_EXPAND_8 _1, _2, _3, _4, _5, _6, _7, _8
#define RAINY_TO_TUPLE_EXPAND_9 _1, _2, _3, _4, _5, _6, _7, _8, _9
#define RAINY_TO_TUPLE_EXPAND_10 _1, _2, _3, _4, _5, _6, _7, _8, _9, _10
#define RAINY_TO_TUPLE_EXPAND_11 _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11
#define RAINY_TO_TUPLE_EXPAND_12 _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12
#define RAINY_TO_TUPLE_EXPAND_13 _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13
#define RAINY_TO_TUPLE_EXPAND_14 _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14
#define RAINY_TO_TUPLE_EXPAND_15 _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15
#define RAINY_TO_TUPLE_EXPAND_16 _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16
#define RAINY_TO_TUPLE_EXPAND_17 _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17
#define RAINY_TO_TUPLE_EXPAND_18 _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18
#define RAINY_TO_TUPLE_EXPAND_19 _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19
#define RAINY_TO_TUPLE_EXPAND_20 _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20
#define RAINY_TO_TUPLE_EXPAND_21 RAINY_TO_TUPLE_EXPAND_20, _21
#define RAINY_TO_TUPLE_EXPAND_22 RAINY_TO_TUPLE_EXPAND_21, _22
#define RAINY_TO_TUPLE_EXPAND_23 RAINY_TO_TUPLE_EXPAND_22, _23
#define RAINY_TO_TUPLE_EXPAND_24 RAINY_TO_TUPLE_EXPAND_23, _24
#define RAINY_TO_TUPLE_EXPAND_25 RAINY_TO_TUPLE_EXPAND_24, _25
#define RAINY_TO_TUPLE_EXPAND_26 RAINY_TO_TUPLE_EXPAND_25, _26
#define RAINY_TO_TUPLE_EXPAND_27 RAINY_TO_TUPLE_EXPAND_26, _27
#define RAINY_TO_TUPLE_EXPAND_28 RAINY_TO_TUPLE_EXPAND_27, _28
#define RAINY_TO_TUPLE_EXPAND_29 RAINY_TO_TUPLE_EXPAND_28, _29
#define RAINY_TO_TUPLE_EXPAND_30 RAINY_TO_TUPLE_EXPAND_29, _30
#define RAINY_TO_TUPLE_EXPAND_31 RAINY_TO_TUPLE_EXPAND_30, _31
#define RAINY_TO_TUPLE_EXPAND_32 RAINY_TO_TUPLE_EXPAND_31, _32
#define RAINY_TO_TUPLE_EXPAND_33 RAINY_TO_TUPLE_EXPAND_32, _33
#define RAINY_TO_TUPLE_EXPAND_34 RAINY_TO_TUPLE_EXPAND_33, _34
#define RAINY_TO_TUPLE_EXPAND_35 RAINY_TO_TUPLE_EXPAND_34, _35
#define RAINY_TO_TUPLE_EXPAND_36 RAINY_TO_TUPLE_EXPAND_35, _36
#define RAINY_TO_TUPLE_EXPAND_37 RAINY_TO_TUPLE_EXPAND_36, _37
#define RAINY_TO_TUPLE_EXPAND_38 RAINY_TO_TUPLE_EXPAND_37, _38
#define RAINY_TO_TUPLE_EXPAND_39 RAINY_TO_TUPLE_EXPAND_38, _39
#define RAINY_TO_TUPLE_EXPAND_40 RAINY_TO_TUPLE_EXPAND_39, _40
#define RAINY_TO_TUPLE_EXPAND_41 RAINY_TO_TUPLE_EXPAND_40, _41
#define RAINY_TO_TUPLE_EXPAND_42 RAINY_TO_TUPLE_EXPAND_41, _42
#define RAINY_TO_TUPLE_EXPAND_43 RAINY_TO_TUPLE_EXPAND_42, _43
#define RAINY_TO_TUPLE_EXPAND_44 RAINY_TO_TUPLE_EXPAND_43, _44
#define RAINY_TO_TUPLE_EXPAND_45 RAINY_TO_TUPLE_EXPAND_44, _45
#define RAINY_TO_TUPLE_EXPAND_46 RAINY_TO_TUPLE_EXPAND_45, _46
#define RAINY_TO_TUPLE_EXPAND_47 RAINY_TO_TUPLE_EXPAND_46, _47
#define RAINY_TO_TUPLE_EXPAND_48 RAINY_TO_TUPLE_EXPAND_47, _48
#define RAINY_TO_TUPLE_EXPAND_49 RAINY_TO_TUPLE_EXPAND_48, _49
#define RAINY_TO_TUPLE_EXPAND_50 RAINY_TO_TUPLE_EXPAND_49, _50
#define RAINY_TO_TUPLE_EXPAND_51 RAINY_TO_TUPLE_EXPAND_50, _51
#define RAINY_TO_TUPLE_EXPAND_52 RAINY_TO_TUPLE_EXPAND_51, _52
#define RAINY_TO_TUPLE_EXPAND_53 RAINY_TO_TUPLE_EXPAND_52, _53
#define RAINY_TO_TUPLE_EXPAND_54 RAINY_TO_TUPLE_EXPAND_53, _54
#define RAINY_TO_TUPLE_EXPAND_55 RAINY_TO_TUPLE_EXPAND_54, _55
#define RAINY_TO_TUPLE_EXPAND_56 RAINY_TO_TUPLE_EXPAND_55, _56
#define RAINY_TO_TUPLE_EXPAND_57 RAINY_TO_TUPLE_EXPAND_56, _57
#define RAINY_TO_TUPLE_EXPAND_58 RAINY_TO_TUPLE_EXPAND_57, _58
#define RAINY_TO_TUPLE_EXPAND_59 RAINY_TO_TUPLE_EXPAND_58, _59
#define RAINY_TO_TUPLE_EXPAND_60 RAINY_TO_TUPLE_EXPAND_59, _60
#define RAINY_TO_TUPLE_EXPAND_61 RAINY_TO_TUPLE_EXPAND_60, _61
#define RAINY_TO_TUPLE_EXPAND_62 RAINY_TO_TUPLE_EXPAND_61, _62
#define RAINY_TO_TUPLE_EXPAND_63 RAINY_TO_TUPLE_EXPAND_62, _63
#define RAINY_TO_TUPLE_EXPAND_64 RAINY_TO_TUPLE_EXPAND_63, _64
#define RAINY_TO_TUPLE_EXPAND_65 RAINY_TO_TUPLE_EXPAND_64, _65
#define RAINY_TO_TUPLE_EXPAND_66 RAINY_TO_TUPLE_EXPAND_65, _66
#define RAINY_TO_TUPLE_EXPAND_67 RAINY_TO_TUPLE_EXPAND_66, _67
#define RAINY_TO_TUPLE_EXPAND_68 RAINY_TO_TUPLE_EXPAND_67, _68
#define RAINY_TO_TUPLE_EXPAND_69 RAINY_TO_TUPLE_EXPAND_68, _69
#define RAINY_TO_TUPLE_EXPAND_70 RAINY_TO_TUPLE_EXPAND_69, _70
#define RAINY_TO_TUPLE_EXPAND_71 RAINY_TO_TUPLE_EXPAND_70, _71
#define RAINY_TO_TUPLE_EXPAND_72 RAINY_TO_TUPLE_EXPAND_71, _72
#define RAINY_TO_TUPLE_EXPAND_73 RAINY_TO_TUPLE_EXPAND_72, _73
#define RAINY_TO_TUPLE_EXPAND_74 RAINY_TO_TUPLE_EXPAND_73, _74
#define RAINY_TO_TUPLE_EXPAND_75 RAINY_TO_TUPLE_EXPAND_74, _75
#define RAINY_TO_TUPLE_EXPAND_76 RAINY_TO_TUPLE_EXPAND_75, _76
#define RAINY_TO_TUPLE_EXPAND_77 RAINY_TO_TUPLE_EXPAND_76, _77
#define RAINY_TO_TUPLE_EXPAND_78 RAINY_TO_TUPLE_EXPAND_77, _78
#define RAINY_TO_TUPLE_EXPAND_79 RAINY_TO_TUPLE_EXPAND_78, _79
#define RAINY_TO_TUPLE_EXPAND_80 RAINY_TO_TUPLE_EXPAND_79, _80

/* 用于创建初始化器 */
#define RAINY_INITIALIZER_LIST(N) RAINY_INITIALIZER_LIST_##N
#define RAINY_INITIALIZER_LIST_1 {}
#define RAINY_INITIALIZER_LIST_2 {} ,{}
#define RAINY_INITIALIZER_LIST_3 {} ,{} ,{}
#define RAINY_INITIALIZER_LIST_4 {} ,{} ,{} ,{}
#define RAINY_INITIALIZER_LIST_5 {} ,{} ,{} ,{} ,{}
#define RAINY_INITIALIZER_LIST_6 {} ,{} ,{} ,{} ,{} ,{}
#define RAINY_INITIALIZER_LIST_7 {} ,{} ,{} ,{} ,{} ,{} ,{}
#define RAINY_INITIALIZER_LIST_8 {} ,{} ,{} ,{} ,{} ,{} ,{}, {}
#define RAINY_INITIALIZER_LIST_9 {} ,{} ,{} ,{} ,{} ,{} ,{}, {}, {}
#define RAINY_INITIALIZER_LIST_10 {} ,{} ,{} ,{} ,{} ,{} ,{} ,{} ,{} ,{}
#define RAINY_INITIALIZER_LIST_11 {} ,{} ,{} ,{} ,{} ,{} ,{} ,{} ,{} ,{}, {}
#define RAINY_INITIALIZER_LIST_12 {} ,{} ,{} ,{} ,{} ,{} ,{} ,{} ,{} ,{} ,{} ,{}
#define RAINY_INITIALIZER_LIST_13 {} ,{} ,{} ,{} ,{} ,{} ,{} ,{} ,{} ,{} ,{} ,{} ,{}
#define RAINY_INITIALIZER_LIST_14 {} ,{} ,{} ,{} ,{} ,{} ,{} ,{} ,{} ,{} ,{} ,{} ,{} ,{}
#define RAINY_INITIALIZER_LIST_15 {} ,{} ,{} ,{} ,{} ,{} ,{} ,{} ,{} ,{} ,{} ,{} ,{} ,{} ,{}
#define RAINY_INITIALIZER_LIST_16 {} ,{} ,{} ,{} ,{} ,{} ,{} ,{} ,{} ,{} ,{} ,{} ,{} ,{} ,{} ,{}
#define RAINY_INITIALIZER_LIST_17 {} ,{} ,{} ,{} ,{} ,{} ,{} ,{} ,{} ,{} ,{} ,{} ,{} ,{} ,{} ,{} ,{}
#define RAINY_INITIALIZER_LIST_18 {} ,{} ,{} ,{} ,{} ,{} ,{} ,{} ,{} ,{} ,{} ,{} ,{} ,{} ,{} ,{} ,{} ,{}
#define RAINY_INITIALIZER_LIST_19 {} ,{} ,{} ,{} ,{} ,{} ,{} ,{} ,{} ,{} ,{} ,{} ,{} ,{} ,{} ,{} ,{} ,{} ,{}
#define RAINY_INITIALIZER_LIST_20 {} ,{} ,{} ,{} ,{} ,{} ,{} ,{} ,{} ,{} ,{} ,{} ,{} ,{} ,{} ,{} ,{} ,{} ,{} ,{}
#define RAINY_INITIALIZER_LIST_21 RAINY_INITIALIZER_LIST_20 ,{}
#define RAINY_INITIALIZER_LIST_22 RAINY_INITIALIZER_LIST_21 ,{}
#define RAINY_INITIALIZER_LIST_23 RAINY_INITIALIZER_LIST_22 ,{}
#define RAINY_INITIALIZER_LIST_24 RAINY_INITIALIZER_LIST_23 ,{}
#define RAINY_INITIALIZER_LIST_25 RAINY_INITIALIZER_LIST_24 ,{}
#define RAINY_INITIALIZER_LIST_26 RAINY_INITIALIZER_LIST_25 ,{}
#define RAINY_INITIALIZER_LIST_27 RAINY_INITIALIZER_LIST_26 ,{}
#define RAINY_INITIALIZER_LIST_28 RAINY_INITIALIZER_LIST_27 ,{}
#define RAINY_INITIALIZER_LIST_29 RAINY_INITIALIZER_LIST_28 ,{}
#define RAINY_INITIALIZER_LIST_30 RAINY_INITIALIZER_LIST_29 ,{}
#define RAINY_INITIALIZER_LIST_31 RAINY_INITIALIZER_LIST_30 ,{}
#define RAINY_INITIALIZER_LIST_32 RAINY_INITIALIZER_LIST_31 ,{}
#define RAINY_INITIALIZER_LIST_33 RAINY_INITIALIZER_LIST_32 ,{}
#define RAINY_INITIALIZER_LIST_34 RAINY_INITIALIZER_LIST_33 ,{}
#define RAINY_INITIALIZER_LIST_35 RAINY_INITIALIZER_LIST_34 ,{}
#define RAINY_INITIALIZER_LIST_36 RAINY_INITIALIZER_LIST_35 ,{}
#define RAINY_INITIALIZER_LIST_37 RAINY_INITIALIZER_LIST_36 ,{}
#define RAINY_INITIALIZER_LIST_38 RAINY_INITIALIZER_LIST_37 ,{}
#define RAINY_INITIALIZER_LIST_39 RAINY_INITIALIZER_LIST_38 ,{}
#define RAINY_INITIALIZER_LIST_40 RAINY_INITIALIZER_LIST_39 ,{}
#define RAINY_INITIALIZER_LIST_41 RAINY_INITIALIZER_LIST_40 ,{}
#define RAINY_INITIALIZER_LIST_42 RAINY_INITIALIZER_LIST_41 ,{}
#define RAINY_INITIALIZER_LIST_43 RAINY_INITIALIZER_LIST_42 ,{}
#define RAINY_INITIALIZER_LIST_44 RAINY_INITIALIZER_LIST_43 ,{}
#define RAINY_INITIALIZER_LIST_45 RAINY_INITIALIZER_LIST_44 ,{}
#define RAINY_INITIALIZER_LIST_46 RAINY_INITIALIZER_LIST_45 ,{}
#define RAINY_INITIALIZER_LIST_47 RAINY_INITIALIZER_LIST_46 ,{}
#define RAINY_INITIALIZER_LIST_48 RAINY_INITIALIZER_LIST_47 ,{}
#define RAINY_INITIALIZER_LIST_49 RAINY_INITIALIZER_LIST_48 ,{}
#define RAINY_INITIALIZER_LIST_50 RAINY_INITIALIZER_LIST_49 ,{}
#define RAINY_INITIALIZER_LIST_51 RAINY_INITIALIZER_LIST_50 ,{}
#define RAINY_INITIALIZER_LIST_52 RAINY_INITIALIZER_LIST_51 ,{}
#define RAINY_INITIALIZER_LIST_53 RAINY_INITIALIZER_LIST_52 ,{}
#define RAINY_INITIALIZER_LIST_54 RAINY_INITIALIZER_LIST_53 ,{}
#define RAINY_INITIALIZER_LIST_55 RAINY_INITIALIZER_LIST_54 ,{}
#define RAINY_INITIALIZER_LIST_56 RAINY_INITIALIZER_LIST_55 ,{}
#define RAINY_INITIALIZER_LIST_57 RAINY_INITIALIZER_LIST_56 ,{}
#define RAINY_INITIALIZER_LIST_58 RAINY_INITIALIZER_LIST_57 ,{}
#define RAINY_INITIALIZER_LIST_59 RAINY_INITIALIZER_LIST_58 ,{}
#define RAINY_INITIALIZER_LIST_60 RAINY_INITIALIZER_LIST_59 ,{}
#define RAINY_INITIALIZER_LIST_61 RAINY_INITIALIZER_LIST_60 ,{}
#define RAINY_INITIALIZER_LIST_62 RAINY_INITIALIZER_LIST_61 ,{}
#define RAINY_INITIALIZER_LIST_63 RAINY_INITIALIZER_LIST_62 ,{}
#define RAINY_INITIALIZER_LIST_64 RAINY_INITIALIZER_LIST_63 ,{}
#define RAINY_INITIALIZER_LIST_65 RAINY_INITIALIZER_LIST_64 ,{}
#define RAINY_INITIALIZER_LIST_66 RAINY_INITIALIZER_LIST_65 ,{}
#define RAINY_INITIALIZER_LIST_67 RAINY_INITIALIZER_LIST_66 ,{}
#define RAINY_INITIALIZER_LIST_68 RAINY_INITIALIZER_LIST_67 ,{}
#define RAINY_INITIALIZER_LIST_69 RAINY_INITIALIZER_LIST_68 ,{}
#define RAINY_INITIALIZER_LIST_70 RAINY_INITIALIZER_LIST_69 ,{}
#define RAINY_INITIALIZER_LIST_71 RAINY_INITIALIZER_LIST_70 ,{}
#define RAINY_INITIALIZER_LIST_72 RAINY_INITIALIZER_LIST_71 ,{}
#define RAINY_INITIALIZER_LIST_73 RAINY_INITIALIZER_LIST_72 ,{}
#define RAINY_INITIALIZER_LIST_74 RAINY_INITIALIZER_LIST_73 ,{}
#define RAINY_INITIALIZER_LIST_75 RAINY_INITIALIZER_LIST_74 ,{}
#define RAINY_INITIALIZER_LIST_76 RAINY_INITIALIZER_LIST_75 ,{}
#define RAINY_INITIALIZER_LIST_77 RAINY_INITIALIZER_LIST_76 ,{}
#define RAINY_INITIALIZER_LIST_78 RAINY_INITIALIZER_LIST_77 ,{}
#define RAINY_INITIALIZER_LIST_79 RAINY_INITIALIZER_LIST_78 ,{}
#define RAINY_INITIALIZER_LIST_80 RAINY_INITIALIZER_LIST_79 ,{}

#define RAINY_DECLARE_SIGNLE_INSTANCE(CLASSNAME) static CLASSNAME &instance() noexcept { static CLASSNAME instance;return instance;}

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

namespace rainy::foundation::system::memory {
    template <typename Ty>
    struct default_deleter {
        constexpr default_deleter() noexcept = default;

        constexpr default_deleter(utility::placeholder_t) noexcept {
        }

        template <typename U,
                  typename = type_traits::other_trans::enable_if_t<type_traits::type_relations::is_convertible_v<U *, Ty *>>>
        constexpr explicit default_deleter(const default_deleter &) noexcept {
        }

        RAINY_CONSTEXPR20 void operator()(const Ty *resource) const noexcept {
            static_assert(0 < sizeof(Ty), "can't delete an incomplete type"); // NOLINT
            delete resource;
        }
    };

    template <typename Ty>
    struct default_deleter<Ty[]> {
        constexpr default_deleter() = default;

        constexpr default_deleter(utility::placeholder_t) noexcept {
        }

        template <typename U,
                  type_traits::other_trans::enable_if_t<type_traits::type_relations::is_convertible_v<U (*)[], Ty (*)[]>, int> = 0>
        constexpr explicit default_deleter(const default_deleter &) noexcept {
        }

        template <typename U, type_traits::other_trans::enable_if_t<std::is_convertible_v<U (*)[], Ty (*)[]>, int> = 0>
        RAINY_CONSTEXPR20 void operator()(const U *resource) const noexcept {
            static_assert(!std::is_abstract_v<U>, "can't delete an incomplete type");
            delete[] resource;
        }
    };

    template <typename Ty>
    struct no_delete {
        constexpr no_delete() noexcept = default;

        constexpr no_delete(utility::placeholder_t) noexcept {
        }

        template <typename U,
                  typename = type_traits::other_trans::enable_if_t<type_traits::type_relations::is_convertible_v<U *, Ty *>>>
        constexpr explicit no_delete(const no_delete &) noexcept {
        }

        RAINY_CONSTEXPR20 void operator()(const Ty *) const noexcept {
            static_assert(0 < sizeof(Ty), "can't delete an incomplete type"); // NOLINT
        }
    };

    template <typename Ty>
    struct no_delete<Ty[]> {
        constexpr no_delete() = default;

        constexpr no_delete(utility::placeholder_t) noexcept {
        }

        template <typename U,
                  type_traits::other_trans::enable_if_t<type_traits::type_relations::is_convertible_v<U (*)[], Ty (*)[]>, int> = 0>
        constexpr explicit no_delete(const no_delete &) noexcept {
        }

        template <typename U,
                  type_traits::other_trans::enable_if_t<type_traits::type_relations::is_convertible_v<U (*)[], Ty (*)[]>, int> = 0>
        RAINY_CONSTEXPR20 void operator()(const U *) const noexcept {
            static_assert(!std::is_abstract_v<U>, "can't delete an incomplete type");
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
    template <typename Ty, typename Dx = foundation::system::memory::default_deleter<Ty>>
    class scope_guard {
    public:
        using value_type = Ty;
        using pointer = value_type *;
        using const_pointer = const value_type *;

        static_assert(!type_traits::type_relations::is_same_v<Ty, std::nullptr_t>, "Ty should not be nullptr_t");

        RAINY_CONSTEXPR20 scope_guard() = default;

        scope_guard(const scope_guard &) = delete;
        scope_guard(scope_guard &&) = delete;
        scope_guard &operator=(const scope_guard &) = delete;
        scope_guard &operator=(scope_guard &&) = delete;
        scope_guard(std::nullptr_t) = delete;

        RAINY_CONSTEXPR20 scope_guard(pointer data) : data_({}, data) {
            check_null(data);
        }

        RAINY_CONSTEXPR20 scope_guard(pointer data, Dx deleter) : data_(deleter, data) {
            check_null(data);
        }

        template <typename Uty, type_traits::other_trans::enable_if_t<type_traits::type_relations::is_convertible_v<Uty, value_type> &&
                                                                          implements::is_deleter_invocable_v<Dx, Ty>,
                                                                      int> = 0>
        RAINY_CONSTEXPR20 scope_guard(Uty *data) : data_({}, data) {
            check_null(data);
        }

        template <typename Uty, type_traits::other_trans::enable_if_t<type_traits::type_relations::is_convertible_v<Uty, value_type> &&
                                                                          implements::is_deleter_invocable_v<Dx, Ty>,
                                                                      int> = 0>
        RAINY_CONSTEXPR20 scope_guard(Uty *data, Dx deleter) : data_(deleter, data) {
            check_null(data);
        }

        RAINY_CONSTEXPR20 ~scope_guard() {
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

        template <type_traits::other_trans::enable_if_t<implements::is_deleter_invocable_v<Dx, Ty>,int> = 0>
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
}

namespace rainy::utility {
    template <typename Key>
    struct hash;

    namespace implements {
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

    template <typename key>
    struct hash : implements::hash_enable_if<
                      key, !type_traits::type_properties::is_const_v<key> && !type_traits::type_properties::is_volatile_v<key> &&
                               (type_traits::primary_types::is_enum_v<key> || type_traits::primary_types::is_integral_v<key> ||
                                type_traits::primary_types::is_pointer_v<key>)> {
        static std::size_t hash_this_val(const key &keyval) noexcept {
            return implements::hash_representation(keyval);
        }
    };

    template <>
    struct hash<float> {
        using argument_type = float;
        using result_type = std::size_t;

        RAINY_AINLINE_NODISCARD result_type operator()(argument_type val) const {
            return implements::hash_representation(val == 0.0f ? 0.0f : val);
        }
    };

    template <>
    struct hash<double> {
        using argument_type = double;
        using result_type = std::size_t;

        RAINY_AINLINE_NODISCARD result_type operator()(argument_type val) const {
            return implements::hash_representation(val == 0.0 ? 0.0 : val);
        }
    };

    template <>
    struct hash<long double> {
        using argument_type = long double;
        using result_type = std::size_t;

        RAINY_AINLINE_NODISCARD result_type operator()(argument_type val) const {
            return implements::hash_representation(val == 0.0L ? 0.0L : val);
        }
    };

    template <>
    struct hash<std::nullptr_t> {
        using argument_type = std::nullptr_t;
        using result_type = std::size_t;

        static std::size_t hash_this_val(std::nullptr_t) noexcept {
            void *null_pointer{};
            return implements::hash_representation(null_pointer);
        }

        RAINY_AINLINE_NODISCARD result_type operator()(std::nullptr_t) const {
            void *null_pointer{};
            return implements::hash_representation(null_pointer);
        }
    };

    template <typename CharType, typename Traits>
    struct hash<std::basic_string_view<CharType, Traits>> {
        using argument_type = std::basic_string_view<CharType, Traits>;
        using result_type = std::size_t;

        static std::size_t hash_this_val(const argument_type &val) noexcept {
            return implements::hash_array_representation(val.data(), val.size());
        }

        RAINY_AINLINE_NODISCARD result_type operator()(argument_type val) const {
            return hash_this_val(val);
        }
    };

    template <typename CharType,typename Traits ,typename Alloc>
    struct hash<std::basic_string<CharType, Traits, Alloc>> {
        using argument_type = std::basic_string<CharType, Traits, Alloc>;
        using result_type = std::size_t;

        static std::size_t hash_this_val(const argument_type &val) noexcept {
            return implements::hash_array_representation(val.data(), val.size());
        }

        RAINY_AINLINE_NODISCARD result_type operator()(const argument_type &val) const {
            return hash_this_val(val);
        }
    };

    template <typename Ty, typename = void>
    struct is_support_standard_hasher_available : type_traits::helper::false_type {};

    template <typename Ty>
    struct is_support_standard_hasher_available<
        Ty, type_traits::other_trans::void_t<decltype(utility::declval<std::hash<Ty>>()(utility::declval<Ty>()))>>
        : type_traits::helper::true_type {};

    template <typename Ty, typename = void>
    struct is_support_rainytoolkit_hasher_available : type_traits::helper::false_type {};

    template <typename Ty>
    struct is_support_rainytoolkit_hasher_available<
        Ty, type_traits::other_trans::void_t<decltype(utility::declval<std::hash<Ty>>()(utility::declval<Ty>()))>>
        : type_traits::helper::true_type {};
}

namespace rainy::utility {
    struct monostate {};
}

namespace rainy::utility::implements {
    template <typename Ty>
    constexpr void verify_range(const Ty *const first, const Ty *const last) noexcept {
        // special case range verification for pointers
        expects(first <= last, "transposed pointer range");
    }

    template <typename Iter, typename = void>
    constexpr bool allow_inheriting_unwrap_v = true;

    template <typename Iter>
    constexpr bool allow_inheriting_unwrap_v<Iter, type_traits::other_trans::void_t<typename Iter::prevent_inheriting_unwrap>> =
        type_traits::implements::is_same_v<Iter, typename Iter::prevent_inheriting_unwrap>;

    template <typename Iter, typename Sentinel = Iter, typename = void>
    constexpr bool range_verifiable_v = false;

    template <typename Iter, typename Sentinel>
    constexpr bool range_verifiable_v<
        Iter, Sentinel,
        type_traits::implements::void_t<decltype(verify_range(declval<const Iter &>(), declval<const Sentinel &>()))>> =
        allow_inheriting_unwrap_v<Iter>;

    template <typename iter, typename sentinel>
    constexpr void adl_verify_range(const iter &first, const sentinel &last) {
        // check that [first, last) forms an iterator range
        if constexpr (type_traits::implements::_is_pointer_v<iter> && type_traits::implements::_is_pointer_v<sentinel>) {
            expects(first <= last, "transposed pointer range");
        } else if constexpr (range_verifiable_v<iter, sentinel>) {
            verify_range(first, last);
        }
    }
}

namespace rainy::utility {
    template <typename Iter>
    RAINY_NODISCARD constexpr std::ptrdiff_t distance(Iter first, Iter last) {
        if constexpr (std::is_same_v<typename std::iterator_traits<Iter>::iterator_category, std::random_access_iterator_tag>) {
            return last - first; // assume the iterator will do debug checking
        } else {
            implements::adl_verify_range(first, last);
            auto ufirst = addressof(*first);
            const auto ulast = addressof(*(last - 1)) + 1;
            std::ptrdiff_t off = 0;
            for (; ufirst != ulast; ++ufirst) {
                ++off;
            }
            return off;
        }
    }
}

namespace rainy::core::algorithm {
    template <typename InputIter, typename OutIter>
    constexpr OutIter copy(InputIter begin, InputIter end, OutIter dest) noexcept(
        std::is_nothrow_copy_constructible_v<type_traits::other_trans::conditional_t<
            type_traits::implements::_is_pointer_v<InputIter>, type_traits::pointer_modify::remove_pointer_t<InputIter>,
            typename utility::iterator_traits<InputIter>::value_type>>) {
        using value_type = typename utility::iterator_traits<InputIter>::value_type;
        if (begin == end || (end - 1) == begin) {
            return dest; // 不进行复制
        }
#if RAINY_HAS_CXX20
        if (std::is_constant_evaluated()) {
            auto input_begin = utility::addressof(*begin);
            auto input_end = utility::addressof(*(end - 1)) + 1; // 防止MSVC编译器的DEBUG功能导致此处无法运作
            auto out_dest = utility::addressof(*dest);
            for (auto i = input_begin; i != input_end; ++i) {
                *out_dest = *i;
            }
            return out_dest;
        }
#endif
        if constexpr (std::is_standard_layout_v<value_type> && std::is_trivial_v<value_type>) {
            const auto input_begin = utility::addressof(*begin);
            const auto input_end = utility::addressof(*(end - 1)) + 1; // 防止MSVC编译器的DEBUG功能导致此处无法运作
            auto out_dest = utility::addressof(*dest);
            std::memcpy(out_dest, input_begin, sizeof(value_type) * utility::distance(input_begin, input_end));
        } else {
            for (InputIter i = begin; begin != end; ++i, ++dest) {
                *dest = *i;
            }
        }
        return dest;
    }

    template <typename InputIter, typename OutIter>
    constexpr OutIter copy_n(InputIter begin, const std::size_t count, OutIter dest) noexcept(
        std::is_nothrow_copy_constructible_v<type_traits::other_trans::conditional_t<
            type_traits::implements::_is_pointer_v<InputIter>, type_traits::pointer_modify::remove_pointer_t<InputIter>,
            typename utility::iterator_traits<InputIter>::value_type>>) {
        using value_type = typename utility::iterator_traits<InputIter>::value_type;
        if (count == 0) {
            return dest; // 不进行复制
        }
#if RAINY_HAS_CXX20
        if (std::is_constant_evaluated()) {
            // 在C++20中，如果我们在常量环境求值，我们不需要考虑过多运行时优化
            auto input_begin = utility::addressof(*begin);
            for (std::size_t i = 0; i < count; ++i, ++input_begin, ++dest) {
                *dest = *input_begin;
            }
        } else
#endif
        {
            if constexpr (std::is_standard_layout_v<value_type> && std::is_trivial_v<value_type>) {
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


    template <typename InputIter, typename OutIter, typename Fx>
    constexpr OutIter transform(InputIter begin, InputIter end, OutIter dest, Fx func) noexcept(
        std::is_nothrow_copy_assignable_v<type_traits::other_trans::conditional_t<
            type_traits::implements::_is_pointer_v<InputIter>, type_traits::pointer_modify::remove_pointer_t<InputIter>,
            typename utility::iterator_traits<InputIter>::value_type>>) {
        for (InputIter iter = begin; iter != end; ++iter, ++dest) {
            *dest = func(*iter);
        }
        return dest;
    }

    template <typename InputIter, typename OutIter, typename Fx>
    constexpr OutIter transform(InputIter begin1, InputIter end1, InputIter begin2, OutIter dest, Fx func) noexcept(
        std::is_nothrow_copy_assignable_v<type_traits::other_trans::conditional_t<
            type_traits::implements::_is_pointer_v<InputIter>, type_traits::pointer_modify::remove_pointer_t<InputIter>,
            typename utility::iterator_traits<InputIter>::value_type>>) {
        if (begin1 == end1 || (end1 - 1) == begin1) {
            return dest;
        }
        for (InputIter iter = begin1; iter != end1; ++iter, ++dest, ++begin2) {
            *dest = func(*iter, *begin2);
        }
        return dest;
    }
}

namespace rainy::utility {
    template <typename Iter>
    class sub_range {
    public:
        sub_range(Iter begin, Iter end) : begin_{begin}, end_{end} {
        }

        auto begin() const noexcept {
            return begin_;
        }

        auto end() const noexcept {
            return end_;
        }

    private:
        Iter begin_;
        Iter end_;
    };
}

#endif
