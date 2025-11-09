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
#ifndef RAINY_COMPONENT_JSON_UTILITY_HPP
#define RAINY_COMPONENT_JSON_UTILITY_HPP
#include <cmath>
#include <rainy/collections/dense_map.hpp>
#include <rainy/component/willow/implements/config.hpp>
#include <rainy/component/willow/implements/exceptions.hpp>
#include <rainy/core/core.hpp>
#include <iostream>
namespace rainy::component::willow::adapters {
	template <typename Ty>
    struct input_adapter_abstract {
        using char_type = Ty;
        using char_traits = std::char_traits<char_type>;
        using char_int_type = typename char_traits::int_type;

        template <typename Impl>
        using impl = type_traits::other_trans::value_list<&Impl::get_char>;

        template <typename Base>
        struct type : Base {
            char_int_type get_char() {
                return this->template invoke<0>(*this);
            }
        };
    };

	template <typename Ty>
	using input_adapter = core::basic_poly<input_adapter_abstract<Ty>>;

	template <typename Ty>
    struct file_input_adapter final {
        using char_type = typename input_adapter_abstract<Ty>::char_type;
        using char_traits = typename input_adapter_abstract<Ty>::char_traits;
        using char_int_type = typename char_traits::int_type;

        explicit file_input_adapter(std::FILE *file) : file(file) {
        }

        char_int_type get_char() {
            return static_cast<char_int_type>(std::fgetc(file));
        }

    private:
        std::FILE *file;
    };

	template <typename Ty>
    struct stream_input_adapter final {
        using char_type = typename input_adapter_abstract<Ty>::char_type;
        using char_traits = typename input_adapter_abstract<Ty>::char_traits;
        using char_int_type = typename char_traits::int_type;

        explicit stream_input_adapter(std::basic_istream<char_type> &stream) : stream(stream), streambuf(*stream.rdbuf()) {
        }

        char_int_type get_char() {
            auto ch = streambuf.sbumpc();
            if (ch == EOF) {
                stream.clear(stream.rdstate() | std::ios::eofbit);
            }
            return ch;
        }

        ~stream_input_adapter() {
            stream.clear(stream.rdstate() & std::ios::eofbit);
        }

    private:
        std::basic_istream<char_type> &stream;
        std::basic_streambuf<char_type> &streambuf;
    };

	template <typename Ty>
    struct string_input_adapter final {
        using char_type = typename input_adapter_abstract<typename Ty::value_type>::char_type;
        using char_traits = typename input_adapter_abstract<typename Ty::value_type>::char_traits;
        using char_int_type = typename char_traits::int_type;

        explicit string_input_adapter(const Ty &str) : str(str), index(0) {
        }

        char_int_type get_char() {
            if (index == str.size()) {
                return char_traits::eof();
            }
            return str[index++];
        }

    private:
        const Ty &str;
        typename Ty::size_type index;
    };

	template <typename Ty>
    struct string_view_input_adapter final {
        using char_type = typename input_adapter_abstract<typename Ty::value_type>::char_type;
        using char_traits = typename input_adapter_abstract<typename Ty::value_type>::char_traits;
        using char_int_type = typename char_traits::int_type;

        explicit string_view_input_adapter(const Ty &str) : str(str), index(0) {
        }

        char_int_type get_char() {
            if (index < str.size()) {
                return str[index++];
            }
            return char_traits::eof();
        }

    private:
        Ty str;
        typename Ty::size_type index;
    };

	template <typename Ty>
    struct buffer_input_adapter final {
        using char_type = typename input_adapter_abstract<Ty>::char_type;
        using char_traits = typename input_adapter_abstract<Ty>::char_traits;
        using char_int_type = typename char_traits::int_type;

        explicit buffer_input_adapter(const Ty *str) : str(str) {
        }

        char_int_type get_char() {
            if (str[index] == '\0') {
                return char_traits::eof();
            }
            return str[index++];
        }

    private:
        const char_type *str;
        std::size_t index{0};
    };

    template <typename Ty>
    struct output_adapter_abstract {
        using char_type = Ty;
        using char_traits = std::char_traits<char_type>;
        using char_int_type = typename char_traits::int_type;

