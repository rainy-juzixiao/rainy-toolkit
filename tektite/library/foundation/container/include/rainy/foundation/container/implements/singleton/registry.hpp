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
#ifndef RAINY_FOUNDATION_CONTAINER_IMPLEMENTS_SINGLETON_REGISTRY_HPP
#define RAINY_FOUNDATION_CONTAINER_IMPLEMENTS_SINGLETON_REGISTRY_HPP

#include <cstddef>
#include <rainy/core/platform.hpp>
#include <rainy/core/typeinfo.hpp>

namespace rainy::foundation::container::implements {
    /**
     * \lang english
     * @brief The process-wide singleton registry.
     *
     * @brief It owns a type-keyed table of instances and a memory pool that
     * serves the storage for them. Because its single definition lives in the
     * compiled container library, every module and dynamic library that links
     * against the toolkit resolves the same registry, so a given type has
     * exactly one instance for the whole process. All mutating operations are
     * performed under the registry's internal lock.
     *
     * \lang simp-chinese
     * @brief 进程级单例注册表。
     *
     * @brief 它持有一张按类型索引的实例表，以及一个为实例提供存储的内存池。由于其唯
     * 一定义位于已编译的 container 库中，所有链接该工具包的模块与动态库都会解析到同
     * 一注册表，因此给定类型在整个进程内只有一个实例。所有修改操作均在注册表内部锁的
     * 保护下进行。
     */
    class RAINY_TOOLKIT_API singleton_registry {
    public:
        /**
         * \lang english
         * @brief Returns the single process-wide registry.
         *
         * \lang simp-chinese
         * @brief 返回进程级唯一的注册表。
         */
        static singleton_registry &instance();

        singleton_registry(const singleton_registry &) = delete;

        singleton_registry &operator=(const singleton_registry &) = delete;

        /**
         * \lang english
         * @brief Acquires the registry lock.
         *
         * \lang simp-chinese
         * @brief 获取注册表锁。
         */
        void lock() noexcept;

        /**
         * \lang english
         * @brief Releases the registry lock.
         *
         * \lang simp-chinese
         * @brief 释放注册表锁。
         */
        void unlock() noexcept;

        /**
         * \lang english
         * @brief Looks up an instance block by type hash while holding the lock.
         *
         * @param hash The type hash of the instance.
         * @return The instance block, or nullptr when none has been recorded.
         *
         * \lang simp-chinese
         * @brief 持锁时按类型哈希查找实例块。
         *
         * @param hash 实例的类型哈希。
         * @return 实例块；尚未记录时返回 nullptr。
         */
        RAINY_NODISCARD void *find_locked(std::size_t hash, const core::typeinfo *type) const noexcept;

        /**
         * \lang english
         * @brief Allocates a block from the pool while holding the lock.
         *
         * @param size The requested size in bytes.
         * @param align The requested alignment in bytes.
         * @return A pointer to uninitialized storage of at least size bytes.
         *
         * \lang simp-chinese
         * @brief 持锁时从池中分配一块存储。
         *
         * @param size 请求的字节数。
         * @param align 请求的对齐字节数。
         * @return 指向至少 size 字节未初始化存储的指针。
         */
        RAINY_NODISCARD void *allocate_locked(std::size_t size, std::size_t align);

        /**
         * \lang english
         * @brief Records an instance block while holding the lock.
         *
         * @param hash The type hash of the instance.
         * @param block The constructed instance block.
         * @param destroy The callback that destroys the instance at shutdown.
         *
         * \lang simp-chinese
         * @brief 持锁时记录一个实例块。
         *
         * @param hash 实例的类型哈希。
         * @param block 已构造的实例块。
         * @param destroy 在关闭时销毁该实例的回调。
         */
        void record_locked(std::size_t hash, const core::typeinfo *type, void *block, void (*destroy)(void *));

        ~singleton_registry();

    private:
        singleton_registry();

        struct entry {
            std::size_t hash;
            const core::typeinfo *type;
            void *block;
            void (*destroy)(void *);
        };

        struct impl;

        impl *global_ptr_;
    };
}

#endif
