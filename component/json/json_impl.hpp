#ifndef RAINY_COMPONENT_JSON_IMPL
#define RAINY_COMPONENT_JSON_IMPL
#include <iomanip>
#include <rainy/base.hpp>
#include <rainy/component/json/utils.hpp>
#include <rainy/component/json/json_iter.hpp>

namespace rainy::component::json::implements {
	using namespace rainy::foundation::system::exceptions::json;

	template <typename basic_json>
	struct value {
		using string_type = typename basic_json::string_type;
		using char_type = typename basic_json::char_type;
		using integer_type = typename basic_json::integer_type;
		using float_type = typename basic_json::float_type;
		using boolean_type = typename basic_json::boolean_type;
		using array_type = typename basic_json::array_type;
		using object_type = typename basic_json::object_type;

		enums::json_type type;
		union {
			object_type *object;
			array_type *vector;
			string_type *string;
			integer_type number_integer;
			float_type number_float;
			boolean_type boolean;
		} data;

		value() {
			type = enums::json_type::null;
			data.object = nullptr;
		}

		value(std::nullptr_t) {
			type = enums::json_type::null;
			data.object = nullptr;
		}

		value(const object_type &value) {
			type = enums::json_type::object;
			data.object = create<object_type>(value);
		}

		value(const array_type &value) {
			type = enums::json_type::array;
			data.vector = create<array_type>(value);
		}

		value(const string_type &value) {
			type = enums::json_type::string;
			data.string = create<string_type>(value);
		}

		template <typename _CharT>
		value(const _CharT *str) {
			type = enums::json_type::string;
			data.string = create<string_type>(str);
		}

		value(const integer_type value) {
			type = enums::json_type::number_integer;
			data.number_integer = value;
		}

		value(const float_type value) {
			type = enums::json_type::number_float;
			data.number_float = value;
		}

		value(const boolean_type value) {
			type = enums::json_type::boolean;
			data.boolean = value;
		}

		value(const enums::json_type value_type) {
			type = value_type;
			switch (type) {
				case enums::json_type::object:
					data.object = create<object_type>();
					break;
				case enums::json_type::array:
					data.vector = create<array_type>();
					break;
				case enums::json_type::string:
					data.string = create<string_type>();
					break;
				case enums::json_type::number_integer:
					data.number_integer = integer_type(0);
					break;
				case enums::json_type::number_float:
					data.number_float = float_type(0.0);
					break;
				case enums::json_type::boolean:
					data.boolean = boolean_type(false);
					break;
				default:
					data.object = nullptr;
					break;
			}
		}

		value(value const &other) {
			type = other.type;

			switch (other.type) {
				case enums::json_type::object:
					data.object = create<object_type>(*other.data.object);
					break;
				case enums::json_type::array:
					data.vector = create<array_type>(*other.data.vector);
					break;
				case enums::json_type::string:
					data.string = create<string_type>(*other.data.string);
					break;
				case enums::json_type::number_integer:
					data.number_integer = other.data.number_integer;
					break;
				case enums::json_type::number_float:
					data.number_float = other.data.number_float;
					break;
				case enums::json_type::boolean:
					data.boolean = other.data.boolean;
					break;
				default:
					data.object = nullptr;
					break;
			}
		}

		value(value &&other) noexcept {
			type = other.type;
			data = other.data;
			other.type = enums::json_type::null;
			other.data.object = nullptr;
		}

		~value() {
			clear();
		}

		void swap(value &other) noexcept {
			std::swap(type, other.type);
			std::swap(data, other.data);
		}

		void clear() {
			switch (type) {
				case enums::json_type::object:
					destroy<object_type>(data.object);
					break;
				case enums::json_type::array:
					destroy<array_type>(data.vector);
					break;
				case enums::json_type::string:
					destroy<string_type>(data.string);
					break;
				default:
					break;
			}
		}

		template <typename Ty, typename... Args>
		Ty *create(Args &&...args) {
			using allocator_type = typename basic_json::template allocator_type<Ty>;
			using allocator_traits = std::allocator_traits<allocator_type>;

			allocator_type allocator;
			Ty *ptr = allocator_traits::allocate(allocator, 1);
			allocator_traits::construct(allocator, ptr, std::forward<Args>(args)...);
			return ptr;
		}

		template <typename Ty>
		void destroy(Ty *ptr) {
			using allocator_type = typename basic_json::template allocator_type<Ty>;
			using allocator_traits = std::allocator_traits<allocator_type>;

			allocator_type allocator;
			allocator_traits::destroy(allocator, ptr);
			allocator_traits::deallocate(allocator, ptr, 1);
		}

		value &operator=(value const &other) {
			value{other}.swap(*this);
			return (*this);
		}

		value &operator=(value &&other) noexcept {
			clear();
			type = other.type;
			data = std::move(other.data);
			// invalidate payload
			other.type = enums::json_type::null;
			other.data.object = nullptr;
			return (*this);
		}

		friend bool operator==(const value &lhs, const value &rhs) {
			if (lhs.type == rhs.type) {
				switch (lhs.type) {
					case enums::json_type::array:
						return (*lhs.data.vector == *rhs.data.vector);

					case enums::json_type::object:
						return (*lhs.data.object == *rhs.data.object);

					case enums::json_type::null:
						return true;

					case enums::json_type::string:
						return (*lhs.data.string == *rhs.data.string);

					case enums::json_type::boolean:
						return (lhs.data.boolean == rhs.data.boolean);

					case enums::json_type::number_integer:
						return (lhs.data.number_integer == rhs.data.number_integer);

					case enums::json_type::number_float:
						return utils::nearly_equal(lhs.data.number_float, rhs.data.number_float);

					default:
						return false;
				}
			} else if (lhs.type == enums::json_type::number_integer && rhs.type == enums::json_type::number_float) {
				return utils::nearly_equal<float_type>(static_cast<float_type>(lhs.data.number_integer), rhs.data.number_float);
			} else if (lhs.type == enums::json_type::number_float && rhs.type == enums::json_type::number_integer) {
				return utils::nearly_equal<float_type>(lhs.data.number_float, static_cast<float_type>(rhs.data.number_integer));
			}
			return false;
		}
	};

