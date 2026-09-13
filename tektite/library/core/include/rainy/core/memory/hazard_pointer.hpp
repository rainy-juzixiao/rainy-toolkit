/*
 * Copyright 2025 rainy-juzixiao
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
#ifndef RAINY_FOUNDATION_MEMORY_HAZARD_POINTER_HPP
#define RAINY_FOUNDATION_MEMORY_HAZARD_POINTER_HPP

#include <functional>
#include <rainy/core/algorithm/basic_algorithm.hpp>
#include <rainy/core/collections/vector.hpp>
#include <rainy/core/concurrency/atomic.hpp>
#include <rainy/core/functional/delegate.hpp>
#include <rainy/core/platform.hpp>

namespace rainy::core::memory {
    /**
     * \lang english
     * @brief Manages hazard pointers and the objects retired through them.
     *
     * \lang simp-chinese
     * @brief 用于管理hazard_pointer及其被析出的对象的操作
     */
    template <typename Ty>
    class hazard_pointer_domain;
}

namespace rainy::core::memory::implements {
    struct thread_hazard_list {
        static constexpr std::size_t MAX_HAZARDS_PER_THREAD = 8;

        thread_hazard_list() {
            for (auto &h: hazards) {
                h.store(nullptr, concurrency::memory_order_relaxed);
            }
        }

        concurrency::atomic<void *> hazards[MAX_HAZARDS_PER_THREAD];
        thread_hazard_list *next{nullptr};
        concurrency::atomic<bool> active{true};
    };

    struct retire_node {
        void *ptr;
        functional::delegate<void(void *)> deleter;
        retire_node *next{nullptr};

        retire_node(void *p, functional::delegate<void(void *)> d) : ptr(p), deleter(utility::move(d)) {
        }
    };

    class RAINY_TOOLKIT_API hazard_pointer_registry {
    public:
        static hazard_pointer_registry &instance();

        thread_hazard_list *get_thread_list();

        void mark_inactive();

        RAINY_NODISCARD collections::vector<void *> scan_hazard_pointers() const;

        RAINY_NODISCARD std::size_t get_active_thread_count() const;

        ~hazard_pointer_registry();

    private:
        hazard_pointer_registry() = default;

        concurrency::atomic<thread_hazard_list *> head_{nullptr};
        concurrency::atomic<std::size_t> thread_count_{0};
    };

    class RAINY_TOOLKIT_API global_reclamation_manager {
    public:
        static global_reclamation_manager &instance();

        void add_orphaned_nodes(retire_node *head, std::size_t count);

        std::size_t reclaim(const collections::vector<void *> &protected_ptrs);

        RAINY_NODISCARD std::size_t get_orphaned_count() const;

    private:
        global_reclamation_manager() = default;

        concurrency::atomic<retire_node *> head_{nullptr};
        concurrency::atomic<std::size_t> count_{0};
    };
}

namespace rainy::core::memory {
    /**
     * \lang english
     * @brief A hazard pointer that protects a pointer from reclamation.
     *
     * hazard_pointer provides a non-blocking mechanism to mark a pointer as
     * currently in use, so that hazard_pointer_domain will not reclaim the
     * object while it is being accessed.
     *
     * \lang simp-chinese
     * @brief 用于保护指针免受回收的 hazard pointer
     *
     * hazard_pointer 提供了一种非阻塞机制来标记当前正在使用的指针，
     * 从而保证在访问对象期间 hazard_pointer_domain 不会回收该对象。
     */
    class RAINY_TOOLKIT_API hazard_pointer {
    public:
        /**
         * \lang english
         * @brief Constructs a hazard_pointer and acquires an available hazard slot.
         *
         * \lang simp-chinese
         * @brief 构造 hazard_pointer 并获取一个可用的 hazard 槽位
         */
        hazard_pointer();

        /**
         * \lang english
         * @brief Move constructor, transfers the acquired hazard slot from other.
         * @param other The hazard_pointer to move from
         *
         * \lang simp-chinese
         * @brief 移动构造函数，从 other 转移已获取的 hazard 槽位
         * @param other 要移入的 hazard_pointer
         */
        hazard_pointer(hazard_pointer &&other) noexcept;