        template <typename Impl>
        using impl = type_traits::other_trans::value_list<utility::get_overloaded_func<Impl, void(Ty)>(&Impl::write),
                                                          utility::get_overloaded_func<Impl, void(const Ty *str, std::size_t size)>(
                                                              &Impl::write)>;

        template <typename Base>
        struct type : Base {
            void write(Ty ch) {
                (void) this->template invoke<0>(*this, ch);
            }

            void write(const char_type *str, std::size_t size) {
                (void) this->template invoke<1>(*this, str, size);
            }
        };
    };

    template <typename Ty>
	using output_adapter = core::basic_poly<output_adapter_abstract<Ty>>;

	template <typename Ty>
	struct string_output_adapter final {
		using char_type = typename Ty::value_type;
		using size_type = typename Ty::size_type;
		using char_traits = std::char_traits<char_type>;

        explicit string_output_adapter(Ty &str) : string(str) {
		}

		void write(const char_type ch) {
			string.push_back(ch);
		}

		void write(const char_type *str, std::size_t size) {
			this->string.append(str, static_cast<size_type>(size));
		}

	private:
		Ty &string;
	};

	template <typename Ty>
	struct stream_output_adapter final {
		using char_type = Ty;
		using size_type = std::streamsize;
		using char_traits = std::char_traits<char_type>;

        explicit stream_output_adapter(std::basic_ostream<char_type> &stream) : stream(stream) {
		}

		void write(const char_type ch) {
			stream.put(ch);
		}

		void write(const char_type *str, const std::size_t size) {
			stream.write(str, static_cast<size_type>(size));
		}

	private:
		std::basic_ostream<char_type> &stream;
	};
}

namespace rainy::component::willow::utils {
	template <typename Ty>
	bool nearly_equal(Ty a, Ty b) noexcept {
		return std::fabs(a - b) < utility::numeric_limits<Ty>::epsilon();
	}

	template <typename Type>
	struct unicode_reader;

	template <typename CharType, typename Traits, typename Alloc, template <typename, typename...> typename StringTemplate>
    struct unicode_reader_impl;

	template <typename Traits, typename Alloc, template <typename CharType, typename...> typename StringTemplate>
    struct unicode_reader_impl<char, Traits, Alloc, StringTemplate> {
        using string_type = StringTemplate<char, Traits, Alloc>;

        unicode_reader_impl(const string_type &val, const bool escape_unicode) : val(val), escape_unicode(escape_unicode) {
        }

        RAINY_NODISCARD uint8_t get_byte(const std::size_t i) const {
            return static_cast<uint8_t>(val.at(i));
        }

        bool get_code(std::size_t &i, std::uint32_t &code_output) const {
            using namespace rainy::foundation::exceptions::willow;
            static constexpr collections::array<std::uint8_t, 256> utf8_extra_bytes = {
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5,
            };
            static constexpr collections::array<std::uint32_t, 6> utf8_offsets = {
                0x00000000, 0x00003080, 0x000E2080, 0x03C82080, 0xFA082080, 0x82082080,
            };
            if (i >= val.size()) {
                return false;
            }
            if (!escape_unicode) {
                // code point will not be escaped
                const auto byte = get_byte(i);
                code_output = static_cast<std::uint32_t>(byte);
                i++;
                return true;
            }
            const auto first_byte = get_byte(i);
            const auto extra_bytes_to_read = utf8_extra_bytes[first_byte];
            if (i + static_cast<std::size_t>(extra_bytes_to_read) >= val.size()) {
                throw_exception(json_serialize_error("string was incomplete"));
            }
            std::uint32_t code = 0;
            switch (extra_bytes_to_read) {
                case 5:
                    code += static_cast<std::uint32_t>(get_byte(i));
                    i++;
                    code <<= 6;
                    RAINY_FALLTHROUGH;
                case 4:
                    code += static_cast<std::uint32_t>(get_byte(i));
                    i++;
                    code <<= 6;
                    RAINY_FALLTHROUGH;
                case 3:
                    code += static_cast<std::uint32_t>(get_byte(i));
                    i++;
                    code <<= 6;
                    RAINY_FALLTHROUGH;
                case 2:
                    code += static_cast<std::uint32_t>(get_byte(i));
                    i++;
                    code <<= 6;
                    RAINY_FALLTHROUGH;
                case 1:
                    code += static_cast<std::uint32_t>(get_byte(i));
                    i++;
                    code <<= 6;
                    RAINY_FALLTHROUGH;
                case 0:
                    code += static_cast<std::uint32_t>(get_byte(i));
                    i++;
                    RAINY_FALLTHROUGH;
                default:
                    break;
            }
            code -= utf8_offsets[extra_bytes_to_read]; // NOLINT
            code_output = code;
            return true;
        }