	template <typename basic_json>
	struct value_getter {
		using string_type = typename basic_json::string_type;
		using char_type = typename basic_json::char_type;
		using integer_type = typename basic_json::integer_type;
		using float_type = typename basic_json::float_type;
		using boolean_type = typename basic_json::boolean_type;
		using array_type = typename basic_json::array_type;
		using object_type = typename basic_json::object_type;

		static void assign(const basic_json &json, object_type &value) {
			if (!json.is_object())
				throw json_type_error("json value type must be object");
			value = *json.value_.data.object;
		}

		static void assign(const basic_json &json, array_type &value) {
			if (!json.is_array())
				throw json_type_error("json value type must be array");
			value = *json.value_.data.vector;
		}

		static void assign(const basic_json &json, string_type &value) {
			if (!json.is_string())
				throw json_type_error("json value type must be string");
			value = *json.value_.data.string;
		}

		static void assign(const basic_json &json, boolean_type &value) {
			if (!json.is_bool())
				throw json_type_error("json value type must be boolean");
			value = json.value_.data.boolean;
		}

		static void assign(const basic_json &json, integer_type &value) {
			if (!json.is_integer())
				throw json_type_error("json value type must be integer");
			value = json.value_.data.number_integer;
		}

		template <typename _IntegerTy, typename std::enable_if<std::is_integral<_IntegerTy>::value, int>::type = 0>
		static void assign(const basic_json &json, _IntegerTy &value) {
			if (!json.is_integer())
				throw json_type_error("json value type must be integer");
			value = static_cast<_IntegerTy>(json.value_.data.number_integer);
		}

		static void assign(const basic_json &json, float_type &value) {
			if (!json.is_float())
				throw json_type_error("json value type must be float");
			value = json.value_.data.number_float;
		}

		template <typename _FloatingTy, typename std::enable_if<std::is_floating_point<_FloatingTy>::value, int>::type = 0>
		static void assign(const basic_json &json, _FloatingTy &value) {
			if (!json.is_float())
				throw json_type_error("json value type must be float");
			value = static_cast<_FloatingTy>(json.value_.data.number_float);
		}
	};

	template <typename basic_json>
	struct json_lexer {
		using string_type = typename basic_json::string_type;
		using char_type = typename basic_json::char_type;
		using integer_type = typename basic_json::integer_type;
		using float_type = typename basic_json::float_type;
		using boolean_type = typename basic_json::boolean_type;
		using array_type = typename basic_json::array_type;
		using object_type = typename basic_json::object_type;
		using char_traits = std::char_traits<char_type>;
		using char_int_type = typename char_traits::int_type;

		json_lexer(adapters::input_adapter<char_type> *adapter) : adapter_(adapter) {
			// read first char
			read_next();
		}

		char_int_type read_next() {
			current = adapter_->get_char();
			return current;
		}

		void skip_spaces() {
			while (current == ' ' || current == '\t' || current == '\n' || current == '\r') {
				read_next();
			}
		}

		enums::token_type scan() {
			skip_spaces();

			enums::token_type result;
			switch (current) {
				case '[':
					result = enums::token_type::begin_array;
					break;
				case ']':
					result = enums::token_type::end_array;
					break;
				case '{':
					result = enums::token_type::begin_object;
					break;
				case '}':
					result = enums::token_type::end_object;
					break;
				case ':':
					result = enums::token_type::name_separator;
					break;
				case ',':
					result = enums::token_type::value_separator;
					break;
				case 't':
					return scan_literal({'t', 'r', 'u', 'e'}, enums::token_type::literal_true);
				case 'f':
					return scan_literal({'f', 'a', 'l', 's', 'e'}, enums::token_type::literal_false);
				case 'n':
					return scan_literal({'n', 'u', 'l', 'l'}, enums::token_type::literal_null);

				case '\"':
					return scan_string();
				case '-':
				case '0':
				case '1':
				case '2':
				case '3':
				case '4':
				case '5':
				case '6':
				case '7':
				case '8':
				case '9':
					return scan_number();
				case '\0':
				case char_traits::eof():
					return enums::token_type::end_of_input;
				default:
					throw json_parse_error("unexpected character");
			}
			read_next();

			return result;
		}

		enums::token_type scan_literal(std::initializer_list<char_type> text, enums::token_type result) {
			for (const auto ch: text) {
				if (ch != char_traits::to_char_type(current)) {
					throw json_parse_error("unexpected literal");
				}
				read_next();
			}
			return result;
		}

