#ifndef RAINY_FOUNDATION_IO_FILESYSTEM_STREAMFILE_HPP
#define RAINY_FOUNDATION_IO_FILESYSTEM_STREAMFILE_HPP
#include <rainy/foundation/io/io_context.hpp>
#include <cstdint>
#include <filesystem>
#include <rainy/foundation/io/filesystem/fwd.hpp>
#include <rainy/foundation/io/net/buffer.hpp>
#include <system_error>

namespace rainy::foundation::io::filesystem::implements {
    using io::implements::completion_op;
    using io::implements::op_result;
    using io::implements::io_context_impl_base;

    class file_impl_base {
    public:
        virtual ~file_impl_base() = default;

        /**
         * @brief  打开文件，绑定到给定的 io_context_impl_base
         * @param  path   文件路径
         * @param  mode   打开标志
         * @param  ctx    调用方持有的 io_context 底层 impl
         * @return 错误码，成功时 value()==0
         */
        virtual std::error_code open(const std::filesystem::path &path, open_mode mode,
                                     io_context_impl_base &ctx) noexcept = 0;

        virtual void close() noexcept = 0;

        RAINY_NODISCARD virtual bool is_open() const noexcept = 0;

        /**
         * @brief  在指定偏移处同步读取，不改变任何内部状态
         * @param  buf     目标缓冲区
         * @param  offset  字节偏移（从文件头）
         * @param  ec      错误输出
         * @return 实际读取字节数
         */
        virtual std::size_t read_some_at(net::mutable_buffer buf, std::uint64_t offset, std::error_code &ec) noexcept = 0;

        /**
         * @brief  在指定偏移处同步写入，不改变任何内部状态
         * @param  buf     源缓冲区
         * @param  offset  字节偏移（从文件头）
         * @param  ec      错误输出
         * @return 实际写入字节数
         */
        virtual std::size_t write_some_at(net::const_buffer buf, std::uint64_t offset, std::error_code &ec) noexcept = 0;

        virtual void async_read_some_at(net::mutable_buffer buf, std::uint64_t offset, io_context_impl_base &ctx,
                                        completion_op *op) noexcept = 0;

        virtual void async_write_some_at(net::const_buffer buf, std::uint64_t offset, io_context_impl_base &ctx,
                                         completion_op *op) noexcept = 0;

        RAINY_NODISCARD virtual std::uint64_t size(std::error_code &ec) const noexcept = 0;

        virtual std::error_code resize(std::uint64_t new_size) noexcept = 0;

        RAINY_NODISCARD virtual std::uintptr_t native_handle() const noexcept = 0;
    };

    RAINY_NODISCARD RAINY_TOOLKIT_API memory::nebula_ptr<file_impl_base> make_file_impl();
}

namespace rainy::foundation::io::filesystem {
    class basic_file {
    public:
        using executor_type = io_context::executor_type;

        /**
         * @brief  构造并立即打开文件
         * @param  ctx   驱动异步 I/O 的 io_context
         * @param  path  目标路径
         * @param  mode  打开模式，默认只读
         */
        explicit basic_file(io_context &ctx, const std::filesystem::path &path, open_mode mode = open_mode::read_only) :
            ctx_(&ctx), impl_(implements::make_file_impl()) {
            std::error_code ec = impl_->open(path, mode, ctx_->under_impl());
            if (ec) {
                throw std::system_error(ec, "basic_file::open");
            }
        }

        /**
         * @brief  构造后不立即打开，稍后调用 open()
         */
        explicit basic_file(io_context &ctx) : ctx_(&ctx), impl_(implements::make_file_impl()) {
        }

        basic_file(const basic_file &) = delete;
        basic_file &operator=(const basic_file &) = delete;

        basic_file(basic_file &&) = default;
        basic_file &operator=(basic_file &&) = default;

        ~basic_file() {
            if (impl_ && impl_->is_open()) {
                impl_->close();
            }
        }

        std::error_code open(const std::filesystem::path &path, open_mode mode = open_mode::read_only) {
            return impl_->open(path, mode, ctx_->under_impl());
        }

        void close() noexcept {
            impl_->close();
        }

        RAINY_NODISCARD bool is_open() const noexcept {
            if (!impl_) {
                return false;
            }
            return impl_->is_open();
        }

        /**
         * @brief  在偏移 offset 处读取至多 buf.size() 字节
         *
         * 配合自由函数 read_at(device, offset, buffers, ec) 使用，
         * 该自由函数会循环调用 read_some_at 直到满足完成条件。
         */
        std::size_t read_some_at(std::uint64_t offset, net::mutable_buffer buf, std::error_code &ec) noexcept {
            return impl_->read_some_at(buf, offset, ec);
        }

