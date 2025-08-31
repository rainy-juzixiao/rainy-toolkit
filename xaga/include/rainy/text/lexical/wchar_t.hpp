#pragma once
#include <map>
#include <string>
#include <format>
#include <stdexcept>
#include <algorithm>
#include <unordered_map>

namespace cloudgameZero {
	namespace foundation {
		namespace user {
			namespace lexical /* 通用类型转换库 */ {
				namespace wchar_type {
					namespace utils /* Lexical使用的辅助工具 */ {
						namespace constant {
							inline constexpr wchar_t OpenCurly = '{';
							inline constexpr wchar_t CloseCurly = '}';

							inline constexpr wchar_t OpenBracket = '[';
							inline constexpr wchar_t CloseBracket = ']';

							inline constexpr wchar_t OpenParen = '(';
							inline constexpr wchar_t CloseParen = ')';

							inline constexpr wchar_t OpenQuote = '"';
							inline constexpr wchar_t CloseQuote = '"';
						}

						static _NODISCARD std::string WideToMuti(std::wstring convert)
						{
							int len = WideCharToMultiByte(CP_ACP, NULL, convert.c_str(), lstrlenW(convert.c_str()), NULL, 0, NULL, NULL);
							CHAR* mutiByte = new CHAR[static_cast<int>(len + 1)];
							WideCharToMultiByte(CP_ACP, NULL, convert.c_str(), lstrlenW(convert.c_str()), mutiByte, len, NULL, NULL);
							mutiByte[len] = '\0';
							std::string ret = mutiByte;
							delete[] mutiByte;
							return ret;
						}

						struct w_lexical_cast_error {
						public:
							w_lexical_cast_error(std::wstring_view input, std::wstring_view type) : str(std::format(L"cannot parse '{}' as '{}'", input, type))
							{
							}

							w_lexical_cast_error(std::wstring_view input, std::wstring_view type,std::wstring_view msg) :
								str(std::format(L"cannot parse '{}' as '{}': {}", input, type, msg))
							{ }
						private:
							std::wstring str;
						};

						template<typename Iter>
						constexpr bool matchingDelims(Iter iter, Iter end) {
							if (iter == end)
								return false;
							wchar_t first = *iter;
							if (first == constant::OpenQuote || first == constant::OpenCurly || first == constant::OpenBracket || first == constant::OpenParen) {
								wchar_t last = *(end - 1);
								return last == constant::CloseQuote || last == constant::CloseCurly || last == constant::CloseBracket || last == constant::CloseParen;
							}
							return false;
						}

						template<typename Iter>
						constexpr std::pair<Iter, Iter> unweapWs(Iter begin, Iter end) {
							begin = std::find_if_not(begin, end, [](wchar_t c) { return std::isspace(c); });
							end = std::find_if_not(std::make_reverse_iterator(end), std::make_reverse_iterator(begin),
								[](wchar_t c) { return std::isspace(c); }).base();
							return { begin, end };
						}

						constexpr std::wstring_view unweapWs(std::wstring_view str) {
							auto [b, e] = unweapWs(str.begin(), str.end());
							return { b, e };
						}

						template<class Iter>
						constexpr std::pair<Iter, Iter> unwrap(Iter begin, Iter end) {
							auto [b, e] = unweapWs(begin, end);
							if (b < e && matchingDelims(b, e)) {
								++b;
								--e;
							}
							return { b, e };
						}

						template<std::integral T>
						constexpr T parseIntegral(std::wstring_view input, std::wstring_view name) {
							try {
								std::string _input = WideToMuti(input.data());
								T value{ 0 };
								int base{ 10 };
								const char* start = _input.data();
								const char* end = _input.data() + _input.size();
								if ((input.size() > 1) && (input[0] == '0') && ((input[1] == 'x') || input[1] == 'X')) {
									start += 2;
									base = 16;
								}
								std::from_chars(start, end, value, base);
								return value;
							}
							catch (const std::invalid_argument&) {
								throw w_lexical_cast_error(input, name);
							}
							catch (const std::out_of_range&) {
								throw w_lexical_cast_error(input, name);
							}
						}

						struct from_wchars_result {
							const wchar_t* ptr;
							std::errc ec;
							_NODISCARD_FRIEND bool operator==(const from_wchars_result&, const from_wchars_result&) = default;
						};

						from_wchars_result fromchars(const wchar_t* begin, const wchar_t* end, float& value) {
							std::size_t idx{};
							value = stof(std::wstring(begin, end - begin), &idx);
							return { begin + idx };
						}

						from_wchars_result fromchars(const wchar_t* begin, const wchar_t* end, double& value) {
							std::size_t idx{};
							value = stod(std::wstring(begin, end - begin), &idx);
							return { begin + idx };
						}

