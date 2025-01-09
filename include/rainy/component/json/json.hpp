#ifndef RAINY_COMPONENT_JSON_MAIN
#define  RAINY_COMPONENT_JSON_MAIN
#include <map>
#include <ios>
#include <memory>
#include <cassert>
#include <vector>
#include <iomanip>
#include <stdexcept>
#include <rainy/component/json/utils.hpp>

#ifndef __winder_lib_lexical_cast_json_support
#define __winder_lib_lexical_cast_json_support 0
#endif // 启用lexical支持

#if __winder_lib_lexical_cast_json_support
//#include <rainy/foundation/user/lexical.hpp>
#endif

namespace rainy::component::json {
	using json = implements::basic_json<>;
	using json64 = implements::basic_json<std::map, std::vector, std::string, int64_t>;
	using wjson = implements::basic_json<std::map, std::vector, std::wstring>;
	using wjson64 = implements::basic_json<std::map, std::vector, std::wstring, int64_t>;

	namespace traits {
		template <typename>
		struct is_basic_json : std::false_type
		{
		};

		template <template <class _Kty, class _Ty, class... _Args> class _ObjectTy,
			template <class _Kty, class... _Args> class _ArrayTy, typename _StringTy,
			typename _IntegerTy, typename _FloatTy, typename _BooleanTy,
			template <class _Ty> class _Allocator>
		struct is_basic_json<implements::basic_json<_ObjectTy, _ArrayTy, _StringTy, _IntegerTy, _FloatTy, _BooleanTy, _Allocator>> : std::true_type
		{
		};

		template <typename _Ty, typename basicJson = implements::basic_json<>,
			typename std::enable_if<is_basic_json<basicJson>::value, int>::type = 0>
		struct json_bind;

		template <typename _Ty, typename basicJson = implements::basic_json<>,
			typename std::enable_if<is_basic_json<basicJson>::value, int>::type = 0,
			typename std::enable_if<std::is_default_constructible<json_bind<_Ty, basicJson>>::value, int>::type = 0>
		inline void to_json(basicJson& j, const _Ty& value)
		{
			json_bind<_Ty, basicJson>().to_json(j, value);
		}

		template <typename _Ty, typename basicJson = implements::basic_json<>,
			typename std::enable_if<is_basic_json<basicJson>::value, int>::type = 0,
			typename std::enable_if<std::is_default_constructible<json_bind<_Ty, basicJson>>::value, int>::type = 0>
		inline void from_json(const basicJson& j, _Ty& value)
		{
			json_bind<_Ty, basicJson>().from_json(j, value);
		}

		template <typename _Ty>
		struct json_bind<std::unique_ptr<_Ty>>
		{
			void to_json(json& j, std::unique_ptr<_Ty> const& v)
			{
				if (v != nullptr)
				{
					::jsonxx::traits::to_json(j, *v);
				}
				else
				{
					j = nullptr;
				}
			}

			void from_json(const json& j, std::unique_ptr<_Ty>& v)
			{
				if (j.is_null())
				{
					v = nullptr;
				}
				else
				{
					if (v == nullptr)
					{
						v.reset(new _Ty);
					}
					::jsonxx::traits::from_json(j, *v);
				}
			}
		};

		template <typename _Ty>
		struct json_bind<std::shared_ptr<_Ty>>
		{
			void to_json(json& j, std::shared_ptr<_Ty> const& v)
			{
				if (v != nullptr)
				{
					::jsonxx::traits::to_json(j, *v);
				}
				else
				{
					j = nullptr;
				}
			}

			void from_json(const json& j, std::shared_ptr<_Ty>& v)
			{
				if (j.is_null())
				{
					v = nullptr;
				}
				else
				{
					if (v == nullptr)
					{
						v = std::make_shared<_Ty>();
					}
					::jsonxx::traits::from_json(j, *v);
				}
			}
		};

		template <typename _Ty>
		struct json_bind<std::vector<_Ty>>
		{
			void to_json(json& j, const std::vector<_Ty>& v)
			{
				j = enums::json_type::array;
				for (size_t i = 0; i < v.size(); i++)
				{
					::jsonxx::traits::to_json(j[i], v[i]);
				}
			}

