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
#ifndef RAINY_CORE_ANNOTATIONS_LIFETIME_ANNOTATION_HPP
#define RAINY_CORE_ANNOTATIONS_LIFETIME_ANNOTATION_HPP
#include <rainy/core/diagnostics/exceptions.hpp>
#include <rainy/core/layer.hpp>
#include <rainy/core/type_traits.hpp>

namespace rainy::annotations::lifetime {
    template <typename Ty>
    class take;
}

/*
生命周期注解

用于指明其类型具体的生命周期模式
主要用于显式指明某个参数或变量的生命周期，以方便对其进行安全的访问，同时增强代码可读性
*/
namespace rainy::annotations::lifetime {
    /**
     * \lang english
     * @brief Input parameter annotation indicating the parameter is used for input only.
     *
     * @tparam Ty The parameter type
     *
     * \lang simp-chinese
     * @brief 输入参数注解，表示参数仅用于输入。
     *
     * @tparam Ty 参数类型
     */
    template <typename Ty>
    using in = type_traits::other_trans::enable_if_t<
        !type_traits::type_relations::is_void_v<Ty>,
        type_traits::other_trans::conditional_t<type_traits::properties::prefer_pass_by_value_v<Ty>, Ty const, Ty const &>>;

    /**
     * \lang english
     * @brief Move-from parameter annotation indicating the parameter will be moved from.
     *
     * @tparam Ty The parameter type
     *
     * \lang simp-chinese
     * @brief 移动来源参数注解，表示参数将被移动。
     *
     * @tparam Ty 参数类型
     */
    template <typename Ty>
    using move_from = type_traits::other_trans::enable_if_t<!type_traits::type_relations::is_void_v<Ty>, Ty &&>;

    /**
     * \lang english
     * @brief Read-only parameter annotation.
     *
     * @tparam Ty The parameter type
     *
     * \lang simp-chinese
     * @brief 只读参数注解。
     *
     * @tparam Ty 参数类型
     */
    template <typename Ty>
    using read_only = Ty const &;

    /**
     * \lang english
     * @brief Static read-only parameter annotation (alias for read_only).
     *
     * @tparam Ty The parameter type
     *
     * \lang simp-chinese
     * @brief 静态只读参数注解（read_only的别名）。
     *
     * @tparam Ty 参数类型
     */
    template <typename Ty>
    using static_read_only = read_only<Ty>;

    /**
     * \lang english
     * @brief A deferred initialization wrapper for types.
     *
     * @tparam Ty The type to be deferred-initialized
     *
     * \lang simp-chinese
     * @brief 类型的延迟初始化包装器。
     *
     * @tparam Ty 需要延迟初始化的类型
     */
    template <typename Ty>
    class deferred_init {
    public:
        /**
         * \lang english
         * @brief Default constructor. Does not initialize the contained object.
         *
         * \lang simp-chinese
         * @brief 默认构造函数。不初始化包含的对象。
         */
        deferred_init() noexcept { // NOLINT
        }

        /**
         * \lang english
         * @brief Destructor. Destroys the contained object if initialized.
         *
         * \lang simp-chinese
         * @brief 析构函数。如果已初始化，则销毁包含的对象。
         */
        ~deferred_init() noexcept {
            destroy();
        }

        /**
         * \lang english
         * @brief Accesses the contained value.
         *
         * @return Reference to the contained object
         * @throws core::exceptions::runtime::runtime_error if not initialized
         *
         * \lang simp-chinese
         * @brief 访问包含的值。
         *
         * @return 包含对象的引用
         * @throws 如果未初始化则抛出异常
         */
        rain_fn value() -> Ty & {
            core::exceptions::throw_exception_if<core::exceptions::runtime::runtime_error>(init);
            return t();
        }

