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
#include <rainy/core/typeinfo.hpp>
#include <unordered_map>
#include <unordered_set>
#include <functional>

namespace rainy::core::implements {
    using type_info_map_t = std::unordered_map<std::size_t, type_upcast_info>;

    namespace {
        RAINY_INLINE type_info_map_t &get_type_info_map() {
            static type_info_map_t instance;
            return instance;
        }
    }

    using direct_bases_map_t = std::unordered_map<std::size_t, std::vector<std::pair<std::size_t, converter_func>>>;
    using upcast_closure_map_t = std::unordered_map<conversion_key, converter_func, conversion_key_hash>;

    namespace {
        RAINY_INLINE direct_bases_map_t &get_direct_bases_map() {
            static direct_bases_map_t instance;
            return instance;
        }
    }

    namespace {
        RAINY_INLINE upcast_closure_map_t &get_upcast_closure_map() {
            static upcast_closure_map_t instance;
            return instance;
        }
    }

    bool is_upcast_reachable(std::size_t from_hash, std::size_t to_hash) {
        if (from_hash == to_hash) {
            return true;
        }
        return get_upcast_closure_map().count(conversion_key{from_hash, to_hash}) > 0;
    }

    void register_direct_base(const std::size_t derived, const std::size_t base, converter_func fn) {
        auto &direct = get_direct_bases_map();
        auto &closure = get_upcast_closure_map();
        direct[derived].emplace_back(base, fn);
        closure.try_emplace(conversion_key{derived, base}, fn);
        auto it = direct.find(base);
        if (it != direct.end()) {
            for (const auto &[key, existing_fn]: closure) {
                if (key.source_hash != base) {
                    continue;
                }
                converter_func combined = [fn, existing_fn](void *ptr) -> void * {
                    void *mid = fn(ptr);
                    if (!mid) {
                        return nullptr;
                    }
                    return existing_fn(mid);
                };
                closure.try_emplace(conversion_key{derived, key.target_hash}, combined);
            }
        }
        for (const auto &[key, existing_fn]: closure) {
            if (key.target_hash != derived) {
                continue;
            }
            const std::size_t sub = key.source_hash;
            const converter_func &to_derived = existing_fn;
            converter_func sub_to_base = [to_derived, fn](void *ptr) -> void * {
                void *mid = to_derived(ptr);
                if (!mid) {
                    return nullptr;
                }
                return fn(mid);
            };
            closure.try_emplace(conversion_key{sub, base}, sub_to_base);
            for (const auto &[key2, base_ancestor_fn]: closure) {
                if (key2.source_hash != base) {
                    continue;
                }
                converter_func sub_to_ancestor = [to_derived, fn, base_ancestor_fn](void *ptr) -> void * {
                    void *mid = to_derived(ptr);
                    if (!mid) {
                        return nullptr;
                    }
                    void *mid2 = fn(mid);
                    if (!mid2) {
                        return nullptr;
                    }
                    return base_ancestor_fn(mid2);
                };
                closure.try_emplace(conversion_key{sub, key2.target_hash}, sub_to_ancestor);
            }
        }
    }

    void *apply_offset(void *ptr, const rainy::core::typeinfo &source, const rainy::core::typeinfo &target) {
        if (!ptr || source.hash_code() == target.hash_code()) {
            return ptr;
        }
        const auto &closure = get_upcast_closure_map();
        const auto it = closure.find(conversion_key{source.hash_code(), target.hash_code()});
        if (it != closure.end()) {
            return it->second(ptr);
        }
        return nullptr;
    }
}
