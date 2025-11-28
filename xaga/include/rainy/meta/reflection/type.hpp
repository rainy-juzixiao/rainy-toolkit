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
#ifndef RAINY_META_REFLECTION_TYPE_HPP
#define RAINY_META_REFLECTION_TYPE_HPP
#include <rainy/collections/string.hpp>
#include <rainy/foundation/typeinfo.hpp>
#include <rainy/meta/reflection/function.hpp>
#include <rainy/meta/reflection/property.hpp>
#include <rainy/utility/iterator.hpp>
#include <rainy/meta/reflection/refl_impl/type_register.hpp>
#include <unordered_map>

/*
Public Member Functions
    type (const type &other) noexcept
variant 	create (std::vector< argument > args=std::vector< argument >()) const
bool 	destroy (variant &obj) const noexcept
array_range< type > 	get_base_classes () const noexcept
constructor 	get_constructor (const std::vector< type > &params=std::vector< type >()) const noexcept
array_range< constructor > 	get_constructors () const noexcept
array_range< constructor > 	get_constructors (filter_items filter) const noexcept
array_range< type > 	get_derived_classes () const noexcept
destructor 	get_destructor () const noexcept
enumeration 	get_enumeration () const noexcept
type_id 	get_id () const noexcept
variant 	get_metadata (const variant &key) const
method 	get_method (string_view name) const noexcept
method 	get_method (string_view name, const std::vector< type > &type_list) const noexcept
array_range< method > 	get_methods () const noexcept
array_range< method > 	get_methods (filter_items filter) const noexcept
string_view 	get_name () const noexcept
array_range< property > 	get_properties () const noexcept
array_range< property > 	get_properties (filter_items filter) const noexcept
property 	get_property (string_view name) const noexcept
variant 	get_property_value (string_view name, instance obj) const
type 	get_raw_type () const noexcept
std::size_t 	get_sizeof () const noexcept
array_range< type > 	get_template_arguments () const noexcept
type 	get_wrapped_type () const noexcept
variant 	invoke (string_view name, instance obj, std::vector< argument > args) const
bool 	is_arithmetic () const noexcept
bool 	is_array () const noexcept
bool 	is_associative_container () const noexcept
bool 	is_base_of (const type &other) const noexcept
bool 	is_base_of () const noexcept
bool 	is_class () const noexcept
bool 	is_derived_from (const type &other) const noexcept
bool 	is_derived_from () const noexcept
bool 	is_enumeration () const noexcept
bool 	is_function_pointer () const noexcept
bool 	is_member_function_pointer () const noexcept
bool 	is_member_object_pointer () const noexcept
bool 	is_pointer () const noexcept
bool 	is_sequential_container () const noexcept
bool 	is_template_instantiation () const noexcept
bool 	is_valid () const noexcept
bool 	is_wrapper () const noexcept
    operator bool () const noexcept
bool 	operator!= (const type &other) const noexcept
bool 	operator< (const type &other) const noexcept
bool 	operator<= (const type &other) const noexcept
type & 	operator= (const type &other) noexcept
bool 	operator== (const type &other) const noexcept
bool 	operator> (const type &other) const noexcept
bool 	operator>= (const type &other) const noexcept
bool 	set_property_value (string_view name, instance obj, argument arg) const
Static Public Member Functions
static type 	get () noexcept
static type 	get (T &&object) noexcept
static type 	get_by_name (string_view name) noexcept
static method 	get_global_method (string_view name) noexcept
static method 	get_global_method (string_view name, const std::vector< type > &params) noexcept
static array_range< method > 	get_global_methods () noexcept
static array_range< property > 	get_global_properties () noexcept
static property 	get_global_property (string_view name) noexcept
static variant 	get_property_value (string_view name)
static array_range< type > 	get_types () noexcept
static variant 	invoke (string_view name, std::vector< argument > args)
static void 	register_comparators ()
static void 	register_converter_func (F func)
static void 	register_equal_comparator ()
static void 	register_less_than_comparator ()
static void 	register_wrapper_converter_for_base_classes ()
static bool 	set_property_value (string_view name, argument arg)
*/

namespace rainy::meta::reflection {
    class type;
}

namespace rainy::meta::reflection::implements {
    using method_storage_t = std::unordered_multimap<std::string_view, method>;
    using property_storage_t = std::unordered_map<std::string_view, property>;
    using ctor_storage_t = std::vector<constructor>;
}

