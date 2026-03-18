/*
 * Copyright 2025 rainy-juzixiao
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
#ifndef RAINY_CORE_TYPE_TRAITS_META_METHOD_HPP
#define RAINY_CORE_TYPE_TRAITS_META_METHOD_HPP
#include <rainy/core/platform.hpp>
#include <rainy/core/type_traits/implements.hpp>
#include <rainy/core/type_traits/templates.hpp>

namespace rainy::type_traits::extras::meta_method {
    /**
     * @brief Primary template for attempting to invoke begin() on a type.
     *        尝试在类型上调用 begin() 的主模板。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename Ty, typename = void>
    struct try_to_invoke_begin {
        static RAINY_CONSTEXPR_BOOL value = false;

        /**
         * @brief Fallback invoke that generates a compile-time error.
         *        生成编译时错误的回退调用。
         *
         * @tparam Uty The type that was attempted
         *             被尝试的类型
         */
        template <typename Uty>
        static void invoke(const Uty &) {
            static_assert(rainy::type_traits::implements::always_false<Uty>, "Can not find begin method! "
                                                                             "rainy::utility::begin "
                                                                             "only support begin() in Container Type "
                                                                             "please add begin() method in Container Definition");
        }
    };

    /**
     * @brief Specialization for types that have a begin() member function.
     *        具有 begin() 成员函数的类型的特化。
     *
     * @tparam Ty The type that provides begin()
     *            提供 begin() 的类型
     */
    template <typename Ty>
    struct try_to_invoke_begin<Ty, type_traits::other_trans::void_t<decltype(utility::declval<Ty>().begin())>> {
        static RAINY_CONSTEXPR_BOOL value = true;

        /**
         * @brief Invokes begin() on the container.
         *        在容器上调用 begin()。
         *
         * @param container The container object
         *                  容器对象
         * @return The result of container.begin()
         *         container.begin() 的结果
         */
        static auto invoke(Ty &container) noexcept(noexcept(container.begin())) -> decltype(container.begin()) {
            return container.begin();
        }
    };

    /**
     * @brief Primary template for attempting to invoke end() on a type.
     *        尝试在类型上调用 end() 的主模板。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename, typename = void>
    struct try_to_invoke_end {
        static RAINY_CONSTEXPR_BOOL value = false;

        /**
         * @brief Fallback invoke that generates a compile-time error.
         *        生成编译时错误的回退调用。
         *
         * @tparam Uty The type that was attempted
         *             被尝试的类型
         */
        template <typename Uty>
        static void invoke(const Uty &) {
            static_assert(rainy::type_traits::implements::always_false<Uty>, "Can not find end method! "
                                                                             "rainy::utility::end "
                                                                             "only support end() in Container Type "
                                                                             "please add end() method in Container Definition");
        }
    };

    /**
     * @brief Specialization for types that have an end() member function.
     *        具有 end() 成员函数的类型的特化。
     *
     * @tparam Ty The type that provides end()
     *            提供 end() 的类型
     */
    template <typename Ty>
    struct try_to_invoke_end<Ty, type_traits::other_trans::void_t<decltype(utility::declval<Ty>().end())>> {
        static RAINY_CONSTEXPR_BOOL value = true;

        /**
         * @brief Invokes end() on the container.
         *        在容器上调用 end()。
         *
         * @param container The container object
         *                  容器对象
         * @return The result of container.end()
         *         container.end() 的结果
         */
        static auto invoke(Ty &container) noexcept(noexcept(container.end())) -> decltype(container.end()) {
            return container.end();
        }
    };

    /**
     * @brief Primary template for attempting to invoke cbegin() on a type.
     *        尝试在类型上调用 cbegin() 的主模板。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename Ty, typename = void>
    struct try_to_invoke_cbegin {
        static RAINY_CONSTEXPR_BOOL value = false;

        /**
         * @brief Fallback invoke that generates a compile-time error.
         *        生成编译时错误的回退调用。
         *
         * @tparam Uty The type that was attempted
         *             被尝试的类型
         */
        template <typename Uty>
        static void invoke(const Uty &) {
            static_assert(rainy::type_traits::implements::always_false<Uty>, "Can not find begin method! "
                                                                             "rainy::utility::cbegin "
                                                                             "only support cbegin() in Container Type "
                                                                             "please add cbegin() method in Container Definition");
        }
    };

    /**
     * @brief Specialization for types that have a cbegin() member function.
     *        具有 cbegin() 成员函数的类型的特化。
     *
     * @tparam Ty The type that provides cbegin()
     *            提供 cbegin() 的类型
     */
    template <typename Ty>
    struct try_to_invoke_cbegin<Ty, type_traits::other_trans::void_t<decltype(utility::declval<Ty>().cbegin())>> {
        static RAINY_CONSTEXPR_BOOL value = true;

        /**
         * @brief Invokes cbegin() on the const container.
         *        在常量容器上调用 cbegin()。
         *
         * @param container The const container object
         *                  常量容器对象
         * @return The result of container.cbegin()
         *         container.cbegin() 的结果
         */
        static auto invoke(const Ty &container) noexcept(noexcept(container.cbegin())) -> decltype(container.cbegin()) {
            return container.cbegin();
        }
    };

    /**
     * @brief Primary template for attempting to invoke cend() on a type.
     *        尝试在类型上调用 cend() 的主模板。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename, typename = void>
    struct try_to_invoke_cend {
        static RAINY_CONSTEXPR_BOOL value = false;

        /**
         * @brief Fallback invoke that generates a compile-time error.
         *        生成编译时错误的回退调用。
         *
         * @tparam Uty The type that was attempted
         *             被尝试的类型
         */
        template <typename Uty>
        static void invoke(const Uty &) {
            static_assert(rainy::type_traits::implements::always_false<Uty>, "Can not find end method! "
                                                                             "rainy::utility::cend "
                                                                             "only support cend() in Container Type "
                                                                             "please add cend() method in Container Definition");
        }
    };

    /**
     * @brief Specialization for types that have a cend() member function.
     *        具有 cend() 成员函数的类型的特化。
     *
     * @tparam Ty The type that provides cend()
     *            提供 cend() 的类型
     */
    template <typename Ty>
    struct try_to_invoke_cend<Ty, type_traits::other_trans::void_t<decltype(utility::declval<Ty>().cend())>> {
        static RAINY_CONSTEXPR_BOOL value = true;

        /**
         * @brief Invokes cend() on the const container.
         *        在常量容器上调用 cend()。
         *
         * @param container The const container object
         *                  常量容器对象
         * @return The result of container.cend()
         *         container.cend() 的结果
         */
        static auto invoke(const Ty &container) noexcept(noexcept(container.cend())) -> decltype(container.cend()) {
            return container.cend();
        }
    };

    /**
     * @brief Primary template for attempting to invoke size() on a type.
     *        尝试在类型上调用 size() 的主模板。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename Ty, typename = void>
    struct try_to_invoke_size {
        static RAINY_CONSTEXPR_BOOL value = false;

        /**
         * @brief Fallback invoke that generates a compile-time error.
         *        生成编译时错误的回退调用。
         *
         * @tparam Uty The type that was attempted
         *             被尝试的类型
         */
        template <typename Uty>
        static void invoke(const Uty &) {
            static_assert(rainy::type_traits::implements::always_false<Uty>, "Can not find begin method! "
                                                                             "rainy::utility::begin "
                                                                             "only support begin() in Container Type "
                                                                             "please add begin() method in Container Definition");
        }
    };

    /**
     * @brief Specialization for types that have a size() member function.
     *        具有 size() 成员函数的类型的特化。
     *
     * @tparam Ty The type that provides size()
     *            提供 size() 的类型
     */
    template <typename Ty>
    struct try_to_invoke_size<Ty, type_traits::other_trans::void_t<decltype(utility::declval<const Ty &>().size())>> {
        static RAINY_CONSTEXPR_BOOL value = true;

        /**
         * @brief Invokes size() on the const container.
         *        在常量容器上调用 size()。
         *
         * @param container The const container object
         *                  常量容器对象
         * @return The result of container.size()
         *         container.size() 的结果
         */
        static auto invoke(const Ty &container) noexcept(noexcept(container.size())) -> decltype(container.size()) {
            return container.size();
        }
    };
}

namespace rainy::utility {
    /**
     * @brief Returns an iterator to the beginning of a container.
     *        返回指向容器起始的迭代器。
     *
     * @tparam Container The container type
     *                   容器类型
     * @param cont The container object
     *             容器对象
     * @return Iterator to the first element
     *         指向第一个元素的迭代器
     */
    template <typename Container>
    RAINY_NODISCARD constexpr rain_fn begin(Container &cont) noexcept(
        noexcept(type_traits::extras::meta_method::try_to_invoke_begin<Container>::invoke(cont)))
        -> decltype(type_traits::extras::meta_method::try_to_invoke_begin<Container>::invoke(cont)) {
        return type_traits::extras::meta_method::try_to_invoke_begin<Container>::invoke(cont);
    }

    /**
     * @brief Returns a const iterator to the beginning of a const container.
     *        返回指向常量容器起始的常量迭代器。
     *
     * @tparam Container The container type
     *                   容器类型
     * @param cont The const container object
     *             常量容器对象
     * @return Const iterator to the first element
     *         指向第一个元素的常量迭代器
     */
    template <typename Container>
    RAINY_NODISCARD constexpr rain_fn begin(const Container &cont) noexcept(
        noexcept(type_traits::extras::meta_method::try_to_invoke_begin<const Container>::invoke(cont)))
        -> decltype(type_traits::extras::meta_method::try_to_invoke_begin<const Container>::invoke(cont)) {
        return type_traits::extras::meta_method::try_to_invoke_begin<const Container>::invoke(cont);
    }

