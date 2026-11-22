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
#include <rainy/foundation/willow/protobuf.hpp>

#include <cstdint>
#include <string>
#include <vector>

using namespace rainy::foundation::willow;
using namespace rainy::foundation::willow::protobuf;

namespace {
    struct direct_phone {
        std::string number{};

        static constexpr rainy::core::container::tuple<member_field<"number", proto::string, 1, &direct_phone::number>>
            protobuf_fields{};
    };

    struct direct_person {
        std::string name{};
        std::int32_t age{};
        std::vector<std::string> tags{};
        direct_phone phone{};

        static constexpr rainy::core::container::tuple<member_field<"name", proto::string, 1, &direct_person::name>,
                                                       member_field<"age", proto::int32, 2, &direct_person::age>,
                                                       member_field<"tags", proto::repeated<proto::string>, 3,
                                                                    &direct_person::tags>,
                                                       member_field<"phone", proto::message<direct_phone>, 4,
                                                                    &direct_person::phone>>
            protobuf_fields{};
    };

    struct direct_scalars {
        std::int32_t i32{};
        std::uint32_t u32{};
        std::int32_t s32{};
        std::int64_t i64{};
        bool flag{};
        std::uint32_t f32{};
        std::int32_t sf32{};
        float f{};
        std::uint64_t f64{};
        std::int64_t sf64{};
        double d{};
        std::string s{};
        std::string b{};

        static constexpr rainy::core::container::tuple<
            member_field<"i32", proto::int32, 1, &direct_scalars::i32>,
            member_field<"u32", proto::uint32, 2, &direct_scalars::u32>,
            member_field<"s32", proto::sint32, 3, &direct_scalars::s32>,
            member_field<"i64", proto::int64, 4, &direct_scalars::i64>,
            member_field<"flag", proto::boolean, 5, &direct_scalars::flag>,
            member_field<"f32", proto::fixed32, 6, &direct_scalars::f32>,
            member_field<"sf32", proto::sfixed32, 7, &direct_scalars::sf32>,
            member_field<"f", proto::floating, 8, &direct_scalars::f>,
            member_field<"f64", proto::fixed64, 9, &direct_scalars::f64>,
            member_field<"sf64", proto::sfixed64, 10, &direct_scalars::sf64>,
            member_field<"d", proto::doubling, 11, &direct_scalars::d>,
            member_field<"s", proto::string, 12, &direct_scalars::s>,
            member_field<"b", proto::bytes, 13, &direct_scalars::b>>
            protobuf_fields{};
    };

    struct direct_numbers {
        std::vector<std::int32_t> e{};

        static constexpr rainy::core::container::tuple<member_field<"e", proto::repeated<proto::int32>, 1, &direct_numbers::e>>
            protobuf_fields{};
    };

    direct_person make_person() {
        direct_person person{};
        person.name = "testing";
        person.age = 32;
        person.tags = {"alpha", "beta"};
        person.phone.number = "1234";
        return person;
    }

    direct_scalars make_scalars() {
        direct_scalars doc{};
        doc.i32 = -2;
        doc.u32 = 200;
        doc.s32 = -500;
        doc.i64 = -2;
        doc.flag = true;
        doc.f32 = 200;
        doc.sf32 = -200;
        doc.f = 25.5f;
        doc.f64 = 200;
        doc.sf64 = -200;
        doc.d = 25.4;
        doc.s = "hello";
        doc.b = "bytes";
        return doc;
    }

    direct_numbers make_numbers() {
        direct_numbers doc{};
        for (int i = 0; i < 2000; ++i) {
            doc.e.push_back(i);
        }
        return doc;
    }

    const direct_person &person_value() {
        static const direct_person value = make_person();
        return value;
    }

    const direct_scalars &scalars_value() {
        static const direct_scalars value = make_scalars();
        return value;
    }

    const direct_numbers &numbers_value() {
        static const direct_numbers value = make_numbers();
        return value;
    }

    const byte_buffer &person_bytes() {
        static const byte_buffer bytes = protobuf::encode_direct(person_value());
        return bytes;
    }

    const byte_buffer &scalars_bytes() {
        static const byte_buffer bytes = protobuf::encode_direct(scalars_value());
        return bytes;
    }

    const byte_buffer &numbers_bytes() {
        static const byte_buffer bytes = protobuf::encode_direct(numbers_value());
        return bytes;
    }
}

static void benchmark_direct_encode_small(benchmark::State &state) {
    const direct_person &value = person_value();
    for (auto _ : state) {
        auto bytes = protobuf::encode_direct(value);
        benchmark::DoNotOptimize(bytes);
    }
}

static void benchmark_direct_decode_small(benchmark::State &state) {
    const byte_buffer &bytes = person_bytes();
    for (auto _ : state) {
        auto value = protobuf::decode_direct<direct_person>(bytes.data(), bytes.size());
        benchmark::DoNotOptimize(value);
    }
}

static void benchmark_direct_encode_scalars(benchmark::State &state) {
    const direct_scalars &value = scalars_value();
    for (auto _ : state) {
        auto bytes = protobuf::encode_direct(value);
        benchmark::DoNotOptimize(bytes);
    }
}

static void benchmark_direct_decode_scalars(benchmark::State &state) {
    const byte_buffer &bytes = scalars_bytes();
    for (auto _ : state) {
        auto value = protobuf::decode_direct<direct_scalars>(bytes.data(), bytes.size());
        benchmark::DoNotOptimize(value);
    }
}

static void benchmark_direct_encode_large_repeated(benchmark::State &state) {
    const direct_numbers &value = numbers_value();
    for (auto _ : state) {
        auto bytes = protobuf::encode_direct(value);
        benchmark::DoNotOptimize(bytes);
    }
}

static void benchmark_direct_decode_large_repeated(benchmark::State &state) {
    const byte_buffer &bytes = numbers_bytes();
    for (auto _ : state) {
        auto value = protobuf::decode_direct<direct_numbers>(bytes.data(), bytes.size());
        benchmark::DoNotOptimize(value);
    }
}

BENCHMARK(benchmark_direct_encode_small);
BENCHMARK(benchmark_direct_decode_small);
BENCHMARK(benchmark_direct_encode_scalars);
BENCHMARK(benchmark_direct_decode_scalars);
BENCHMARK(benchmark_direct_encode_large_repeated);
BENCHMARK(benchmark_direct_decode_large_repeated);