namespace rainy::meta::reflection::implements {
    template <typename TypeList>
    struct template_argument_generater {};

    template <typename... Types>
    struct template_argument_generater<type_traits::other_trans::type_list<Types...>> {
        static collections::views::array_view<foundation::ctti::typeinfo> get() noexcept {
            static collections::array<foundation::ctti::typeinfo, sizeof...(Types)> list = {
                foundation::ctti::typeinfo::create<Types>()...};
            return list;
        }
    };

    class dyn_typeinfo final : public foundation::ctti::typeinfo {
    public:
        using base = foundation::ctti::typeinfo;

        template <typename Ty>
        dyn_typeinfo(std::in_place_type_t<Ty>) : base{base::create<Ty>()} {
        }

        template <typename Ty>
        static dyn_typeinfo create() noexcept {
            dyn_typeinfo info{std::in_place_type<Ty>};
            if constexpr (type_traits::type_relations::is_void_v<Ty>) {
                info.sizeof_ = 0;
            } else {
                info.sizeof_ = sizeof(Ty);
            }
            if constexpr (type_traits::primary_types::template_traits<type_traits::cv_modify::remove_cvref_t<Ty>>::value) {
                info.template_arguemnts_ = template_argument_generater<
                    typename type_traits::primary_types::template_traits<type_traits::cv_modify::remove_cvref_t<Ty>>::types>::get();
            }
            return info;
        }

        RAINY_NODISCARD std::size_t get_sizeof() const noexcept {
            return sizeof_;
        }

        RAINY_NODISCARD collections::views::array_view<foundation::ctti::typeinfo> template_arguemnts() const noexcept {
            return template_arguemnts_;
        }

    private:
        dyn_typeinfo() noexcept = default;

        collections::views::array_view<foundation::ctti::typeinfo> template_arguemnts_;
        std::size_t sizeof_{0};
    };
}

namespace rainy::meta::reflection::implements {
    struct type_accessor {
        virtual ~type_accessor() = default;
        /* 类型名称标记 */
        RAINY_NODISCARD virtual std::string_view name() const noexcept = 0;
        /* 类型信息 */
        virtual dyn_typeinfo &typeinfo() noexcept = 0;
        /* 方法、静态方法、重载运算符 */
        virtual method_storage_t &methods() noexcept = 0;
        /* 构造函数集合 */
        virtual ctor_storage_t &ctors() noexcept = 0;
        /* 属性、静态属性 */
        virtual std::unordered_map<std::string_view, property> &properties() noexcept = 0;
        /* 基类集合 */
        virtual std::unordered_map<std::string_view, type> &bases() noexcept = 0;
        /* 派生类集合，用于未来设计dynamic_cast */
        virtual std::unordered_map<std::string_view, type> &deriveds() noexcept = 0;
        // 基类/派生类返回的是一张表，其中的指针指向位于反射系统内部的实例（其实是静态示例）
        // const在此部分非一等公民，由面向用户的接口进行const属性添加
        // 不考虑过多性能
        virtual std::vector<metadata>& metadatas() noexcept = 0;
    };
}

namespace rainy::meta::reflection {
    /**
     * @brief 用于反射的类型
     * @attention 若需要使用，请确保类型已被注册
     * @remarks type对象是轻量级的，可以按值传递
     */
    class RAINY_TOOLKIT_API type {
    public:
        using type_id = std::size_t;
        using methods_view_t = collections::views::iterator_range<utility::map_mapped_iterator<implements::method_storage_t>>;
        using property_view_t = collections::views::iterator_range<utility::map_mapped_iterator<implements::property_storage_t>>;
        using base_classes_view_t = collections::views::iterator_range<utility::map_mapped_iterator<std::unordered_map<std::string_view, type>>>;
        using derived_classes_view_t = collections::views::iterator_range<utility::map_mapped_iterator<std::unordered_map<std::string_view, type>>>;
        using constcutor_view_t = collections::views::array_view<constructor>;

        /**
         * @brief 默认构造函数，无作用.
         */
        type() noexcept = default;


        /**
         * @brief 默认析构函数，无作用.
         */
        ~type() = default;

        type(const type &) = default;
        type(type &&) = default;

        type &operator=(const type &) = default;
        type &operator=(type &&) = default;

