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
#ifndef RAINY_FOUNDATION_IO_NET_IMPLEMENTS_HANDLER_TRACKING_HPP
#define RAINY_FOUNDATION_IO_NET_IMPLEMENTS_HANDLER_TRACKING_HPP
#include <rainy/core/core.hpp>

namespace rainy::foundation::io::net::implements {
    class handler_tracking {
    public:
        class completion;

        class tracked_handler {
        protected:
            tracked_handler() = default;
            ~tracked_handler() = default;

        private:
            friend class handler_tracking;
            friend class completion;
            std::uint64_t id_{0};
        };

        static void init() {
            get_state(); // 确保状态被初始化
            write_line("Handler tracking initialized");
        }

        static void creation(execution_context &context, tracked_handler &h, const char *object_type, void *object,
                             uintmax_t native_handle, const char *op_name) {
            auto *state = get_state();
            concurrency::lock_guard lock(state->mutex);
            h.id_ = ++state->next_handler_id;
            const auto now = std::chrono::steady_clock::now();
            write_line("[Handler Creation] ID: {}, Context: {}, Object Type: {}, Object: {}, Native Handle: {}, Operation: {}", h.id_,
                       reinterpret_cast<uintptr_t>(&context), object_type, object, native_handle, op_name);
            state->handler_info[h.id_] = handler_info{object_type, object, native_handle, op_name, now}; // NOLINT
        }

        class completion {
        public:
            explicit completion(const tracked_handler &h) : id_(h.id_) {
                auto *state = get_state();
                concurrency::lock_guard lock(state->mutex);
                // 将这个completion添加到链表中
                next_ = state->completions_head;
                state->completions_head = this;
                write_line("[Completion Created] Handler ID: {}", id_);
            }

            // NOLINTBEGIN
            ~completion() { // NOLINT
                auto *state = get_state();
                concurrency::lock_guard lock(state->mutex);
                // 从链表中移除
                if (state->completions_head == this) {
                    state->completions_head = next_;
                } else {
                    for (auto *curr = state->completions_head; curr; curr = curr->next_) {
                        if (curr->next_ == this) {
                            curr->next_ = next_;
                            break;
                        }
                    }
                }
                if (!invoked_) {
                    write_line("[Warning] Completion destroyed without invocation! Handler ID: {}", id_);
                }
            }
            // NOLINTEND

            void invocation_begin() {
                invoked_ = true;
                auto *state = get_state();
                concurrency::lock_guard lock(state->mutex);

                state->invocation_begin_time[id_] = std::chrono::steady_clock::now();
                write_line("[Invocation Begin] Handler ID: {}", id_);
            }

            void invocation_begin(const std::error_code &ec) {
                invoked_ = true;
                auto *state = get_state();
                concurrency::lock_guard lock(state->mutex);

                state->invocation_begin_time[id_] = std::chrono::steady_clock::now();
                write_line("[Invocation Begin] Handler ID: {}, Error: {} ({})", id_, ec.message(), ec.value());
            }

            void invocation_begin(const std::error_code &ec, std::size_t bytes_transferred) {
                invoked_ = true;
                auto *state = get_state();
                concurrency::lock_guard lock(state->mutex);

                state->invocation_begin_time[id_] = std::chrono::steady_clock::now();
                write_line("[Invocation Begin] Handler ID: {}, Error: {} ({}), Bytes: {}", id_, ec.message(), ec.value(),
                           bytes_transferred);
            }

            void invocation_begin(const std::error_code &ec, int signal_number) {
                invoked_ = true;
                auto *state = get_state();
                concurrency::lock_guard lock(state->mutex);

                state->invocation_begin_time[id_] = std::chrono::steady_clock::now();
                write_line("[Invocation Begin] Handler ID: {}, Error: {} ({}), Signal: {}", id_, ec.message(), ec.value(),
                           signal_number);
            }

            void invocation_begin(const std::error_code &ec, const char *arg) {
                invoked_ = true;
                auto *state = get_state();
                concurrency::lock_guard lock(state->mutex);

                state->invocation_begin_time[id_] = std::chrono::steady_clock::now();
                write_line("[Invocation Begin] Handler ID: {}, Error: {} ({}), Arg: {}", id_, ec.message(), ec.value(), arg);
            }