        /**
         * \lang english
         * @brief Constructs the contained object with the given arguments.
         *
         * @tparam Args Constructor argument types
         * @param args Arguments to forward to the constructor
         * @throws core::exceptions::runtime::runtime_error if already initialized
         *
         * \lang simp-chinese
         * @brief 使用给定的参数构造包含的对象。
         *
         * @tparam Args 构造函数参数类型
         * @param args 要转发给构造函数的参数
         * @throws 如果已初始化则抛出异常
         */
        template <typename... Args>
        rain_fn construct(Args &&...args) -> void {
            core::exceptions::throw_exception_if<core::exceptions::runtime::runtime_error>(!init);
            new (&data) Ty{utility::forward<Args>(args)...};
            init = true;
        }

        /**
         * \lang english
         * @brief Checks whether the contained object has been initialized.
         *
         * @return true if the contained object has been constructed, false otherwise.
         *
         * @note This function is noexcept and never throws exceptions.
         *
         * @see construct() To initialize the contained object.
         * @see value() To access the contained object (throws if not initialized).
         *
         * \lang simp-chinese
         * @brief 检查包含的对象是否已被初始化。
         *
         * @return 如果包含的对象已被构造则返回 true，否则返回 false。
         *
         * @note 此函数是 noexcept 的，永远不会抛出异常。
         *
         * @see 参见 construct() 以初始化包含的对象。
         * @see 参见 value() 以访问包含的对象（未初始化时抛出异常）。
         */
        rain_fn is_init() const noexcept -> bool {
            return init;
        }

    private:
        rain_fn t() -> Ty & {
            return *std::launder(reinterpret_cast<Ty *>(&data));
        }

        template <typename U>
        friend class out;

        rain_fn destroy() -> void {
            if (init) {
                t().~Ty();
            }
            init = false;
        }

        alignas(Ty) core::byte_t data[sizeof(Ty)];
        bool init = false;
    };

    /**
     * \lang english
     * @brief Output parameter wrapper that guarantees initialization.
     *
     * @tparam Ty The type of the output parameter
     *
     * \lang simp-chinese
     * @brief 保证初始化的输出参数包装器。
     *
     * @tparam Ty 输出参数的类型
     */
    template <typename Ty>
    class out {
    public:
        template <typename>
        friend class borrow_out;

        /**
         * \lang english
         * @brief Constructs an out wrapper from a raw pointer.
         *
         * @param t_ Pointer to an already constructed object
         * @throws core::exceptions::runtime::runtime_error if pointer is null
         *
         * \lang simp-chinese
         * @brief 从原始指针构造out包装器。
         *
         * @param t_ 指向已构造对象的指针
         * @throws 如果指针为空则抛出异常
         */
        out(Ty *t_) : t{t_}, has_t{true} {
            core::exceptions::throw_exception_if<core::exceptions::runtime::runtime_error>(t);
        }

        /**
         * \lang english
         * @brief Constructs an out wrapper from a deferred_init object.
         *
         * @param dt_ Pointer to a deferred_init object
         * @throws core::exceptions::runtime::runtime_error if pointer is null
         *
         * \lang simp-chinese
         * @brief 从deferred_init对象构造out包装器。
         *
         * @param dt_ 指向deferred_init对象的指针
         * @throws 如果指针为空则抛出异常
         */
        out(deferred_init<Ty> *dt_) : dt{dt_}, has_t{false} {
            core::exceptions::throw_exception_if<core::exceptions::runtime::runtime_error>(dt);
        }

        /**
         * \lang english
         * @brief Constructs an out wrapper from another out object.
         *
         * @param ot_ Pointer to another out object
         * @throws core::exceptions::runtime::runtime_error if pointer is null
         *
         * \lang simp-chinese
         * @brief 从另一个out对象构造out包装器。
         *
         * @param ot_ 指向另一个out对象的指针
         * @throws 如果指针为空则抛出异常
         */
        out(out<Ty> *ot_) : ot{ot_}, has_t{ot_->has_t} {
            core::exceptions::throw_exception_if<core::exceptions::runtime::runtime_error>(ot_);
            if (has_t) {
                t = ot->t;
            } else {
                dt = ot->dt;
            }
        }