        /**
         * @brief 从Ty类型获取对应的反射类型对象
         * @tparam Type 要获取反射类型的类型
         * @return 返回对应的反射类型对象
         */
        template <typename Type>
        static type get() noexcept {
            type instance{};
            instance.accessor = implements::register_table::get_accessor(rainy_typeid(Type));
            return instance;
        }

        /**
         * @brief 从类型名称获取对应的反射类型对象
         * @param name 要获取反射类型的类型名称
         * @return 返回对应的反射类型对象
         */
        static type get_by_name(std::string_view name) noexcept;

        /**
         * @brief 从类型信息获取对应的反射类型对象
         * @param typeinfo 要获取反射类型的类型信息
         * @return 返回对应的反射类型对象
         */
        RAINY_NODISCARD std::string_view get_name() const noexcept;

        /**
         * @brief 获取类型的唯一标识符
         * @return 返回类型的唯一标识符
         */
        RAINY_NODISCARD type_id get_id() const noexcept;

        /**
         * @brief 获取对应类型的大小
         * @return 返回对应类型的大小
         */
        RAINY_NODISCARD std::size_t get_sizeof() const noexcept;

        /**
         * @brief 获取模板参数列表
         * @return 返回对应模板参数列表
         */
        RAINY_NODISCARD collections::views::array_view<foundation::ctti::typeinfo> get_template_arguments() const noexcept;

        /**
         * @brief 根据名称，获取指定的方法反射对象
         * @param name 要获取的方法名称
         * @attention 如果目标方法名称存在多个反射对象，则优先选择第一个匹配到的反射函数对象
         * @return 如果找到对应的方法，返回方法反射对象的常量引用，否则返回一个无效的空对象引用
         */
        RAINY_NODISCARD const method &get_method(const std::string_view name) const noexcept;

        /**
         * @brief 根据名称和参数列表，获取指定的方法反射对象
         * @param name 要获取的方法名称
         * @param overload_version_paramlist 要获取的方法的筛选参数类型列表 
         * @param filter_item 筛选的flag项
         * @return 如果找到对应的方法，返回方法反射对象的常量引用，否则返回一个无效的空对象引用
         */
        RAINY_NODISCARD const method &get_method(
            const std::string_view name, const collections::views::array_view<foundation::ctti::typeinfo> overload_version_paramlist,
            const method_flags filter_item = method_flags::none) const noexcept;

        /**
         * @brief 获取类型的所有方法反射对象
         * @return 返回类型的所有方法反射对象的视图
         */
        RAINY_NODISCARD methods_view_t get_methods() const;

        /**
         * @brief 获取类型的指定名称的属性反射对象
         * @param name 要获取的属性名称
         * @return 如果找到对应的属性，返回属性反射对象的常量引用，否则返回一个无效的空对象引用
         */
        RAINY_NODISCARD const property &get_property(const std::string_view name) const noexcept;

        /**
         * @brief 获取类型的所有属性反射对象
         * @return name 返回类型的所有属性反射对象的视图
         */
        RAINY_NODISCARD property_view_t get_properties() const noexcept;

        /**
         * @brief 获取类型的所有构造函数反射对象
         * @return 返回类型的所有构造函数反射对象的视图
         */
        RAINY_NODISCARD type::constcutor_view_t get_construtors() const noexcept;

        /**
         * @brief 根据参数列表，获取指定的构造函数反射对象
         * @param overload_version_paramlist 
         * @return 如果找到对应的构造函数，返回构造函数反射对象的常量引用，否则返回一个无效的空对象引用
         */
        RAINY_NODISCARD const constructor &get_construtor(
            const collections::views::array_view<foundation::ctti::typeinfo> overload_version_paramlist) const noexcept;
        
        /**
         * @brief 获取基类反射类型对象
         * @return 返回类型的所有基类反射类型对象的映射表视图
         */
        RAINY_NODISCARD base_classes_view_t get_base_classes() const noexcept;

        /**
         * @brief 获取基类反射类型对象
         * @return 返回类型的所有基类反射类型对象的映射表视图
         */
        RAINY_NODISCARD derived_classes_view_t get_derived_classes() const noexcept;

        /**
         * @brief 判断当前类型是否为指定类型的基类
         * @param typeinfo 类型信息对象
         * @attention typeinfo必须是已注册类型的信息，且已绑定到继承树，否则将始终返回false
         * @return 如果是基类，返回true，否则返回false
         */
        bool is_base_of(annotations::lifetime::in<foundation::ctti::typeinfo> typeinfo) const noexcept;

