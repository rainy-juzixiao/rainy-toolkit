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
#ifndef RAINY_META_RELF_IMPL_FUNCTION_HPP
#define RAINY_META_RELF_IMPL_FUNCTION_HPP
#include <rainy/foundation/diagnostics/contract.hpp>
#include <rainy/meta/reflection/metadata.hpp>
#include <rainy/meta/reflection/refl_impl/invoker_accessor.hpp>

namespace rainy::meta::reflection {
    /**
     * @brief 表示一个反射函数对象。
     *
     * 该类提供了一种存储和调用具有不同签名的函数的方式。
     * 它支持静态函数和成员函数。
     */
    class RAINY_TOOLKIT_API function {
    public:
        /**
         * @brief 构造一个空的反射函数对象。
         */
        function() noexcept;

        /**
         * @brief 从右值引用中进行移动构造。
         * @param right 待移动的函数对象。
         */
        function(function &&right) noexcept;

        /**
         * @brief 从左值常量引用中进行拷贝构造。
         * @param right 待拷贝的函数对象。
         */
        function(const function &right) noexcept;

        /**
         * @brief 与默认构造函数的行为相同。
         */
        function(std::nullptr_t) noexcept; // NOLINT

        /**
         * @brief 从给定的函数对象构造一个反射函数对象。
         * @tparam Fx 函数对象类型。
         * @param function 函数对象。
         */
        template <typename Fx, typename... Args,
                  type_traits::other_trans::enable_if_t<type_traits::primary_types::function_traits<Fx>::valid, int> = 0>
        function(Fx function, Args &&...default_arguments) noexcept : invoke_accessor_{} { // NOLINT
            using traits = type_traits::primary_types::function_traits<Fx>;
            using paramlist = typename type_traits::other_trans::tuple_like_to_type_list<typename traits::tuple_like_type>::type;

            static constexpr std::size_t arity = traits::arity;
            static constexpr std::size_t default_arg_count = sizeof...(Args);
            static constexpr std::size_t start_index = arity - default_arg_count;

            static_assert(default_arg_count <= arity, "Too many default arguments provided for the function.");
            static_assert(implements::check_default_args_compatibility<paramlist, start_index, Args...>(),
                          "Default arguments are not compatible with corresponding function parameters.");

            using implemented_type = typename implements::get_ia_implement_type<Fx, implements::default_arguments_store<Args...>,
                                                                                type_traits::primary_types::function_traits<Fx>>::type;
            invoke_accessor_ = utility::construct_at(reinterpret_cast<implemented_type *>(invoker_storage),
                                                     utility::forward<Fx>(function), utility::forward<Args>(default_arguments)...);
            if constexpr (sizeof(implemented_type) >= core::fn_obj_soo_buffer_size) {
                invoke_accessor_ =
                    ::new implemented_type(utility::forward<Fx>(function), utility::forward<Args>(default_arguments)...);
            } else {
                invoke_accessor_ = utility::construct_at(reinterpret_cast<implemented_type *>(invoker_storage),
                                                         utility::forward<Fx>(function), utility::forward<Args>(default_arguments)...);
            }
        }

        ~function();

        /**
         * @brief 调用函数，以静态方式，并返回结果。
         * @tparam Args 任意数量的函数参数类型，但是其数量需要与目标调用的参数数量一致。
         * @param args 任意数量的函数实参，需要与目标调用的参数数量一致。
         * @return 函数调用结果，以any形式。
         */
        template <typename... Args>
        RAINY_INLINE utility::any static_invoke(Args &&...args) const {
            return invoke(non_exists_instance, utility::forward<Args>(args)...);
        }

        /**
         * @brief 调用函数，并返回结果。
         * @tparam Args 任意数量的函数参数类型，但是其数量需要与目标调用的参数数量一致。
         * @param instance 对象实例。
         * @param args 任意数量的函数实参，需要与目标调用的参数数量一致。
         * @return 函数调用结果，以any形式。
         */
        template <typename... Args>
        RAINY_INLINE utility::any invoke(object_view instance, Args &&...args) const {
            using namespace foundation::ctti;
#if RAINY_ENABLE_DEBUG
            utility::expects(!empty(), "Cannot call [invoke] method, curent object is empty!");
            if (instance.ctti().is_const()) {
                if (!is_const()) {
                    errno = ECANCELED;
                    return {};
                }
            } else if (instance.ctti().has_traits(traits::is_volatile)) {
                if (!is_volatile()) {
                    errno = ECANCELED;
                    return {};
                }
            } else if (instance.ctti().is_rvalue_reference()) {
                if (!is_invoke_for_rvalue()) {
                    errno = ECANCELED;
                    return {};
                }
            }
#endif
            if constexpr (sizeof...(Args) == 0) {
                return invoke_accessor()->invoke(instance);
            } else {
                return invoke_accessor()->invoke(instance, implements::arg_store{utility::forward<Args>(args)...});
            }
        }