		enums::token_type scan_string() {
			if (current != '\"')
				throw json_parse_error("string must start with '\"'");

			string_buffer_.clear();

			while (true) {
				switch (const auto ch = read_next();ch) {
					case char_traits::eof(): {
						throw json_parse_error("unexpected end");
					}

					case '\"': {
						// skip last `\"`
						read_next();
						return enums::token_type::value_string;
					}

					case 0x00:
					case 0x01:
					case 0x02:
					case 0x03:
					case 0x04:
					case 0x05:
					case 0x06:
					case 0x07:
					case 0x08:
					case 0x09:
					case 0x0A:
					case 0x0B:
					case 0x0C:
					case 0x0D:
					case 0x0E:
					case 0x0F:
					case 0x10:
					case 0x11:
					case 0x12:
					case 0x13:
					case 0x14:
					case 0x15:
					case 0x16:
					case 0x17:
					case 0x18:
					case 0x19:
					case 0x1A:
					case 0x1B:
					case 0x1C:
					case 0x1D:
					case 0x1E:
					case 0x1F:
						throw json_parse_error("invalid control character");
					case '\\': {
						switch (const uint32_t code = read_one_escaped_code();read_next()) {
							case '\"':
								add_char('\"');
								break;
							case '\\':
								add_char('\\');
								break;
							case '/':
								add_char('/');
								break;
							case 'b':
								add_char('\b');
								break;
							case 'f':
								add_char('\f');
								break;
							case 'n':
								add_char('\n');
								break;
							case 'r':
								add_char('\r');
								break;
							case 't':
								add_char('\t');
								break;
							case 'u': {
								if (constant::UNICODE_SUR_LEAD_BEGIN <= code && code <= constant::UNICODE_SUR_LEAD_END) {
									if (read_next() != '\\' || read_next() != 'u') {
										throw json_parse_error("lead surrogate must be followed by trail surrogate");
									}

									const auto lead_surrogate = code;
									const auto trail_surrogate = read_one_escaped_code();

									if (!(constant::UNICODE_SUR_LEAD_BEGIN <= trail_surrogate &&
										  trail_surrogate <= constant::UNICODE_SUR_LEAD_END)) {
										throw json_parse_error("surrogate U+D800...U+DBFF must be followed by U+DC00...U+DFFF");
									}
									utils::unicode_writer<string_type> uw(this->string_buffer_);
									uw.add_surrogates(lead_surrogate, trail_surrogate);
								} else {
									utils::unicode_writer<string_type> uw(this->string_buffer_);
									uw.add_code(code);
								}
								break;
							}

							default: {
								throw json_parse_error("invalid character");
							}
						}
						break;
					}

					default: {
						add_char(ch);
					}
				}
			}
		}

		enums::token_type scan_number() {
			is_negative_ = false;
			number_value_ = static_cast<float_type>(0.0);
			if (current == '-') {
				is_negative_ = true;
				read_next();
				return scan_integer();
			}

			if (current == '0') {
				if (read_next() == '.')
					return scan_float();
				else
					return enums::token_type::value_integer;
			}
			return scan_integer();
		}

		enums::token_type scan_integer() {
			if (!std::isdigit(current))
				throw json_parse_error("invalid integer");

			number_value_ = static_cast<float_type>(current - '0');

			while (true) {
				const auto ch = read_next();
				if (ch == '.')
					return scan_float();

				if (ch == 'e' || ch == 'E')
					return scan_exponent();

				if (std::isdigit(ch))
					number_value_ = number_value_ * 10 + (ch - '0');
				else
					break;
			}
			return enums::token_type::value_integer;
		}

		enums::token_type scan_float() {
			if (current != '.')
				throw json_parse_error("float number must start with '.'");

			read_next(); // 读取下一字符
			if (!std::isdigit(current))
				throw json_parse_error("invalid float number");

			float_type fraction = static_cast<float_type>(0.1);
			number_value_ = static_cast<float_type>(current - '0') * fraction;

			read_next(); // 读取下一字符

			while (true) {
				if (current == 'e' || current == 'E') {
					return scan_exponent();  // 确保 scan_exponent() 正常返回 enums::token_type
				}
				if (std::isdigit(current)) {
					fraction *= static_cast<float_type>(0.1);
					number_value_ += static_cast<float_type>(current - '0') * fraction;
					read_next(); // 读取下一字符
				} else {
					break;
				}
			}
			return enums::token_type::value_float;
		}



		enums::token_type scan_exponent() {
			if (current != 'e' && current != 'E')
				throw json_parse_error("exponent number must contains 'e' or 'E'");
			read_next();
			if (const bool invalid = (std::isdigit(current) && current != '0') || (current == '-') || (current == '+');!invalid)
				throw_exception(json_parse_error("invalid exponent number"));
			float_type base = 10;
			if (current == '+') {
				read_next();
			} else if (current == '-') {
				base = static_cast<float_type>(0.1);
				read_next();
			}
			uint32_t exponent = static_cast<uint32_t>(current - '0');
			while (std::isdigit(read_next())) {
				exponent = (exponent * 10) + static_cast<uint32_t>(current - '0');
			}
			float_type power = 1;
			for (; exponent; exponent >>= 1, base *= base)
				if (exponent & 1)
					power *= base;
			number_value_ *= power;
			return enums::token_type::value_float;
		}

		integer_type token_to_integer() const {
			integer_type integer = static_cast<integer_type>(number_value_);
			return is_negative_ ? -integer : integer;
		}

		float_type token_to_float() const {
			return is_negative_ ? -number_value_ : number_value_;
		}

		string_type token_to_string() const {
			return string_buffer_;
		}

		uint32_t read_one_escaped_code() {
			uint32_t code = 0;
			for (const auto factor: {12, 8, 4, 0}) {
				const auto ch = read_next();
				if (ch >= '0' && ch <= '9') {
					code += ((ch - '0') << factor);
				} else if (ch >= 'A' && ch <= 'F') {
					code += ((ch - 'A' + 10) << factor);
				} else if (ch >= 'a' && ch <= 'f') {
					code += ((ch - 'a' + 10) << factor);
				} else {
					throw json_parse_error("'\\u' must be followed by 4 hex digits");
				}
			}
			return code;
		}

		void add_char(const char_int_type ch) {
			string_buffer_.push_back(char_traits::to_char_type(ch));
		}

	private:
		bool is_negative_;
		float_type number_value_;
		string_type string_buffer_;
		adapters::input_adapter<char_type> *adapter_;
		char_int_type current;
	};