        /**
         * @brief 判断当前类型是否为指定类型的基类
         * @param type 反射类型信息对象
         * @return 如果是基类，返回true，否则返回false
         */
        bool is_base_of(annotations::lifetime::in<type> type) const noexcept;
        
        /**
         * @brief 判断当前类型是否为指定类型的基类
         * @tparam Type 要测试的类型
         * @return 如果是基类，返回true，否则返回false
         */
        template <typename Type>
        bool is_base_of() const noexcept {
            return is_base_of(rainy_typeid(Type));
        }
    
        /**
         * @brief 判断当前类型是否为指定类型的派生类
         * @param typeinfo 类型信息对象
         * @attention typeinfo必须是已注册类型的信息，且已绑定到继承树，否则将始终返回false
         * @return 如果是派生类，返回true，否则返回false
         */
        bool is_derived_from(annotations::lifetime::in<foundation::ctti::typeinfo> typeinfo) const noexcept;
        
        /**
         * @brief 判断当前类型是否为指定类型的派生类
         * @param type 反射类型信息对象
         * @return 如果是派生类，返回true，否则返回false
         */
        bool is_derived_from(annotations::lifetime::in<type> type) const noexcept;

        /**
         * @brief 判断当前类型是否为指定类型的派生类
         * @tparam Type 要测试的类型
         * @return 如果是派生类，返回true，否则返回false
         */
        template <typename Type>
        bool is_derived_from() const noexcept {
            return is_derived_from(rainy_typeid(Type));
        }

        /**
         * @brief 获取类型信息对象
         * @return 返回类型信息对象的常量引用
         */
        RAINY_NODISCARD const foundation::ctti::typeinfo &get_typeinfo() noexcept;

        /**
         * @brief 获取元数据信息
         * @return 如果指定的key与注册数据匹配，则获得对应的metadata，否则返回空metadata
         */
        RAINY_NODISCARD const metadata &get_metadata(const utility::any &key) const noexcept;

        /**
         * @brief 检查是否包含指定名称的方法
         * @param name 要检查的方法名称
         * @return 如果包含该方法，返回true，否则返回false
         */
        bool has_method(std::string_view name) const noexcept;

        /**
         * @brief 检查是否包含指定名称的属性
         * @param name 要检查的属性名称
         * @return 如果包含该属性，返回true，否则返回false
         */
        bool has_property(std::string_view name) const noexcept;

        template <typename... Args>
        utility::any create(Args &&...args) const {
            for (const auto &item: accessor->ctors()) {
                const function &cur_ctor = item;
                bool invocable{};
                constexpr bool has_dynamic =
                    type_traits::type_relations::is_any_of_v<utility::any, type_traits::other_trans::decay_t<Args>...> ||
                    type_traits::type_relations::is_any_of_v<object_view, type_traits::other_trans::decay_t<Args>...>;
                if constexpr (has_dynamic) {
                    invocable = cur_ctor.is_invocable_with(utility::forward<Args>(args)...);
                } else {
                    static implements::make_nondynamic_paramlist<Args...> paramlist;
                    invocable = cur_ctor.is_invocable(paramlist.get());
                }
                if (invocable) {
                    return cur_ctor.static_invoke(utility::forward<Args>(args)...);
                }
            }
            return {};
        }

        utility::any create_object(collections::views::array_view<utility::any> args) const;

        /**
         * @brief 检查当前反射类型对象是否有效
         * @return 如果有效，返回true，否则返回false
         */
        RAINY_NODISCARD bool is_valid() const noexcept;

        template <typename... Args>
        utility::any invoke_method(std::string_view name, object_view instance, Args &&...args) const {
            using namespace foundation::ctti;
            auto flag = method_flags::none;
            if (instance.type().has_traits(traits::is_const)) {
                flag = flag | method_flags::const_qualified;
            }
            if (instance.type().has_traits(traits::is_volatile)) {
                flag = flag | method_flags::volatile_qualified;
            }
            if (instance.type().has_traits(traits::is_rref)) {
                flag = flag | method_flags::rvalue_qualified;
            }
            return invoke_method(flag, name, instance, utility::forward<Args>(args)...);
        }

