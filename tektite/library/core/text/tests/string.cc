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

#include <catch2/catch_template_test_macros.hpp>
#include <catch2/catch_test_macros.hpp>
#include <rainy/core/text/string.hpp>
#include <rainy/core/text/string_view.hpp>

using namespace rainy;
using namespace rainy::core::text;

constexpr bool test_default_constructor() {
    basic_string<char> s;
    return s.empty() && s.size() == 0 && s.capacity() == basic_string<char>::default_capacity;
}

constexpr bool test_allocator_constructor() {
    core::memory::allocator<char> alloc;
    basic_string<char> s(alloc);
    return s.empty() && s.size() == 0 && s.get_allocator() == alloc;
}

constexpr bool test_count_char_constructor() {
    basic_string<char> s(5, 'a');
    return s.size() == 5 && s[0] == 'a' && s[4] == 'a';
}

constexpr bool test_substr_constructor() {
    basic_string<char> s("hello world");
    basic_string<char> sub(s, 0, 5);
    return sub.size() == 5 && sub[0] == 'h' && sub[4] == 'o';
}

constexpr bool test_ptr_count_constructor() {
    basic_string<char> s("hello", 3);
    return s.size() == 3 && s[0] == 'h' && s[2] == 'l';
}

constexpr bool test_ptr_constructor() {
    basic_string<char> s("hello");
    return s.size() == 5 && s[4] == 'o';
}

constexpr bool test_copy_constructor() {
    basic_string<char> s("hello");
    basic_string<char> copy(s);
    return copy.size() == 5 && copy == s;
}

constexpr bool test_move_constructor() {
    basic_string<char> s("hello");
    basic_string<char> moved(static_cast<basic_string<char> &&>(s));
    return moved.size() == 5 && moved[0] == 'h';
}

constexpr bool test_initializer_list_constructor() {
    basic_string<char> s({'a', 'b', 'c'});
    return s.size() == 3 && s[0] == 'a' && s[2] == 'c';
}

// 极端测试：大量字符的构造函数
constexpr bool test_large_count_char_constructor() {
    basic_string<char> s(1000, 'x');
    return s.size() == 1000 && s[0] == 'x' && s[999] == 'x' && s[500] == 'x';
}

constexpr bool test_empty() {
    basic_string<char> s;
    return s.empty();
}

constexpr bool test_size() {
    basic_string<char> s("abc");
    return s.size() == 3;
}

constexpr bool test_length() {
    basic_string<char> s("hello");
    return s.length() == 5;
}

constexpr bool test_max_size() {
    auto ms = basic_string<char>::max_size();
    return ms > 0;
}

constexpr bool test_capacity() {
    basic_string<char> s;
    return s.capacity() == basic_string<char>::default_capacity;
}

constexpr bool test_data() {
    basic_string<char> s("abc");
    return s.data()[0] == 'a';
}

constexpr bool test_c_str() {
    basic_string<char> s("abc");
    return s.c_str()[2] == 'c';
}

constexpr bool test_at() {
    basic_string<char> s("abc");
    return s.at(0) == 'a' && s.at(2) == 'c';
}

constexpr bool test_operator_subscript() {
    basic_string<char> s("abc");
    return s[0] == 'a' && s[2] == 'c';
}

constexpr bool test_front() {
    basic_string<char> s("abc");
    return s.front() == 'a';
}

constexpr bool test_back() {
    basic_string<char> s("abc");
    return s.back() == 'c';
}

constexpr bool test_begin_end() {
    basic_string<char> s("abc");
    return *s.begin() == 'a' && *(s.end() - 1) == 'c';
}

constexpr bool test_cbegin_cend() {
    basic_string<char> s("abc");
    return *s.cbegin() == 'a' && *(s.cend() - 1) == 'c';
}

constexpr bool test_rbegin_rend() {
    basic_string<char> s("abc");
    return *s.rbegin() == 'c' && *(s.rend() - 1) == 'a';
}

constexpr bool test_crbegin_crend() {
    basic_string<char> s("abc");
    return *s.crbegin() == 'c' && *(s.crend() - 1) == 'a';
}

// 极端测试：迭代器遍历长字符串
constexpr bool test_iterator_large_string() {
    basic_string<char> s(500, 'a');
    int count = 0;
    for (auto it = s.begin(); it != s.end(); ++it) {
        if (*it != 'a') return false;
        ++count;
    }
    return count == 500;
}

constexpr bool test_shrink_to_fit() {
    basic_string<char> s("hello world");
    s.reserve(100);
    auto cap_before = s.capacity();
    s.shrink_to_fit();
    auto cap_after = s.capacity();
    return cap_after <= cap_before && s == "hello world";
}

// 极端测试：shrink_to_fit 在临界点
constexpr bool test_shrink_to_fit_at_boundary() {
    basic_string<char> s;
    // 刚好填满默认容量
    for (int i = 0; i < basic_string<char>::default_capacity; ++i) {
        s.push_back('a');
    }
    auto cap_before = s.capacity();
    s.shrink_to_fit();
    auto cap_after = s.capacity();
    return cap_after <= cap_before && s.size() == basic_string<char>::default_capacity;
}

constexpr bool test_reserve() {
    basic_string<char> s("abc");
    s.reserve(50);
    return s.capacity() >= 50 && s == "abc";
}

constexpr bool test_reserve_exact_capacity() {
    basic_string<char> s("abc");
    auto old_cap = s.capacity();
    // 请求刚好等于当前容量的reserve
    s.reserve(old_cap);
    return s.capacity() >= old_cap && s == "abc";
}

constexpr bool test_reserve_large() {
    basic_string<char> s("abc");
    s.reserve(10000);
    return s.capacity() >= 10000 && s == "abc";
}

// 极端测试：reserve 从0开始
constexpr bool test_reserve_from_zero() {
    basic_string<char> s;
    s.reserve(50);
    return s.capacity() >= 50 && s.empty();
}

constexpr bool test_resize_default() {
    basic_string<char> s("abc");
    s.resize(6);
    return s.size() == 6 && s[0] == 'a' && s[3] == '\0';
}

// 极端测试：resize 跨扩容边界
constexpr bool test_resize_across_capacity_boundary() {
    basic_string<char> s("abc");
    auto old_cap = s.capacity();
    // resize到刚好超过当前容量的位置
    s.resize(old_cap + 1, 'x');
    return s.size() == old_cap + 1 && s.capacity() > old_cap && s[old_cap] == 'x';
}

// 极端测试：resize 到很大
constexpr bool test_resize_large() {
    basic_string<char> s("abc");
    s.resize(2000, 'z');
    return s.size() == 2000 && s[0] == 'a' && s[1] == 'b' && s[2] == 'c'
           && s[3] == 'z' && s[1999] == 'z';
}

// 极端测试：resize 缩小并再扩大
constexpr bool test_resize_shrink_then_grow() {
    basic_string<char> s("hello world");
    s.resize(3);
    s.resize(10, '!');
    return s.size() == 10 && s[0] == 'h' && s[2] == 'l' && s[3] == '!';
}

constexpr bool test_resize_with_char() {
    basic_string<char> s("abc");
    s.resize(6, 'x');
    return s.size() == 6 && s[0] == 'a' && s[3] == 'x' && s[5] == 'x';
}

constexpr bool test_clear() {
    basic_string<char> s("abc");
    s.clear();
    return s.empty() && s.size() == 0;
}

// 极端测试：clear 后重新使用
constexpr bool test_clear_and_reuse() {
    basic_string<char> s("hello");
    s.clear();
    s.append("world");
    return s == "world" && s.size() == 5;
}

constexpr bool test_push_back() {
    basic_string<char> s("ab");
    s.push_back('c');
    return s.size() == 3 && s[2] == 'c';
}

// 极端测试：push_back 触发多次扩容
constexpr bool test_push_back_many() {
    basic_string<char> s;
    for (int i = 0; i < 1000; ++i) {
        s.push_back(static_cast<char>('a' + (i % 26)));
    }
    return s.size() == 1000 && s[0] == 'a' && s[999] == static_cast<char>('a' + (999 % 26));
}

