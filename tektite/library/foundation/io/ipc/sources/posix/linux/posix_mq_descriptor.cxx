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
#include <cerrno>
#include <ctime>
#include <fcntl.h>
#include <mqueue.h>
#include <rainy/foundation/concurrency/executor.hpp>
#include <rainy/foundation/io/ipc/implements/message_queue_impl.hpp>
#include <sys/stat.h>

namespace rainy::foundation::io::ipc::message_queue::implements {
    using io::implements::op_result;

    static std::error_code posix_error(int e = errno) noexcept {
        return std::error_code{e, std::system_category()};
    }

    static concurrency::executor &get_executor() noexcept {
        return concurrency::get_global_pooled_executor();
    }

    static timespec absolute_timeout(std::chrono::milliseconds timeout) {
        timespec ts;
        ::clock_gettime(CLOCK_REALTIME, &ts);
        auto sec = std::chrono::duration_cast<std::chrono::seconds>(timeout);
        auto nsec = std::chrono::duration_cast<std::chrono::nanoseconds>(timeout - sec);
        ts.tv_sec += sec.count();
        ts.tv_nsec += static_cast<long>(nsec.count());
        if (ts.tv_nsec >= 1000000000L) {
            ts.tv_sec += 1;
            ts.tv_nsec -= 1000000000L;
        }
        return ts;
    }

    static const timespec &zero_timeout() noexcept {
        static timespec ts{0, 0};
        return ts;
    }

    class posix_mq_impl final : public message_queue_impl_base {
    public:
        posix_mq_impl(mqd_t mq, std::size_t max_msgs, std::size_t max_msg_size) noexcept :
            mq_(mq), max_msgs_(max_msgs), max_msg_size_(max_msg_size) {
        }

        ~posix_mq_impl() override {
            if (is_open()) {
                utility::ignore = close();
            }
        }

        bool is_open() const noexcept override {
            return mq_ != static_cast<mqd_t>(-1);
        }

        std::error_code close() noexcept override {
            if (mq_ == static_cast<mqd_t>(-1)) {
                return posix_error(EBADF);
            }
            ::mq_close(mq_);
            mq_ = static_cast<mqd_t>(-1);
            return {};
        }

        std::error_code cancel() noexcept override {
            return {};
        }

        native_handle_type native_handle() const noexcept override {
            return static_cast<native_handle_type>(mq_);
        }

        std::ptrdiff_t send(const void *buf, std::size_t len, priority priority, std::error_code &ec) noexcept override {
            if (mq_ == static_cast<mqd_t>(-1)) {
                ec = posix_error(EBADF);
                return -1;
            }
            if (::mq_send(mq_, static_cast<const char *>(buf), len, static_cast<unsigned int>(priority)) == -1) {
                ec = posix_error();
                return -1;
            }
            ec.clear();
            return static_cast<std::ptrdiff_t>(len);
        }

        std::ptrdiff_t receive(void *buf, std::size_t len, priority &priority, std::error_code &ec) noexcept override {
            if (mq_ == static_cast<mqd_t>(-1)) {
                ec = posix_error(EBADF);
                return -1;
            }
            unsigned int native_prio = 0;
            auto r = ::mq_receive(mq_, static_cast<char *>(buf), len, &native_prio);
            if (r == -1) {
                ec = posix_error();
                return -1;
            }
            priority = static_cast<ipc::message_queue::priority>(native_prio);
            ec.clear();
            return static_cast<std::ptrdiff_t>(r);
        }

        std::ptrdiff_t try_send(const void *buf, std::size_t len, priority priority, std::error_code &ec) noexcept override {
            if (mq_ == static_cast<mqd_t>(-1)) {
                ec = posix_error(EBADF);
                return -1;
            }
            if (::mq_timedsend(mq_, static_cast<const char *>(buf), len, static_cast<unsigned int>(priority), &zero_timeout()) == -1) {
                if (errno == ETIMEDOUT) {
                    ec = std::make_error_code(std::errc::operation_would_block);
                    return -1;
                }
                ec = posix_error();
                return -1;
            }
            ec.clear();
            return static_cast<std::ptrdiff_t>(len);
        }

