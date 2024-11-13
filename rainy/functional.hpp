#ifndef RAINY_FUNCTIONAL_HPP
#define RAINY_FUNCTIONAL_HPP
#include <functional>
#include <rainy/base.hpp>
#include <rainy/meta/type_traits.hpp>
#include <rainy/system/exceptions.hpp>

namespace rainy::foundation::system::exceptions::runtime {
    class invalid_delegate : public runtime_error {
    public:
        using base = runtime_error;

        invalid_delegate(const char *message, source loc = source::current()) :
            base(std::format("invalid delegate,reason : {}", message).c_str(), loc) {
        }
    };

    class internal_error : public runtime_error {
    public:
        using base = runtime_error;

        internal_error(const char *message, const source &loc = source::current()) :
            base(std::format("we found a error,reason : {}", message).c_str(), loc) {
        }
    };

    class invoke_error : public runtime_error {
    public:
        using base = runtime_error;

        invoke_error(const char *message, source loc = source::current()) :
            base(std::format("we can't invoke this object,reason : {}", message).c_str(), loc) {
        }
    };

    void throw_invoke_error(const char *message, utility::source_location loc = utility::source_location::current()) {
        throw_exception(invoke_error(message, loc));
    }
}

namespace rainy::foundation::functional::internals {
    template <typename Rx, typename... Args>
    class delegate_abstract { // abstract base for implementation types
    public:
        delegate_abstract() = default;
        delegate_abstract(const delegate_abstract &) = default;
        delegate_abstract(delegate_abstract &&) = default;
        delegate_abstract &operator=(delegate_abstract &&) = default;
        delegate_abstract &operator=(const delegate_abstract &) = default;
        virtual ~delegate_abstract() = default;

        /**
         * \brief 此部分写于2024/1/18.
         * \brief 应当实现拷贝，若大小大于std::_Space_size，则在堆区创建对象，否则在栈区创建.
         *
         * \param where 目标位置
         * \return 应当返回子类指针
         */
        virtual delegate_abstract *copy(void *where) const = 0;

        /**
         * \brief 此部分写于2024/1/18.
         * \brief 应当实现移动，若对象分配在堆区，应返回nullptr.
         *
         * \param where 目标位置
         * \return 应当返回子类指针
         */
        virtual delegate_abstract *move(void *where) noexcept = 0;

        /**
         * \brief 此部分写于2024/1/18.
         * \brief 应当实现调用，而且应当调用的是函数对象.
         *
         * \param args 调用的参数
         * \return 函数对象的operator()返回值
         */
        virtual Rx invoke(Args &&...args) const = 0;

        /**
         * \brief 此部分写于2024/1/18.
         * \brief 应由负责清理的函数调用
         *
         * \param dealloc 如果对象是堆区分配的，应为true，否则仅调用析构
         */
        virtual void release(bool dealloc) noexcept = 0;

        /**
         * \brief 此部分写于2024/4/22
         * \brief 应当实现函数对象大小查询
         *
         * \return 当前函数对象的字节大小
         */
        RAINY_NODISCARD virtual std::size_t size() const noexcept = 0;

        /**
         * \brief 此部分写于2024/4/25.
         * \brief 应当实现函数对象的std::type_info.
         *
         * \return 返回当前函数对象的typeid
         */
        RAINY_NODISCARD virtual const std::type_info &target_type() const noexcept = 0;

        /**
         * \brief 此部分写于2024/4/25.
         * \brief 应当返回这个函数对象的地址.
         *
         * \return 返回当前函数对象的地址，以const void*形式
         */
        RAINY_NODISCARD virtual const void *get() const noexcept = 0;
    };

    template <typename Fx, typename Rx, typename... Args>
    struct invoker final : delegate_abstract<Rx, Args...> {
        using base = delegate_abstract<Rx, Args...>;
        using nothrow_move = std::is_nothrow_move_constructible<Fx>;

        template <typename impl>
        static constexpr bool is_large = sizeof(impl) > information::space_size ||
                                                alignof(impl) > alignof(std::max_align_t) ||
                                                  !impl::nothrow_move::value;