// 极端测试：push_back 在临界扩容点
constexpr bool test_push_back_at_capacity_boundary() {
    basic_string<char> s;
    // 填充到刚好满容量
    for (int i = 0; i < basic_string<char>::default_capacity; ++i) {
        s.push_back('a');
    }
    auto old_cap = s.capacity();
    // 再push一次触发扩容
    s.push_back('b');
    return s.size() == basic_string<char>::default_capacity + 1
           && s.capacity() > old_cap
           && s[s.size() - 1] == 'b';
}

constexpr bool test_swap() {
    basic_string<char> s1("abc");
    basic_string<char> s2("xyz");
    s1.swap(s2);
    return s1 == "xyz" && s2 == "abc";
}

// 极端测试：swap 不同容量的字符串
constexpr bool test_swap_different_capacities() {
    basic_string<char> s1("a");
    basic_string<char> s2;
    s2.reserve(100);
    s2.append("large string with more content");
    auto s1_old_size = s1.size();
    auto s2_old_size = s2.size();
    auto s1_old_cap = s1.capacity();
    auto s2_old_cap = s2.capacity();
    s1.swap(s2);
    return s1.size() == s2_old_size && s2.size() == s1_old_size
           && s1.capacity() == s2_old_cap && s2.capacity() == s1_old_cap;
}

// 极端测试：swap 自交换
constexpr bool test_swap_self() {
    basic_string<char> s("hello");
    s.swap(s);
    return s == "hello" && s.size() == 5;
}

constexpr bool test_assign_count_char() {
    basic_string<char> s;
    s.assign(3, 'a');
    return s == "aaa";
}

// 极端测试：assign 大数量字符
constexpr bool test_assign_large_count() {
    basic_string<char> s("hello");
    s.assign(5000, 'z');
    return s.size() == 5000 && s[0] == 'z' && s[4999] == 'z';
}

constexpr bool test_assign_string() {
    basic_string<char> s("abc");
    basic_string<char> t("xyz");
    s.assign(t);
    return s == "xyz";
}

constexpr bool test_assign_substring() {
    basic_string<char> s;
    basic_string<char> t("hello");
    s.assign(t, 0, 3);
    return s == "hel";
}

constexpr bool test_assign_ptr_count() {
    basic_string<char> s;
    s.assign("hello", 3);
    return s == "hel";
}

constexpr bool test_assign_ptr() {
    basic_string<char> s;
    s.assign("hello");
    return s == "hello";
}

constexpr bool test_assign_initializer_list() {
    basic_string<char> s;
    s.assign({'a', 'b', 'c'});
    return s == "abc";
}

// 极端测试：assign 自赋值
constexpr bool test_assign_self() {
    basic_string<char> s("hello");
    s.assign(s);
    return s == "hello";
}

constexpr bool test_append_count_char() {
    basic_string<char> s("ab");
    s.append(2, 'c');
    return s == "abcc";
}

// 极端测试：append 触发扩容
constexpr bool test_append_trigger_reallocation() {
    basic_string<char> s("abc");
    auto old_cap = s.capacity();
    // 追加足够多的字符以触发扩容
    s.append(1000, 'x');
    return s.size() == 1003 && s.capacity() > old_cap && s[0] == 'a' && s[1002] == 'x';
}

// 极端测试：多次append在临界点
constexpr bool test_append_at_boundary() {
    basic_string<char> s;
    for (int i = 0; i < basic_string<char>::default_capacity - 2; ++i) {
        s.push_back('a');
    }
    auto old_cap = s.capacity();
    s.append("bcde");  // 触发扩容
    return s.size() == basic_string<char>::default_capacity + 2
           && s.capacity() > old_cap;
}

constexpr bool test_append_string() {
    basic_string<char> s("ab");
    basic_string<char> t("cd");
    s.append(t);
    return s == "abcd";
}

constexpr bool test_append_substring() {
    basic_string<char> s("ab");
    basic_string<char> t("cdef");
    s.append(t, 0, 2);
    return s == "abcd";
}

constexpr bool test_append_ptr_count() {
    basic_string<char> s("ab");
    s.append("cd", 2);
    return s == "abcd";
}

constexpr bool test_append_ptr() {
    basic_string<char> s("ab");
    s.append("cd");
    return s == "abcd";
}

constexpr bool test_operator_plus_equal_string() {
    basic_string<char> s("ab");
    basic_string<char> t("cd");
    s += t;
    return s == "abcd";
}

// 极端测试：operator+= 长字符串
constexpr bool test_operator_plus_equal_long_string() {
    basic_string<char> s("start:");
    basic_string<char> t(1000, 'x');
    s += t;
    return s.size() == 1006 && s.starts_with("start:") && s[1005] == 'x';
}

constexpr bool test_operator_plus_equal_char() {
    basic_string<char> s("ab");
    s += 'c';
    return s == "abc";
}

// 极端测试：operator+= char 在临界点
constexpr bool test_operator_plus_equal_char_at_boundary() {
    basic_string<char> s;
    for (int i = 0; i < basic_string<char>::default_capacity; ++i) {
        s.push_back('a');
    }
    auto old_cap = s.capacity();
    s += 'b';
    return s.size() == basic_string<char>::default_capacity + 1
           && s.capacity() > old_cap;
}

constexpr bool test_operator_plus_equal_ptr() {
    basic_string<char> s("ab");
    s += "cd";
    return s == "abcd";
}

constexpr bool test_find_sv() {
    basic_string<char> s("hello world");
    auto pos = s.find(basic_string_view<char>("world"));
    return pos == 6;
}

// 极端测试：find 不存在的子串
constexpr bool test_find_not_found() {
    basic_string<char> s("hello world");
    auto pos = s.find(basic_string_view<char>("xyz"));
    return pos == basic_string<char>::npos;
}

// 极端测试：find 在长字符串中
constexpr bool test_find_in_large_string() {
    basic_string<char> s(1000, 'a');
    // 在末尾添加标记
    basic_string<char> t(s);
    t[998] = 'b';
    t[999] = 'c';
    auto pos = t.find(basic_string_view<char>("bc"));
    return pos == 998;
}

constexpr bool test_find_char() {
    basic_string<char> s("hello");
    auto pos = s.find('l');
    return pos == 2;
}

// 极端测试：find 不存在的字符
constexpr bool test_find_char_not_found() {
    basic_string<char> s("hello");
    auto pos = s.find('z');
    return pos == basic_string<char>::npos;
}

// 极端测试：find 空字符串
constexpr bool test_find_empty_string() {
    basic_string<char> s("hello");
    auto pos = s.find(basic_string_view<char>(""));
    return pos == 0;
}

constexpr bool test_find_ptr_count() {
    basic_string<char> s("hello world");
    auto pos = s.find("world", 0, 3);
    return pos == 6;
}

constexpr bool test_find_ptr() {
    basic_string<char> s("hello world");
    auto pos = s.find("world");
    return pos == 6;
}

constexpr bool test_rfind_sv() {
    basic_string<char> s("hello hello");
    auto pos = s.rfind(basic_string_view<char>("hello"));
    return pos == 6;
}

// 极端测试：rfind 重复模式
constexpr bool test_rfind_repeated_pattern() {
    basic_string<char> s("ababababab");
    auto pos = s.rfind(basic_string_view<char>("ab"));
    return pos == 8;
}

constexpr bool test_rfind_char() {
    basic_string<char> s("hello");
    auto pos = s.rfind('l');
    return pos == 3;
}

// 极端测试：rfind 首字符
constexpr bool test_rfind_first_char() {
    basic_string<char> s("abcde");
    auto pos = s.rfind('a');
    return pos == 0;
}

constexpr bool test_find_first_of_sv() {
    basic_string<char> s("hello");
    auto pos = s.find_first_of(basic_string_view<char>("eo"));
    return pos == 1;
}

// 极端测试：find_first_of 无匹配
constexpr bool test_find_first_of_no_match() {
    basic_string<char> s("abc");
    auto pos = s.find_first_of(basic_string_view<char>("xyz"));
    return pos == basic_string<char>::npos;
}

constexpr bool test_find_first_of_char() {
    basic_string<char> s("hello");
    auto pos = s.find_first_of('l');
    return pos == 2;
}

constexpr bool test_find_last_of_sv() {
    basic_string<char> s("hello");
    auto pos = s.find_last_of(basic_string_view<char>("eo"));
    return pos == 4;
}