        const string_type &val;
        const bool escape_unicode;
        std::uint8_t state = 0;
    };

	template <typename Traits, typename Alloc>
    struct unicode_reader<std::basic_string<char, Traits, Alloc>> : unicode_reader_impl<char, Traits, Alloc, std::basic_string> {
        using base = unicode_reader_impl<char, Traits, Alloc, std::basic_string>;
        using base::base;
    };

	template <typename Traits, typename Alloc>
    struct unicode_reader<text::basic_string<char, Traits, Alloc>> : unicode_reader_impl<char, Traits, Alloc, text::basic_string> {
        using base = unicode_reader_impl<char, Traits, Alloc, text::basic_string>;
        using base::base;
    };

	template <>
	struct unicode_reader<std::wstring> {
		using string_type = std::wstring;
		using char_type = string_type::value_type;

		const string_type &val;
		const bool escape_unicode;

		unicode_reader(const string_type &val, const bool escape_unicode) : val(val), escape_unicode(escape_unicode) {
		}

		bool get_code(std::size_t &i, std::uint32_t &code) const {
			return get_code(i, code, std::integral_constant<bool, sizeof(char_type) == 4>());
		}

		bool get_code(std::size_t &i, std::uint32_t &code, std::true_type /* sizeof(wchar_t) == 4 */) const {
			if (i >= val.size()) {
				return false;
			}
			code = static_cast<std::uint32_t>(val.at(i));
			i++;
			return true;
		}

		bool get_code(std::size_t &i, std::uint32_t &code, std::false_type /* sizeof(wchar_t) == 2 */) const {
			using namespace foundation::exceptions::willow;
			if (i >= val.size()) {
				return false;
			}
			code = static_cast<std::uint32_t>(static_cast<uint16_t>(val.at(i)));
			i++;
			if (!escape_unicode) {
				return true;
			}
            if (implements::unicode_surrogate_lead_begin <= code && code <= implements::unicode_surrogate_lead_end) {
				if (i >= val.size()) {
					throw_exception(json_serialize_error("string was incomplete"));
				}
				const std::uint32_t lead_surrogate = code;
				const std::uint32_t trail_surrogate = static_cast<uint16_t>(val.at(i));
				code = implements::merge_surrogates(lead_surrogate, trail_surrogate);
				i++;
			}
			return true;
		}
	};

	template <>
	struct unicode_reader<std::u32string> {
		using string_type = std::u32string;
		using char_type = string_type::value_type;
		const string_type &val;
		const bool escape_unicode;

		unicode_reader(const string_type &val, const bool escape_unicode) : val(val), escape_unicode(escape_unicode) {
		}

		bool get_code(std::size_t &i, std::uint32_t &code) const {
			if (i >= val.size()) {
				return false;
			}
			code = static_cast<std::uint32_t>(val.at(i));
			i++;
			return true;
		}
	};

	template <>
	struct unicode_reader<std::u16string> {
		using string_type = std::u16string;
		using char_type = string_type::value_type;

		const string_type &val;
		const bool escape_unicode;

		unicode_reader(const string_type &val, const bool escape_unicode) : val(val), escape_unicode(escape_unicode) {
		}

		bool get_code(std::size_t &i, std::uint32_t &code) const {
			using namespace foundation::exceptions::willow;
			if (i >= val.size()) {
				return false;
			}
			code = static_cast<std::uint32_t>(static_cast<uint16_t>(val.at(i)));
			i++;
			if (!escape_unicode) {
				return true;
			}
            if (implements::unicode_surrogate_lead_begin <= code && code <= implements::unicode_surrogate_lead_end) {
				if (i >= val.size()) {
					throw_exception(json_serialize_error("string was incomplete"));
				}
				const std::uint32_t lead_surrogate = code;
				const std::uint32_t trail_surrogate = static_cast<uint16_t>(val.at(i));
                code = implements::merge_surrogates(lead_surrogate, trail_surrogate);
				i++;
			}
			return true;
		}
	};

