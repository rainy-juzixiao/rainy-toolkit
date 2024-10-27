#pragma once
#include <cmath>
#include <rainy/base.hpp>
#include <rainy/system/exceptions.hpp>
#include <string>
#include "predef.hpp"

namespace rainy::foundation::system::exceptions::json {
	class json_exception : public runtime::runtime_error {
	public:
		using base = runtime_error;

		json_exception(const char *message, const source &loc = source::current()) : base(message, loc) {
		}
	};

	class json_type_error final : json_exception {
	public:
		using base = json_exception;

		json_type_error(const char *message, const source &loc = source::current()) : base(message, loc) {
		}
	};

	class json_invalid_key final : json_exception {
	public:
		json_invalid_key(const char *message, const source &loc = source::current()) : json_exception(message, loc) {
		}
	};

	class json_invalid_iterator final : json_exception {
	public:
		using base = json_exception;

		json_invalid_iterator(const char *message, const source &loc = source::current()) : base(message, loc) {
		}
	};

	class json_parse_error final : json_exception {
	public:
		using base = json_exception;

		json_parse_error(const char *message, const source &loc = source::current()) : base(message, loc) {
		}
	};

	class json_serialize_error final : json_exception {
	public:
		using base = json_exception;

		json_serialize_error(const char *message, const source &loc = source::current()) : base(message, loc) {
		}
	};
}

namespace rainy::component::json::adapters {
	template <typename Ty>
	struct input_adapter {
		using char_type = Ty;
		using char_traits = std::char_traits<char_type>;
		using char_int_type = typename char_traits::int_type;

		virtual char_int_type get_char() = 0;
		virtual ~input_adapter() = default;
	};

	template <typename Ty>
	struct file_input_adapter final : input_adapter<Ty> {
		using char_type = typename input_adapter<Ty>::char_type;
		using char_traits = typename input_adapter<Ty>::char_traits;
		using char_int_type = typename char_traits::int_type;

		file_input_adapter(std::FILE *file) : file(file) {
		}

		char_int_type get_char() override {
			return std::fgetc(file);
		}

	private:
		std::FILE *file;
	};

	template <typename Ty>
	struct stream_input_adapter final : input_adapter<Ty> {
		using char_type = typename input_adapter<Ty>::char_type;
		using char_traits = typename input_adapter<Ty>::char_traits;
		using char_int_type = typename char_traits::int_type;

		stream_input_adapter(std::basic_istream<char_type> &stream) : stream(stream), streambuf(*stream.rdbuf()) {
		}

		char_int_type get_char() override {
			auto ch = streambuf.sbumpc();
			if (ch == EOF) {
				stream.clear(stream.rdstate() | std::ios::eofbit);
			}
			return ch;
		}

		~stream_input_adapter() override {
			stream.clear(stream.rdstate() & std::ios::eofbit);
		}

	private:
		std::basic_istream<char_type> &stream;
		std::basic_streambuf<char_type> &streambuf;
	};

	template <typename Ty>
	struct string_input_adapter final : input_adapter<typename Ty::value_type> {
		using char_type = typename input_adapter<typename Ty::value_type>::char_type;
		using char_traits = typename input_adapter<typename Ty::value_type>::char_traits;
		using char_int_type = typename char_traits::int_type;

		string_input_adapter(const Ty &str) : str(str), index(0) {
		}

		char_int_type get_char() override {
			if (index == str.size())
				return char_traits::eof();
			return str[index++];
		}

	private:
		const Ty &str;
		typename Ty::size_type index;
	};

	template <typename Ty>
	struct buffer_input_adapter : input_adapter<Ty> {
		using char_type = typename input_adapter<Ty>::char_type;
		using char_traits = typename input_adapter<Ty>::char_traits;
		using char_int_type = typename char_traits::int_type;

		buffer_input_adapter(const Ty *str) : str(str), index(0) {
		}

		char_int_type get_char() override {
			if (str[index] == '\0')
				return char_traits::eof();
			return str[index++];
		}

	private:
		const char_type *str;
		std::size_t index;
	};