			template <typename std::enable_if<std::is_default_constructible<_Ty>::value, int>::type = 0>
			void from_json(const json& j, std::vector<_Ty>& v)
			{
				v.resize(j.size());
				for (size_t i = 0; i < j.size(); i++)
				{
					::jsonxx::traits::from_json(j[i], v[i]);
				}
			}
		};

		template <typename _Ty>
		struct json_bind<std::map<std::string, _Ty>>
		{
			void to_json(json& j, const std::map<std::string, _Ty>& v)
			{
				j = enums::json_type::object;
				for (const auto& p : v)
				{
					::jsonxx::traits::to_json(j[p.first], p.second);
				}
			}

			template <typename std::enable_if<std::is_default_constructible<_Ty>::value, int>::type = 0>
			void from_json(const json& j, std::map<std::string, _Ty>& v)
			{
				for (auto iter = j.cbegin(); iter != j.cend(); iter++)
				{
					_Ty item{};
					::jsonxx::traits::from_json(iter->value(), item);
					v.insert(std::make_pair(iter->key(), item));
				}
			}
		};

		template <>
		struct json_bind<std::string>
		{
			using value_type = std::string;

			void to_json(json& j, const value_type& v)
			{
				j = v;
			}
			void from_json(const json& j, value_type& v)
			{
				v = static_cast<value_type>(j);
			}
		};

		template <>
		struct json_bind<int>
		{
			using value_type = int;

			void to_json(json& j, const value_type& v)
			{
				j = v;
			}
			void from_json(const json& j, value_type& v)
			{
				v = static_cast<value_type>(j);
			}
		};

		template <>
		struct json_bind<unsigned int>
		{
			using value_type = unsigned int;

			void to_json(json& j, const value_type& v)
			{
				j = v;
			}
			void from_json(const json& j, value_type& v)
			{
				v = static_cast<value_type>(j);
			}
		};

		template <>
		struct json_bind<short>
		{
			using value_type = short;

			void to_json(json& j, const value_type& v)
			{
				j = v;
			}
			void from_json(const json& j, value_type& v)
			{
				v = static_cast<value_type>(j);
			}
		};

		template <>
		struct json_bind<unsigned short>
		{
			using value_type = unsigned short;

			void to_json(json& j, const value_type& v)
			{
				j = v;
			}
			void from_json(const json& j, value_type& v)
			{
				v = static_cast<value_type>(j);
			}
		};

		template <>
		struct json_bind<long>
		{
			using value_type = long;

			void to_json(json& j, const value_type& v)
			{
				j = v;
			}
			void from_json(const json& j, value_type& v)
			{
				v = static_cast<value_type>(j);
			}
		};

		template <>
		struct json_bind<unsigned long>
		{
			using value_type = unsigned long;

			void to_json(json& j, const value_type& v)
			{
				j = v;
			}
			void from_json(const json& j, value_type& v)
			{
				v = static_cast<value_type>(j);
			}
		};

		template <>
		struct json_bind<float>
		{
			using value_type = float;

			void to_json(json& j, const value_type& v)
			{
				j = v;
			}
			void from_json(const json& j, value_type& v)
			{
				v = static_cast<value_type>(j);
			}
		};

		template <>
		struct json_bind<double>
		{
			using value_type = double;

			void to_json(json& j, const value_type& v)
			{
				j = v;
			}
			void from_json(const json& j, value_type& v)
			{
				v = static_cast<value_type>(j);
			}
		};

		template <>
		struct json_bind<bool>
		{
			using value_type = bool;

			void to_json(json& j, const value_type& v)
			{
				j = v;
			}
			void from_json(const json& j, value_type& v)
			{
				v = static_cast<value_type>(j);
			}
		};

		template <>
		struct json_bind<json>
		{
			using value_type = json;

			void to_json(json& j, const value_type& v)
			{
				j = v;
			}
			void from_json(const json& j, value_type& v)
			{
				v = j;
			}
		};

		template <typename _Ty, typename basicJson = implements::basic_json<>>
		class readJsonWrapper
		{
		public:
			using char_type = typename basicJson::char_type;

			readJsonWrapper(const _Ty& v)
				: v_(v)
			{
			}