        /**
         * \lang english
         * @brief Returns a reference to the called_construct flag.
         *
         * @return Reference to the flag indicating whether construct was called
         *
         * \lang simp-chinese
         * @brief 返回called_construct标志的引用。
         *
         * @return 指示是否调用了construct的标志的引用
         */
        rain_fn called_construct() -> bool & {
            if (ot) {
                return ot->called_construct();
            } else {
                return called_construct_;
            }
        }

        /**
         * \lang english
         * @brief Destructor. Ensures proper cleanup if construction was interrupted.
         *
         * \lang simp-chinese
         * @brief 析构函数。确保如果构造被中断则进行适当的清理。
         */
        ~out() {
            if (called_construct() && uncaught_count != core::exceptions::uncaught_exceptions()) {
                core::exceptions::throw_exception_if<core::exceptions::runtime::runtime_error>(!has_t);
                dt->destroy();
                called_construct() = false;
            }
        }

        /**
         * \lang english
         * @brief Constructs or assigns the output value.
         *
         * @tparam Args Constructor argument types
         * @param args Arguments to forward to the constructor or assignment
         * @throws core::exceptions::runtime::runtime_error on failure
         *
         * \lang simp-chinese
         * @brief 构造或赋值输出值。
         *
         * @tparam Args 构造函数参数类型
         * @param args 要转发给构造函数或赋值的参数
         * @throws 失败时抛出异常
         */
        template <typename... Args>
        rain_fn construct(Args &&...args) -> void {
            using namespace type_traits::properties;
            if (has_t || called_construct()) {
                if constexpr (is_constructible_v<Ty, Args...> && is_copy_assignable_v<Ty>) {
                    core::exceptions::throw_exception_if<core::exceptions::runtime::runtime_error>(t);
                    *t = Ty(utility::forward<Args>(args)...);
                } else {
                    core::exceptions::throw_exception_if<core::exceptions::runtime::runtime_error>(
                        false, "attempted to copy assign, but copy assignment is not available");
                }
            } else {
                core::exceptions::throw_exception_if<core::exceptions::runtime::runtime_error>(dt);
                if (dt->init) {
                    if constexpr (is_constructible_v<Ty, Args...> && is_copy_assignable_v<Ty>) {
                        dt->value() = Ty(utility::forward<Args>(args)...);
                    } else {
                        core::exceptions::throw_exception_if<core::exceptions::runtime::runtime_error>(
                            false, "attempted to copy assign, but copy assignment is not available");
                    }
                } else {
                    dt->construct(utility::forward<Args>(args)...);
                    called_construct() = true;
                }
            }
        }

        /**
         * \lang english
         * @brief Accesses the output value.
         *
         * @return Reference to the output value
         * @throws core::exceptions::runtime::runtime_error if not properly initialized
         *
         * \lang simp-chinese
         * @brief 访问输出值。
         *
         * @return 输出值的引用
         * @throws 如果未正确初始化则抛出异常
         */
        rain_fn value() -> Ty & {
            if (has_t) {
                core::exceptions::throw_exception_if<core::exceptions::runtime::runtime_error>(t);
                return *t;
            } else {
                core::exceptions::throw_exception_if<core::exceptions::runtime::runtime_error>(dt);
                return dt->value();
            }
        }

    private:
        union {
            Ty *t;
            deferred_init<Ty> *dt;
        };
        out<Ty> *ot = {};
        bool has_t;
        int uncaught_count = core::exceptions::uncaught_exceptions();
        bool called_construct_ = false;
    };
}

namespace rainy::annotations::lifetime::implements {
    class RAINY_TOOLKIT_API atomic_counter {
    public:
        void operator++() {
            core::layer::interlocked_increment32(&this->count);
        }

        void operator--() {
            core::layer::interlocked_decrement32(&this->count);
        }

        int get() {
            return core::layer::iso_volatile_load32(static_cast<const volatile int *>(&this->count));
        }

    private:
        int count{0};
    };

    struct borrow_control_block {
        atomic_counter immutable_count{};
        bool mutable_active{false};
        int strong_count{1};
    };

    template <typename Ty>
    class refwrap {
    public:
        using type = type_traits::modifers::remove_cvref_t<Ty>;