        /**
         * @brief 重载函数调用运算符，以调用函数并返回结果。
         * @tparam Args 任意数量的函数参数类型，但是其数量需要与目标调用的参数数量一致。
         * @param instance 对象实例。
         * @param args 任意数量的函数实参，需要与目标调用的参数数量一致。
         * @return 函数调用结果，以any形式。
         */
        template <typename... Args>
        RAINY_INLINE utility::any operator()(object_view instance, Args &&...args) const {
            return invoke(instance, utility::forward<Args>(args)...);
        }

        /**
         * @brief 检查当前反射函数对象是否为空。
         * @return 如果为空，则返回true；否则返回false。
         */
        RAINY_NODISCARD bool empty() const noexcept;

        /**
         * @brief 从常量左值引用中拷贝函数对象。
         * @param right 从此对象拷贝函数对象。
         */
        void copy_from_other(const function &right) noexcept;

        /**
         * @brief 从右值引用中移动函数对象。
         * @param right 从此对象移动函数对象。
         */
        void move_from_other(function &&right) noexcept;

        /**
         * @brief 交换两个函数对象。
         * @param right 另一个函数对象。
         */
        void swap(function &right) noexcept;

        /**
         * @brief 从常量左值引用中拷贝函数对象。
         * @param right 从此对象拷贝函数对象。
         * @return 返回对象本身的引用。
         */
        function &operator=(const function &right) noexcept;

        /**
         * @brief 通过移动语义转移函数对象。
         * @param right 从此对象移动函数对象。
         * @return 返回对象本身的引用。
         */
        function &operator=(function &&right) noexcept;

        /**
         * @brief 将函数对象设置为空。
         * @return 返回对象本身的引用。
         */
        function &operator=(std::nullptr_t) noexcept;

        /**
         * @brief 获取函数对象的函数签名。
         * @return 返回函数签名。
         */
        RAINY_NODISCARD const foundation::ctti::typeinfo &function_signature() const noexcept;

        /**
         * @brief 获取函数对象的函数类型。
         * @return 返回函数类型。
         */
        RAINY_NODISCARD method_flags type() const noexcept;

        /**
         * @brief 获取函数对象的函数类型。检查是否具有特定的属性
         */
        RAINY_NODISCARD bool has(method_flags flag) const noexcept;

        /**
         * @brief 检查当前函数对象是否有效。
         * @return 如果有效，则返回true；否则返回false。
         */
        explicit operator bool() const noexcept;

        /**
         * @brief 检查当前函数对象是否与另一个函数对象相同。
         * @param right 另一个函数对象。
         * @return 如果相同，则返回true；否则返回false。
         */
        RAINY_NODISCARD bool equal_with(const function &right) const noexcept;

        /**
         * @brief 检查当前函数对象是否与另一个函数对象不同。
         * @param right 另一个函数对象。
         * @return 如果不同，则返回true；否则返回false。
         */
        RAINY_NODISCARD bool not_equal_with(const function &right) const noexcept;

        /**
         * @brief 清空函数对象。
         */
        void reset() noexcept;

        /**
         * @brief 重新绑定函数对象。
         * @param function 待绑定的函数对象。
         * @remark 以移动语义的方式绑定函数对象。
         */
        void rebind(function &&function) noexcept;

        /**
         * @brief 重新绑定函数对象。
         * @param function 待绑定的函数对象。
         * @remark 以拷贝语义的方式绑定函数对象。
         */
        void rebind(const function &function) noexcept;

        /**
         * @brief 重新绑定函数对象。
         * @remark 将函数对象设置为空。
         */
        void rebind(std::nullptr_t) noexcept;

        /**
         * @brief 获取函数对象的所属类。
         * @return 返回函数对象的所属类。
         */
        RAINY_NODISCARD const foundation::ctti::typeinfo &which_belongs() const noexcept;

        /**
         * @brief 获取函数对象的返回类型。
         * @return 返回函数对象的返回类型。
         */
        RAINY_NODISCARD const foundation::ctti::typeinfo &return_type() const noexcept;