    /**
     * @brief Returns an iterator to the beginning of a C-style array.
     *        返回指向C风格数组起始的迭代器。
     *
     * @tparam Container The array element type
     *                   数组元素类型
     * @tparam N Array size
     *           数组大小
     * @param container The C-style array
     *                  C风格数组
     * @return Pointer to the first element
     *         指向第一个元素的指针
     */
    template <typename Container, std::size_t N>
    static rain_fn begin(Container (&container)[N]) noexcept -> auto {
        return container;
    }

    /**
     * @brief Returns an iterator to the end of a container.
     *        返回指向容器末尾的迭代器。
     *
     * @tparam Container The container type
     *                   容器类型
     * @param cont The container object
     *             容器对象
     * @return Iterator to one past the last element
     *         指向最后一个元素之后位置的迭代器
     */
    template <typename Container>
    RAINY_NODISCARD constexpr rain_fn end(Container &cont) noexcept(
        noexcept(type_traits::extras::meta_method::try_to_invoke_end<Container>::invoke(cont)))
        -> decltype(type_traits::extras::meta_method::try_to_invoke_end<Container>::invoke(cont)) {
        return type_traits::extras::meta_method::try_to_invoke_end<Container>::invoke(cont);
    }

    /**
     * @brief Returns a const iterator to the end of a const container.
     *        返回指向常量容器末尾的常量迭代器。
     *
     * @tparam Container The container type
     *                   容器类型
     * @param cont The const container object
     *             常量容器对象
     * @return Const iterator to one past the last element
     *         指向最后一个元素之后位置的常量迭代器
     */
    template <typename Container>
    RAINY_NODISCARD constexpr rain_fn end(const Container &cont) noexcept(
        noexcept(type_traits::extras::meta_method::try_to_invoke_end<const Container>::invoke(cont)))
        -> decltype(type_traits::extras::meta_method::try_to_invoke_end<const Container>::invoke(cont)) {
        return type_traits::extras::meta_method::try_to_invoke_end<const Container>::invoke(cont);
    }

    /**
     * @brief Returns an iterator to the end of a C-style array.
     *        返回指向C风格数组末尾的迭代器。
     *
     * @tparam Container The array element type
     *                   数组元素类型
     * @tparam N Array size
     *           数组大小
     * @param container The C-style array
     *                  C风格数组
     * @return Pointer to one past the last element
     *         指向最后一个元素之后位置的指针
     */
    template <typename Container, std::size_t N>
    static rain_fn end(Container (&container)[N]) noexcept -> auto {
        return container + N;
    }

    /**
     * @brief Returns a const iterator to the beginning of a container.
     *        返回指向容器起始的常量迭代器。
     *
     * @tparam Container The container type
     *                   容器类型
     * @param cont The const container object
     *             常量容器对象
     * @return Const iterator to the first element
     *         指向第一个元素的常量迭代器
     */
    template <typename Container>
    RAINY_NODISCARD constexpr rain_fn cbegin(const Container &cont) noexcept(
        noexcept(type_traits::extras::meta_method::try_to_invoke_cbegin<const Container>::invoke(cont)))
        -> decltype(type_traits::extras::meta_method::try_to_invoke_cbegin<const Container>::invoke(cont)) {
        return type_traits::extras::meta_method::try_to_invoke_cbegin<const Container>::invoke(cont);
    }

    /**
     * @brief Returns a const iterator to the end of a container.
     *        返回指向容器末尾的常量迭代器。
     *
     * @tparam Container The container type
     *                   容器类型
     * @param cont The const container object
     *             常量容器对象
     * @return Const iterator to one past the last element
     *         指向最后一个元素之后位置的常量迭代器
     */
    template <typename Container>
    RAINY_NODISCARD constexpr rain_fn cend(const Container &cont) noexcept(
        noexcept(type_traits::extras::meta_method::try_to_invoke_cbegin<const Container>::invoke(cont)))
        -> decltype(type_traits::extras::meta_method::try_to_invoke_cbegin<const Container>::invoke(cont)) {
        return type_traits::extras::meta_method::try_to_invoke_cbegin<const Container>::invoke(cont);
    }

    /**
     * @brief Returns the size of a container.
     *        返回容器的大小。
     *
     * @tparam Container The container type
     *                   容器类型
     * @param cont The const container object
     *             常量容器对象
     * @return The number of elements in the container
     *         容器中的元素数量
     */
    template <typename Container>
    RAINY_NODISCARD constexpr rain_fn size(const Container &cont) noexcept(
        noexcept(type_traits::extras::meta_method::try_to_invoke_size<const Container>::invoke(cont)))
        -> decltype(type_traits::extras::meta_method::try_to_invoke_size<const Container>::invoke(cont)) {
        return type_traits::extras::meta_method::try_to_invoke_size<const Container>::invoke(cont);
    }
}