			friend std::basic_ostream<char_type>& operator<<(std::basic_ostream<char_type>& out,
				const readJsonWrapper& wrapper)
			{
				basicJson j{};
				::jsonxx::traits::to_json(j, wrapper.v_);
				out << j;
				return out;
			}

		private:
			const _Ty& v_;
		};

		template <typename _Ty, typename basicJson = implements::basic_json<>>
		class writeJsonWrapper : public readJsonWrapper<_Ty, basicJson>
		{
		public:
			using char_type = typename basicJson::char_type;

			writeJsonWrapper(_Ty& v)
				: readJsonWrapper<_Ty, basicJson>(v)
				, v_(v)
			{
			}

			friend std::basic_istream<char_type>& operator>>(std::basic_istream<char_type>& in,
				const writeJsonWrapper& wrapper)
			{
				basicJson j{};
				in >> j;
				::jsonxx::traits::from_json(j, const_cast<_Ty&>(wrapper.v_));
				return in;
			}

		private:
			_Ty& v_;
		};

		template <typename _Ty, typename basicJson = implements::basic_json<>,
			typename std::enable_if<traits::is_basic_json<basicJson>::value, int>::type = 0,
			typename std::enable_if<std::is_default_constructible<traits::json_bind<_Ty, basicJson>>::value, int>::type = 0>
		inline writeJsonWrapper<_Ty, basicJson> json_wrap(_Ty& v)
		{
			return writeJsonWrapper<_Ty, basicJson>(v);
		}

		template <typename _Ty, typename basicJson = implements::basic_json<>,
			typename std::enable_if<traits::is_basic_json<basicJson>::value, int>::type = 0,
			typename std::enable_if<std::is_default_constructible<traits::json_bind<_Ty, basicJson>>::value, int>::type = 0>
		inline readJsonWrapper<_Ty, basicJson> json_wrap(const _Ty& v)
		{
			return readJsonWrapper<_Ty, basicJson>(v);
		}
	}
}

namespace std {
	template <>
	struct formatter<rainy::component::json::json> {
	public:
		using json = rainy::component::json::json;

		explicit formatter() noexcept
		{ }

		auto parse(format_parse_context& ctx) {
			if (ctx.begin() == ctx.end() || *ctx.begin() == '}') {
				need_indent = true;
				return ctx.end();
			}
			switch (*ctx.begin()) {
			case 'r':
				need_indent = false;
				break;
			case 'f':
				need_indent = true;
				break;
			case 't':
				use_tab = true;
				break;
			default:
				throw std::format_error("Invalid format specification");
			}
			return ctx.begin() + 1;
		}

		template <typename OutputIt>
		auto format(const json& value, std::basic_format_context<OutputIt, char> fc) const noexcept
		{
			if(use_tab)
				return std::format_to(fc.out(), "{}", value.dump('\t',1));
			return std::format_to(fc.out(), "{}", need_indent ? value.dump(4, ' ') : value.dump());
		}

	private:
		bool need_indent{};
		bool use_tab{};
	};

	template <>
	struct formatter<rainy::component::json::wjson> {
	public:
		using wjson = rainy::component::json::wjson;

		explicit formatter() noexcept
		{ }

		auto parse(format_parse_context& ctx) {
			if (ctx.begin() == ctx.end() || *ctx.begin() == L'}') {
				need_indent = true;
				return ctx.end();
			}
			switch (*ctx.begin()) {
			case L'r':
				need_indent = false;
				break;
			case L'f':
				need_indent = true;
				break;
			case L't':
				use_tab = true;
				break;
			default:
				throw std::format_error("Invalid format specification");
			}
			return ctx.begin() + 1;
		}

		template <typename OutputIt>
		auto format(const wjson& value, std::basic_format_context<OutputIt, wchar_t> fc) const noexcept
		{
			if (use_tab)
				return std::format_to(fc.out(), L"{}", value.dump(L'\t', 1));
			return std::format_to(fc.out(), L"{}", need_indent ? value.dump(4, ' ') : value.dump());
		}

	private:
		bool need_indent{};
		bool use_tab{};
	};
}