	template <typename CharType, typename Traits, typename Alloc, template <typename, typename...> typename StringTemplate>
    struct unicode_writer_impl;

    template <typename Traits, typename Alloc, template <typename CharType, typename...> typename StringTemplate>
    struct unicode_writer_impl<char, Traits, Alloc, StringTemplate> {
        using string_type = StringTemplate<char, Traits, Alloc>;
        using char_type = typename string_type::value_type;
        using char_traits = std::char_traits<char_type>;
        using char_int_type = typename char_traits::int_type;

        explicit unicode_writer_impl(string_type &buffer) : buffer(buffer) {};

        void add_char(const char_int_type ch) const {
            buffer.push_back(char_traits::to_char_type(ch));
        }

        void add_code(const std::uint32_t code) const {
            if (code < 0x80) {
                // 0xxxxxxx
                add_char(static_cast<char_int_type>(code));
            } else if (code <= 0x7FF) {
                // 110xxxxx 10xxxxxx
                add_char(static_cast<char_int_type>(0xC0 | (code >> 6)));
                add_char(static_cast<char_int_type>(0x80 | (code & 0x3F)));
            } else if (code <= 0xFFFF) {
                // 1110xxxx 10xxxxxx 10xxxxxx
                add_char(static_cast<char_int_type>(0xE0 | (code >> 12)));
                add_char(static_cast<char_int_type>(0x80 | ((code >> 6) & 0x3F)));
                add_char(static_cast<char_int_type>(0x80 | (code & 0x3F)));
            } else {
                // 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
                add_char(static_cast<char_int_type>(0xF0 | (code >> 18)));
                add_char(static_cast<char_int_type>(0x80 | ((code >> 12) & 0x3F)));
                add_char(static_cast<char_int_type>(0x80 | ((code >> 6) & 0x3F)));
                add_char(static_cast<char_int_type>(0x80 | (code & 0x3F)));
            }
        }

        void add_surrogates(const std::uint32_t lead_surrogate, const std::uint32_t trail_surrogate) const {
            add_code(implements::merge_surrogates(lead_surrogate, trail_surrogate));
        }

        string_type &buffer;
	};

    template <typename Traits, typename Alloc, template <typename CharType, typename...> typename StringTemplate>
    struct unicode_writer_impl<wchar_t, Traits, Alloc, StringTemplate> {
        using string_type = StringTemplate<wchar_t, Traits, Alloc>;
        using char_type = typename string_type::value_type;
        using char_traits = std::char_traits<char_type>;
        using char_int_type = typename char_traits::int_type;

        explicit unicode_writer_impl(string_type &buffer) : buffer(buffer) {};

        void add_char(const char_int_type ch) const {
            buffer.push_back(char_traits::to_char_type(ch));
        }

        void add_code(const std::uint32_t code) const {
            if constexpr (sizeof(char_type) == 4) {
                add_char(static_cast<char_int_type>(code));
            } else {
                using namespace rainy::foundation::exceptions::willow;
                if (code < 0xFFFF) {
                    add_char(static_cast<char_int_type>(code));
                } else {
                    throw_json_parse_error("invalid 16-bits unicode character");
                }
            }
        }

        void add_surrogates(const std::uint32_t lead_surrogate, const std::uint32_t trail_surrogate) const {
            if constexpr (sizeof(char_type) == 4) {
                add_code(implements::merge_surrogates(lead_surrogate, trail_surrogate));
            } else {
                add_code(lead_surrogate);
                add_code(trail_surrogate);
            }
        }

        string_type &buffer;
    };

    template <typename Traits, typename Alloc, template <typename CharType, typename...> typename StringTemplate>
    struct unicode_writer_impl<char16_t, Traits, Alloc, StringTemplate> {
        using string_type = StringTemplate<char16_t, Traits, Alloc>;
        using char_type = typename string_type::value_type;
        using char_traits = std::char_traits<char_type>;
        using char_int_type = typename char_traits::int_type;