        template <typename Other, std::enable_if_t<!std::is_same_v<invoker, std::decay_t<Other>>, int> = 0>
        explicit invoker(Other &&callable) : callable(std::forward<Other>(callable)) {
        }

        base *copy(void *where) const override {
            if constexpr (is_large<invoker>) {
                return ::new invoker(callable);
            } else {
                if (where) {
                    rainy_let location = static_cast<invoker *>(where);
                    return dynamic_cast<delegate_abstract<Rx, Args...> *>(utility::construct_at(location, callable));
                }
                return ::new invoker(callable);
            }
        }

        base *move(void *where) noexcept override {
            if constexpr (is_large<invoker>) {
                return nullptr;
            } else {
                rainy_let location = static_cast<invoker *>(where);
                return dynamic_cast<delegate_abstract<Rx, Args...> *>(utility::construct_at(location, callable));
            }
        }

        Rx invoke(Args &&...args) const override {
            if constexpr (std::is_void_v<Rx>) {
                std::invoke(callable, std::forward<Args>(args)...);
            } else {
                return std::invoke(callable, std::forward<Args>(args)...);
            }
            return Rx();
        }

        void release(const bool dealloc) noexcept override { // destroy self
            this->~invoker();
            if (dealloc) {
                std::allocator<invoker>{}.deallocate(this, 1);
            }
        }

        std::size_t size() const noexcept override {
            return sizeof(callable);
        }

        const std::type_info &target_type() const noexcept override {
            return typeid(callable);
        }

        const void *get() const noexcept override {
            return static_cast<const void *>(&callable);
        }

        mutable Fx callable;
    };
}

namespace rainy::foundation::functional::internals {
    template <typename Fx>
    class delegate_base;

    template <typename Rx, typename... Args>
    class delegate_base<Rx(Args...)> {
    public:
        using return_type = Rx;

        static inline constexpr std::size_t arity = sizeof...(Args);

        template <typename Fx>
        delegate_base(Fx &&function) : instance(make_invoker(utility::forward<Fx>(function))) {
        }

        ~delegate_base() {
            reset();
        }

        return_type invoke(Args... args) const {
            if (empty()) {
                system::exceptions::runtime::throw_invoke_error("internal object is null!");
            }
            if constexpr (type_traits::primary_types::is_void_v<return_type>) {
                instance->invoke(args...);
            } else {
                return instance->invoke(args...);
            }
        }

        return_type operator()(Args... args) const {
            return invoke(args...);
        }

        const std::type_info &target_type() const noexcept {
            if (empty()) {
                return typeid(void);
            }
            return instance->target_type();
        }

        void reset() {
            if (!empty()) {
                instance->release(!using_local());
            }
            instance = nullptr;
        }

        void reset(std::nullptr_t) {
            reset();
        }

        template <typename Fx>
        void reset(Fx&& function) {
            if (!empty()) {
                reset();
            }
            instance = make_invoker(utility::forward<Fx>(function));
        }



        RAINY_NODISCARD bool empty() const noexcept {
            return !static_cast<bool>(instance);
        }

        RAINY_NODISCARD bool using_local() const noexcept {
            return static_cast<const void *>(instance) == static_cast<const void *>(&small_object[0]);
        }

    private:
        template <typename Fx>
        delegate_abstract<Rx, Args...> *make_invoker(Fx &&handler) {
            using invoker_type = invoker<Fx, Rx, Args...>;
            static constexpr bool is_large = sizeof(invoker_type) > information::space_size ||
                                             alignof(invoker_type) > alignof(std::max_align_t) || !invoker_type::nothrow_move::value;
            if constexpr (is_large) {
                return new invoker_type(utility::forward<Fx>(handler));
            } else {
                return utility::construct_at(reinterpret_cast<invoker_type *>(small_object), utility::forward<Fx>(handler));
            }
        }

        union {
            char small_object[information::space_size];
            std::max_align_t dummy;
        };
        delegate_abstract<Rx, Args...> *instance;
    };

    template <typename Rx, typename... Args>
    class delegate_base<Rx(*)(Args...)> : public delegate_base<Rx(Args...)> {};

}
#endif