/* 元方法Trait */
namespace rainy::type_traits::extras::meta_method {
    /**
     * @brief Variable template for checking if a type satisfies iterator requirements.
     *        检查类型是否满足迭代器要求的变量模板。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL has_iterator_v = false;

    /**
     * @brief Specialization that detects iterator operations (begin/end comparison, increment, decrement, dereference).
     *        检测迭代器操作（begin/end比较、递增、递减、解引用）的特化。
     *
     * @tparam Ty The type that provides iterator operations
     *            提供迭代器操作的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL has_iterator_v<
        Ty,
        type_traits::other_trans::void_t<decltype(utility::begin(utility::declval<Ty &>()) != utility::end(utility::declval<Ty &>()),
                                                  ++utility::declval<decltype(utility::begin(utility::declval<Ty &>())) &>(),
                                                  --utility::declval<decltype(utility::begin(utility::declval<Ty &>())) &>(),
                                                  (void) utility::declval<decltype(utility::begin(utility::declval<Ty &>())) &>()++,
                                                  (void) utility::declval<decltype(utility::begin(utility::declval<Ty &>())) &>()--,
                                                  *utility::begin(utility::declval<Ty &>()))>> = true;

    /**
     * @brief Type template for checking if a type satisfies iterator requirements.
     *        检查类型是否满足迭代器要求的类型模板。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename Ty>
    struct has_iterator : helper::bool_constant<has_iterator_v<Ty>> {};
}

namespace rainy::type_traits::extras::meta_method {
    /**
     * @brief Variable template for checking if a type supports operator+.
     *        检查类型是否支持 operator+ 的变量模板。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL has_operator_add_v = false;

    /**
     * @brief Specialization that detects operator+ for the same type.
     *        检测相同类型的 operator+ 的特化。
     *
     * @tparam Ty The type that provides operator+
     *            提供 operator+ 的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL has_operator_add_v<
        Ty, type_traits::other_trans::void_t<decltype(utility::declval<const Ty &>() + utility::declval<const Ty &>())>> = true;

    /**
     * @brief Type template for checking if a type supports operator+.
     *        检查类型是否支持 operator+ 的类型模板。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename Ty>
    struct has_operator_add : helper::bool_constant<has_operator_add_v<Ty>> {};

    /**
     * @brief Variable template for checking if a type supports addition with ptrdiff_t.
     *        检查类型是否支持与 ptrdiff_t 的加法操作的变量模板。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL has_operator_addition_v = false;

    /**
     * @brief Specialization that detects operator+ with ptrdiff_t.
     *        检测与 ptrdiff_t 的 operator+ 的特化。
     *
     * @tparam Ty The type that provides operator+ with ptrdiff_t
     *            提供与 ptrdiff_t 的 operator+ 的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL
        has_operator_addition_v<Ty, other_trans::void_t<decltype(utility::declval<Ty &>() + utility::declval<std::ptrdiff_t>())>> =
            true;

    /**
     * @brief Type template for checking if a type supports addition with ptrdiff_t.
     *        检查类型是否支持与 ptrdiff_t 的加法操作的类型模板。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename Ty>
    struct has_operator_addition : helper::bool_constant<has_operator_addition_v<Ty>> {};

    /**
     * @brief Variable template for checking if a type supports subtraction with ptrdiff_t.
     *        检查类型是否支持与 ptrdiff_t 的减法操作的变量模板。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL has_operator_subtraction_v = false;

    /**
     * @brief Specialization that detects operator- with ptrdiff_t.
     *        检测与 ptrdiff_t 的 operator- 的特化。
     *
     * @tparam Ty The type that provides operator- with ptrdiff_t
     *            提供与 ptrdiff_t 的 operator- 的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL
        has_operator_subtraction_v<Ty, other_trans::void_t<decltype(utility::declval<Ty &>() - utility::declval<std::ptrdiff_t>())>> =
            true;

    /**
     * @brief Type template for checking if a type supports subtraction with ptrdiff_t.
     *        检查类型是否支持与 ptrdiff_t 的减法操作的类型模板。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename Ty>
    struct has_operator_subtraction : helper::bool_constant<has_operator_subtraction_v<Ty>> {};

    /**
     * @brief Variable template for checking if a type supports operator+=.
     *        检查类型是否支持 operator+= 的变量模板。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL has_operator_plus_equal_v = false;

    /**
     * @brief Specialization that detects operator+=.
     *        检测 operator+= 的特化。
     *
     * @tparam Ty The type that provides operator+=
     *            提供 operator+= 的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL has_operator_plus_equal_v<
        Ty, type_traits::other_trans::void_t<decltype(utility::declval<Ty &>() += utility::declval<Ty &>())>> = true;

    /**
     * @brief Type template for checking if a type supports operator+=.
     *        检查类型是否支持 operator+= 的类型模板。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename Ty>
    struct has_operator_plus_equal : helper::bool_constant<has_operator_plus_equal_v<Ty>> {};

    /**
     * @brief Variable template for checking if a type supports operator- (binary).
     *        检查类型是否支持二元 operator- 的变量模板。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL has_operator_sub_v = false;

    /**
     * @brief Specialization that detects binary operator-.
     *        检测二元 operator- 的特化。
     *
     * @tparam Ty The type that provides binary operator-
     *            提供二元 operator- 的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL
        has_operator_sub_v<Ty, type_traits::other_trans::void_t<decltype(utility::declval<Ty &>() - utility::declval<Ty &>())>> = true;

    /**
     * @brief Type template for checking if a type supports binary operator-.
     *        检查类型是否支持二元 operator- 的类型模板。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename Ty>
    struct has_operator_sub : helper::bool_constant<has_operator_sub_v<Ty>> {};

    /**
     * @brief Variable template for checking if a type supports operator-=.
     *        检查类型是否支持 operator-= 的变量模板。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL has_operator_sub_equal_v = false;

    /**
     * @brief Specialization that detects operator-=.
     *        检测 operator-= 的特化。
     *
     * @tparam Ty The type that provides operator-=
     *            提供 operator-= 的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL has_operator_sub_equal_v<
        Ty, type_traits::other_trans::void_t<decltype(utility::declval<Ty &>() -= utility::declval<Ty &>())>> = true;

    /**
     * @brief Type template for checking if a type supports operator-=.
     *        检查类型是否支持 operator-= 的类型模板。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename Ty>
    struct has_operator_sub_equal : helper::bool_constant<has_operator_plus_equal_v<Ty>> {};

    /**
     * @brief Variable template for checking if a type supports operator*.
     *        检查类型是否支持 operator* 的变量模板。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL has_operator_mul_v = false;

    /**
     * @brief Specialization that detects operator*.
     *        检测 operator* 的特化。
     *
     * @tparam Ty The type that provides operator*
     *            提供 operator* 的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL
        has_operator_mul_v<Ty, type_traits::other_trans::void_t<decltype(utility::declval<Ty &>() * utility::declval<Ty &>())>> = true;

    /**
     * @brief Type template for checking if a type supports operator*.
     *        检查类型是否支持 operator* 的类型模板。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename Ty>
    struct has_operator_mul : helper::bool_constant<has_operator_mul_v<Ty>> {};

    /**
     * @brief Variable template for checking if a type supports operator*=.
     *        检查类型是否支持 operator*= 的变量模板。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL has_operator_mul_equal_v = false;

    /**
     * @brief Specialization that detects operator*=.
     *        检测 operator*= 的特化。
     *
     * @tparam Ty The type that provides operator*=
     *            提供 operator*= 的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL has_operator_mul_equal_v<
        Ty, type_traits::other_trans::void_t<decltype(utility::declval<Ty &>() *= utility::declval<Ty &>())>> = true;

    /**
     * @brief Type template for checking if a type supports operator*=.
     *        检查类型是否支持 operator*= 的类型模板。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename Ty>
    struct has_operator_mul_equal : helper::bool_constant<has_operator_mul_equal_v<Ty>> {};

    /**
     * @brief Variable template for checking if a type supports operator/.
     *        检查类型是否支持 operator/ 的变量模板。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL has_operator_div_v = false;

    /**
     * @brief Specialization that detects operator/.
     *        检测 operator/ 的特化。
     *
     * @tparam Ty The type that provides operator/
     *            提供 operator/ 的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL
        has_operator_div_v<Ty, type_traits::other_trans::void_t<decltype(utility::declval<Ty &>() / utility::declval<Ty &>())>> = true;

    /**
     * @brief Type template for checking if a type supports operator/.
     *        检查类型是否支持 operator/ 的类型模板。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename Ty>
    struct has_operator_div : helper::bool_constant<has_operator_div_v<Ty>> {};

    /**
     * @brief Variable template for checking if a type supports operator/=.
     *        检查类型是否支持 operator/= 的变量模板。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL has_operator_div_equal_v = false;

    /**
     * @brief Specialization that detects operator/=.
     *        检测 operator/= 的特化。
     *
     * @tparam Ty The type that provides operator/=
     *            提供 operator/= 的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL has_operator_div_equal_v<
        Ty, type_traits::other_trans::void_t<decltype(utility::declval<Ty &>() /= utility::declval<Ty &>())>> = true;

    /**
     * @brief Type template for checking if a type supports operator/=.
     *        检查类型是否支持 operator/= 的类型模板。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename Ty>
    struct has_operator_div_equal : helper::bool_constant<has_operator_div_equal_v<Ty>> {};

    /**
     * @brief Variable template for checking if a type supports operator%.
     *        检查类型是否支持 operator% 的变量模板。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL has_operator_mod_v = false;

    /**
     * @brief Specialization that detects operator%.
     *        检测 operator% 的特化。
     *
     * @tparam Ty The type that provides operator%
     *            提供 operator% 的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL
        has_operator_mod_v<Ty, type_traits::other_trans::void_t<decltype(utility::declval<Ty &>() % utility::declval<Ty &>())>> = true;

    /**
     * @brief Type template for checking if a type supports operator%.
     *        检查类型是否支持 operator% 的类型模板。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename Ty>
    struct has_operator_mod : helper::bool_constant<has_operator_mod_v<Ty>> {};

    /**
     * @brief Variable template for checking if a type supports operator%=.
     *        检查类型是否支持 operator%= 的变量模板。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL has_operator_mod_equal_v = false;

    /**
     * @brief Specialization that detects operator%=.
     *        检测 operator%= 的特化。
     *
     * @tparam Ty The type that provides operator%=
     *            提供 operator%= 的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL has_operator_mod_equal_v<
        Ty, type_traits::other_trans::void_t<decltype(utility::declval<Ty &>() %= utility::declval<Ty &>())>> = true;

    /**
     * @brief Type template for checking if a type supports operator%=.
     *        检查类型是否支持 operator%= 的类型模板。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename Ty>
    struct has_operator_mod_equal : helper::bool_constant<has_operator_mod_equal_v<Ty>> {};

    /**
     * @brief Variable template for checking if a type supports operator==.
     *        检查类型是否支持 operator== 的变量模板。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL has_operator_eq_v = false;

    /**
     * @brief Specialization that detects operator==.
     *        检测 operator== 的特化。
     *
     * @tparam Ty The type that provides operator==
     *            提供 operator== 的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL
        has_operator_eq_v<Ty, type_traits::other_trans::void_t<decltype(utility::declval<Ty &>() == utility::declval<Ty &>())>> = true;

    /**
     * @brief Type template for checking if a type supports operator==.
     *        检查类型是否支持 operator== 的类型模板。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename Ty>
    struct has_operator_eq : helper::bool_constant<has_operator_eq_v<Ty>> {};

    /**
     * @brief Variable template for checking if a type supports operator!=.
     *        检查类型是否支持 operator!= 的变量模板。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL has_operator_neq_v = false;

    /**
     * @brief Specialization that detects operator!=.
     *        检测 operator!= 的特化。
     *
     * @tparam Ty The type that provides operator!=
     *            提供 operator!= 的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL
        has_operator_neq_v<Ty, type_traits::other_trans::void_t<decltype(utility::declval<Ty &>() != utility::declval<Ty &>())>> =
            true;

    /**
     * @brief Type template for checking if a type supports operator!=.
     *        检查类型是否支持 operator!= 的类型模板。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename Ty>
    struct has_operator_neq : helper::bool_constant<has_operator_neq_v<Ty>> {};

    /**
     * @brief Variable template for checking if a type supports operator<.
     *        检查类型是否支持 operator< 的变量模板。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL has_operator_lt_v = false;

    /**
     * @brief Specialization that detects operator<.
     *        检测 operator< 的特化。
     *
     * @tparam Ty The type that provides operator<
     *            提供 operator< 的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL has_operator_lt_v<
        Ty, type_traits::other_trans::void_t<decltype(utility::declval<const Ty &>() < utility::declval<const Ty &>())>> = true;

    /**
     * @brief Type template for checking if a type supports operator<.
     *        检查类型是否支持 operator< 的类型模板。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename Ty>
    struct has_operator_lt : helper::bool_constant<has_operator_lt_v<Ty>> {};

    /**
     * @brief Variable template for checking if a type supports operator>.
     *        检查类型是否支持 operator> 的变量模板。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL has_operator_gt_v = false;

    /**
     * @brief Specialization that detects operator>.
     *        检测 operator> 的特化。
     *
     * @tparam Ty The type that provides operator>
     *            提供 operator> 的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL
        has_operator_gt_v<Ty, type_traits::other_trans::void_t<decltype(utility::declval<Ty &>() > utility::declval<Ty &>())>> = true;

    /**
     * @brief Type template for checking if a type supports operator>.
     *        检查类型是否支持 operator> 的类型模板。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename Ty>
    struct has_operator_gt : helper::bool_constant<has_operator_gt_v<Ty>> {};

    /**
     * @brief Variable template for checking if a type supports operator<=.
     *        检查类型是否支持 operator<= 的变量模板。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL has_operator_le_v = false;

    /**
     * @brief Specialization that detects operator<=.
     *        检测 operator<= 的特化。
     *
     * @tparam Ty The type that provides operator<=
     *            提供 operator<= 的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL
        has_operator_le_v<Ty, type_traits::other_trans::void_t<decltype(utility::declval<Ty &>() <= utility::declval<Ty &>())>> = true;

    /**
     * @brief Type template for checking if a type supports operator<=.
     *        检查类型是否支持 operator<= 的类型模板。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename Ty>
    struct has_operator_le : helper::bool_constant<has_operator_le_v<Ty>> {};

    /**
     * @brief Variable template for checking if a type supports operator>=.
     *        检查类型是否支持 operator>= 的变量模板。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL has_operator_ge_v = false;

    /**
     * @brief Specialization that detects operator>=.
     *        检测 operator>= 的特化。
     *
     * @tparam Ty The type that provides operator>=
     *            提供 operator>= 的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL
        has_operator_ge_v<Ty, type_traits::other_trans::void_t<decltype(utility::declval<Ty &>() >= utility::declval<Ty &>())>> = true;

    /**
     * @brief Type template for checking if a type supports operator>=.
     *        检查类型是否支持 operator>= 的类型模板。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename Ty>
    struct has_operator_ge : helper::bool_constant<has_operator_ge_v<Ty>> {};

    /**
     * @brief Variable template for checking if a type supports operator=.
     *        检查类型是否支持 operator= 的变量模板。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL has_operator_assign_v = false;

    /**
     * @brief Specialization that detects operator=.
     *        检测 operator= 的特化。
     *
     * @tparam Ty The type that provides operator=
     *            提供 operator= 的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL
        has_operator_assign_v<Ty, type_traits::other_trans::void_t<decltype(utility::declval<Ty &>() = utility::declval<Ty &>())>> =
            true;

    /**
     * @brief Type template for checking if a type supports operator=.
     *        检查类型是否支持 operator= 的类型模板。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename Ty>
    struct has_operator_assign : helper::bool_constant<has_operator_assign_v<Ty>> {};

    /**
     * @brief Variable template for checking if a type supports operator[].
     *        检查类型是否支持 operator[] 的变量模板。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL has_operator_index_v = false;

    /**
     * @brief Specialization that detects operator[] with size_t.
     *        检测与 size_t 的 operator[] 的特化。
     *
     * @tparam Ty The type that provides operator[]
     *            提供 operator[] 的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL
        has_operator_index_v<Ty, type_traits::other_trans::void_t<decltype(utility::declval<Ty &>()[std::declval<std::size_t>()])>> =
            true;

    /**
     * @brief Variable template for checking if a type supports operator[] with key_type.
     *        检查类型是否支持使用 key_type 的 operator[] 的变量模板。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL has_operator_index_for_key_v = false;

    /**
     * @brief Specialization that detects operator[] with key_type.
     *        检测与 key_type 的 operator[] 的特化。
     *
     * @tparam Ty The type that provides operator[] with key_type
     *            提供与 key_type 的 operator[] 的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL has_operator_index_for_key_v<
        Ty, type_traits::other_trans::void_t<typename Ty::key_type,
                                             decltype(utility::declval<Ty &>()[std::declval<typename Ty::key_type>()])>> = true;

    /**
     * @brief Type template for checking if a type supports operator[].
     *        检查类型是否支持 operator[] 的类型模板。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename Ty>
    struct has_operator_index : helper::bool_constant<has_operator_index_v<Ty>> {};

    /**
     * @brief Variable template for checking if a type supports operator().
     *        检查类型是否支持 operator() 的变量模板。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL has_operator_call_v = false;

    /**
     * @brief Specialization that detects operator() with size_type parameter.
     *        检测带有 size_type 参数的 operator() 的特化。
     *
     * @tparam Ty The type that provides operator()
     *            提供 operator() 的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL has_operator_call_v<
        Ty, type_traits::other_trans::void_t<decltype(utility::declval<Ty &>()(std::declval<typename Ty::size_type>()))>> = true;

    /**
     * @brief Type template for checking if a type supports operator().
     *        检查类型是否支持 operator() 的类型模板。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename Ty>
    struct has_operator_call : helper::bool_constant<has_operator_call_v<Ty>> {};

    /**
     * @brief Variable template for checking if a type supports operator->.
     *        检查类型是否支持 operator-> 的变量模板。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL has_operator_arrow_v = false;

    /**
     * @brief Specialization that detects operator->.
     *        检测 operator-> 的特化。
     *
     * @tparam Ty The type that provides operator->
     *            提供 operator-> 的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL has_operator_arrow_v<Ty, type_traits::other_trans::void_t<decltype(utility::declval<Ty &>().operator->())>> =
        true;

    /**
     * @brief Type template for checking if a type supports operator->.
     *        检查类型是否支持 operator-> 的类型模板。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename Ty>
    struct has_operator_arrow : helper::bool_constant<has_operator_arrow_v<Ty>> {};

    /**
     * @brief Variable template for checking if a type supports three-way comparison (C++20).
     *        检查类型是否支持三路比较（C++20）的变量模板。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL has_operator_three_way_compare_v = false;

#if RAINY_HAS_CXX20
    /**
     * @brief Specialization that detects operator<=> (C++20).
     *        检测 operator<=> 的特化（C++20）。
     *
     * @tparam Ty The type that provides operator<=>
     *            提供 operator<=> 的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL has_operator_three_way_compare_v<
        Ty, type_traits::other_trans::void_t<decltype(utility::declval<Ty &>() <=> utility::declval<Ty &>())>> = true;
#endif

    /**
     * @brief Type template for checking if a type supports three-way comparison.
     *        检查类型是否支持三路比较的类型模板。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename Ty>
    struct has_operator_three_way_compare : helper::bool_constant<has_operator_three_way_compare_v<Ty>> {};

    /**
     * @brief Variable template for checking if a type supports operator* (dereference).
     *        检查类型是否支持 operator*（解引用）的变量模板。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL has_operator_deref_v = false;

    /**
     * @brief Specialization that detects operator* (dereference).
     *        检测 operator*（解引用）的特化。
     *
     * @tparam Ty The type that provides operator* (dereference)
     *            提供 operator*（解引用）的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL has_operator_deref_v<Ty, type_traits::other_trans::void_t<decltype(*utility::declval<Ty &>())>> = true;

    /**
     * @brief Type template for checking if a type supports operator* (dereference).
     *        检查类型是否支持 operator*（解引用）的类型模板。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename Ty>
    struct has_operator_deref : helper::bool_constant<has_operator_deref_v<Ty>> {};

    /**
     * @brief Variable template for checking if a type supports operator& (address-of).
     *        检查类型是否支持 operator&（取地址）的变量模板。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL has_operator_addr_v = false;

    /**
     * @brief Specialization that detects operator& (address-of).
     *        检测 operator&（取地址）的特化。
     *
     * @tparam Ty The type that provides operator&
     *            提供 operator& 的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL has_operator_addr_v<Ty, type_traits::other_trans::void_t<decltype(&utility::declval<Ty &>())>> = true;

    /**
     * @brief Type template for checking if a type supports operator& (address-of).
     *        检查类型是否支持 operator&（取地址）的类型模板。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename Ty>
    struct has_operator_addr : helper::bool_constant<has_operator_addr_v<Ty>> {};

    /**
     * @brief Variable template for checking if a type supports prefix increment (++).
     *        检查类型是否支持前置自增（++）的变量模板。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL has_operator_preinc_v = false;

    /**
     * @brief Specialization that detects prefix increment.
     *        检测前置自增的特化。
     *
     * @tparam Ty The type that provides prefix increment
     *            提供前置自增的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL has_operator_preinc_v<Ty, type_traits::other_trans::void_t<decltype(++utility::declval<Ty &>())>> = true;

    /**
     * @brief Type template for checking if a type supports prefix increment.
     *        检查类型是否支持前置自增的类型模板。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename Ty>
    struct has_operator_preinc : helper::bool_constant<has_operator_preinc_v<Ty>> {};

    /**
     * @brief Variable template for checking if a type supports prefix decrement (--).
     *        检查类型是否支持前置自减（--）的变量模板。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL has_operator_predec_v = false;

    /**
     * @brief Specialization that detects prefix decrement.
     *        检测前置自减的特化。
     *
     * @tparam Ty The type that provides prefix decrement
     *            提供前置自减的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL has_operator_predec_v<Ty, type_traits::other_trans::void_t<decltype(--utility::declval<Ty &>())>> = true;

    /**
     * @brief Type template for checking if a type supports prefix decrement.
     *        检查类型是否支持前置自减的类型模板。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename Ty>
    struct has_operator_predec : helper::bool_constant<has_operator_predec_v<Ty>> {};

    /**
     * @brief Variable template for checking if a type supports postfix increment (++).
     *        检查类型是否支持后置自增（++）的变量模板。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL has_operator_postinc_v = false;

    /**
     * @brief Specialization that detects postfix increment.
     *        检测后置自增的特化。
     *
     * @tparam Ty The type that provides postfix increment
     *            提供后置自增的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL has_operator_postinc_v<Ty, type_traits::other_trans::void_t<decltype(utility::declval<Ty &>()++)>> = true;

    /**
     * @brief Type template for checking if a type supports postfix increment.
     *        检查类型是否支持后置自增的类型模板。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename Ty>
    struct has_operator_postinc : helper::bool_constant<has_operator_preinc_v<Ty>> {};

    /**
     * @brief Variable template for checking if a type supports postfix decrement (--).
     *        检查类型是否支持后置自减（--）的变量模板。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL has_operator_postdec_v = false;

    /**
     * @brief Specialization that detects postfix decrement.
     *        检测后置自减的特化。
     *
     * @tparam Ty The type that provides postfix decrement
     *            提供后置自减的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL has_operator_postdec_v<Ty, type_traits::other_trans::void_t<decltype(utility::declval<Ty &>()--)>> = true;

    /**
     * @brief Type template for checking if a type supports postfix decrement.
     *        检查类型是否支持后置自减的类型模板。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename Ty>
    struct has_operator_postdec : helper::bool_constant<has_operator_postdec_v<Ty>> {};

    /**
     * @brief Variable template for checking if a type supports operator||.
     *        检查类型是否支持 operator|| 的变量模板。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL has_operator_or_v = false;

    /**
     * @brief Specialization that detects operator||.
     *        检测 operator|| 的特化。
     *
     * @tparam Ty The type that provides operator||
     *            提供 operator|| 的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL
        has_operator_or_v<Ty, type_traits::other_trans::void_t<decltype(utility::declval<Ty &>() || utility::declval<Ty &>())>> = true;

    /**
     * @brief Type template for checking if a type supports operator||.
     *        检查类型是否支持 operator|| 的类型模板。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename Ty>
    struct has_operator_or : helper::bool_constant<has_operator_or_v<Ty>> {};

    /**
     * @brief Variable template for checking if a type supports operator&&.
     *        检查类型是否支持 operator&& 的变量模板。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL has_operator_and_v = false;

    /**
     * @brief Specialization that detects operator&&.
     *        检测 operator&& 的特化。
     *
     * @tparam Ty The type that provides operator&&
     *            提供 operator&& 的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL
        has_operator_and_v<Ty, type_traits::other_trans::void_t<decltype(utility::declval<Ty &>() && utility::declval<Ty &>())>> =
            true;

    /**
     * @brief Type template for checking if a type supports operator&&.
     *        检查类型是否支持 operator&& 的类型模板。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename Ty>
    struct has_operator_and : helper::bool_constant<has_operator_and_v<Ty>> {};

    /**
     * @brief Variable template for checking if a type supports operator!.
     *        检查类型是否支持 operator! 的变量模板。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL has_operator_not_v = false;

    /**
     * @brief Specialization that detects operator!.
     *        检测 operator! 的特化。
     *
     * @tparam Ty The type that provides operator!
     *            提供 operator! 的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL has_operator_not_v<Ty, type_traits::other_trans::void_t<decltype(!utility::declval<Ty &>())>> = true;

    /**
     * @brief Type template for checking if a type supports operator!.
     *        检查类型是否支持 operator! 的类型模板。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename Ty>
    struct has_operator_not : helper::bool_constant<has_operator_not_v<Ty>> {};

    /**
     * @brief Variable template for checking if a type supports operator|.
     *        检查类型是否支持 operator| 的变量模板。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL has_operator_bit_or_v = false;

    /**
     * @brief Specialization that detects operator|.
     *        检测 operator| 的特化。
     *
     * @tparam Ty The type that provides operator|
     *            提供 operator| 的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL
        has_operator_bit_or_v<Ty, type_traits::other_trans::void_t<decltype(utility::declval<Ty &>() | utility::declval<Ty &>())>> =
            true;

    /**
     * @brief Type template for checking if a type supports operator|.
     *        检查类型是否支持 operator| 的类型模板。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename Ty>
    struct has_operator_bit_or : helper::bool_constant<has_operator_bit_or_v<Ty>> {};

    /**
     * @brief Variable template for checking if a type supports operator&.
     *        检查类型是否支持 operator& 的变量模板。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL has_operator_bit_and_v = false;

    /**
     * @brief Specialization that detects operator&.
     *        检测 operator& 的特化。
     *
     * @tparam Ty The type that provides operator&
     *            提供 operator& 的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL
        has_operator_bit_and_v<Ty, type_traits::other_trans::void_t<decltype(utility::declval<Ty &>() & utility::declval<Ty &>())>> =
            true;

    /**
     * @brief Type template for checking if a type supports operator&.
     *        检查类型是否支持 operator& 的类型模板。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename Ty>
    struct has_operator_bit_and : helper::bool_constant<has_operator_bit_and_v<Ty>> {};

    /**
     * @brief Variable template for checking if a type supports operator^.
     *        检查类型是否支持 operator^ 的变量模板。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL has_operator_bit_xor_v = false;

    /**
     * @brief Specialization that detects operator^.
     *        检测 operator^ 的特化。
     *
     * @tparam Ty The type that provides operator^
     *            提供 operator^ 的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL
        has_operator_bit_xor_v<Ty, type_traits::other_trans::void_t<decltype(utility::declval<Ty &>() ^ utility::declval<Ty &>())>> =
            true;

    /**
     * @brief Type template for checking if a type supports operator^.
     *        检查类型是否支持 operator^ 的类型模板。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename Ty>
    struct has_operator_bit_xor : helper::bool_constant<has_operator_bit_xor_v<Ty>> {};

    /**
     * @brief Variable template for checking if a type supports operator~.
     *        检查类型是否支持 operator~ 的变量模板。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL has_operator_bit_not_v = false;

    /**
     * @brief Specialization that detects operator~.
     *        检测 operator~ 的特化。
     *
     * @tparam Ty The type that provides operator~
     *            提供 operator~ 的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL has_operator_bit_not_v<Ty, type_traits::other_trans::void_t<decltype(~utility::declval<Ty &>())>> = true;

    /**
     * @brief Type template for checking if a type supports operator~.
     *        检查类型是否支持 operator~ 的类型模板。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename Ty>
    struct has_operator_bit_not : helper::bool_constant<has_operator_bit_not_v<Ty>> {};

    /**
     * @brief Variable template for checking if a type supports operator<<.
     *        检查类型是否支持 operator<< 的变量模板。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL has_operator_shift_l_v = false;

    /**
     * @brief Specialization that detects operator<<.
     *        检测 operator<< 的特化。
     *
     * @tparam Ty The type that provides operator<<
     *            提供 operator<< 的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL
        has_operator_shift_l_v<Ty, type_traits::other_trans::void_t<decltype(utility::declval<Ty &>() << utility::declval<Ty &>())>> =
            true;

    /**
     * @brief Type template for checking if a type supports operator<<.
     *        检查类型是否支持 operator<< 的类型模板。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename Ty>
    struct has_operator_shift_l : helper::bool_constant<has_operator_shift_l_v<Ty>> {};

    /**
     * @brief Variable template for checking if a type supports operator>>.
     *        检查类型是否支持 operator>> 的变量模板。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL has_operator_shift_r_v = false;

    /**
     * @brief Specialization that detects operator>>.
     *        检测 operator>> 的特化。
     *
     * @tparam Ty The type that provides operator>>
     *            提供 operator>> 的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL
        has_operator_shift_r_v<Ty, type_traits::other_trans::void_t<decltype(utility::declval<Ty &>() >> utility::declval<Ty &>())>> =
            true;

    /**
     * @brief Type template for checking if a type supports operator>>.
     *        检查类型是否支持 operator>> 的类型模板。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename Ty>
    struct has_operator_shift_r : helper::bool_constant<has_operator_shift_r_v<Ty>> {};
}

namespace rainy::type_traits::extras::meta_method {
    /**
     * @brief Variable template for checking if a type has a begin() member function.
     *        检查类型是否具有 begin() 成员函数的变量模板。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL has_begin_v = false;

    /**
     * @brief Specialization that detects the presence of Ty::begin().
     *        检测是否存在 Ty::begin() 的特化。
     *
     * @tparam Ty The type that provides begin()
     *            提供 begin() 的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL has_begin_v<Ty, type_traits::other_trans::void_t<decltype(utility::declval<Ty &>().begin())>> = true;

    /**
     * @brief Type template for checking if a type has a begin() member function.
     *        检查类型是否具有 begin() 成员函数的类型模板。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename Ty>
    struct has_begin : helper::bool_constant<has_begin_v<Ty>> {};

    /**
     * @brief Variable template for checking if a type has an end() member function.
     *        检查类型是否具有 end() 成员函数的变量模板。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL has_end_v = false;

    /**
     * @brief Specialization that detects the presence of Ty::end().
     *        检测是否存在 Ty::end() 的特化。
     *
     * @tparam Ty The type that provides end()
     *            提供 end() 的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL has_end_v<Ty, type_traits::other_trans::void_t<decltype(utility::declval<Ty &>().end())>> = true;

    /**
     * @brief Type template for checking if a type has an end() member function.
     *        检查类型是否具有 end() 成员函数的类型模板。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename Ty>
    struct has_end : helper::bool_constant<has_end_v<Ty>> {};

    /**
     * @brief Variable template for checking if a type has a cbegin() member function.
     *        检查类型是否具有 cbegin() 成员函数的变量模板。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL has_cbegin_v = false;

    /**
     * @brief Specialization that detects the presence of Ty::cbegin().
     *        检测是否存在 Ty::cbegin() 的特化。
     *
     * @tparam Ty The type that provides cbegin()
     *            提供 cbegin() 的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL has_cbegin_v<Ty, type_traits::other_trans::void_t<decltype(utility::declval<const Ty &>().cbegin())>> = true;

    /**
     * @brief Type template for checking if a type has a cbegin() member function.
     *        检查类型是否具有 cbegin() 成员函数的类型模板。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename Ty>
    struct has_cbegin : helper::bool_constant<has_cbegin_v<Ty>> {};

    /**
     * @brief Variable template for checking if a type has a cend() member function.
     *        检查类型是否具有 cend() 成员函数的变量模板。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL has_cend_v = false;

    /**
     * @brief Specialization that detects the presence of Ty::cend().
     *        检测是否存在 Ty::cend() 的特化。
     *
     * @tparam Ty The type that provides cend()
     *            提供 cend() 的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL has_cend_v<Ty, type_traits::other_trans::void_t<decltype(utility::declval<const Ty &>().cend())>> = true;

    /**
     * @brief Type template for checking if a type has a cend() member function.
     *        检查类型是否具有 cend() 成员函数的类型模板。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename Ty>
    struct has_cend : helper::bool_constant<has_cend_v<Ty>> {};

    /**
     * @brief Variable template for checking if a type has an rbegin() member function.
     *        检查类型是否具有 rbegin() 成员函数的变量模板。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL has_rbegin_v = false;

    /**
     * @brief Specialization that detects the presence of Ty::rbegin().
     *        检测是否存在 Ty::rbegin() 的特化。
     *
     * @tparam Ty The type that provides rbegin()
     *            提供 rbegin() 的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL has_rbegin_v<Ty, type_traits::other_trans::void_t<decltype(utility::declval<Ty &>().rbegin())>> = true;

    /**
     * @brief Type template for checking if a type has an rbegin() member function.
     *        检查类型是否具有 rbegin() 成员函数的类型模板。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename Ty>
    struct has_rbegin : helper::bool_constant<has_rbegin_v<Ty>> {};

    /**
     * @brief Variable template for checking if a type has a rend() member function.
     *        检查类型是否具有 rend() 成员函数的变量模板。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL has_rend_v = false;

    /**
     * @brief Specialization that detects the presence of Ty::rend().
     *        检测是否存在 Ty::rend() 的特化。
     *
     * @tparam Ty The type that provides rend()
     *            提供 rend() 的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL has_rend_v<Ty, type_traits::other_trans::void_t<decltype(utility::declval<Ty &>().rend())>> = true;

    /**
     * @brief Type template for checking if a type has a rend() member function.
     *        检查类型是否具有 rend() 成员函数的类型模板。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename Ty>
    struct has_rend : helper::bool_constant<has_rend_v<Ty>> {};

    /**
     * @brief Variable template for checking if a type has a size() member function.
     *        检查类型是否具有 size() 成员函数的变量模板。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL has_size_v = false;

    /**
     * @brief Specialization that detects the presence of Ty::size().
     *        检测是否存在 Ty::size() 的特化。
     *
     * @tparam Ty The type that provides size()
     *            提供 size() 的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL has_size_v<Ty, type_traits::other_trans::void_t<decltype(utility::declval<Ty &>().size())>> = true;

    /**
     * @brief Type template for checking if a type has a size() member function.
     *        检查类型是否具有 size() 成员函数的类型模板。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename Ty>
    struct has_size : helper::bool_constant<has_size_v<Ty>> {};

    /**
     * @brief Variable template for checking if a type has an empty() member function.
     *        检查类型是否具有 empty() 成员函数的变量模板。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL has_empty_v = false;

    /**
     * @brief Specialization that detects the presence of Ty::empty().
     *        检测是否存在 Ty::empty() 的特化。
     *
     * @tparam Ty The type that provides empty()
     *            提供 empty() 的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL has_empty_v<Ty, type_traits::other_trans::void_t<decltype(utility::declval<Ty &>().empty())>> = true;

    /**
     * @brief Type template for checking if a type has an empty() member function.
     *        检查类型是否具有 empty() 成员函数的类型模板。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename Ty>
    struct has_empty : helper::bool_constant<has_empty_v<Ty>> {};

    /**
     * @brief Variable template for checking if a type has a clear() member function.
     *        检查类型是否具有 clear() 成员函数的变量模板。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL has_clear_v = false;

    /**
     * @brief Specialization that detects the presence of Ty::clear().
     *        检测是否存在 Ty::clear() 的特化。
     *
     * @tparam Ty The type that provides clear()
     *            提供 clear() 的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL has_clear_v<Ty, type_traits::other_trans::void_t<decltype(utility::declval<Ty &>().clear())>> = true;

    /**
     * @brief Type template for checking if a type has a clear() member function.
     *        检查类型是否具有 clear() 成员函数的类型模板。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename Ty>
    struct has_clear : helper::bool_constant<has_clear_v<Ty>> {};

    /**
     * @brief Variable template for checking if a type has a push_back() member function.
     *        检查类型是否具有 push_back() 成员函数的变量模板。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL has_push_back_v = false;

    /**
     * @brief Specialization that detects the presence of Ty::push_back() with value_type parameter.
     *        检测是否存在带有 value_type 参数的 Ty::push_back() 的特化。
     *
     * @tparam Ty The type that provides push_back()
     *            提供 push_back() 的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL has_push_back_v<
        Ty, type_traits::other_trans::void_t<decltype(utility::declval<Ty &>().push_back(std::declval<typename Ty::value_type>()))>> =
        true;

    /**
     * @brief Type template for checking if a type has a push_back() member function.
     *        检查类型是否具有 push_back() 成员函数的类型模板。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename Ty>
    struct has_push_back : helper::bool_constant<has_push_back_v<Ty>> {};

    /**
     * @brief Variable template for checking if a type has an emplace_back() member function.
     *        检查类型是否具有 emplace_back() 成员函数的变量模板。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL has_emplace_back_v = false;

    /**
     * @brief Specialization that detects the presence of Ty::emplace_back().
     *        检测是否存在 Ty::emplace_back() 的特化。
     *
     * @tparam Ty The type that provides emplace_back()
     *            提供 emplace_back() 的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL has_emplace_back_v<Ty, type_traits::other_trans::void_t<decltype(utility::declval<Ty &>().emplace_back(
                                                    std::declval<typename Ty::value_type>()))>> = true;

    /**
     * @brief Type template for checking if a type has an emplace_back() member function.
     *        检查类型是否具有 emplace_back() 成员函数的类型模板。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename Ty>
    struct has_emplace_back : helper::bool_constant<has_emplace_back_v<Ty>> {};

    /**
     * @brief Variable template for checking if a type has a pop_back() member function.
     *        检查类型是否具有 pop_back() 成员函数的变量模板。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL has_pop_back_v = false;

    /**
     * @brief Specialization that detects the presence of Ty::pop_back().
     *        检测是否存在 Ty::pop_back() 的特化。
     *
     * @tparam Ty The type that provides pop_back()
     *            提供 pop_back() 的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL has_pop_back_v<Ty, type_traits::other_trans::void_t<decltype(utility::declval<Ty &>().pop_back())>> = true;

    /**
     * @brief Type template for checking if a type has a pop_back() member function.
     *        检查类型是否具有 pop_back() 成员函数的类型模板。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename Ty>
    struct has_pop_back : helper::bool_constant<has_pop_back_v<Ty>> {};

    /**
     * @brief Variable template for checking if a type has a pop_front() member function.
     *        检查类型是否具有 pop_front() 成员函数的变量模板。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL has_pop_front_v = false;

    /**
     * @brief Specialization that detects the presence of Ty::pop_front().
     *        检测是否存在 Ty::pop_front() 的特化。
     *
     * @tparam Ty The type that provides pop_front()
     *            提供 pop_front() 的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL has_pop_front_v<Ty, type_traits::other_trans::void_t<decltype(utility::declval<Ty &>().pop_front())>> = true;

    /**
     * @brief Type template for checking if a type has a pop_front() member function.
     *        检查类型是否具有 pop_front() 成员函数的类型模板。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename Ty>
    struct has_pop_front : helper::bool_constant<has_pop_front_v<Ty>> {};

    /**
     * @brief Variable template for checking if a type has an insert() member function that takes a key.
     *        检查类型是否具有接受键参数的 insert() 成员函数的变量模板。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL has_insert_for_key_v = false;

    /**
     * @brief Specialization that detects insert() with key_type parameter.
     *        检测带有 key_type 参数的 insert() 的特化。
     *
     * @tparam Ty The type that provides insert() with key
     *            提供带有键的 insert() 的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL has_insert_for_key_v<
        Ty, other_trans::void_t<decltype(utility::declval<Ty &>().insert(utility::declval<typename Ty::key_type>()))>> = true;

    /**
     * @brief Type template for checking if a type has an insert() member function that takes a key.
     *        检查类型是否具有接受键参数的 insert() 成员函数的类型模板。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename Ty>
    struct has_insert_for_key : helper::bool_constant<has_insert_for_key_v<Ty>> {};

    /**
     * @brief Variable template for checking if a type has an insert() member function that takes a value.
     *        检查类型是否具有接受值参数的 insert() 成员函数的变量模板。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL has_insert_for_value_v = false;

    /**
     * @brief Specialization that detects insert() with value_type parameter.
     *        检测带有 value_type 参数的 insert() 的特化。
     *
     * @tparam Ty The type that provides insert() with value
     *            提供带有值的 insert() 的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL has_insert_for_value_v<
        Ty, other_trans::void_t<decltype(utility::declval<Ty &>().insert(utility::declval<typename Ty::value_type>()))>> = true;

    /**
     * @brief Type template for checking if a type has an insert() member function that takes a value.
     *        检查类型是否具有接受值参数的 insert() 成员函数的类型模板。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename Ty>
    struct has_insert_for_value : helper::bool_constant<has_insert_for_value_v<Ty>> {};

    /**
     * @brief Variable template for checking if a type has an insert() member function that takes an iterator and value.
     *        检查类型是否具有接受迭代器和值参数的 insert() 成员函数的变量模板。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL has_insert_for_iter_and_value_v = false;

    /**
     * @brief Specialization that detects insert() with iterator and value_type parameters.
     *        检测带有迭代器和 value_type 参数的 insert() 的特化。
     *
     * @tparam Ty The type that provides insert() with iterator and value
     *            提供带有迭代器和值的 insert() 的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL has_insert_for_iter_and_value_v<
        Ty, other_trans::void_t<decltype(utility::declval<Ty &>().insert(utility::declval<typename Ty::const_iterator>(),
                                                                         utility::declval<typename Ty::value_type>()))>> = true;

    /**
     * @brief Type template for checking if a type has an insert() member function that takes an iterator and value.
     *        检查类型是否具有接受迭代器和值参数的 insert() 成员函数的类型模板。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename Ty>
    struct has_insert_for_iter_and_value : helper::bool_constant<has_insert_for_iter_and_value_v<Ty>> {};

    /**
     * @brief Variable template for checking if a type has an insert() member function that takes a key and value (associative
     * containers). 检查类型是否具有接受键和值参数的 insert() 成员函数（关联容器）的变量模板。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL has_insert_for_key_and_value_v = false;

    /**
     * @brief Specialization that detects insert() with key and value (for associative containers).
     *        检测带有键和值的 insert() 的特化（用于关联容器）。
     *
     * @tparam Ty The type that provides insert() with key and value
     *            提供带有键和值的 insert() 的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL has_insert_for_key_and_value_v<
        Ty, other_trans::void_t<decltype(utility::declval<Ty &>().insert(utility::declval<typename Ty::const_iterator>(),
                                                                         utility::declval<typename Ty::value_type>()))>> = true;

    /**
     * @brief Type template for checking if a type has an insert() member function that takes a key and value.
     *        检查类型是否具有接受键和值参数的 insert() 成员函数的类型模板。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename Ty>
    struct has_insert_for_key_and_value : helper::bool_constant<has_insert_for_key_and_value_v<Ty>> {};

    /**
     * @brief Variable template for checking if a type has an erase() member function.
     *        检查类型是否具有 erase() 成员函数的变量模板。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL has_erase_v = false;

    /**
     * @brief Specialization that detects erase() with const_iterator parameter.
     *        检测带有 const_iterator 参数的 erase() 的特化。
     *
     * @tparam Ty The type that provides erase()
     *            提供 erase() 的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL has_erase_v<
        Ty, type_traits::other_trans::void_t<decltype(utility::declval<Ty &>().erase(std::declval<typename Ty::const_iterator>()))>> =
        true;

    /**
     * @brief Type template for checking if a type has an erase() member function.
     *        检查类型是否具有 erase() 成员函数的类型模板。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename Ty>
    struct has_erase : helper::bool_constant<has_erase_v<Ty>> {};

    /**
     * @brief Variable template for checking if a type has a find() member function.
     *        检查类型是否具有 find() 成员函数的变量模板。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL has_find_v = false;

    /**
     * @brief Specialization that detects find() with value_type parameter.
     *        检测带有 value_type 参数的 find() 的特化。
     *
     * @tparam Ty The type that provides find()
     *            提供 find() 的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL has_find_v<
        Ty, type_traits::other_trans::void_t<decltype(utility::declval<Ty &>().find(std::declval<typename Ty::value_type>()))>> = true;

    /**
     * @brief Type template for checking if a type has a find() member function.
     *        检查类型是否具有 find() 成员函数的类型模板。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename Ty>
    struct has_find : helper::bool_constant<has_find_v<Ty>> {};

    /**
     * @brief Variable template for checking if a type has a resize() member function.
     *        检查类型是否具有 resize() 成员函数的变量模板。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL has_resize_v = false;

    /**
     * @brief Specialization that detects resize() with size_type parameter.
     *        检测带有 size_type 参数的 resize() 的特化。
     *
     * @tparam Ty The type that provides resize()
     *            提供 resize() 的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL has_resize_v<
        Ty, type_traits::other_trans::void_t<decltype(utility::declval<Ty &>().resize(std::declval<typename Ty::size_type>()))>> =
        true;

    /**
     * @brief Type template for checking if a type has a resize() member function.
     *        检查类型是否具有 resize() 成员函数的类型模板。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename Ty>
    struct has_resize : helper::bool_constant<has_resize_v<Ty>> {};

    /**
     * @brief Variable template for checking if a type has a front() member function.
     *        检查类型是否具有 front() 成员函数的变量模板。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL has_front_v = false;

    /**
     * @brief Specialization that detects front().
     *        检测 front() 的特化。
     *
     * @tparam Ty The type that provides front()
     *            提供 front() 的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL has_front_v<Ty, type_traits::other_trans::void_t<decltype(utility::declval<Ty &>().front())>> = true;

    /**
     * @brief Type template for checking if a type has a front() member function.
     *        检查类型是否具有 front() 成员函数的类型模板。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename Ty>
    struct has_front : helper::bool_constant<has_front_v<Ty>> {};

    /**
     * @brief Variable template for checking if a type has a back() member function.
     *        检查类型是否具有 back() 成员函数的变量模板。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL has_back_v = false;

    /**
     * @brief Specialization that detects back().
     *        检测 back() 的特化。
     *
     * @tparam Ty The type that provides back()
     *            提供 back() 的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL has_back_v<Ty, type_traits::other_trans::void_t<decltype(utility::declval<Ty &>().back())>> = true;

    /**
     * @brief Type template for checking if a type has a back() member function.
     *        检查类型是否具有 back() 成员函数的类型模板。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename Ty>
    struct has_back : helper::bool_constant<has_back_v<Ty>> {};

    /**
     * @brief Variable template for checking if a type has a nested mapped_type.
     *        检查类型是否具有嵌套的 mapped_type 的变量模板。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL has_mapped_type_v = false;

    /**
     * @brief Specialization that detects the presence of Ty::mapped_type.
     *        检测是否存在 Ty::mapped_type 的特化。
     *
     * @tparam Ty The type that provides mapped_type
     *            提供 mapped_type 的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL has_mapped_type_v<Ty, type_traits::other_trans::void_t<typename Ty::mapped_type>> = true;

    /**
     * @brief Type template for checking if a type has a nested mapped_type.
     *        检查类型是否具有嵌套的 mapped_type 的类型模板。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename Ty>
    struct has_mapped_type : type_traits::helper::bool_constant<has_mapped_type_v<Ty>> {};

    /**
     * @brief Variable template for checking if a type has a max_size() member function.
     *        检查类型是否具有 max_size() 成员函数的变量模板。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL has_max_size_v = false;

    /**
     * @brief Specialization that detects max_size().
     *        检测 max_size() 的特化。
     *
     * @tparam Ty The type that provides max_size()
     *            提供 max_size() 的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL has_max_size_v<Ty, type_traits::other_trans::void_t<decltype(utility::declval<const Ty &>().max_size())>> =
        true;

    /**
     * @brief Type template for checking if a type has a max_size() member function.
     *        检查类型是否具有 max_size() 成员函数的类型模板。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename Ty>
    struct has_max_size : type_traits::helper::bool_constant<has_max_size_v<Ty>> {};

    /**
     * @brief Variable template for checking if a type has a data() member function.
     *        检查类型是否具有 data() 成员函数的变量模板。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL has_data_v = false;

    /**
     * @brief Specialization that detects data().
     *        检测 data() 的特化。
     *
     * @tparam Ty The type that provides data()
     *            提供 data() 的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL has_data_v<Ty, type_traits::other_trans::void_t<decltype(utility::declval<const Ty &>().data())>> = true;

    /**
     * @brief Type template for checking if a type has a data() member function.
     *        检查类型是否具有 data() 成员函数的类型模板。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename Ty>
    struct has_data : type_traits::helper::bool_constant<has_data_v<Ty>> {};

    /**
     * @brief Variable template for checking if a type has a length() member function.
     *        检查类型是否具有 length() 成员函数的变量模板。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL has_length_v = false;

    /**
     * @brief Specialization that detects length().
     *        检测 length() 的特化。
     *
     * @tparam Ty The type that provides length()
     *            提供 length() 的类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL has_length_v<Ty, type_traits::other_trans::void_t<decltype(utility::declval<const Ty &>().length())>> = true;

    /**
     * @brief Type template for checking if a type has a length() member function.
     *        检查类型是否具有 length() 成员函数的类型模板。
     *
     * @tparam Ty The type to check
     *            要检查的类型
     */
    template <typename Ty>
    struct has_length : type_traits::helper::bool_constant<has_length_v<Ty>> {};
}

