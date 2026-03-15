#ifndef RAINY_CORE_TMP_LIFETIME_ANNOTATION_HPP
#define RAINY_CORE_TMP_LIFETIME_ANNOTATION_HPP
#include <rainy/core/implements/exceptions.hpp>
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
     * @brief Input parameter annotation indicating the parameter is used for input only.
     *        输入参数注解，表示参数仅用于输入。
     *
     * @tparam Ty The parameter type
     *            参数类型
     */
    template <typename Ty>
    using in = type_traits::other_trans::enable_if_t<
        !type_traits::type_relations::is_void_v<Ty>,
        type_traits::other_trans::conditional_t<type_traits::type_properties::prefer_pass_by_value_v<Ty>, Ty const, Ty const &>>;

    /**
     * @brief Move-from parameter annotation indicating the parameter will be moved from.
     *        移动来源参数注解，表示参数将被移动。
     *
     * @tparam Ty The parameter type
     *            参数类型
     */
    template <typename Ty>
    using move_from = type_traits::other_trans::enable_if_t<!type_traits::type_relations::is_void_v<Ty>, Ty &&>;

    /**
     * @brief Read-only parameter annotation.
     *        只读参数注解。
     *
     * @tparam Ty The parameter type
     *            参数类型
     */
    template <typename Ty>
    using read_only = Ty const &;

    /**
     * @brief Static read-only parameter annotation (alias for read_only).
     *        静态只读参数注解（read_only的别名）。
     *
     * @tparam Ty The parameter type
     *            参数类型
     */
    template <typename Ty>
    using static_read_only = read_only<Ty>;

    /**
     * @brief A deferred initialization wrapper for types.
     *        类型的延迟初始化包装器。
     *
     * @tparam T The type to be deferred-initialized
     *           需要延迟初始化的类型
     */
    template <typename T>
    class deferred_init {
    public:
        /**
         * @brief Default constructor. Does not initialize the contained object.
         *        默认构造函数。不初始化包含的对象。
         */
        deferred_init() noexcept {
        }

        /**
         * @brief Destructor. Destroys the contained object if initialized.
         *        析构函数。如果已初始化，则销毁包含的对象。
         */
        ~deferred_init() noexcept {
            destroy();
        }

        /**
         * @brief Accesses the contained value.
         *        访问包含的值。
         *
         * @return Reference to the contained object
         *         包含对象的引用
         * @throws foundation::exceptions::runtime::runtime_error if not initialized
         *         如果未初始化则抛出异常
         */
        rain_fn value() noexcept -> T & {
            foundation::exceptions::throw_exception_if<foundation::exceptions::runtime::runtime_error>(init);
            return t();
        }

        /**
         * @brief Constructs the contained object with the given arguments.
         *        使用给定的参数构造包含的对象。
         *
         * @tparam Args Constructor argument types
         *               构造函数参数类型
         * @param args Arguments to forward to the constructor
         *             要转发给构造函数的参数
         * @throws foundation::exceptions::runtime::runtime_error if already initialized
         *         如果已初始化则抛出异常
         */
        template <typename... Args>
        rain_fn construct(Args &&...args) -> void {
            foundation::exceptions::throw_exception_if<foundation::exceptions::runtime::runtime_error>(init);
            new (&data) T{utility::forward(args)...};
            init = true;
        }

    private:
        rain_fn t() -> T & {
            return *std::launder(reinterpret_cast<T *>(&data));
        }

        template <typename U>
        friend class out;

        rain_fn destroy() -> void {
            if (init) {
                t().~T();
            }
            init = false;
        }

        alignas(T) core::byte_t data[sizeof(T)];
        bool init = false;
    };

    /**
     * @brief Output parameter wrapper that guarantees initialization.
     *        保证初始化的输出参数包装器。
     *
     * @tparam T The type of the output parameter
     *           输出参数的类型
     */
    template <typename T>
    class out {
    public:
        template <typename>
        friend class borrow_out;

        /**
         * @brief Constructs an out wrapper from a raw pointer.
         *        从原始指针构造out包装器。
         *
         * @param t_ Pointer to an already constructed object
         *           指向已构造对象的指针
         * @throws foundation::exceptions::runtime::runtime_error if pointer is null
         *         如果指针为空则抛出异常
         */
        out(T *t_) noexcept : t{t_}, has_t{true} {
            foundation::exceptions::throw_exception_if<foundation::exceptions::runtime::runtime_error>(t);
        }

        /**
         * @brief Constructs an out wrapper from a deferred_init object.
         *        从deferred_init对象构造out包装器。
         *
         * @param dt_ Pointer to a deferred_init object
         *            指向deferred_init对象的指针
         * @throws foundation::exceptions::runtime::runtime_error if pointer is null
         *         如果指针为空则抛出异常
         */
        out(deferred_init<T> *dt_) noexcept : dt{dt_}, has_t{false} {
            foundation::exceptions::throw_exception_if<foundation::exceptions::runtime::runtime_error>(dt);
        }

        /**
         * @brief Constructs an out wrapper from another out object.
         *        从另一个out对象构造out包装器。
         *
         * @param ot_ Pointer to another out object
         *            指向另一个out对象的指针
         * @throws foundation::exceptions::runtime::runtime_error if pointer is null
         *         如果指针为空则抛出异常
         */
        out(out<T> *ot_) noexcept : ot{ot_}, has_t{ot_->has_t} {
            foundation::exceptions::throw_exception_if<foundation::exceptions::runtime::runtime_error>(ot_);
            if (has_t) {
                t = ot->t;
            } else {
                dt = ot->dt;
            }
        }

        /**
         * @brief Returns a reference to the called_construct flag.
         *        返回called_construct标志的引用。
         *
         * @return Reference to the flag indicating whether construct was called
         *         指示是否调用了construct的标志的引用
         */
        rain_fn called_construct() -> bool & {
            if (ot) {
                return ot->called_construct();
            } else {
                return called_construct_;
            }
        }

        /**
         * @brief Destructor. Ensures proper cleanup if construction was interrupted.
         *        析构函数。确保如果构造被中断则进行适当的清理。
         */
        ~out() {
            if (called_construct() && uncaught_count != foundation::exceptions::uncaught_exceptions()) {
                foundation::exceptions::throw_exception_if<foundation::exceptions::runtime::runtime_error>(!has_t);
                dt->destroy();
                called_construct() = false;
            }
        }

        /**
         * @brief Constructs or assigns the output value.
         *        构造或赋值输出值。
         *
         * @tparam Args Constructor argument types
         *               构造函数参数类型
         * @param args Arguments to forward to the constructor or assignment
         *             要转发给构造函数或赋值的参数
         * @throws foundation::exceptions::runtime::runtime_error on failure
         *         失败时抛出异常
         */
        template <typename... Args>
        rain_fn construct(Args &&...args) -> void {
            using namespace type_traits::type_properties;
            if (has_t || called_construct()) {
                if constexpr (is_constructible_v<T, Args...> && is_copy_assignable_v<T>) {
                    foundation::exceptions::throw_exception_if<foundation::exceptions::runtime::runtime_error>(t);
                    *t = T(utility::forward<Args>(args)...);
                } else {
                    foundation::exceptions::throw_exception_if<foundation::exceptions::runtime::runtime_error>(
                        false, "attempted to copy assign, but copy assignment is not available");
                }
            } else {
                foundation::exceptions::throw_exception_if<foundation::exceptions::runtime::runtime_error>(dt);
                if (dt->init) {
                    if constexpr (is_constructible_v<T, Args...> && is_copy_assignable_v<T>) {
                        dt->value() = T(utility::forward(args)...);
                    } else {
                        foundation::exceptions::throw_exception_if<foundation::exceptions::runtime::runtime_error>(
                            false, "attempted to copy assign, but copy assignment is not available");
                    }
                } else {
                    dt->construct(utility::forward(args)...);
                    called_construct() = true;
                }
            }
        }

        /**
         * @brief Accesses the output value.
         *        访问输出值。
         *
         * @return Reference to the output value
         *         输出值的引用
         * @throws foundation::exceptions::runtime::runtime_error if not properly initialized
         *         如果未正确初始化则抛出异常
         */
        rain_fn value() noexcept -> T & {
            if (has_t) {
                foundation::exceptions::throw_exception_if<foundation::exceptions::runtime::runtime_error>(t);
                return *t;
            } else {
                foundation::exceptions::throw_exception_if<foundation::exceptions::runtime::runtime_error>(dt);
                return dt->value();
            }
        }

    private:
        union {
            T *t;
            deferred_init<T> *dt;
        };
        out<T> *ot = {};
        bool has_t;
        int uncaught_count = foundation::exceptions::uncaught_exceptions();
        bool called_construct_ = false;
    };
}