	template <typename basic_json>
	struct json_parser {
		using string_type = typename basic_json::string_type;
		using char_type = typename basic_json::char_type;
		using integer_type = typename basic_json::integer_type;
		using float_type = typename basic_json::float_type;
		using boolean_type = typename basic_json::boolean_type;
		using array_type = typename basic_json::array_type;
		using object_type = typename basic_json::object_type;
		using char_traits = std::char_traits<char_type>;

		json_parser(adapters::input_adapter<char_type> *adapter) : lexer(adapter), last_token(enums::token_type::uninitialized) {
		}

		void parse(basic_json &json) {
			parse_value(json);

			if (get_token() != enums::token_type::end_of_input)
				throw json_parse_error("unexpected token, expect end");
		}

	private:
		enums::token_type get_token() {
			last_token = lexer.scan();
			return last_token;
		}

		void parse_value(basic_json &json, bool read_next = true) {
			enums::token_type token = last_token;
			if (read_next) {
				token = get_token();
			}

			switch (token) {
				case enums::token_type::literal_true:
					json = enums::json_type::boolean;
					json.value_.data.boolean = true;
					break;

				case enums::token_type::literal_false:
					json = enums::json_type::boolean;
					json.value_.data.boolean = false;
					break;

				case enums::token_type::literal_null:
					json = enums::json_type::null;
					break;

				case enums::token_type::value_string:
					json = lexer.token_to_string();
					break;

				case enums::token_type::value_integer:
					json = lexer.token_to_integer();
					break;

				case enums::token_type::value_float:
					json = lexer.token_to_float();
					break;

				case enums::token_type::begin_array:
					json = enums::json_type::array;
					while (true) {
						if (get_token() == enums::token_type::end_array)
							break;

						json.value_.data.vector->push_back(basic_json());
						parse_value(json.value_.data.vector->back(), false);

						// read ','
						if (get_token() != enums::token_type::value_separator)
							break;
					}
					if (last_token != enums::token_type::end_array)
						throw json_parse_error("unexpected token in array");
					break;

				case enums::token_type::begin_object:
					json = enums::json_type::object;
					while (true) {
						if (get_token() != enums::token_type::value_string)
							break;
						string_type key = lexer.token_to_string();

						if (get_token() != enums::token_type::name_separator)
							break;

						basic_json object;
						parse_value(object);
						json.value_.data.object->insert(std::make_pair(key, object));

						// read ','
						if (get_token() != enums::token_type::value_separator)
							break;
					}
					if (last_token != enums::token_type::end_object)
						throw_exception(json_parse_error("unexpected token in object"));
					break;

				default:
					throw_exception(json_parse_error("unexpected token"));
			}
		}

		json_lexer<basic_json> lexer;
		enums::token_type last_token;
	};

	template <typename basic_json>
	struct json_serializer {
		using string_type = typename basic_json::string_type;
		using char_type = typename basic_json::char_type;
		using integer_type = typename basic_json::integer_type;
		using float_type = typename basic_json::float_type;
		using boolean_type = typename basic_json::boolean_type;
		using array_type = typename basic_json::array_type;
		using object_type = typename basic_json::object_type;
		using char_traits = std::char_traits<char_type>;
		using char_int_type = typename char_traits::int_type;
		using args = utils::serializer_args<basic_json>;

		json_serializer(adapters::output_adapter<char_type> *adapter, const args &args) :
			buf_(adapter), out_(&buf_), arg(args), indent_(), newline_func_(stream_do_nothing) {
			out_ << std::setprecision(arg.precision);
			out_ << std::right;
			out_ << std::noshowbase;
			if (const bool pretty_print = arg.indent > 0;pretty_print) {
				newline_func_ = stream_new_line;
				indent_.init(arg.indent_char);
			}
		}

		void dump(const basic_json &json, const unsigned int currentindent = 0) {
			switch (json.type()) {
				case enums::json_type::object: {
					auto &object = *json.value_.data.object;
					if (object.empty()) {
						out_ << '{' << '}';
						return;
					}
					out_ << '{' << newline_func_;
					auto iter = object.cbegin();
					const auto size = object.size();
					const auto new_indent = currentindent + arg.indent;
					for (std::size_t i = 0; i < size; ++i, ++iter) {
						out_ << indent_.set(new_indent);
						out_ << '\"' << iter->first << '\"' << ':';
						out_ << indent_.set(1);
						dump(iter->second, new_indent);
						if (i != size - 1)
							out_ << ',' << newline_func_;
					}
					out_ << newline_func_ << indent_.set(currentindent) << '}';
					return;
				}

				case enums::json_type::array: {
					auto &v = *json.value_.data.vector;

					if (v.empty()) {
						out_ << '[' << ']';
						return;
					}

					out_ << '[' << newline_func_;

					const auto new_indent = currentindent + arg.indent;
					const auto size = v.size();
					for (std::size_t i = 0; i < size; ++i) {
						out_ << indent_.set(new_indent);
						dump(v.at(i), new_indent);

						// not last element
						if (i != size - 1) {
							out_ << ',' << newline_func_;
						}
					}
					out_ << newline_func_ << indent_.set(currentindent) << ']';
					return;
				}

				case enums::json_type::string: {
					out_ << '\"';
					dump_string(*json.value_.data.string);
					out_ << '\"';
					return;
				}

				case enums::json_type::boolean: {
					if (json.value_.data.boolean) {
						out_ << 't' << 'r' << 'u' << 'e';
					} else {
						out_ << 'f' << 'a' << 'l' << 's' << 'e';
					}
					return;
				}

				case enums::json_type::number_integer: {
					out_ << json.value_.data.number_integer;
					return;
				}

				case enums::json_type::number_float: {
					out_ << json.value_.data.number_float;
					return;
				}

				case enums::json_type::null: {
					out_ << 'n' << 'u' << 'l' << 'l';
					return;
				}
			}
		}