namespace rainy::foundation::memory::implements {
    template <typename Ty, typename Elem>
    struct pointer_traits_base {
        using pointer = Ty;
        using element_type = Elem;
        using difference_type = typename type_traits::extras::templates::get_ptr_difference_type<Ty>::type;

        template <typename other>
        using rebind = typename type_traits::extras::templates::get_rebind_alias<Ty, other>::type;

        using ref_type = type_traits::other_trans::conditional_t<type_traits::primary_types::is_void_v<Elem>, char, Elem> &;

        /**
         * @brief Creates a pointer to the given reference using the type's pointer_to function.
         *        使用类型的 pointer_to 函数创建指向给定引用的指针。
         *
         * @param val The reference to create a pointer to
         *            要创建指针的引用
         * @return A pointer created by Ty::pointer_to(val)
         *         由 Ty::pointer_to(val) 创建的指针
         */
        RAINY_NODISCARD static RAINY_CONSTEXPR20 rain_fn pointer_to(ref_type val) noexcept(noexcept(Ty::pointer_to(val))) -> pointer {
            return Ty::pointer_to(val);
        }
    };

    template <typename, typename = void, typename = void>
    struct ptr_traits_sfinae_layer {};

    template <typename Ty, typename Uty>
    struct ptr_traits_sfinae_layer<
        Ty, Uty, type_traits::other_trans::void_t<typename type_traits::extras::templates::get_first_parameter<Ty>::type>>
        : implements::pointer_traits_base<Ty, typename type_traits::extras::templates::get_first_parameter<Ty>::type> {};