constexpr bool test_find_last_of_char() {
    basic_string<char> s("hello");
    auto pos = s.find_last_of('l');
    return pos == 3;
}

constexpr bool test_find_first_not_of_sv() {
    basic_string<char> s("hello");
    auto pos = s.find_first_not_of(basic_string_view<char>("hel"));
    return pos == 4;
}

// 极端测试：find_first_not_of 全部匹配
constexpr bool test_find_first_not_of_all_match() {
    basic_string<char> s("aaa");
    auto pos = s.find_first_not_of('a');
    return pos == basic_string<char>::npos;
}

constexpr bool test_find_first_not_of_char() {
    basic_string<char> s("aaab");
    auto pos = s.find_first_not_of('a');
    return pos == 3;
}

constexpr bool test_find_last_not_of_sv() {
    basic_string<char> s("hello");
    auto pos = s.find_last_not_of(basic_string_view<char>("lo"));
    return pos == 1;
}

constexpr bool test_find_last_not_of_char() {
    basic_string<char> s("baaa");
    auto pos = s.find_last_not_of('a');
    return pos == 0;
}

constexpr bool test_starts_with_sv() {
    basic_string<char> s("hello world");
    return s.starts_with(basic_string_view<char>("hello"));
}

// 极端测试：starts_with 空字符串
constexpr bool test_starts_with_empty() {
    basic_string<char> s("hello");
    return s.starts_with(basic_string_view<char>(""));
}

// 极端测试：starts_with 长度相等
constexpr bool test_starts_with_exact_match() {
    basic_string<char> s("hello");
    return s.starts_with(basic_string_view<char>("hello"));
}

// 极端测试：starts_with 长度超过
constexpr bool test_starts_with_longer_than_string() {
    basic_string<char> s("hi");
    return !s.starts_with(basic_string_view<char>("hello"));
}

constexpr bool test_starts_with_char() {
    basic_string<char> s("hello");
    return s.starts_with('h');
}

// 极端测试：starts_with 不匹配的字符
constexpr bool test_starts_with_wrong_char() {
    basic_string<char> s("hello");
    return !s.starts_with('x');
}

constexpr bool test_starts_with_ptr() {
    basic_string<char> s("hello world");
    return s.starts_with("hello");
}

constexpr bool test_ends_with_sv() {
    basic_string<char> s("hello world");
    return s.ends_with(basic_string_view<char>("world"));
}

// 极端测试：ends_with 空字符串
constexpr bool test_ends_with_empty() {
    basic_string<char> s("hello");
    return s.ends_with(basic_string_view<char>(""));
}

constexpr bool test_ends_with_char() {
    basic_string<char> s("hello");
    return s.ends_with('o');
}

// 极端测试：ends_with 空字符串的字符
constexpr bool test_ends_with_char_on_empty() {
    basic_string<char> s;
    return !s.ends_with('a');
}

constexpr bool test_ends_with_ptr() {
    basic_string<char> s("hello world");
    return s.ends_with("world");
}

constexpr bool test_contains_sv() {
    basic_string<char> s("hello world");
    return s.contains(basic_string_view<char>("llo"));
}

// 极端测试：contains 空字符串
constexpr bool test_contains_empty() {
    basic_string<char> s("hello");
    return s.contains(basic_string_view<char>(""));
}

// 极端测试：contains 不存在的子串
constexpr bool test_contains_not_found() {
    basic_string<char> s("hello");
    return !s.contains(basic_string_view<char>("xyz"));
}

constexpr bool test_contains_char() {
    basic_string<char> s("hello");
    return s.contains('e');
}

constexpr bool test_contains_ptr() {
    basic_string<char> s("hello world");
    return s.contains("world");
}

constexpr bool test_insert_count_char() {
    basic_string<char> s("abc");
    s.insert(1, 2, 'x');
    return s == "axxbc";
}

// 极端测试：insert 在临界位置
constexpr bool test_insert_at_boundary() {
    basic_string<char> s;
    for (int i = 0; i < basic_string<char>::default_capacity - 2; ++i) {
        s.push_back('a');
    }
    s.insert(s.size(), "bc");  // 在末尾插入，触发扩容
    return s.size() == basic_string<char>::default_capacity
           && s[s.size() - 1] == 'c';
}

// 极端测试：insert 大字符串
constexpr bool test_insert_large_string() {
    basic_string<char> s("start");
    basic_string<char> t(500, 'x');
    s.insert(0, t);
    return s.size() == 505 && s[0] == 'x' && s[504] == 't';
}

constexpr bool test_insert_ptr_count() {
    basic_string<char> s("abc");
    s.insert(1, "xy", 2);
    return s == "axybc";
}

constexpr bool test_insert_ptr() {
    basic_string<char> s("abc");
    s.insert(1, "xy");
    return s == "axybc";
}

constexpr bool test_insert_string() {
    basic_string<char> s("abc");
    basic_string<char> t("xy");
    s.insert(1, t);
    return s == "axybc";
}

constexpr bool test_erase_index_count() {
    basic_string<char> s("hello");
    s.erase(2, 2);
    return s == "heo";
}

// 极端测试：erase 所有字符
constexpr bool test_erase_all() {
    basic_string<char> s("hello");
    s.erase(0, 5);
    return s.empty();
}

// 极端测试：erase 超出长度
constexpr bool test_erase_beyond_length() {
    basic_string<char> s("hello");
    s.erase(3, 100);
    return s == "hel";
}

constexpr bool test_erase_iterator() {
    basic_string<char> s("hello");
    s.erase(s.begin() + 2);
    return s == "helo";
}

// 极端测试：erase 首尾迭代器
constexpr bool test_erase_first_and_last() {
    basic_string<char> s("abcde");
    s.erase(s.begin());
    s.erase(s.end() - 1);
    return s == "bcd";
}

constexpr bool test_pop_back() {
    basic_string<char> s("abc");
    s.pop_back();
    return s == "ab";
}

// 极端测试：pop_back 直到空
constexpr bool test_pop_back_until_empty() {
    basic_string<char> s("abc");
    s.pop_back();
    s.pop_back();
    s.pop_back();
    return s.empty();
}

constexpr bool test_replace_pos_count_string() {
    basic_string<char> s("hello world");
    basic_string<char> t("there");
    s.replace(0, 5, t);
    return s == "there world";
}

// 极端测试：replace 导致扩容
constexpr bool test_replace_with_larger_string() {
    basic_string<char> s("hi");
    s.replace(0, 2, "hello world, this is a much longer string");
    return s.starts_with("hello world");
}

// 极端测试：replace 完全相同的内容
constexpr bool test_replace_same_content() {
    basic_string<char> s("hello");
    s.replace(0, 5, "hello");
    return s == "hello";
}

constexpr bool test_replace_pos_count_ptr_count() {
    basic_string<char> s("hello world");
    s.replace(0, 5, "there", 5);
    return s == "there world";
}

constexpr bool test_replace_pos_count_ptr() {
    basic_string<char> s("hello world");
    s.replace(0, 5, "there");
    return s == "there world";
}

constexpr bool test_compare_equal() {
    basic_string<char> s("abc");
    return s.compare("abc") == 0;
}

// 极端测试：compare 与空字符串
constexpr bool test_compare_with_empty() {
    basic_string<char> s("abc");
    return s.compare(basic_string<char>()) > 0;
}

// 极端测试：compare 长字符串
constexpr bool test_compare_long_strings() {
    basic_string<char> s1(1000, 'a');
    basic_string<char> s2(1000, 'a');
    s2[999] = 'b';
    return s1.compare(s2) < 0;
}

constexpr bool test_compare_less() {
    basic_string<char> s("aaa");
    return s.compare("aab") < 0;
}

constexpr bool test_compare_greater() {
    basic_string<char> s("aac");
    return s.compare("aab") > 0;
}

constexpr bool test_substr() {
    basic_string<char> s("hello world");
    auto sub = s.substr(0, 5);
    return sub == "hello";
}

// 极端测试：substr 整个字符串
constexpr bool test_substr_entire_string() {
    basic_string<char> s("hello");
    auto sub = s.substr(0, 5);
    return sub == s;
}

// 极端测试：substr 空子串
constexpr bool test_substr_zero_length() {
    basic_string<char> s("hello");
    auto sub = s.substr(2, 0);
    return sub.empty();
}

