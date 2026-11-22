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
#ifndef RAINY_FOUNDATION_CONCURRENCY_CONTEXT_HPP
#define RAINY_FOUNDATION_CONCURRENCY_CONTEXT_HPP
#include <rainy/core/platform.hpp>
#include <rainy/core/annotations/lifetime_annotation.hpp>
#include <rainy/core/concurrency/tss_ptr.hpp>

#ifndef RAINY_RECYCLING_ALLOCATOR_CACHE_SIZE
#define RAINY_RECYCLING_ALLOCATOR_CACHE_SIZE 2
#endif

namespace rainy::core::concurrency::implements {
    class RAINY_TOOLKIT_API thread_info_base : type_traits::helper::non_copyable {
    public:
        struct default_tag {
            enum {
                cache_size = RAINY_RECYCLING_ALLOCATOR_CACHE_SIZE,
                begin_mem_index = 0,
                end_mem_index = cache_size
            };
        };

        struct awaitable_frame_tag {
            enum {
                cache_size = RAINY_RECYCLING_ALLOCATOR_CACHE_SIZE,
                begin_mem_index = default_tag::end_mem_index,
                end_mem_index = begin_mem_index + cache_size
            };
        };

        struct executor_function_tag {
            enum {
                cache_size = RAINY_RECYCLING_ALLOCATOR_CACHE_SIZE,
                begin_mem_index = awaitable_frame_tag::end_mem_index,
                end_mem_index = begin_mem_index + cache_size
            };
        };

        struct cancellation_signal_tag {
            enum {
                cache_size = RAINY_RECYCLING_ALLOCATOR_CACHE_SIZE,
                begin_mem_index = executor_function_tag::end_mem_index,
                end_mem_index = begin_mem_index + cache_size
            };
        };

        struct parallel_group_tag {
            enum {
                cache_size = RAINY_RECYCLING_ALLOCATOR_CACHE_SIZE,
                begin_mem_index = cancellation_signal_tag::end_mem_index,
                end_mem_index = begin_mem_index + cache_size
            };
        };

        enum {
            max_mem_index = parallel_group_tag::end_mem_index
        };

        thread_info_base();

        ~thread_info_base();

        static void *allocate(thread_info_base *this_thread, const std::size_t size,
                              const std::size_t align = alignof(std::max_align_t)) {
            return allocate(default_tag(), this_thread, size, align);
        }

        static void deallocate(thread_info_base *this_thread, void *pointer, const std::size_t size,
                               const std::size_t align = alignof(std::max_align_t)) {
            deallocate(default_tag(), this_thread, pointer, size, align);
        }

        template <typename Purpose>
        static void *allocate(Purpose, thread_info_base *this_thread, const std::size_t size,
                              const std::size_t align = alignof(std::max_align_t)) {
            const std::size_t chunks = (size + chunk_size - 1) / chunk_size;
            if (this_thread) {
                for (int mem_index = Purpose::begin_mem_index; mem_index < Purpose::end_mem_index; ++mem_index) {
                    if (this_thread->reusable_memory_[mem_index].pointer) {
                        void *const pointer = this_thread->reusable_memory_[mem_index].pointer;
                        if (rainy_const mem = static_cast<unsigned char *>(pointer);
                            static_cast<std::size_t>(mem[0]) >= chunks && this_thread->reusable_memory_[mem_index].alignment >= align &&
                                reinterpret_cast<std::size_t>(pointer) % align == 0) {
                            this_thread->reusable_memory_[mem_index] = {};
                            mem[size] = mem[0];
                            return pointer;
                        }
                    }
                }
                for (int mem_index = Purpose::begin_mem_index; mem_index < Purpose::end_mem_index; ++mem_index) {
                    if (this_thread->reusable_memory_[mem_index].pointer) {
                        const auto cached = this_thread->reusable_memory_[mem_index];
                        this_thread->reusable_memory_[mem_index] = {};
                        layer::deallocate(cached.pointer, cached.alignment);
                        break;
                    }
                }
            }
            void *const pointer = core::layer::allocate(chunks * chunk_size + 1, align);
            rainy_const mem = static_cast<unsigned char *>(pointer);
            mem[size] = (chunks <= UCHAR_MAX) ? static_cast<unsigned char>(chunks) : 0;
            return pointer;
        }

