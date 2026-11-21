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

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <rainy/core/text/string.hpp>
#include <rainy/foundation/concurrency/executor.hpp>
#include <rainy/foundation/io/ipc/implements/message_queue_impl.hpp>
#include <windows.h>

namespace rainy::foundation::io::ipc::message_queue::implements {
    using io::implements::op_result;

    static std::error_code win_error(DWORD e = ::GetLastError()) noexcept {
        return std::error_code{static_cast<int>(e), std::system_category()};
    }

    static concurrency::executor &get_executor() noexcept {
        return concurrency::get_global_pooled_executor();
    }

    static rainy::core::text::wstring to_wide(const char *name) {
        int n = ::MultiByteToWideChar(CP_UTF8, 0, name, -1, nullptr, 0);
        rainy::core::text::wstring wname;
        if (n > 1) {
            wname.resize(static_cast<rainy::core::text::wstring::size_type>(n - 1));
            ::MultiByteToWideChar(CP_UTF8, 0, name, -1, wname.data(), n);
        }
        return wname;
    }

    static rainy::core::text::wstring safe_name(const char *name) {
        auto wname = to_wide(name);
        for (auto &c : wname) {
            if (c == L'/' || c == L'\\') {
                c = L'_';
            }
        }
        return wname;
    }

    static rainy::core::text::wstring mapping_name(const char *name) {
        auto n = safe_name(name);
        n.insert(0, L"Global\\rainy_mq_shm_");
        return n;
    }

    static rainy::core::text::wstring mutex_name(const char *name) {
        auto n = safe_name(name);
        n.insert(0, L"Global\\rainy_mq_mtx_");
        return n;
    }

    static rainy::core::text::wstring data_event_name(const char *name) {
        auto n = safe_name(name);
        n.insert(0, L"Global\\rainy_mq_data_");
        return n;
    }

    static rainy::core::text::wstring space_event_name(const char *name) {
        auto n = safe_name(name);
        n.insert(0, L"Global\\rainy_mq_space_");
        return n;
    }

    struct mq_shared_header {
        std::uint32_t magic;
        std::uint32_t max_messages;
        std::uint32_t max_msg_size;
        std::uint32_t head;
        std::uint32_t tail;
        std::uint32_t count;
    };

    static constexpr std::uint32_t MQ_MAGIC = 0x524E514D;

    static std::size_t shared_mem_size(std::size_t max_msgs, std::size_t max_msg_size) {
        return sizeof(mq_shared_header) + max_msgs * (max_msg_size + sizeof(std::uint32_t) + sizeof(std::uint16_t));
    }

    static std::size_t slot_size(std::size_t max_msg_size) {
        return max_msg_size + sizeof(std::uint32_t) + sizeof(std::uint16_t);
    }

    static std::byte *slot_ptr(std::byte *base, std::size_t slot_sz, std::uint32_t index) {
        return base + sizeof(mq_shared_header) + index * slot_sz;
    }

    static void slot_write_data(std::byte *slot, const void *data, std::uint32_t size, std::uint16_t prio) {
        std::memcpy(slot, &size, sizeof(size));
        std::memcpy(slot + sizeof(std::uint32_t), &prio, sizeof(prio));
        std::memcpy(slot + sizeof(std::uint32_t) + sizeof(std::uint16_t), data, size);
    }

    static std::uint32_t slot_read_size(const std::byte *slot) {
        std::uint32_t size;
        std::memcpy(&size, slot, sizeof(size));
        return size;
    }

    static std::uint16_t slot_read_priority(const std::byte *slot) {
        std::uint16_t prio;
        std::memcpy(&prio, slot + sizeof(std::uint32_t), sizeof(prio));
        return prio;
    }

    static const std::byte *slot_data_ptr(const std::byte *slot) {
        return slot + sizeof(std::uint32_t) + sizeof(std::uint16_t);
    }

    static std::byte *slot_data_ptr(std::byte *slot) {
        return slot + sizeof(std::uint32_t) + sizeof(std::uint16_t);
    }

