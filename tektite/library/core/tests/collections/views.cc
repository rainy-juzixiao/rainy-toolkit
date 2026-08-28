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
#include <catch2/catch_test_macros.hpp>
#include <rainy/core/collections/views.hpp>

namespace views = rainy::core::collections::views;

template <typename T>
class vector_view : public views::view_interface<vector_view<T>> {
public:
    using iterator = T *;
    using difference_type = std::ptrdiff_t;

    vector_view(T *first, T *last) : first_(first), last_(last) {
    }

    iterator begin() const {
        return first_;
    }
    iterator end() const {
        return last_;
    }
    iterator cbegin() const {
        return first_;
    }
    iterator cend() const {
        return last_;
    }

private:
    T *first_;
    T *last_;
};

TEST_CASE("view_interface provides range interface", "[collections][views]") {
    int raw[5] = {1, 2, 3, 4, 5};
    vector_view<int> view(raw, raw + 5);

    REQUIRE_FALSE(view.empty());
    REQUIRE(view.size() == 5);
    REQUIRE(view);
    REQUIRE(view.front() == 1);
    REQUIRE(view.back() == 5);
    REQUIRE(view[2] == 3);
    REQUIRE(*view.begin() == 1);
    REQUIRE(*view.cbegin() == 1);
    REQUIRE(view.data() == raw);

    view[0] = 10;
    REQUIRE(raw[0] == 10);

    vector_view<int> empty_view(raw, raw);
    REQUIRE(empty_view.empty());
    REQUIRE_FALSE(empty_view);
    REQUIRE(empty_view.size() == 0);
}

namespace testing {
    struct collect_fn {
        template <typename Range>
        std::vector<std::size_t> operator()(Range &&range) const {
            std::vector<std::size_t> out;
            for (auto &&element : range) {
                out.push_back(static_cast<std::size_t>(element));
            }
            return out;
        }
    };

    struct scale_fn {
        template <typename Range, typename Factor>
        std::vector<int> operator()(Range &&range, Factor factor) const {
            std::vector<int> out;
            for (auto &&element : range) {
                out.push_back(element * factor);
            }
            return out;
        }
    };
} // namespace testing

TEST_CASE("range_closure applies a stateless functor with captures", "[collections][views]") {
    std::vector<int> data{1, 2, 3, 4};
    views::implements::range_closure<testing::scale_fn, int> scale(2);
    std::vector<int> scaled = scale(data);
    REQUIRE((scaled == std::vector<int>{2, 4, 6, 8}));
}

TEST_CASE("pipeline composes range adaptor closures with operator|", "[collections][views]") {
    using views::implements::range_closure;

    constexpr views::implements::range_closure<testing::collect_fn> collect{};
    constexpr views::implements::range_closure<testing::scale_fn, int> scale(3);

    std::vector<int> data{1, 2, 3};

    SECTION("range piped into a single closure") {
        std::vector<int> result = data | scale;
        REQUIRE((result == std::vector<int>{3, 6, 9}));
    }
    SECTION("two closures compose into a pipeline") {
        auto composed = scale | collect;
        std::vector<std::size_t> result = composed(data);
        REQUIRE((result == std::vector<std::size_t>{3, 6, 9}));
    }
    SECTION("pipeline applied directly to a range") {
        std::vector<std::size_t> result = (data | scale | collect);
        REQUIRE((result == std::vector<std::size_t>{3, 6, 9}));
    }
}
