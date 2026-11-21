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
#include <benchmark/benchmark.h>
#include <rainy/core/collections/array.hpp>
#include <rainy/core/container/tuple.hpp>
#include <rainy/core/container/variant.hpp>
#include <rainy/core/mechanism/pattern_matching.hpp>

namespace pm = rainy::core::mechanism::pattern_matching;
namespace rc = rainy::core::container;
namespace rcol = rainy::core::collections;
namespace ru = rainy::utility;

namespace {
    template <typename... Ts>
    struct visitor : Ts... {
        using Ts::operator()...;
    };
    template <typename... Ts>
    visitor(Ts...) -> visitor<Ts...>;

    using dispatch_value = rc::variant<ru::monostate, int, double>;

    int variant_input(std::size_t salt) {
        dispatch_value value{};
        switch (salt % 3) {
            case 0:
                value = 42;
                break;
            case 1:
                value = 3.5;
                break;
            default:
                break;
        }
        return salt % 3;
    }
} // namespace

static void bm_variant_dsl(benchmark::State &state) {
    const std::size_t salt = static_cast<std::size_t>(state.range(0));
    dispatch_value value = variant_input(salt) == 0 ? dispatch_value{ru::monostate{}} : (salt % 3 == 1 ? dispatch_value{42} : dispatch_value{3.5});
    for (auto _ : state) {
        int result = pm::match(value)(
            (pm::pattern | pm::meet([](const dispatch_value &v) { return rc::holds_alternative<int>(v) && rc::get<int>(v) > 0; })) =
                [] { return 1; },
            (pm::pattern | pm::meet([](const dispatch_value &v) { return rc::holds_alternative<double>(v) && rc::get<double>(v) > 0; })) =
                [] { return 2; },
            (pm::pattern | pm::_) = [] { return 0; });
        benchmark::DoNotOptimize(result);
    }
}

static void bm_variant_if_else(benchmark::State &state) {
    const std::size_t salt = static_cast<std::size_t>(state.range(0));
    dispatch_value value = variant_input(salt) == 0 ? dispatch_value{ru::monostate{}} : (salt % 3 == 1 ? dispatch_value{42} : dispatch_value{3.5});
    for (auto _ : state) {
        int result = 0;
        if (rc::holds_alternative<int>(value)) {
            const int &v = rc::get<int>(value);
            result = v > 0 ? 1 : 0;
        } else if (rc::holds_alternative<double>(value)) {
            const double &v = rc::get<double>(value);
            result = v > 0 ? 2 : 0;
        }
        benchmark::DoNotOptimize(result);
    }
}

static void bm_variant_visit(benchmark::State &state) {
    const std::size_t salt = static_cast<std::size_t>(state.range(0));
    dispatch_value value = variant_input(salt) == 0 ? dispatch_value{ru::monostate{}} : (salt % 3 == 1 ? dispatch_value{42} : dispatch_value{3.5});
    for (auto _ : state) {
        int result = rc::visit(visitor{[](int v) { return v > 0 ? 1 : 0; }, [](double v) { return v > 0 ? 2 : 0; },
                                       [](ru::monostate) { return 0; }},
                               value);
        benchmark::DoNotOptimize(result);
    }
}

static void bm_tuple_dsl(benchmark::State &state) {
    const std::size_t salt = static_cast<std::size_t>(state.range(0));
    rc::tuple<int, int, int> value{static_cast<int>(salt % 10), 2, 3};
    for (auto _ : state) {
        int result = pm::match(value)((pm::pattern | pm::ds(pm::_, pm::meet([](int v) { return v == 2; }), 3)) = [] { return 1; },
                                      (pm::pattern | pm::_) = [] { return 0; });
        benchmark::DoNotOptimize(result);
    }
}

static void bm_tuple_if_else(benchmark::State &state) {
    const std::size_t salt = static_cast<std::size_t>(state.range(0));
    rc::tuple<int, int, int> value{static_cast<int>(salt % 10), 2, 3};
    for (auto _ : state) {
        int result = (rc::get<1>(value) == 2 && rc::get<2>(value) == 3) ? 1 : 0;
        benchmark::DoNotOptimize(result);
    }
}

static void bm_array_ooo_dsl(benchmark::State &state) {
    const std::size_t salt = static_cast<std::size_t>(state.range(0));
    const std::size_t length = static_cast<std::size_t>(state.range(1));
    rcol::array<int, 8> value{1, 2, 3, 4, 5, 6, 7, 8};
    value[7] = static_cast<int>((salt + length) % 10);
    pm::id<pm::implements::subrange_t<rcol::array<int, 8>>> rest;
    for (auto _ : state) {
        int result = pm::match(value)((pm::pattern | pm::ds(1, pm::ooo(rest), value[7])) = [&] { return static_cast<int>((*rest).size()); },
                                      (pm::pattern | pm::_) = [] { return 0; });
        benchmark::DoNotOptimize(result);
    }
}

static void bm_array_ooo_handwritten(benchmark::State &state) {
    const std::size_t salt = static_cast<std::size_t>(state.range(0));
    const std::size_t length = static_cast<std::size_t>(state.range(1));
    rcol::array<int, 8> value{1, 2, 3, 4, 5, 6, 7, 8};
    value[7] = static_cast<int>((salt + length) % 10);
    for (auto _ : state) {
        int result = value[0] == 1 && value[7] == static_cast<int>((salt + length) % 10) ? static_cast<int>(6) : 0;
        benchmark::DoNotOptimize(result);
    }
}

BENCHMARK(bm_variant_dsl)->Arg(0)->Arg(1)->Arg(2);
BENCHMARK(bm_variant_if_else)->Arg(0)->Arg(1)->Arg(2);
BENCHMARK(bm_variant_visit)->Arg(0)->Arg(1)->Arg(2);
BENCHMARK(bm_tuple_dsl)->Arg(0)->Arg(5);
BENCHMARK(bm_tuple_if_else)->Arg(0)->Arg(5);
BENCHMARK(bm_array_ooo_dsl)->Args({0, 8})->Args({3, 8});
BENCHMARK(bm_array_ooo_handwritten)->Args({0, 8})->Args({3, 8});