        /**
         * \lang english
         * @brief Move assignment operator, transfers the hazard slot from other.
         * @param other The hazard_pointer to move from
         * @return Reference to this hazard_pointer
         *
         * \lang simp-chinese
         * @brief 移动赋值运算符，从 other 转移 hazard 槽位
         * @param other 要移入的 hazard_pointer
         * @return 返回本 hazard_pointer 的引用
         */
        hazard_pointer &operator=(hazard_pointer &&other) noexcept;

        /**
         * \lang english
         * @brief Destructor, releases the acquired hazard slot.
         *
         * \lang simp-chinese
         * @brief 析构函数，释放已获取的 hazard 槽位
         */
        ~hazard_pointer();

        hazard_pointer(const hazard_pointer &) = delete;
        hazard_pointer &operator=(const hazard_pointer &) = delete;

        /**
         * \lang english
         * @brief Protects the given pointer, making it immune to reclamation.
         * @tparam Ty The type of the pointed-to object
         * @param ptr The pointer to protect
         * @return The protected pointer, or nullptr if no hazard slot is available
         *
         * \lang simp-chinese
         * @brief 保护给定的指针，使其免于被回收
         * @tparam Ty 所指向对象的类型
         * @param ptr 要保护的指针
         * @return 被保护的指针；如果没有可用的 hazard 槽位则返回 nullptr
         */
        template <typename Ty>
        Ty *protect(Ty *ptr) noexcept {
            if (slot_) {
                slot_->store(static_cast<void *>(ptr), concurrency::memory_order_release);
                return static_cast<Ty *>(slot_->load());
            }
            return nullptr;
        }

        /**
         * \lang english
         * @brief Atomically retries to protect ptr loaded from src.
         * @tparam Ty The type of the pointed-to object
         * @param ptr A reference to the current pointer, updated with the new load
         * @param src The atomic source to load the pointer from
         * @return true if the loaded pointer equals the old value and was protected, otherwise false
         *
         * \lang simp-chinese
         * @brief 原子性地重试保护从 src 加载的指针
         * @tparam Ty 所指向对象的类型
         * @param ptr 当前指针的引用，会更新为新的加载值
         * @param src 用于加载指针的原子源
         * @return 如果加载的指针与旧值相等并被保护返回 true，否则返回 false
         */
        template <typename Ty>
        bool try_protect(Ty *&ptr, const concurrency::atomic<Ty *> &src) noexcept {
            Ty *old = ptr;
            reset_protection(old);
            ptr = src.load(concurrency::memory_order_acquire);
            if (old != ptr) {
                reset_protection();
            }
            return old == ptr;
        }

        /**
         * \lang english
         * @brief Updates the hazard slot to protect the given pointer.
         * @tparam Ty The type of the pointed-to object
         * @param ptr The pointer to protect, or nullptr to clear the slot
         *
         * \lang simp-chinese
         * @brief 更新 hazard 槽位以保护给定的指针
         * @tparam Ty 所指向对象的类型
         * @param ptr 要保护的指针；传入 nullptr 可清除槽位
         */
        template <typename Ty>
        void reset_protection(const Ty *ptr) noexcept {
            if (!ptr) {
                slot_->store(nullptr, concurrency::memory_order_release);
                return;
            }
            slot_->store(static_cast<void *>(const_cast<Ty *>(ptr)), concurrency::memory_order_release);
        }

        /**
         * \lang english
         * @brief Clears the hazard slot.
         * @param value Unused, defaults to nullptr
         *
         * \lang simp-chinese
         * @brief 清除 hazard 槽位
         * @param value 未使用，默认为 nullptr
         */
        void reset_protection(std::nullptr_t = nullptr) noexcept { // NOLINT
            slot_->store(nullptr, concurrency::memory_order_release);
        }

        /**
         * \lang english
         * @brief Checks whether this hazard_pointer currently protects a pointer.
         * @return true if a pointer is protected, otherwise false
         *
         * \lang simp-chinese
         * @brief 检查本 hazard_pointer 当前是否保护着某个指针
         * @return 如果保护着指针返回 true，否则返回 false
         */
        RAINY_NODISCARD bool is_protected() const noexcept;

        /**
         * \lang english
         * @brief Gets the currently protected pointer.
         * @return The protected pointer, or nullptr if none is protected
         *
         * \lang simp-chinese
         * @brief 获取当前被保护的指针
         * @return 被保护的指针；如果没有则返回 nullptr
         */
        RAINY_NODISCARD void *get_protected() const noexcept;

