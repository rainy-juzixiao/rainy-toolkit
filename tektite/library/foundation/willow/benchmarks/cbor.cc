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
#include <rainy/foundation/willow/cbor.hpp>

#include <string>

using namespace rainy::foundation::willow;
using namespace rainy::foundation::willow::cbor;

namespace {
    document make_small_record() {
        document record(document_type::object);
        record["name"] = document(std::string("testing"));
        record["age"] = document(32);
        document tags(document_type::array);
        tags.push_back(document(std::string("alpha")));
        tags.push_back(document(std::string("beta")));
        record["tags"] = tags;
        document phone(document_type::object);
        phone["number"] = document(std::string("1234"));
        record["phone"] = phone;
        return record;
    }

    document make_scalars() {
        document doc(document_type::object);
        doc["neg"] = document(-2);
        doc["small"] = document(200);
        doc["mid"] = document(-500);
        doc["big"] = document(70000);
        doc["flag"] = document(true);
        doc["ratio"] = document(25.5);
        doc["precise"] = document(25.4);
        doc["text"] = document(std::string("hello"));
        return doc;
    }

    document make_large_array() {
        document doc(document_type::array);
        for (int i = 0; i < 2000; ++i) {
            doc.push_back(document(i));
        }
        return doc;
    }

    document make_many_keys() {
        document doc(document_type::object);
        for (int i = 0; i < 500; ++i) {
            doc["key" + std::to_string(i)] = document(std::string("value") + std::to_string(i));
        }
        return doc;
    }

    const document &small_record_doc() {
        static const document doc = make_small_record();
        return doc;
    }

    const document &scalars_doc() {
        static const document doc = make_scalars();
        return doc;
    }

    const document &large_array_doc() {
        static const document doc = make_large_array();
        return doc;
    }

    const document &many_keys_doc() {
        static const document doc = make_many_keys();
        return doc;
    }

    const byte_buffer &small_record_bytes() {
        static const byte_buffer bytes = cbor::encode(small_record_doc());
        return bytes;
    }

    const byte_buffer &scalars_bytes() {
        static const byte_buffer bytes = cbor::encode(scalars_doc());
        return bytes;
    }

    const byte_buffer &large_array_bytes() {
        static const byte_buffer bytes = cbor::encode(large_array_doc());
        return bytes;
    }

    const byte_buffer &many_keys_bytes() {
        static const byte_buffer bytes = cbor::encode(many_keys_doc());
        return bytes;
    }
}

static void benchmark_cbor_encode_small(benchmark::State &state) {
    const document &doc = small_record_doc();
    for (auto _: state) {
        auto bytes = cbor::encode(doc);
        benchmark::DoNotOptimize(bytes);
    }
}

static void benchmark_cbor_decode_small(benchmark::State &state) {
    const byte_buffer &bytes = small_record_bytes();
    for (auto _: state) {
        auto doc = cbor::decode(bytes);
        benchmark::DoNotOptimize(doc);
    }
}

static void benchmark_cbor_encode_scalars(benchmark::State &state) {
    const document &doc = scalars_doc();
    for (auto _: state) {
        auto bytes = cbor::encode(doc);
        benchmark::DoNotOptimize(bytes);
    }
}

static void benchmark_cbor_decode_scalars(benchmark::State &state) {
    const byte_buffer &bytes = scalars_bytes();
    for (auto _: state) {
        auto doc = cbor::decode(bytes);
        benchmark::DoNotOptimize(doc);
    }
}

static void benchmark_cbor_encode_large_array(benchmark::State &state) {
    const document &doc = large_array_doc();
    for (auto _: state) {
        auto bytes = cbor::encode(doc);
        benchmark::DoNotOptimize(bytes);
    }
}

static void benchmark_cbor_decode_large_array(benchmark::State &state) {
    const byte_buffer &bytes = large_array_bytes();
    for (auto _: state) {
        auto doc = cbor::decode(bytes);
        benchmark::DoNotOptimize(doc);
    }
}

static void benchmark_cbor_encode_many_keys(benchmark::State &state) {
    const document &doc = many_keys_doc();
    for (auto _: state) {
        auto bytes = cbor::encode(doc);
        benchmark::DoNotOptimize(bytes);
    }
}

static void benchmark_cbor_decode_many_keys(benchmark::State &state) {
    const byte_buffer &bytes = many_keys_bytes();
    for (auto _: state) {
        auto doc = cbor::decode(bytes);
        benchmark::DoNotOptimize(doc);
    }
}

BENCHMARK(benchmark_cbor_encode_small);
BENCHMARK(benchmark_cbor_decode_small);
BENCHMARK(benchmark_cbor_encode_scalars);
BENCHMARK(benchmark_cbor_decode_scalars);
BENCHMARK(benchmark_cbor_encode_large_array);
BENCHMARK(benchmark_cbor_decode_large_array);
BENCHMARK(benchmark_cbor_encode_many_keys);
BENCHMARK(benchmark_cbor_decode_many_keys);
