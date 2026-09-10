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
#ifndef RAINY_ANNOTATIONS_MOON_HPP
#define RAINY_ANNOTATIONS_MOON_HPP
#include <rainy/core/platform.hpp>

#if RAINY_HAS_CXX26 && RAINY_HAS_CXX26_STATIC_REFLECTION
#include <rainy/core/collections/vector.hpp>
#include <rainy/core/text/string.hpp>
#include <rainy/core/text/string_view.hpp>

// reference https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2025/p3394r4.html
namespace rainy::annotations::moon {
    // clang-format off

    struct rename_tag { char const* new_name; };
    consteval rename_tag rename(core::text::string_view new_name = "") { return {std::define_static_string(new_name)}; }

    struct with_prefix_tag { char const* data; };
    consteval with_prefix_tag with_prefix(core::text::string_view prefix = "") { return {std::define_static_string(prefix)}; }

    struct with_suffix_tag { char const* data; };
    consteval with_suffix_tag with_suffix(core::text::string_view suffix = "") { return {std::define_static_string(suffix)}; }

    struct no_prefix_tag {};
    inline static constexpr auto no_prefix = no_prefix_tag{};

    struct no_suffix_tag {};
    inline static constexpr auto no_suffix = no_suffix_tag{};

    // clang-format on

    enum class named_style {
        none,
        snake_case,
        pascal,
        camel,
        all_caps,
        all_caps_no_underline
    };

    // clang-format off

    struct use_name_style { named_style style; };

    // clang-format on

    inline static constexpr auto no_name_style = use_name_style{named_style::none};
    inline static constexpr auto use_snake_case = use_name_style{named_style::snake_case};
    inline static constexpr auto use_pascal = use_name_style{named_style::pascal};
    inline static constexpr auto use_camel = use_name_style{named_style::camel};
    inline static constexpr auto all_caps = use_name_style{named_style::all_caps};
    inline static constexpr auto all_caps_no_underline = use_name_style{named_style::all_caps_no_underline};

    struct ignore_tag {};

    /**
     * @brief 指示moon静态反射在处理某些成员时，进行忽略
     * @attention 如果moon模块检测到有自定义的注册源，则此标签无效果
     */
    inline static constexpr auto ignore = ignore_tag{};
}

namespace rainy::core::meta::implements {
    // 预定义：通用处理工具

    template <typename Ty, std::meta::info Member>
    RAINY_CONSTEVAL rain_fn try_apply_prefix_and_suffix_tag(core::text::string_view &name) -> void {
        constexpr auto type_anno = annotations::make_type_anno<Ty>();
        constexpr auto member_anno = annotations::make_member_anno(Member);

        core::text::string_view prefix = {};
        core::text::string_view suffix = {};

        constexpr bool has_no_prefix = member_anno.has<annotations::moon::no_prefix_tag>();
        constexpr bool has_no_suffix = member_anno.has<annotations::moon::no_suffix_tag>();

        if constexpr (member_anno.has<annotations::moon::with_prefix_tag>() && !has_no_prefix) {
            // member 级别优先
            prefix = member_anno.fetch<annotations::moon::with_prefix_tag>().data;
        } else if constexpr (type_anno.template has<annotations::moon::with_prefix_tag>() && !has_no_prefix) {
            // 回退到 type 级别
            prefix = type_anno.template fetch<annotations::moon::with_prefix_tag>().data;
        }

        if constexpr (member_anno.has<annotations::moon::with_suffix_tag>() && !has_no_suffix) {
            // member 级别优先
            suffix = member_anno.fetch<annotations::moon::with_suffix_tag>().data;
        } else if constexpr (type_anno.template has<annotations::moon::with_suffix_tag>() && !has_no_suffix) {
            // 回退到 type 级别
            suffix = type_anno.template fetch<annotations::moon::with_suffix_tag>().data;
        }

        core::text::string str;
        str += prefix;
        str += name;
        str += suffix;
        name = std::define_static_string(str);
    }