    class win_shared_mq_impl final : public message_queue_impl_base {
    public:
        win_shared_mq_impl(HANDLE mapping, void *view, HANDLE mtx, HANDLE data_evt, HANDLE space_evt,
                          std::size_t max_msgs, std::size_t max_msg_sz) noexcept
            : mapping_(mapping), view_(static_cast<std::byte *>(view)), mtx_(mtx),
              data_evt_(data_evt), space_evt_(space_evt),
              max_msgs_(max_msgs), max_msg_size_(max_msg_sz),
              slot_sz_(slot_size(max_msg_sz)) {
            header_ = static_cast<mq_shared_header *>(view);
        }

        ~win_shared_mq_impl() override {
            if (is_open()) {
                utility::ignore = close();
            }
        }

        bool is_open() const noexcept override {
            return view_ != nullptr;
        }

        std::error_code close() noexcept override {
            if (view_) {
                ::UnmapViewOfFile(view_);
                view_ = nullptr;
                header_ = nullptr;
            }
            if (mapping_ != nullptr && mapping_ != INVALID_HANDLE_VALUE) {
                ::CloseHandle(mapping_);
                mapping_ = nullptr;
            }
            if (mtx_ != nullptr && mtx_ != INVALID_HANDLE_VALUE) {
                ::CloseHandle(mtx_);
                mtx_ = nullptr;
            }
            if (data_evt_ != nullptr && data_evt_ != INVALID_HANDLE_VALUE) {
                ::CloseHandle(data_evt_);
                data_evt_ = nullptr;
            }
            if (space_evt_ != nullptr && space_evt_ != INVALID_HANDLE_VALUE) {
                ::CloseHandle(space_evt_);
                space_evt_ = nullptr;
            }
            return {};
        }

        std::error_code cancel() noexcept override {
            return {};
        }

        native_handle_type native_handle() const noexcept override {
            return reinterpret_cast<native_handle_type>(mapping_);
        }

        std::ptrdiff_t send(const void *buf, std::size_t len, priority prio, std::error_code &ec) noexcept override {
            if (!view_) {
                ec.assign(ERROR_INVALID_HANDLE, std::system_category());
                return -1;
            }
            if (::WaitForSingleObject(mtx_, INFINITE) != WAIT_OBJECT_0) {
                ec = win_error();
                return -1;
            }
            while (header_->count >= header_->max_messages) {
                ::ReleaseMutex(mtx_);
                if (::WaitForSingleObject(space_evt_, INFINITE) != WAIT_OBJECT_0) {
                    ec = win_error();
                    return -1;
                }
                if (::WaitForSingleObject(mtx_, INFINITE) != WAIT_OBJECT_0) {
                    ec = win_error();
                    return -1;
                }
            }
            auto *slot = slot_ptr(view_, slot_sz_, header_->head);
            slot_write_data(slot, buf, static_cast<std::uint32_t>(len), static_cast<std::uint16_t>(prio));
            header_->head = (header_->head + 1) % header_->max_messages;
            header_->count++;
            ::SetEvent(data_evt_);
            ::ReleaseMutex(mtx_);
            ec.clear();
            return static_cast<std::ptrdiff_t>(len);
        }

        std::ptrdiff_t receive(void *buf, std::size_t len, priority &prio, std::error_code &ec) noexcept override {
            if (!view_) {
                ec.assign(ERROR_INVALID_HANDLE, std::system_category());
                return -1;
            }
            if (::WaitForSingleObject(mtx_, INFINITE) != WAIT_OBJECT_0) {
                ec = win_error();
                return -1;
            }
            while (header_->count == 0) {
                ::ReleaseMutex(mtx_);
                if (::WaitForSingleObject(data_evt_, INFINITE) != WAIT_OBJECT_0) {
                    ec = win_error();
                    return -1;
                }
                if (::WaitForSingleObject(mtx_, INFINITE) != WAIT_OBJECT_0) {
                    ec = win_error();
                    return -1;
                }
            }
            auto *slot = slot_ptr(view_, slot_sz_, header_->tail);
            std::uint32_t msg_size = slot_read_size(slot);
            std::uint16_t native_prio = slot_read_priority(slot);
            std::size_t copy_len = (static_cast<std::size_t>(msg_size) < len) ? static_cast<std::size_t>(msg_size) : len;
            std::memcpy(buf, slot_data_ptr(slot), copy_len);
            prio = static_cast<ipc::message_queue::priority>(native_prio);
            header_->tail = (header_->tail + 1) % header_->max_messages;
            header_->count--;
            ::SetEvent(space_evt_);
            ::ReleaseMutex(mtx_);
            ec.clear();
            return static_cast<std::ptrdiff_t>(copy_len);
        }

