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
#include <rainy/foundation/memory/hazard_pointer.hpp>

// NOLINTBEGIN(cppcoreguidelines-avoid-do-while)

namespace rainy::foundation::memory::implements {
    hazard_pointer_registry &hazard_pointer_registry::instance() {
        static hazard_pointer_registry registry;
        return registry;
    }

    hazard_pointer_registry::~hazard_pointer_registry() {
        // 结束时，清除所有hazard链表
        if (thread_count_.load(std::memory_order_acquire) == 0) {
            thread_hazard_list *curr = head_.load(std::memory_order_acquire);
            while (curr) {
                thread_hazard_list *next = curr->next;
                delete curr;
                curr = next;
            }
        }
    }

    thread_hazard_list *hazard_pointer_registry::get_thread_list() {
        thread_local thread_hazard_list *thread_list = nullptr;
        if (thread_list == nullptr) {
            thread_hazard_list *curr = head_.load(std::memory_order_acquire);
            while (curr) {
                if (bool expected = false;
                    curr->active.compare_exchange_strong(expected, true, std::memory_order_acquire, std::memory_order_relaxed)) {
                    thread_list = curr;
                    thread_count_.fetch_add(1, std::memory_order_relaxed);
                    break;
                }
                curr = curr->next;
            }
            if (thread_list == nullptr) {
                thread_list = new thread_hazard_list();
                thread_hazard_list *old_head = head_.load(std::memory_order_relaxed);
                do {
                    thread_list->next = old_head;
                } while (!head_.compare_exchange_weak(old_head, thread_list, std::memory_order_release, std::memory_order_relaxed));
                thread_count_.fetch_add(1, std::memory_order_relaxed);
            }
            // Register thread cleanup
            thread_local struct thread_cleanup {
                thread_hazard_list *list;

                ~thread_cleanup() {
                    if (list) {
                        instance().mark_inactive();
                    }
                }
            } cleanup{thread_list};
        }
        return thread_list;
    }

    void hazard_pointer_registry::mark_inactive() {
        static thread_local thread_hazard_list *thread_list = get_thread_list();
        if (thread_list) {
            for (auto &h: thread_list->hazards) {
                h.store(nullptr, std::memory_order_release);
            }
            thread_list->active.store(false, std::memory_order_release);
            thread_count_.fetch_sub(1, std::memory_order_relaxed);
        }
    }

    std::vector<void *> hazard_pointer_registry::scan_hazard_pointers() const {
        std::vector<void *> protected_ptrs;
        protected_ptrs.reserve(256);
        const void *SLOT_OWNED_MARKER = reinterpret_cast<void *>(0x1);
        thread_hazard_list *curr = head_.load(std::memory_order_acquire);
        while (curr) {
            if (const bool is_active = curr->active.load(std::memory_order_acquire); is_active) {
                for (auto &hazard: curr->hazards) {
                    if (void *ptr = hazard.load(std::memory_order_acquire); ptr != nullptr && ptr != SLOT_OWNED_MARKER) {
                        protected_ptrs.push_back(ptr);
                    }
                }
            }
            curr = curr->next;
        }
        std::sort(protected_ptrs.begin(), protected_ptrs.end()); // NOLINT
        // 对受保护的指针去重，
        protected_ptrs.erase(std::unique(protected_ptrs.begin(), protected_ptrs.end()), protected_ptrs.end()); // NOLINT
        return protected_ptrs;
    }

    std::size_t hazard_pointer_registry::get_active_thread_count() const {
        return thread_count_.load(std::memory_order_relaxed);
    }

    global_reclamation_manager &global_reclamation_manager::instance() {
        static global_reclamation_manager manager;
        return manager;
    }