    template <typename Ty>
    struct ptr_traits_sfinae_layer<Ty, type_traits::other_trans::void_t<typename Ty::element_type>, void>
        : implements::pointer_traits_base<Ty, typename Ty::element_type> {};
}

namespace rainy::foundation::memory {
    template <typename Ty>
    struct pointer_traits : implements::ptr_traits_sfinae_layer<Ty> {};

    template <typename Ty>
    struct pointer_traits<Ty *> {
        using pointer = Ty *;
        using elemen_type = Ty;
        using difference_type = ptrdiff_t;

        template <typename other>
        using rebind = other *;

        using ref_type = type_traits::other_trans::conditional_t<type_traits::primary_types::is_void_v<Ty>, char, Ty> &;

        /**
         * @brief Creates a pointer to the given reference using addressof.
         *        使用 addressof 创建指向给定引用的指针。
         *
         * @param val The reference to create a pointer to
         *            要创建指针的引用
         * @return A pointer to the referenced object
         *         指向被引用对象的指针
         */
        RAINY_NODISCARD static constexpr rain_fn pointer_to(ref_type val) noexcept -> pointer {
            return utility::addressof(val);
        }
    };
}

namespace rainy::utility {
    using foundation::memory::pointer_traits;
}

namespace rainy::utility::implements {
    /**
     * @brief Variable template for detecting if pointer_traits<Pointer> has a to_address member function.
     *        检测 pointer_traits<Pointer> 是否具有 to_address 成员函数的变量模板。
     *
     * @tparam Ty The pointer type to check
     *            要检查的指针类型
     */
    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL has_to_address = false;