        /**
         * \lang english
         * @brief Constructs a mutable reference wrapper.
         * @param ctrl Pointer to borrow control block
         * @throws core::exceptions::runtime::runtime_error if immutable references exist or mutable reference already active
         *
         * \lang simp-chinese
         * @brief 构造可变引用包装器。
         *
         * @param value Reference to the value         *              值的引用
         * @param ctrl 指向借用控制块的指针
         * @throws 如果存在不可变引用或可变引用已激活则抛出异常
         */
        refwrap(type &value, borrow_control_block *ctrl) : value{utility::addressof(value)}, ctrl{ctrl} {
            core::exceptions::throw_exception_if<core::exceptions::runtime::runtime_error>(
                ctrl->immutable_count.get() == 0, "you can not get a mut reference when you have cref");
            core::exceptions::throw_exception_if<core::exceptions::runtime::runtime_error>(!ctrl->mutable_active,
                                                                                           "you cannot get a another mut reference");
        }

        ~refwrap() {
            ctrl->mutable_active = false;
        }

        /**
         * \lang english
         * @brief Conversion operator to mutable reference.
         *
         * @return Mutable reference to the value
         * @throws core::exceptions::runtime::runtime_error if immutable references exist or mutable reference not active
         *
         * \lang simp-chinese
         * @brief 到可变引用的转换运算符。
         *
         * @return 值的可变引用
         * @throws 如果存在不可变引用或可变引用未激活则抛出异常
         */
        operator type &() {
            core::exceptions::throw_exception_if<core::exceptions::runtime::runtime_error>(
                ctrl->immutable_count.get() == 0, "you can not get a mut reference when you have cref");
            core::exceptions::throw_exception_if<core::exceptions::runtime::runtime_error>(!ctrl->mutable_active,
                                                                                           "you cannot get a another mut reference");
            ctrl->mutable_active = true;
            return *value;
        }

    private:
        type *value{nullptr};
        borrow_control_block *ctrl{nullptr};
    };

    template <typename Ty>
    class crefwrap {
    public:
        using type = type_traits::modifers::remove_volatile_t<type_traits::modifers::remove_reference_t<Ty>>;

        /**
         * \lang english
         * @brief Constructs an immutable reference wrapper.
         *
         * @param value Reference to the value
         * @param ctrl Pointer to borrow control block
         * @throws core::exceptions::runtime::runtime_error if mutable reference is active
         *
         * \lang simp-chinese
         * @brief 构造不可变引用包装器。
         *
         * @param value 值的引用
         * @param ctrl 指向借用控制块的指针
         * @throws 如果可变引用已激活则抛出异常
         */
        crefwrap(type &value, borrow_control_block *ctrl) : value{utility::addressof(value)}, ctrl{ctrl} {
            core::exceptions::throw_exception_if<core::exceptions::runtime::runtime_error>(
                !ctrl->mutable_active, "you cannot get cref when you get a mut ref.");
        }

        ~crefwrap() {
            release();
        }

        /**
         * \lang english
         * @brief Conversion operator to const reference.
         *
         * @return Const reference to the value
         * @throws core::exceptions::runtime::runtime_error if mutable reference is active
         *
         * \lang simp-chinese
         * @brief 到常量引用的转换运算符。
         *
         * @return 值的常量引用
         * @throws 如果可变引用已激活则抛出异常
         */
        operator const type &() {
            core::exceptions::throw_exception_if<core::exceptions::runtime::runtime_error>(
                !ctrl->mutable_active, "you cannot get cref when you release this ref.");
            core::exceptions::throw_exception_if<core::exceptions::runtime::runtime_error>(
                !ctrl->mutable_active, "you cannot get cref when you get a mut ref.");
            ++ctrl->immutable_count;
            return *value;
        }

        /**
         * \lang english
         * @brief Explicitly releases the reference.
         *
         * \lang simp-chinese
         * @brief 显式释放引用。
         */
        void release() {
            if (!ctrl) {
                return;
            }
            if (ctrl->immutable_count.get() > 0) {
                --ctrl->immutable_count;
            }
            value = nullptr;
            ctrl = nullptr;
        }