		void dump_string(const string_type &val) {
			size_t i = 0;
			uint32_t code = 0;
			utils::unicode_reader<string_type> ur(val, arg.escape_unicode);
			while (ur.get_code(i, code)) {
				switch (code) {
					case '\t': {
						out_ << '\\' << 't';
						break;
					}
					case '\r': {
						out_ << '\\' << 'r';
						break;
					}
					case '\n': {
						out_ << '\\' << 'n';
						break;
					}

					case '\b': {
						out_ << '\\' << 'b';
						break;
					}

					case '\f': {
						out_ << '\\' << 'f';
						break;
					}

					case '\"': {
						out_ << '\\' << '\"';
						break;
					}

					case '\\': {
						out_ << '\\' << '\\';
						break;
					}

					default: {
						const bool need_escape = code <= 0x1F || (arg.escape_unicode && code >= 0x7F);
						if (!need_escape)
							out_ << char_traits::to_char_type(static_cast<char_int_type>(code));
						else {
							if (code <= 0xFFFF) {
								out_ << std::setfill(char_type('0')) << std::hex;
								out_ << '\\' << 'u' << std::setw(4) << static_cast<uint16_t>(code);
								out_ << std::dec;
							} else {
								code = code - constant::UNICODE_SUR_BASE;
								const auto lead_surrogate =
										static_cast<uint16_t>(constant::UNICODE_SUR_LEAD_BEGIN + (code >> constant::UNICODE_SUR_BITS));
								const auto trail_surrogate =
										static_cast<uint16_t>(constant::UNICODE_SUR_LEAD_BEGIN + (code & constant::UNICODE_SUR_MAX));
								out_ << std::setfill(char_type('0')) << std::hex;
								out_ << '\\' << 'u' << std::setw(4) << lead_surrogate;
								out_ << '\\' << 'u' << std::setw(4) << trail_surrogate;
								out_ << std::dec;
							}
						}
						break;
					}
				}
			}
		}

	private:
		using ostream_type = std::basic_ostream<char_type>;
		using ostreambuf_type = utils::output_streambuf<char_type>;

		class indent {
		public:
			indent() : length_(0), indent_char_(0), indent_string_() {
			}

			void init(char_type ch) {
				indent_char_ = ch;
				indent_string_.resize(16, indent_char_);
			}

			indent &set(unsigned int length) {
				length_ = length;
				if (indent_string_.size() < static_cast<size_t>(length)) {
					indent_string_.resize(indent_string_.size() * 2, indent_char_);
				}
				return *this;
			}

			friend ostream_type &operator<<(ostream_type &os, const indent &i) {
				if (i.indent_char_) {
					os.write(i.indent_string_.c_str(), static_cast<std::streamsize>(i.length_));
				}
				return os;
			}

		private:
			unsigned int length_;
			char_type indent_char_;
			string_type indent_string_;
		};

		static std::basic_ostream<char_type> &stream_do_nothing(std::basic_ostream<char_type> &out) {
			return out;
		}

		static std::basic_ostream<char_type> &stream_new_line(std::basic_ostream<char_type> &out) {
			return out << '\n';
		}

		ostreambuf_type buf_;
		ostream_type out_;
		const args &arg;
		indent indent_;
		using stream_func = ostream_type &(*) (ostream_type &);
		stream_func newline_func_;
	};

	template <template <class _Kty, class _Ty, class... Args> class _ObjectTy = std::map,
			  template <class _Kty, class... Args> class _ArrayTy = std::vector, typename _StringTy = std::string,
			  typename _IntegerTy = std::int32_t, typename _FloatTy = double, typename _BooleanTy = bool,
			  template <class _Ty> class _Allocator = std::allocator>
	class basic_json {
	public:
		friend struct iterators::json_iterator<basic_json>;
		friend struct iterators::json_iterator<const basic_json>;
		friend struct json_serializer<basic_json>;
		friend struct json_parser<basic_json>;
		friend struct value_getter<basic_json>;

		template <typename Ty>
		using allocator_type = _Allocator<Ty>;
		using size_type = std::size_t;
		using difference_type = std::ptrdiff_t;
		using string_type = _StringTy;
		using char_type = typename _StringTy::value_type;
		using integer_type = _IntegerTy;
		using float_type = _FloatTy;
		using boolean_type = _BooleanTy;
		using array_type = _ArrayTy<basic_json, allocator_type<basic_json>>;
		using object_type =
				_ObjectTy<string_type, basic_json, std::less<string_type>, allocator_type<std::pair<const string_type, basic_json>>>;
		using initializer_list = std::initializer_list<basic_json>;
		using iterator = iterators::json_iterator<basic_json>;
		using const_iterator = iterators::json_iterator<const basic_json>;
		using reverse_iterator = std::reverse_iterator<iterator>;
		using const_reverse_iterator = std::reverse_iterator<const_iterator>;
		using dumpArgs = utils::serializer_args<basic_json>;

		basic_json() noexcept = default;

		basic_json(std::nullptr_t) {
		}

		basic_json(const enums::json_type type) : value_(type) {
		}

		basic_json(basic_json const &other) : value_(other.value_) {
		}

		basic_json(basic_json &&other) noexcept : value_(std::move(other.value_)) {
			// invalidate payload
			other.value_.type = enums::json_type::null;
			other.value_.data.object = nullptr;
		}

		basic_json(string_type const &value) : value_(value) {
		}

		template <typename _Compatible>
			requires std::is_constructible<string_type, _Compatible>::value
		basic_json(const _Compatible &value) {
			value_.type = enums::json_type::string;
			value_.data.string = value_.template create<string_type>(value);
		}

		basic_json(array_type const &arr) : value_(arr) {
		}

