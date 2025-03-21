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
#ifndef RAINY_UTILITY_STREAM_ITERATOR_HPP
#define RAINY_UTILITY_STREAM_ITERATOR_HPP
#include <rainy/utility/iterator.hpp>
#include <istream>
#include <ostream>

namespace rainy::utility {
    template <typename Ty, typename Elem = char, typename Traits = std::char_traits<Elem>, typename Diff = std::ptrdiff_t>
    class istream_iterator {
    public:
        using iterator_category = std::input_iterator_tag;
        using value_type = Ty;
        using difference_type = Diff;
        using pointer = const value_type *;
        using reference = const value_type &;
        using char_type = Elem;
        using traits_type = Traits;
        using istream_type = std::basic_istream<Elem, Traits>;

        static_assert(type_traits::logical_traits::conjunction_v<std::is_default_constructible<Ty>, std::is_copy_constructible<Ty>,
                                                                 std::is_copy_assignable<Ty>>,
                      "istream_iterator<T> requires T to be default constructible, copy constructible, and copy assignable. ");

        constexpr istream_iterator() noexcept(std::is_nothrow_default_constructible_v<Ty>) {
        }

        istream_iterator(istream_iterator &ifstream) : istream(addressof(ifstream)) {
        }

        RAINY_NODISCARD reference operator*() const noexcept {
            expects(istream, "The stored istream pointer in istream_iterator must be not-null!");
            return value;
        }

        RAINY_NODISCARD pointer operator->() const noexcept {
            expects(istream, "The stored istream pointer in istream_iterator must be not-null!");
            return addressof(value);
        }

        istream_iterator &self_inc_prefix() {
            read_next();
            ++(*this);
            return *this;
        }

        istream_iterator &operator++() {
            return self_inc_prefix();
        }

        istream_iterator self_inc_postfix() {
            istream_iterator tmp = *this;
            ++(*this);
            return tmp;
        }

        istream_iterator operator++(int) {
            return self_inc_postfix();
        }

        RAINY_NODISCARD bool equal(const istream_iterator &right) const noexcept {
            return istream == right.istream;
        }

        template <typename Ty_, typename Elem_, typename Traits_, typename Diff_>
        RAINY_NODISCARD friend bool operator==(const istream_iterator<Ty_, Elem_, Traits_, Diff_> &left,
                                               const istream_iterator<Ty_, Elem_, Traits_, Diff_> &right) {
            return left.equal(right);
        }

        RAINY_NODISCARD friend bool operator==(const istream_iterator &left, const istream_iterator &right) {
            return left.equal(right);
        }

    private:
        void read_next() {
            expects(istream, "The stored istream pointer in istream_iterator must be not-null!");
            if (!(*istream >> value)) {
                istream = nullptr;
            }
        }

        istream_type *istream{nullptr};
        Ty value{};
    };

    template <typename Ty, typename Elem = char, typename Traits = std::char_traits<Elem>>
    class ostream_iterator {
    public:
        using iterator_category = std::output_iterator_tag;
        using value_type = void;
        using difference_type = std::ptrdiff_t;
        using pointer = void;
        using reference = void;
        using char_type = Elem;
        using traits_type = Traits;
        using ostream_type = std::basic_ostream<Elem, Traits>;

        ostream_iterator() = delete;

        ostream_iterator(ostream_type &ostream, const char_type *const delim = nullptr) noexcept :
            delim(delim), ostream(rainy::utility::addressof(ostream)) {
        }

        ostream_iterator &operator=(const Ty &val) {
            *ostream << val;
            if (delim) {
                *ostream << delim;
            }
            return *this;
        }

        RAINY_NODISCARD ostream_iterator &self_inc_prefix() noexcept {
            return *this;
        }

        RAINY_NODISCARD ostream_iterator &self_inc_postfix(int) noexcept {
            return *this;
        }

        RAINY_NODISCARD ostream_iterator &dereference() noexcept {
            return *this;
        }

        ostream_iterator &operator++() noexcept {
            return self_inc_prefix();
        }

        ostream_iterator &operator++(int) noexcept {
            return self_inc_postfix(0); // 不改变对象
        }

        ostream_iterator &operator*() noexcept {
            return dereference();
        }

    private:
        const char_type *delim;
        ostream_type *ostream;
    };
}

#endif