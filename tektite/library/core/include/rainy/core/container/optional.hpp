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
#ifndef RAINY_CORE_CONTAINER_OPTIONAL_HPP
#define RAINY_CORE_CONTAINER_OPTIONAL_HPP
#include <optional>
#include <rainy/core/annotations/smf_control.hpp>
#include <rainy/core/diagnostics/exceptions.hpp>
#include <rainy/core/text/string.hpp>
#include <rainy/core/type_traits.hpp>

#if RAINY_USING_MSVC
#pragma warning(push)
#pragma warning(disable : 26495)
#endif

namespace rainy::core::container {
    using std::nullopt;
    using std::nullopt_t;

    template <typename Ty>
    class optional;
}

namespace rainy::core::exceptions::runtime {
    /**
     * \lang english
     * @brief Exception thrown on invalid access to the contained value of an empty optional.
     *
     * \lang simp-chinese
     * @brief 在非法访问空的 optional 中存放的值时抛出的异常。
     */
    class bad_optional_access final : public runtime_error {
    public:
        using base = runtime_error;

        /**
         * \lang english
         * @brief Constructs a bad_optional_access exception.
         *
         * @param location The source location where the exception was created
         *
         * \lang simp-chinese
         * @brief 构造一个 bad_optional_access 异常。
         *
         * @param location 创建异常时的源位置
         */
        explicit bad_optional_access(const source &location = source::current()) : base("bad variant access", location) {
        }
    };

    // clang-format off

    /**
     * \lang english
     * @brief Throws a bad_optional_access exception.
     *
     * @param location The source location of the throw
     *
     * \lang simp-chinese
     * @brief 抛出 bad_optional_access 异常。
     *
     * @param location 抛出点的源位置
     */
    RAINY_INLINE rain_fn throw_bad_optional_access(const diagnostics::source_location &location = diagnostics::source_location::current()) -> void {
        throw_exception(bad_optional_access{location});
    }

    // clang-format on
}

// @NODOCBEGIN
namespace rainy::core::container::implements {
#if RAINY_HAS_CXX20
    template <typename UTy>
    concept is_derived_from_optional =
        requires { typename UTy::value_type; } && type_traits::type_relations::is_same_v<UTy, optional<typename UTy::value_type>>;
#endif

    template <typename Ty, bool = type_traits::properties::is_trivially_destructible_v<Ty>>
    struct optional_destruct_base {
        constexpr optional_destruct_base() : dummy{}, has_value_{false} { // NOLINT
        }

        // NOLINTBEGIN
        template <typename... Args>
        constexpr optional_destruct_base(std::in_place_t, Args &&...args) : value_(utility::forward<Args>(args)...), has_value_{true} {
        }

        constexpr optional_destruct_base(const optional_destruct_base &other) : has_value_{false} {
            if (other.has_value_) {
                utility::construct_in_place(value_, other.value_);
                has_value_ = true;
            }
        }

        constexpr optional_destruct_base(optional_destruct_base &&other) noexcept(
            type_traits::properties::is_nothrow_move_constructible_v<Ty>) : has_value_{false} {
            if (other.has_value_) {
                utility::construct_in_place(value_, utility::move(other.value_));
                has_value_ = true;
            }
        }
        // NOLINTEND

#if RAINY_HAS_CXX20
        constexpr ~optional_destruct_base() {
            reset();
        }
#endif

        constexpr optional_destruct_base &operator=(const optional_destruct_base &other) {
            if (this != &other) {
                if (other.has_value_) {
                    if (has_value_) {
                        value_ = other.value_;
                    } else {
                        utility::construct_in_place(value_, other.value_);
                        has_value_ = true;
                    }
                } else {
                    reset();
                }
            }
            return *this;
        }

        constexpr optional_destruct_base &operator=(optional_destruct_base &&other) noexcept(
            type_traits::properties::is_nothrow_move_constructible_v<Ty> &&
            type_traits::properties::is_nothrow_move_assignable_v<Ty>) {
            if (this != &other) {
                if (other.has_value_) {
                    if (has_value_) {
                        value_ = utility::move(other.value_);
                    } else {
                        utility::construct_in_place(value_, utility::move(other.value_));
                        has_value_ = true;
                    }
                } else {
                    reset();
                }
            }
            return *this;
        }

        constexpr void reset() noexcept {
            this->has_value_ = false;
        }

        union {
            std::in_place_t dummy;
            type_traits::modifers::remove_cv_t<Ty> value_;
        };
        bool has_value_;
    };

    template <typename Ty>
    struct optional_destruct_base<Ty, false> {
        optional_destruct_base() : has_value_{false} { // NOLINT
        }

        template <typename... Args>
        constexpr optional_destruct_base(std::in_place_t, Args &&...args) :
            value_(utility::forward<Args>(args)...), has_value_{true} { // NOLINT
        }

        RAINY_CONSTEXPR20 ~optional_destruct_base() {
            this->reset();
        }

        RAINY_CONSTEXPR20 optional_destruct_base(const optional_destruct_base &other) : has_value_{false} { // NOLINT
            if (other.has_value_) {
                utility::construct_in_place(value_, other.value_);
                has_value_ = true;
            }
        }

        RAINY_CONSTEXPR20 optional_destruct_base(optional_destruct_base &&other) noexcept( // NOLINT
            type_traits::properties::is_nothrow_move_constructible_v<Ty>) : has_value_{false} {
            if (other.has_value_) {
                utility::construct_in_place(value_, utility::move(other.value_));
                has_value_ = true;
            }
        }

        RAINY_CONSTEXPR20 optional_destruct_base &operator=(const optional_destruct_base &other) noexcept(
            type_traits::properties::is_nothrow_copy_constructible_v<Ty> &&
            type_traits::properties::is_nothrow_copy_assignable_v<Ty>) {
            if (this != &other) {
                if (other.has_value_) {
                    if (has_value_) {
                        value_ = other.value_;
                    } else {
                        utility::construct_in_place(value_, other.value_);
                        has_value_ = true;
                    }
                } else {
                    reset();
                }
            }
            return *this;
        }

        RAINY_CONSTEXPR20 optional_destruct_base &operator=(optional_destruct_base &&other) noexcept(
            type_traits::properties::is_nothrow_move_constructible_v<Ty> &&
            type_traits::properties::is_nothrow_move_assignable_v<Ty>) {
            if (this != &other) {
                if (other.has_value_) {
                    if (has_value_) {
                        value_ = utility::move(other.value_);
                    } else {
                        utility::construct_in_place(value_, utility::move(other.value_));
                        has_value_ = true;
                    }
                } else {
                    reset();
                }
            }
            return *this;
        }

        constexpr void reset() noexcept {
            if (this->has_value_) {
                value_.~Ty();
            }
            this->has_value_ = false;
        }

        union {
            std::in_place_t dummy;
            type_traits::modifers::remove_cv_t<Ty> value_;
        };
        bool has_value_;
    };