		basic_json(object_type const &object) : value_(object) {
		}

		basic_json(integer_type value) : value_(value) {
		}

		template <typename _IntegerUTy, typename std::enable_if<std::is_integral<_IntegerUTy>::value, int>::type = 0>
		basic_json(_IntegerUTy value) : value_(static_cast<integer_type>(value)) {
		}

		basic_json(float_type value) : value_(value) {
		}

		template <typename _FloatingTy, typename std::enable_if<std::is_floating_point<_FloatingTy>::value, int>::type = 0>
		basic_json(_FloatingTy value) : value_(static_cast<float_type>(value)) {
		}

		basic_json(boolean_type value) : value_(value) {
		}

		basic_json(initializer_list const &init_list, enums::json_type exact_type = enums::json_type::null) {
			bool is_an_object = std::all_of(init_list.begin(), init_list.end(), [](const basic_json &json) {
				return (json.is_array() && json.size() == 2 && json[0].is_string());
			});

			if (exact_type != enums::json_type::object && exact_type != enums::json_type::array) {
				exact_type = is_an_object ? enums::json_type::object : enums::json_type::array;
			}

			if (exact_type == enums::json_type::object) {
				assert(is_an_object);
				value_ = enums::json_type::object;

				std::for_each(init_list.begin(), init_list.end(), [this](const basic_json &json) {
					value_.data.object->emplace(*((*json.value_.data.vector)[0].value_.data.string), (*json.value_.data.vector)[1]);
				});
			} else {
				value_ = enums::json_type::array;
				value_.data.vector->reserve(init_list.size());
				value_.data.vector->assign(init_list.begin(), init_list.end());
			}
		}

		static basic_json object(initializer_list const &init_list) {
			return basic_json(init_list, enums::json_type::object);
		}

		static basic_json array(initializer_list const &init_list) {
			return basic_json(init_list, enums::json_type::array);
		}

		bool is_object() const {
			return value_.type == enums::json_type::object;
		}

		bool is_array() const {
			return value_.type == enums::json_type::array;
		}

		bool is_string() const {
			return value_.type == enums::json_type::string;
		}

		bool is_bool() const {
			return value_.type == enums::json_type::boolean;
		}

		bool is_integer() const {
			return value_.type == enums::json_type::number_integer;
		}

		bool is_float() const {
			return value_.type == enums::json_type::number_float;
		}

		bool is_number() const {
			return is_integer() || is_float();
		}

		bool is_null() const {
			return value_.type == enums::json_type::null;
		}

		enums::json_type type() const {
			return value_.type;
		}

		string_type type_name() const {
			switch (type()) {
				case enums::json_type::object:
					return string_type("object");
				case enums::json_type::array:
					return string_type("array");
				case enums::json_type::string:
					return string_type("string");
				case enums::json_type::number_integer:
					return string_type("integer");
				case enums::json_type::number_float:
					return string_type("float");
				case enums::json_type::boolean:
					return string_type("boolean");
				case enums::json_type::null:
					return string_type("null");
			}
			return string_type();
		}

		void swap(basic_json &rhs) noexcept {
			value_.swap(rhs.value_);
		}

		iterator begin() {
			iterator iter(this);
			iter.set_begin();
			return iter;
		}
		const_iterator begin() const {
			return cbegin();
		}
		const_iterator cbegin() const {
			const_iterator iter(this);
			iter.set_begin();
			return iter;
		}
		iterator end() {
			iterator iter(this);
			iter.set_end();
			return iter;
		}
		const_iterator end() const {
			return cend();
		}
		const_iterator cend() const {
			const_iterator iter(this);
			iter.set_end();
			return iter;
		}
		reverse_iterator rbegin() {
			return reverse_iterator(end());
		}
		const_reverse_iterator rbegin() const {
			return const_reverse_iterator(end());
		}
		const_reverse_iterator crbegin() const {
			return rbegin();
		}
		reverse_iterator rend() {
			return reverse_iterator(begin());
		}
		const_reverse_iterator rend() const {
			return const_reverse_iterator(begin());
		}
		const_reverse_iterator crend() const {
			return rend();
		}

		size_type size() const {
			switch (type()) {
				case enums::json_type::null:
					return 0;
				case enums::json_type::array:
					return value_.data.vector->size();
				case enums::json_type::object:
					return value_.data.object->size();
				default:
					return 1;
			}
		}

		bool empty() const {
			if (is_null())
				return true;

			if (is_object())
				return value_.data.object->empty();

			if (is_array())
				return value_.data.vector->empty();
			return false;
		}

		template <typename _Kty>
		const_iterator find(_Kty &&key) const {
			if (is_object()) {
				const_iterator iter(this);
				iter.object_it_ = value_.data.object->find(std::forward<_Kty>(key));
				return iter;
			}
			return cend();
		}

		template <typename _Kty>
		size_type count(_Kty &&key) const {
			return is_object() ? value_.data.object->count(std::forward<_Kty>(key)) : 0;
		}

		size_type erase(const typename object_type::key_type &key) {
			if (!is_object()) {
				throw json_invalid_key("cannot use erase() with non-object value");
			}
			return value_.data.object->erase(key);
		}

		void erase(const size_type index) {
			if (!is_array()) {
				throw json_invalid_key("cannot use erase() with non-array value");
			}
			value_.data.vector->erase(value_.data.vector->begin() + static_cast<difference_type>(index));
		}

		template <class _IteratorTy,
				  typename std::enable_if<std::is_same<_IteratorTy, iterator>::value || std::is_same<_IteratorTy, const_iterator>::value,
										  int>::type = 0>
		_IteratorTy erase(_IteratorTy pos) {
			_IteratorTy result = end();

			switch (type()) {
				case enums::json_type::object: {
					result.it_.object_iter = value_.data.object->erase(pos.it_.object_iter);
					break;
				}

				case enums::json_type::array: {
					result.it_.array_iter = value_.data.vector->erase(pos.it_.array_iter);
					break;
				}

				default:
					throw json_invalid_iterator("cannot use erase() with non-object & non-array value");
			}

			return result;
		}