        template <typename... Args>
        utility::any invoke_method(method_flags flag, std::string_view name, object_view instance, Args &&...args) const {
            using namespace type_traits::other_trans;
            using namespace type_traits::type_relations;
            using namespace foundation::ctti;
            const method *invoker{nullptr};
            if constexpr (is_any_of_v<utility::any, decay_t<Args>...> || is_any_of_v<object_view, decay_t<Args>...>) {
                implements::make_paramlist paramlist{utility::forward<Args>(args)...};
                invoker = &get_method(name, paramlist, flag);
            } else {
                static collections::array<foundation::ctti::typeinfo, sizeof...(Args)> paramlist = {
                    foundation::ctti::typeinfo::create<Args>()...};
                invoker = &get_method(name, paramlist, flag);
            }
            if (invoker->empty()) {
                errno = EINVAL;
                return {};
            }
            if (invoker->is_static()) {
                return invoker->static_invoke(utility::forward<Args>(args)...);
            }
            return invoker->invoke(instance, utility::forward<Args>(args)...);
        }

    private:
        implements::type_accessor *accessor{nullptr};
    };
}

namespace rainy::meta::reflection::implements{
    template <typename Type>
    class type_accessor_impl_class final : public type_accessor {
    public:
        explicit type_accessor_impl_class(const std::string_view name) noexcept : name_(name), typeinfo_(dyn_typeinfo::create<Type>()) {
        }

        RAINY_NODISCARD std::string_view name() const noexcept override {
            return name_;
        }

        dyn_typeinfo &typeinfo() noexcept override {
            return typeinfo_;
        }

        method_storage_t &methods() noexcept override {
            return methods_;
        }

        ctor_storage_t &ctors() noexcept override {
            return ctors_;
        }

        std::unordered_map<std::string_view, property> &properties() noexcept override {
            return properties_;
        }

        std::unordered_map<std::string_view, type> &bases() noexcept override {
            return bases_;
        }

        std::unordered_map<std::string_view, type> &deriveds() noexcept override {
            return deriveds_;
        }

        std::vector<metadata> &metadatas() noexcept override {
            return metadatas_;
        }

    private:
        std::string_view name_;
        dyn_typeinfo typeinfo_;
        method_storage_t methods_;
        ctor_storage_t ctors_;
        std::unordered_map<std::string_view, property> properties_;
        std::unordered_map<std::string_view, type> bases_;
        std::unordered_map<std::string_view, type> deriveds_;
        std::vector<metadata> metadatas_;
    };
}

namespace rainy::meta::reflection::implements {
    template <typename Type, typename... Args>
    constexpr auto make_ctor_name() {
        using namespace foundation::ctti;
        constexpr auto type_str = type_name<Type>();
        if constexpr (sizeof...(Args) == 0) {
            constexpr std::size_t total_len = type_str.size() + 2 + 1; // "Type()" + '\0'
            type_traits::helper::constexpr_string<total_len> result{};
            std::size_t pos = 0;
            for (char c: type_str) {
                result[pos++] = c;
            }
            result[pos++] = '(';
            result[pos++] = ')';
            result[pos++] = '\0';
            return result;
        } else {
            constexpr collections::array<std::string_view, sizeof...(Args)> arg_names = {type_name<Args>()...};
            constexpr std::size_t args_len = [&arg_names] {
                std::size_t len = 0;
                for (auto &arg: arg_names) {
                    len += arg.size();
                }
                len += 2 * (sizeof...(Args) - 1); // ", "
                return len;
            }();
            constexpr std::size_t total_len = type_str.size() + 1 + args_len + 1 + 1; // '(' + args + ')' + '\0'
            type_traits::helper::constexpr_string<total_len> result{};
            std::size_t pos = 0;
            auto append = [&](const std::string_view &s) {
                for (char c: s) {
                    result[pos++] = c;
                }
            };
            append(type_str);
            result[pos++] = '(';
            for (std::size_t i = 0; i < arg_names.size(); ++i) {
                append(arg_names[i]);
                if (i < arg_names.size() - 1) {
                    result[pos++] = ',';
                    result[pos++] = ' ';
                }
            }
            result[pos++] = ')';
            result[pos++] = '\0';
            return result;
        }
    }

    RAINY_INLINE std::size_t eval_hash_from_paramlist(collections::views::array_view<utility::any> view) {
        return core::accumulate(view.begin(), view.end(), std::size_t{0},
                                [right = std::size_t{1}](const std::size_t acc, const utility::any &arg) mutable {
                                    return acc + (arg.type().hash_code() * right++);
                                });
    }
}

#endif