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
#include <rainy/foundation/io/ipc/implements/message_queue_impl.hpp>
#include <rainy/foundation/concurrency/executor.hpp>
#include <cerrno>
#include <cstring>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <ctime>
#include <thread>

namespace rainy::foundation::io::ipc::message_queue::implements {
    using io::implements::op_result;

    static std::error_code posix_error(int e = errno) noexcept {
        return std::error_code{e, std::system_category()};
    }

    static concurrency::executor &get_executor() noexcept {
        return concurrency::get_global_pooled_executor();
    }

    struct sysv_msgbuf {
        long mtype;
        char mtext[1];
    };

    static key_t name_to_key(const char *name) {
        return ::ftok(name, 'q');
    }

    static std::size_t msgbuf_size(std::size_t data_size) {
        return sizeof(long) + data_size;
    }

    class sysv_mq_impl final : public message_queue_impl_base {
    public:
        sysv_mq_impl(int msgid, std::size_t max_msgs, std::size_t max_msg_size) noexcept
            : msgid_(msgid), max_msgs_(max_msgs), max_msg_size_(max_msg_size) {
        }

        ~sysv_mq_impl() override {
            if (is_open()) {
                utility::ignore = close();
            }
        }

        bool is_open() const noexcept override {
            return msgid_ >= 0;
        }

        std::error_code close() noexcept override {
            if (msgid_ < 0) {
                return posix_error(EBADF);
            }
            if (::msgctl(msgid_, IPC_RMID, nullptr) == -1) {
                return posix_error();
            }
            msgid_ = -1;
            return {};
        }

        std::error_code cancel() noexcept override {
            return {};
        }

        native_handle_type native_handle() const noexcept override {
            return static_cast<native_handle_type>(msgid_);
        }

        std::ptrdiff_t send(const void *buf, std::size_t len, priority priority, std::error_code &ec) noexcept override {
            if (msgid_ < 0) {
                ec = posix_error(EBADF);
                return -1;
            }
            auto *msgbuf = static_cast<sysv_msgbuf *>(std::malloc(msgbuf_size(len)));
            if (!msgbuf) {
                ec = posix_error(ENOMEM);
                return -1;
            }
            unsigned int raw = static_cast<unsigned int>(priority);
            msgbuf->mtype = static_cast<long>(raw > 0 ? raw : 1);
            std::memcpy(msgbuf->mtext, buf, len);
            if (::msgsnd(msgid_, msgbuf, len, 0) == -1) {
                std::free(msgbuf);
                ec = posix_error();
                return -1;
            }
            std::free(msgbuf);
            ec.clear();
            return static_cast<std::ptrdiff_t>(len);
        }

        std::ptrdiff_t receive(void *buf, std::size_t len, priority &priority, std::error_code &ec) noexcept override {
            if (msgid_ < 0) {
                ec = posix_error(EBADF);
                return -1;
            }
            auto *msgbuf = static_cast<sysv_msgbuf *>(std::malloc(msgbuf_size(len)));
            if (!msgbuf) {
                ec = posix_error(ENOMEM);
                return -1;
            }
            auto r = ::msgrcv(msgid_, msgbuf, len, 0, 0);
            if (r == -1) {
                std::free(msgbuf);
                ec = posix_error();
                return -1;
            }
            std::memcpy(buf, msgbuf->mtext, static_cast<std::size_t>(r));
            priority = static_cast<priority>(msgbuf->mtype);
            std::free(msgbuf);
            ec.clear();
            return static_cast<std::ptrdiff_t>(r);
        }

        std::ptrdiff_t try_send(const void *buf, std::size_t len, priority priority, std::error_code &ec) noexcept override {
            if (msgid_ < 0) {
                ec = posix_error(EBADF);
                return -1;
            }
            auto *msgbuf = static_cast<sysv_msgbuf *>(std::malloc(msgbuf_size(len)));
            if (!msgbuf) {
                ec = posix_error(ENOMEM);
                return -1;
            }
            unsigned int raw = static_cast<unsigned int>(priority);
            msgbuf->mtype = static_cast<long>(raw > 0 ? raw : 1);
            std::memcpy(msgbuf->mtext, buf, len);
            if (::msgsnd(msgid_, msgbuf, len, IPC_NOWAIT) == -1) {
                std::free(msgbuf);
                if (errno == EAGAIN) {
                    ec = std::make_error_code(std::errc::operation_would_block);
                    return -1;
                }
                ec = posix_error();
                return -1;
            }
            std::free(msgbuf);
            ec.clear();
            return static_cast<std::ptrdiff_t>(len);
        }