        std::size_t read_some_at(std::uint64_t offset, net::mutable_buffer buf) {
            std::error_code ec;
            const std::size_t n = read_some_at(offset, buf, ec);
            if (ec) {
                throw std::system_error(ec, "basic_file::read_some_at");
            }
            return n;
        }

        /**
         * @brief  在偏移 offset 处写入至多 buf.size() 字节
         */
        std::size_t write_some_at(std::uint64_t offset, net::const_buffer buf, std::error_code &ec) noexcept {
            return impl_->write_some_at(buf, offset, ec);
        }

        std::size_t write_some_at(std::uint64_t offset, net::const_buffer buf) {
            std::error_code ec;
            const std::size_t n = write_some_at(offset, buf, ec);
            if (ec) {
                throw std::system_error(ec, "basic_file::write_some_at");
            }
            return n;
        }

        template <typename MutableBufferSequence, typename Handler>
        void async_read_some_at(std::uint64_t offset, const MutableBufferSequence &buf, Handler &&handler) {
            ctx_->under_impl().on_work_started();
            auto *op = io::implements::make_io_completion_op(
                [h = utility::forward<Handler>(handler), this](const implements::op_result &res, bool cancelled) mutable {
                    std::error_code ec;
                    if (cancelled) {
                        ec = std::make_error_code(std::errc::operation_canceled);
                    } else if (res.error_code) {
                        ec.assign(res.error_code, std::system_category());
                    }
                    h(ec, res.bytes_transferred);
                    ctx_->under_impl().on_work_finished();
                });

            impl_->async_read_some_at(net::buffer(buf), offset, ctx_->under_impl(), op);
        }

        template <typename ConstBufferSequence, typename Handler>
        void async_write_some_at(std::uint64_t offset, const ConstBufferSequence &buf, Handler &&handler) {
            ctx_->under_impl().on_work_started();
            auto *op = io::implements::make_io_completion_op(
                [h = utility::forward<Handler>(handler), this](const implements::op_result &res, bool cancelled) mutable {
                    std::error_code ec;
                    if (cancelled) {
                        ec = std::make_error_code(std::errc::operation_canceled);
                    } else if (res.error_code) {
                        ec.assign(res.error_code, std::system_category());
                    }
                    h(ec, res.bytes_transferred);
                    ctx_->under_impl().on_work_finished();
                });

            impl_->async_write_some_at(net::buffer(buf), offset, ctx_->under_impl(), op);
        }
        RAINY_NODISCARD std::uint64_t size() const {
            std::error_code ec;
            auto s = impl_->size(ec);
            if (ec) {
                throw std::system_error(ec, "basic_file::size");
            }
            return s;
        }

        RAINY_NODISCARD std::uint64_t size(std::error_code &ec) const noexcept {
            return impl_->size(ec);
        }

        std::error_code resize(std::uint64_t new_size) noexcept {
            return impl_->resize(new_size);
        }

        RAINY_NODISCARD std::uintptr_t native_handle() const noexcept {
            return impl_->native_handle();
        }

        RAINY_NODISCARD executor_type get_executor() noexcept { // NOLINT
            return ctx_->get_executor();
        }

    private:
        io::io_context *ctx_;
        memory::nebula_ptr<implements::file_impl_base> impl_;
    };
}

namespace rainy::foundation::io::filesystem {
    class stream_file {
    public:
        using executor_type = io::io_context::executor_type;

        explicit stream_file(io::io_context &ctx, const std::filesystem::path &path, open_mode mode = open_mode::read_only) :
            file_(ctx, path, mode), offset_(0) {
            if (has_flag(mode, open_mode::append)) {
                std::error_code ec;
                offset_ = file_.size(ec); // 失败时 offset_ 保持 0
            }
        }

        explicit stream_file(io::io_context &ctx) : file_(ctx), offset_(0) {
        }

        stream_file(const stream_file &) = delete;
        stream_file &operator=(const stream_file &) = delete;

        stream_file(stream_file &&o) noexcept : file_(utility::move(o.file_)), offset_(o.offset_) {
            o.offset_ = 0;
        }

        stream_file &operator=(stream_file &&o) noexcept {
            if (this != &o) {
                file_ = utility::move(o.file_);
                offset_ = o.offset_;
                o.offset_ = 0;
            }
            return *this;
        }

        std::error_code open(const std::filesystem::path &path, open_mode mode = open_mode::read_only) {
            auto ec = file_.open(path, mode);
            if (!ec) {
                offset_ = has_flag(mode, open_mode::append) ? file_.size(ec) : 0;
            }
            return ec;
        }

        void close() noexcept {
            file_.close();
        }

        RAINY_NODISCARD bool is_open() const noexcept {
            return file_.is_open();
        }

