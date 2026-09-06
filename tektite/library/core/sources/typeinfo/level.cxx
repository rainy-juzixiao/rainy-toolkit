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
#include <cstddef>
#include <cstdlib>
#include <functional>
#include <rainy/core/container/pair.hpp>
#include <rainy/core/functional/delegate.hpp>
#include <rainy/core/typeinfo.hpp>
#include <rainy/core/utility/hash.hpp>

namespace rainy::core::implements {
    template <typename Key, typename Value, typename Hasher = conversion_key_hash>
    struct hash_node {
        Key key;
        Value value;
        hash_node *next;
    };

    template <typename Key, typename Value, typename Hasher = conversion_key_hash>
    class hash_map {
    public:
        hash_map() : buckets(nullptr), bucket_count(0), size(0), hasher() {
        }

        explicit hash_map(const Hasher &hasher) : buckets(nullptr), bucket_count(0), size(0), hasher(hasher) {
        }

        ~hash_map() {
            clear();
            std::free(buckets);
        }

        void emplace(const Key &key, const Value &value) {
            if (bucket_count == 0 || size >= bucket_count * 0.75) {
                rehash();
            }

            std::size_t idx = hasher(key) % bucket_count;
            for (hash_node<Key, Value> *node = buckets[idx]; node; node = node->next) {
                if (node->key == key) {
                    node->value = value;
                    return;
                }
            }

            auto *new_node = new hash_node<Key, Value>{key, value, buckets[idx]};
            buckets[idx] = new_node;
            ++size;
        }

        Value *find(const Key &key) {
            if (bucket_count == 0) {
                return nullptr;
            }
            std::size_t idx = hasher(key) % bucket_count;
            hash_node<Key, Value> *node = buckets[idx];
            while (node) {
                if (node->key == key) {
                    return &node->value;
                }
                node = node->next;
            }
            return nullptr;
        }

        bool contains(const Key &key) const {
            return const_cast<hash_map *>(this)->find(key) != nullptr;
        }

    private:
        void clear() {
            for (std::size_t i = 0; i < bucket_count; ++i) {
                hash_node<Key, Value> *node = buckets[i];
                while (node) {
                    hash_node<Key, Value> *next = node->next;
                    delete node;
                    node = next;
                }
                buckets[i] = nullptr;
            }
            size = 0;
        }

        void rehash() {
            std::size_t new_count = bucket_count == 0 ? 8 : bucket_count * 2;
            auto **new_buckets = static_cast<hash_node<Key, Value> **>(std::calloc(new_count, sizeof(hash_node<Key, Value> *)));
            if (!new_buckets) {
                std::abort();
            }

            for (std::size_t i = 0; i < bucket_count; ++i) {
                hash_node<Key, Value> *node = buckets[i];
                while (node) {
                    hash_node<Key, Value> *next = node->next;
                    std::size_t idx = hasher(node->key) % new_count;
                    node->next = new_buckets[idx];
                    new_buckets[idx] = node;
                    node = next;
                }
            }

            std::free(buckets);
            buckets = new_buckets;
            bucket_count = new_count;
        }

        hash_node<Key, Value> **buckets;
        std::size_t bucket_count;
        std::size_t size;
        Hasher hasher;
    };

    using actal_conv_fn = functional::delegate<void *(void *)>;

    using direct_bases_map_t =
        hash_map<std::size_t, collections::vector<container::pair<std::size_t, actal_conv_fn>>, utility::hash<std::size_t>>;
    using upcast_closure_map_t = hash_map<conversion_key, actal_conv_fn>;

    namespace {
        direct_bases_map_t &get_direct_bases_map() {
            static direct_bases_map_t instance;
            return instance;
        }

        upcast_closure_map_t &get_upcast_closure_map() {
            static upcast_closure_map_t instance;
            return instance;
        }
    }

    bool is_upcast_reachable(std::size_t from_hash, std::size_t to_hash) {
        if (from_hash == to_hash) {
            return true;
        }
        return get_upcast_closure_map().contains({from_hash, to_hash});
    }

    void register_direct_base(std::size_t derived, std::size_t base, converter_func fn) {
        auto &direct = get_direct_bases_map();
        auto &closure = get_upcast_closure_map();

        auto *vec = direct.find(derived);
        if (vec) {
            vec->emplace_back(base, fn);
        } else {
            collections::vector<container::pair<std::size_t, actal_conv_fn>> v;
            v.push_back({base, fn});
            direct.emplace(derived, v);
        }

        closure.emplace({derived, base}, fn);

        if (auto *base_vec = direct.find(base)) {
            for (const auto &entry: *base_vec) {
                auto combined = [fn, entry](void *ptr) -> void * {
                    void *mid = fn(ptr);
                    if (!mid) {
                        return nullptr;
                    }
                    return entry.second(mid);
                };
                closure.emplace({derived, entry.first}, combined);
            }
        }
    }

    void *apply_offset(void *ptr, const rainy::core::typeinfo &source, const rainy::core::typeinfo &target) {
        if (!ptr || source.hash_code() == target.hash_code()) {
            return ptr;
        }
        auto &closure = get_upcast_closure_map();
        auto *fn = closure.find({source.hash_code(), target.hash_code()});
        if (fn) {
            return (*fn)(ptr);
        }
        return nullptr;
    }

}