    template <typename Ty>
    struct optional_base : optional_destruct_base<Ty>,
                           annotations::smf_control::constructible_base<optional_base<Ty>>,
                           annotations::smf_control::assignable_base<optional_base<Ty>> {
        using optional_destruct_base<Ty>::optional_destruct_base;

        template <typename UTy>
        using allow_direct_conversion = type_traits::helper::bool_constant<
            !type_traits::type_relations::is_same_v<type_traits::modifers::remove_cvref_t<UTy>, optional<Ty>> &&
            !type_traits::type_relations::is_same_v<type_traits::modifers::remove_cvref_t<UTy>, std::in_place_t> &&
            !(type_traits::type_relations::is_same_v<type_traits::modifers::remove_cv_t<Ty>, bool> &&
              type_traits::primary_types::is_specialization_v<type_traits::modifers::remove_cvref_t<UTy>, optional>) &&
            type_traits::properties::is_constructible_v<Ty, UTy>>;

        template <typename UTy>
        using allow_unwrapping_assignment = type_traits::helper::bool_constant<!type_traits::logical_traits::disjunction_v<
            type_traits::type_relations::is_same<Ty, UTy>, type_traits::properties::is_assignable<Ty &, optional<UTy> &>,
            type_traits::properties::is_assignable<Ty &, const optional<UTy> &>,
            type_traits::properties::is_assignable<Ty &, const optional<UTy>>,
            type_traits::properties::is_assignable<Ty &, optional<UTy>>>>;

        template <typename UTy>
        using allow_unwrapping = type_traits::helper::bool_constant<type_traits::logical_traits::disjunction_v<
            type_traits::type_relations::is_same<type_traits::modifers::remove_cv_t<Ty>, bool>,
            type_traits::logical_traits::negation<type_traits::logical_traits::disjunction<
                type_traits::type_relations::is_same<Ty, UTy>, type_traits::properties::is_constructible<Ty, optional<UTy> &>,
                type_traits::properties::is_constructible<Ty, const optional<UTy> &>,
                type_traits::properties::is_constructible<Ty, const optional<UTy>>,
                type_traits::properties::is_constructible<Ty, optional<UTy>>,
                type_traits::type_relations::is_convertible<optional<UTy> &, Ty>,
                type_traits::type_relations::is_convertible<const optional<UTy> &, Ty>,
                type_traits::type_relations::is_convertible<const optional<UTy>, Ty>,
                type_traits::type_relations::is_convertible<optional<UTy>, Ty>>>>>;

        template <typename UTy>
        using allow_assignment = type_traits::helper::bool_constant<
            !type_traits::type_relations::is_same_v<optional<Ty>, type_traits::modifers::remove_cvref_t<UTy>> &&
            !type_traits::logical_traits::conjunction_v<
                type_traits::composite_types::is_scalar<Ty>,
                type_traits::type_relations::is_same<Ty, type_traits::other_trans::decay_t<UTy>>> &&
            type_traits::properties::is_constructible_v<Ty, UTy> && type_traits::properties::is_assignable_v<Ty &, UTy>>;

        template <typename Self>
        RAINY_CONSTEXPR20 void construct_impl_(Self &&right) noexcept(
            type_traits::properties::is_nothrow_constructible_v<Ty, decltype(*utility::forward<Self>(right))>) {
            if (right.has_value_) {
                construct_value_(*utility::forward<Self>(right));
            }
        }

        template <typename Self>
        RAINY_CONSTEXPR20 void assign_impl_(Self &&right) noexcept(
            type_traits::properties::is_nothrow_constructible_v<Ty, decltype(*utility::forward<Self>(right))> &&
            type_traits::properties::is_nothrow_assignable_v<Ty &, decltype(*utility::forward<Self>(right))>) {
            if (right.has_value_) {
                assign_value_(*utility::forward<Self>(right));
            } else {
                this->reset();
            }
        }

        template <typename UTy>
        RAINY_CONSTEXPR20 void assign_value_(UTy &&right) noexcept(type_traits::properties::is_nothrow_assignable_v<Ty &, UTy> &&
                                                                   type_traits::properties::is_nothrow_constructible_v<Ty, UTy>) {
            if (this->has_value_) {
                static_cast<Ty &>(this->value_) = utility::forward<UTy>(right);
            } else {
                construct_value_(utility::forward<UTy>(right));
            }
        }

        template <typename UTy>
        RAINY_CONSTEXPR20 void construct_value_(UTy &&val) {
            utility::construct_in_place(this->value_, utility::forward<UTy>(val));
            this->has_value_ = true;
        }

        RAINY_NODISCARD constexpr Ty &operator*() & noexcept {
            assert(this->has_value_ && "operator*() called on empty optional");
            return this->value_;
        }

        RAINY_NODISCARD constexpr const Ty &operator*() const & noexcept {
            assert(this->has_value_ && "operator*() called on empty optional");
            return this->value_;
        }

        RAINY_NODISCARD constexpr Ty &&operator*() && noexcept {
            assert(this->has_value_ && "operator*() called on empty optional");
            return utility::move(this->value_);
        }

        RAINY_NODISCARD constexpr const Ty &&operator*() const && noexcept {
            assert(this->has_value_ && "operator*() called on empty optional");
            return utility::move(this->value_);
        }

        template <typename... Args>
        RAINY_CONSTEXPR20 Ty &construct_(Args &&...val) {
            this->reset();
            utility::construct_in_place(this->value_, utility::forward<Args>(val)...);
            this->has_value_ = true;
            return this->value_;
        }
    };
}
// @NODOCEND

namespace rainy::core::container {
    /**
     * \lang english
     * @brief A value wrapper that may or may not contain a value of type Ty.
     *         Provides checked access to the contained value and supports monadic operations.
     *
     * @tparam Ty The type of the value that may be contained
     *
     * \lang simp-chinese
     * @brief 可能包含也可能不包含 Ty 类型值的值包装器。
     *         提供对被包含值的安全访问，并支持单子操作。
     *
     * @tparam Ty 可能被包含的值的类型
     */
    template <typename Ty>
    class optional final : private annotations::smf_control::control<implements::optional_base<Ty>> {
    public:
        using base = annotations::smf_control::control<implements::optional_base<Ty>>;

        /**
         * \lang english
         * @brief The type of the contained value.
         *
         * \lang simp-chinese
         * @brief 被包含值的类型。
         */
        using value_type = Ty;

        /**
         * \lang english
         * @brief Iterator type over the contained value.
         *
         * \lang simp-chinese
         * @brief 遍历被包含值的迭代器类型。
         */
        using iterator = Ty *;

        /**
         * \lang english
         * @brief Const iterator type over the contained value.
         *
         * \lang simp-chinese
         * @brief 遍历被包含值的常量迭代器类型。
         */
        using const_iterator = const Ty *;

        // @NODOCBEGIN
        template <typename UTy>
        using allow_direct_conversion = typename base::template allow_direct_conversion<UTy>; // NOLINT

        template <typename UTy>
        using allow_unwrapping = typename base::template allow_unwrapping<UTy>; // NOLINT

        template <typename UTy>
        using allow_unwrapping_assignment = typename base::template allow_unwrapping_assignment<UTy>; // NOLINT

        template <typename UTy>
        using allow_assignment = typename base::template allow_assignment<UTy>; // NOLINT
        // @NODOCEND

        /**
         * \lang english
         * @brief Default constructor. Creates an empty optional.
         *
         * \lang simp-chinese
         * @brief 默认构造函数。创建一个空的 optional。
         */
        constexpr optional() noexcept = default;

        /**
         * \lang english
         * @brief Constructs an empty optional from nullopt.
         *
         * @param nullopt_t A tag value of type nullopt_t
         *
         * \lang simp-chinese
         * @brief 从 nullopt 构造一个空的 optional。
         *
         * @param nullopt_t nullopt_t 类型的标签值
         */
        constexpr optional(nullopt_t) noexcept {
        }

        /**
         * \lang english
         * @brief In-place constructor. Constructs the contained value from the given arguments.
         *
         * @tparam Args The types of the arguments forwarded to the constructor of Ty
         * @param args The arguments forwarded to the constructor of Ty
         *
         * \lang simp-chinese
         * @brief 就地构造函数。使用给定实参构造被包含的值。
         *
         * @tparam Args 转发给 Ty 构造函数的实参类型
         * @param args 转发给 Ty 构造函数的实参
         */
        template <typename... Args>
        constexpr explicit optional(std::in_place_t, Args &&...args) : base(std::in_place, utility::forward<Args>(args)...) {
        }

        /**
         * \lang english
         * @brief In-place constructor with an initializer list.
         *
         * @tparam Elem The type of the elements of the initializer list
         * @tparam Args The types of the remaining arguments forwarded to the constructor of Ty
         * @param ilist The initializer list passed to the constructor of Ty
         * @param args The remaining arguments forwarded to the constructor of Ty
         *
         * \lang simp-chinese
         * @brief 带初始化器列表的就地构造函数。
         *
         * @tparam Elem 初始化器列表元素的类型
         * @tparam Args 转发给 Ty 构造函数的其余实参类型
         * @param ilist 传递给 Ty 构造函数的初始化器列表
         * @param args 转发给 Ty 构造函数的其余实参
         */
        template <typename Elem, typename... Args>
        constexpr explicit optional(std::in_place_t, std::initializer_list<Elem> ilist, Args &&...args) :
            base(std::in_place, ilist, utility::forward<Args>(args)...) {
        }

        /**
         * \lang english
         * @brief Converting constructor. Constructs the contained value from a value convertible to Ty.
         *
         * @tparam UTy The type of the value to convert from
         * @param val The value used to initialize the contained value
         *
         * \lang simp-chinese
         * @brief 转换构造函数。从可转换为 Ty 的值构造被包含的值。
         *
         * @tparam UTy 要转换的值的类型
         * @param val 用于初始化被包含值的值
         */
        template <typename UTy = type_traits::modifers::remove_cv_t<Ty>,
                  type_traits::other_trans::enable_if_t<allow_direct_conversion<UTy>::value, int> = 0>
        constexpr optional(UTy &&val) noexcept(type_traits::properties::is_nothrow_constructible_v<Ty, UTy>) : // NOLINT
            base(std::in_place, utility::forward<UTy>(val)) {
        }

        /**
         * \lang english
         * @brief Converting copy constructor from another optional whose value type is convertible to Ty.
         *
         * @tparam UTy The value type of the source optional
         * @param right The source optional to copy from
         *
         * \lang simp-chinese
         * @brief 从值类型可转换为 Ty 的另一个 optional 转换拷贝构造。
         *
         * @tparam UTy 源 optional 的值类型
         * @param right 要拷贝的源 optional
         */
        template <typename UTy,
                  type_traits::other_trans::enable_if_t<type_traits::logical_traits::conjunction_v<allow_unwrapping<UTy>>, int> = 0>
        explicit RAINY_CONSTEXPR20 optional(const optional<UTy> &right) {
            static_assert(type_traits::properties::is_constructible_v<Ty, const UTy &>,
                          "Cannot passing right [type = const optional<UTy>&] to make a copy because is_constructible<Ty, const UTy "
                          "&> results false");
            if (right.has_value()) {
                this->construct_(*right);
            }
        }

