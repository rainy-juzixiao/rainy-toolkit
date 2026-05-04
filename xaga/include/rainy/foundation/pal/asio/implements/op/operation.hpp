#ifndef RAINY_FOUNDATION_PAL_ASIO_IMPLEMENTS_OP_OPERATION_HPP
#define RAINY_FOUNDATION_PAL_ASIO_IMPLEMENTS_OP_OPERATION_HPP
#include <rainy/core/core.hpp>

namespace rainy::foundation::pal::asio {
    class scheduler;
}

namespace rainy::foundation::pal::asio::implements {
    class op_queue_access;
}

#if RAINY_USING_WINDOWS
#include <windows.h>

namespace rainy::foundation::pal::asio::implements {
    class win_operation : public OVERLAPPED {
    public:
        using operation_type = win_operation;

        void complete(void *owner, const std::error_code &ec, std::size_t bytes_transferred) {
            func_(owner, this, ec, bytes_transferred);
        }

        void destroy() {
            func_(0, this, std::error_code(), 0);
        }

        void reset() {
            Internal = 0;
            InternalHigh = 0;
            Offset = 0;
            OffsetHigh = 0;
            hEvent = 0;
            ready_ = 0;
        }

    protected:
        using func_type = void (*)(void *, win_operation *, const std::error_code &, std::size_t);

        win_operation(func_type func) : next_(0), func_(func) {
            reset();
        }

        ~win_operation() {
        }

    private:
        friend class op_queue_access;
        friend class win_iocp_io_context;
        win_operation *next_;
        func_type func_;
        long ready_;
    };

    using operation = win_operation;
}

#else

namespace rainy::foundation::pal::asio::implements {
    class scheduler_operation {
    public:
        typedef scheduler_operation operation_type;

        void complete(void *owner, const std::error_code &ec, std::size_t bytes_transferred) {
            func_(owner, this, ec, bytes_transferred);
        }

        void destroy() {
            func_(0, this, std::error_code(), 0);
        }

    protected:
        using func_type = void (*)(void *, scheduler_operation *, const std::error_code &, std::size_t);

        scheduler_operation(func_type func) : next_(nullptr), func_(func), task_result_(0) {
        }

        // Prevents deletion through this type.
        ~scheduler_operation() {
        }

    private:
        friend class op_queue_access;
        scheduler_operation *next_;
        func_type func_;

    protected:
        friend class scheduler;
        unsigned int task_result_;
    };

    using operation = scheduler_operation;
}

#endif

#endif