        /**
         * @brief 获取函数对象的参数类型列表。
         * @return 返回函数对象的参数类型列表。
         */
        RAINY_NODISCARD const collections::views::array_view<foundation::ctti::typeinfo> &paramlists() const noexcept;

        /**
         * @brief 获取函数对象的所需参数数量。
         * @return 返回函数对象的所需参数数量。
         */
        RAINY_NODISCARD std::size_t arity() const noexcept;

        /**
         * @brief 获取当前function对象中，从0开始索引的参数列表中的某个参数的类型信息
         * @param idx 索引
         * @return 返回参数类型信息
         */
        RAINY_NODISCARD const foundation::ctti::typeinfo &arg(std::size_t idx) const;

        /**
         * @brief 检查当前函数对象是否为静态函数。
         * @return 如果是静态函数，则返回true；否则返回false。
         */
        RAINY_NODISCARD bool is_static() const noexcept;

        /**
         * @brief 检查当前函数对象是否为成员实例函数。
         * @return 如果是成员实例函数，则返回true；否则返回false。
         */
        RAINY_NODISCARD bool is_memfn() const noexcept;

        /**
         * @brief 检查当前函数对象是否为const函数。
         * @return 如果是const函数，则返回true；否则返回false。
         */
        RAINY_NODISCARD bool is_const() const noexcept;

        /**
         * @brief 检查当前函数对象是否为noexcept函数。
         * @return 如果是noexcept函数，则返回true；否则返回false。
         */
        RAINY_NODISCARD bool is_noexcept() const noexcept;

        /**
         * @brief 检查当前函数对象是否为volatile函数。
         * @return 如果是volatile函数，则返回true；否则返回false。
         */
        RAINY_NODISCARD bool is_volatile() const noexcept;

        /**
         * @brief 检查当前函数对象是否支持lvalue调用。
         * @return 如果支持lvalue调用，则返回true；否则返回false。
         */
        RAINY_NODISCARD bool is_invoke_for_lvalue() const noexcept;

        /**
         * @brief 检查当前函数对象是否支持rvalue调用。
         * @return 如果支持rvalue调用，则返回true；否则返回false。
         */
        RAINY_NODISCARD bool is_invoke_for_rvalue() const noexcept;

        /**
         * @brief 检查当前函数对象是否可以调用给定的参数列表。
         * @param paramlist 参数列表。
         * @return 如果可以调用，则返回true；否则返回false。
         */
        RAINY_NODISCARD bool is_invocable(collections::views::array_view<foundation::ctti::typeinfo> paramlist) const noexcept;

        /**
         * @brief 检查当前函数对象是否可以调用给定的参数类型。
         * @tparam Args 参数类型。
         * @return 如果可以调用，则返回true；否则返回false。
         */
        template <typename... Args>
        RAINY_NODISCARD bool is_invocable() const noexcept {
            if constexpr (sizeof...(Args) == 0) {
                return is_invocable();
            } else {
                static collections::array<foundation::ctti::typeinfo, sizeof...(Args)> paramlist = {
                    foundation::ctti::typeinfo::create<Args>()...};
                return is_invocable(paramlist);
            }
        }

        template <typename... Args>
        bool is_invocable_with(Args &&...args) const noexcept {
            implements::make_paramlist paramlist{utility::forward<Args>(args)...};
            return is_invocable(paramlist.get());
        }

        /**
         * @brief 获取函数对象的目标函数指针。
         * @tparam Fx 目标函数类型。
         * @return 返回目标函数指针。
         */
        template <typename Fx>
        RAINY_NODISCARD Fx *target() const noexcept {
            utility::expects(!empty(), "You're trying to get the arg count of a empty object!");
            if constexpr (type_traits::type_relations::is_same_v<Fx, function>) {
                return reinterpret_cast<function *>(invoke_accessor()->target(rainy_typeid(Fx)));
            } else {
                auto ptr = reinterpret_cast<Fx *>(invoke_accessor()->target(rainy_typeid(Fx)));
                return ptr;
            }
        }

        /**
         * @brief 比较两个函数对象是否相等。
         * @param left 左侧函数对象。
         * @param right 右侧函数对象。
         * @return 如果相等，则返回true；否则返回false。
         */
        friend bool operator==(const function &left, const function &right) noexcept {
            return left.equal_with(right);
        }