        /**
         * \lang english
         * @brief Converting move constructor from another optional whose value type is convertible to Ty.
         *
         * @tparam UTy The value type of the source optional
         * @param right The source optional to move from
         *
         * \lang simp-chinese
         * @brief 从值类型可转换为 Ty 的另一个 optional 转换移动构造。
         *
         * @tparam UTy 源 optional 的值类型
         * @param right 要移动的源 optional
         */
        template <typename UTy,
                  type_traits::other_trans::enable_if_t<type_traits::logical_traits::conjunction_v<allow_unwrapping<UTy>>, int> = 0>
        explicit RAINY_CONSTEXPR20 optional(optional<UTy> &&right) {
            static_assert(
                type_traits::properties::is_constructible_v<Ty, UTy>,
                "Cannot passing right [type = optional<UTy>&&] to make a move because is_constructible<Ty, UTy> results false");
            if (right.has_value()) {
                this->construct_(utility::move(*right));
            }
        }

        /**
         * \lang english
         * @brief Destructor. Destroys the contained value if present.
         *
         * \lang simp-chinese
         * @brief 析构函数。若存在被包含的值则销毁它。
         */
        ~optional() = default;

        /**
         * \lang english
         * @brief Checks whether the optional contains a value.
         *
         * @return true if a value is contained, false otherwise
         *
         * \lang simp-chinese
         * @brief 检查 optional 是否包含值。
         *
         * @return 若包含值则返回 true，否则返回 false
         */
        RAINY_NODISCARD constexpr bool has_value() const noexcept {
            return this->has_value_;
        }

        /**
         * \lang english
         * @brief Assigns a value convertible to Ty to the optional, constructing it if the optional is empty.
         *
         * @tparam UTy The type of the value to assign
         * @param right The value to assign
         *
         * \lang simp-chinese
         * @brief 将可转换为 Ty 的值赋给 optional，若 optional 为空则构造它。
         *
         * @tparam UTy 要赋的值的类型
         * @param right 要赋的值
         */
        template <typename UTy, type_traits::other_trans::enable_if_t<allow_assignment<UTy>::value, int> = 0>
        RAINY_CONSTEXPR20 void assign(UTy &&right) noexcept(type_traits::properties::is_nothrow_assignable_v<Ty &, UTy> &&
                                                            type_traits::properties::is_nothrow_constructible_v<Ty, UTy>) {
            if (this->has_value()) {
                static_cast<Ty &>(this->value_) = utility::forward<UTy>(right);
            } else {
                this->construct_(utility::forward<UTy>(right));
            }
        }

        /**
         * \lang english
         * @brief Assigns nullopt to the optional, destroying the contained value if present.
         *
         * @return Reference to this optional
         *
         * \lang simp-chinese
         * @brief 将 nullopt 赋给 optional，若存在被包含的值则销毁它。
         *
         * @return 此 optional 的引用
         */
        RAINY_CONSTEXPR20 optional &operator=(nullopt_t) noexcept {
            this->reset();
            return *this;
        }

        /**
         * \lang english
         * @brief Assigns from another optional whose value type is convertible to Ty.
         *
         * @tparam UTy The value type of the source optional
         * @param right The source optional to copy from
         * @return Reference to this optional
         *
         * \lang simp-chinese
         * @brief 从值类型可转换为 Ty 的另一个 optional 赋值。
         *
         * @tparam UTy 源 optional 的值类型
         * @param right 要拷贝的源 optional
         * @return 此 optional 的引用
         */
        template <typename UTy, type_traits::other_trans::enable_if_t<allow_unwrapping_assignment<UTy>::value, int> = 0>
        RAINY_CONSTEXPR20 optional &operator=(const optional<UTy> &right) noexcept(
            type_traits::properties::is_nothrow_assignable_v<Ty &, const UTy &> &&
            type_traits::properties::is_nothrow_constructible_v<Ty, const UTy &>) /* strengthened */ {
            static_assert(type_traits::properties::is_constructible_v<Ty, const UTy &> &&
                              type_traits::properties::is_assignable_v<Ty &, const UTy &>,
                          "Cannot passing right [type = const optional<UTy>&] to make a copy because "
                          "type_traits::properties::is_constructible_v<Ty, const UTy &> &&"
                          "type_traits::properties::is_assignable_v<Ty &, const UTy &> results false");
            if (right) {
                this->assign(*right);
            } else {
                reset();
            }
            return *this;
        }

        /**
         * \lang english
         * @brief Assigns from another optional by moving, whose value type is convertible to Ty.
         *
         * @tparam UTy The value type of the source optional
         * @param right The source optional to move from
         * @return Reference to this optional
         *
         * \lang simp-chinese
         * @brief 通过移动从值类型可转换为 Ty 的另一个 optional 赋值。
         *
         * @tparam UTy 源 optional 的值类型
         * @param right 要移动的源 optional
         * @return 此 optional 的引用
         */
        template <typename UTy, type_traits::other_trans::enable_if_t<allow_unwrapping_assignment<UTy>::value, int> = 0>
        RAINY_CONSTEXPR20 optional &operator=(optional<UTy> &&right) noexcept(
            type_traits::properties::is_nothrow_assignable_v<Ty &, const UTy &> &&
            type_traits::properties::is_nothrow_constructible_v<Ty, const UTy &>) /* strengthened */ {
            static_assert(type_traits::properties::is_constructible_v<Ty, UTy> && type_traits::properties::is_assignable_v<Ty &, UTy>,
                          "Cannot passing right [type = optional<UTy>&&] to make a copy because "
                          "type_traits::properties::is_constructible_v<Ty, UTy> && "
                          "type_traits::properties::is_assignable_v<Ty &, UTy> results false");
            if (right) {
                this->assign(*right);
            } else {
                reset();
            }
            return *this;
        }

        using base::operator*;

        /**
         * \lang english
         * @brief Accesses the contained value through a pointer. Throws bad_optional_access if the optional is empty.
         *
         * @return A pointer to the contained value
         *
         * \lang simp-chinese
         * @brief 通过指针访问被包含的值。若 optional 为空则抛出 bad_optional_access。
         *
         * @return 指向被包含值的指针
         */
        RAINY_NODISCARD constexpr value_type *operator->() {
            if (!has_value()) {
                core::exceptions::runtime::throw_bad_optional_access();
            }
            return utility::addressof(this->value_);
        }

        /**
         * \lang english
         * @brief Accesses the contained value through a const pointer. Throws bad_optional_access if the optional is empty.
         *
         * @return A const pointer to the contained value
         *
         * \lang simp-chinese
         * @brief 通过常量指针访问被包含的值。若 optional 为空则抛出 bad_optional_access。
         *
         * @return 指向被包含值的常量指针
         */
        RAINY_NODISCARD constexpr const value_type *operator->() const {
            if (!has_value()) {
                core::exceptions::runtime::throw_bad_optional_access();
            }
            return utility::addressof(this->value_);
        }

        /**
         * \lang english
         * @brief Returns a reference to the contained value. Throws bad_optional_access if the optional is empty.
         *
         * @return A reference to the contained value
         *
         * \lang simp-chinese
         * @brief 返回被包含值的引用。若 optional 为空则抛出 bad_optional_access。
         *
         * @return 被包含值的引用
         */
        RAINY_NODISCARD constexpr Ty &value() & {
            if (!has_value()) {
                core::exceptions::runtime::throw_bad_optional_access();
            }
            return this->value_;
        }

        /**
         * \lang english
         * @brief Returns a const reference to the contained value. Throws bad_optional_access if the optional is empty.
         *
         * @return A const reference to the contained value
         *
         * \lang simp-chinese
         * @brief 返回被包含值的常量引用。若 optional 为空则抛出 bad_optional_access。
         *
         * @return 被包含值的常量引用
         */
        RAINY_NODISCARD constexpr const Ty &value() const & {
            if (!has_value()) {
                core::exceptions::runtime::throw_bad_optional_access();
            }
            return this->value_;
        }

        /**
         * \lang english
         * @brief Returns an rvalue reference to the contained value. Throws bad_optional_access if the optional is empty.
         *
         * @return An rvalue reference to the contained value
         *
         * \lang simp-chinese
         * @brief 返回被包含值的右值引用。若 optional 为空则抛出 bad_optional_access。
         *
         * @return 被包含值的右值引用
         */
        RAINY_NODISCARD constexpr Ty &&value() && {
            if (!has_value()) {
                core::exceptions::runtime::throw_bad_optional_access();
            }
            return utility::move(this->value_);
        }

        /**
         * \lang english
         * @brief Returns a const rvalue reference to the contained value. Throws bad_optional_access if the optional is empty.
         *
         * @return A const rvalue reference to the contained value
         *
         * \lang simp-chinese
         * @brief 返回被包含值的常量右值引用。若 optional 为空则抛出 bad_optional_access。
         *
         * @return 被包含值的常量右值引用
         */
        RAINY_NODISCARD constexpr const Ty &&value() const && {
            if (!has_value()) {
                core::exceptions::runtime::throw_bad_optional_access();
            }
            return utility::move(this->value_);
        }