        std::ptrdiff_t try_send(const void *buf, std::size_t len, priority prio, std::error_code &ec) noexcept override {
            if (!view_) {
                ec.assign(ERROR_INVALID_HANDLE, std::system_category());
                return -1;
            }
            if (::WaitForSingleObject(mtx_, INFINITE) != WAIT_OBJECT_0) {
                ec = win_error();
                return -1;
            }
            if (header_->count >= header_->max_messages) {
                ::ReleaseMutex(mtx_);
                ec = std::make_error_code(std::errc::operation_would_block);
                return -1;
            }
            auto *slot = slot_ptr(view_, slot_sz_, header_->head);
            slot_write_data(slot, buf, static_cast<std::uint32_t>(len), static_cast<std::uint16_t>(prio));
            header_->head = (header_->head + 1) % header_->max_messages;
            header_->count++;
            ::SetEvent(data_evt_);
            ::ReleaseMutex(mtx_);
            ec.clear();
            return static_cast<std::ptrdiff_t>(len);
        }

        std::ptrdiff_t try_receive(void *buf, std::size_t len, priority &prio, std::error_code &ec) noexcept override {
            if (!view_) {
                ec.assign(ERROR_INVALID_HANDLE, std::system_category());
                return -1;
            }
            if (::WaitForSingleObject(mtx_, INFINITE) != WAIT_OBJECT_0) {
                ec = win_error();
                return -1;
            }
            if (header_->count == 0) {
                ::ReleaseMutex(mtx_);
                ec = std::make_error_code(std::errc::operation_would_block);
                return -1;
            }
            auto *slot = slot_ptr(view_, slot_sz_, header_->tail);
            std::uint32_t msg_size = slot_read_size(slot);
            std::uint16_t native_prio = slot_read_priority(slot);
            std::size_t copy_len = (static_cast<std::size_t>(msg_size) < len) ? static_cast<std::size_t>(msg_size) : len;
            std::memcpy(buf, slot_data_ptr(slot), copy_len);
            prio = static_cast<ipc::message_queue::priority>(native_prio);
            header_->tail = (header_->tail + 1) % header_->max_messages;
            header_->count--;
            ::SetEvent(space_evt_);
            ::ReleaseMutex(mtx_);
            ec.clear();
            return static_cast<std::ptrdiff_t>(copy_len);
        }

        std::ptrdiff_t timed_send(const void *buf, std::size_t len, priority prio, std::chrono::milliseconds timeout,
                                  std::error_code &ec) noexcept override {
            if (!view_) {
                ec.assign(ERROR_INVALID_HANDLE, std::system_category());
                return -1;
            }
            if (::WaitForSingleObject(mtx_, INFINITE) != WAIT_OBJECT_0) {
                ec = win_error();
                return -1;
            }
            while (header_->count >= header_->max_messages) {
                ::ReleaseMutex(mtx_);
                DWORD wait = ::WaitForSingleObject(space_evt_, static_cast<DWORD>(timeout.count()));
                if (wait == WAIT_TIMEOUT) {
                    ec = std::make_error_code(std::errc::timed_out);
                    return -1;
                }
                if (wait != WAIT_OBJECT_0) {
                    ec = win_error();
                    return -1;
                }
                if (::WaitForSingleObject(mtx_, INFINITE) != WAIT_OBJECT_0) {
                    ec = win_error();
                    return -1;
                }
            }
            auto *slot = slot_ptr(view_, slot_sz_, header_->head);
            slot_write_data(slot, buf, static_cast<std::uint32_t>(len), static_cast<std::uint16_t>(prio));
            header_->head = (header_->head + 1) % header_->max_messages;
            header_->count++;
            ::SetEvent(data_evt_);
            ::ReleaseMutex(mtx_);
            ec.clear();
            return static_cast<std::ptrdiff_t>(len);
        }