constexpr bool test_operator_equals() {
    basic_string<char> s1("abc");
    basic_string<char> s2("abc");
    return s1 == s2;
}

// 极端测试：operator== 与空字符串
constexpr bool test_operator_equals_with_empty() {
    basic_string<char> s1;
    basic_string<char> s2;
    return s1 == s2;
}

constexpr bool test_operator_not_equals() {
    basic_string<char> s1("abc");
    basic_string<char> s2("abd");
    return s1 != s2;
}

// 极端测试：operator!= 不同长度
constexpr bool test_operator_not_equals_different_lengths() {
    basic_string<char> s1("abc");
    basic_string<char> s2("abcd");
    return s1 != s2;
}

constexpr bool test_operator_less() {
    basic_string<char> s1("aaa");
    basic_string<char> s2("aab");
    return s1 < s2;
}

constexpr bool test_operator_greater() {
    basic_string<char> s1("aac");
    basic_string<char> s2("aab");
    return s1 > s2;
}

constexpr bool test_operator_less_equal() {
    basic_string<char> s1("aaa");
    basic_string<char> s2("aab");
    return s1 <= s2;
}

constexpr bool test_operator_greater_equal() {
    basic_string<char> s1("aac");
    basic_string<char> s2("aab");
    return s1 >= s2;
}

constexpr bool test_operator_plus() {
    basic_string<char> s1("abc");
    basic_string<char> s2("def");
    auto result = s1 + s2;
    return result == "abcdef";
}

// 极端测试：operator+ 多个字符串
constexpr bool test_operator_plus_multiple() {
    basic_string<char> s1("a");
    basic_string<char> s2("b");
    basic_string<char> s3("c");
    auto result = s1 + s2 + s3;
    return result == "abc";
}

constexpr bool test_repeat() {
    basic_string<char> s("abc");
    s.repeat(3);
    return s == "abcabcabc";
}

// 极端测试：repeat 0次
constexpr bool test_repeat_zero() {
    basic_string<char> s("abc");
    s.repeat(0);
    return s.empty();
}

// 极端测试：repeat 1次（不变）
constexpr bool test_repeat_once() {
    basic_string<char> s("abc");
    s.repeat(1);
    return s == "abc";
}

// 极端测试：repeat 多次导致扩容
constexpr bool test_repeat_many_times() {
    basic_string<char> s("abc");
    s.repeat(100);
    return s.size() == 300 && s.starts_with("abcabc");
}

constexpr bool test_repeat_copy() {
    basic_string<char> s("abc");
    auto result = s.repeat_copy(3);
    return result == "abcabcabc" && s == "abc";
}

// 极端测试：repeat_copy 不修改原字符串
constexpr bool test_repeat_copy_preserves_original() {
    basic_string<char> s("xyz");
    auto result = s.repeat_copy(5);
    return result.size() == 15 && s == "xyz";
}

constexpr bool test_split() {
    basic_string<char> s("a,b,c");
    auto parts = s.split(',');
    return parts.size() == 3 && parts[0] == "a" && parts[1] == "b" && parts[2] == "c";
}

// 极端测试：split 连续分隔符
constexpr bool test_split_consecutive_delimiters() {
    basic_string<char> s("a,,b");
    auto parts = s.split(',');
    return parts.size() == 3 && parts[0] == "a" && parts[1] == "" && parts[2] == "b";
}

// 极端测试：split 无分隔符
constexpr bool test_split_no_delimiter() {
    basic_string<char> s("hello");
    auto parts = s.split(',');
    return parts.size() == 1 && parts[0] == "hello";
}

// 极端测试：split 空字符串
constexpr bool test_split_empty_string() {
    basic_string<char> s;
    auto parts = s.split(',');
    return parts.size() == 0;
}

// 综合极端测试：大量操作组合
constexpr bool test_combined_operations() {
    basic_string<char> s;
    // 大量push_back
    for (int i = 0; i < 100; ++i) {
        s.push_back('a');
    }
    // insert
    s.insert(50, 10, 'b');
    // append
    s.append("cdef");
    // erase
    s.erase(0, 20);
    // replace
    s.replace(10, 5, "REPLACED");
    // resize
    s.resize(200, 'z');
    return s.size() == 200;
}

// ============================================================================
// 原有 TEST_CASE 保持不变
// ============================================================================

TEST_CASE("basic_string default constructor") {
    STATIC_REQUIRE(test_default_constructor());
    basic_string<char> s;
    REQUIRE(s.empty());
    REQUIRE(s.size() == 0);
}

TEST_CASE("basic_string allocator constructor") {
    STATIC_REQUIRE(test_allocator_constructor());
    core::memory::allocator<char> alloc;
    basic_string<char> s(alloc);
    REQUIRE(s.empty());
    REQUIRE(s.get_allocator() == alloc);
}

TEST_CASE("basic_string count-char constructor") {
    STATIC_REQUIRE(test_count_char_constructor());
    basic_string<char> s(5, 'a');
    REQUIRE(s.size() == 5);
    REQUIRE(s[0] == 'a');
}

TEST_CASE("basic_string substring constructor") {
    STATIC_REQUIRE(test_substr_constructor());
    basic_string<char> s("hello world");
    basic_string<char> sub(s, 0, 5);
    REQUIRE(sub.size() == 5);
    REQUIRE(sub[0] == 'h');
}

TEST_CASE("basic_string ptr-count constructor") {
    STATIC_REQUIRE(test_ptr_count_constructor());
    basic_string<char> s("hello", 3);
    REQUIRE(s.size() == 3);
    REQUIRE(s[0] == 'h');
}

TEST_CASE("basic_string ptr constructor") {
    STATIC_REQUIRE(test_ptr_constructor());
    basic_string<char> s("hello");
    REQUIRE(s.size() == 5);
    REQUIRE(s[4] == 'o');
}

TEST_CASE("basic_string copy constructor") {
    STATIC_REQUIRE(test_copy_constructor());
    basic_string<char> s("hello");
    basic_string<char> copy(s);
    REQUIRE(copy == s);
}

TEST_CASE("basic_string move constructor") {
    STATIC_REQUIRE(test_move_constructor());
    basic_string<char> s("hello");
    basic_string<char> moved(std::move(s));
    REQUIRE(moved.size() == 5);
}

TEST_CASE("basic_string initializer list constructor") {
    STATIC_REQUIRE(test_initializer_list_constructor());
    basic_string<char> s({'a', 'b', 'c'});
    REQUIRE(s.size() == 3);
}

TEST_CASE("basic_string empty") {
    STATIC_REQUIRE(test_empty());
    basic_string<char> s;
    REQUIRE(s.empty());
}

TEST_CASE("basic_string size") {
    STATIC_REQUIRE(test_size());
    basic_string<char> s("abc");
    REQUIRE(s.size() == 3);
}

TEST_CASE("basic_string length") {
    STATIC_REQUIRE(test_length());
    basic_string<char> s("hello");
    REQUIRE(s.length() == 5);
}

TEST_CASE("basic_string max_size") {
    STATIC_REQUIRE(test_max_size());
    REQUIRE(basic_string<char>::max_size() > 0);
}

TEST_CASE("basic_string capacity") {
    STATIC_REQUIRE(test_capacity());
    basic_string<char> s;
    REQUIRE(s.capacity() == basic_string<char>::default_capacity);
}

TEST_CASE("basic_string data") {
    STATIC_REQUIRE(test_data());
    basic_string<char> s("abc");
    REQUIRE(s.data()[0] == 'a');
}

TEST_CASE("basic_string c_str") {
    STATIC_REQUIRE(test_c_str());
    basic_string<char> s("abc");
    REQUIRE(s.c_str()[2] == 'c');
}

TEST_CASE("basic_string at") {
    STATIC_REQUIRE(test_at());
    basic_string<char> s("abc");
    REQUIRE(s.at(0) == 'a');
}

TEST_CASE("basic_string operator[]") {
    STATIC_REQUIRE(test_operator_subscript());
    basic_string<char> s("abc");
    REQUIRE(s[0] == 'a');
}

TEST_CASE("basic_string front") {
    STATIC_REQUIRE(test_front());
    basic_string<char> s("abc");
    REQUIRE(s.front() == 'a');
}