        /**
         * \lang english
         * @brief Returns the contained value or a default value if the optional is empty.
         *
         * @tparam UTy The type of the fallback value
         * @param right The fallback value used when the optional is empty
         * @return The contained value if present, otherwise the fallback value
         *
         * \lang simp-chinese
         * @brief 返回被包含的值；若 optional 为空则返回默认值。
         *
         * @tparam UTy 回退值的类型
         * @param right 当 optional 为空时使用的回退值
         * @return 若存在被包含的值则返回它，否则返回回退值
         */
        template <typename UTy = type_traits::modifers::remove_cv_t<Ty>>
        RAINY_NODISCARD constexpr type_traits::modifers::remove_cv_t<Ty> value_or(UTy &&right) const & {
            static_assert(type_traits::type_relations::is_convertible_v<const Ty &, type_traits::modifers::remove_cv_t<Ty>>,
                          "The const overload of optional<Ty>::value_or requires const Ty& to be convertible to remove_cv_t<Ty> ");
            static_assert(type_traits::type_relations::is_convertible_v<UTy, type_traits::modifers::remove_cv_t<Ty>>,
                          "optional<Ty>::value_or(UTy) requires UTy to be convertible to remove_cv_t<Ty> ");
            if (this->has_value()) {
                return static_cast<const Ty &>(this->value_);
            }
            return static_cast<type_traits::modifers::remove_cv_t<Ty>>(utility::forward<UTy>(right));
        }

        /**
         * \lang english
         * @brief Returns the contained value by moving, or a default value if the optional is empty.
         *
         * @tparam UTy The type of the fallback value
         * @param right The fallback value used when the optional is empty
         * @return The contained value if present, otherwise the fallback value
         *
         * \lang simp-chinese
         * @brief 通过移动返回被包含的值；若 optional 为空则返回默认值。
         *
         * @tparam UTy 回退值的类型
         * @param right 当 optional 为空时使用的回退值
         * @return 若存在被包含的值则返回它，否则返回回退值
         */
        template <typename UTy = type_traits::modifers::remove_cv_t<Ty>>
        RAINY_NODISCARD constexpr type_traits::modifers::remove_cv_t<Ty> value_or(UTy &&right) && {
            static_assert(type_traits::type_relations::is_convertible_v<Ty, type_traits::modifers::remove_cv_t<Ty>>,
                          "The rvalue overload of optional<Ty>::value_or requires Ty to be convertible to remove_cv_t<Ty> ");
            static_assert(type_traits::type_relations::is_convertible_v<UTy, type_traits::modifers::remove_cv_t<Ty>>,
                          "optional<Ty>::value_or(UTy) requires UTy to be convertible to remove_cv_t<Ty> ");
            if (this->has_value()) {
                return static_cast<Ty &&>(this->value_);
            }
            return static_cast<type_traits::modifers::remove_cv_t<Ty>>(utility::forward<UTy>(right));
        }

        /**
         * \lang english
         * @brief Swaps the contents of two optionals.
         *
         * @param right The optional to swap with
         *
         * \lang simp-chinese
         * @brief 交换两个 optional 的内容。
         *
         * @param right 要与之交换的 optional
         */
        RAINY_CONSTEXPR20 void swap(optional &right) noexcept(type_traits::properties::is_nothrow_move_constructible_v<Ty> &&
                                                              type_traits::properties::is_nothrow_swappable_v<Ty>) {

            if constexpr (type_traits::properties::is_move_constructible_v<Ty>) {
                static_assert(type_traits::properties::is_swappable_v<Ty>, "optional<Ty>::swap requires Ty to be swappable");
            } else {
                static_assert(false, "optional<Ty>::swap requires Ty to be move constructible");
            }
            const bool this_engaged = this->has_value();
            const bool right_engaged = right.has_value();
            if (!this_engaged && !right_engaged) {
                return;
            }
            if (this_engaged && right_engaged) {
                using std::swap;
                swap(*(*this), *right);
            } else {
                optional &source = this_engaged ? *this : right;
                optional &target = this_engaged ? right : *this;
                target.construct_(utility::move(*source));
                source.reset();
            }
        }

        using base::reset;

        /**
         * \lang english
         * @brief Assigns a value convertible to Ty to the optional.
         *
         * @tparam UTy The type of the value to assign
         * @param right The value to assign
         * @return Reference to this optional
         *
         * \lang simp-chinese
         * @brief 将可转换为 Ty 的值赋给 optional。
         *
         * @tparam UTy 要赋的值的类型
         * @param right 要赋的值
         * @return 此 optional 的引用
         */
        template <typename UTy, type_traits::other_trans::enable_if_t<allow_assignment<UTy>::value, int> = 0>
        RAINY_CONSTEXPR20 optional &operator=(UTy &&right) noexcept(type_traits::properties::is_nothrow_assignable_v<Ty &, UTy> &&
                                                                    type_traits::properties::is_nothrow_constructible_v<Ty, UTy>) {
            this->assign(utility::forward<UTy>(right));
            return *this;
        }

        /**
         * \lang english
         * @brief Checks whether the optional contains a value.
         *
         * @return true if a value is contained, false otherwise
         *
         * \lang simp-chinese
         * @brief 检查 optional 是否包含值。
         *
         * @return 若包含值则返回 true，否则返回 false
         */
        explicit operator bool() const noexcept {
            return this->has_value();
        }

        /**
         * \lang english
         * @brief Destroys the contained value if present, then constructs a new one in place from the given arguments.
         *
         * @tparam Args The types of the arguments forwarded to the constructor of Ty
         * @param args The arguments forwarded to the constructor of Ty
         * @return A reference to the newly constructed value
         *
         * \lang simp-chinese
         * @brief 若存在被包含的值则销毁它，然后使用给定实参就地构造一个新的值。
         *
         * @tparam Args 转发给 Ty 构造函数的实参类型
         * @param args 转发给 Ty 构造函数的实参
         * @return 新构造值的引用
         */
        template <typename... Args>
        RAINY_CONSTEXPR20 value_type &emplace(Args &&...args) {
            this->reset();
            return this->construct_(utility::forward<Args>(args)...);
        }

        /**
         * \lang english
         * @brief Destroys the contained value if present, then constructs a new one in place from an initializer list and arguments.
         *
         * @tparam Elem The type of the elements of the initializer list
         * @tparam Args The types of the remaining arguments forwarded to the constructor of Ty
         * @param ilist The initializer list passed to the constructor of Ty
         * @param args The remaining arguments forwarded to the constructor of Ty
         * @return A reference to the newly constructed value
         *
         * \lang simp-chinese
         * @brief 若存在被包含的值则销毁它，然后使用初始化器列表和实参就地构造一个新的值。
         *
         * @tparam Elem 初始化器列表元素的类型
         * @tparam Args 转发给 Ty 构造函数的其余实参类型
         * @param ilist 传递给 Ty 构造函数的初始化器列表
         * @param args 转发给 Ty 构造函数的其余实参
         * @return 新构造值的引用
         */
        template <typename Elem, typename... Args>
        RAINY_CONSTEXPR20 value_type &emplace(std::initializer_list<Elem> ilist, Args &&...args) {
            this->reset();
            return this->construct_(ilist, utility::forward<Args>(args)...);
        }

        /**
         * \lang english
         * @brief Applies a callable to the contained value and returns the resulting optional, or an empty optional if empty.
         *
         * @tparam Fx The type of the callable
         * @param fx The callable applied to the contained value
         * @return The optional returned by the callable, or an empty optional
         *
         * \lang simp-chinese
         * @brief 将可调用对象应用于被包含的值并返回结果 optional；若为空则返回空的 optional。
         *
         * @tparam Fx 可调用对象的类型
         * @param fx 应用于被包含值的可调用对象
         * @return 可调用对象返回的 optional；若为空则返回空的 optional
         */
        template <typename Fx>
        constexpr auto and_then(Fx fx) & {
            using result_type = type_traits::properties::invoke_result_t<Fx, Ty &>;
            static_assert(type_traits::primary_types::is_specialization_v<result_type, optional>, "Fx must return optional<UTy>");
            if (has_value()) {
                return utility::invoke(utility::forward<Fx>(fx), this->value_);
            }
            return result_type{};
        }

        /**
         * \lang english
         * @brief Applies a callable to the contained value and returns the resulting optional, or an empty optional if empty.
         *
         * @tparam Fx The type of the callable
         * @param fx The callable applied to the contained value
         * @return The optional returned by the callable, or an empty optional
         *
         * \lang simp-chinese
         * @brief 将可调用对象应用于被包含的值并返回结果 optional；若为空则返回空的 optional。
         *
         * @tparam Fx 可调用对象的类型
         * @param fx 应用于被包含值的可调用对象
         * @return 可调用对象返回的 optional；若为空则返回空的 optional
         */
        template <typename Fx>
        constexpr auto and_then(Fx &&fx) && {
            using result_type = type_traits::properties::invoke_result_t<Fx, Ty &&>;
            static_assert(type_traits::primary_types::is_specialization_v<result_type, optional>, "Fx must return optional<UTy>");
            if (has_value()) {
                return utility::invoke(utility::forward<Fx>(fx), utility::move(this->value_));
            }
            return result_type{};
        }

        /**
         * \lang english
         * @brief Applies a callable to the contained value and returns the resulting optional, or an empty optional if empty.
         *
         * @tparam Fx The type of the callable
         * @param fx The callable applied to the contained value
         * @return The optional returned by the callable, or an empty optional
         *
         * \lang simp-chinese
         * @brief 将可调用对象应用于被包含的值并返回结果 optional；若为空则返回空的 optional。
         *
         * @tparam Fx 可调用对象的类型
         * @param fx 应用于被包含值的可调用对象
         * @return 可调用对象返回的 optional；若为空则返回空的 optional
         */
        template <typename Fx>
        constexpr auto and_then(Fx fx) const & {
            using result_type = type_traits::properties::invoke_result_t<Fx, const Ty &>;
            static_assert(type_traits::primary_types::is_specialization_v<result_type, optional>, "Fx must return optional<UTy>");
            if (has_value()) {
                return utility::invoke(utility::forward<Fx>(fx), this->value_);
            }
            return result_type{};
        }