namespace rainy::annotations::lifetime::implements {
    class RAINY_TOOLKIT_API atomic_counter {
    public:
        void operator++();
        void operator--();
        int get();

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
        using type = type_traits::cv_modify::remove_cvref_t<Ty>;

        /**
         * @brief Constructs a mutable reference wrapper.
         *        构造可变引用包装器。
         *
         * @param value Reference to the value         *              值的引用
         * @param ctrl Pointer to borrow control block
         *             指向借用控制块的指针
         * @throws foundation::exceptions::runtime::runtime_error if immutable references exist or mutable reference already active
         *         如果存在不可变引用或可变引用已激活则抛出异常
         */
        refwrap(type &value, borrow_control_block *ctrl) : value{utility::addressof(value)}, ctrl{ctrl} {
            foundation::exceptions::throw_exception_if<foundation::exceptions::runtime::runtime_error>(
                ctrl->immutable_count.get() == 0, "you can not get a mut reference when you have cref");
            foundation::exceptions::throw_exception_if<foundation::exceptions::runtime::runtime_error>(
                !ctrl->mutable_active, "you cannot get a another mut reference");
        }

        ~refwrap() {
            ctrl->mutable_active = false;
        }

        /**
         * @brief Conversion operator to mutable reference.
         *        到可变引用的转换运算符。
         *
         * @return Mutable reference to the value
         *         值的可变引用
         * @throws foundation::exceptions::runtime::runtime_error if immutable references exist or mutable reference not active
         *         如果存在不可变引用或可变引用未激活则抛出异常
         */
        operator type &() {
            foundation::exceptions::throw_exception_if<foundation::exceptions::runtime::runtime_error>(
                ctrl->immutable_count.get() == 0, "you can not get a mut reference when you have cref");
            foundation::exceptions::throw_exception_if<foundation::exceptions::runtime::runtime_error>(
                !ctrl->mutable_active, "you cannot get a another mut reference");
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
        using type = type_traits::cv_modify::remove_volatile_t<type_traits::reference_modify::remove_reference_t<Ty>>;

        /**
         * @brief Constructs an immutable reference wrapper.
         *        构造不可变引用包装器。
         *
         * @param value Reference to the value
         *              值的引用
         * @param ctrl Pointer to borrow control block
         *             指向借用控制块的指针
         * @throws foundation::exceptions::runtime::runtime_error if mutable reference is active
         *         如果可变引用已激活则抛出异常
         */
        crefwrap(type &value, borrow_control_block *ctrl) : value{utility::addressof(value)}, ctrl{ctrl} {
            foundation::exceptions::throw_exception_if<foundation::exceptions::runtime::runtime_error>(
                !ctrl->mutable_active, "you cannot get cref when you get a mut ref.");
        }

        ~crefwrap() {
            release();
        }

        /**
         * @brief Conversion operator to const reference.
         *        到常量引用的转换运算符。
         *
         * @return Const reference to the value
         *         值的常量引用
         * @throws foundation::exceptions::runtime::runtime_error if mutable reference is active
         *         如果可变引用已激活则抛出异常
         */
        operator const type &() {
            foundation::exceptions::throw_exception_if<foundation::exceptions::runtime::runtime_error>(
                !ctrl->mutable_active, "you cannot get cref when you release this ref.");
            foundation::exceptions::throw_exception_if<foundation::exceptions::runtime::runtime_error>(
                !ctrl->mutable_active, "you cannot get cref when you get a mut ref.");
            ++ctrl->immutable_count;
            return *value;
        }

        /**
         * @brief Explicitly releases the reference.
         *        显式释放引用。
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
     * @brief 借用所有权注解
     * @brief 仅允许一个mut引用或是多个不可变引用在其所在的生命周期
     * @brief 若声明在函数，则生命周期为整个函数其作用域
     * @brief 若局部声明，则由其所在作用域决定
     * @param Ty 要借用的类型
     */
    /**
     * @brief Borrow ownership annotation.
     * @brief Allows either one mutable reference or multiple immutable references within its lifetime.
     * @brief If declared in a function, the lifetime spans the entire function scope.
     * @brief If declared locally, the lifetime is determined by its scope.
     * @tparam Ty The type to borrow
     *            要借用的类型
     */
    template <typename Ty>
    class borrow_out : out<Ty> {
    public:
        static_assert(type_traits::type_relations::is_same_v<type_traits::other_trans::decay_t<Ty>, Ty>);

        using refwrap = implements::refwrap<Ty>;
        using crefwrap = implements::crefwrap<Ty>;

        /**
         * @brief Constructs a borrow_out from a raw pointer.
         *        从原始指针构造borrow_out。
         *
         * @param t_ Pointer to the value
         *           指向值的指针
         */
        borrow_out(Ty *t_) noexcept : out<Ty>{t_} {
            ctrl = new implements::borrow_control_block{};
        }

        /**
         * @brief Constructs a borrow_out from a deferred_init object.
         *        从deferred_init对象构造borrow_out。
         *
         * @param dt_ Pointer to deferred_init object
         *            指向deferred_init对象的指针
         */
        borrow_out(deferred_init<Ty> *dt_) noexcept : out<Ty>{dt_} {
            ctrl = new implements::borrow_control_block{};
        }

        /**
         * @brief Constructs a borrow_out from an out object.
         *        从out对象构造borrow_out。
         *
         * @param ot_ Pointer to out object
         *            指向out对象的指针
         */
        borrow_out(out<Ty> *ot_) noexcept : out<Ty>{ot_} {
            ctrl = new implements::borrow_control_block{};
        }

        /**
         * @brief Copy constructor. Shares the control block.
         *        拷贝构造函数。共享控制块。
         *
         * @param other Another borrow_out to copy from
         *              要拷贝的另一个borrow_out
         */
        borrow_out(const borrow_out &other) : out<Ty>{other}, ctrl{other.ctrl} {
            ++ctrl->strong_count;
        }

        /**
         * @brief Copy assignment operator. Shares the control block.
         *        拷贝赋值运算符。共享控制块。
         *
         * @param other Another borrow_out to copy from
         *              要拷贝的另一个borrow_out
         * @return Reference to this object
         *         此对象的引用
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
         * @brief Destructor. Releases the control block when no longer needed.
         *        析构函数。不再需要时释放控制块。
         */
        ~borrow_out() {
            release_ctrl();
        }

        /**
         * @brief Obtains a mutable reference wrapper.
         *        获取可变引用包装器。
         *
         * @return Mutable reference wrapper
         *         可变引用包装器
         */
        rain_fn mut() -> refwrap {
            return refwrap{this->value(), ctrl};
        }

        /**
         * @brief Obtains an immutable (const) reference wrapper.
         *        获取不可变（常量）引用包装器。
         *
         * @return Immutable reference wrapper
         *         不可变引用包装器
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
     * @brief Trait to detect if a type is not a take<Ty> specialization.
     *        检测类型是否为take<Ty>特化的特性。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename Ty>
    static RAINY_CONSTEXPR_BOOL not_take_v = true;

    /**
     * @brief Specialization that detects take<Ty>.
     *        检测take<Ty>的特化。
     *
     * @tparam Template The template type
     *                  模板类型
     * @tparam Ty The inner type
     *            内部类型
     */
    template <template <typename Ty> typename Template, typename Ty>
    static RAINY_CONSTEXPR_BOOL not_take_v<Template<Ty>> = type_traits::type_relations::is_same_v<Template<Ty>, take<Ty>>;
}

namespace rainy::annotations::lifetime {
    /**
     * @brief Ownership transfer annotation that moves resources.
     *        移动资源的所有权转移注解。
     *
     * @tparam Ty The type whose ownership is being transferred
     *            所有权被转移的类型
     */
    template <typename Ty>
    class take : type_traits::helper::non_copyable {
    public:
        using type = type_traits::cv_modify::remove_cvref_t<Ty>;
        using reference = type &;
        using pointer = type *;

        static_assert(implements::not_take_v<type>, "Do not pass take<Ty> to take<>");
        static_assert(type_traits::type_properties::is_move_constructible_v<type>,
                      "take<> require Ty is move constructible, but Ty is not.");

        static constexpr bool is_nothrow_move_construtible = type_traits::type_properties::is_nothrow_move_constructible_v<type>;
        static constexpr bool is_nothrow_move_assignible = type_traits::type_properties::is_nothrow_move_assignable_v<type>;

        /**
         * @brief Constructs a take from an lvalue reference (moves from it).
         *        从左值引用构造take（从中移动）。
         *
         * @param resources Lvalue reference to resources
         *                  资源的左值引用
         */
        take(Ty &resources) noexcept(is_nothrow_move_construtible) : take_resources{utility::move(resources)} {
        }

        /**
         * @brief Constructs a take from an rvalue reference.
         *        从右值引用构造take。
         *
         * @param resources Rvalue reference to resources
         *                  资源的右值引用
         */
        take(Ty &&resources) noexcept(is_nothrow_move_construtible) : take_resources{utility::move(resources)} {
        }

        /**
         * @brief Move constructor.
         *        移动构造函数。
         *
         * @param right Another take to move from
         *              要移动的另一个take
         */
        take(take &&right) noexcept(is_nothrow_move_construtible) : take_resources{utility::move(right.take_resources)} {
        }

        /**
         * @brief Conversion operator to reference.
         *        到引用的转换运算符。
         *
         * @return Reference to the held resources
         *         持有的资源的引用
         */
        operator reference() noexcept {
            return take_resources;
        }

        /**
         * @brief Address-of operator.
         *        取地址运算符。
         *
         * @return Pointer to the held resources
         *         指向持有的资源的指针
         */
        rain_fn operator&() noexcept -> pointer {
            return &take_resources;
        }

        /**
         * @brief Gets a reference to the held resources.
         *        获取持有的资源的引用。
         *
         * @return Reference to the held resources
         *         持有的资源的引用
         */
        rain_fn get() -> reference {
            return take_resources;
        }

        /**
         * @brief Assignment operator from any assignable type.
         *        从任何可赋值类型的赋值运算符。
         *
         * @tparam UTy The assigned type
         *             被赋值的类型
         * @param value Value to assign
         *              要赋值的值
         * @return Reference to the held resources
         *         持有的资源的引用
         */
        template <typename UTy,
                  type_traits::other_trans::enable_if_t<type_traits::type_properties::is_assignable_v<type, UTy>, int> = 0>
        rain_fn operator=(UTy &&value) noexcept(type_traits::type_properties::is_nothrow_assignable_v<type, UTy>)->reference {
            take_resources = utility::forward<UTy>(value);
            return *this;
        }

        /**
         * @brief Swaps the held resources with another object.
         *        与另一个对象交换持有的资源。
         *
         * @tparam UTy The type to swap with
         *             要交换的类型
         * @param right The object to swap with
         *              要交换的对象
         */
        template <typename UTy, type_traits::other_trans::enable_if_t<type_traits::type_properties::is_swappable_v<type>, int> = 0>
        rain_fn swap(type &right) noexcept(type_traits::type_properties::is_nothrow_swappable_v<type>) -> void {
            using std::swap;
            swap(take_resources, right.take_resources);
        }

        /**
         * @brief Function call operator if the held type is invocable.
         *        如果持有的类型可调用，则提供函数调用运算符。
         *
         * @tparam Args Argument types
         *              参数类型
         * @param args Arguments to forward
         *             要转发的参数
         * @return Result of invoking the held callable
         *         调用持有的可调用对象的结果
         */
        template <typename... Args,
                  type_traits::other_trans::enable_if_t<type_traits::type_properties::is_invocable_v<type, Args...>, int> = 0>
        rain_fn operator()(Args &&...args)->type_traits::type_properties::invoke_result_t<type, Args...> {
            return utility::invoke(take_resources, utility::forward<Args>(args)...);
        }

        /**
         * @brief Move assignment operator.
         *        移动赋值运算符。
         *
         * @param right Another take to move from
         *              要移动的另一个take
         * @return Reference to this object
         *         此对象的引用
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
     * @brief Uninitialized storage wrapper.
     *        未初始化存储的包装器。
     *
     * @tparam Ty The type to store uninitialized
     *            要未初始化存储的类型
     */
    template <typename Ty>
    struct uninitialized {
        using type = Ty;

        uninitialized() {
        }
        uninitialized(const uninitialized &) = default;
        uninitialized(uninitialized &&) = default;
        uninitialized &operator=(const uninitialized &) = default;
        uninitialized &operator=(uninitialized &&) = default;

        type value;
    };

    /**
     * @brief Deleted specialization for const types.
     *        常量类型的已删除特化。
     */
    template <typename Ty>
    struct uninitialized<const Ty> {
        uninitialized() = delete;
    };

    /**
     * @brief Deleted specialization for lvalue reference types.
     *        左值引用类型的已删除特化。
     */
    template <typename Ty>
    struct uninitialized<Ty &> {
        uninitialized() = delete;
    };

    /**
     * @brief Deleted specialization for const lvalue reference types.
     *        常量左值引用类型的已删除特化。
     */
    template <typename Ty>
    struct uninitialized<const Ty &> {
        uninitialized() = delete;
    };

    /**
     * @brief Deleted specialization for rvalue reference types.
     *        右值引用类型的已删除特化。
     */
    template <typename Ty>
    struct uninitialized<Ty &&> {
        uninitialized() = delete;
    };

    /**
     * @brief Deleted specialization for const rvalue reference types.
     *        常量右值引用类型的已删除特化。
     */
    template <typename Ty>
    struct uninitialized<const Ty &&> {
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