    private:
        type *value{nullptr};
        borrow_control_block *ctrl{nullptr};
    };
}

namespace rainy::annotations::lifetime {
    /**
     * \lang english
     * @brief Borrow ownership annotation.
     *
     * Allows either one mutable reference or multiple immutable references within its lifetime.
     * If declared in a function, the lifetime spans the entire function scope.
     * If declared locally, the lifetime is determined by its scope.
     *
     * @tparam Ty The type to borrow
     *
     * \lang simp-chinese
     * @brief 借用所有权注解。
     *
     * 仅允许一个mut引用或是多个不可变引用在其所在的生命周期。
     * 若声明在函数，则生命周期为整个函数其作用域。
     * 若局部声明，则由其所在作用域决定。
     *
     * @tparam Ty 要借用的类型
     */
    template <typename Ty>
    class borrow_out : public out<Ty> {
    public:
        static_assert(type_traits::type_relations::is_same_v<type_traits::other_trans::decay_t<Ty>, Ty>);

        using refwrap = implements::refwrap<Ty>;
        using crefwrap = implements::crefwrap<Ty>;

        /**
         * \lang english
         * @brief Constructs a borrow_out from a raw pointer.
         *
         * @param t_ Pointer to the value
         *
         * \lang simp-chinese
         * @brief 从原始指针构造borrow_out。
         *
         * @param t_ 指向值的指针
         */
        borrow_out(Ty *t_) : out<Ty>{t_}, ctrl(new implements::borrow_control_block{}) {
        }

        /**
         * \lang english
         * @brief Constructs a borrow_out from a deferred_init object.
         *
         * @param dt_ Pointer to deferred_init object
         *
         * \lang simp-chinese
         * @brief 从deferred_init对象构造borrow_out。
         *
         * @param dt_ 指向deferred_init对象的指针
         */
        borrow_out(deferred_init<Ty> *dt_) : out<Ty>{dt_}, ctrl(new implements::borrow_control_block{}) {
        }

        /**
         * \lang english
         * @brief Constructs a borrow_out from an out object.
         *
         * @param ot_ Pointer to out object
         *
         * \lang simp-chinese
         * @brief 从out对象构造borrow_out。
         *
         * @param ot_ 指向out对象的指针
         */
        borrow_out(out<Ty> *ot_) : out<Ty>{ot_}, ctrl(new implements::borrow_control_block{}) {
        }

        /**
         * \lang english
         * @brief Copy constructor. Shares the control block.
         *
         * @param other Another borrow_out to copy from
         *
         * \lang simp-chinese
         * @brief 拷贝构造函数。共享控制块。
         *
         * @param other 要拷贝的另一个borrow_out
         */
        borrow_out(const borrow_out &other) : out<Ty>{other}, ctrl{other.ctrl} {
            ++ctrl->strong_count;
        }

        /**
         * \lang english
         * @brief Copy assignment operator. Shares the control block.
         *
         * @param other Another borrow_out to copy from
         * @return Reference to this object
         *
         * \lang simp-chinese
         * @brief 拷贝赋值运算符。共享控制块。
         *
         * @param other 要拷贝的另一个borrow_out
         * @return 此对象的引用
         */
        rain_fn operator=(const borrow_out &other)->borrow_out & {
            if (this != &other) {
                release_ctrl();
                out<Ty>::operator=(other);
                ctrl = other.ctrl;
                ++ctrl->strong_count;
            }
            return *this;
        }

        /**
         * \lang english
         * @brief Destructor. Releases the control block when no longer needed.
         *
         * \lang simp-chinese
         * @brief 析构函数。不再需要时释放控制块。
         */
        ~borrow_out() {
            release_ctrl();
        }

        /**
         * \lang english
         * @brief Obtains a mutable reference wrapper.
         *
         * @return Mutable reference wrapper
         *
         * \lang simp-chinese
         * @brief 获取可变引用包装器。
         *
         * @return 可变引用包装器
         */
        rain_fn mut() -> refwrap {
            return refwrap{this->value(), ctrl};
        }

