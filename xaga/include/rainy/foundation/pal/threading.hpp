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
#ifndef RAINY_FOUNDATION_PAL_THREADING_HPP
#define RAINY_FOUNDATION_PAL_THREADING_HPP
/**
 * @file threading.hpp
 * @brief 此头文件是rainy-toolkit用于实现跨平台线程API的库，若要访问内部的API，请参阅implements文件夹的tgc_layer_threading.hpp
 */
#include <chrono>
#include <rainy/core/core.hpp>
#include <rainy/foundation/functional/function_pointer.hpp>
#include <rainy/foundation/diagnostics/contract.hpp>
#include <rainy/foundation/system/memory/nebula_ptr.hpp>
#include <rainy/foundation/memory/allocator.hpp>
#include <rainy/utility/any.hpp>
#include <rainy/foundation/pal/implements/tgc_layer_threading.hpp>
#include <rainy/foundation/pal/atomicinfra.hpp>

namespace rainy::foundation::pal::threading {
    template <typename Ty>
    class tss_ptr {
    public:
        tss_ptr() : tss_key{implements::tss_create()} {
        }

        ~tss_ptr() {
            implements::tss_delete(tss_key);
        }

        operator Ty *() { // NOLINT
            return static_cast<Ty*>(implements::tss_get(tss_key));
        }

        void operator=(Ty *value) {
            implements::tss_set(tss_key, value);
        }

    private:
        core::handle tss_key;
    };
}

#ifndef RAINY_RECYCLING_ALLOCATOR_CACHE_SIZE
#define RAINY_RECYCLING_ALLOCATOR_CACHE_SIZE 2
#endif

namespace rainy::foundation::pal::threading::implements {
    class thread_info_base : type_traits::helper::non_copyable {
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

        thread_info_base() : has_pending_exception_{0} {
            for (int i = 0; i < max_mem_index; ++i) {
                reusable_memory_[i] = nullptr;
            }
        }

        ~thread_info_base() {
            for (int i = 0; i < max_mem_index; ++i) {
                if (reusable_memory_[i]) {
                    core::pal::deallocate(reusable_memory_[i]);
                }
            }
        }

        static void *allocate(thread_info_base *this_thread, std::size_t size, std::size_t align = alignof(std::max_align_t)) {
            return allocate(default_tag(), this_thread, size, align);
        }

        static void deallocate(thread_info_base *this_thread, void *pointer, std::size_t size,
                               std::size_t align = alignof(std::max_align_t)) {
            deallocate(default_tag(), this_thread, pointer, size, align);
        }

        template <typename Purpose>
        static void *allocate(Purpose, thread_info_base *this_thread, std::size_t size,
                              std::size_t align = alignof(std::max_align_t)) {
            std::size_t chunks = (size + chunk_size - 1) / chunk_size;
            if (this_thread) {
                for (int mem_index = Purpose::begin_mem_index; mem_index < Purpose::end_mem_index; ++mem_index) {
                    if (this_thread->reusable_memory_[mem_index]) {
                        void *const pointer = this_thread->reusable_memory_[mem_index];
                        unsigned char *const mem = static_cast<unsigned char *>(pointer);
                        if (static_cast<std::size_t>(mem[0]) >= chunks && reinterpret_cast<std::size_t>(pointer) % align == 0) {
                            this_thread->reusable_memory_[mem_index] = 0;
                            mem[size] = mem[0];
                            return pointer;
                        }
                    }
                }
                for (int mem_index = Purpose::begin_mem_index; mem_index < Purpose::end_mem_index; ++mem_index) {
                    if (this_thread->reusable_memory_[mem_index]) {
                        void *const pointer = this_thread->reusable_memory_[mem_index];
                        this_thread->reusable_memory_[mem_index] = 0;
                        core::pal::deallocate(pointer, align);
                        break;
                    }
                }
            }
            void *const pointer = core::pal::allocate(chunks * chunk_size + 1, align);
            unsigned char *const mem = static_cast<unsigned char *>(pointer);
            mem[size] = (chunks <= UCHAR_MAX) ? static_cast<unsigned char>(chunks) : 0;
            return pointer;
        }