        std::ptrdiff_t try_receive(void *buf, std::size_t len, priority &priority, std::error_code &ec) noexcept override {
            if (msgid_ < 0) {
                ec = posix_error(EBADF);
                return -1;
            }
            auto *msgbuf = static_cast<sysv_msgbuf *>(std::malloc(msgbuf_size(len)));
            if (!msgbuf) {
                ec = posix_error(ENOMEM);
                return -1;
            }
            auto r = ::msgrcv(msgid_, msgbuf, len, 0, IPC_NOWAIT);
            if (r == -1) {
                std::free(msgbuf);
                if (errno == ENOMSG) {
                    ec = std::make_error_code(std::errc::operation_would_block);
                    return -1;
                }
                ec = posix_error();
                return -1;
            }
            std::memcpy(buf, msgbuf->mtext, static_cast<std::size_t>(r));
            priority = static_cast<priority>(msgbuf->mtype);
            std::free(msgbuf);
            ec.clear();
            return static_cast<std::ptrdiff_t>(r);
        }

        std::ptrdiff_t timed_send(const void *buf, std::size_t len, priority priority,
                                   std::chrono::milliseconds timeout, std::error_code &ec) noexcept override {
            if (msgid_ < 0) {
                ec = posix_error(EBADF);
                return -1;
            }
            auto deadline = std::chrono::steady_clock::now() + timeout;
            auto *msgbuf = static_cast<sysv_msgbuf *>(std::malloc(msgbuf_size(len)));
            if (!msgbuf) {
                ec = posix_error(ENOMEM);
                return -1;
            }
            unsigned int raw = static_cast<unsigned int>(priority);
            msgbuf->mtype = static_cast<long>(raw > 0 ? raw : 1);
            std::memcpy(msgbuf->mtext, buf, len);
            do {
                if (::msgsnd(msgid_, msgbuf, len, IPC_NOWAIT) == 0) {
                    std::free(msgbuf);
                    ec.clear();
                    return static_cast<std::ptrdiff_t>(len);
                }
                if (errno != EAGAIN) {
                    std::free(msgbuf);
                    ec = posix_error();
                    return -1;
                }
                if (std::chrono::steady_clock::now() >= deadline) {
                    std::free(msgbuf);
                    ec = std::make_error_code(std::errc::timed_out);
                    return -1;
                }
                std::this_thread::sleep_for(std::chrono::milliseconds{10});
            } while (true);
        }

        std::ptrdiff_t timed_receive(void *buf, std::size_t len, priority &priority,
                                      std::chrono::milliseconds timeout, std::error_code &ec) noexcept override {
            if (msgid_ < 0) {
                ec = posix_error(EBADF);
                return -1;
            }
            auto deadline = std::chrono::steady_clock::now() + timeout;
            auto *msgbuf = static_cast<sysv_msgbuf *>(std::malloc(msgbuf_size(len)));
            if (!msgbuf) {
                ec = posix_error(ENOMEM);
                return -1;
            }
            do {
                auto r = ::msgrcv(msgid_, msgbuf, len, 0, IPC_NOWAIT);
                if (r >= 0) {
                    std::memcpy(buf, msgbuf->mtext, static_cast<std::size_t>(r));
                    priority = static_cast<priority>(msgbuf->mtype);
                    std::free(msgbuf);
                    ec.clear();
                    return static_cast<std::ptrdiff_t>(r);
                }
                if (errno != ENOMSG) {
                    std::free(msgbuf);
                    ec = posix_error();
                    return -1;
                }
                if (std::chrono::steady_clock::now() >= deadline) {
                    std::free(msgbuf);
                    ec = std::make_error_code(std::errc::timed_out);
                    return -1;
                }
                std::this_thread::sleep_for(std::chrono::milliseconds{10});
            } while (true);
        }

