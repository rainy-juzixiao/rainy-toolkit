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
#ifndef RAINY_FOUNDATION_CONTAINER_SINGLETON_HPP
#define RAINY_FOUNDATION_CONTAINER_SINGLETON_HPP

#include <rainy/core/platform.hpp>
#include <rainy/core/typeinfo.hpp>
#include <rainy/foundation/concurrency/mutex.hpp>
#include <rainy/foundation/container/implements/singleton/registry.hpp>

namespace rainy::foundation::container {
    /**
     * \lang english
     * @brief A singleton accessor that yields one instance per process.
     *
     * @brief Unlike a function-local static, whose definition may be duplicated
     * across dynamic libraries, singleton stores its instance in a shared
     * registry keyed by the type. All modules and libraries that link against
     * the toolkit therefore observe the same instance. Storage for the instance
     * is served from the registry's memory pool, and construction happens once,
     * under the registry lock, on first use.
     *
     * @tparam Ty The type whose single instance is exposed.
     *
     * \lang simp-chinese
     * @brief 每个进程仅产生一个实例的单例访问器。
     *
     * @brief 与函数局部静态量（其定义可能在多个动态库中重复）不同，singleton 把实例
     * 存放在按类型索引的共享注册表中。所有链接该工具包的模块与库因此观察到同一实例。
     * 实例的存储由注册表的内存池提供，构造在首次使用时于注册表锁下进行且仅一次。
     *
     * @tparam Ty 要暴露唯一实例的类型。
     */
    template <typename Ty>
    class singleton {
    public:
        using value_type = Ty;

        singleton() = delete;

        singleton(const singleton &) = delete;

        singleton(singleton &&) = delete;

        singleton &operator=(const singleton &) = delete;

        singleton &operator=(singleton &&) = delete;

        /**
         * \lang english
         * @brief Returns the single instance, constructing it on first use.
         *
         * @tparam Args The constructor argument types.
         * @param args Arguments forwarded to the instance on first construction.
         * @return A reference to the single instance.
         *
         * \lang simp-chinese
         * @brief 返回唯一实例，首次使用时构造。
         *
         * @tparam Args 构造函数参数类型。
         * @param args 首次构造时转发给实例的参数。
         * @return 唯一实例的引用。
         */
        template <typename... Args>
        RAINY_NODISCARD static Ty &instance(Args &&...args) {
            implements::singleton_registry &registry = implements::singleton_registry::instance();
            const core::typeinfo &type = rainy_typeid(Ty);
            const std::size_t hash = type.hash_code();
            concurrency::lock_guard<implements::singleton_registry> guard(registry);
            if (void *found = registry.find_locked(hash, &type); found != nullptr) {
                return *static_cast<Ty *>(found);
            }
            void *block = registry.allocate_locked(sizeof(Ty), alignof(Ty));
            Ty *value = ::new (block) Ty(utility::forward<Args>(args)...);
            registry.record_locked(hash, &type, value, &destroy_at);
            return *value;
        }

    private:
        static void destroy_at(void *block) noexcept {
            static_cast<Ty *>(block)->~Ty();
        }
    };
}

#endif
