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
#ifndef RAINY_FOUNDATION_WILLOW_INI_SERIALIZER_HPP
#define RAINY_FOUNDATION_WILLOW_INI_SERIALIZER_HPP
#include <rainy/core/collections/vector.hpp>
#include <rainy/core/platform.hpp>
#include <rainy/foundation/willow/implements/common/value.hpp>
#include <rainy/foundation/willow/implements/ini/config.hpp>
#include <rainy/foundation/willow/implements/ini/exceptions.hpp>

namespace rainy::foundation::willow::ini::implements {
    using namespace rainy::foundation::exceptions::willow;

    template <typename BasicDocument>
    struct ini_serializer {
        using string_type = typename BasicDocument::string_type;
        using char_type = typename BasicDocument::char_type;
        using char_traits = core::text::char_traits<char_type>;
        using char_int_type = typename char_traits::int_type;
        using args = serializer_args<BasicDocument>;

        static constexpr std::size_t flush_threshold = 4096;

        ini_serializer(output_adapter<char_type> adapter, const args &args) :
            out_(utility::move(adapter)), arg_(args) {
            out_buffer_.reserve(flush_threshold);
        }

        void dump(const BasicDocument &ini) {
            if (!ini.is_object()) {
                exceptions::willow::ini::throw_ini_serialize_error("ini document must be an object");
            }
            const auto &object = ini.as_object();
            for (const auto &entry: object) {
                if (entry.second.is_string()) {
                    check_key(entry.first);
                    check_value(entry.second.as_string());
                } else if (entry.second.is_object()) {
                    check_section(entry.first);
                    for (const auto &item: entry.second.as_object()) {
                        if (!item.second.is_string()) {
                            exceptions::willow::ini::throw_ini_serialize_error("ini section values must be strings");
                        }
                        check_key(item.first);
                        check_value(item.second.as_string());
                    }
                } else {
                    exceptions::willow::ini::throw_ini_serialize_error("ini values must be strings or sections");
                }
            }
            for (const auto &entry: object) {
                if (!entry.second.is_string()) {
                    continue;
                }
                put(entry.first.data(), entry.first.size());
                put(separator());
                put(entry.second.as_string().data(), entry.second.as_string().size());
                put(newline());
            }
            for (const auto &entry: object) {
                if (!entry.second.is_object()) {
                    continue;
                }
                put(open_section());
                put(entry.first.data(), entry.first.size());
                put(close_section());
                put(newline());
                for (const auto &item: entry.second.as_object()) {
                    put(item.first.data(), item.first.size());
                    put(separator());
                    put(item.second.as_string().data(), item.second.as_string().size());
                    put(newline());
                }
            }
            flush_buffer();
        }

    private:
        void put(const char_type ch) {
            out_buffer_.push_back(ch);
            if (out_buffer_.size() >= flush_threshold) {
                flush_buffer();
            }
        }

        void put(const char_type *str, const std::size_t size) {
            if (size == 0) {
                return;
            }
            if (size >= flush_threshold) {
                flush_buffer();
                out_->write(str, size);
                return;
            }
            if (out_buffer_.size() + size > flush_threshold) {
                flush_buffer();
            }
            out_buffer_.insert(out_buffer_.end(), str, str + size);
        }

        void flush_buffer() {
            if (!out_buffer_.empty()) {
                out_->write(out_buffer_.data(), out_buffer_.size());
                out_buffer_.clear();
            }
        }

        void check_key(const string_type &key) const {
            if (key.empty()) {
                exceptions::willow::ini::throw_ini_serialize_error("ini key is empty");
            }
            for (const auto ch: key) {
                if (ch == char_type('\n') || ch == char_type('\r') || ch == char_type('=') || ch == char_type('[') ||
                    ch == char_type(']')) {
                    exceptions::willow::ini::throw_ini_serialize_error("ini key holds a reserved character");
                }
            }
        }

        void check_value(const string_type &value) const {
            for (const auto ch: value) {
                if (ch == char_type('\n') || ch == char_type('\r')) {
                    exceptions::willow::ini::throw_ini_serialize_error("ini value holds a line break");
                }
            }
        }

        void check_section(const string_type &name) const {
            if (name.empty()) {
                exceptions::willow::ini::throw_ini_serialize_error("ini section name is empty");
            }
            for (const auto ch: name) {
                if (ch == char_type('\n') || ch == char_type('\r') || ch == char_type('[') || ch == char_type(']')) {
                    exceptions::willow::ini::throw_ini_serialize_error("ini section name holds a reserved character");
                }
            }
        }

        const char_type *separator() const {
            if constexpr (type_traits::type_relations::is_same_v<char_type, char>) {
                return " = ";
            } else if constexpr (type_traits::type_relations::is_same_v<char_type, wchar_t>) {
                return L" = ";
            } else if constexpr (type_traits::type_relations::is_same_v<char_type, char16_t>) {
                return u" = ";
            } else if constexpr (type_traits::type_relations::is_same_v<char_type, char32_t>) {
                return U" = ";
            } else {
#if RAINY_HAS_CXX20
                if constexpr (type_traits::type_relations::is_same_v<char_type, char8_t>) {
                    return u8" = ";
                }
#endif
                return nullptr;
            }
        }

        char_type newline() const {
            return char_type('\n');
        }

        char_type open_section() const {
            return char_type('[');
        }

        char_type close_section() const {
            return char_type(']');
        }

    private:
        output_adapter<char_type> out_;
        const args &arg_;
        core::collections::vector<char_type> out_buffer_;
    };
}

#endif