		template <class _IteratorTy,
				  typename std::enable_if<std::is_same<_IteratorTy, iterator>::value || std::is_same<_IteratorTy, const_iterator>::value,
										  int>::type = 0>
		_IteratorTy erase(_IteratorTy first, _IteratorTy last) {
			_IteratorTy result = end();

			switch (type()) {
				case enums::json_type::object: {
					result.it_.object_iter = value_.data.object->erase(first.it_.object_iter, last.it_.object_iter);
					break;
				}

				case enums::json_type::array: {
					result.it_.array_iter = value_.data.vector->erase(first.it_.array_iter, last.it_.array_iter);
					break;
				}

				default:
					throw json_invalid_iterator("cannot use erase() with non-object & non-array value");
			}

			return result;
		}

		void push_back(basic_json &&json) {
			if (!is_null() && !is_array()) {
				throw json_type_error("cannot use push_back() with non-array value");
			}

			if (is_null()) {
				value_ = enums::json_type::array;
			}

			value_.data.vector->push_back(std::move(json));
		}

		basic_json &operator+=(basic_json &&json) {
			push_back(std::move(json));
			return (*this);
		}

		void clear() {
			switch (type()) {
				case enums::json_type::number_integer: {
					value_.data.number_integer = 0;
					break;
				}

				case enums::json_type::number_float: {
					value_.data.number_float = static_cast<float_type>(0.0);
					break;
				}

				case enums::json_type::boolean: {
					value_.data.boolean = false;
					break;
				}

				case enums::json_type::string: {
					value_.data.string->clear();
					break;
				}

				case enums::json_type::array: {
					value_.data.vector->clear();
					break;
				}

				case enums::json_type::object: {
					value_.data.object->clear();
					break;
				}

				default:
					break;
			}
		}

		bool get_value(boolean_type &val) const {
			if (is_bool()) {
				val = value_.data.boolean;
				return true;
			}
			return false;
		}

		bool get_value(integer_type &val) const {
			if (is_integer()) {
				val = value_.data.number_integer;
				return true;
			}
			if (is_float()) {
				val = static_cast<integer_type>(value_.data.number_float);
				return true;
			}
			return false;
		}

		bool get_value(float_type &val) const {
			if (is_float()) {
				val = value_.data.number_float;
				return true;
			}
			if (is_integer()) {
				val = static_cast<float_type>(value_.data.number_integer);
				return true;
			}
			return false;
		}

		template <typename _IntegerUTy, typename std::enable_if<std::is_integral<_IntegerUTy>::value, int>::type = 0>
		bool get_value(_IntegerUTy &val) const {
			if (is_integer()) {
				val = static_cast<_IntegerUTy>(value_.data.number_integer);
				return true;
			}
			if (is_float()) {
				val = static_cast<_IntegerUTy>(value_.data.number_float);
				return true;
			}
			return false;
		}

		template <typename _FloatingTy, typename std::enable_if<std::is_floating_point<_FloatingTy>::value, int>::type = 0>
		bool get_value(_FloatingTy &val) const {
			if (is_float()) {
				val = static_cast<_FloatingTy>(value_.data.number_float);
				return true;
			}
			if (is_integer()) {
				val = static_cast<_FloatingTy>(value_.data.number_integer);
				return true;
			}
			return false;
		}

		bool get_value(array_type &val) const {
			if (is_array()) {
				val.assign((*value_.data.vector).begin(), (*value_.data.vector).end());
				return true;
			}
			return false;
		}

		bool get_value(string_type &val) const {
			if (is_string()) {
				val.assign(*value_.data.string);
				return true;
			}
			return false;
		}

		bool get_value(object_type &val) const {
			if (is_object()) {
				val.assign(*value_.data.object);
				return true;
			}
			return false;
		}

		boolean_type as_bool() const {
			if (!is_bool())
				throw json_type_error("json value must be boolean");
			return value_.data.boolean;
		}

		integer_type as_integer() const {
			if (!is_number())
				throw json_type_error("json value must be integer");
			if (is_float())
				return static_cast<integer_type>(value_.data.number_float);
			return value_.data.number_integer;
		}

		float_type as_float() const {
			if (!is_number())
				throw json_type_error("json value must be float");
			if (is_integer())
				return static_cast<float_type>(value_.data.number_integer);
			return value_.data.number_float;
		}

		const array_type &as_array() const {
			if (!is_array())
				throw json_type_error("json value must be array");
			return *value_.data.vector;
		}

		const string_type &as_string() const {
			if (!is_string())
				throw json_type_error("json value must be string");
			return *value_.data.string;
		}

		const object_type &as_object() const {
			if (!is_object())
				throw json_type_error("json value must be object");
			return *value_.data.object;
		}

		template <typename Ty>
		Ty get() const {
			Ty value{};
			value_getter<basic_json>::assign(*this, value);
			return value;
		}

		basic_json &operator=(basic_json const &other) {
			value_ = other.value_;
			return (*this);
		}

		basic_json &operator=(basic_json &&other) noexcept {
			value_ = std::move(other.value_);
			return (*this);
		}
		// operator[] functions

		basic_json &operator[](size_type index) {
			if (is_null())
				value_ = enums::json_type::array;
			if (!is_array())
				throw json_invalid_key("operator[] called on a non-array object");
			if (index >= value_.data.vector->size())
				value_.data.vector->insert(value_.data.vector->end(), index - value_.data.vector->size() + 1, basic_json());
			return (*value_.data.vector)[index];
		}

