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
#include <rainy/foundation/willow/xml.hpp>

#include <string>

using namespace rainy::foundation::willow;
using doc_string = document::string_type;

namespace {
    doc_string repeat(const std::size_t count, const char fill) {
        doc_string result;
        result.reserve(count);
        for (std::size_t i = 0; i < count; ++i) {
            result.push_back(fill);
        }
        return result;
    }

    const doc_string &small_xml() {
        static const doc_string text{"<config><name>rainy</name><port>8080</port><enabled>true</enabled></config>"};
        return text;
    }

    const doc_string &realistic_xml() {
        static const doc_string text{
            "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
            "<project name=\"tekkitte-demo\" version=\"1.0.9-beta\">\n"
            "  <!-- build configuration -->\n"
            "  <server host=\"example.com\" port=\"8080\" threads=\"16\"/>\n"
            "  <database host=\"db.internal\" port=\"5432\" user=\"rainy\" ssl=\"true\">\n"
            "    <replica host=\"replica-a.internal\" lag-ms=\"12\"/>\n"
            "    <replica host=\"replica-b.internal\" lag-ms=\"31\"/>\n"
            "  </database>\n"
            "  <thresholds memory-limit-mb=\"2048\" timeout-ms=\"30000\"/>\n"
            "  <logging severity=\"ERROR\" retries=\"3\">Unhandled exception &amp; retry</logging>\n"
            "</project>"};
        return text;
    }

    const doc_string &large_sections_xml() {
        static const doc_string text = [] {
            doc_string source{"<root>"};
            for (int s = 0; s < 200; ++s) {
                source += doc_string{"<section id=\""} + doc_string{std::to_string(s).c_str()} + doc_string{"\">"};
                for (int k = 0; k < 10; ++k) {
                    source += doc_string{"<key"} + doc_string{std::to_string(k).c_str()} + doc_string{">"};
                    source += doc_string{"value"};
                    source += doc_string{std::to_string(k).c_str()};
                    source += doc_string{"</key"} + doc_string{std::to_string(k).c_str()} + doc_string{">"};
                }
                source += doc_string{"</section>"};
            }
            source += doc_string{"</root>"};
            return source;
        }();
        return text;
    }

    const doc_string &long_text_xml() {
        static const doc_string text = [] {
            const doc_string body = repeat(240, 'x');
            doc_string source{"<root>"};
            for (int k = 0; k < 200; ++k) {
                source += doc_string{"<paragraph id=\""} + doc_string{std::to_string(k).c_str()} + doc_string{"\">"};
                source += body;
                source += doc_string{"</paragraph>"};
            }
            source += doc_string{"<blob><![CDATA["};
            for (int line = 0; line < 200; ++line) {
                source += body;
                source += doc_string{"\n"};
            }
            source += doc_string{"]]></blob>"};
            source += doc_string{"</root>"};
            return source;
        }();
        return text;
    }

    const doc_string &many_attributes_xml() {
        static const doc_string text = [] {
            doc_string source{"<root>"};
            for (int k = 0; k < 500; ++k) {
                source += doc_string{"<item a=\"1\" b=\"2\" c=\"three\" d=\"four\" e=\"five\"/>"};
            }
            source += doc_string{"</root>"};
            return source;
        }();
        return text;
    }
}

static void benchmark_xml_parse_small(benchmark::State &state) {
    for (auto _: state) {
        auto doc = xml::parse(small_xml());
        benchmark::DoNotOptimize(doc);
    }
}

static void benchmark_xml_parse_realistic(benchmark::State &state) {
    for (auto _: state) {
        auto doc = xml::parse(realistic_xml());
        benchmark::DoNotOptimize(doc);
    }
}

static void benchmark_xml_parse_large_sections(benchmark::State &state) {
    for (auto _: state) {
        auto doc = xml::parse(large_sections_xml());
        benchmark::DoNotOptimize(doc);
    }
}

static void benchmark_xml_parse_long_text(benchmark::State &state) {
    for (auto _: state) {
        auto doc = xml::parse(long_text_xml());
        benchmark::DoNotOptimize(doc);
    }
}

static void benchmark_xml_parse_many_attributes(benchmark::State &state) {
    for (auto _: state) {
        auto doc = xml::parse(many_attributes_xml());
        benchmark::DoNotOptimize(doc);
    }
}

static void benchmark_xml_dump_small(benchmark::State &state) {
    auto doc = xml::parse(small_xml());
    for (auto _: state) {
        auto text = xml::dump(doc);
        benchmark::DoNotOptimize(text);
    }
}

static void benchmark_xml_dump_realistic(benchmark::State &state) {
    auto doc = xml::parse(realistic_xml());
    for (auto _: state) {
        auto text = xml::dump(doc);
        benchmark::DoNotOptimize(text);
    }
}

static void benchmark_xml_dump_large_sections(benchmark::State &state) {
    auto doc = xml::parse(large_sections_xml());
    for (auto _: state) {
        auto text = xml::dump(doc);
        benchmark::DoNotOptimize(text);
    }
}

static void benchmark_xml_dump_long_text(benchmark::State &state) {
    auto doc = xml::parse(long_text_xml());
    for (auto _: state) {
        auto text = xml::dump(doc);
        benchmark::DoNotOptimize(text);
    }
}

BENCHMARK(benchmark_xml_parse_small);
BENCHMARK(benchmark_xml_parse_realistic);
BENCHMARK(benchmark_xml_parse_large_sections);
BENCHMARK(benchmark_xml_parse_long_text);
BENCHMARK(benchmark_xml_parse_many_attributes);
BENCHMARK(benchmark_xml_dump_small);
BENCHMARK(benchmark_xml_dump_realistic);
BENCHMARK(benchmark_xml_dump_large_sections);
BENCHMARK(benchmark_xml_dump_long_text);
