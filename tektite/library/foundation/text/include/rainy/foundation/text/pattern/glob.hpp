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
#ifndef RAINY_FOUNDATION_TEXT_GLOB_HPP
#define RAINY_FOUNDATION_TEXT_GLOB_HPP
#include <rainy/core/collections/vector.hpp>
#include <rainy/core/platform.hpp>
#include <rainy/core/text/string_view.hpp>

namespace rainy::foundation::text::pattern::implements {
    /**
     * \lang english
     * @brief The result of reading one pattern element.
     *
     * \lang simp-chinese
     * @brief 读取一个模式元素的结果。
     */
    struct element {
        enum class kind {
            literal,
            any_char,
            star,
            globstar,
            char_class,
            invalid
        };

        kind type = kind::invalid;
        char32_t literal = 0;
        std::size_t consumed = 0;
        bool negated = false;
        const char *class_begin = nullptr;
        std::size_t class_size = 0;
    };

    inline bool is_separator(char32_t unit) {
        return unit == '/' || unit == '\\';
    }

    inline char32_t decode_unit(const char *data, std::size_t size, std::size_t &consumed) {
        const unsigned char lead = static_cast<unsigned char>(data[0]);
        if (lead < 0x80) {
            consumed = 1;
            return lead;
        }
        std::size_t needed = 0;
        char32_t value = 0;
        if (lead < 0xE0) {
            value = lead & 0x1F;
            needed = 1;
        } else if (lead < 0xF0) {
            value = lead & 0x0F;
            needed = 2;
        } else if (lead < 0xF8) {
            value = lead & 0x07;
            needed = 3;
        } else {
            consumed = 1;
            return lead;
        }
        if (size < needed + 1) {
            consumed = 1;
            return lead;
        }
        for (std::size_t index = 1; index <= needed; ++index) {
            const unsigned char unit = static_cast<unsigned char>(data[index]);
            if ((unit & 0xC0) != 0x80) {
                consumed = 1;
                return lead;
            }
            value = (value << 6) | (unit & 0x3F);
        }
        consumed = needed + 1;
        return value;
    }

    inline bool class_matches(const element &entry, char32_t value) {
        const char *data = entry.class_begin;
        std::size_t offset = 0;
        bool matched = false;
        while (offset < entry.class_size) {
            std::size_t consumed = 0;
            const char32_t low = decode_unit(data + offset, entry.class_size - offset, consumed);
            offset += consumed;
            if (offset + 1 < entry.class_size && data[offset] == '-' && data[offset + 1] != ']') {
                std::size_t range_consumed = 0;
                const char32_t high = decode_unit(data + offset + 1, entry.class_size - offset - 1, range_consumed);
                offset += 1 + range_consumed;
                if (value >= low && value <= high) {
                    matched = true;
                }
            } else if (value == low) {
                matched = true;
            }
        }
        return entry.negated ? !matched : matched;
    }

    inline element read_element(const char *data, std::size_t size, bool path_mode) {
        element entry;
        if (size == 0) {
            return entry;
        }
        const char unit = data[0];
        if (unit == '*') {
            if (path_mode && size >= 2 && data[1] == '*') {
                entry.type = element::kind::globstar;
                if (size >= 3 && is_separator(static_cast<unsigned char>(data[2]))) {
                    entry.consumed = 3;
                } else {
                    entry.consumed = 2;
                }
            } else {
                entry.type = element::kind::star;
                entry.consumed = 1;
            }
            return entry;
        }
        if (unit == '?') {
            entry.type = element::kind::any_char;
            entry.consumed = 1;
            return entry;
        }
        if (unit == '[') {
            std::size_t cursor = 1;
            bool negated = false;
            if (cursor < size && (data[cursor] == '!' || data[cursor] == '^')) {
                negated = true;
                ++cursor;
            }
            const std::size_t content_begin = cursor;
            bool closed = false;
            while (cursor < size) {
                if (data[cursor] == ']' && cursor > content_begin) {
                    closed = true;
                    break;
                }
                ++cursor;
            }
            if (!closed) {
                entry.type = element::kind::literal;
                entry.literal = '[';
                entry.consumed = 1;
                return entry;
            }
            if (cursor == content_begin) {
                entry.type = element::kind::literal;
                entry.literal = '[';
                entry.consumed = 1;
                return entry;
            }
            entry.type = element::kind::char_class;
            entry.negated = negated;
            entry.class_begin = data + content_begin;
            entry.class_size = cursor - content_begin;
            entry.consumed = cursor + 1;
            return entry;
        }
        std::size_t consumed = 0;
        entry.type = element::kind::literal;
        entry.literal = decode_unit(data, size, consumed);
        entry.consumed = consumed;
        return entry;
    }

    /**
     * \lang english
     * @brief A pending wildcard that can be extended during backtracking.
     *
     * \lang simp-chinese
     * @brief 回溯期间可被扩展的待定通配符。
     */
    struct backtrack_point {
        std::size_t pattern_pos = 0;
        std::size_t text_pos = 0;
        bool crosses = false;
    };