#if __winder_lib_lexical_cast_json_support
namespace rainy {
	namespace foundation {
		namespace user {
			namespace lexical /* ͨ������ת���� */ {
				namespace char_type {
					namespace Impl {
						template<>
						struct lexicalCastimplement<rainy::component::json::implements::basic_json<std::map, std::vector, std::string, int32_t>> {
							using json = rainy::component::json::implements::basic_json<std::map, std::vector, std::string, int32_t>;

							json convert(std::string_view s) const {
								if (s.empty()) {
									return {}; // ���sΪ�յ��������Ϊ�ַ�������Ϊjsonʱ�������ܿ��ַ�������ˣ��˴���������
								}
								try {
									json ret = json::parse(s.data());
									return std::move(ret); // ����Դ�����ƶ��������ظ�����
								}
								catch (rainy::foundation::system::exception::exception& except) {
									std::cerr << except.what() << "\n";
									return{}; // �ڳ����쳣��ʱ�򣬴�ӡ������Ϣ�����ؿ�
								}
							}

							std::string to_string(const json& _json) const {
								if (_json.empty()) {
									return { "null" };
								}
								return _json.dump();
							}
						};

						template<>
						struct lexicalCastimplement<rainy::component::json::implements::basic_json<std::map, std::vector, std::string, int64_t>> {
							using json = rainy::component::json::implements::basic_json<std::map, std::vector, std::string, int32_t>;

							json convert(std::string_view s) const {
								if (s.empty()) {
									return {}; // ���sΪ�յ��������Ϊ�ַ�������Ϊjsonʱ�������ܿ��ַ�������ˣ��˴���������
								}
								try {
									json ret = json::parse(s.data());
									return std::move(ret); // ����Դ�����ƶ��������ظ�����
								}
								catch (rainy::foundation::system::exception::exception& except) {
									std::cerr << except.what() << "\n";
									return{}; // �ڳ����쳣��ʱ�򣬴�ӡ������Ϣ�����ؿ�
								}
							}

							std::string to_string(const json& _json) const {
								if (_json.empty()) {
									return { "null" };
								}
								return _json.dump();
							}
						};
					}
				}

				namespace wchar_type {
					namespace Impl {
						template<>
						struct lexicalCastimplement<rainy::component::json::implements::basic_json<std::map, std::vector, std::wstring, int32_t>> {
							using json = rainy::component::json::implements::basic_json<std::map, std::vector, std::wstring, int32_t>;

							json convert(std::wstring_view s) const {
								if (s.empty()) {
									return {}; // ���sΪ�յ��������Ϊ�ַ�������Ϊjsonʱ�������ܿ��ַ�������ˣ��˴���������
								}
								try {
									json ret = json::parse(s.data());
									return std::move(ret); // ����Դ�����ƶ��������ظ�����
								}
								catch (rainy::foundation::system::exception::exception& except) {
									std::cerr << except.what() << "\n";
									return{}; // �ڳ����쳣��ʱ�򣬴�ӡ������Ϣ�����ؿ�
								}
							}

							std::wstring to_string(const json& _json) const {
								if (_json.empty()) {
									return { L"null" };
								}
								return _json.dump();
							}
						};

						template<>
						struct lexicalCastimplement<rainy::component::json::implements::basic_json<std::map, std::vector, std::wstring, int64_t>> {
							using json = rainy::component::json::implements::basic_json<std::map, std::vector, std::wstring, int32_t>;

							json convert(std::wstring_view s) const {
								if (s.empty()) {
									return {}; // ���sΪ�յ��������Ϊ�ַ�������Ϊjsonʱ�������ܿ��ַ�������ˣ��˴���������
								}
								try {
									json ret = json::parse(s.data());
									return std::move(ret); // ����Դ�����ƶ��������ظ�����
								}
								catch (rainy::foundation::system::exception::exception& except) {
									std::cerr << except.what() << "\n";
									return{}; // �ڳ����쳣��ʱ�򣬴�ӡ������Ϣ�����ؿ�
								}
							}

							std::wstring to_string(const json& _json) const {
								if (_json.empty()) {
									return { L"null" };
								}
								return _json.dump();
							}
						};
					}
				}
			}
		}
	}
}
#endif
#endif