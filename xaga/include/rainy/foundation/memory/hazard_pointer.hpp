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

#include <algorithm>
#include <atomic>
#include <cstddef>
#include <functional>
#include <memory>
#include <rainy/core/core.hpp>
#include <vector>

namespace rainy::foundation::memory {
    /**
     * @brief 用于管理hazard_pointer及其被析出的对象的操作
     */
    template <typename T>
    class hazard_pointer_domain;
}

namespace rainy::foundation::memory::implements {
    struct thread_hazard_list {
        static constexpr std::size_t MAX_HAZARDS_PER_THREAD = 8;

        thread_hazard_list() {
            for (auto& h : hazards) {
                h.store(nullptr, std::memory_order_relaxed);
            }
        }

        std::atomic<void*> hazards[MAX_HAZARDS_PER_THREAD];
        thread_hazard_list *next{nullptr};
        std::atomic<bool> active{true};
    };

    struct retire_node {
        void* ptr;
        std::function<void(void*)> deleter;
        retire_node* next{nullptr};

        retire_node(void* p, std::function<void(void*)> d)
            : ptr(p), deleter(std::move(d)) {}
    };

    class RAINY_TOOLKIT_API hazard_pointer_registry {
    public:
        static hazard_pointer_registry &instance();

        thread_hazard_list* get_thread_list();

        void mark_inactive();

        RAINY_NODISCARD std::vector<void*> scan_hazard_pointers() const;

        RAINY_NODISCARD std::size_t get_active_thread_count() const;

        ~hazard_pointer_registry();
    private:
        hazard_pointer_registry() = default;

        std::atomic<thread_hazard_list*> head_{nullptr};
        std::atomic<std::size_t> thread_count_{0};
    };

    class RAINY_TOOLKIT_API global_reclamation_manager {
    public:
        static global_reclamation_manager& instance();

        void add_orphaned_nodes(retire_node* head, std::size_t count);

        std::size_t reclaim(const std::vector<void*>& protected_ptrs);

        RAINY_NODISCARD std::size_t get_orphaned_count() const;

    private:
        global_reclamation_manager() = default;

        std::atomic<retire_node*> head_{nullptr};
        std::atomic<std::size_t> count_{0};
    };
}

namespace rainy::foundation::memory {
    class RAINY_TOOLKIT_API hazard_pointer {
    public:
        hazard_pointer();

        hazard_pointer(hazard_pointer&& other) noexcept;

        hazard_pointer& operator=(hazard_pointer&& other) noexcept;

        ~hazard_pointer();

        hazard_pointer(const hazard_pointer&) = delete;
        hazard_pointer& operator=(const hazard_pointer&) = delete;

        template <typename T>
        T* protect(T* ptr) noexcept {
            if (slot_) {
                slot_->store(static_cast<void*>(ptr), std::memory_order_release);
                return static_cast<T*>(slot_->load());
            }
            return nullptr;
        }

        template <typename T>
        bool try_protect(T *&ptr, const std::atomic<T *> &src) noexcept {
            T *old = ptr;
            reset_protection(old);
            ptr = src.load(std::memory_order_acquire);
            if (old != ptr) {
                reset_protection();
            }
            return old == ptr;
        }

        template <typename T>
        void reset_protection(const T *ptr) noexcept {
            if (!ptr) {
                slot_->store(nullptr, std::memory_order_release);
                return;
            }
            slot_->store(static_cast<void *>(const_cast<T *>(ptr)), std::memory_order_release);
        }

        void reset_protection(std::nullptr_t = nullptr) noexcept { // NOLINT
            slot_->store(nullptr, std::memory_order_release);    
        }

        RAINY_NODISCARD bool is_protected() const noexcept;

        RAINY_NODISCARD void* get_protected() const noexcept;

    private:
        static inline const void* SLOT_OWNED_MARKER = reinterpret_cast<void*>(0x1);

        std::atomic<void*>* slot_;
        std::size_t slot_index_;
    };

    template <typename T>
    class hazard_pointer_domain {
    public:
        static hazard_pointer_domain &global() {
            static hazard_pointer_domain instance;
            return instance;
        }

        hazard_pointer acquire() { // NOLINT
            return {};
        }

        void retire(T *ptr);

        std::size_t reclaim();

        /// Get statistics
        struct stats {
            std::size_t hazard_pointers_allocated;
            std::size_t objects_retired;
            std::size_t objects_reclaimed;
            std::size_t scan_count;
        };

        stats get_stats() const;

        ~hazard_pointer_domain() {
            auto &retire_list = get_thread_retire_list();
            retire_list.reclaim_all();
        }

    private:
        hazard_pointer_domain() = default;

        struct thread_retire_list {
            implements::retire_node* head = nullptr;
            std::size_t count = 0;