        /**
         * \lang english
         * @brief Applies a callable to the contained value and returns the resulting optional, or an empty optional if empty.
         *
         * @tparam Fx The type of the callable
         * @param fx The callable applied to the contained value
         * @return The optional returned by the callable, or an empty optional
         *
         * \lang simp-chinese
         * @brief 将可调用对象应用于被包含的值并返回结果 optional；若为空则返回空的 optional。
         *
         * @tparam Fx 可调用对象的类型
         * @param fx 应用于被包含值的可调用对象
         * @return 可调用对象返回的 optional；若为空则返回空的 optional
         */
        template <typename Fx>
        constexpr auto and_then(Fx &&fx) const && {
            using result_type = type_traits::properties::invoke_result_t<Fx, const Ty &&>;
            static_assert(type_traits::primary_types::is_specialization_v<result_type, optional>, "Fx must return optional<UTy>");
            if (has_value()) {
                return utility::invoke(utility::forward<Fx>(fx), utility::move(this->value_));
            }
            return result_type{};
        }

        /**
         * \lang english
         * @brief Applies a callable to the contained value and returns an optional of the result, or an empty optional if empty.
         *
         * @tparam Fx The type of the callable
         * @param fx The callable applied to the contained value
         * @return An optional containing the result, or an empty optional
         *
         * \lang simp-chinese
         * @brief 将可调用对象应用于被包含的值并返回包含结果的 optional；若为空则返回空的 optional。
         *
         * @tparam Fx 可调用对象的类型
         * @param fx 应用于被包含值的可调用对象
         * @return 包含结果的 optional；若为空则返回空的 optional
         */
        template <typename Fx>
        constexpr auto transform(Fx &&fx) & {
            using UTy = type_traits::properties::invoke_result_t<Fx, Ty &>;
            if (has_value()) {
                return optional<UTy>(utility::invoke(utility::forward<Fx>(fx), this->value_));
            }
            return optional<UTy>{};
        }

        /**
         * \lang english
         * @brief Applies a callable to the contained value and returns an optional of the result, or an empty optional if empty.
         *
         * @tparam Fx The type of the callable
         * @param fx The callable applied to the contained value
         * @return An optional containing the result, or an empty optional
         *
         * \lang simp-chinese
         * @brief 将可调用对象应用于被包含的值并返回包含结果的 optional；若为空则返回空的 optional。
         *
         * @tparam Fx 可调用对象的类型
         * @param fx 应用于被包含值的可调用对象
         * @return 包含结果的 optional；若为空则返回空的 optional
         */
        template <typename Fx>
        constexpr auto transform(Fx &&fx) && {
            using UTy = type_traits::properties::invoke_result_t<Fx, Ty &&>;
            if (has_value()) {
                return optional<UTy>(utility::invoke(utility::forward<Fx>(fx), utility::move(this->value_)));
            }
            return optional<UTy>{};
        }

        /**
         * \lang english
         * @brief Applies a callable to the contained value and returns an optional of the result, or an empty optional if empty.
         *
         * @tparam Fx The type of the callable
         * @param fx The callable applied to the contained value
         * @return An optional containing the result, or an empty optional
         *
         * \lang simp-chinese
         * @brief 将可调用对象应用于被包含的值并返回包含结果的 optional；若为空则返回空的 optional。
         *
         * @tparam Fx 可调用对象的类型
         * @param fx 应用于被包含值的可调用对象
         * @return 包含结果的 optional；若为空则返回空的 optional
         */
        template <typename Fx>
        constexpr auto transform(Fx &&fx) const & {
            using UTy = type_traits::properties::invoke_result_t<Fx, const Ty &>;
            if (has_value()) {
                return optional<UTy>(utility::invoke(utility::forward<Fx>(fx), this->value_));
            }
            return optional<UTy>{};
        }

        /**
         * \lang english
         * @brief Applies a callable to the contained value and returns an optional of the result, or an empty optional if empty.
         *
         * @tparam Fx The type of the callable
         * @param fx The callable applied to the contained value
         * @return An optional containing the result, or an empty optional
         *
         * \lang simp-chinese
         * @brief 将可调用对象应用于被包含的值并返回包含结果的 optional；若为空则返回空的 optional。
         *
         * @tparam Fx 可调用对象的类型
         * @param fx 应用于被包含值的可调用对象
         * @return 包含结果的 optional；若为空则返回空的 optional
         */
        template <typename Fx>
        constexpr auto transform(Fx &&fx) const && {
            using UTy = type_traits::properties::invoke_result_t<Fx, const Ty &&>;
            if (has_value()) {
                return optional<UTy>(utility::invoke(utility::forward<Fx>(fx), utility::move(this->value_)));
            }
            return optional<UTy>{};
        }

        /**
         * \lang english
         * @brief Returns this optional, or the result of the callable if this optional is empty.
         *
         * @tparam Fx The type of the callable
         * @param fx The callable invoked when the optional is empty
         * @return This optional if it has a value, otherwise the result of the callable
         *
         * \lang simp-chinese
         * @brief 返回此 optional；若此 optional 为空则返回可调用对象的结果。
         *
         * @tparam Fx 可调用对象的类型
         * @param fx 当 optional 为空时调用的可调用对象
         * @return 若此 optional 有值则返回它，否则返回可调用对象的结果
         */
        template <typename Fx>
        constexpr optional or_else(Fx &&fx) && {
            static_assert(type_traits::type_relations::is_same_v<type_traits::properties::invoke_result_t<Fx>, optional>,
                          "Fx must return optional<Ty>");
            if (has_value()) {
                return utility::move(*this);
            }
            return utility::invoke(utility::forward<Fx>(fx));
        }

        /**
         * \lang english
         * @brief Returns this optional, or the result of the callable if this optional is empty.
         *
         * @tparam Fx The type of the callable
         * @param fx The callable invoked when the optional is empty
         * @return This optional if it has a value, otherwise the result of the callable
         *
         * \lang simp-chinese
         * @brief 返回此 optional；若此 optional 为空则返回可调用对象的结果。
         *
         * @tparam Fx 可调用对象的类型
         * @param fx 当 optional 为空时调用的可调用对象
         * @return 若此 optional 有值则返回它，否则返回可调用对象的结果
         */
        template <typename Fx>
        constexpr optional or_else(Fx &&fx) const & {
            static_assert(type_traits::type_relations::is_same_v<type_traits::properties::invoke_result_t<Fx>, optional>,
                          "Fx must return optional<Ty>");
            if (has_value()) {
                return *this;
            }
            return utility::invoke(utility::forward<Fx>(fx));
        }

        /**
         * \lang english
         * @brief Returns an iterator to the contained value, or nullptr if the optional is empty.
         *
         * @return An iterator to the contained value, or nullptr
         *
         * \lang simp-chinese
         * @brief 返回指向被包含值的迭代器；若 optional 为空则返回 nullptr。
         *
         * @return 指向被包含值的迭代器；若为空则返回 nullptr
         */
        constexpr iterator begin() noexcept {
            return has_value() ? utility::addressof(this->value_) : nullptr;
        }

        /**
         * \lang english
         * @brief Returns a const iterator to the contained value, or nullptr if the optional is empty.
         *
         * @return A const iterator to the contained value, or nullptr
         *
         * \lang simp-chinese
         * @brief 返回指向被包含值的常量迭代器；若 optional 为空则返回 nullptr。
         *
         * @return 指向被包含值的常量迭代器；若为空则返回 nullptr
         */
        constexpr const_iterator begin() const noexcept {
            return has_value() ? utility::addressof(this->value_) : nullptr;
        }

        /**
         * \lang english
         * @brief Returns an iterator one past the contained value, or nullptr if the optional is empty.
         *
         * @return An iterator one past the contained value, or nullptr
         *
         * \lang simp-chinese
         * @brief 返回指向被包含值之后一个位置的迭代器；若 optional 为空则返回 nullptr。
         *
         * @return 指向被包含值之后一个位置的迭代器；若为空则返回 nullptr
         */
        constexpr iterator end() noexcept {
            return has_value() ? utility::addressof(this->value_) + 1 : nullptr;
        }

        /**
         * \lang english
         * @brief Returns a const iterator one past the contained value, or nullptr if the optional is empty.
         *
         * @return A const iterator one past the contained value, or nullptr
         *
         * \lang simp-chinese
         * @brief 返回指向被包含值之后一个位置的常量迭代器；若 optional 为空则返回 nullptr。
         *
         * @return 指向被包含值之后一个位置的常量迭代器；若为空则返回 nullptr
         */
        constexpr const_iterator end() const noexcept {
            return has_value() ? utility::addressof(this->value_) + 1 : nullptr;
        }

        /**
         * \lang english
         * @brief Returns a const iterator to the contained value, or nullptr if the optional is empty.
         *
         * @return A const iterator to the contained value, or nullptr
         *
         * \lang simp-chinese
         * @brief 返回指向被包含值的常量迭代器；若 optional 为空则返回 nullptr。
         *
         * @return 指向被包含值的常量迭代器；若为空则返回 nullptr
         */
        constexpr const_iterator cbegin() const noexcept {
            return begin();
        }

