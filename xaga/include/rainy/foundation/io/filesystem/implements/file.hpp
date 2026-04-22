#ifndef RAINY_FOUNDATION_IO_FILESYSTEM_IMPLEMENTS_FILE_HPP
#define RAINY_FOUNDATION_IO_FILESYSTEM_IMPLEMENTS_FILE_HPP
#include <rainy/foundation/io/filesystem/fwd.hpp>
#include <rainy/foundation/io/implements/io_context.hpp>

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
        virtual std::size_t read_some_at(mutable_buffer buf, std::uint64_t offset, std::error_code &ec) noexcept = 0;

        /**
         * @brief  在指定偏移处同步写入，不改变任何内部状态
         * @param  buf     源缓冲区
         * @param  offset  字节偏移（从文件头）
         * @param  ec      错误输出
         * @return 实际写入字节数
         */
        virtual std::size_t write_some_at(const_buffer buf, std::uint64_t offset, std::error_code &ec) noexcept = 0;

        virtual void async_read_some_at(mutable_buffer buf, std::uint64_t offset, io_context_impl_base &ctx,
                                        completion_op *op) noexcept = 0;

        virtual void async_write_some_at(const_buffer buf, std::uint64_t offset, io_context_impl_base &ctx,
                                         completion_op *op) noexcept = 0;

        RAINY_NODISCARD virtual std::uint64_t size(std::error_code &ec) const noexcept = 0;

        virtual std::error_code resize(std::uint64_t new_size) noexcept = 0;

        RAINY_NODISCARD virtual std::uintptr_t native_handle() const noexcept = 0;
    };

    RAINY_NODISCARD RAINY_TOOLKIT_API memory::nebula_ptr<file_impl_base> make_file_impl();
}

#endif