TEST_CASE("basic_string back") {
    STATIC_REQUIRE(test_back());
    basic_string<char> s("abc");
    REQUIRE(s.back() == 'c');
}

TEST_CASE("basic_string begin/end") {
    STATIC_REQUIRE(test_begin_end());
    basic_string<char> s("abc");
    REQUIRE(*s.begin() == 'a');
}

TEST_CASE("basic_string cbegin/cend") {
    STATIC_REQUIRE(test_cbegin_cend());
    basic_string<char> s("abc");
    REQUIRE(*s.cbegin() == 'a');
}

TEST_CASE("basic_string rbegin/rend") {
    STATIC_REQUIRE(test_rbegin_rend());
    basic_string<char> s("abc");
    REQUIRE(*s.rbegin() == 'c');
}

TEST_CASE("basic_string crbegin/crend") {
    STATIC_REQUIRE(test_crbegin_crend());
    basic_string<char> s("abc");
    REQUIRE(*s.crbegin() == 'c');
}

TEST_CASE("basic_string shrink_to_fit") {
    STATIC_REQUIRE(test_shrink_to_fit());
    basic_string<char> s("hello world");
    s.reserve(100);
    s.shrink_to_fit();
    REQUIRE(s == "hello world");
}

TEST_CASE("basic_string reserve") {
    STATIC_REQUIRE(test_reserve());
    basic_string<char> s("abc");
    s.reserve(50);
    REQUIRE(s.capacity() >= 50);
}

TEST_CASE("basic_string resize default") {
    STATIC_REQUIRE(test_resize_default());
    basic_string<char> s("abc");
    s.resize(6);
    REQUIRE(s.size() == 6);
}

TEST_CASE("basic_string resize with char") {
    STATIC_REQUIRE(test_resize_with_char());
    basic_string<char> s("abc");
    s.resize(6, 'x');
    REQUIRE(s.size() == 6);
    REQUIRE(s[3] == 'x');
}

TEST_CASE("basic_string clear") {
    STATIC_REQUIRE(test_clear());
    basic_string<char> s("abc");
    s.clear();
    REQUIRE(s.empty());
}

TEST_CASE("basic_string push_back") {
    STATIC_REQUIRE(test_push_back());
    basic_string<char> s("ab");
    s.push_back('c');
    REQUIRE(s[2] == 'c');
}

TEST_CASE("basic_string swap") {
    STATIC_REQUIRE(test_swap());
    basic_string<char> s1("abc");
    basic_string<char> s2("xyz");
    s1.swap(s2);
    REQUIRE(s1 == "xyz");
}

TEST_CASE("basic_string assign count-char") {
    STATIC_REQUIRE(test_assign_count_char());
    basic_string<char> s;
    s.assign(3, 'a');
    REQUIRE(s == "aaa");
}

TEST_CASE("basic_string assign string") {
    STATIC_REQUIRE(test_assign_string());
    basic_string<char> s("abc");
    basic_string<char> t("xyz");
    s.assign(t);
    REQUIRE(s == "xyz");
}

TEST_CASE("basic_string assign substring") {
    STATIC_REQUIRE(test_assign_substring());
    basic_string<char> s;
    basic_string<char> t("hello");
    s.assign(t, 0, 3);
    REQUIRE(s == "hel");
}

TEST_CASE("basic_string assign ptr-count") {
    STATIC_REQUIRE(test_assign_ptr_count());
    basic_string<char> s;
    s.assign("hello", 3);
    REQUIRE(s == "hel");
}

TEST_CASE("basic_string assign ptr") {
    STATIC_REQUIRE(test_assign_ptr());
    basic_string<char> s;
    s.assign("hello");
    REQUIRE(s == "hello");
}

TEST_CASE("basic_string assign initializer_list") {
    STATIC_REQUIRE(test_assign_initializer_list());
    basic_string<char> s;
    s.assign({'a', 'b', 'c'});
    REQUIRE(s == "abc");
}

TEST_CASE("basic_string append count-char") {
    STATIC_REQUIRE(test_append_count_char());
    basic_string<char> s("ab");
    s.append(2, 'c');
    REQUIRE(s == "abcc");
}

TEST_CASE("basic_string append string") {
    STATIC_REQUIRE(test_append_string());
    basic_string<char> s("ab");
    basic_string<char> t("cd");
    s.append(t);
    REQUIRE(s == "abcd");
}

TEST_CASE("basic_string append substring") {
    STATIC_REQUIRE(test_append_substring());
    basic_string<char> s("ab");
    basic_string<char> t("cdef");
    s.append(t, 0, 2);
    REQUIRE(s == "abcd");
}

TEST_CASE("basic_string append ptr-count") {
    STATIC_REQUIRE(test_append_ptr_count());
    basic_string<char> s("ab");
    s.append("cd", 2);
    REQUIRE(s == "abcd");
}

TEST_CASE("basic_string append ptr") {
    STATIC_REQUIRE(test_append_ptr());
    basic_string<char> s("ab");
    s.append("cd");
    REQUIRE(s == "abcd");
}

TEST_CASE("basic_string operator+= string") {
    STATIC_REQUIRE(test_operator_plus_equal_string());
    basic_string<char> s("ab");
    basic_string<char> t("cd");
    s += t;
    REQUIRE(s == "abcd");
}

TEST_CASE("basic_string operator+= char") {
    STATIC_REQUIRE(test_operator_plus_equal_char());
    basic_string<char> s("ab");
    s += 'c';
    REQUIRE(s == "abc");
}

TEST_CASE("basic_string operator+= ptr") {
    STATIC_REQUIRE(test_operator_plus_equal_ptr());
    basic_string<char> s("ab");
    s += "cd";
    REQUIRE(s == "abcd");
}

TEST_CASE("basic_string find string_view") {
    STATIC_REQUIRE(test_find_sv());
    basic_string<char> s("hello world");
    REQUIRE(s.find(basic_string_view<char>("world")) == 6);
}

TEST_CASE("basic_string find char") {
    STATIC_REQUIRE(test_find_char());
    basic_string<char> s("hello");
    REQUIRE(s.find('l') == 2);
}

TEST_CASE("basic_string find ptr-count") {
    STATIC_REQUIRE(test_find_ptr_count());
    basic_string<char> s("hello world");
    REQUIRE(s.find("world", 0, 3) == 6);
}

TEST_CASE("basic_string find ptr") {
    STATIC_REQUIRE(test_find_ptr());
    basic_string<char> s("hello world");
    REQUIRE(s.find("world") == 6);
}

TEST_CASE("basic_string rfind string_view") {
    STATIC_REQUIRE(test_rfind_sv());
    basic_string<char> s("hello hello");
    REQUIRE(s.rfind(basic_string_view<char>("hello")) == 6);
}

TEST_CASE("basic_string rfind char") {
    STATIC_REQUIRE(test_rfind_char());
    basic_string<char> s("hello");
    REQUIRE(s.rfind('l') == 3);
}

TEST_CASE("basic_string find_first_of string_view") {
    STATIC_REQUIRE(test_find_first_of_sv());
    basic_string<char> s("hello");
    REQUIRE(s.find_first_of(basic_string_view<char>("eo")) == 1);
}

TEST_CASE("basic_string find_first_of char") {
    STATIC_REQUIRE(test_find_first_of_char());
    basic_string<char> s("hello");
    REQUIRE(s.find_first_of('l') == 2);
}

TEST_CASE("basic_string find_last_of string_view") {
    STATIC_REQUIRE(test_find_last_of_sv());
    basic_string<char> s("hello");
    REQUIRE(s.find_last_of(basic_string_view<char>("eo")) == 4);
}

TEST_CASE("basic_string find_last_of char") {
    STATIC_REQUIRE(test_find_last_of_char());
    basic_string<char> s("hello");
    REQUIRE(s.find_last_of('l') == 3);
}

TEST_CASE("basic_string find_first_not_of string_view") {
    STATIC_REQUIRE(test_find_first_not_of_sv());
    basic_string<char> s("hello");
    REQUIRE(s.find_first_not_of(basic_string_view<char>("hel")) == 4);
}

TEST_CASE("basic_string find_first_not_of char") {
    STATIC_REQUIRE(test_find_first_not_of_char());
    basic_string<char> s("aaab");
    REQUIRE(s.find_first_not_of('a') == 3);
}