    private:
        static inline const void *SLOT_OWNED_MARKER = reinterpret_cast<void *>(0x1);

        concurrency::atomic<void *> *slot_;
        std::size_t slot_index_;
    };

    /**
     * \lang english
     * @brief A reclamation domain that manages hazard pointers and retired objects.
     *
     * Objects retired through retire() are only reclaimed once no thread
     * currently protects them with a hazard_pointer.
     *
     * @tparam Ty The type of the objects managed by this domain
     *
     * \lang simp-chinese
     * @brief 管理 hazard pointer 与被弃置对象回收的域
     *
     * 通过 retire() 弃置的对象只有在没有线程正用 hazard_pointer 保护它们时才会被回收。
     *
     * @tparam Ty 该域管理的对象类型
     */
    template <typename Ty>
    class hazard_pointer_domain {
    public:
        /**
         * \lang english
         * @brief Gets the process-wide global hazard_pointer_domain instance.
         * @return Reference to the global hazard_pointer_domain
         *
         * \lang simp-chinese
         * @brief 获取进程级全局 hazard_pointer_domain 实例
         * @return 全局 hazard_pointer_domain 的引用
         */
        static hazard_pointer_domain &global() {
            static hazard_pointer_domain instance;
            return instance;
        }

        /**
         * \lang english
         * @brief Acquires a hazard_pointer from this domain.
         * @return A hazard_pointer that can protect pointers
         *
         * \lang simp-chinese
         * @brief 从该域获取一个 hazard_pointer
         * @return 一个可用于保护指针的 hazard_pointer
         */
        hazard_pointer acquire() { // NOLINT
            return {};
        }

        /**
         * \lang english
         * @brief Retires an object for later reclamation.
         * @param ptr The pointer to the object to retire
         *
         * \lang simp-chinese
         * @brief 弃置一个对象以便稍后回收
         * @param ptr 要弃置对象的指针
         */
        void retire(Ty *ptr);

        /**
         * \lang english
         * @brief Reclaims all objects that are no longer protected.
         * @return The number of reclaimed objects
         *
         * \lang simp-chinese
         * @brief 回收所有不再被保护的对象
         * @return 被回收对象的数量
         */
        std::size_t reclaim();

        /**
         * \lang english
         * @brief Statistics about this hazard_pointer_domain.
         *
         * \lang simp-chinese
         * @brief 关于该 hazard_pointer_domain 的统计信息
         */
        /// Get statistics
        struct stats {
            std::size_t hazard_pointers_allocated;
            std::size_t objects_retired;
            std::size_t objects_reclaimed;
            std::size_t scan_count;
        };

        /**
         * \lang english
         * @brief Gets the statistics of this domain.
         * @return The collected statistics
         *
         * \lang simp-chinese
         * @brief 获取该域的统计信息
         * @return 收集到的统计信息
         */
        stats get_stats() const;

        /**
         * \lang english
         * @brief Destructor, reclaims all remaining retired objects.
         *
         * \lang simp-chinese
         * @brief 析构函数，回收所有剩余的已弃置对象
         */
        ~hazard_pointer_domain() {
            auto &retire_list = get_thread_retire_list();
            retire_list.reclaim_all();
        }

    private:
        hazard_pointer_domain() = default;

        struct thread_retire_list {
            implements::retire_node *head = nullptr;
            std::size_t count = 0;

            /*
             * 在此设置了自适应阈值：随活跃线程数动态调整
             * 基础阈值（BASE_RECLAIM_THRESHOLD）：64个对象
             * 其中，每个活跃线程处理16个对象的缩放因子
             */
            static constexpr std::size_t BASE_RECLAIM_THRESHOLD = 64;
            static constexpr std::size_t RECLAIM_THRESHOLD_PER_THREAD = 16;

            RAINY_NODISCARD std::size_t get_adaptive_threshold() const { // NOLINT
                const auto &registry = implements::hazard_pointer_registry::instance();
                const std::size_t active_threads = registry.get_active_thread_count(); // 先
                // 避免极端值
                return (core::min) (static_cast<std::size_t>(512),
                                    BASE_RECLAIM_THRESHOLD + active_threads * RECLAIM_THRESHOLD_PER_THREAD);
            }