        std::ptrdiff_t timed_receive(void *buf, std::size_t len, priority &prio, std::chrono::milliseconds timeout,
                                     std::error_code &ec) noexcept override {
            if (!view_) {
                ec.assign(ERROR_INVALID_HANDLE, std::system_category());
                return -1;
            }
            if (::WaitForSingleObject(mtx_, INFINITE) != WAIT_OBJECT_0) {
                ec = win_error();
                return -1;
            }
            while (header_->count == 0) {
                ::ReleaseMutex(mtx_);
                DWORD wait = ::WaitForSingleObject(data_evt_, static_cast<DWORD>(timeout.count()));
                if (wait == WAIT_TIMEOUT) {
                    ec = std::make_error_code(std::errc::timed_out);
                    return -1;
                }
                if (wait != WAIT_OBJECT_0) {
                    ec = win_error();
                    return -1;
                }
                if (::WaitForSingleObject(mtx_, INFINITE) != WAIT_OBJECT_0) {
                    ec = win_error();
                    return -1;
                }
            }
            auto *slot = slot_ptr(view_, slot_sz_, header_->tail);
            std::uint32_t msg_size = slot_read_size(slot);
            std::uint16_t native_prio = slot_read_priority(slot);
            std::size_t copy_len = (static_cast<std::size_t>(msg_size) < len) ? static_cast<std::size_t>(msg_size) : len;
            std::memcpy(buf, slot_data_ptr(slot), copy_len);
            prio = static_cast<ipc::message_queue::priority>(native_prio);
            header_->tail = (header_->tail + 1) % header_->max_messages;
            header_->count--;
            ::SetEvent(space_evt_);
            ::ReleaseMutex(mtx_);
            ec.clear();
            return static_cast<std::ptrdiff_t>(copy_len);
        }

        void async_send(const void *buf, std::size_t len, priority prio, executor_type executor,
                        completion_op *op) noexcept override {
            auto *buf_copy = new char[len];
            std::memcpy(buf_copy, buf, len);
            get_executor().submit([this, buf_copy, len, prio, op]() mutable {
                op_result result{};
                result.user_data = op;
                priority p = prio;
                std::error_code ec;
                auto r = send(buf_copy, len, p, ec);
                if (ec) {
                    result.error_code = ec.value();
                    result.bytes_transferred = 0;
                } else {
                    result.error_code = 0;
                    result.bytes_transferred = static_cast<std::size_t>(r);
                }
                delete[] buf_copy;
                op->complete(result, false);
            });
        }

