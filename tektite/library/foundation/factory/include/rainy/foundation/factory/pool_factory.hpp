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
#ifndef RAINY_FOUNDATION_FACTORY_POOL_FACTORY_HPP
#define RAINY_FOUNDATION_FACTORY_POOL_FACTORY_HPP
#include <rainy/foundation/concurrency/shared_mutex.hpp>
#include <rainy/foundation/factory/fact.hpp>

namespace rainy::foundation::fact {
    template <typename Prod, template <typename Key, typename Mapped, typename...> typename Map, typename ProdKey,
              template <typename FactoryStorageType> typename Alloc>
    class pool_factory;

    namespace implements {
        template <typename Factory>
        class pool_lease {
        public:
            using product = typename Factory::product;
            using product_key = typename Factory::product_key;

            pool_lease() noexcept = default;

            pool_lease(Factory *owner, const product_key &id, product *instance) :
                owner_{owner}, id_{id}, instance_{instance} {
            }

            pool_lease(pool_lease &&right) noexcept :
                owner_{right.owner_}, id_{utility::move(right.id_)}, instance_{right.instance_} {
                right.owner_ = nullptr;
                right.instance_ = nullptr;
            }

            pool_lease &operator=(pool_lease &&right) noexcept {
                if (this != &right) {
                    release();
                    owner_ = right.owner_;
                    id_ = utility::move(right.id_);
                    instance_ = right.instance_;
                    right.owner_ = nullptr;
                    right.instance_ = nullptr;
                }
                return *this;
            }

            ~pool_lease() {
                release();
            }

            RAINY_NODISCARD product *get() const noexcept {
                return instance_;
            }

            RAINY_NODISCARD product *operator->() const noexcept {
                return instance_;
            }

            RAINY_NODISCARD product &operator*() const noexcept {
                return *instance_;
            }

            RAINY_NODISCARD explicit operator bool() const noexcept {
                return instance_ != nullptr;
            }

            void release() noexcept {
                if (instance_) {
                    owner_->recycle(id_, instance_);
                    owner_ = nullptr;
                    instance_ = nullptr;
                }
            }

            pool_lease(const pool_lease &) = delete;
            pool_lease &operator=(const pool_lease &) = delete;

        private:
            Factory *owner_ = nullptr;
            product_key id_;
            product *instance_ = nullptr;
        };
    }

    /**
     * \lang english
     * @brief An object-pool factory that creates products on demand and recycles them into an idle pool.
     *
     * Products are created through registered creators keyed by product_key. When a lease is
     * released, the product is returned to the pool and reused by later acquires of the same key,
     * avoiding repeated construction and destruction. If the product type provides a public
     * `reset()` member, it is invoked when a pooled instance is reused; otherwise the product is
     * returned as-is and the caller is responsible for leaving it reusable.
     *
     * @tparam Prod The product type.
     * @tparam Map The associative container template used for storage.
     * @tparam ProdKey The key type, defaults to core::text::hashed_string.
     * @tparam Alloc The allocator template for storage.
     *
     * \lang simp-chinese
     * @brief 对象池工厂：按需创建产品并在租约释放时回收至空闲池。
     *
     * 产品通过以 product_key 为键注册的创建器创建。租约释放后，产品回到池中，后续对同一键的
     * acquire 直接复用，避免反复构造与析构。若产品类型提供公有的 reset() 成员，复用时会调用
     * 它；否则产品按原样回池，调用方需自行保证其可复用。
     *
     * @tparam Prod 产品类型。
     * @tparam Map 用于存储的关联容器模板。
     * @tparam ProdKey 键类型，默认为 core::text::hashed_string。
     * @tparam Alloc 存储使用的分配器模板。
     */
    template <typename Prod, template <typename Key, typename Mapped, typename...> typename Map,
              typename ProdKey = core::text::hashed_string,
              template <typename FactoryStorageType> typename Alloc = core::memory::allocator>
    class pool_factory {
        using creater = functional::delegate<Prod *()>;
        using value_type = typename Map<ProdKey, creater>::value_type;
        using allocator_type = Alloc<value_type>;
        using map = typename type_traits::extras::templates::replace_last_parameter<Map<ProdKey, creater>, allocator_type>::type;
        using idle_vector = core::collections::vector<Prod *, Alloc<Prod *>>;

