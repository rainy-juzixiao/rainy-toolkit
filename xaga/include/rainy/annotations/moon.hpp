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
 * \lang chinese
 * @file moon.hpp
 * @brief 适用于meta/moon模块的注解
 * @brief 此文件定义了一套注解，用于控制meta/moon模块的能力
 *
 * @attention 必须启用C++26，且编译器必须支持static reflection特性
 *
 * @author rainy-juzixiao
 */
#ifndef RAINY_ANNOTATIONS_MOON_HPP
#define RAINY_ANNOTATIONS_MOON_HPP
#include <rainy/core/core.hpp>

#if RAINY_HAS_CXX26 && RAINY_HAS_CXX26_STATIC_REFLECTION

// reference https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2025/p3394r4.html
namespace rainy::annotations::moon {
    struct rename_tag { char const* new_name; };
    consteval rename_tag rename(std::string_view new_name = "") { return {std::define_static_string(new_name)}; }

    struct with_prefix_tag { char const* data; };
    consteval with_prefix_tag with_prefix(std::string_view prefix = "") { return {std::define_static_string(prefix)}; }

    struct with_suffix_tag { char const* data; };
    consteval with_suffix_tag with_suffix(std::string_view suffix = "") { return {std::define_static_string(suffix)}; }

    struct no_prefix_tag {};
    inline static constexpr auto no_prefix = no_prefix_tag{};

    struct no_suffix_tag {};
    inline static constexpr auto no_suffix = no_suffix_tag{};

    enum class named_style {
        snake_case,
        pascal,
        camel
    };

    struct use_name_style { named_style style; };

    inline static constexpr auto use_snake_case = named_style::snake_case;
    inline static constexpr auto use_pascal = named_style::pascal;
    inline static constexpr auto use_camel = named_style::camel;

    struct ignore_tag {};
    inline static constexpr auto ignore = ignore_tag{};
}

#endif

#endif