        explicit unicode_writer_impl(string_type &buffer) : buffer(buffer) {};

        void add_char(const char_int_type ch) const {
            buffer.push_back(char_traits::to_char_type(ch));
        }

        void add_code(const std::uint32_t code) const {
            using namespace foundation::exceptions::willow;
            if (code < 0xFFFF) {
                add_char(static_cast<char_int_type>(code));
            } else {
                throw_exception(json_parse_error("Invalid 16-bits unicode character"));
            }
        }

        void add_surrogates(const std::uint32_t lead_surrogate, const std::uint32_t trail_surrogate) const {
            add_code(lead_surrogate);
            add_code(trail_surrogate);
        }

        string_type &buffer;
    };

	template <typename Type>
	struct unicode_writer;

	template <typename Traits, typename Alloc>
    struct unicode_writer<std::basic_string<char, Traits, Alloc>> : unicode_writer_impl<char, Traits, Alloc, std::basic_string> {
        using base = unicode_writer_impl<char,Traits, Alloc, std::basic_string>;
        using base::base;
    };

	template <typename Traits, typename Alloc>
    struct unicode_writer<text::basic_string<char, Traits, Alloc>> : unicode_writer_impl<char, Traits, Alloc, text::basic_string> {
        using base = unicode_writer_impl<char,Traits, Alloc, text::basic_string>;
        using base::base;
    };

	template <typename Traits, typename Alloc>
    struct unicode_writer<std::basic_string<wchar_t, Traits, Alloc>> : unicode_writer_impl<wchar_t, Traits, Alloc, std::basic_string> {
        using base = unicode_writer_impl<wchar_t, Traits, Alloc, std::basic_string>;
        using base::base;
    };

    template <typename Traits, typename Alloc>
    struct unicode_writer<text::basic_string<wchar_t, Traits, Alloc>>
        : unicode_writer_impl<wchar_t, Traits, Alloc, text::basic_string> {
        using base = unicode_writer_impl<wchar_t, Traits, Alloc, text::basic_string>;
        using base::base;
    };

	template <typename Traits, typename Alloc>
    struct unicode_writer<std::basic_string<char16_t, Traits, Alloc>>
        : unicode_writer_impl<char16_t, Traits, Alloc, std::basic_string> {
        using base = unicode_writer_impl<char16_t, Traits, Alloc, std::basic_string>;
        using base::base;
    };

    template <typename Traits, typename Alloc>
    struct unicode_writer<text::basic_string<char16_t, Traits, Alloc>>
        : unicode_writer_impl<char16_t, Traits, Alloc, text::basic_string> {
        using base = unicode_writer_impl<char16_t, Traits, Alloc, text::basic_string>;
        using base::base;
    };

	template <>
	struct unicode_writer<std::u32string> {
		using string_type = std::u32string;
		using char_type = string_type::value_type;
		using char_traits = std::char_traits<char_type>;
		using char_int_type = char_traits::int_type;

		explicit unicode_writer(string_type &buffer) : buffer(buffer){};

		void add_char(const char_int_type ch) const {
			buffer.push_back(char_traits::to_char_type(ch));
		}

		void add_code(const std::uint32_t code) const {
			add_char(code);
		}

		void add_surrogates(const std::uint32_t lead_surrogate, const std::uint32_t trail_surrogate) const {
            add_code(implements::merge_surrogates(lead_surrogate, trail_surrogate));
		}

		string_type &buffer;
	};

	template <typename Ty>
	class output_streambuf final : public std::basic_streambuf<Ty> {
	public:
		using char_type = typename std::basic_streambuf<Ty>::char_type;
		using int_type = typename std::basic_streambuf<Ty>::int_type;
		using char_traits = std::char_traits<char_type>;

		explicit output_streambuf(adapters::output_adapter<char_type> adapter) : adapter(adapter) {
		}

	protected:
		int_type overflow(int_type c) override {
			if (c != EOF) {
				adapter->write(char_traits::to_char_type(c));
            }
			return c;
		}

		std::streamsize xsputn(const char_type *s, std::streamsize num) override {
			adapter->write(s, static_cast<std::size_t>(num));
			return num;
		}

	private:
		adapters::output_adapter<char_type> adapter;
	};
}

#endif