TEST_CASE("basic_string find_last_not_of string_view") {
    STATIC_REQUIRE(test_find_last_not_of_sv());
    basic_string<char> s("hello");
    REQUIRE(s.find_last_not_of(basic_string_view<char>("lo")) == 1);
}

TEST_CASE("basic_string find_last_not_of char") {
    STATIC_REQUIRE(test_find_last_not_of_char());
    basic_string<char> s("baaa");
    REQUIRE(s.find_last_not_of('a') == 0);
}

TEST_CASE("basic_string starts_with string_view") {
    STATIC_REQUIRE(test_starts_with_sv());
    basic_string<char> s("hello world");
    REQUIRE(s.starts_with(basic_string_view<char>("hello")));
}

TEST_CASE("basic_string starts_with char") {
    STATIC_REQUIRE(test_starts_with_char());
    basic_string<char> s("hello");
    REQUIRE(s.starts_with('h'));
}

TEST_CASE("basic_string starts_with ptr") {
    STATIC_REQUIRE(test_starts_with_ptr());
    basic_string<char> s("hello world");
    REQUIRE(s.starts_with("hello"));
}

TEST_CASE("basic_string ends_with string_view") {
    STATIC_REQUIRE(test_ends_with_sv());
    basic_string<char> s("hello world");
    REQUIRE(s.ends_with(basic_string_view<char>("world")));
}

TEST_CASE("basic_string ends_with char") {
    STATIC_REQUIRE(test_ends_with_char());
    basic_string<char> s("hello");
    REQUIRE(s.ends_with('o'));
}

TEST_CASE("basic_string ends_with ptr") {
    STATIC_REQUIRE(test_ends_with_ptr());
    basic_string<char> s("hello world");
    REQUIRE(s.ends_with("world"));
}

TEST_CASE("basic_string contains string_view") {
    STATIC_REQUIRE(test_contains_sv());
    basic_string<char> s("hello world");
    REQUIRE(s.contains(basic_string_view<char>("llo")));
}

TEST_CASE("basic_string contains char") {
    STATIC_REQUIRE(test_contains_char());
    basic_string<char> s("hello");
    REQUIRE(s.contains('e'));
}

TEST_CASE("basic_string contains ptr") {
    STATIC_REQUIRE(test_contains_ptr());
    basic_string<char> s("hello world");
    REQUIRE(s.contains("world"));
}

TEST_CASE("basic_string insert count-char") {
    STATIC_REQUIRE(test_insert_count_char());
    basic_string<char> s("abc");
    s.insert(1, 2, 'x');
    REQUIRE(s == "axxbc");
}

TEST_CASE("basic_string insert ptr-count") {
    STATIC_REQUIRE(test_insert_ptr_count());
    basic_string<char> s("abc");
    s.insert(1, "xy", 2);
    REQUIRE(s == "axybc");
}

TEST_CASE("basic_string insert ptr") {
    STATIC_REQUIRE(test_insert_ptr());
    basic_string<char> s("abc");
    s.insert(1, "xy");
    REQUIRE(s == "axybc");
}

TEST_CASE("basic_string insert string") {
    STATIC_REQUIRE(test_insert_string());
    basic_string<char> s("abc");
    basic_string<char> t("xy");
    s.insert(1, t);
    REQUIRE(s == "axybc");
}

TEST_CASE("basic_string erase index-count") {
    STATIC_REQUIRE(test_erase_index_count());
    basic_string<char> s("hello");
    s.erase(2, 2);
    REQUIRE(s == "heo");
}

TEST_CASE("basic_string erase iterator") {
    STATIC_REQUIRE(test_erase_iterator());
    basic_string<char> s("hello");
    s.erase(s.begin() + 2);
    REQUIRE(s == "helo");
}

TEST_CASE("basic_string pop_back") {
    STATIC_REQUIRE(test_pop_back());
    basic_string<char> s("abc");
    s.pop_back();
    REQUIRE(s == "ab");
}

TEST_CASE("basic_string replace pos-count string") {
    STATIC_REQUIRE(test_replace_pos_count_string());
    basic_string<char> s("hello world");
    basic_string<char> t("there");
    s.replace(0, 5, t);
    REQUIRE(s == "there world");
}

TEST_CASE("basic_string replace pos-count ptr-count") {
    STATIC_REQUIRE(test_replace_pos_count_ptr_count());
    basic_string<char> s("hello world");
    s.replace(0, 5, "there", 5);
    REQUIRE(s == "there world");
}

TEST_CASE("basic_string replace pos-count ptr") {
    STATIC_REQUIRE(test_replace_pos_count_ptr());
    basic_string<char> s("hello world");
    s.replace(0, 5, "there");
    REQUIRE(s == "there world");
}

TEST_CASE("basic_string compare equal") {
    STATIC_REQUIRE(test_compare_equal());
    basic_string<char> s("abc");
    REQUIRE(s.compare("abc") == 0);
}

TEST_CASE("basic_string compare less") {
    STATIC_REQUIRE(test_compare_less());
    basic_string<char> s("aaa");
    REQUIRE(s.compare("aab") < 0);
}

TEST_CASE("basic_string compare greater") {
    STATIC_REQUIRE(test_compare_greater());
    basic_string<char> s("aac");
    REQUIRE(s.compare("aab") > 0);
}

TEST_CASE("basic_string substr") {
    STATIC_REQUIRE(test_substr());
    basic_string<char> s("hello world");
    REQUIRE(s.substr(0, 5) == "hello");
}

TEST_CASE("basic_string operator==") {
    STATIC_REQUIRE(test_operator_equals());
    basic_string<char> s1("abc");
    basic_string<char> s2("abc");
    REQUIRE(s1 == s2);
}

TEST_CASE("basic_string operator!=") {
    STATIC_REQUIRE(test_operator_not_equals());
    basic_string<char> s1("abc");
    basic_string<char> s2("abd");
    REQUIRE(s1 != s2);
}

TEST_CASE("basic_string operator<") {
    STATIC_REQUIRE(test_operator_less());
    basic_string<char> s1("aaa");
    basic_string<char> s2("aab");
    REQUIRE(s1 < s2);
}

TEST_CASE("basic_string operator>") {
    STATIC_REQUIRE(test_operator_greater());
    basic_string<char> s1("aac");
    basic_string<char> s2("aab");
    REQUIRE(s1 > s2);
}

TEST_CASE("basic_string operator<=") {
    STATIC_REQUIRE(test_operator_less_equal());
    basic_string<char> s1("aaa");
    basic_string<char> s2("aab");
    REQUIRE(s1 <= s2);
}

TEST_CASE("basic_string operator>=") {
    STATIC_REQUIRE(test_operator_greater_equal());
    basic_string<char> s1("aac");
    basic_string<char> s2("aab");
    REQUIRE(s1 >= s2);
}

TEST_CASE("basic_string operator+") {
    STATIC_REQUIRE(test_operator_plus());
    basic_string<char> s1("abc");
    basic_string<char> s2("def");
    REQUIRE(s1 + s2 == "abcdef");
}

TEST_CASE("basic_string repeat") {
    STATIC_REQUIRE(test_repeat());
    basic_string<char> s("abc");
    s.repeat(3);
    REQUIRE(s == "abcabcabc");
}

TEST_CASE("basic_string repeat_copy") {
    STATIC_REQUIRE(test_repeat_copy());
    basic_string<char> s("abc");
    auto result = s.repeat_copy(3);
    REQUIRE(result == "abcabcabc");
}

TEST_CASE("basic_string split") {
    STATIC_REQUIRE(test_split());
    basic_string<char> s("a,b,c");
    auto parts = s.split(',');
    REQUIRE(parts.size() == 3);
    REQUIRE(parts[0] == "a");
}

// ============================================================================
// 新增极端测试 TEST_CASE
// ============================================================================

TEST_CASE("basic_string large count-char constructor") {
    STATIC_REQUIRE(test_large_count_char_constructor());
    basic_string<char> s(1000, 'x');
    REQUIRE(s.size() == 1000);
    REQUIRE(s[999] == 'x');
    REQUIRE(s[500] == 'x');
}