	template <typename Ty>
	struct output_adapter {
		using char_type = Ty;
		using char_traits = std::char_traits<char_type>;

		virtual ~output_adapter() = default;

		virtual void write(Ty ch) = 0;
		virtual void write(const Ty *str, std::size_t size) = 0;
	};

	template <typename Ty>
	struct string_output_adapter final : output_adapter<typename Ty::value_type> {
		using char_type = typename Ty::value_type;
		using size_type = typename Ty::size_type;
		using char_traits = std::char_traits<char_type>;

		string_output_adapter(Ty &str) : str(str) {
		}

		void write(const char_type ch) override {
			str.push_back(ch);
		}

		void write(const char_type *str, std::size_t size) override {
			str.append(str, static_cast<size_type>(size));
		}

	private:
		Ty &str;
	};

	template <typename Ty>
	struct stream_output_adapter final : output_adapter<Ty> {
		using char_type = Ty;
		using size_type = std::streamsize;
		using char_traits = std::char_traits<char_type>;

		stream_output_adapter(std::basic_ostream<char_type> &stream) : stream(stream) {
		}

		void write(const char_type ch) override {
			stream.put(ch);
		}

		void write(const char_type *str, std::size_t size) override {
			stream.write(str, static_cast<size_type>(size));
		}

	private:
		std::basic_ostream<char_type> &stream;
	};
}

namespace rainy::component::json::utils {
	template <typename Ty>
	bool nearly_equal(Ty a, Ty b) {
		return std::fabs(a - b) < std::numeric_limits<Ty>::epsilon();
	}

	template <typename basicJson>
	struct serializer_args {
		using char_type = typename basicJson::char_type;
		using float_type = typename basicJson::float_type;

		int precision = std::numeric_limits<float_type>::digits10 + 1;
		unsigned int indent = 0;
		char_type indent_char = ' ';
		bool escape_unicode = false;
	};

	uint32_t merge_surrogates(const uint32_t lead_surrogate, const uint32_t trail_surrogate) {
		uint32_t code = ((lead_surrogate - constant::UNICODE_SUR_LEAD_BEGIN) << constant::UNICODE_SUR_BITS);
		code += (trail_surrogate - constant::UNICODE_SUR_TRAIL_BEGIN);
		code += constant::UNICODE_SUR_BASE;
		return code;
	}

	template <typename type>
	struct unicode_reader;

	template <>
	struct unicode_reader<std::string> {
		using string_type = std::string;

		const string_type &val;
		const bool escape_unicode;
		uint8_t state = 0;

		unicode_reader(const string_type &val, const bool escape_unicode) : val(val), escape_unicode(escape_unicode) {
		}

		uint8_t get_byte(const size_t i) const {
			return static_cast<uint8_t>(val.at(i));
		}

		bool get_code(size_t &i, uint32_t &code_output) const {
			using namespace rainy::foundation::system::exceptions::json;
			static constexpr foundation::containers::array<std::uint8_t, 256> utf8_extra_bytes = {
					0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
					0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
					0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
					0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
					0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
					0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
					1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5,
			};
			static constexpr foundation::containers::array<std::uint32_t, 6> utf8_offsets = {
					0x00000000, 0x00003080, 0x000E2080, 0x03C82080, 0xFA082080, 0x82082080,
			};
			if (i >= val.size())
				return false;
			if (!escape_unicode) {
				// code point will not be escaped
				const auto byte = get_byte(i);
				code_output = static_cast<uint32_t>(byte);
				i++;
				return true;
			}
			const auto first_byte = get_byte(i);
			const auto extra_bytes_to_read = utf8_extra_bytes[first_byte];
			if (i + static_cast<size_t>(extra_bytes_to_read) >= val.size())
				throw_exception(json_serialize_error("string was incomplete"));
			uint32_t code = 0;
			switch (extra_bytes_to_read) {
				case 5:
					code += static_cast<uint32_t>(get_byte(i));
					i++;
					code <<= 6;
					RAINY_FALLTHROUGH;
				case 4:
					code += static_cast<uint32_t>(get_byte(i));
					i++;
					code <<= 6;
					RAINY_FALLTHROUGH;
				case 3:
					code += static_cast<uint32_t>(get_byte(i));
					i++;
					code <<= 6;
					RAINY_FALLTHROUGH;
				case 2:
					code += static_cast<uint32_t>(get_byte(i));
					i++;
					code <<= 6;
					RAINY_FALLTHROUGH;
				case 1:
					code += static_cast<uint32_t>(get_byte(i));
					i++;
					code <<= 6;
					RAINY_FALLTHROUGH;
				case 0:
					code += static_cast<uint32_t>(get_byte(i));
					i++;
					RAINY_FALLTHROUGH;
				default:
					// Do nothing
					RAINY_FALLTHROUGH;
			}
			code -= utf8_offsets[extra_bytes_to_read];
			code_output = code;
			return true;
		}
	};