        /**
         * \lang english
         * @brief Obtains an immutable (const) reference wrapper.
         *
         * @return Immutable reference wrapper
         *
         * \lang simp-chinese
         * @brief 获取不可变（常量）引用包装器。
         *
         * @return 不可变引用包装器
         */
        rain_fn const_ref() -> crefwrap {
            return crefwrap{this->value(), ctrl};
        }

    private:
        void release_ctrl() {
            if (!ctrl) {
                return;
            }
            --ctrl->strong_count;
            ctrl->mutable_active = false;
            if (ctrl->strong_count == 0) {
                delete ctrl;
                ctrl = nullptr;
            }
        }

        implements::borrow_control_block *ctrl;
    };
}

namespace rainy::annotations::lifetime::implements {
    /**
     * \lang english
     * @brief Trait to detect if a type is not a take<Ty> specialization.
     *
     * @tparam Ty The type to check
     *
     * \lang simp-chinese
     * @brief 检测类型是否为take<Ty>特化的特性。
     *
     * @tparam Ty 要检查的类型
     */
    template <typename Ty>
    static RAINY_CONSTEXPR_BOOL not_take_v = true;

    /**
     * \lang english
     * @brief Specialization that detects take<Ty>.
     *
     * @tparam Template The template type
     * @tparam Ty The inner type
     *
     * \lang simp-chinese
     * @brief 检测take<Ty>的特化。
     *
     * @tparam Template 模板类型
     * @tparam Ty 内部类型
     */
    template <template <typename Ty> typename Template, typename Ty>
    static RAINY_CONSTEXPR_BOOL not_take_v<Template<Ty>> = type_traits::type_relations::is_same_v<Template<Ty>, take<Ty>>;
}

namespace rainy::annotations::lifetime {
    /**
     * \lang english
     * @brief Ownership transfer annotation that moves resources.
     *
     * @tparam Ty The type whose ownership is being transferred
     *
     * \lang simp-chinese
     * @brief 移动资源的所有权转移注解。
     *
     * @tparam Ty 所有权被转移的类型
     */
    template <typename Ty>
    class take : type_traits::helper::non_copyable {
    public:
        using type = type_traits::modifers::remove_cvref_t<Ty>;
        using reference = type &;
        using pointer = type *;

        static_assert(implements::not_take_v<type>, "Do not pass take<Ty> to take<>");
        static_assert(type_traits::properties::is_move_constructible_v<type>,
                      "take<> require Ty is move constructible, but Ty is not.");

        static constexpr bool is_nothrow_move_construtible = type_traits::properties::is_nothrow_move_constructible_v<type>;
        static constexpr bool is_nothrow_move_assignible = type_traits::properties::is_nothrow_move_assignable_v<type>;

        /**
         * \lang english
         * @brief Constructs a take from an lvalue reference (moves from it).
         *
         * @param resources Lvalue reference to resources
         *
         * \lang simp-chinese
         * @brief 从左值引用构造take（从中移动）。
         *
         * @param resources 资源的左值引用
         */
        take(Ty &resources) noexcept(is_nothrow_move_construtible) : take_resources{utility::move(resources)} {
        }

        /**
         * \lang english
         * @brief Constructs a take from an rvalue reference.
         *
         * @param resources Rvalue reference to resources
         *
         * \lang simp-chinese
         * @brief 从右值引用构造take。
         *
         * @param resources 资源的右值引用
         */
        take(Ty &&resources) noexcept(is_nothrow_move_construtible) : take_resources{utility::move(resources)} {
        }

        /**
         * \lang english
         * @brief Move constructor.
         *
         * @param right Another take to move from
         *
         * \lang simp-chinese
         * @brief 移动构造函数。
         *
         * @param right 要移动的另一个take
         */
        take(take &&right) noexcept(is_nothrow_move_construtible) : take_resources{utility::move(right.take_resources)} {
        }