        /**
         * \lang english
         * @brief Returns a const iterator one past the contained value, or nullptr if the optional is empty.
         *
         * @return A const iterator one past the contained value, or nullptr
         *
         * \lang simp-chinese
         * @brief 返回指向被包含值之后一个位置的常量迭代器；若 optional 为空则返回 nullptr。
         *
         * @return 指向被包含值之后一个位置的常量迭代器；若为空则返回 nullptr
         */
        constexpr const_iterator cend() const noexcept {
            return end();
        }
    };

    /**
     * \lang english
     * @brief Creates an optional containing a decayed copy of the given value.
     *
     * @tparam Ty The type of the value, deduced from the argument
     * @param value The value used to initialize the contained value
     * @return An optional containing the value
     *
     * \lang simp-chinese
     * @brief 创建包含给定值（去除引用/限定符后）拷贝的 optional。
     *
     * @tparam Ty 值的类型，由实参推导
     * @param value 用于初始化被包含值的值
     * @return 包含该值的 optional
     */
    template <typename Ty>
    constexpr optional<type_traits::other_trans::decay_t<Ty>> make_optional(Ty &&value) {
        return optional<type_traits::other_trans::decay_t<Ty>>(utility::forward<Ty>(value));
    }

    /**
     * \lang english
     * @brief Creates an optional whose contained value is constructed in place from the given arguments.
     *
     * @tparam Ty The type of the contained value
     * @tparam Args The types of the arguments forwarded to the constructor of Ty
     * @param args The arguments forwarded to the constructor of Ty
     * @return An optional containing the constructed value
     *
     * \lang simp-chinese
     * @brief 创建 optional，其被包含的值使用给定实参就地构造。
     *
     * @tparam Ty 被包含值的类型
     * @tparam Args 转发给 Ty 构造函数的实参类型
     * @param args 转发给 Ty 构造函数的实参
     * @return 包含所构造值的 optional
     */
    template <typename Ty, typename... Args>
    constexpr optional<Ty> make_optional(Args &&...args) {
        return optional<Ty>(std::in_place, utility::forward<Args>(args)...);
    }

    /**
     * \lang english
     * @brief Creates an optional whose contained value is constructed in place from an initializer list and arguments.
     *
     * @tparam Ty The type of the contained value
     * @tparam UTy The type of the elements of the initializer list
     * @tparam Args The types of the remaining arguments forwarded to the constructor of Ty
     * @param il The initializer list passed to the constructor of Ty
     * @param args The remaining arguments forwarded to the constructor of Ty
     * @return An optional containing the constructed value
     *
     * \lang simp-chinese
     * @brief 创建 optional，其被包含的值使用初始化器列表和实参就地构造。
     *
     * @tparam Ty 被包含值的类型
     * @tparam UTy 初始化器列表元素的类型
     * @tparam Args 转发给 Ty 构造函数的其余实参类型
     * @param il 传递给 Ty 构造函数的初始化器列表
     * @param args 转发给 Ty 构造函数的其余实参
     * @return 包含所构造值的 optional
     */
    template <typename Ty, typename UTy, typename... Args>
    constexpr optional<Ty> make_optional(std::initializer_list<UTy> il, Args &&...args) {
        return optional<Ty>(std::in_place, il, utility::forward<Args>(args)...);
    }

    /**
     * \lang english
     * @brief Swaps the contents of two optionals.
     *
     * @tparam Ty The value type of the optionals
     * @param left The first optional to swap
     * @param right The second optional to swap
     *
     * \lang simp-chinese
     * @brief 交换两个 optional 的内容。
     *
     * @tparam Ty optional 的值类型
     * @param left 要交换的第一个 optional
     * @param right 要交换的第二个 optional
     */
    template <typename Ty>
    RAINY_CONSTEXPR20 void swap(optional<Ty> &left,
                                optional<Ty> &right) noexcept(type_traits::properties::is_nothrow_swappable_v<Ty>) {
        left.swap(right);
    }
}

namespace rainy::core::container {
    /**
     * \lang english
     * @brief Compares two optionals for equality.
     *
     * @tparam Ty The value type of the left optional
     * @tparam UTy The value type of the right optional
     * @param left The left optional
     * @param right The right optional
     * @return true if both are empty, or both contain equal values; false otherwise
     *
     * \lang simp-chinese
     * @brief 比较两个 optional 是否相等。
     *
     * @tparam Ty 左侧 optional 的值类型
     * @tparam UTy 右侧 optional 的值类型
     * @param left 左侧 optional
     * @param right 右侧 optional
     * @return 若两者都为空，或两者都包含相等的值则返回 true，否则返回 false
     */
    template <typename Ty, typename UTy>
    constexpr bool operator==(const optional<Ty> &left, const optional<UTy> &right) {
        if (left.has_value() != right.has_value()) {
            return false;
        }
        if (!left.has_value()) {
            return true;
        }
        return *left == *right;
    }

#if !RAINY_HAS_CXX20
    /**
     * \lang english
     * @brief Compares two optionals for inequality.
     *
     * @tparam Ty The value type of the left optional
     * @tparam UTy The value type of the right optional
     * @param left The left optional
     * @param right The right optional
     * @return true if the optionals are not equal, false otherwise
     *
     * \lang simp-chinese
     * @brief 比较两个 optional 是否不相等。
     *
     * @tparam Ty 左侧 optional 的值类型
     * @tparam UTy 右侧 optional 的值类型
     * @param left 左侧 optional
     * @param right 右侧 optional
     * @return 若两个 optional 不相等则返回 true，否则返回 false
     */
    template <typename Ty, typename UTy>
    constexpr bool operator!=(const optional<Ty> &left, const optional<UTy> &right) {
        return !(left == right);
    }
#endif

    /**
     * \lang english
     * @brief Compares two optionals lexicographically with operator<.
     *
     * @tparam Ty The value type of the left optional
     * @tparam UTy The value type of the right optional
     * @param left The left optional
     * @param right The right optional
     * @return true if left compares less than right, false otherwise
     *
     * \lang simp-chinese
     * @brief 按字典序用 operator< 比较两个 optional。
     *
     * @tparam Ty 左侧 optional 的值类型
     * @tparam UTy 右侧 optional 的值类型
     * @param left 左侧 optional
     * @param right 右侧 optional
     * @return 若 left 小于 right 则返回 true，否则返回 false
     */
    template <typename Ty, typename UTy>
    constexpr bool operator<(const optional<Ty> &left, const optional<UTy> &right) {
        if (!right.has_value()) {
            return false;
        }
        if (!left.has_value()) {
            return true;
        }
        return *left < *right;
    }

#if !RAINY_HAS_CXX20
    /**
     * \lang english
     * @brief Compares two optionals lexicographically with operator>.
     *
     * @tparam Ty The value type of the left optional
     * @tparam UTy The value type of the right optional
     * @param left The left optional
     * @param right The right optional
     * @return true if left compares greater than right, false otherwise
     *
     * \lang simp-chinese
     * @brief 按字典序用 operator> 比较两个 optional。
     *
     * @tparam Ty 左侧 optional 的值类型
     * @tparam UTy 右侧 optional 的值类型
     * @param left 左侧 optional
     * @param right 右侧 optional
     * @return 若 left 大于 right 则返回 true，否则返回 false
     */
    template <typename Ty, typename UTy>
    constexpr bool operator>(const optional<Ty> &left, const optional<UTy> &right) {
        return right < left;
    }

    /**
     * \lang english
     * @brief Compares two optionals lexicographically with operator<=.
     *
     * @tparam Ty The value type of the left optional
     * @tparam UTy The value type of the right optional
     * @param left The left optional
     * @param right The right optional
     * @return true if left compares less than or equal to right, false otherwise
     *
     * \lang simp-chinese
     * @brief 按字典序用 operator<= 比较两个 optional。
     *
     * @tparam Ty 左侧 optional 的值类型
     * @tparam UTy 右侧 optional 的值类型
     * @param left 左侧 optional
     * @param right 右侧 optional
     * @return 若 left 小于等于 right 则返回 true，否则返回 false
     */
    template <typename Ty, typename UTy>
    constexpr bool operator<=(const optional<Ty> &left, const optional<UTy> &right) {
        return !(right < left);
    }

    /**
     * \lang english
     * @brief Compares two optionals lexicographically with operator>=.
     *
     * @tparam Ty The value type of the left optional
     * @tparam UTy The value type of the right optional
     * @param left The left optional
     * @param right The right optional
     * @return true if left compares greater than or equal to right, false otherwise
     *
     * \lang simp-chinese
     * @brief 按字典序用 operator>= 比较两个 optional。
     *
     * @tparam Ty 左侧 optional 的值类型
     * @tparam UTy 右侧 optional 的值类型
     * @param left 左侧 optional
     * @param right 右侧 optional
     * @return 若 left 大于等于 right 则返回 true，否则返回 false
     */
    template <typename Ty, typename UTy>
    constexpr bool operator>=(const optional<Ty> &left, const optional<UTy> &right) {
        return !(left < right);
    }
#endif

#if RAINY_HAS_CXX20
    /**
     * \lang english
     * @brief Compares two optionals with the three-way comparison operator.
     *
     * @tparam Ty The value type of the left optional
     * @tparam UTy The value type of the right optional
     * @param left The left optional
     * @param right The right optional
     * @return The three-way comparison result of the two optionals
     *
     * \lang simp-chinese
     * @brief 用三路比较运算符比较两个 optional。
     *
     * @tparam Ty 左侧 optional 的值类型
     * @tparam UTy 右侧 optional 的值类型
     * @param left 左侧 optional
     * @param right 右侧 optional
     * @return 两个 optional 的三路比较结果
     */
    template <typename Ty, typename UTy>
        requires std::three_way_comparable_with<Ty, UTy>
    constexpr std::compare_three_way_result_t<Ty, UTy> operator<=>(const optional<Ty> &left, const optional<UTy> &right) {
        if (left.has_value() && right.has_value()) {
            return *left <=> *right;
        }
        return left.has_value() <=> right.has_value();
    }
#endif