            void invocation_end() {
                auto *state = get_state();
                concurrency::lock_guard lock(state->mutex);
                if (const auto it = state->invocation_begin_time.find(id_); it != state->invocation_begin_time.end()) {
                    const auto duration = std::chrono::steady_clock::now() - it->second;
                    write_line("[Invocation End] Handler ID: {}, Duration: {} ns", id_,
                               std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count());
                    state->invocation_begin_time.erase(it);
                } else {
                    write_line("[Invocation End] Handler ID: {}", id_);
                }
            }

        private:
            friend class handler_tracking;

            uint64_t id_;
            bool invoked_{false};
            completion *next_{nullptr};
        };

        static void operation(execution_context &context, const char *object_type, void *object, uintmax_t native_handle,
                              const char *op_name) {
            write_line("[Operation] Context: {}, Object Type: {}, Object: {}, Native Handle: {}, Operation: {}",
                       reinterpret_cast<uintptr_t>(&context), object_type, object, native_handle, op_name);
        }

        static void reactor_registration(execution_context &context, uintmax_t native_handle, uintmax_t registration) {
            auto *state = get_state();
            concurrency::lock_guard lock(state->mutex);
            state->reactor_registrations[registration] = reactor_reg_info{native_handle, &context};
            write_line("[Reactor Registration] Context: {}, Native Handle: {}, Registration: {}",
                       reinterpret_cast<uintptr_t>(&context), native_handle, registration);
        }

        static void reactor_deregistration(execution_context &context, uintmax_t native_handle, uintmax_t registration) {
            auto *state = get_state();
            concurrency::lock_guard lock(state->mutex);

            state->reactor_registrations.erase(registration);
            write_line("[Reactor Deregistration] Context: {}, Native Handle: {}, Registration: {}",
                       reinterpret_cast<uintptr_t>(&context), native_handle, registration);
        }

        static void reactor_events(execution_context &context, uintmax_t registration, unsigned events) {
            write_line("[Reactor Events] Context: {}, Registration: {}, Events: {}", reinterpret_cast<uintptr_t>(&context),
                       registration, events);
        }

        static void reactor_operation(const tracked_handler &h, const char *op_name, const std::error_code &ec) {
            write_line("[Reactor Operation] Handler ID: {}, Operation: {}, Error: {} ({})", h.id_, op_name, ec.message(), ec.value());
        }

        static void reactor_operation(const tracked_handler &h, const char *op_name, const std::error_code &ec,
                                      std::size_t bytes_transferred) {
            write_line("[Reactor Operation] Handler ID: {}, Operation: {}, Error: {} ({}), Bytes: {}", h.id_, op_name, ec.message(),
                       ec.value(), bytes_transferred);
        }

        template <typename... Args>
        static void write_line(const text::string_view fmt, Args &&...args) {
            auto str = text::format(fmt, utility::forward<Args>(args)...);
            (void) std::fwrite(str.data(), 1, str.size(), stderr);
            (void) std::fwrite("\n", 1, 1, stderr);
            (void) std::fflush(stderr);
        }

    private:
        struct handler_info {
            const char *object_type{};
            void *object{};
            uintmax_t native_handle{};
            const char *op_name{};
            std::chrono::steady_clock::time_point creation_time;
        };

        struct reactor_reg_info {
            uintmax_t native_handle{};
            execution_context *context{};
        };

        struct tracking_state {
            concurrency::mutex mutex;
            concurrency::atomic<uint64_t> next_handler_id{0};
            std::unordered_map<uint64_t, handler_info> handler_info;
            std::unordered_map<uint64_t, std::chrono::steady_clock::time_point> invocation_begin_time;
            std::unordered_map<uintmax_t, reactor_reg_info> reactor_registrations;
            completion *completions_head{nullptr};
            bool initialized{false};
        };

        static tracking_state *get_state() {
            static tracking_state state;
            return &state;
        }
    };
}

#define NET_TS_HANDLER_INVOCATION_BEGIN(args) tracked_completion.invocation_begin args
#define NET_TS_HANDLER_INVOCATION_END tracked_completion.invocation_end()
# define NET_TS_HANDLER_CREATION(args) handler_tracking::creation args


#endif