        /**
         * \lang english
         * @brief Conversion operator to reference.
         *
         * @return Reference to the held resources
         *
         * \lang simp-chinese
         * @brief 到引用的转换运算符。
         *
         * @return 持有的资源的引用
         */
        operator reference() noexcept {
            return take_resources;
        }

        /**
         * \lang english
         * @brief Address-of operator.
         *
         * @return Pointer to the held resources
         *
         * \lang simp-chinese
         * @brief 取地址运算符。
         *
         * @return 指向持有的资源的指针
         */
        rain_fn operator&() noexcept -> pointer {
            return &take_resources;
        }

        /**
         * \lang english
         * @brief Gets a reference to the held resources.
         *
         * @return Reference to the held resources
         *
         * \lang simp-chinese
         * @brief 获取持有的资源的引用。
         *
         * @return 持有的资源的引用
         */
        rain_fn get() -> reference {
            return take_resources;
        }

        /**
         * \lang english
         * @brief Assignment operator from any assignable type.
         *
         * @tparam UTy The assigned type
         * @param value Value to assign
         * @return Reference to the held resources
         *
         * \lang simp-chinese
         * @brief 从任何可赋值类型的赋值运算符。
         *
         * @tparam UTy 被赋值的类型
         * @param value 要赋值的值
         * @return 持有的资源的引用
         */
        template <typename UTy, type_traits::other_trans::enable_if_t<type_traits::properties::is_assignable_v<type, UTy>, int> = 0>
        rain_fn operator=(UTy &&value) noexcept(type_traits::properties::is_nothrow_assignable_v<type, UTy>)->reference {
            take_resources = utility::forward<UTy>(value);
            return *this;
        }

        /**
         * \lang english
         * @brief Swaps the held resources with another object.
         *
         * @tparam UTy The type to swap with
         * @param right The object to swap with
         *
         * \lang simp-chinese
         * @brief 与另一个对象交换持有的资源。
         *
         * @tparam UTy 要交换的类型
         * @param right 要交换的对象
         */
        template <type_traits::other_trans::enable_if_t<type_traits::properties::is_swappable_v<type>, int> = 0>
        rain_fn swap(take &right) noexcept(type_traits::properties::is_nothrow_swappable_v<type>) -> void {
            using std::swap;
            swap(take_resources, right.take_resources);
        }

        /**
         * \lang english
         * @brief Function call operator if the held type is invocable.
         *
         * @tparam Args Argument types
         * @param args Arguments to forward
         * @return Result of invoking the held callable
         *
         * \lang simp-chinese
         * @brief 如果持有的类型可调用，则提供函数调用运算符。
         *
         * @tparam Args 参数类型
         * @param args 要转发的参数
         * @return 调用持有的可调用对象的结果
         */
        template <typename... Args,
                  type_traits::other_trans::enable_if_t<type_traits::properties::is_invocable_v<type, Args...>, int> = 0>
        rain_fn operator()(Args &&...args)->type_traits::properties::invoke_result_t<type, Args...> {
            return utility::invoke(take_resources, utility::forward<Args>(args)...);
        }

        /**
         * \lang english
         * @brief Move assignment operator.
         *
         * @param right Another take to move from
         * @return Reference to this object
         *
         * \lang simp-chinese
         * @brief 移动赋值运算符。
         *
         * @param right 要移动的另一个take
         * @return 此对象的引用
         */
        rain_fn operator=(take &&right) noexcept(is_nothrow_move_assignible)->take & {
            take_resources = utility::move(right.take_resources);
            return *this;
        }

    private:
        type take_resources;
    };
}

// NOLINTBEGIN
namespace rainy::annotations::lifetime {
    /**
     * \lang english
     * @brief Uninitialized storage wrapper.
     *
     * @tparam Ty The type to store uninitialized
     *
     * \lang simp-chinese
     * @brief 未初始化存储的包装器。
     *
     * @tparam Ty 要未初始化存储的类型
     */
    template <typename Ty>
    struct uninitialized {
        using type = Ty;