    /**
     * @brief Specialization that detects the presence of pointer_traits<Pointer>::to_address.
     *        检测 pointer_traits<Pointer>::to_address 是否存在的特化。
     *
     * @tparam Ty The pointer type that provides to_address
     *            提供 to_address 的指针类型
     */
    template <typename Ty>
    RAINY_CONSTEXPR_BOOL
        has_to_address<Ty, type_traits::other_trans::void_t<decltype(foundation::memory::pointer_traits<Ty>::to_address(
                               utility::declval<const Ty &>()))>> = true;
}

namespace rainy::utility {
    /**
     * @brief Converts a raw pointer to an address (identity function).
     *        将原始指针转换为地址（恒等函数）。
     *
     * @tparam Ty The type pointed to
     *            指向的类型
     * @param val The raw pointer
     *            原始指针
     * @return The same pointer value
     *         相同的指针值
     */
    template <typename Ty>
    constexpr rain_fn to_address(Ty *const val) noexcept -> Ty * {
        static_assert(!type_traits::primary_types::is_function_v<Ty>, "Ty cannot be a function type.");
        return val;
    }

    /**
     * @brief Converts any fancy pointer to a raw address.
     *        将任何花哨指针转换为原始地址。
     *
     * @tparam Pointer The fancy pointer type
     *                 花哨指针类型
     * @param val The fancy pointer to convert
     *            要转换的花哨指针
     * @return The raw address obtained either from pointer_traits or operator->
     *         从 pointer_traits 或 operator-> 获取的原始地址
     */
    template <typename Pointer>
    RAINY_NODISCARD constexpr rain_fn to_address(const Pointer &val) noexcept -> auto {
        if constexpr (implements::has_to_address<Pointer>) {
            return pointer_traits<Pointer>::to_address(val);
        } else {
            return utility::to_address(val.operator->());
        }
    }
}