    inline bool match_impl(core::text::string_view pattern, core::text::string_view text, bool path_mode) {
        const char *p = pattern.data();
        const std::size_t p_size = pattern.size();
        const char *t = text.data();
        const std::size_t t_size = text.size();
        std::size_t p_pos = 0;
        std::size_t t_pos = 0;

        core::collections::vector<backtrack_point> stack;

        while (true) {
            if (t_pos >= t_size) {
                std::size_t probe = p_pos;
                bool tail_ok = true;
                while (probe < p_size) {
                    const element entry = read_element(p + probe, p_size - probe, path_mode);
                    if (entry.type != element::kind::star && entry.type != element::kind::globstar) {
                        tail_ok = false;
                        break;
                    }
                    probe += entry.consumed;
                }
                if (tail_ok) {
                    return true;
                }
            } else if (p_pos < p_size) {
                const element entry = read_element(p + p_pos, p_size - p_pos, path_mode);
                if (entry.type == element::kind::star || entry.type == element::kind::globstar) {
                    backtrack_point point;
                    point.pattern_pos = p_pos + entry.consumed;
                    point.text_pos = t_pos;
                    point.crosses = entry.type == element::kind::globstar;
                    stack.push_back(point);
                    p_pos += entry.consumed;
                    continue;
                }
                std::size_t consumed = 0;
                const char32_t value = decode_unit(t + t_pos, t_size - t_pos, consumed);
                bool matched = false;
                if (entry.type == element::kind::any_char) {
                    matched = !(path_mode && is_separator(value));
                } else if (entry.type == element::kind::char_class) {
                    matched = !(path_mode && is_separator(value)) && class_matches(entry, value);
                } else if (entry.type == element::kind::literal) {
                    matched = value == entry.literal;
                }
                if (matched) {
                    p_pos += entry.consumed;
                    t_pos += consumed;
                    continue;
                }
            }

            bool resumed = false;
            while (!stack.empty()) {
                backtrack_point &point = stack.back();
                if (point.text_pos >= t_size) {
                    stack.pop_back();
                    continue;
                }
                std::size_t consumed = 0;
                const char32_t value = decode_unit(t + point.text_pos, t_size - point.text_pos, consumed);
                if (path_mode && !point.crosses && is_separator(value)) {
                    stack.pop_back();
                    continue;
                }
                point.text_pos += consumed;
                p_pos = point.pattern_pos;
                t_pos = point.text_pos;
                resumed = true;
                break;
            }
            if (!resumed) {
                return false;
            }
        }
    }
}

namespace rainy::foundation::text::pattern {

    /**
     * \lang english
     * @brief Matches a sequence against a glob pattern.
     *
     * The pattern language is * for any run of characters including none, ? for
     * a single character, and [..] for a character class with an optional
     * leading ! or ^ for negation and a-b ranges. A class that is never closed
     * is treated as a literal opening bracket. Matching is greedy with
     * backtracking.
     *
     * @param pattern The glob pattern
     * @param text The sequence to test
     * @return true when the whole sequence matches the whole pattern
     *
     * \lang simp-chinese
     * @brief 用 glob 模式匹配序列。
     *
     * 模式语言为：* 匹配任意长度（可为空）的字符序列，? 匹配单个字符，[..] 为字符类，
     * 可带前缀 ! 或 ^ 表示否定，可用 a-b 表示区间。未闭合的字符类按字面量左括号处理。
     * 匹配为贪心加回溯。
     *
     * @param pattern glob 模式
     * @param text 待测试的序列
     * @return 整个序列匹配整个模式时返回 true
     */
    inline bool glob_match(core::text::string_view pattern, core::text::string_view text) {
        return implements::match_impl(pattern, text, false);
    }

    /**
     * \lang english
     * @brief Matches a sequence against a path aware glob pattern.
     *
     * On top of the general pattern language, * and ? never match a separator,
     * while ** matches across separators. Both / and \ are treated as
     * separators. This follows the gitignore convention.
     *
     * @param pattern The path glob pattern
     * @param text The path to test
     * @return true when the whole path matches the whole pattern
     *
     * \lang simp-chinese
     * @brief 用路径感知的 glob 模式匹配序列。
     *
     * 在通用模式语言之上，* 与 ? 不匹配分隔符，而 ** 可跨分隔符。分隔符同时接受 /
     * 与 \。此语义遵循 gitignore 约定。
     *
     * @param pattern 路径 glob 模式
     * @param text 待测试的路径
     * @return 整个路径匹配整个模式时返回 true
     */
    inline bool glob_match_path(core::text::string_view pattern, core::text::string_view text) {
        return implements::match_impl(pattern, text, true);
    }
}

namespace rainy::text {
    using foundation::text::pattern::glob_match;
    using foundation::text::pattern::glob_match_path;
}

#endif