        /**
         * \lang english
         * @brief Default constructor. Leaves the contained value uninitialized.
         *
         * \lang simp-chinese
         * @brief 默认构造函数。保持包含的值未初始化。
         */
        uninitialized() {
        }

        /**
         * \lang english
         * @brief Copy constructor. Copies the contained value.
         *
         * @param other The other uninitialized object to copy from
         *
         * \lang simp-chinese
         * @brief 拷贝构造函数。拷贝包含的值。
         *
         * @param other 要拷贝的另一个uninitialized对象
         */
        uninitialized(const uninitialized &) = default;

        /**
         * \lang english
         * @brief Move constructor. Moves the contained value.
         *
         * @param other The other uninitialized object to move from
         *
         * \lang simp-chinese
         * @brief 移动构造函数。移动包含的值。
         *
         * @param other 要移动的另一个uninitialized对象
         */
        uninitialized(uninitialized &&) = default;

        /**
         * \lang english
         * @brief Copy assignment operator. Copies the contained value.
         *
         * @param other The other uninitialized object to copy from
         * @return Reference to this object
         *
         * \lang simp-chinese
         * @brief 拷贝赋值运算符。拷贝包含的值。
         *
         * @param other 要拷贝的另一个uninitialized对象
         * @return 此对象的引用
         */
        uninitialized &operator=(const uninitialized &) = default;

        /**
         * \lang english
         * @brief Move assignment operator. Moves the contained value.
         *
         * @param other The other uninitialized object to move from
         * @return Reference to this object
         *
         * \lang simp-chinese
         * @brief 移动赋值运算符。移动包含的值。
         *
         * @param other 要移动的另一个uninitialized对象
         * @return 此对象的引用
         */
        uninitialized &operator=(uninitialized &&) = default;

        type value;
    };

    /**
     * \lang english
     * @brief Deleted specialization for const types.
     *
     * \lang simp-chinese
     * @brief 常量类型的已删除特化。
     */
    template <typename Ty>
    struct uninitialized<const Ty> {
        /**
         * \lang english
         * @brief Deleted default constructor.
         *
         * \lang simp-chinese
         * @brief 已删除的默认构造函数。
         */
        uninitialized() = delete;
    };

    /**
     * \lang english
     * @brief Deleted specialization for lvalue reference types.
     *
     * \lang simp-chinese
     * @brief 左值引用类型的已删除特化。
     */
    template <typename Ty>
    struct uninitialized<Ty &> {
        /**
         * \lang english
         * @brief Deleted default constructor.
         *
         * \lang simp-chinese
         * @brief 已删除的默认构造函数。
         */
        uninitialized() = delete;
    };

    /**
     * \lang english
     * @brief Deleted specialization for const lvalue reference types.
     *
     * \lang simp-chinese
     * @brief 常量左值引用类型的已删除特化。
     */
    template <typename Ty>
    struct uninitialized<const Ty &> {
        uninitialized() = delete;
    };

    /**
     * \lang english
     * @brief Deleted specialization for rvalue reference types.
     *
     * \lang simp-chinese
     * @brief 右值引用类型的已删除特化。
     */
    template <typename Ty>
    struct uninitialized<Ty &&> {
        uninitialized() = delete;
    };

    /**
     * \lang english
     * @brief Deleted specialization for const rvalue reference types.
     *
     * \lang simp-chinese
     * @brief 常量右值引用类型的已删除特化。
     */
    template <typename Ty>
    struct uninitialized<const Ty &&> {
        /**
         * \lang english
         * @brief Deleted default constructor.
         *
         * \lang simp-chinese
         * @brief 已删除的默认构造函数。
         */
        uninitialized() = delete;
    };
}
// NOLINTEND

namespace rainy::utility {
    using annotations::lifetime::borrow_out;
    using annotations::lifetime::deferred_init;
    using annotations::lifetime::in;
    using annotations::lifetime::move_from;
    using annotations::lifetime::out;
    using annotations::lifetime::read_only;
    using annotations::lifetime::static_read_only;
    using annotations::lifetime::take;
    using annotations::lifetime::uninitialized;
}

#endif