        template <typename Purpose>
        static void deallocate(Purpose, thread_info_base *this_thread, void *pointer, const std::size_t size,
                               const std::size_t align = alignof(std::max_align_t)) {
            if (size <= chunk_size * UCHAR_MAX) {
                if (this_thread) {
                    for (int mem_index = Purpose::begin_mem_index; mem_index < Purpose::end_mem_index; ++mem_index) {
                        if (this_thread->reusable_memory_[mem_index].pointer == nullptr) {
                            rainy_const mem = static_cast<unsigned char *>(pointer);
                            mem[0] = mem[size];
                            this_thread->reusable_memory_[mem_index] = {pointer, align};
                            return;
                        }
                    }
                }
            }
            layer::deallocate(pointer, align);
        }

        void capture_current_exception();

        void rethrow_pending_exception();

    private:
#if RAINY_HAS_IO_URING
        static constexpr std::size_t chunk_size = 8;
#else
        static constexpr std::size_t chunk_size = 4;
#endif
        struct reusable_memory {
            void *pointer{nullptr};
            std::size_t alignment{alignof(std::max_align_t)};
        };

        reusable_memory reusable_memory_[max_mem_index];

        int has_pending_exception_;
        std::exception_ptr pending_exception_;
    };

    template <typename Key, typename Value = unsigned char>
    class call_stack {
    public:
        class context : type_traits::helper::non_copyable {
        public:
            explicit context(Key *k) : key_(k), next_(call_stack<Key, Value>::top_) { // NOLINT
                value_ = reinterpret_cast<unsigned char *>(this);
                call_stack<Key, Value>::top_ = this; // NOLINT
            }

            context(Key *k, Value &v) : key_(k), value_(&v), next_(call_stack<Key, Value>::top_) { // NOLINT
                call_stack<Key, Value>::top_ = this; // NOLINT
            }

            ~context() {
                call_stack<Key, Value>::top_ = next_; // NOLINT
            }

            Value *next_by_key() const {
                context *elem = next_;
                while (elem) {
                    if (elem->key_ == key_) {
                        return elem->value_;
                    }
                    elem = elem->next_;
                }
                return nullptr;
            }

        private:
            friend class call_stack<Key, Value>;

            Key *key_;
            Value *value_;
            context *next_;
        };

        friend class context;

        static Value *contains(Key *k) {
            context *elem = top_;
            while (elem) {
                if (elem->key_ == k) {
                    return elem->value_;
                }
                elem = elem->next_;
            }
            return nullptr;
        }

        static Value *top() {
            context *elem = top_;
            return elem ? elem->value_ : nullptr;
        }

    private:
        static tss_ptr<context> top_;
    };

    template <typename Key, typename Value>
    tss_ptr<typename call_stack<Key, Value>::context> call_stack<Key, Value>::top_;
}

namespace rainy::core::concurrency {
    /**
     * \lang english
     * @brief Provides access to the thread-local execution context.
     *
     *  Hosts the per-thread call stack used to track the innermost active
     *  context (such as the recycling allocator or executor state) for the calling thread.
     *
     * \lang simp-chinese
     * @brief 提供对线程局部执行上下文的访问。
     *
     *  持有用于跟踪调用线程最内层活动上下文（例如回收分配器或执行器状态）的每线程调用栈。
     */
    class thread_context {
    public:
        /**
         * \lang english
         * @brief The call stack type associated with the current thread.
         *
         * \lang simp-chinese
         * @brief 与当前线程关联的调用栈类型。
         */
        using thread_call_stack = implements::call_stack<thread_context, implements::thread_info_base>;

        /**
         * \lang english
         * @brief Returns the top of the current thread's call stack.
         *
         * @return The top `thread_info_base` of the current thread's call stack, or `nullptr` if the stack is empty.
         *
         * \lang simp-chinese
         * @brief 返回当前线程调用栈的栈顶。
         *
         * @return 当前线程调用栈的栈顶 `thread_info_base`，若调用栈为空则返回 `nullptr`。
         */
        static implements::thread_info_base *top_of_thread_call_stack();
    };
}

#endif