TEST_CASE("basic_string iterator traversal on large string") {
    STATIC_REQUIRE(test_iterator_large_string());
    basic_string<char> s(500, 'a');
    int count = 0;
    for (auto it = s.begin(); it != s.end(); ++it) {
        REQUIRE(*it == 'a');
        ++count;
    }
    REQUIRE(count == 500);
}

TEST_CASE("basic_string shrink_to_fit at boundary") {
    STATIC_REQUIRE(test_shrink_to_fit_at_boundary());
    basic_string<char> s;
    for (int i = 0; i < basic_string<char>::default_capacity; ++i) {
        s.push_back('a');
    }
    auto cap_before = s.capacity();
    s.shrink_to_fit();
    REQUIRE(s.capacity() <= cap_before);
    REQUIRE(s.size() == basic_string<char>::default_capacity);
}

TEST_CASE("basic_string reserve exact capacity") {
    STATIC_REQUIRE(test_reserve_exact_capacity());
    basic_string<char> s("abc");
    auto old_cap = s.capacity();
    s.reserve(old_cap);
    REQUIRE(s.capacity() >= old_cap);
    REQUIRE(s == "abc");
}

TEST_CASE("basic_string reserve large") {
    STATIC_REQUIRE(test_reserve_large());
    basic_string<char> s("abc");
    s.reserve(10000);
    REQUIRE(s.capacity() >= 10000);
    REQUIRE(s == "abc");
}

TEST_CASE("basic_string reserve from zero") {
    STATIC_REQUIRE(test_reserve_from_zero());
    basic_string<char> s;
    s.reserve(50);
    REQUIRE(s.capacity() >= 50);
    REQUIRE(s.empty());
}

TEST_CASE("basic_string resize across capacity boundary") {
    STATIC_REQUIRE(test_resize_across_capacity_boundary());
    basic_string<char> s("abc");
    auto old_cap = s.capacity();
    s.resize(old_cap + 1, 'x');
    REQUIRE(s.size() == old_cap + 1);
    REQUIRE(s.capacity() > old_cap);
    REQUIRE(s[old_cap] == 'x');
}

TEST_CASE("basic_string resize large") {
    STATIC_REQUIRE(test_resize_large());
    basic_string<char> s("abc");
    s.resize(2000, 'z');
    REQUIRE(s.size() == 2000);
    REQUIRE(s[0] == 'a');
    REQUIRE(s[1999] == 'z');
}

TEST_CASE("basic_string resize shrink then grow") {
    STATIC_REQUIRE(test_resize_shrink_then_grow());
    basic_string<char> s("hello world");
    s.resize(3);
    s.resize(10, '!');
    REQUIRE(s.size() == 10);
    REQUIRE(s[3] == '!');
}

TEST_CASE("basic_string clear and reuse") {
    STATIC_REQUIRE(test_clear_and_reuse());
    basic_string<char> s("hello");
    s.clear();
    s.append("world");
    REQUIRE(s == "world");
}

TEST_CASE("basic_string push_back many") {
    STATIC_REQUIRE(test_push_back_many());
    basic_string<char> s;
    for (int i = 0; i < 1000; ++i) {
        s.push_back(static_cast<char>('a' + (i % 26)));
    }
    REQUIRE(s.size() == 1000);
    REQUIRE(s[0] == 'a');
}

TEST_CASE("basic_string push_back at capacity boundary") {
    STATIC_REQUIRE(test_push_back_at_capacity_boundary());
    basic_string<char> s;
    for (int i = 0; i < basic_string<char>::default_capacity; ++i) {
        s.push_back('a');
    }
    auto old_cap = s.capacity();
    s.push_back('b');
    REQUIRE(s.size() == basic_string<char>::default_capacity + 1);
    REQUIRE(s.capacity() > old_cap);
    REQUIRE(s[s.size() - 1] == 'b');
}

TEST_CASE("basic_string swap different capacities") {
    STATIC_REQUIRE(test_swap_different_capacities());
    basic_string<char> s1("a");
    basic_string<char> s2;
    s2.reserve(100);
    s2.append("large string with more content");
    auto s1_old_size = s1.size();
    auto s2_old_size = s2.size();
    s1.swap(s2);
    REQUIRE(s1.size() == s2_old_size);
    REQUIRE(s2.size() == s1_old_size);
}

TEST_CASE("basic_string swap self") {
    STATIC_REQUIRE(test_swap_self());
    basic_string<char> s("hello");
    s.swap(s);
    REQUIRE(s == "hello");
    REQUIRE(s.size() == 5);
}

TEST_CASE("basic_string assign large count") {
    STATIC_REQUIRE(test_assign_large_count());
    basic_string<char> s("hello");
    s.assign(5000, 'z');
    REQUIRE(s.size() == 5000);
    REQUIRE(s[4999] == 'z');
}

TEST_CASE("basic_string assign self") {
    STATIC_REQUIRE(test_assign_self());
    basic_string<char> s("hello");
    s.assign(s);
    REQUIRE(s == "hello");
}

TEST_CASE("basic_string append trigger reallocation") {
    STATIC_REQUIRE(test_append_trigger_reallocation());
    basic_string<char> s("abc");
    auto old_cap = s.capacity();
    s.append(1000, 'x');
    REQUIRE(s.size() == 1003);
    REQUIRE(s.capacity() > old_cap);
    REQUIRE(s[1002] == 'x');
}

TEST_CASE("basic_string append at boundary") {
    STATIC_REQUIRE(test_append_at_boundary());
    basic_string<char> s;
    for (int i = 0; i < basic_string<char>::default_capacity - 2; ++i) {
        s.push_back('a');
    }
    auto old_cap = s.capacity();
    s.append("bcde");
    REQUIRE(s.size() == basic_string<char>::default_capacity + 2);
    REQUIRE(s.capacity() > old_cap);
}

TEST_CASE("basic_string operator+= long string") {
    STATIC_REQUIRE(test_operator_plus_equal_long_string());
    basic_string<char> s("start:");
    basic_string<char> t(1000, 'x');
    s += t;
    REQUIRE(s.size() == 1006);
    REQUIRE(s.starts_with("start:"));
}

TEST_CASE("basic_string operator+= char at boundary") {
    STATIC_REQUIRE(test_operator_plus_equal_char_at_boundary());
    basic_string<char> s;
    for (int i = 0; i < basic_string<char>::default_capacity; ++i) {
        s.push_back('a');
    }
    auto old_cap = s.capacity();
    s += 'b';
    REQUIRE(s.size() == basic_string<char>::default_capacity + 1);
    REQUIRE(s.capacity() > old_cap);
}

TEST_CASE("basic_string find not found") {
    STATIC_REQUIRE(test_find_not_found());
    basic_string<char> s("hello world");
    REQUIRE(s.find(basic_string_view<char>("xyz")) == basic_string<char>::npos);
}

TEST_CASE("basic_string find in large string") {
    STATIC_REQUIRE(test_find_in_large_string());
    basic_string<char> s(1000, 'a');
    basic_string<char> t(s);
    t[998] = 'b';
    t[999] = 'c';
    REQUIRE(t.find(basic_string_view<char>("bc")) == 998);
}

TEST_CASE("basic_string find char not found") {
    STATIC_REQUIRE(test_find_char_not_found());
    basic_string<char> s("hello");
    REQUIRE(s.find('z') == basic_string<char>::npos);
}

TEST_CASE("basic_string find empty string") {
    STATIC_REQUIRE(test_find_empty_string());
    basic_string<char> s("hello");
    REQUIRE(s.find(basic_string_view<char>("")) == 0);
}

TEST_CASE("basic_string rfind repeated pattern") {
    STATIC_REQUIRE(test_rfind_repeated_pattern());
    basic_string<char> s("ababababab");
    REQUIRE(s.rfind(basic_string_view<char>("ab")) == 8);
}

TEST_CASE("basic_string rfind first char") {
    STATIC_REQUIRE(test_rfind_first_char());
    basic_string<char> s("abcde");
    REQUIRE(s.rfind('a') == 0);
}

TEST_CASE("basic_string find_first_of no match") {
    STATIC_REQUIRE(test_find_first_of_no_match());
    basic_string<char> s("abc");
    REQUIRE(s.find_first_of(basic_string_view<char>("xyz")) == basic_string<char>::npos);
}

