#pragma once
namespace rainy::component::json {
	namespace constant {
		inline constexpr int UNICODE_SUR_BASE = 0x10000;
		inline constexpr int UNICODE_SUR_LEAD_BEGIN = 0xD800;
		inline constexpr int UNICODE_SUR_LEAD_END = 0xDBFF;
		inline constexpr int UNICODE_SUR_TRAIL_BEGIN = 0xDC00;
		inline constexpr int UNICODE_SUR_TRAIL_END = 0xDFFF;
		inline constexpr int UNICODE_SUR_BITS = 10;
		inline constexpr int UNICODE_SUR_MAX = 0x3FF;
	}

	namespace enums {
		enum class json_type
		{
			number_integer,
			number_float,
			string,
			array,
			object,
			boolean,
			null,
		};

		enum class token_type
		{
			uninitialized,
			literal_true,
			literal_false,
			literal_null,
			value_string,
			value_integer,
			value_float,
			begin_array,
			end_array,
			begin_object,
			end_object,
			name_separator,
			value_separator,
			end_of_input
		};
	}
}