    /**
     * \lang english
     * @brief Compares an optional with nullopt for equality.
     *
     * @tparam Ty The value type of the optional
     * @param left The optional
     * @return true if the optional is empty, false otherwise
     *
     * \lang simp-chinese
     * @brief 比较 optional 与 nullopt 是否相等。
     *
     * @tparam Ty optional 的值类型
     * @param left optional
     * @return 若 optional 为空则返回 true，否则返回 false
     */
    template <typename Ty>
    constexpr bool operator==(const optional<Ty> &left, nullopt_t) noexcept {
        return !left.has_value();
    }

#if RAINY_HAS_CXX20
    /**
     * \lang english
     * @brief Compares an optional with nullopt using the three-way comparison operator.
     *
     * @tparam Ty The value type of the optional
     * @param left The optional
     * @return The strong ordering result of the optional against nullopt
     *
     * \lang simp-chinese
     * @brief 用三路比较运算符比较 optional 与 nullopt。
     *
     * @tparam Ty optional 的值类型
     * @param left optional
     * @return optional 与 nullopt 的比较结果
     */
    template <typename Ty>
    constexpr std::strong_ordering operator<=>(const optional<Ty> &left, nullopt_t) noexcept {
        return left.has_value() <=> false;
    }
#else
    /**
     * \lang english
     * @brief Compares nullopt with an optional for equality.
     *
     * @tparam Ty The value type of the optional
     * @param right The optional
     * @return true if the optional is empty, false otherwise
     *
     * \lang simp-chinese
     * @brief 比较 nullopt 与 optional 是否相等。
     *
     * @tparam Ty optional 的值类型
     * @param right optional
     * @return 若 optional 为空则返回 true，否则返回 false
     */
    template <typename Ty>
    constexpr bool operator==(nullopt_t, const optional<Ty> &right) noexcept {
        return !right.has_value();
    }

    /**
     * \lang english
     * @brief Compares an optional with nullopt for inequality.
     *
     * @tparam Ty The value type of the optional
     * @param left The optional
     * @return true if the optional has a value, false otherwise
     *
     * \lang simp-chinese
     * @brief 比较 optional 与 nullopt 是否不相等。
     *
     * @tparam Ty optional 的值类型
     * @param left optional
     * @return 若 optional 有值则返回 true，否则返回 false
     */
    template <typename Ty>
    constexpr bool operator!=(const optional<Ty> &left, nullopt_t) noexcept {
        return left.has_value();
    }

    /**
     * \lang english
     * @brief Compares nullopt with an optional for inequality.
     *
     * @tparam Ty The value type of the optional
     * @param right The optional
     * @return true if the optional has a value, false otherwise
     *
     * \lang simp-chinese
     * @brief 比较 nullopt 与 optional 是否不相等。
     *
     * @tparam Ty optional 的值类型
     * @param right optional
     * @return 若 optional 有值则返回 true，否则返回 false
     */
    template <typename Ty>
    constexpr bool operator!=(nullopt_t, const optional<Ty> &right) noexcept {
        return right.has_value();
    }

    /**
     * \lang english
     * @brief Compares an optional with nullopt using operator<.
     *
     * @tparam Ty The value type of the optional
     * @return false always
     *
     * \lang simp-chinese
     * @brief 用 operator< 比较 optional 与 nullopt。
     *
     * @tparam Ty optional 的值类型
     * @return 始终返回 false
     */
    template <typename Ty>
    constexpr bool operator<(const optional<Ty> &, nullopt_t) noexcept {
        return false;
    }

    /**
     * \lang english
     * @brief Compares nullopt with an optional using operator<.
     *
     * @tparam Ty The value type of the optional
     * @param right The optional
     * @return true if the optional has a value, false otherwise
     *
     * \lang simp-chinese
     * @brief 用 operator< 比较 nullopt 与 optional。
     *
     * @tparam Ty optional 的值类型
     * @param right optional
     * @return 若 optional 有值则返回 true，否则返回 false
     */
    template <typename Ty>
    constexpr bool operator<(nullopt_t, const optional<Ty> &right) noexcept {
        return right.has_value();
    }

    /**
     * \lang english
     * @brief Compares an optional with nullopt using operator>.
     *
     * @tparam Ty The value type of the optional
     * @param left The optional
     * @return true if the optional has a value, false otherwise
     *
     * \lang simp-chinese
     * @brief 用 operator> 比较 optional 与 nullopt。
     *
     * @tparam Ty optional 的值类型
     * @param left optional
     * @return 若 optional 有值则返回 true，否则返回 false
     */
    template <typename Ty>
    constexpr bool operator>(const optional<Ty> &left, nullopt_t) noexcept {
        return left.has_value();
    }

    /**
     * \lang english
     * @brief Compares nullopt with an optional using operator>.
     *
     * @tparam Ty The value type of the optional
     * @return false always
     *
     * \lang simp-chinese
     * @brief 用 operator> 比较 nullopt 与 optional。
     *
     * @tparam Ty optional 的值类型
     * @return 始终返回 false
     */
    template <typename Ty>
    constexpr bool operator>(nullopt_t, const optional<Ty> &) noexcept {
        return false;
    }

    /**
     * \lang english
     * @brief Compares an optional with nullopt using operator<=.
     *
     * @tparam Ty The value type of the optional
     * @param left The optional
     * @return true if the optional is empty, false otherwise
     *
     * \lang simp-chinese
     * @brief 用 operator<= 比较 optional 与 nullopt。
     *
     * @tparam Ty optional 的值类型
     * @param left optional
     * @return 若 optional 为空则返回 true，否则返回 false
     */
    template <typename Ty>
    constexpr bool operator<=(const optional<Ty> &left, nullopt_t) noexcept {
        return !left.has_value();
    }

    /**
     * \lang english
     * @brief Compares nullopt with an optional using operator<=.
     *
     * @tparam Ty The value type of the optional
     * @return true always
     *
     * \lang simp-chinese
     * @brief 用 operator<= 比较 nullopt 与 optional。
     *
     * @tparam Ty optional 的值类型
     * @return 始终返回 true
     */
    template <typename Ty>
    constexpr bool operator<=(nullopt_t, const optional<Ty> &) noexcept {
        return true;
    }

    /**
     * \lang english
     * @brief Compares an optional with nullopt using operator>=.
     *
     * @tparam Ty The value type of the optional
     * @return true always
     *
     * \lang simp-chinese
     * @brief 用 operator>= 比较 optional 与 nullopt。
     *
     * @tparam Ty optional 的值类型
     * @return 始终返回 true
     */
    template <typename Ty>
    constexpr bool operator>=(const optional<Ty> &, nullopt_t) noexcept {
        return true;
    }

    /**
     * \lang english
     * @brief Compares nullopt with an optional using operator>=.
     *
     * @tparam Ty The value type of the optional
     * @param right The optional
     * @return true if the optional is empty, false otherwise
     *
     * \lang simp-chinese
     * @brief 用 operator>= 比较 nullopt 与 optional。
     *
     * @tparam Ty optional 的值类型
     * @param right optional
     * @return 若 optional 为空则返回 true，否则返回 false
     */
    template <typename Ty>
    constexpr bool operator>=(nullopt_t, const optional<Ty> &right) noexcept {
        return !right.has_value();
    }
#endif

    /**
     * \lang english
     * @brief Compares an optional with a value for equality.
     *
     * @tparam Ty The value type of the optional
     * @tparam UTy The type of the value
     * @param left The optional
     * @param right The value
     * @return true if the optional has a value equal to the given value, false otherwise
     *
     * \lang simp-chinese
     * @brief 比较 optional 与值是否相等。
     *
     * @tparam Ty optional 的值类型
     * @tparam UTy 值的类型
     * @param left optional
     * @param right 值
     * @return 若 optional 包含的值等于给定值则返回 true，否则返回 false
     */
    template <typename Ty, typename UTy>
    constexpr bool operator==(const optional<Ty> &left, const UTy &right) {
        return left.has_value() ? *left == right : false;
    }