TEST_CASE("basic_string find_first_not_of all match") {
    STATIC_REQUIRE(test_find_first_not_of_all_match());
    basic_string<char> s("aaa");
    REQUIRE(s.find_first_not_of('a') == basic_string<char>::npos);
}

TEST_CASE("basic_string starts_with empty") {
    STATIC_REQUIRE(test_starts_with_empty());
    basic_string<char> s("hello");
    REQUIRE(s.starts_with(basic_string_view<char>("")));
}

TEST_CASE("basic_string starts_with exact match") {
    STATIC_REQUIRE(test_starts_with_exact_match());
    basic_string<char> s("hello");
    REQUIRE(s.starts_with(basic_string_view<char>("hello")));
}

TEST_CASE("basic_string starts_with longer than string") {
    STATIC_REQUIRE(test_starts_with_longer_than_string());
    basic_string<char> s("hi");
    REQUIRE_FALSE(s.starts_with(basic_string_view<char>("hello")));
}

TEST_CASE("basic_string starts_with wrong char") {
    STATIC_REQUIRE(test_starts_with_wrong_char());
    basic_string<char> s("hello");
    REQUIRE_FALSE(s.starts_with('x'));
}

TEST_CASE("basic_string ends_with empty") {
    STATIC_REQUIRE(test_ends_with_empty());
    basic_string<char> s("hello");
    REQUIRE(s.ends_with(basic_string_view<char>("")));
}

TEST_CASE("basic_string ends_with char on empty") {
    STATIC_REQUIRE(test_ends_with_char_on_empty());
    basic_string<char> s;
    REQUIRE_FALSE(s.ends_with('a'));
}

TEST_CASE("basic_string contains empty") {
    STATIC_REQUIRE(test_contains_empty());
    basic_string<char> s("hello");
    REQUIRE(s.contains(basic_string_view<char>("")));
}

TEST_CASE("basic_string contains not found") {
    STATIC_REQUIRE(test_contains_not_found());
    basic_string<char> s("hello");
    REQUIRE_FALSE(s.contains(basic_string_view<char>("xyz")));
}

TEST_CASE("basic_string insert at boundary") {
    STATIC_REQUIRE(test_insert_at_boundary());
    basic_string<char> s;
    for (int i = 0; i < basic_string<char>::default_capacity - 2; ++i) {
        s.push_back('a');
    }
    s.insert(s.size(), "bc");
    REQUIRE(s.size() == basic_string<char>::default_capacity);
    REQUIRE(s[s.size() - 1] == 'c');
}

TEST_CASE("basic_string insert large string") {
    STATIC_REQUIRE(test_insert_large_string());
    basic_string<char> s("start");
    basic_string<char> t(500, 'x');
    s.insert(0, t);
    REQUIRE(s.size() == 505);
    REQUIRE(s[0] == 'x');
    REQUIRE(s[504] == 't');
}

TEST_CASE("basic_string erase all") {
    STATIC_REQUIRE(test_erase_all());
    basic_string<char> s("hello");
    s.erase(0, 5);
    REQUIRE(s.empty());
}

TEST_CASE("basic_string erase beyond length") {
    STATIC_REQUIRE(test_erase_beyond_length());
    basic_string<char> s("hello");
    s.erase(3, 100);
    REQUIRE(s == "hel");
}

TEST_CASE("basic_string erase first and last") {
    STATIC_REQUIRE(test_erase_first_and_last());
    basic_string<char> s("abcde");
    s.erase(s.begin());
    s.erase(s.end() - 1);
    REQUIRE(s == "bcd");
}

TEST_CASE("basic_string pop_back until empty") {
    STATIC_REQUIRE(test_pop_back_until_empty());
    basic_string<char> s("abc");
    s.pop_back();
    s.pop_back();
    s.pop_back();
    REQUIRE(s.empty());
}

TEST_CASE("basic_string replace with larger string") {
    STATIC_REQUIRE(test_replace_with_larger_string());
    basic_string<char> s("hi");
    s.replace(0, 2, "hello world, this is a much longer string");
    REQUIRE(s.starts_with("hello world"));
}

TEST_CASE("basic_string replace same content") {
    STATIC_REQUIRE(test_replace_same_content());
    basic_string<char> s("hello");
    s.replace(0, 5, "hello");
    REQUIRE(s == "hello");
}

TEST_CASE("basic_string compare with empty") {
    STATIC_REQUIRE(test_compare_with_empty());
    basic_string<char> s("abc");
    REQUIRE(s.compare(basic_string<char>()) > 0);
}

TEST_CASE("basic_string compare long strings") {
    STATIC_REQUIRE(test_compare_long_strings());
    basic_string<char> s1(1000, 'a');
    basic_string<char> s2(1000, 'a');
    s2[999] = 'b';
    REQUIRE(s1.compare(s2) < 0);
}

TEST_CASE("basic_string substr entire string") {
    STATIC_REQUIRE(test_substr_entire_string());
    basic_string<char> s("hello");
    REQUIRE(s.substr(0, 5) == s);
}

TEST_CASE("basic_string substr zero length") {
    STATIC_REQUIRE(test_substr_zero_length());
    basic_string<char> s("hello");
    REQUIRE(s.substr(2, 0).empty());
}

TEST_CASE("basic_string operator== with empty") {
    STATIC_REQUIRE(test_operator_equals_with_empty());
    basic_string<char> s1;
    basic_string<char> s2;
    REQUIRE(s1 == s2);
}

TEST_CASE("basic_string operator!= different lengths") {
    STATIC_REQUIRE(test_operator_not_equals_different_lengths());
    basic_string<char> s1("abc");
    basic_string<char> s2("abcd");
    REQUIRE(s1 != s2);
}

TEST_CASE("basic_string operator+ multiple") {
    STATIC_REQUIRE(test_operator_plus_multiple());
    basic_string<char> s1("a");
    basic_string<char> s2("b");
    basic_string<char> s3("c");
    REQUIRE(s1 + s2 + s3 == "abc");
}

TEST_CASE("basic_string repeat zero") {
    STATIC_REQUIRE(test_repeat_zero());
    basic_string<char> s("abc");
    s.repeat(0);
    REQUIRE(s.empty());
}

TEST_CASE("basic_string repeat once") {
    STATIC_REQUIRE(test_repeat_once());
    basic_string<char> s("abc");
    s.repeat(1);
    REQUIRE(s == "abc");
}

TEST_CASE("basic_string repeat many times") {
    STATIC_REQUIRE(test_repeat_many_times());
    basic_string<char> s("abc");
    s.repeat(100);
    REQUIRE(s.size() == 300);
    REQUIRE(s.starts_with("abcabc"));
}

TEST_CASE("basic_string repeat_copy preserves original") {
    STATIC_REQUIRE(test_repeat_copy_preserves_original());
    basic_string<char> s("xyz");
    auto result = s.repeat_copy(5);
    REQUIRE(result.size() == 15);
    REQUIRE(s == "xyz");
}

TEST_CASE("basic_string split consecutive delimiters") {
    STATIC_REQUIRE(test_split_consecutive_delimiters());
    basic_string<char> s("a,,b");
    auto parts = s.split(',');
    REQUIRE(parts.size() == 3);
    REQUIRE(parts[0] == "a");
    REQUIRE(parts[1] == "");
    REQUIRE(parts[2] == "b");
}

TEST_CASE("basic_string split no delimiter") {
    STATIC_REQUIRE(test_split_no_delimiter());
    basic_string<char> s("hello");
    auto parts = s.split(',');
    REQUIRE(parts.size() == 1);
    REQUIRE(parts[0] == "hello");
}

TEST_CASE("basic_string split empty string") {
    STATIC_REQUIRE(test_split_empty_string());
    basic_string<char> s;
    auto parts = s.split(',');
    REQUIRE(parts.size() == 0);
}

TEST_CASE("basic_string combined operations") {
    STATIC_REQUIRE(test_combined_operations());
    basic_string<char> s;
    for (int i = 0; i < 100; ++i) {
        s.push_back('a');
    }
    s.insert(50, 10, 'b');
    s.append("cdef");
    s.erase(0, 20);
    s.replace(10, 5, "REPLACED");
    s.resize(200, 'z');
    REQUIRE(s.size() == 200);
}