		basic_json &operator[](size_type index) const {
			if (!is_array())
				throw json_invalid_key("operator[] called on a non-array type");
			if (index >= value_.data.vector->size())
				throw std::out_of_range("operator[] index out of range");
			return (*value_.data.vector)[index];
		}

		basic_json &operator[](const typename object_type::key_type &key) {
			if (is_null()) {
				value_ = enums::json_type::object;
			}

			if (!is_object()) {
				throw json_invalid_key("operator[] called on a non-object type");
			}
			return (*value_.data.object)[key];
		}

		basic_json &operator[](const typename object_type::key_type &key) const {
			if (!is_object()) {
				throw json_invalid_key("operator[] called on a non-object object");
			}

			auto iter = value_.data.object->find(key);
			if (iter == value_.data.object->end()) {
				throw std::out_of_range("operator[] key out of range");
			}
			return iter->second;
		}

		template <typename _CharT>
		basic_json &operator[](_CharT *key) {
			if (is_null()) {
				value_ = enums::json_type::object;
			}

			if (!is_object()) {
				throw json_invalid_key("operator[] called on a non-object object");
			}
			return (*value_.data.object)[key];
		}

		template <typename _CharT>
		basic_json &operator[](_CharT *key) const {
			if (!is_object()) {
				throw json_invalid_key("operator[] called on a non-object object");
			}

			auto iter = value_.data.object->find(key);
			if (iter == value_.data.object->end()) {
				throw std::out_of_range("operator[] key out of range");
			}
			return iter->second;
		}

		template <typename Ty>
		explicit operator Ty() const {
			return get<Ty>();
		}

		friend std::basic_ostream<char_type> &operator<<(std::basic_ostream<char_type> &out, const basic_json &j) {
			using char_type = typename std::basic_ostream<char_type>::char_type;

			dumpArgs args;
			args.indent = static_cast<unsigned int>(out.width());
			args.indent_char = out.fill();
			args.precision = static_cast<int>(out.precision());

			out.width(0);

			adapters::stream_output_adapter<char_type> adapter(out);
			j.dump(&adapter, args);
			return out;
		}

		string_type dump(const dumpArgs &args = dumpArgs{}) const {
			string_type result{};
			adapters::string_output_adapter<string_type> adapter(result);
			this->dump(&adapter, args);
			return result;
		}

		string_type dump(unsigned int indent, char_type indent_char = ' ', bool escape_unicode = false) const {
			dumpArgs args;
			args.indent = indent;
			args.indent_char = indent_char;
			args.escape_unicode = escape_unicode;
			return this->dump(args);
		}

		void dump(adapters::output_adapter<char_type> *adapter, const dumpArgs &args = dumpArgs()) const {
			json_serializer<basic_json>(adapter, args).dump(*this);
		}

		friend std::basic_istream<char_type> &operator>>(std::basic_istream<char_type> &in, basic_json &json) {
			adapters::stream_input_adapter<char_type> adapter(in);
			json_parser<basic_json>(&adapter).parse(json);
			return in;
		}

		static basic_json parse(const string_type &str) {
			adapters::string_input_adapter<string_type> adapter(str);
			return parse(&adapter);
		}

		static basic_json parse(const char_type *str) {
			adapters::buffer_input_adapter<char_type> adapter(str);
			return parse(&adapter);
		}

		static basic_json parse(std::FILE *file) {
			adapters::file_input_adapter<char_type> adapter(file);
			return parse(&adapter);
		}

		static basic_json parse(adapters::input_adapter<char_type> *adapter) {
			basic_json result;
			json_parser<basic_json>(adapter).parse(result);
			return result;
		}

		friend bool operator==(const basic_json &lhs, const basic_json &rhs) {
			return lhs.value_ == rhs.value_;
		}

		friend bool operator!=(const basic_json &lhs, const basic_json &rhs) {
			return !(lhs == rhs);
		}

		friend bool operator<(const basic_json &lhs, const basic_json &rhs) {
			const auto lhs_type = lhs.type();
			const auto rhs_type = rhs.type();

			if (lhs_type == rhs_type) {
				switch (lhs_type) {
					case enums::json_type::array:
						return (*lhs.value_.data.vector) < (*rhs.value_.data.vector);

					case enums::json_type::object:
						return (*lhs.value_.data.object) < (*rhs.value_.data.object);

					case enums::json_type::null:
						return false;

					case enums::json_type::string:
						return (*lhs.value_.data.string) < (*rhs.value_.data.string);

					case enums::json_type::boolean:
						return (lhs.value_.data.boolean < rhs.value_.data.boolean);

					case enums::json_type::number_integer:
						return (lhs.value_.data.number_integer < rhs.value_.data.number_integer);

					case enums::json_type::number_float:
						return (lhs.value_.data.number_float < rhs.value_.data.number_float);

					default:
						return false;
				}
			}
			if (lhs_type == enums::json_type::number_integer && rhs_type == enums::json_type::number_float) {
				return (static_cast<float_type>(lhs.value_.data.number_integer) < rhs.value_.data.number_float);
			}
			if (lhs_type == enums::json_type::number_float && rhs_type == enums::json_type::number_integer) {
				return (lhs.value_.data.number_float < static_cast<float_type>(rhs.value_.data.number_integer));
			}

			return false;
		}

		friend bool operator<=(const basic_json &lhs, const basic_json &rhs) {
			return !(rhs < lhs);
		}

		friend bool operator>(const basic_json &lhs, const basic_json &rhs) {
			return rhs < lhs;
		}

		friend bool operator>=(const basic_json &lhs, const basic_json &rhs) {
			return !(lhs < rhs);
		}

	private:
		value<basic_json> value_;
	};
}


#endif