            /*
             * 在此设置了自适应阈值：随活跃线程数动态调整
             * 基础阈值（BASE_RECLAIM_THRESHOLD）：64个对象
             * 其中，每个活跃线程处理16个对象的缩放因子
             */
            static constexpr std::size_t BASE_RECLAIM_THRESHOLD = 64;
            static constexpr std::size_t RECLAIM_THRESHOLD_PER_THREAD = 16;

           RAINY_NODISCARD std::size_t get_adaptive_threshold() const { // NOLINT
                const auto & registry = implements::hazard_pointer_registry::instance();
                const std::size_t active_threads = registry.get_active_thread_count(); // 先
                // 避免极端值
                return (core::min)(static_cast<std::size_t>(512),
                                BASE_RECLAIM_THRESHOLD + active_threads * RECLAIM_THRESHOLD_PER_THREAD);
            }

            void add(T* ptr);
            std::size_t scan_and_reclaim(const std::vector<void*>& protected_ptrs);
            void reclaim_all();
            ~thread_retire_list();
        };

        static thread_retire_list& get_thread_retire_list() {
            thread_local thread_retire_list list;
            return list;
        }

        mutable std::atomic<std::size_t> objects_retired_{0};
        mutable std::atomic<std::size_t> objects_reclaimed_{0};
        mutable std::atomic<std::size_t> scan_count_{0};
    };

    // Template implementations

    template <typename T>
    void hazard_pointer_domain<T>::retire(T* ptr) {
        if (!ptr) {
            return;
        }
        auto& retire_list = get_thread_retire_list();
        retire_list.add(ptr);
        objects_retired_.fetch_add(1, std::memory_order_relaxed);
        if (retire_list.count >= retire_list.get_adaptive_threshold()) {
            reclaim();
        }
    }

    template <typename T>
    std::size_t hazard_pointer_domain<T>::reclaim() {
        scan_count_.fetch_add(1, std::memory_order_relaxed);
        const auto& registry = implements::hazard_pointer_registry::instance();
        auto protected_ptrs = registry.scan_hazard_pointers();
        auto& retire_list = get_thread_retire_list();
        std::size_t reclaimed = retire_list.scan_and_reclaim(protected_ptrs);
        // 先清理不在保护状态的hazard_pointer
        reclaimed += implements::global_reclamation_manager::instance().reclaim(protected_ptrs);
        // 清理掉被弃置的hazard_pointer
        objects_reclaimed_.fetch_add(reclaimed, std::memory_order_relaxed);
        return reclaimed;
    }

    template <typename T>
    auto hazard_pointer_domain<T>::get_stats() const -> stats {
        const auto &registry = implements::hazard_pointer_registry::instance();
        stats ret{};
        ret.hazard_pointers_allocated = registry.get_active_thread_count() * implements::thread_hazard_list::MAX_HAZARDS_PER_THREAD;
        ret.objects_retired = objects_retired_.load(std::memory_order_relaxed);
        ret.objects_reclaimed = objects_reclaimed_.load(std::memory_order_relaxed);
        ret.scan_count = scan_count_.load(std::memory_order_relaxed);
        return ret;
    }

    template <typename T>
    void hazard_pointer_domain<T>::thread_retire_list::add(T* ptr) {
        auto* node = new implements::retire_node(static_cast<void*>(ptr),
                                             [](void* p) { delete static_cast<T*>(p); });
        node->next = head;
        head = node;
        ++count;
    }

    template <typename T>
    std::size_t hazard_pointer_domain<T>::thread_retire_list::scan_and_reclaim(
        const std::vector<void*>& protected_ptrs) {
        std::size_t reclaimed = 0;
        implements::retire_node** curr = &head;
        while (*curr) {
            bool is_protected = false;
            if (core::algorithm::binary_search(protected_ptrs.begin(), protected_ptrs.end(), (*curr)->ptr)) {
                is_protected = true;
            }
            if (!is_protected) {
                const implements::retire_node* to_delete = *curr;
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

    template <typename T>
    void hazard_pointer_domain<T>::thread_retire_list::reclaim_all() {
        const auto & registry = implements::hazard_pointer_registry::instance();
        const auto protected_ptrs = registry.scan_hazard_pointers(); // 先扫描所有受保护的hazard_pointer
        scan_and_reclaim(protected_ptrs);
        // 把仍在保护的节点添加到全局表
        if (head) {
            implements::global_reclamation_manager::instance().add_orphaned_nodes(head, count);
            head = nullptr;
            count = 0;
        }
    }

    template <typename T>
    hazard_pointer_domain<T>::thread_retire_list::~thread_retire_list() {
        reclaim_all();
    }

    RAINY_INLINE hazard_pointer make_hazard_pointer() {
        return {};
    }
}

#endif