    void global_reclamation_manager::add_orphaned_nodes(retire_node *head,const std::size_t count) {
        if (!head) {
            return;
        }
        // Find tail of the new list
        retire_node *tail = head;
        while (tail->next) {
            tail = tail->next;
        }
        // Atomically prepend to the global list
        retire_node *old_head = head_.load(std::memory_order_relaxed);
        // NOLINTBEGIN
        do {
            tail->next = old_head;
        } while (!head_.compare_exchange_weak(old_head, head, std::memory_order_release, std::memory_order_relaxed));
        // NOLINTEND
        count_.fetch_add(count, std::memory_order_relaxed);
    }

    std::size_t global_reclamation_manager::reclaim(const std::vector<void *> &protected_ptrs) {
        retire_node *curr = head_.exchange(nullptr, std::memory_order_acquire);
        if (!curr) {
            return 0;
        }
        count_.store(0, std::memory_order_relaxed);
        std::size_t reclaimed = 0;
        retire_node *keep_head = nullptr;
        retire_node *keep_tail = nullptr;
        std::size_t keep_count = 0;
        while (curr) {
            retire_node *next = curr->next;
            bool is_protected = false;
            if (core::algorithm::binary_search(protected_ptrs.begin(), protected_ptrs.end(), curr->ptr)) {
                is_protected = true;
            }
            if (!is_protected) {
                curr->deleter(curr->ptr);
                delete curr;
                ++reclaimed;
            } else {
                // Keep node
                if (!keep_head) {
                    keep_head = curr;
                    keep_tail = curr;
                } else {
                    keep_tail->next = curr; // NOLINT
                    keep_tail = curr;
                }
                curr->next = nullptr;
                ++keep_count;
            }

            curr = next;
        }
        if (keep_head) {
            add_orphaned_nodes(keep_head, keep_count);
        }
        return reclaimed;
    }

    std::size_t global_reclamation_manager::get_orphaned_count() const {
        return count_.load(std::memory_order_relaxed);
    }
}

namespace rainy::foundation::memory {
    hazard_pointer::hazard_pointer() : slot_(nullptr), slot_index_(0) {
        auto *thread_list = implements::hazard_pointer_registry::instance().get_thread_list();
        for (std::size_t i = 0; i < implements::thread_hazard_list::MAX_HAZARDS_PER_THREAD; ++i) {
            // NOLINTBEGIN
            if (void *expected = nullptr; thread_list->hazards[i].compare_exchange_strong(expected, const_cast<void *>(SLOT_OWNED_MARKER),
                                                                std::memory_order_acquire, std::memory_order_relaxed)) {
                // 寻找一个空槽用于存储hazard_pointer，若当前槽位值不为nullptr，在找到后，我们再将其标记为已占有的hazard_pointer，表示已分配
                slot_ = &thread_list->hazards[i];
                slot_index_ = i;
                return;
            }
            // NOLINTEND
        }
        exceptions::runtime::throw_runtime_error("Can not create hazard pointer, because slots is full");
    }

    hazard_pointer::hazard_pointer(hazard_pointer &&other) noexcept : slot_(other.slot_), slot_index_(other.slot_index_) {
        other.slot_ = nullptr;
        other.slot_index_ = 0;
    }

    hazard_pointer &hazard_pointer::operator=(hazard_pointer &&other) noexcept {
        if (this != &other) {
            reset_protection();
            slot_ = other.slot_;
            slot_index_ = other.slot_index_;
            other.slot_ = nullptr;
            other.slot_index_ = 0;
        }
        return *this;
    }

    hazard_pointer::~hazard_pointer() {
        if (slot_) {
            slot_->store(nullptr, std::memory_order_release);
        }
    }

    bool hazard_pointer::is_protected() const noexcept {
        if (!slot_) {
            return false;
        }
        const void *ptr = slot_->load(std::memory_order_acquire);
        return ptr != nullptr && ptr != SLOT_OWNED_MARKER;
    }

    void *hazard_pointer::get_protected() const noexcept {
        if (!slot_) {
            return nullptr;
        }
        void *ptr = slot_->load(std::memory_order_acquire);
        return (ptr == SLOT_OWNED_MARKER) ? nullptr : ptr;
    }
}

// NOLINTEND(cppcoreguidelines-avoid-do-while)