						from_wchars_result fromchars(const wchar_t* begin, const wchar_t* end, long double& value) {
							std::size_t idx{};
							value = stold(std::wstring(begin, end - begin), &idx);
							return { begin + idx };
						}

						template<class T>
						constexpr T parseFloatingPoint(std::wstring_view input, std::wstring_view name) {
							try {
								T value = 0;
								auto r = fromchars(input._Unchecked_begin(), input._Unchecked_end(), value);
								if (r.ptr != input._Unchecked_end())
									throw w_lexical_cast_error(input, name);
								return value;
							}
							catch (std::invalid_argument const&) {
								throw w_lexical_cast_error(input, name);
							}
							catch (std::out_of_range const&) {
								throw w_lexical_cast_error(input, name);
							}
						}
					}

					namespace Impl {
						template <typename T>
						struct lexicalCastimplement;

						template<>
						struct lexicalCastimplement<bool> {
							constexpr bool convert(std::wstring_view raw_input) const {
								std::wstring_view input = utils::unweapWs(raw_input);
								if (input == L"0" || input == L"f" || input == L"F" || input == L"false")
									return false;
								if (input == L"1" || input == L"t" || input == L"T" || input == L"true")
									return true;
								throw utils::w_lexical_cast_error(input, L"bool");
							}

							constexpr std::wstring to_string(bool input) const {
								return input ? L"true" : L"false";
							}
						};

						template<>
						struct lexicalCastimplement<wchar_t> {
							constexpr wchar_t convert(std::wstring_view input) const {
								if (input.size() != 1)
									throw utils::w_lexical_cast_error(input, L"wchar_t");
								return input[0];
							}

							constexpr std::wstring to_string(wchar_t input) const {
								return { input };
							}
						};

						template<std::integral T>
						struct lexicalCastimplement<T> {
							constexpr T convert(std::wstring_view input) const {
								return utils::parseIntegral<T>(utils::unweapWs(input), L"integral");
							}

							constexpr std::wstring to_string(T input) const {
								return std::to_string(input);
							}
						};

						template<std::floating_point T>
						struct lexicalCastimplement<T> {
							constexpr T convert(std::wstring_view input) const {
								return utils::parseFloatingPoint<T>(utils::unweapWs(input), "floating point");
							}

							constexpr std::wstring to_string(T input) const {
								return std::to_string(input);
							}
						};

						template<>
						struct lexicalCastimplement<const wchar_t*> {
							constexpr const wchar_t* convert(std::wstring_view s) const {
								return s.data();
							}

							constexpr const wchar_t* to_string(std::wstring& value) const {
								return value.c_str();
							}
						};

						template<>
						struct lexicalCastimplement<std::wstring> {
							constexpr std::wstring convert(std::wstring_view s) const {
								return std::wstring(s);
							}

							constexpr std::wstring to_string(const std::wstring& value) const {
								return value;
							}
						};

						template<typename Ty, std::size_t N>
						struct lexicalCastimplement<std::array<Ty, N>> {
							constexpr std::array<Ty, N> convert(std::wstring_view s) const {
								const wchar_t* iter = s.data();
								const wchar_t* end = iter + s.size();
								std::size_t idx{};
								std::array<Ty, N> arr{};
								while (iter < end) {
									if (idx >= N)
										throw utils::w_lexical_cast_error(s, L"std::array", L"too many elements");
									const wchar_t* delim = std::find(iter, end, L',');
									if (iter < delim) {
										arr[idx] = Impl::lexicalCastimplement<Ty>{}.convert({ iter, static_cast<std::size_t>(delim - iter) });
										++idx;
									}
									iter = delim + 1;
								}
								if (idx < N)
									throw utils::w_lexical_cast_error(s, L"std::array", L"too few elements");
								return arr;
							}

							constexpr std::wstring to_string(const std::array<Ty, N>& arr) const {
								std::wstring r = L"[";
								for (std::size_t i = 0; i < N; ++i) {
									if (i > 0)
										r += L",";
									r += Impl::lexicalCastimplement<Ty>{}.to_string(arr[i]);
								}
								r += L"]";
								return r;
							}
						};

						template<typename Ty>
						struct lexicalCastimplement<std::vector<Ty>> {
							constexpr std::vector<Ty> convert(std::wstring_view s) const {
								const wchar_t* iter = s.data();
								const wchar_t* end = iter + s.size();
								std::size_t idx{};
								std::vector<Ty> arr{};
								while (iter < end) {
									const wchar_t* delim = std::find(iter, end, L',');
									if (iter < delim)
										arr.push_back(Impl::lexicalCastimplement<Ty>{}.convert({ iter, static_cast<std::size_t>(delim - iter) }));
									iter = delim + 1;  // Move past the delimiter
								}
								return arr;
							}