        template <typename Purpose>
        static void deallocate(Purpose, thread_info_base *this_thread, void *pointer, std::size_t size,
                               std::size_t align = alignof(std::max_align_t)) {
            if (size <= chunk_size * UCHAR_MAX) {
                if (this_thread) {
                    for (int mem_index = Purpose::begin_mem_index; mem_index < Purpose::end_mem_index; ++mem_index) {
                        if (this_thread->reusable_memory_[mem_index] == 0) {
                            unsigned char *const mem = static_cast<unsigned char *>(pointer);
                            mem[0] = mem[size];
                            this_thread->reusable_memory_[mem_index] = pointer;
                            return;
                        }
                    }
                }
            }
            core::pal::deallocate(pointer, align);
        }

        void capture_current_exception() {
            switch (has_pending_exception_) {
                case 0:
                    has_pending_exception_ = 1;
                    pending_exception_ = std::current_exception();
                    break;
                case 1:
                    has_pending_exception_ = 2;
                    pending_exception_ = std::make_exception_ptr<foundation::exceptions::multiple_exceptions>(
                        foundation::exceptions::multiple_exceptions(pending_exception_));
                    break;
                default:
                    break;
            }
        }

        void rethrow_pending_exception() {
            if (has_pending_exception_ > 0) {
                has_pending_exception_ = 0;
                std::exception_ptr ex(utility::move(pending_exception_));
                std::rethrow_exception(ex);
            }
        }

    private:
#if RAINY_HAS_IO_URING
        static constexpr std::size_t chunk_size = 8;
#else
        static constexpr std::size_t chunk_size = 4;
#endif
        void *reusable_memory_[max_mem_index];

        int has_pending_exception_;
        std::exception_ptr pending_exception_;
    };

    template <typename Key, typename Value = unsigned char>
    class call_stack {
    public:
        class context : private type_traits::helper::non_copyable {
        public:
            explicit context(Key *k) : key_(k), next_(call_stack<Key, Value>::top_) {
                value_ = reinterpret_cast<unsigned char *>(this);
                call_stack<Key, Value>::top_ = this;
            }

            context(Key *k, Value &v) : key_(k), value_(&v), next_(call_stack<Key, Value>::top_) {
                call_stack<Key, Value>::top_ = this;
            }

            ~context() {
                call_stack<Key, Value>::top_ = next_;
            }