namespace rainy::type_traits::extras::iterators {
    /**
     * @brief Variable template for checking if a type is an input iterator.
     *        Input iterators support dereference and pre-increment.
     *
     *        检查类型是否为输入迭代器的变量模板。
     *        输入迭代器支持解引用和前自增。
     *
     * @tparam It The type to check
     *            要检查的类型
     */
    template <typename It>
    RAINY_CONSTEXPR_BOOL is_input_iterator_v = meta_method::has_operator_deref_v<It> && meta_method::has_operator_preinc_v<It>;

    /**
     * @brief Variable template for checking if a type is an output iterator (primary template).
     *        Output iterators support dereference as lvalue.
     *
     *        检查类型是否为输出迭代器的变量模板（主模板）。
     *        输出迭代器支持解引用作为左值。
     *
     * @tparam It The type to check
     *            要检查的类型
     */
    template <typename It, typename = void>
    RAINY_CONSTEXPR_BOOL is_output_iterator_v = false;

    /**
     * @brief Specialization that checks for lvalue reference from dereference.
     *        检查解引用是否产生左值引用的特化。
     *
     * @tparam It The type to check
     *            要检查的类型
     */
    template <typename It>
    RAINY_CONSTEXPR_BOOL
        is_output_iterator_v<It, other_trans::enable_if_t<meta_method::has_operator_deref_v<It> &&
                                                          primary_types::is_lvalue_reference_v<decltype(*utility::declval<It &>())>>> =
            false;

