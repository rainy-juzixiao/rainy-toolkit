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
    struct bench_phone : add_unknown_fields<bench_phone>, add_reflection<bench_phone>, add_descriptor<bench_phone> {
        std::string number{};

        static constexpr rainy::core::container::tuple<member_field<"number", proto::string, 1, &bench_phone::number>>
            protobuf_fields{};
    };

    struct bench_person : add_unknown_fields<bench_person>, add_reflection<bench_person>, add_descriptor<bench_person> {
        std::string name{};
        std::int32_t age{};
        std::vector<std::string> tags{};
        bench_phone phone{};

        static constexpr rainy::core::container::tuple<member_field<"name", proto::string, 1, &bench_person::name>,
                                                       member_field<"age", proto::int32, 2, &bench_person::age>,
                                                       member_field<"tags", proto::repeated<proto::string>, 3, &bench_person::tags>,
                                                       member_field<"phone", proto::message<bench_phone>, 4, &bench_person::phone>>
            protobuf_fields{};
    };

    struct bench_scalars : add_unknown_fields<bench_scalars>, add_reflection<bench_scalars>, add_descriptor<bench_scalars> {
        std::int32_t i32{};
        std::uint32_t u32{};
        std::int64_t i64{};
        bool flag{};
        float f{};
        double d{};
        std::string s{};

        static constexpr rainy::core::container::tuple<
            member_field<"i32", proto::int32, 1, &bench_scalars::i32>, member_field<"u32", proto::uint32, 2, &bench_scalars::u32>,
            member_field<"i64", proto::int64, 3, &bench_scalars::i64>, member_field<"flag", proto::boolean, 4, &bench_scalars::flag>,
            member_field<"f", proto::floating, 5, &bench_scalars::f>, member_field<"d", proto::doubling, 6, &bench_scalars::d>,
            member_field<"s", proto::string, 7, &bench_scalars::s>>
            protobuf_fields{};
    };

    bench_person make_person() {
        bench_person p{};
        p.name = "testing";
        p.age = 32;
        p.tags = {"alpha", "beta"};
        p.phone.number = "1234";
        return p;
    }

    const bench_person &person_value() {
        static const bench_person value = make_person();
        return value;
    }

    const byte_buffer &person_bytes_with_unknown() {
        static const byte_buffer bytes = []() {
            bench_person p = person_value();
            byte_buffer out = encode_direct(p);
            out.push_back(0x78);
            out.push_back(0x01);
            return out;
        }();
        return bytes;
    }

    struct bench_plain {
        std::string name{};
        std::int32_t age{};
        std::vector<std::string> tags{};
        bench_phone phone{};

        static constexpr rainy::core::container::tuple<member_field<"name", proto::string, 1, &bench_plain::name>,
                                                       member_field<"age", proto::int32, 2, &bench_plain::age>,
                                                       member_field<"tags", proto::repeated<proto::string>, 3, &bench_plain::tags>,
                                                       member_field<"phone", proto::message<bench_phone>, 4, &bench_plain::phone>>
            protobuf_fields{};
    };
}

static void benchmark_descriptor_first_instance(benchmark::State &state) {
    for (auto _: state) {
        auto &d = descriptor<bench_person>::instance();
        benchmark::DoNotOptimize(d);
    }
}
BENCHMARK(benchmark_descriptor_first_instance);

static void benchmark_descriptor_find_field_by_name_steady(benchmark::State &state) {
    auto &d = descriptor<bench_person>::instance();
    for (auto _: state) {
        const auto *fd = d.find_field_by_name("age");
        benchmark::DoNotOptimize(fd);
    }
}
BENCHMARK(benchmark_descriptor_find_field_by_name_steady);

static void benchmark_descriptor_find_field_by_number_steady(benchmark::State &state) {
    auto &d = descriptor<bench_person>::instance();
    for (auto _: state) {
        const auto *fd = d.find_field_by_number(2);
        benchmark::DoNotOptimize(fd);
    }
}
BENCHMARK(benchmark_descriptor_find_field_by_number_steady);

static void benchmark_reflection_get(benchmark::State &state) {
    const bench_person &p = person_value();
    for (auto _: state) {
        auto v = p.get("age");
        benchmark::DoNotOptimize(v);
    }
}
BENCHMARK(benchmark_reflection_get);

static void benchmark_reflection_get_by_number(benchmark::State &state) {
    const bench_person &p = person_value();
    for (auto _: state) {
        auto v = p.get(2);
        benchmark::DoNotOptimize(v);
    }
}
BENCHMARK(benchmark_reflection_get_by_number);

static void benchmark_reflection_set(benchmark::State &state) {
    bench_person p = person_value();
    for (auto _: state) {
        p.set("age", std::int64_t{30});
        benchmark::DoNotOptimize(p);
    }
}
BENCHMARK(benchmark_reflection_set);

static void benchmark_reflection_has(benchmark::State &state) {
    const bench_person &p = person_value();
    for (auto _: state) {
        bool present = p.has("name");
        benchmark::DoNotOptimize(present);
    }
}
BENCHMARK(benchmark_reflection_has);

static void benchmark_reflection_clear(benchmark::State &state) {
    for (auto _: state) {
        bench_person p = person_value();
        p.clear("tags");
        benchmark::DoNotOptimize(p);
    }
}
BENCHMARK(benchmark_reflection_clear);

static void benchmark_unknown_decode_with_capture(benchmark::State &state) {
    const byte_buffer &bytes = person_bytes_with_unknown();
    for (auto _: state) {
        bench_person p = decode_direct<bench_person>(bytes.data(), bytes.size());
        benchmark::DoNotOptimize(p);
    }
}
BENCHMARK(benchmark_unknown_decode_with_capture);

static void benchmark_unknown_roundtrip(benchmark::State &state) {
    const byte_buffer &bytes = person_bytes_with_unknown();
    for (auto _: state) {
        bench_person p = decode_direct<bench_person>(bytes.data(), bytes.size());
        byte_buffer out = encode_direct(p);
        benchmark::DoNotOptimize(out);
    }
}
BENCHMARK(benchmark_unknown_roundtrip);

static void benchmark_unknown_empty_path_zero_overhead(benchmark::State &state) {
    bench_plain p{};
    p.name = "testing";
    p.age = 32;
    p.tags = {"alpha", "beta"};
    p.phone.number = "1234";
    byte_buffer bytes = encode_direct(p);
    for (auto _: state) {
        bench_plain back = decode_direct<bench_plain>(bytes.data(), bytes.size());
        benchmark::DoNotOptimize(back);
    }
}
BENCHMARK(benchmark_unknown_empty_path_zero_overhead);