        /**
         * @brief 比较两个函数对象是否不相等。
         * @param left 左侧函数对象。
         * @param right 右侧函数对象。
         * @return 如果不相等，则返回true；否则返回false。
         */
        friend bool operator!=(const function &left, const function &right) noexcept {
            return left.not_equal_with(right);
        }

        /**
         * @brief 将函数对象的信息输出到流中。
         * @param os 输出流。
         * @param f 函数对象。
         * @return 返回输出流。
         */
        friend std::ostream &operator<<(std::ostream &os, const function &f) {
            if (f.empty()) {
                return os;
            }
            return os << f.function_signature().name() << " -> " << f.target<function *>();
        }

    private:
        bool is_local() const noexcept;

        RAINY_INLINE implements::invoker_accessor *invoke_accessor() const noexcept {
            return invoke_accessor_;
        }

        alignas(std::max_align_t) core::byte_t
            invoker_storage[core::fn_obj_soo_buffer_size]{}; // 不使用std::array/std::aligned_storage
        implements::invoker_accessor *invoke_accessor_{nullptr};
    };

    /**
     * @brief 创建一个反射函数对象。
     * @tparam Fx 函数对象类型。
     * @param fx 函数对象。
     * @return 返回创建的反射函数对象。
     */
    template <typename Fx,
              type_traits::other_trans::enable_if_t<type_traits::type_properties::is_constructible_v<function, Fx>, int> = 0>
    function make_function(Fx &&fx) noexcept {
        return function{utility::forward<Fx>(fx)};
    }

    /**
     * @brief 调用函数对象。
     * @tparam Args 函数参数类型。
     * @param fn 函数对象。
     * @param instance 对象实例。
     * @param args 函数实参。
     * @return 函数调用结果。
     */
    template <typename... Args>
    utility::any invoke(const function &fn, object_view instance, Args &&...args) {
        return fn.invoke(instance, utility::forward<Args>(args)...);
    }
}

namespace rainy::utility {
    RAINY_INLINE void swap(meta::reflection::function &left, meta::reflection::function &right) noexcept {
        left.swap(right);
    }
}

namespace std {
    inline void swap(rainy::meta::reflection::function &left, rainy::meta::reflection::function &right) noexcept { // NOLINT
        left.swap(right);
    }
}

namespace rainy::meta::reflection {
    class method : public function {
    public:
        method() noexcept = default;

        template <typename Fx, typename... Args, std::size_t N = 0,
                  type_traits::other_trans::enable_if_t<type_traits::type_properties::is_constructible_v<function, Fx>, int> = 0>
        static method make(std::string_view name, Fx &&fn, std::tuple<Args...> &default_arguemnts,
                           collections::array<metadata, N> &metadatas) {
            return method{name, fn, default_arguemnts, metadatas, type_traits::helper::index_sequence_for<Args...>{}};
        }

        method(const method &) = delete;
        method &operator=(const method &) = delete;

        method(method &&right) noexcept :
            function(utility::move(right)), name_(utility::move(right.name_)), metadata_(utility::move(right.metadata_)) {
        }

        RAINY_TOOLKIT_API method &operator=(method &&right) noexcept;

        RAINY_TOOLKIT_API void rebind(method &&right) noexcept;

        RAINY_TOOLKIT_API void rebind(std::nullptr_t) noexcept;

        RAINY_TOOLKIT_API void swap(method &right) noexcept;

        RAINY_TOOLKIT_API std::string_view name() const noexcept;

        RAINY_TOOLKIT_API const metadata &get_metadata(const std::string_view key) const noexcept;

        RAINY_TOOLKIT_API const std::unordered_map<std::string_view, reflection::metadata> &metadatas() const noexcept;

    private:
        template <typename Fx, typename... Args, std::size_t N = 0, std::size_t... I,
                  type_traits::other_trans::enable_if_t<type_traits::type_properties::is_constructible_v<function, Fx>, int> = 0>
        method(std::string_view name, Fx &&fn, std::tuple<Args...> &default_arguemnts, collections::array<metadata, N> &metadatas,
               type_traits::helper::index_sequence<I...>) noexcept :
            function(utility::forward<Fx>(fn), std::get<I>(default_arguemnts)...), name_(utility::move(name)), metadata_{} {
            if constexpr (N != 0) {
                for (metadata &meta: metadatas) {
                    std::string_view name = meta.key();
                    this->metadata_.emplace(name, utility::move(meta));
                }
            }
        }

        std::string_view name_;
        std::unordered_map<std::string_view, reflection::metadata> metadata_;
    };
}

#endif