        void async_receive(void *buf, std::size_t len, executor_type executor, completion_op *op) noexcept override {
            get_executor().submit([this, buf, len, op]() mutable {
                op_result result{};
                result.user_data = op;
                priority p{};
                std::error_code ec;
                auto r = receive(buf, len, p, ec);
                if (ec) {
                    result.error_code = ec.value();
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
            if (!view_) {
                return 0;
            }
            if (::WaitForSingleObject(mtx_, INFINITE) != WAIT_OBJECT_0) {
                return 0;
            }
            std::size_t cnt = static_cast<std::size_t>(header_->count);
            ::ReleaseMutex(mtx_);
            return cnt;
        }

    private:
        HANDLE mapping_{nullptr};
        std::byte *view_{nullptr};
        mq_shared_header *header_{nullptr};
        HANDLE mtx_{nullptr};
        HANDLE data_evt_{nullptr};
        HANDLE space_evt_{nullptr};
        std::size_t max_msgs_{0};
        std::size_t max_msg_size_{0};
        std::size_t slot_sz_{0};
    };

    static bool create_kernel_objects(const char *name, const attributes &attr,
                                      HANDLE &mapping, void *&view, HANDLE &mtx, HANDLE &data_evt, HANDLE &space_evt,
                                      std::error_code &ec) {
        std::size_t shm_size = shared_mem_size(attr.max_messages, attr.max_message_size);

        auto shm_name = mapping_name(name);
        mapping = ::CreateFileMappingW(INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE,
                                       static_cast<DWORD>(shm_size >> 32), static_cast<DWORD>(shm_size),
                                       shm_name.c_str());
        if (!mapping) {
            ec = win_error();
            return false;
        }

        if (::GetLastError() == ERROR_ALREADY_EXISTS) {
            ::CloseHandle(mapping);
            ec.assign(ERROR_ALREADY_EXISTS, std::system_category());
            return false;
        }

        auto mtx_name = mutex_name(name);
        mtx = ::CreateMutexW(nullptr, FALSE, mtx_name.c_str());
        if (!mtx) {
            ec = win_error();
            ::CloseHandle(mapping);
            return false;
        }

        auto data_name = data_event_name(name);
        data_evt = ::CreateEventW(nullptr, FALSE, FALSE, data_name.c_str());
        if (!data_evt) {
            ec = win_error();
            ::CloseHandle(mtx);
            ::CloseHandle(mapping);
            return false;
        }

        auto space_name = space_event_name(name);
        space_evt = ::CreateEventW(nullptr, FALSE, FALSE, space_name.c_str());
        if (!space_evt) {
            ec = win_error();
            ::CloseHandle(data_evt);
            ::CloseHandle(mtx);
            ::CloseHandle(mapping);
            return false;
        }

        view = ::MapViewOfFile(mapping, FILE_MAP_ALL_ACCESS, 0, 0, shm_size);
        if (!view) {
            ec = win_error();
            ::CloseHandle(space_evt);
            ::CloseHandle(data_evt);
            ::CloseHandle(mtx);
            ::CloseHandle(mapping);
            return false;
        }

        auto *header = static_cast<mq_shared_header *>(view);
        header->magic = MQ_MAGIC;
        header->max_messages = static_cast<std::uint32_t>(attr.max_messages);
        header->max_msg_size = static_cast<std::uint32_t>(attr.max_message_size);
        header->head = 0;
        header->tail = 0;
        header->count = 0;

        ec.clear();
        return true;
    }

    static bool open_kernel_objects(const char *name, HANDLE &mapping, HANDLE &mtx, HANDLE &data_evt, HANDLE &space_evt,
                                    std::error_code &ec) {
        auto shm_name = mapping_name(name);
        mapping = ::OpenFileMappingW(FILE_MAP_ALL_ACCESS, FALSE, shm_name.c_str());
        if (!mapping) {
            ec = win_error();
            return false;
        }

        auto mtx_name = mutex_name(name);
        mtx = ::OpenMutexW(SYNCHRONIZE, FALSE, mtx_name.c_str());
        if (!mtx) {
            ec = win_error();
            ::CloseHandle(mapping);
            return false;
        }

        auto data_name = data_event_name(name);
        data_evt = ::OpenEventW(EVENT_MODIFY_STATE | SYNCHRONIZE, FALSE, data_name.c_str());
        if (!data_evt) {
            ec = win_error();
            ::CloseHandle(mtx);
            ::CloseHandle(mapping);
            return false;
        }

        auto space_name = space_event_name(name);
        space_evt = ::OpenEventW(EVENT_MODIFY_STATE | SYNCHRONIZE, FALSE, space_name.c_str());
        if (!space_evt) {
            ec = win_error();
            ::CloseHandle(data_evt);
            ::CloseHandle(mtx);
            ::CloseHandle(mapping);
            return false;
        }

        ec.clear();
        return true;
    }

    core::memory::nebula_ptr<message_queue_impl_base> create_message_queue_impl(executor_type executor, const char *name,
                                                                                const attributes &attr, direction dir,
                                                                                std::error_code &ec) {
        HANDLE mapping = nullptr, mtx = nullptr, data_evt = nullptr, space_evt = nullptr;
        void *view = nullptr;
        if (!create_kernel_objects(name, attr, mapping, view, mtx, data_evt, space_evt, ec)) {
            return create_null_message_queue_impl(executor);
        }
        return core::memory::make_nebula<win_shared_mq_impl>(mapping, view, mtx, data_evt, space_evt,
                                                             attr.max_messages, attr.max_message_size);
    }

    core::memory::nebula_ptr<message_queue_impl_base> open_message_queue_impl(executor_type executor, const char *name, direction dir,
                                                                              std::error_code &ec) {
        HANDLE mapping = nullptr, mtx = nullptr, data_evt = nullptr, space_evt = nullptr;
        if (!open_kernel_objects(name, mapping, mtx, data_evt, space_evt, ec)) {
            return create_null_message_queue_impl(executor);
        }

        void *view = ::MapViewOfFile(mapping, FILE_MAP_ALL_ACCESS, 0, 0, 0);
        if (!view) {
            ec = win_error();
            ::CloseHandle(space_evt);
            ::CloseHandle(data_evt);
            ::CloseHandle(mtx);
            ::CloseHandle(mapping);
            return create_null_message_queue_impl(executor);
        }

        auto *header = static_cast<mq_shared_header *>(view);
        if (header->magic != MQ_MAGIC) {
            ec.assign(ERROR_INVALID_DATA, std::system_category());
            ::UnmapViewOfFile(view);
            ::CloseHandle(space_evt);
            ::CloseHandle(data_evt);
            ::CloseHandle(mtx);
            ::CloseHandle(mapping);
            return create_null_message_queue_impl(executor);
        }

        std::size_t max_msgs = static_cast<std::size_t>(header->max_messages);
        std::size_t max_msg_sz = static_cast<std::size_t>(header->max_msg_size);

        return core::memory::make_nebula<win_shared_mq_impl>(mapping, view, mtx, data_evt, space_evt,
                                                             max_msgs, max_msg_sz);
    }

    core::memory::nebula_ptr<message_queue_impl_base> open_or_create_message_queue_impl(executor_type executor, const char *name,
                                                                                        const attributes &attr, direction dir,
                                                                                        std::error_code &ec) {
        std::error_code open_ec;
        auto result = open_message_queue_impl(executor, name, dir, open_ec);
        if (!open_ec) {
            return result;
        }
        return create_message_queue_impl(executor, name, attr, dir, ec);
    }

    core::memory::nebula_ptr<message_queue_impl_base> create_null_message_queue_impl(executor_type executor) {
        class null_mq_impl final : public message_queue_impl_base {
        public:
            explicit null_mq_impl(executor_type) noexcept {}

            bool is_open() const noexcept override { return true; }
            std::error_code close() noexcept override { return {}; }
            std::error_code cancel() noexcept override { return {}; }
            native_handle_type native_handle() const noexcept override {
                return reinterpret_cast<native_handle_type>(INVALID_HANDLE_VALUE);
            }

            std::ptrdiff_t send(const void *, std::size_t len, priority, std::error_code &ec) noexcept override {
                ec.clear();
                return static_cast<std::ptrdiff_t>(len);
            }

            std::ptrdiff_t receive(void *, std::size_t, priority &, std::error_code &ec) noexcept override {
                ec.assign(ERROR_NO_DATA, std::system_category());
                return -1;
            }

            std::ptrdiff_t try_send(const void *, std::size_t len, priority, std::error_code &ec) noexcept override {
                ec.clear();
                return static_cast<std::ptrdiff_t>(len);
            }

            std::ptrdiff_t try_receive(void *, std::size_t, priority &, std::error_code &ec) noexcept override {
                ec.assign(ERROR_NO_DATA, std::system_category());
                return -1;
            }

            std::ptrdiff_t timed_send(const void *, std::size_t len, priority, std::chrono::milliseconds,
                                      std::error_code &ec) noexcept override {
                ec.clear();
                return static_cast<std::ptrdiff_t>(len);
            }

            std::ptrdiff_t timed_receive(void *, std::size_t, priority &, std::chrono::milliseconds,
                                         std::error_code &ec) noexcept override {
                ec.assign(ERROR_NO_DATA, std::system_category());
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
                result.error_code = ERROR_NO_DATA;
                result.bytes_transferred = 0;
                op->complete(result, false);
            }

            std::size_t max_messages() const noexcept override { return 0; }
            std::size_t max_message_size() const noexcept override { return 0; }
            std::size_t current_messages() const noexcept override { return 0; }
        };
        return core::memory::make_nebula<null_mq_impl>(executor);
    }

    std::error_code unlink_message_queue(const char *name) noexcept {
        return {};
    }
}