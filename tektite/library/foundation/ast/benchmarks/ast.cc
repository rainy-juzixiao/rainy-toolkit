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
#include <memory_resource>
#include <rainy/foundation/ast/ast_dump.hpp>
#include <rainy/foundation/ast/node.hpp>
#include <rainy/foundation/ast/node_allocator.hpp>
#include <rainy/foundation/ast/visitor.hpp>

using namespace rainy::foundation::ast;

namespace {
    struct counting_visitor {
        std::size_t visited = 0;

        void visit(ast_node *) {
            ++visited;
        }
    };

    ast_node *build_chain(const std::size_t depth, std::pmr::memory_resource *resource, node_allocator<ast_node> &allocator) {
        ast_node *root = allocator.create<ast_node>(node_kind::object, resource);
        ast_node *current = root;
        for (std::size_t i = 0; i < depth; ++i) {
            ast_node *child = allocator.create<ast_node>(node_kind::array, resource);
            current->add_child(child);
            current = child;
        }
        return root;
    }
}

static void benchmark_ast_allocate(benchmark::State &state) {
    const auto depth = static_cast<std::size_t>(state.range(0));
    for (auto _: state) {
        std::pmr::monotonic_buffer_resource resource;
        node_allocator<ast_node> allocator{&resource};
        ast_node *root = build_chain(depth, &resource, allocator);
        benchmark::DoNotOptimize(root);
    }
}

static void benchmark_ast_traverse(benchmark::State &state) {
    const auto depth = static_cast<std::size_t>(state.range(0));
    std::pmr::monotonic_buffer_resource resource;
    node_allocator<ast_node> allocator{&resource};
    ast_node *root = build_chain(depth, &resource, allocator);
    for (auto _: state) {
        counting_visitor visitor;
        traverse(visitor, root);
        benchmark::DoNotOptimize(visitor.visited);
    }
}

static void benchmark_ast_dump(benchmark::State &state) {
    const auto depth = static_cast<std::size_t>(state.range(0));
    std::pmr::monotonic_buffer_resource resource;
    node_allocator<ast_node> allocator{&resource};
    ast_node *root = build_chain(depth, &resource, allocator);
    dump_options options;
    options.annotations = false;
    for (auto _: state) {
        auto text = dump_to_string(root, options);
        benchmark::DoNotOptimize(text);
    }
}

BENCHMARK(benchmark_ast_allocate)->Arg(16)->Arg(256)->Arg(4096);
BENCHMARK(benchmark_ast_traverse)->Arg(16)->Arg(256)->Arg(4096);
BENCHMARK(benchmark_ast_dump)->Arg(16)->Arg(256)->Arg(4096);
