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
    template <typename Ty>
    using in = type_traits::other_trans::enable_if_t<
        !type_traits::type_relations::is_void_v<Ty>,
        type_traits::other_trans::conditional_t<type_traits::type_properties::prefer_pass_by_value_v<Ty>, Ty const, Ty const &>>;

    template <typename T>
    class deferred_init {
    public:
        deferred_init() noexcept {
        }

        ~deferred_init() noexcept {
            destroy();
        }

        rain_fn value() noexcept -> T & {
            foundation::exceptions::throw_exception_if<foundation::exceptions::runtime::runtime_error>(init);
            return t();
        }

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

    template <typename T>
    class out {
    public:
        template <typename>
        friend class borrow_out;

        out(T *t_) noexcept : t{t_}, has_t{true} {
            foundation::exceptions::throw_exception_if<foundation::exceptions::runtime::runtime_error>(t);
        }

        out(deferred_init<T> *dt_) noexcept : dt{dt_}, has_t{false} {
            foundation::exceptions::throw_exception_if<foundation::exceptions::runtime::runtime_error>(dt);
        }

        out(out<T> *ot_) noexcept : ot{ot_}, has_t{ot_->has_t} {
            foundation::exceptions::throw_exception_if<foundation::exceptions::runtime::runtime_error>(ot_);
            if (has_t) {
                t = ot->t;
            } else {
                dt = ot->dt;
            }
        }

        rain_fn called_construct() -> bool & {
            if (ot) {
                return ot->called_construct();
            } else {
                return called_construct_;
            }
        }

        ~out() {
            if (called_construct() && uncaught_count != foundation::exceptions::uncaught_exceptions()) {
                foundation::exceptions::throw_exception_if<foundation::exceptions::runtime::runtime_error>(!has_t);
                dt->destroy();
                called_construct() = false;
            }
        }

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

        refwrap(type &value, borrow_control_block *ctrl) : value{utility::addressof(value)}, ctrl{ctrl} {
            foundation::exceptions::throw_exception_if<foundation::exceptions::runtime::runtime_error>(
                ctrl->immutable_count.get() == 0, "you can not get a mut reference when you have cref");
            foundation::exceptions::throw_exception_if<foundation::exceptions::runtime::runtime_error>(
                !ctrl->mutable_active, "you cannot get a another mut reference");
        }

        ~refwrap() {
            ctrl->mutable_active = false;
        }

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

        crefwrap(type &value, borrow_control_block *ctrl) : value{utility::addressof(value)}, ctrl{ctrl} {
            foundation::exceptions::throw_exception_if<foundation::exceptions::runtime::runtime_error>(
                !ctrl->mutable_active, "you cannot get cref when you get a mut ref.");
        }

        ~crefwrap() {
            release();
        }

        operator const type &() {
            foundation::exceptions::throw_exception_if<foundation::exceptions::runtime::runtime_error>(
                !ctrl->mutable_active, "you cannot get cref when you release this ref.");
            foundation::exceptions::throw_exception_if<foundation::exceptions::runtime::runtime_error>(
                !ctrl->mutable_active, "you cannot get cref when you get a mut ref.");
            ++ctrl->immutable_count;
            return *value;
        }

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
    template <typename Ty>
    class borrow_out : out<Ty> {
    public:
        static_assert(type_traits::type_relations::is_same_v<type_traits::other_trans::decay_t<Ty>, Ty>);

        using refwrap = implements::refwrap<Ty>;
        using crefwrap = implements::crefwrap<Ty>;

        borrow_out(Ty *t_) noexcept : out<Ty>{t_} {
            ctrl = new implements::borrow_control_block{};
        }

        borrow_out(deferred_init<Ty> *dt_) noexcept : out<Ty>{dt_} {
            ctrl = new implements::borrow_control_block{};
        }

        borrow_out(out<Ty> *ot_) noexcept : out<Ty>{ot_} {
            ctrl = new implements::borrow_control_block{};
        }

        borrow_out(const borrow_out &other) : out<Ty>{other}, ctrl{other.ctrl} {
            ++ctrl->strong_count;
        }

        borrow_out &operator=(const borrow_out &other) {
            if (this != &other) {
                release_ctrl();
                out<Ty>::operator=(other);
                ctrl = other.ctrl;
                ++ctrl->strong_count;
            }
            return *this;
        }

        ~borrow_out() {
            release_ctrl();
        }

        rain_fn mut() -> refwrap {
            return refwrap{this->value(), ctrl};
        }

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
    template <typename Ty>
    static RAINY_CONSTEXPR_BOOL not_take_v = true;

    template <template <typename Ty> typename Template, typename Ty>
    static RAINY_CONSTEXPR_BOOL not_take_v<Template<Ty>> = type_traits::type_relations::is_same_v<Template<Ty>, take<Ty>>;
}

namespace rainy::annotations::lifetime {
    template <typename Ty>
    class take : type_traits::helper::non_copyable {
    public:
        using type = type_traits::cv_modify::remove_cvref_t<Ty>;
        using reference = type&;
        using pointer = type*;

        static_assert(implements::not_take_v<type>, "Do not pass take<Ty> to take<>");
        static_assert(type_traits::type_properties::is_move_constructible_v<type>,
                      "take<> require Ty is move constructible, but Ty is not.");

        static constexpr bool is_nothrow_move_construtible = type_traits::type_properties::is_nothrow_move_constructible_v<type>;
        static constexpr bool is_nothrow_move_assignible = type_traits::type_properties::is_nothrow_move_assignable_v<type>;

        take(Ty &resources) noexcept(is_nothrow_move_construtible) :
            take_resources{utility::move(resources)} {
        }

        take(Ty &&resources) noexcept(is_nothrow_move_construtible) :
            take_resources{utility::move(resources)} {
        }

        take(take &&right) noexcept(is_nothrow_move_construtible) : take_resources{utility::move(right.take_resources)} {
        }

        operator reference () noexcept {
            return take_resources;
        }

        rain_fn operator&() noexcept -> pointer {
            return &take_resources;
        }

        rain_fn get() -> reference {
            return take_resources;
        }

        template <typename UTy,
                  type_traits::other_trans::enable_if_t<type_traits::type_properties::is_assignable_v<type, UTy>, int> = 0>
        rain_fn operator=(UTy &&value) noexcept(type_traits::type_properties::is_nothrow_assignable_v<type, UTy>)->reference {
            take_resources = utility::forward<UTy>(value);
            return *this;
        }

        template <type_traits::other_trans::enable_if_t<type_traits::type_properties::is_swappable_v<type>, int> = 0>
        rain_fn swap(take<type> &right) noexcept(type_traits::type_properties::is_nothrow_swappable_v<type>) -> void {
            using utility::swap;
            swap(take_resources, right.take_resources);
        }

        template <typename... Args,
                  type_traits::other_trans::enable_if_t<type_traits::type_properties::is_invocable_v<type, Args...>, int> = 0>
        rain_fn operator()(Args &&...args) -> type_traits::type_properties::invoke_result_t<type, Args...> {
            return utility::invoke(take_resources, utility::forward<Args>(args)...);
        }

        rain_fn operator=(take &&right) noexcept(is_nothrow_move_assignible) -> take& {
            take_resources = utility::move(right.take_resources);
            return *this;
        }

    private:
        type take_resources;
    };
}

namespace rainy::utility {
    using annotations::lifetime::borrow_out;
    using annotations::lifetime::deferred_init;
    using annotations::lifetime::in;
    using annotations::lifetime::out;
    using annotations::lifetime::take;
}

#endif