	template <>
	struct unicode_reader<std::wstring> {
		using string_type = std::wstring;
		using char_type = string_type::value_type;

		const string_type &val;
		const bool escape_unicode;

		unicode_reader(const string_type &val, const bool escape_unicode) : val(val), escape_unicode(escape_unicode) {
		}

		bool get_code(size_t &i, uint32_t &code) const {
			return get_code(i, code, std::integral_constant<bool, sizeof(char_type) == 4>());
		}

		bool get_code(size_t &i, uint32_t &code, std::true_type /* sizeof(wchar_t) == 4 */) const {
			if (i >= val.size()) {
				return false;
			}
			code = static_cast<uint32_t>(val.at(i));
			i++;
			return true;
		}

		bool get_code(size_t &i, uint32_t &code, std::false_type /* sizeof(wchar_t) == 2 */) const {
			using namespace foundation::system::exceptions::json;
			if (i >= val.size()) {
				return false;
			}
			code = static_cast<uint32_t>(static_cast<uint16_t>(val.at(i)));
			i++;
			if (!escape_unicode) {
				return true;
			}
			if (constant::UNICODE_SUR_LEAD_BEGIN <= code && code <= constant::UNICODE_SUR_LEAD_END) {
				if (i >= val.size()) {
					throw_exception(json_serialize_error("string was incomplete"));
				}
				const uint32_t lead_surrogate = code;
				const uint32_t trail_surrogate = static_cast<uint16_t>(val.at(i));
				code = merge_surrogates(lead_surrogate, trail_surrogate);
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

		bool get_code(size_t &i, uint32_t &code) const {
			if (i >= val.size()) {
				return false;
			}
			code = static_cast<uint32_t>(val.at(i));
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

		bool get_code(size_t &i, uint32_t &code) const {
			using namespace foundation::system::exceptions::json;
			if (i >= val.size()) {
				return false;
			}
			code = static_cast<uint32_t>(static_cast<uint16_t>(val.at(i)));
			i++;
			if (!escape_unicode) {
				return true;
			}
			if (constant::UNICODE_SUR_LEAD_BEGIN <= code && code <= constant::UNICODE_SUR_LEAD_END) {
				if (i >= val.size()) {
					throw_exception(json_serialize_error("string was incomplete"));
				}
				const uint32_t lead_surrogate = code;
				const uint32_t trail_surrogate = static_cast<uint16_t>(val.at(i));
				code = merge_surrogates(lead_surrogate, trail_surrogate);
				i++;
			}
			return true;
		}
	};

	template <typename type>
	struct unicode_writer;

	template <>
	struct unicode_writer<std::string> {
		using string_type = std::string;
		using char_type = string_type::value_type;
		using char_traits = std::char_traits<char_type>;
		using char_int_type = char_traits::int_type;

		unicode_writer(string_type &buffer) : buffer(buffer){};

		void add_char(const char_int_type ch) const {
			buffer.push_back(char_traits::to_char_type(ch));
		}

		void add_code(const uint32_t code) const {
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

		void add_surrogates(const uint32_t lead_surrogate, const uint32_t trail_surrogate) const {
			add_code(merge_surrogates(lead_surrogate, trail_surrogate));
		}

		string_type &buffer;
	};

	template <>
	struct unicode_writer<std::wstring> {
		using string_type = std::wstring;
		using char_type = string_type::value_type;
		using char_traits = std::char_traits<char_type>;
		using char_int_type = char_traits::int_type;

		unicode_writer(string_type &buffer) : buffer(buffer){};

		void add_char(const char_int_type ch) const {
			buffer.push_back(char_traits::to_char_type(ch));
		}

		void add_code(const uint32_t code) const {
			add_code(code, std::integral_constant<bool, sizeof(char_type) == 4>());
		}

		void add_code(const uint32_t code, std::true_type /* sizeof(wchar_t) == 4 */) const {
			add_char(static_cast<char_int_type>(code));
		}

		void add_code(const uint32_t code, std::false_type /* sizeof(wchar_t) == 2 */) const {
			using namespace rainy::foundation::system::exceptions::json;
			if (code < 0xFFFF) {
				add_char(static_cast<char_int_type>(code));
			} else {
				throw_exception(json_parse_error("invalid 16-bits unicode character"));
			}
		}

		void add_surrogates(const uint32_t lead_surrogate, const uint32_t trail_surrogate) const {
			add_surrogates(lead_surrogate, trail_surrogate, std::integral_constant<bool, sizeof(char_type) == 4>());
		}

		void add_surrogates(const uint32_t lead_surrogate, const uint32_t trail_surrogate,
							std::true_type /* sizeof(wchar_t) == 4 */) const {
			add_code(merge_surrogates(lead_surrogate, trail_surrogate));
		}

		void add_surrogates(const uint32_t lead_surrogate, const uint32_t trail_surrogate,
							std::false_type /* sizeof(wchar_t) == 2 */) const {
			add_code(lead_surrogate);
			add_code(trail_surrogate);
		}

		string_type &buffer;
	};

	template <>
	struct unicode_writer<std::u16string> {
		using string_type = std::u16string;
		using char_type = string_type::value_type;
		using char_traits = std::char_traits<char_type>;
		using char_int_type = char_traits::int_type;

		unicode_writer(string_type &buffer) : buffer(buffer){};

		void add_char(const char_int_type ch) const {
			buffer.push_back(char_traits::to_char_type(ch));
		}

		void add_code(const uint32_t code) const {
			using namespace foundation::system::exceptions::json;
			if (code < 0xFFFF) {
				add_char(static_cast<char_int_type>(code));
			} else {
				throw_exception(json_parse_error("invalid 16-bits unicode character"));
			}
		}

		void add_surrogates(const uint32_t lead_surrogate, const uint32_t trail_surrogate) const {
			add_code(lead_surrogate);
			add_code(trail_surrogate);
		}

		string_type &buffer;
	};

	template <>
	struct unicode_writer<std::u32string> {
		using string_type = std::u32string;
		using char_type = string_type::value_type;
		using char_traits = std::char_traits<char_type>;
		using char_int_type = char_traits::int_type;

		unicode_writer(string_type &buffer) : buffer(buffer){};

		void add_char(const char_int_type ch) const {
			buffer.push_back(char_traits::to_char_type(ch));
		}

		void add_code(const uint32_t code) const {
			add_char(code);
		}

		void add_surrogates(const uint32_t lead_surrogate, const uint32_t trail_surrogate) const {
			add_code(merge_surrogates(lead_surrogate, trail_surrogate));
		}

		string_type &buffer;
	};

	template <typename Ty>
	class output_streambuf final : std::basic_streambuf<Ty> {
	public:
		using char_type = typename std::basic_streambuf<Ty>::char_type;
		using int_type = typename std::basic_streambuf<Ty>::int_type;
		using char_traits = std::char_traits<char_type>;

		output_streambuf(adapters::output_adapter<char_type> *adapter) : adapter(adapter) {
		}

	protected:
		int_type overflow(int_type c) override {
			if (c != EOF)
				adapter->write(char_traits::to_char_type(c));
			return c;
		}

		std::streamsize xsputn(const char_type *s, std::streamsize num) override {
			adapter->write(s, static_cast<size_t>(num));
			return num;
		}

	private:
		adapters::output_adapter<char_type> *adapter;
	};
}