    public:
        using product = Prod;
        using product_key = ProdKey;
        using lease = implements::pool_lease<pool_factory>;
        using size_type = typename map::size_type;

        pool_factory() = default;

        explicit pool_factory(const allocator_type &allocator) : storage_{allocator} {
        }

        pool_factory(const pool_factory &) = delete;
        pool_factory &operator=(const pool_factory &) = delete;

        template <typename Producer>
        pool_factory &emplace(const product_key &id, Producer &&producer) {
            concurrency::lock_guard<concurrency::shared_mutex> lock{mtx_};
            storage_.emplace(id, utility::forward<Producer>(producer));
            return *this;
        }

        bool contains(const product_key &id) const {
            concurrency::shared_lock<concurrency::shared_mutex> lock{mtx_};
            return storage_.find(id) != storage_.end();
        }

        /**
         * \lang english
         * @brief Acquires a product: reuses a pooled instance or creates a new one.
         *
         * Returns an empty lease when no creator is registered under the key.
         *
         * \lang simp-chinese
         * @brief 获取产品：优先复用池中实例，否则创建新实例。
         *
         * 若该键未注册创建器，则返回空租约。
         */
        lease acquire(const product_key &id) {
            product *instance = nullptr;
            {
                concurrency::lock_guard<concurrency::shared_mutex> lock{mtx_};
                if (const auto it = storage_.find(id); it != storage_.end()) {
                    auto &idle = idle_pool_[id];
                    if (!idle.empty()) {
                        instance = idle.back();
                        idle.pop_back();
                        reset_if_available(instance);
                    } else {
                        instance = it->second();
                    }
                }
            }
            return lease{this, id, instance};
        }

        /**
         * \lang english
         * @brief Number of instances currently pooled and idle for the given key.
         *
         * \lang simp-chinese
         * @brief 指定键当前在池中空闲的实例数量。
         */
        size_type pooled_size(const product_key &id) {
            concurrency::lock_guard<concurrency::shared_mutex> lock{mtx_};
            const auto it = idle_pool_.find(id);
            return it == idle_pool_.end() ? size_type{0} : it->second.size();
        }

        /**
         * \lang english
         * @brief Destroys all idle instances for the given key and frees them.
         *
         * \lang simp-chinese
         * @brief 销毁指定键的所有空闲实例并释放内存。
         */
        void shrink(const product_key &id) {
            concurrency::lock_guard<concurrency::shared_mutex> lock{mtx_};
            const auto it = idle_pool_.find(id);
            if (it == idle_pool_.end()) {
                return;
            }
            for (product *instance : it->second) {
                delete instance;
            }
            idle_pool_.erase(it);
        }

        void clear() {
            concurrency::lock_guard<concurrency::shared_mutex> lock{mtx_};
            for (auto &entry : idle_pool_) {
                for (product *instance : entry.second) {
                    delete instance;
                }
            }
            idle_pool_.clear();
            storage_.clear();
        }

    private:
        friend lease;

        void recycle(const product_key &id, product *instance) noexcept {
            concurrency::lock_guard<concurrency::shared_mutex> lock{mtx_};
            idle_pool_[id].push_back(instance);
        }

        template <typename Ty = product>
        static rain_fn reset_if_available(Ty *instance) noexcept -> decltype(void(instance->reset())) {
            instance->reset();
        }

        static void reset_if_available(...) noexcept {
        }

        map storage_;
        Map<product_key, idle_vector> idle_pool_;
        mutable concurrency::shared_mutex mtx_;
    };
}

#endif