            Value *next_by_key() const {
                context *elem = next_;
                while (elem) {
                    if (elem->key_ == key_)
                        return elem->value_;
                    elem = elem->next_;
                }
                return 0;
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
            return 0;
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

namespace rainy::foundation::pal::threading {
    class thread_context {
    public:
        static implements::thread_info_base *top_of_thread_call_stack();
    protected:
        using thread_call_stack = implements::call_stack<thread_context, implements::thread_info_base>;
    };
}

namespace rainy::foundation::pal::threading {
    class RAINY_TOOLKIT_API thread {
    public:
        class id;

        enum class policy {
            manual,
            auto_join,
            auto_detach
        };

#if RAINY_USING_LINUX
        using native_handle_type = ::pthread_t;
#else
        using native_handle_type = void *;
#endif

        thread() = default;

        thread(policy policy) : policy_(policy), thread_handle{} {
        }

        thread(thread &&right) noexcept :
            policy_(utility::exchange(right.policy_, policy::manual)), thread_handle(utility::exchange(right.thread_handle, {})) {
        }

        template <typename Fx, typename... Args>
        RAINY_NODISCARD explicit thread(Fx &&f, Args &&...args) : policy_(policy::manual) {
            start(utility::forward<Fx>(f), utility::forward<Args>(args)...);
        }

        template <typename Fx, typename... Args>
        RAINY_NODISCARD explicit thread(policy policy, Fx &&f, Args &&...args) : policy_(policy) {
            start(utility::forward<Fx>(f), utility::forward<Args>(args)...);
        }

        ~thread();

        thread &operator=(thread &&right) noexcept;
        
        thread(const thread &) = delete;
        thread &operator=(const thread &) = delete;

        template <typename Fx, typename... Args>
        void start(Fx &&fn, Args &&...args) {
            using tuple = std::tuple<type_traits::other_trans::decay_t<Fx>, type_traits::other_trans::decay_t<Args>...>;
            if (thread_handle.handle) {
                return;
            }
            auto decay_copied =
                foundation::system::memory::make_nebula<tuple>(utility::forward<Fx>(fn), utility::forward<Args>(args)...);
            constexpr auto invoker = get_invoke_function_addr<tuple>(std::make_index_sequence<1 + sizeof...(Args)>{});
            thread_handle = implements::create_thread(nullptr, 0, invoker, decay_copied.get(), 0, nullptr);
            if (thread_handle.handle) {
                (void) decay_copied.release();
            } else {
                throw std::system_error(errno, std::generic_category(), "Failed to create thread");
            }
        }

        void join();

        void detach();

        RAINY_NODISCARD bool joinable() const noexcept;

        static void sleep_for(const unsigned long ms) noexcept;

        RAINY_NODISCARD native_handle_type native_handle() const noexcept;

        void suspend() noexcept;

        void resume() noexcept;

        id get_id() const noexcept;

    private:
        template <typename Tuple, std::size_t... Indices>
        static unsigned int invoke_function(void *arg_list) {
            foundation::system::memory::nebula_ptr<Tuple> fn_vals(static_cast<Tuple *>(arg_list));
            Tuple &tuple = *fn_vals.get();
            utility::invoke(utility::move(std::get<Indices>(tuple))...);
            return 0;
        }

        template <typename Tuple, std::size_t... Indices>
        static constexpr auto get_invoke_function_addr(std::index_sequence<Indices...>) {
            return &invoke_function<Tuple, Indices...>;
        }

        policy policy_{};
        implements::schd_thread_t thread_handle{};
    };

    template <typename Fx, typename... Args>
    thread make_thread(Fx &&fn, Args &&...args) {
        return thread(utility::forward<Fx>(fn), utility::forward<Args>(args)...);
    }

    template <typename Mutex, typename Fx, typename... Args>
    RAINY_INLINE decltype(auto) create_synchronized_task(Mutex &mtx, Fx &&callable, Args &&...args) noexcept(
        noexcept(utility::invoke(utility::forward<Fx>(callable), utility::forward<Args>(args)...))) {
        std::lock_guard<Mutex> guard{mtx};
        return utility::invoke(utility::forward<Fx>(callable), utility::forward<Args>(args)...);
    }
}

namespace rainy::foundation::system::this_thread::implements {
    template <typename Rep, typename Period>
    RAINY_NODISCARD auto to_absolute_time(const std::chrono::duration<Rep, Period> &rel_time) noexcept {
        constexpr auto zero = std::chrono::duration<Rep, Period>::zero();
        const auto now = std::chrono::steady_clock::now();
        decltype(now + rel_time) abs_time = now;
        if (rel_time > zero) {
            constexpr auto forever = (std::chrono::steady_clock::time_point::max)();
            if (abs_time < forever - rel_time) {
                abs_time += rel_time;
            } else {
                abs_time = forever;
            }
        }
        return abs_time;
    }

    using pal::threading::implements::thread_sleep_for;
}

namespace rainy::foundation::system::this_thread {
    template <typename Clock, typename Duration>
    void sleep_until(const std::chrono::time_point<Clock, Duration> &abs_time) {
#if RAINY_HAS_CXX20
        static_assert(std::chrono::is_clock_v<Clock>, "Clock type required");
#endif
        for (;;) {
            const auto now = Clock::now();
            if (abs_time <= now) {
                return;
            }
            constexpr std::chrono::milliseconds clamp{std::chrono::hours{24}};
            const auto rel = abs_time - now;
            if (rel >= clamp) {
                implements::thread_sleep_for(clamp.count());
            } else {
                const auto rel_ms = std::chrono::ceil<std::chrono::milliseconds>(rel);
                implements::thread_sleep_for(static_cast<unsigned long>(rel_ms.count()));
            }
        }
    }

    template <typename Rep, typename Period>
    void sleep_for(const std::chrono::duration<Rep, Period> &rel_time) {
        sleep_until(implements::to_absolute_time(rel_time));
    }

#if RAINY_USING_LINUX
    RAINY_NODISCARD ::pthread_t get_current_thread() noexcept;
#else
    RAINY_NODISCARD void *get_current_thread() noexcept;
#endif

    RAINY_NODISCARD pal::threading::thread::id get_id() noexcept;

    void yield() noexcept;
}

namespace rainy::foundation::pal::threading {
    class thread::id {
    public:
        id() noexcept = default;

        friend class thread;

    private:
        explicit id(std::uint64_t id) noexcept : id_{id} {
        }

        friend thread::id thread::get_id() const noexcept;
        friend thread::id foundation::system::this_thread::get_id() noexcept;

        friend bool operator==(thread::id left, thread::id right) noexcept;

#if RAINY_HAS_CXX20
        friend std::strong_ordering operator<=>(thread::id left, thread::id right) noexcept {
            return left.id_ <=> right.id_;
        }
#else
        friend bool operator<(thread::id left, thread::id right) noexcept;
#endif

        template <typename Elem, typename Traits>
        friend std::basic_ostream<Elem, Traits> &operator<<(std::basic_ostream<Elem, Traits> &ostream, id id) {
            ostream << id.id_;
            return ostream;
        }

        friend utility::hash<thread::id>;
        friend std::hash<thread::id>;

        std::uint64_t id_{};
    };

#if !RAINY_HAS_CXX20
    RAINY_NODISCARD bool operator!=(thread::id left, thread::id right) noexcept;
    RAINY_NODISCARD bool operator<(thread::id left, thread::id right) noexcept;
    RAINY_NODISCARD bool operator<=(thread::id left, thread::id right) noexcept;
    RAINY_NODISCARD bool operator>(thread::id left, thread::id right) noexcept;
    RAINY_NODISCARD bool operator>=(thread::id left, thread::id right) noexcept;
#endif
}

namespace std {
    template <>
    struct hash<rainy::foundation::pal::threading::thread::id> {
        using argument_type = rainy::foundation::pal::threading::thread::id;
        using result_type = std::uint64_t;

        RAINY_NODISCARD result_type operator()(const argument_type keyval) noexcept {
            return rainy::utility::implements::hash_representation(keyval.id_);
        }
    };
}

namespace rainy::utility {
    template <>
    struct hash<foundation::pal::threading::thread::id> {
        using argument_type = foundation::pal::threading::thread::id;
        using result_type = std::uint64_t;

        RAINY_NODISCARD result_type operator()(const argument_type keyval) noexcept {
            return implements::hash_representation(keyval.id_);
        }
    };
}

namespace rainy::foundation::pal::threading {
    enum class future_errc {
        broken_promise = 1,
        future_already_retrieved,
        promise_already_satisfied,
        no_state
    };

    enum class launch {
        async = 0x1,
        deferred = 0x2
    };

    RAINY_ENABLE_ENUM_CLASS_BITMASK_OPERATORS(launch);

    enum class future_status {
        ready,
        timeout,
        deferred
    };
}

namespace rainy::foundation::pal::threading::implements {
    template <typename Ty>
    class associated_state;

    template <typename Ty>
    struct deleter_bridge {
        virtual void delete_this(associated_state<Ty> *) noexcept = 0;
        virtual ~deleter_bridge() = default;
    };

    template <typename Ty, typename Derived, typename Alloc>
    struct state_deleter : deleter_bridge<Ty> {
        state_deleter(const Alloc &al) : alloc(al) {
        }

        state_deleter(const state_deleter &) = delete;
        state_deleter &operator=(const state_deleter &) = delete;

        void delete_this(associated_state<Ty> *state) noexcept override {
            (void) state;
        }

        Alloc alloc;
    };

    template <class Ty>
    union _Result_holder {
        _Result_holder() noexcept {
        }
        ~_Result_holder() noexcept {
        }

        Ty _Held_value;
    };
}

#endif