            void add(Ty *ptr);
            std::size_t scan_and_reclaim(const collections::vector<void *> &protected_ptrs);
            void reclaim_all();
            ~thread_retire_list();
        };

        static thread_retire_list &get_thread_retire_list() {
            thread_local thread_retire_list list;
            return list;
        }

        mutable concurrency::atomic<std::size_t> objects_retired_{0};
        mutable concurrency::atomic<std::size_t> objects_reclaimed_{0};
        mutable concurrency::atomic<std::size_t> scan_count_{0};
    };

    // Template implementations

    template <typename Ty>
    void hazard_pointer_domain<Ty>::retire(Ty *ptr) {
        if (!ptr) {
            return;
        }
        auto &retire_list = get_thread_retire_list();
        retire_list.add(ptr);
        objects_retired_.fetch_add(1, concurrency::memory_order_relaxed);
        if (retire_list.count >= retire_list.get_adaptive_threshold()) {
            reclaim();
        }
    }

    template <typename Ty>
    std::size_t hazard_pointer_domain<Ty>::reclaim() {
        scan_count_.fetch_add(1, concurrency::memory_order_relaxed);
        const auto &registry = implements::hazard_pointer_registry::instance();
        auto protected_ptrs = registry.scan_hazard_pointers();
        auto &retire_list = get_thread_retire_list();
        std::size_t reclaimed = retire_list.scan_and_reclaim(protected_ptrs);
        // 先清理不在保护状态的hazard_pointer
        reclaimed += implements::global_reclamation_manager::instance().reclaim(protected_ptrs);
        // 清理掉被弃置的hazard_pointer
        objects_reclaimed_.fetch_add(reclaimed, concurrency::memory_order_relaxed);
        return reclaimed;
    }

    template <typename Ty>
    auto hazard_pointer_domain<Ty>::get_stats() const -> stats {
        const auto &registry = implements::hazard_pointer_registry::instance();
        stats ret{};
        ret.hazard_pointers_allocated = registry.get_active_thread_count() * implements::thread_hazard_list::MAX_HAZARDS_PER_THREAD;
        ret.objects_retired = objects_retired_.load(concurrency::memory_order_relaxed);
        ret.objects_reclaimed = objects_reclaimed_.load(concurrency::memory_order_relaxed);
        ret.scan_count = scan_count_.load(concurrency::memory_order_relaxed);
        return ret;
    }

    template <typename Ty>
    void hazard_pointer_domain<Ty>::thread_retire_list::add(Ty *ptr) {
        auto *node = new implements::retire_node(static_cast<void *>(ptr), [](void *p) { delete static_cast<Ty *>(p); });
        node->next = head;
        head = node;
        ++count;
    }

    template <typename Ty>
    std::size_t hazard_pointer_domain<Ty>::thread_retire_list::scan_and_reclaim(const collections::vector<void *> &protected_ptrs) {
        std::size_t reclaimed = 0;
        implements::retire_node **curr = &head;
        while (*curr) {
            bool is_protected = false;
            if (core::algorithm::binary_search(protected_ptrs.begin(), protected_ptrs.end(), (*curr)->ptr)) {
                is_protected = true;
            }
            if (!is_protected) {
                const implements::retire_node *to_delete = *curr;
                *curr = (*curr)->next;
                to_delete->deleter(to_delete->ptr);
                delete to_delete;
                ++reclaimed;
                --count;
            } else {
                curr = &(*curr)->next;
            }
        }

        return reclaimed;
    }

    template <typename Ty>
    void hazard_pointer_domain<Ty>::thread_retire_list::reclaim_all() {
        const auto &registry = implements::hazard_pointer_registry::instance();
        const auto protected_ptrs = registry.scan_hazard_pointers(); // 先扫描所有受保护的hazard_pointer
        scan_and_reclaim(protected_ptrs);
        // 把仍在保护的节点添加到全局表
        if (head) {
            implements::global_reclamation_manager::instance().add_orphaned_nodes(head, count);
            head = nullptr;
            count = 0;
        }
    }

    template <typename Ty>
    hazard_pointer_domain<Ty>::thread_retire_list::~thread_retire_list() {
        reclaim_all();
    }

    RAINY_INLINE hazard_pointer make_hazard_pointer() {
        return {};
    }
}

#endif