        void async_send(const void *buf, std::size_t len, priority priority,
                        executor_type executor, completion_op *op) noexcept override {
            int msgid = msgid_;
            unsigned int native_prio = static_cast<unsigned int>(priority);
            auto *buf_copy = new char[len];
            std::memcpy(buf_copy, buf, len);
            get_executor().submit([msgid, buf_copy, len, native_prio, op]() mutable {
                op_result result{};
                result.user_data = op;
                auto *msgbuf = static_cast<sysv_msgbuf *>(std::malloc(msgbuf_size(len)));
                if (!msgbuf) {
                    result.error_code = ENOMEM;
                    result.bytes_transferred = 0;
                    delete[] buf_copy;
                    op->complete(result, false);
                    return;
                }
                msgbuf->mtype = static_cast<long>(native_prio > 0 ? native_prio : 1);
                std::memcpy(msgbuf->mtext, buf_copy, len);
                delete[] buf_copy;
                if (::msgsnd(msgid, msgbuf, len, 0) == -1) {
                    result.error_code = errno;
                    result.bytes_transferred = 0;
                } else {
                    result.error_code = 0;
                    result.bytes_transferred = len;
                }
                std::free(msgbuf);
                op->complete(result, false);
            });
        }

        void async_receive(void *buf, std::size_t len, executor_type executor, completion_op *op) noexcept override {
            int msgid = msgid_;
            get_executor().submit([msgid, buf, len, op]() mutable {
                op_result result{};
                result.user_data = op;
                auto *msgbuf = static_cast<sysv_msgbuf *>(std::malloc(msgbuf_size(len)));
                if (!msgbuf) {
                    result.error_code = ENOMEM;
                    result.bytes_transferred = 0;
                    op->complete(result, false);
                    return;
                }
                auto r = ::msgrcv(msgid, msgbuf, len, 0, 0);
                if (r == -1) {
                    result.error_code = errno;
                    result.bytes_transferred = 0;
                } else {
                    std::memcpy(buf, msgbuf->mtext, static_cast<std::size_t>(r));
                    result.error_code = 0;
                    result.bytes_transferred = static_cast<std::size_t>(r);
                }
                std::free(msgbuf);
                op->complete(result, false);
            });
        }

        std::size_t max_messages() const noexcept override {
            return max_msgs_;
        }

        std::size_t max_message_size() const noexcept override {
            return max_msg_size_;
        }

        std::size_t current_messages() const noexcept override {
            struct msqid_ds ds;
            if (::msgctl(msgid_, IPC_STAT, &ds) == -1) {
                return 0;
            }
            return static_cast<std::size_t>(ds.msg_qnum);
        }

    private:
        int msgid_{-1};
        std::size_t max_msgs_{0};
        std::size_t max_msg_size_{0};
    };

    class null_mq_impl final : public message_queue_impl_base {
    public:
        explicit null_mq_impl(executor_type) noexcept {
        }

        bool is_open() const noexcept override {
            return true;
        }

        std::error_code close() noexcept override {
            return {};
        }

        std::error_code cancel() noexcept override {
            return {};
        }

        native_handle_type native_handle() const noexcept override {
            return static_cast<native_handle_type>(-1);
        }

        std::ptrdiff_t send(const void *, std::size_t len, priority, std::error_code &ec) noexcept override {
            ec.clear();
            return static_cast<std::ptrdiff_t>(len);
        }

        std::ptrdiff_t receive(void *, std::size_t, priority &, std::error_code &ec) noexcept override {
            ec = std::make_error_code(std::errc::no_message_available);
            return -1;
        }

        std::ptrdiff_t try_send(const void *, std::size_t len, priority, std::error_code &ec) noexcept override {
            ec.clear();
            return static_cast<std::ptrdiff_t>(len);
        }

        std::ptrdiff_t try_receive(void *, std::size_t, priority &, std::error_code &ec) noexcept override {
            ec = std::make_error_code(std::errc::no_message_available);
            return -1;
        }

        std::ptrdiff_t timed_send(const void *, std::size_t len, priority,
                                   std::chrono::milliseconds, std::error_code &ec) noexcept override {
            ec.clear();
            return static_cast<std::ptrdiff_t>(len);
        }