    /**
     * \lang english
     * @brief Compares a value with an optional for equality.
     *
     * @tparam Ty The type of the value
     * @tparam UTy The value type of the optional
     * @param left The value
     * @param right The optional
     * @return true if the optional has a value equal to the given value, false otherwise
     *
     * \lang simp-chinese
     * @brief 比较值与 optional 是否相等。
     *
     * @tparam Ty 值的类型
     * @tparam UTy optional 的值类型
     * @param left 值
     * @param right optional
     * @return 若 optional 包含的值等于给定值则返回 true，否则返回 false
     */
    template <typename Ty, typename UTy>
    constexpr bool operator==(const Ty &left, const optional<UTy> &right) {
        return right.has_value() ? left == *right : false;
    }

#if !RAINY_HAS_CXX20
    /**
     * \lang english
     * @brief Compares an optional with a value for inequality.
     *
     * @tparam Ty The value type of the optional
     * @tparam UTy The type of the value
     * @param left The optional
     * @param right The value
     * @return true if the optional is empty or its value differs from the given value
     *
     * \lang simp-chinese
     * @brief 比较 optional 与值是否不相等。
     *
     * @tparam Ty optional 的值类型
     * @tparam UTy 值的类型
     * @param left optional
     * @param right 值
     * @return 若 optional 为空或其值不同于给定值则返回 true
     */
    template <typename Ty, typename UTy>
    constexpr bool operator!=(const optional<Ty> &left, const UTy &right) {
        return left.has_value() ? *left != right : true;
    }

    /**
     * \lang english
     * @brief Compares a value with an optional for inequality.
     *
     * @tparam Ty The type of the value
     * @tparam UTy The value type of the optional
     * @param left The value
     * @param right The optional
     * @return true if the optional is empty or its value differs from the given value
     *
     * \lang simp-chinese
     * @brief 比较值与 optional 是否不相等。
     *
     * @tparam Ty 值的类型
     * @tparam UTy optional 的值类型
     * @param left 值
     * @param right optional
     * @return 若 optional 为空或其值不同于给定值则返回 true
     */
    template <typename Ty, typename UTy>
    constexpr bool operator!=(const Ty &left, const optional<UTy> &right) {
        return right.has_value() ? left != *right : true;
    }
#endif

    /**
     * \lang english
     * @brief Compares an optional with a value using operator<.
     *
     * @tparam Ty The value type of the optional
     * @tparam UTy The type of the value
     * @param left The optional
     * @param right The value
     * @return true if the optional is empty or its value is less than the given value
     *
     * \lang simp-chinese
     * @brief 用 operator< 比较 optional 与值。
     *
     * @tparam Ty optional 的值类型
     * @tparam UTy 值的类型
     * @param left optional
     * @param right 值
     * @return 若 optional 为空或其值小于给定值则返回 true
     */
    template <typename Ty, typename UTy>
    constexpr bool operator<(const optional<Ty> &left, const UTy &right) {
        return left.has_value() ? *left < right : true;
    }

    /**
     * \lang english
     * @brief Compares a value with an optional using operator<.
     *
     * @tparam Ty The type of the value
     * @tparam UTy The value type of the optional
     * @param left The value
     * @param right The optional
     * @return true if the optional has a value greater than the given value
     *
     * \lang simp-chinese
     * @brief 用 operator< 比较值与 optional。
     *
     * @tparam Ty 值的类型
     * @tparam UTy optional 的值类型
     * @param left 值
     * @param right optional
     * @return 若 optional 包含的值大于给定值则返回 true
     */
    template <typename Ty, typename UTy>
    constexpr bool operator<(const Ty &left, const optional<UTy> &right) {
        return right.has_value() ? left < *right : false;
    }

#if !RAINY_HAS_CXX20
    /**
     * \lang english
     * @brief Compares an optional with a value using operator>.
     *
     * @tparam Ty The value type of the optional
     * @tparam UTy The type of the value
     * @param left The optional
     * @param right The value
     * @return true if the optional has a value greater than the given value
     *
     * \lang simp-chinese
     * @brief 用 operator> 比较 optional 与值。
     *
     * @tparam Ty optional 的值类型
     * @tparam UTy 值的类型
     * @param left optional
     * @param right 值
     * @return 若 optional 包含的值大于给定值则返回 true
     */
    template <typename Ty, typename UTy>
    constexpr bool operator>(const optional<Ty> &left, const UTy &right) {
        return left.has_value() ? *left > right : false;
    }

    /**
     * \lang english
     * @brief Compares a value with an optional using operator>.
     *
     * @tparam Ty The type of the value
     * @tparam UTy The value type of the optional
     * @param left The value
     * @param right The optional
     * @return true if the optional is empty or its value is less than the given value
     *
     * \lang simp-chinese
     * @brief 用 operator> 比较值与 optional。
     *
     * @tparam Ty 值的类型
     * @tparam UTy optional 的值类型
     * @param left 值
     * @param right optional
     * @return 若 optional 为空或其值小于给定值则返回 true
     */
    template <typename Ty, typename UTy>
    constexpr bool operator>(const Ty &left, const optional<UTy> &right) {
        return right.has_value() ? left > *right : true;
    }

    /**
     * \lang english
     * @brief Compares an optional with a value using operator<=.
     *
     * @tparam Ty The value type of the optional
     * @tparam UTy The type of the value
     * @param left The optional
     * @param right The value
     * @return true if the optional is empty or its value is less than or equal to the given value
     *
     * \lang simp-chinese
     * @brief 用 operator<= 比较 optional 与值。
     *
     * @tparam Ty optional 的值类型
     * @tparam UTy 值的类型
     * @param left optional
     * @param right 值
     * @return 若 optional 为空或其值小于等于给定值则返回 true
     */
    template <typename Ty, typename UTy>
    constexpr bool operator<=(const optional<Ty> &left, const UTy &right) {
        return left.has_value() ? *left <= right : true;
    }

    /**
     * \lang english
     * @brief Compares a value with an optional using operator<=.
     *
     * @tparam Ty The type of the value
     * @tparam UTy The value type of the optional
     * @param left The value
     * @param right The optional
     * @return true if the optional has a value greater than or equal to the given value
     *
     * \lang simp-chinese
     * @brief 用 operator<= 比较值与 optional。
     *
     * @tparam Ty 值的类型
     * @tparam UTy optional 的值类型
     * @param left 值
     * @param right optional
     * @return 若 optional 包含的值大于等于给定值则返回 true
     */
    template <typename Ty, typename UTy>
    constexpr bool operator<=(const Ty &left, const optional<UTy> &right) {
        return right.has_value() ? left <= *right : false;
    }

    /**
     * \lang english
     * @brief Compares an optional with a value using operator>=.
     *
     * @tparam Ty The value type of the optional
     * @tparam UTy The type of the value
     * @param left The optional
     * @param right The value
     * @return true if the optional has a value greater than or equal to the given value
     *
     * \lang simp-chinese
     * @brief 用 operator>= 比较 optional 与值。
     *
     * @tparam Ty optional 的值类型
     * @tparam UTy 值的类型
     * @param left optional
     * @param right 值
     * @return 若 optional 包含的值大于等于给定值则返回 true
     */
    template <typename Ty, typename UTy>
    constexpr bool operator>=(const optional<Ty> &left, const UTy &right) {
        return left.has_value() ? *left >= right : false;
    }

    /**
     * \lang english
     * @brief Compares a value with an optional using operator>=.
     *
     * @tparam Ty The type of the value
     * @tparam UTy The value type of the optional
     * @param left The value
     * @param right The optional
     * @return true if the optional is empty or its value is less than or equal to the given value
     *
     * \lang simp-chinese
     * @brief 用 operator>= 比较值与 optional。
     *
     * @tparam Ty 值的类型
     * @tparam UTy optional 的值类型
     * @param left 值
     * @param right optional
     * @return 若 optional 为空或其值小于等于给定值则返回 true
     */
    template <typename Ty, typename UTy>
    constexpr bool operator>=(const Ty &left, const optional<UTy> &right) {
        return right.has_value() ? left >= *right : true;
    }
#endif

#if RAINY_HAS_CXX20
    /**
     * \lang english
     * @brief Compares an optional with a value using the three-way comparison operator.
     *
     * @tparam Ty The value type of the optional
     * @tparam UTy The type of the value
     * @param left The optional
     * @param right The value
     * @return The three-way comparison result, or less if the optional is empty
     *
     * \lang simp-chinese
     * @brief 用三路比较运算符比较 optional 与值。
     *
     * @tparam Ty optional 的值类型
     * @tparam UTy 值的类型
     * @param left optional
     * @param right 值
     * @return 三路比较结果；若 optional 为空则为 less
     */
    template <typename Ty, typename UTy>
        requires(!implements::is_derived_from_optional<UTy>) && std::three_way_comparable_with<Ty, UTy>
    constexpr std::compare_three_way_result_t<Ty, UTy> operator<=>(const optional<Ty> &left, const UTy &right) {
        return left.has_value() ? *left <=> right : std::strong_ordering::less;
    }
#endif
}

#if RAINY_USING_MSVC
#pragma warning(pop)
#endif

namespace rainy::container {
    using rainy::core::container::optional;
    using rainy::core::container::nullopt;
    using rainy::core::container::nullopt_t;
    using rainy::core::container::make_optional;
    using rainy::core::container::swap;
    using rainy::core::container::operator==;
    using rainy::core::container::operator<;
#if !RAINY_HAS_CXX20
    using rainy::core::container::operator!=;
    using rainy::core::container::operator>;
    using rainy::core::container::operator<=;
    using rainy::core::container::operator>=;
#else
    using rainy::core::container::operator<=>;
#endif
}


#endif