    /**
     * @brief Variable template for checking if a type is a forward iterator.
     *        Forward iterators are input iterators that are copyable, default constructible,
     *        and support multiple passes.
     *
     *        检查类型是否为前向迭代器的变量模板。
     *        前向迭代器是可拷贝、可默认构造的输入迭代器，支持多次遍历。
     *
     * @tparam It The type to check
     *            要检查的类型
     */
    template <typename It>
    RAINY_CONSTEXPR_BOOL is_forward_iterator_v =
        is_input_iterator_v<It> && type_properties::is_copy_constructible_v<It> && type_properties::is_copy_assignable_v<It> &&
        type_properties::is_default_constructible_v<It>;

    /**
     * @brief Variable template for checking if a type is a bidirectional iterator.
     *        Bidirectional iterators support decrement in addition to forward iterator operations.
     *
     *        检查类型是否为双向迭代器的变量模板。
     *        双向迭代器在前向迭代器操作的基础上支持自减。
     *
     * @tparam It The type to check
     *            要检查的类型
     */
    template <typename It>
    RAINY_CONSTEXPR_BOOL is_bidirectional_iterator_v = is_forward_iterator_v<It> && meta_method::has_operator_predec_v<It>;

    /**
     * @brief Variable template for checking if a type is a random access iterator.
     *        Random access iterators support addition, indexing, and comparison operations.
     *
     *        检查类型是否为随机访问迭代器的变量模板。
     *        随机访问迭代器支持加法、索引和比较操作。
     *
     * @tparam It The type to check
     *            要检查的类型
     */
    template <typename It>
    RAINY_CONSTEXPR_BOOL is_random_access_iterator_v = is_bidirectional_iterator_v<It> && meta_method::has_operator_addition_v<It> &&
                                                       meta_method::has_operator_index_v<It> && meta_method::has_operator_lt_v<It>;

    /**
     * @brief Variable template for checking if a type is a contiguous iterator (primary template).
     *        Contiguous iterators store elements in contiguous memory.
     *
     *        检查类型是否为连续迭代器的变量模板（主模板）。
     *        连续迭代器将元素存储在连续内存中。
     *
     * @tparam It The type to check
     *            要检查的类型
     */
    template <typename It, typename = void>
    RAINY_CONSTEXPR_BOOL is_contiguous_iterator_v = false;

    /**
     * @brief Specialization that checks if the iterator can be converted to a pointer.
     *        检查迭代器是否可以转换为指针的特化。
     *
     * @tparam It The type to check
     *            要检查的类型
     */
    template <typename It>
    RAINY_CONSTEXPR_BOOL is_contiguous_iterator_v<
        It,
        type_traits::other_trans::enable_if_t<is_random_access_iterator_v<It> &&
                                              primary_types::is_pointer_v<decltype(utility::to_address(utility::declval<It>()))>>> =
        true;
}

#endif