        std::ptrdiff_t timed_receive(void *, std::size_t, priority &,
                                      std::chrono::milliseconds, std::error_code &ec) noexcept override {
            ec = std::make_error_code(std::errc::no_message_available);
            return -1;
        }

        void async_send(const void *, std::size_t len, priority, executor_type, completion_op *op) noexcept override {
            op_result result{};
            result.user_data = op;
            result.error_code = 0;
            result.bytes_transferred = len;
            op->complete(result, false);
        }

        void async_receive(void *, std::size_t, executor_type, completion_op *op) noexcept override {
            op_result result{};
            result.user_data = op;
            result.error_code = ENOMSG;
            result.bytes_transferred = 0;
            op->complete(result, false);
        }

        std::size_t max_messages() const noexcept override {
            return 0;
        }

        std::size_t max_message_size() const noexcept override {
            return 0;
        }

        std::size_t current_messages() const noexcept override {
            return 0;
        }
    };

    memory::nebula_ptr<message_queue_impl_base>
    create_message_queue_impl(executor_type executor, const char *name, const attributes &attr, direction dir, std::error_code &ec) {
        auto key = name_to_key(name);
        if (key == static_cast<key_t>(-1)) {
            ec = posix_error();
            return create_null_message_queue_impl(executor);
        }
        int msgflg = IPC_CREAT | IPC_EXCL | 0600;
        std::size_t msgsize = attr.max_message_size > 0 ? attr.max_message_size : 8192;
        std::size_t maxmsgs = attr.max_messages > 0 ? attr.max_messages : 16;
        int msgid = ::msgget(key, msgflg);
        if (msgid == -1) {
            ec = posix_error();
            return create_null_message_queue_impl(executor);
        }
        ec.clear();
        return memory::make_nebula<sysv_mq_impl>(msgid, maxmsgs, msgsize);
    }

    memory::nebula_ptr<message_queue_impl_base>
    open_message_queue_impl(executor_type executor, const char *name, direction dir, std::error_code &ec) {
        auto key = name_to_key(name);
        if (key == static_cast<key_t>(-1)) {
            ec = posix_error();
            return create_null_message_queue_impl(executor);
        }
        int msgid = ::msgget(key, 0);
        if (msgid == -1) {
            ec = posix_error();
            return create_null_message_queue_impl(executor);
        }
        struct msqid_ds ds;
        if (::msgctl(msgid, IPC_STAT, &ds) == -1) {
            ec = posix_error();
            return create_null_message_queue_impl(executor);
        }
        ec.clear();
        return memory::make_nebula<sysv_mq_impl>(msgid, 0, 0);
    }

    memory::nebula_ptr<message_queue_impl_base>
    open_or_create_message_queue_impl(executor_type executor, const char *name, const attributes &attr, direction dir,
                                       std::error_code &ec) {
        auto key = name_to_key(name);
        if (key == static_cast<key_t>(-1)) {
            ec = posix_error();
            return create_null_message_queue_impl(executor);
        }
        int msgflg = IPC_CREAT | 0600;
        std::size_t msgsize = attr.max_message_size > 0 ? attr.max_message_size : 8192;
        std::size_t maxmsgs = attr.max_messages > 0 ? attr.max_messages : 16;
        int msgid = ::msgget(key, msgflg);
        if (msgid == -1) {
            ec = posix_error();
            return create_null_message_queue_impl(executor);
        }
        ec.clear();
        return memory::make_nebula<sysv_mq_impl>(msgid, maxmsgs, msgsize);
    }

    memory::nebula_ptr<message_queue_impl_base> create_null_message_queue_impl(executor_type executor) {
        return memory::make_nebula<null_mq_impl>(executor);
    }

    std::error_code unlink_message_queue(const char *name) noexcept {
        auto key = name_to_key(name);
        if (key == static_cast<key_t>(-1)) {
            return posix_error();
        }
        int msgid = ::msgget(key, 0);
        if (msgid == -1) {
            return posix_error();
        }
        if (::msgctl(msgid, IPC_RMID, nullptr) == -1) {
            return posix_error();
        }
        return {};
    }
}