							constexpr std::wstring to_string(const std::vector<Ty>& arr) const {
								std::wstring r = L"[";
								for (std::size_t i = 0; i < arr.size(); ++i) {
									if (i > 0)
										r += L",";
									r += Impl::lexicalCastimplement<Ty>{}.to_string(arr[i]);
								}
								r += L"]";
								return r;
							}
						};

						template<typename Ty>
						struct lexicalCastimplement<std::list<Ty>> {
							constexpr std::list<Ty> convert(std::wstring_view s) const {
								const wchar_t* iter = s.data();
								const wchar_t* end = iter + s.size();
								std::size_t idx{};
								std::list<Ty> arr{};
								while (iter < end) {
									const wchar_t* delim = std::find(iter, end, L',');
									if (iter < delim)
										arr.emplace_back(Impl::lexicalCastimplement<Ty>{}.convert({ iter, static_cast<std::size_t>(delim - iter) }));
									iter = delim + 1;  // Move past the delimiter
								}
								return arr;
							}

							constexpr std::wstring to_string(const std::list<Ty>& arr) const {
								std::wstring r = L"[";
								auto it = arr.begin();
								while (it != arr.end()) {
									if (it != arr.begin())
										r += L",";
									r += Impl::lexicalCastimplement<Ty>{}.to_string(*it);
									++it;
								}
								r += L"]";
								return r;
							}
						};


						template<typename T1, typename T2>
						struct lexicalCastimplement<std::pair<T1, T2>> {
							constexpr std::pair<T1, T2> convert(std::wstring_view s) const {
								const wchar_t* iter = s.data();
								const wchar_t* end = iter + s.size();
								const wchar_t* delim = std::find(iter, end, L':');
								if (delim == end)
									throw utils::w_lexical_cast_error(s, L"std::pair", L"too few elements");
								T1 first = Impl::lexicalCastimplement<T1>{}.convert({ iter, static_cast<std::size_t>(delim - iter) });
								++delim;  // Move past the delimiter
								T2 second = Impl::lexicalCastimplement<T2>{}.convert({ delim, static_cast<std::size_t>(end - delim) });
								return { first, second };
							}

							constexpr std::wstring to_string(const std::pair<T1, T2>& p) const {
								return Impl::lexicalCastimplement<T1>{}.to_string(p.first) + "," + Impl::lexicalCastimplement<T2>{}.to_string(p.second);
							}
						};

						template<typename Key, typename Value>
						struct lexicalCastimplement<std::map<Key, Value>> {
							constexpr std::map<Key, Value> convert(std::wstring_view s) const {
								const wchar_t* iter = s.data();
								const wchar_t* end = iter + s.size();
								std::map<Key, Value> map;
								while (iter < end) {
									const wchar_t* delim = std::find(iter, end, L',');
									if (delim == iter)
										throw utils::w_lexical_cast_error(s, L"std::map", L"invalid format");
									map.emplace(Impl::lexicalCastimplement<std::pair<Key, Value>>{}.convert({ iter, static_cast<std::size_t>(delim - iter) }));
									iter = delim + 1;
								}
								return map;
							}

							constexpr std::wstring to_string(const std::map<Key, Value>& map) const {
								std::wstring result = "{";
								auto it = map.begin();
								while (it != map.end()) {
									if (it != map.begin())
										result += L", ";
									result += Impl::lexicalCastimplement<std::pair<Key, Value>>{}.to_string(*it);
									++it;
								}
								result += L"}";
								return result;
							}
						};

						template<typename Key, typename Value>
						struct lexicalCastimplement<std::unordered_map<Key, Value>> {
							constexpr std::unordered_map<Key, Value> convert(std::wstring_view s) const {
								const wchar_t* iter = s.data();
								const wchar_t* end = iter + s.size();
								std::unordered_map<Key, Value> map;
								while (iter < end) {
									const wchar_t* delim = std::find(iter, end, L',');
									if (delim == iter)
										throw utils::w_lexical_cast_error(s, L"std::map", L"invalid format");
									map.emplace(Impl::lexicalCastimplement<std::pair<Key, Value>>{}.convert({ iter, static_cast<std::size_t>(delim - iter) }));
									iter = delim + 1;
								}
								return map;
							}

							constexpr std::wstring to_string(const std::unordered_map<Key, Value>& map) const {
								std::wstring result = L"{";
								auto it = map.begin();
								while (it != map.end()) {
									if (it != map.begin())
										result += L", ";
									result += Impl::lexicalCastimplement<std::pair<Key, Value>>{}.to_string(*it);
									++it;
								}
								result += L"}";
								return result;
							}
						};
					}
				}
			}
		}
	}
}