        std::ptrdiff_t try_receive(void *buf, std::size_t len, priority &priority, std::error_code &ec) noexcept override {
            if (mq_ == static_cast<mqd_t>(-1)) {
                ec = posix_error(EBADF);
                return -1;
            }
            unsigned int native_prio = 0;
            auto r = ::mq_timedreceive(mq_, static_cast<char *>(buf), len, &native_prio, &zero_timeout());
            if (r == -1) {
                if (errno == ETIMEDOUT) {
                    ec = std::make_error_code(std::errc::operation_would_block);
                    return -1;
                }
                ec = posix_error();
                return -1;
            }
            priority = static_cast<ipc::message_queue::priority>(native_prio);
            ec.clear();
            return static_cast<std::ptrdiff_t>(r);
        }

        std::ptrdiff_t timed_send(const void *buf, std::size_t len, priority priority, std::chrono::milliseconds timeout,
                                  std::error_code &ec) noexcept override {
            if (mq_ == static_cast<mqd_t>(-1)) {
                ec = posix_error(EBADF);
                return -1;
            }
            auto ts = absolute_timeout(timeout);
            if (::mq_timedsend(mq_, static_cast<const char *>(buf), len, static_cast<unsigned int>(priority), &ts) == -1) {
                if (errno == ETIMEDOUT) {
                    ec = std::make_error_code(std::errc::timed_out);
                    return -1;
                }
                ec = posix_error();
                return -1;
            }
            ec.clear();
            return static_cast<std::ptrdiff_t>(len);
        }

        std::ptrdiff_t timed_receive(void *buf, std::size_t len, priority &priority, std::chrono::milliseconds timeout,
                                     std::error_code &ec) noexcept override {
            if (mq_ == static_cast<mqd_t>(-1)) {
                ec = posix_error(EBADF);
                return -1;
            }
            unsigned int native_prio = 0;
            auto ts = absolute_timeout(timeout);
            auto r = ::mq_timedreceive(mq_, static_cast<char *>(buf), len, &native_prio, &ts);
            if (r == -1) {
                if (errno == ETIMEDOUT) {
                    ec = std::make_error_code(std::errc::timed_out);
                    return -1;
                }
                ec = posix_error();
                return -1;
            }
            priority = static_cast<ipc::message_queue::priority>(native_prio);
            ec.clear();
            return static_cast<std::ptrdiff_t>(r);
        }

        void async_send(const void *buf, std::size_t len, priority priority, executor_type executor,
                        completion_op *op) noexcept override {
            mqd_t mq = mq_;
            unsigned int native_prio = static_cast<unsigned int>(priority);
            auto *buf_copy = new char[len];
            std::memcpy(buf_copy, buf, len);
            get_executor().submit([mq, buf_copy, len, native_prio, op]() mutable {
                op_result result{};
                result.user_data = op;
                if (::mq_send(mq, buf_copy, len, native_prio) == -1) {
                    result.error_code = errno;
                    result.bytes_transferred = 0;
                } else {
                    result.error_code = 0;
                    result.bytes_transferred = len;
                }
                delete[] buf_copy;
                op->complete(result, false);
            });
        }

