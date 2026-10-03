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

#include <string>

using namespace rainy::foundation::willow;
using namespace rainy::foundation::willow::protobuf;

namespace {
    struct bench_phone {
        static constexpr rainy::core::container::tuple<field<"number", proto::string, 1>> protobuf_fields{};
    };

    struct bench_person {
        static constexpr rainy::core::container::tuple<field<"name", proto::string, 1>, field<"age", proto::int32, 2>,
                                                       field<"tags", proto::repeated<proto::string>, 3>,
                                                       field<"phone", proto::message<bench_phone>, 4>>
            protobuf_fields{};
    };

    struct bench_scalars {
        static constexpr rainy::core::container::tuple<
            field<"i32", proto::int32, 1>, field<"u32", proto::uint32, 2>, field<"s32", proto::sint32, 3>,
            field<"i64", proto::int64, 4>, field<"flag", proto::boolean, 5>, field<"f32", proto::fixed32, 6>,
            field<"sf32", proto::sfixed32, 7>, field<"f", proto::floating, 8>, field<"f64", proto::fixed64, 9>,
            field<"sf64", proto::sfixed64, 10>, field<"d", proto::doubling, 11>, field<"s", proto::string, 12>,
            field<"b", proto::bytes, 13>>
            protobuf_fields{};
    };

    struct bench_numbers {
        static constexpr rainy::core::container::tuple<field<"e", proto::repeated<proto::int32>, 1>> protobuf_fields{};
    };

    document make_small_person() {
        document person(document_type::object);
        person["name"] = document(std::string("testing"));
        person["age"] = document(32);
        document tags(document_type::array);
        tags.push_back(document(std::string("alpha")));
        tags.push_back(document(std::string("beta")));
        person["tags"] = tags;
        document phone(document_type::object);
        phone["number"] = document(std::string("1234"));
        person["phone"] = phone;
        return person;
    }

    document make_scalars() {
        document doc(document_type::object);
        doc["i32"] = document(-2);
        doc["u32"] = document(200);
        doc["s32"] = document(-500);
        doc["i64"] = document(-2);
        doc["flag"] = document(true);
        doc["f32"] = document(200);
        doc["sf32"] = document(-200);
        doc["f"] = document(25.5f);
        doc["f64"] = document(200);
        doc["sf64"] = document(-200);
        doc["d"] = document(25.4);
        doc["s"] = document(std::string("hello"));
        doc["b"] = document(std::string("bytes"));
        return doc;
    }

    document make_large_numbers() {
        document doc(document_type::object);
        document array(document_type::array);
        for (int i = 0; i < 2000; ++i) {
            array.push_back(document(i));
        }
        doc["e"] = array;
        return doc;
    }

    const document &small_person_doc() {
        static const document doc = make_small_person();
        return doc;
    }

    const document &scalars_doc() {
        static const document doc = make_scalars();
        return doc;
    }

    const document &large_numbers_doc() {
        static const document doc = make_large_numbers();
        return doc;
    }

    const byte_buffer &small_person_bytes() {
        static const byte_buffer bytes = protobuf::encode<bench_person>(small_person_doc());
        return bytes;
    }

    const byte_buffer &scalars_bytes() {
        static const byte_buffer bytes = protobuf::encode<bench_scalars>(scalars_doc());
        return bytes;
    }

    const byte_buffer &large_numbers_bytes() {
        static const byte_buffer bytes = protobuf::encode<bench_numbers>(large_numbers_doc());
        return bytes;
    }
}

static void benchmark_protobuf_encode_small(benchmark::State &state) {
    const document &doc = small_person_doc();
    for (auto _ : state) {
        auto bytes = protobuf::encode<bench_person>(doc);
        benchmark::DoNotOptimize(bytes);
    }
}

static void benchmark_protobuf_decode_small(benchmark::State &state) {
    const byte_buffer &bytes = small_person_bytes();
    for (auto _ : state) {
        auto doc = protobuf::decode<bench_person>(bytes.data(), bytes.size());
        benchmark::DoNotOptimize(doc);
    }
}

static void benchmark_protobuf_encode_scalars(benchmark::State &state) {
    const document &doc = scalars_doc();
    for (auto _ : state) {
        auto bytes = protobuf::encode<bench_scalars>(doc);
        benchmark::DoNotOptimize(bytes);
    }
}

static void benchmark_protobuf_decode_scalars(benchmark::State &state) {
    const byte_buffer &bytes = scalars_bytes();
    for (auto _ : state) {
        auto doc = protobuf::decode<bench_scalars>(bytes.data(), bytes.size());
        benchmark::DoNotOptimize(doc);
    }
}

static void benchmark_protobuf_encode_large_repeated(benchmark::State &state) {
    const document &doc = large_numbers_doc();
    for (auto _ : state) {
        auto bytes = protobuf::encode<bench_numbers>(doc);
        benchmark::DoNotOptimize(bytes);
    }
}

static void benchmark_protobuf_decode_large_repeated(benchmark::State &state) {
    const byte_buffer &bytes = large_numbers_bytes();
    for (auto _ : state) {
        auto doc = protobuf::decode<bench_numbers>(bytes.data(), bytes.size());
        benchmark::DoNotOptimize(doc);
    }
}

BENCHMARK(benchmark_protobuf_encode_small);
BENCHMARK(benchmark_protobuf_decode_small);
BENCHMARK(benchmark_protobuf_encode_scalars);
BENCHMARK(benchmark_protobuf_decode_scalars);
BENCHMARK(benchmark_protobuf_encode_large_repeated);
BENCHMARK(benchmark_protobuf_decode_large_repeated);
