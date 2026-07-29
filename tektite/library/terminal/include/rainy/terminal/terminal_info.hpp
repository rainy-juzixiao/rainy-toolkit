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
#ifndef RAINY_TERMINAL_TERMINFO_HPP
#define RAINY_TERMINAL_TERMINFO_HPP
#include <rainy/core/collections/array.hpp>
#include <rainy/core/memory/nebula_ptr.hpp>
#include <rainy/core/text/string.hpp>

namespace rainy::terminal {
    class terminal_info {
    public:
        enum class color_mode : std::uint8_t {
            unset,
            no_color,
            bit3,
            bit4,
            bit8,
            bit24
        };

        enum class boolean : std::uint8_t {
            utf8 = 0,
            legacy,
            control_sequences
        };

        enum class string : std::uint8_t {
            term_env,
            term_name,
            term_version
        };

        enum class integer : std::uint8_t {
        };

        static bool from(const terminal_info::boolean &key);
        static std::uint32_t from(const terminal_info::integer &key);
        static core::text::string from(const terminal_info::string &key);

        using booleans = collections::array<boolean, 3>;
        using strings = collections::array<std::string, 3>;
        using integers = collections::array<std::uint32_t, 0>;

        terminal_info();

        static color_mode the_color_mode();

    private:
        struct impl;

        core::memory::nebula_ptr<impl> impl_;
    };
}

#endif