        void async_receive(void *buf, std::size_t len, executor_type executor, completion_op *op) noexcept override {
            mqd_t mq = mq_;
            get_executor().submit([mq, buf, len, op]() mutable {
                op_result result{};
                result.user_data = op;
                unsigned int native_prio = 0;
                auto r = ::mq_receive(mq, static_cast<char *>(buf), len, &native_prio);
                if (r == -1) {
                    result.error_code = errno;
                    result.bytes_transferred = 0;
                } else {
                    result.error_code = 0;
                    result.bytes_transferred = static_cast<std::size_t>(r);
                }
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
            struct mq_attr attr;
            if (::mq_getattr(mq_, &attr) == -1) {
                return 0;
            }
            return static_cast<std::size_t>(attr.mq_curmsgs);
        }

    private:
        mqd_t mq_{static_cast<mqd_t>(-1)};
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

        std::ptrdiff_t timed_send(const void *, std::size_t len, priority, std::chrono::milliseconds,
                                  std::error_code &ec) noexcept override {
            ec.clear();
            return static_cast<std::ptrdiff_t>(len);
        }

        std::ptrdiff_t timed_receive(void *, std::size_t, priority &, std::chrono::milliseconds,
                                     std::error_code &ec) noexcept override {
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

    core::memory::nebula_ptr<message_queue_impl_base> create_message_queue_impl(executor_type executor, const char *name,
                                                                                const attributes &attr, direction dir,
                                                                                std::error_code &ec) {
        int oflag = O_CREAT | O_EXCL | O_RDWR;
        if (attr.non_blocking) {
            oflag |= O_NONBLOCK;
        }
        struct mq_attr mq_attr_val{};
        mq_attr_val.mq_maxmsg = static_cast<long>(attr.max_messages > 0 ? attr.max_messages : 10);
        mq_attr_val.mq_msgsize = static_cast<long>(attr.max_message_size > 0 ? attr.max_message_size : 8192);
        mqd_t mq = ::mq_open(name, oflag, 0600, &mq_attr_val);
        if (mq == static_cast<mqd_t>(-1)) {
            ec = posix_error();
            return create_null_message_queue_impl(executor);
        }
        ec.clear();
        return core::memory::make_nebula<posix_mq_impl>(mq, static_cast<std::size_t>(mq_attr_val.mq_maxmsg),
                                                        static_cast<std::size_t>(mq_attr_val.mq_msgsize));
    }

    core::memory::nebula_ptr<message_queue_impl_base> open_message_queue_impl(executor_type executor, const char *name, direction dir,
                                                                              std::error_code &ec) {
        int oflag = O_RDWR;
        mqd_t mq = ::mq_open(name, oflag);
        if (mq == static_cast<mqd_t>(-1)) {
            ec = posix_error();
            return create_null_message_queue_impl(executor);
        }
        struct mq_attr attr;
        if (::mq_getattr(mq, &attr) == -1) {
            ec = posix_error();
            ::mq_close(mq);
            return create_null_message_queue_impl(executor);
        }
        ec.clear();
        return core::memory::make_nebula<posix_mq_impl>(mq, static_cast<std::size_t>(attr.mq_maxmsg),
                                                  static_cast<std::size_t>(attr.mq_msgsize));
    }

    core::memory::nebula_ptr<message_queue_impl_base> open_or_create_message_queue_impl(executor_type executor, const char *name,
                                                                                        const attributes &attr, direction dir,
                                                                                        std::error_code &ec) {
        int oflag = O_CREAT | O_RDWR;
        if (attr.non_blocking) {
            oflag |= O_NONBLOCK;
        }
        struct mq_attr mq_attr_val{};
        mq_attr_val.mq_maxmsg = static_cast<long>(attr.max_messages > 0 ? attr.max_messages : 10);
        mq_attr_val.mq_msgsize = static_cast<long>(attr.max_message_size > 0 ? attr.max_message_size : 8192);
        mqd_t mq = ::mq_open(name, oflag, 0600, &mq_attr_val);
        if (mq == static_cast<mqd_t>(-1)) {
            ec = posix_error();
            return create_null_message_queue_impl(executor);
        }
        struct mq_attr mq_attr_result{};
        if (::mq_getattr(mq, &mq_attr_result) == -1) {
            ec = posix_error();
            ::mq_close(mq);
            return create_null_message_queue_impl(executor);
        }
        ec.clear();
        return core::memory::make_nebula<posix_mq_impl>(mq, static_cast<std::size_t>(mq_attr_result.mq_maxmsg),
                                                        static_cast<std::size_t>(mq_attr_result.mq_msgsize));
    }

    core::memory::nebula_ptr<message_queue_impl_base> create_null_message_queue_impl(executor_type executor) {
        return core::memory::make_nebula<null_mq_impl>(executor);
    }

    std::error_code unlink_message_queue(const char *name) noexcept {
        if (::mq_unlink(name) == -1) {
            return posix_error();
        }
        return {};
    }
}