        /**
         * @brief  移动内部偏移量
         * @return 移动后的绝对偏移，出错时返回 (uint64_t)-1 并设置 ec
         */
        std::uint64_t seek(std::int64_t offset, seek_basis basis, std::error_code &ec) noexcept {
            std::uint64_t new_offset{};
            switch (basis) {
                case seek_basis::begin:
                    if (offset < 0) {
                        ec = std::make_error_code(std::errc::invalid_argument);
                        return static_cast<std::uint64_t>(-1);
                    }
                    new_offset = static_cast<std::uint64_t>(offset);
                    break;
                case seek_basis::current:
                    if (offset < 0 && static_cast<std::uint64_t>(-offset) > offset_) {
                        ec = std::make_error_code(std::errc::invalid_argument);
                        return static_cast<std::uint64_t>(-1);
                    }
                    new_offset = static_cast<std::uint64_t>(static_cast<std::int64_t>(offset_) + offset);
                    break;
                case seek_basis::end: {
                    std::uint64_t sz = file_.size(ec);
                    if (ec)
                        return static_cast<std::uint64_t>(-1);
                    if (offset > 0 || static_cast<std::uint64_t>(-offset) > sz) {
                        ec = std::make_error_code(std::errc::invalid_argument);
                        return static_cast<std::uint64_t>(-1);
                    }
                    new_offset = static_cast<std::uint64_t>(static_cast<std::int64_t>(sz) + offset);
                    break;
                }
            }
            offset_ = new_offset;
            return offset_;
        }

        std::uint64_t seek(std::int64_t offset, seek_basis basis) {
            std::error_code ec;
            auto pos = this->seek(offset, basis, ec);
            if (ec)
                throw std::system_error(ec, "stream_file::seek");
            return pos;
        }

        RAINY_NODISCARD std::uint64_t tell() const noexcept {
            return offset_;
        }

        /**
         * @brief  从当前偏移读取，自动推进 offset_
         *
         * 与自由函数 read(stream, buffers, ec) 对接：
         *   read() 循环调用 read_some 直到缓冲区满或 EOF
         */
        std::size_t read_some(net::mutable_buffer buf, std::error_code &ec) noexcept {
            std::size_t n = file_.read_some_at(offset_, buf, ec);
            offset_ += n;
            return n;
        }

        std::size_t read_some(net::mutable_buffer buf) {
            std::error_code ec;
            std::size_t n = read_some(buf, ec);
            if (ec)
                throw std::system_error(ec, "stream_file::read_some");
            return n;
        }

        /**
         * @brief  向当前偏移写入，自动推进 offset_
         */
        std::size_t write_some(net::const_buffer buf, std::error_code &ec) noexcept {
            std::size_t n = file_.write_some_at(offset_, buf, ec);
            offset_ += n;
            return n;
        }

        std::size_t write_some(net::const_buffer buf) {
            std::error_code ec;
            std::size_t n = write_some(buf, ec);
            if (ec)
                throw std::system_error(ec, "stream_file::write_some");
            return n;
        }

        template <typename MutableBufferSequence, typename Handler>
        void async_read_some(const MutableBufferSequence &buf, Handler &&handler) {
            const std::uint64_t op_offset = offset_;
            const std::size_t op_size = net::buffer_size(buf);
            offset_ += op_size; // 乐观
            file_.async_read_some_at(
                op_offset, buf,
                [this, op_size, h = utility::forward<Handler>(handler)](std::error_code ec, std::size_t transferred) mutable {
                    if (transferred < op_size) {
                        offset_ -= (op_size - transferred);
                    }
                    h(ec, transferred);
                });
        }

        template <typename ConstBufferSequence, typename Handler>
        void async_write_some(const ConstBufferSequence &buf, Handler &&handler) {
            const std::uint64_t op_offset = offset_;
            const std::size_t op_size = net::buffer_size(buf);
            offset_ += op_size;
            file_.async_write_some_at(
                op_offset, buf,
                [this, op_size, h = utility::forward<Handler>(handler)](std::error_code ec, std::size_t transferred) mutable {
                    if (transferred < op_size) {
                        offset_ -= (op_size - transferred);
                    }
                    h(ec, transferred);
                });
        }

        RAINY_NODISCARD std::uint64_t size() const {
            return file_.size();
        }

        RAINY_NODISCARD std::uint64_t size(std::error_code &ec) const noexcept {
            return file_.size(ec);
        }

        std::error_code resize(std::uint64_t sz) noexcept {
            return file_.resize(sz);
        }

        RAINY_NODISCARD std::uintptr_t native_handle() const noexcept {
            return file_.native_handle();
        }
        RAINY_NODISCARD executor_type get_executor() noexcept {
            return file_.get_executor();
        }

    private:
        basic_file file_;
        std::uint64_t offset_;
    };
}

#endif