    template <std::meta::info Member>
    RAINY_CONSTEVAL rain_fn try_apply_rename(core::text::string_view &name) noexcept -> void {
        constexpr auto anno = annotations::make_member_anno(Member);
        if constexpr (anno.has<annotations::moon::rename_tag>()) {
            name = anno.fetch<annotations::moon::rename_tag>().new_name;
        }
    }

    template <typename Ty, std::meta::info Member>
    RAINY_CONSTEVAL rain_fn try_apply_name_style(core::text::string_view &name) noexcept -> void {
        using annotations::moon::named_style;
        using annotations::moon::use_name_style;
        constexpr auto style = []() consteval -> named_style {
            constexpr auto type_anno = annotations::make_type_anno<Ty>();
            if constexpr (constexpr auto member_anno = annotations::make_member_anno(Member); member_anno.has<use_name_style>()) {
                return member_anno.fetch<use_name_style>().style;
            } else if constexpr (type_anno.template has<use_name_style>()) {
                return type_anno.template fetch<use_name_style>().style;
            }
            return named_style::none;
        }();

        if constexpr (style != named_style::none) {
            constexpr auto convert = [](core::text::string_view src, named_style s) consteval -> core::text::string {
                constexpr auto to_lower = [](const char ch) consteval -> char {
                    return (ch >= 'A' && ch <= 'Z') ? static_cast<char>(ch + 32) : ch;
                };
                constexpr auto to_upper = [](const char ch) consteval -> char {
                    return (ch >= 'a' && ch <= 'z') ? static_cast<char>(ch - 32) : ch;
                };

                collections::vector<core::text::string> words;

                {
                    core::text::string current;

                    for (std::size_t i = 0; i < src.size(); ++i) {
                        const char ch = src[i];
                        // 分隔符
                        if (ch == '_' || ch == '-') {
                            if (!current.empty()) {
                                words.push_back(current);
                                current.clear();
                            }
                            continue;
                        }
                        // 驼峰边界
                        if (!current.empty() && ch >= 'A' && ch <= 'Z') {

                            const char prev = src[i - 1]; // NOLINT

                            bool boundary =
                                (prev >= 'a' && prev <= 'z') || (i + 1 < src.size() && src[i + 1] >= 'a' && src[i + 1] <= 'z');

                            if (boundary) {
                                words.push_back(current);
                                current.clear();
                            }
                        }

                        current += ch;
                    }

                    if (!current.empty()) {
                        words.push_back(current);
                    }
                }
                core::text::string result;
                // NOLINTBEGIN
                switch (s) {
                    case named_style::snake_case:
                        for (std::size_t i = 0; i < words.size(); ++i) {
                            if (i != 0) {
                                result += '_';
                            }

                            for (const char ch: words[i]) {
                                result += to_lower(ch);
                            }
                        }
                        break;
                    case named_style::camel: {
                        // 第一个 word 全小写，后续每个 word 首字母大写
                        for (std::size_t i = 0; i < words.size(); ++i) {
                            if (i == 0) {
                                for (const char ch: words[i]) {
                                    result += to_lower(ch);
                                }
                            } else {
                                result += to_upper(words[i][0]);
                                for (std::size_t j = 1; j < words[i].size(); ++j) {
                                    result += to_lower(words[i][j]);
                                }
                            }
                        }
                        break;
                    }

                    case named_style::pascal: {
                        // 每个 word 首字母大写
                        for (const auto &w: words) {
                            result += to_upper(w[0]);
                            for (std::size_t j = 1; j < w.size(); ++j) {
                                result += to_lower(w[j]);
                            }
                        }
                        break;
                    }

                    case named_style::all_caps: {
                        for (const char ch: src) {
                            result += to_upper(ch);
                        }
                        break;
                    }

                    case named_style::all_caps_no_underline: {
                        for (const char ch: src) {
                            if (ch == '_') {
                                continue;
                            }
                            result += to_upper(ch);
                        }
                        break;
                    }

                    default:
                        result = core::text::string{src};
                        break;
                }
                // NOLINTEND
                return result;
            };

            name = std::define_static_string(convert(name, style));
        }
    }
}

#endif

#endif
