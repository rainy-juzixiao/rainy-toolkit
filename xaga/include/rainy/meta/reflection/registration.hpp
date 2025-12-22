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
#ifndef RAINY_META_REFLECTION_REGISTRATION_HPP
#define RAINY_META_REFLECTION_REGISTRATION_HPP
#include <rainy/meta/moon/enumeration.hpp>
#include <rainy/meta/reflection/metadata.hpp>
#include <rainy/meta/reflection/moon/reflect.hpp>
#include <rainy/meta/reflection/type.hpp>
#include <rainy/utility/arg_extractor.hpp>
#include <string_view>

namespace rainy::meta::reflection::implements {
    RAINY_TOOLKIT_API bool check_method_field(type_accessor *type, std::string_view name, method &meth);
    RAINY_TOOLKIT_API bool check_ctor_field(type_accessor *type, constructor &ctor);
    RAINY_TOOLKIT_API void register_method_helper(type_accessor *type, std::string_view name, method &&meth);
    RAINY_TOOLKIT_API type_accessor *do_inject(std::once_flag &consume, std::string_view name, type_accessor *accessor);
    RAINY_TOOLKIT_API type_accessor* global_type_accessor();

    template <typename Type>
    static type_accessor *new_type_accessor_instance(std::string_view name) noexcept {
        static std::once_flag flag;
        using impl_t = type_traits::other_trans::conditional_t<
            type_traits::primary_types::is_enum_v<Type>, type_accessor_impl_enumeration<Type>,
            type_traits::other_trans::conditional_t<type_traits::composite_types::is_fundamental_v<Type>,
                                                    type_accessor_impl_fundmental_type<Type>, type_accessor_impl_class<Type>>>;
        static impl_t instance{name};
        return do_inject(flag, name, &instance);
    }
}

namespace rainy::meta::reflection::implements {
    template <typename Ty>
    struct is_tuple : type_traits::helper::false_type {};

    template <typename... Ts>
    struct is_tuple<std::tuple<Ts...>> : type_traits::helper::true_type {};

    template <typename... Args>
    struct extract_unique_tuple;

    template <>
    struct extract_unique_tuple<> {
        using type = void;
        static constexpr bool valid = false;
    };

    template <typename T, typename... Rest>
    struct extract_unique_tuple<T, Rest...> {
        using next = extract_unique_tuple<Rest...>;

        static constexpr bool current_is_tuple = is_tuple<std::decay_t<T>>::value;
        static constexpr bool valid = (current_is_tuple && !next::valid) || (!current_is_tuple && next::valid);

        using type =
            type_traits::other_trans::conditional_t<current_is_tuple, type_traits::other_trans::decay_t<T>, typename next::type>;
    };

    template <typename TupleType, typename First, typename... Rest>
    decltype(auto) extract_tuple_from_args(First &&first, Rest &&...rest) {
        using FirstClean = std::remove_reference_t<First>;
        using TupleClean = std::remove_reference_t<TupleType>;

        if constexpr (std::is_same_v<FirstClean, TupleClean>) {
            return static_cast<TupleType>(first); // 保持原类型返回
        } else {
            return extract_tuple_from_args<TupleType>(std::forward<Rest>(rest)...);
        }
    }
}

namespace rainy::meta::reflection {
    /**
     * @brief registration是一个类似于RTTR库注册的机制，负责提供基本的写法兼容性
     */
    class registration {
    public:
        template <typename... Types>
        class bind;

        template <typename Type>
        class class_ {
        public:
            using type_accessor = implements::type_accessor;

            explicit class_(const std::string_view name = rainy_typeid(Type).name()) :
                type(implements::new_type_accessor_instance<Type>(name)) {
            }

            explicit class_(core::internal_construct_tag_t, type_accessor *type) : type(type) {
            }

            template <typename... Args>
            class_ &operator()(Args &&...args) {
                static constexpr std::size_t metadata_count = implements::metadata_count<Args...>;
                collections::array<metadata, metadata_count> metadatas =
                    utility::extract_args_to_array<metadata>(utility::forward<Args>(args)...);
                if constexpr (metadata_count != 0) {
                    for (auto &&item: metadatas) {
                        type->metadatas().emplace_back(utility::move(item));
                    }
                }
                return *this;
            }

            class_ &reflect_moon() {
                using namespace moon::implements;
                using reflect = moon::reflect<Type>;
                if constexpr (has_ctors<Type>) {
                    // reflect::ctors.for_each([this](auto pack) {
                    //     /*std::unordered_map<std::string_view, utility::any> metadata;
                    //     if constexpr (!type_traits::type_relations::is_same_v<decltype(pack.metas), moon::meta_list<>>) {
                    //         pack.metas.for_each([&metadata](auto meta) {
                    //             metadata.emplace(meta.name, type_traits::other_trans::decay_t<decltype(meta.value)>{meta.value});
                    //         });
                    //     }
                    //     this->constructor(pack.value, utility::move(metadata));*/
                    // });
                }
                if constexpr (has_methods<Type>) {
                    reflect::methods.for_each([this](auto pack) {
                        /*std::unordered_map<std::string_view, utility::any> metadata;
                        if constexpr (!type_traits::type_relations::is_same_v<decltype(pack.metas), moon::meta_list<>>) {
                            pack.metas.for_each([&metadata](auto meta) {
                                metadata.emplace(meta.name, type_traits::other_trans::decay_t<decltype(meta.value)>{meta.value});
                            });
                        }
                        this->method(pack.name, pack.value, utility::move(metadata));*/
                    });
                }
                if constexpr (has_properties<Type>) {
                    /*reflect::properties.for_each([this](auto pack) {
                        std::unordered_map<std::string_view, utility::any> metadata;
                        if constexpr (!type_traits::type_relations::is_same_v<decltype(pack.metas), moon::meta_list<>>) {
                            pack.metas.for_each([&metadata](auto meta) {
                                metadata.emplace(meta.name, type_traits::other_trans::decay_t<decltype(meta.value)>{meta.value});
                            });
                        }
                        this->property(pack.name, pack.value, utility::move(metadata));
                    });*/
                }
                return *this;
            }

            /**
             * @brief 向当前类注册成员函数或静态函数
             * @tparam Fx 函数对象类型
             */
            template <typename Fx,
                      type_traits::other_trans::enable_if_t<type_traits::type_properties::is_constructible_v<function, Fx>, int> = 0>
            bind<reflection::method, Type, Fx> method(std::string_view name, Fx &&fn) {
                return bind<reflection::method, Type, Fx>{this->type, name, utility::forward<Fx>(fn)};
            }

            /**
             * @brief 向当前类注册属性或静态属性
             * @tparam Property 属性指针类型，可以是成员指针或静态指针
             * @param name 属性名称
             * @param property 属性指针
             * @return 当前注册器实例的引用
             */
            template <typename Property,
                      type_traits::other_trans::enable_if_t<type_traits::primary_types::is_pointer_v<Property> ||
                                                                (type_traits::primary_types::is_member_object_pointer_v<Property>),
                                                            int> = 0>
            bind<property, Type, Property> property(std::string_view name, Property &&property) {
                return bind<reflection::property, Type, Property>{this->type, name, utility::forward<Property>(property)};
            }

            /**
             * @brief 向当前类注册构造函数
             * @tparam Args 构造函数参数类型列表
             * @return 一个绑定构造函数的注册器对象
             */
            template <typename... Args,
                      type_traits::other_trans::enable_if_t<type_traits::type_properties::is_constructible_v<Type, Args...>, int> = 0>
            bind<constructor, Type, Type (*)(Args...)> constructor() {
                static constexpr auto name_var = implements::make_ctor_name<Type, Args...>();
                std::string_view name{name_var.data(), name_var.size()};
                return bind<reflection::constructor, Type, Type (*)(Args...)>{this->type, name, utility::get_ctor_fn<Type, Args...>()};
            }

            /**
             * @brief 向当前类注册拷贝构造函数
             * @return 一个绑定构造函数的注册器对象
             * @attention 当前Type必须支持拷贝操作，否则此函数将被禁用
             */
            template <typename UTy = Type,type_traits::other_trans::enable_if_t<type_traits::type_properties::is_copy_constructible_v<UTy>, int> = 0>
            bind<reflection::constructor, Type, UTy (*)(const UTy &)> copy_constructor() {
                static constexpr auto name_var = implements::make_ctor_name<Type, const UTy &>();
                std::string_view name{name_var.data(), name_var.size()};
                return bind<reflection::constructor, Type, UTy (*)(const UTy &)>{this->type, name,
                                                                                 utility::get_ctor_fn<Type, const UTy &>()};
            }

            /**
             * @brief 向当前类注册移动构造函数
             * @return 一个绑定构造函数的注册器对象
             * @attention 当前Type必须支持拷贝操作，否则此函数将被禁用
             */
            template <typename UTy = Type,
                      type_traits::other_trans::enable_if_t<type_traits::type_properties::is_copy_constructible_v<UTy>, int> = 0>
            bind<reflection::constructor, Type, UTy (*)(UTy &&)> move_constructor() {
                static constexpr auto name_var = implements::make_ctor_name<Type, UTy &&>();
                std::string_view name{name_var.data(), name_var.size()};
                return bind<reflection::constructor, Type, UTy (*)(UTy &&)>{this->type, name,
                                                                                 utility::get_ctor_fn<Type, UTy &&>()};
            }

            /**
             * @brief 向当前类注册构造函数
             * @tparam Fx 构造函数类型
             * @param fn 构造函数对象
             * @return 一个绑定构造函数的注册器对象
             * @attention Fx必须是一个可调用对象，且不可为成员函数指针
             */
            template <typename Fx, type_traits::other_trans::enable_if_t<
                                       type_traits::type_properties::is_constructible_v<reflection::function, Fx> &&
                                           !type_traits::primary_types::is_member_function_pointer_v<Fx>,
                                       int> = 0>
            bind<reflection::method, Type, Fx> constructor(Fx &&fn) {
                return bind<reflection::method, Type, Fx>{this->type, rainy_typeid(Fx).name(), utility::forward<Fx>(fn)};
            }

            template <typename Enum>
            bind<reflection::enumeration, Type, Enum> enumeration(std::string_view name) {
                auto accessor = implements::new_type_accessor_instance<Enum>(name);
                return bind<reflection::enumeration, Type, Enum>{accessor, name, this->type};
            }

            /**
             * @brief 向当前类注册基类
             * @tparam Base 基类的类型，要求Base必须是目标类型的基类
             * @param name Base类的名称，默认为类型名称，由CTTI生成
             * @param reflect_moon 保留参数，暂未使用（未来用于自动反射基类的moon信息）
             * @return 当前注册器实例的引用
             */
            template <typename Base,
                      type_traits::other_trans::enable_if_t<type_traits::type_relations::is_base_of_v<Base, Type>, int> = 0>
            class_ &base(std::string_view name = rainy_typeid(Base).name(), bool reflect_moon = false) { // NOLINT
                if (auto &bases = this->type->bases(); bases.find(rainy_typeid(Base)) == bases.end()) { // NOLINT
                    if (!implements::register_table::has_register(rainy_typeid(Base))) {
                        class_<Base>(name).template derive<Type>(type->name()); // 注册一个新类型，并将该类直接注入到表中
                    }
                    foundation::ctti::register_base<Type, Base>();
                    bases.emplace(rainy_typeid(Base), type::get<Base>());
                }
                return *this;
            }

            /**
             * @brief 向当前类注册派生类
             * @tparam Derive 派生类的类型，要求Derive必须是目标类型的派生类
             * @param name Derive类的名称，默认为类型名称，由CTTI生成
             * @param reflect_moon 保留参数，暂未使用（未来用于自动反射派生类的moon信息）
             * @return 当前注册器实例的引用
             */
            template <typename Derive,
                      type_traits::other_trans::enable_if_t<type_traits::type_relations::is_base_of_v<Type, Derive>, int> = 0>
            class_ &derive(std::string_view name = rainy_typeid(Derive).name(), bool reflect_moon = false) { // NOLINT
                if (auto &deriveds = type->deriveds(); deriveds.find(rainy_typeid(Derive)) == deriveds.end()) { // NOLINT
                    if (!implements::register_table::has_register(rainy_typeid(Derive))) {
                        class_<Derive>(name).template base<Type>(type->name()); // 注册一个新类型，并将该类直接注入到表中
                    }
                    foundation::ctti::register_base<Derive, Type>();
                    deriveds.emplace(rainy_typeid(Derive), type::get<Derive>());
                }
                return *this;
            }

        private:
            type_accessor *type;
        };

        template <typename Enum>
        static rain_fn enumeration(std::string_view name) -> bind<reflection::enumeration, void, Enum> {
            auto accessor = implements::new_type_accessor_instance<Enum>(name);
            return bind<reflection::enumeration, void, Enum>{accessor, name, implements::global_type_accessor()};
        }

        template <typename Type>
        static rain_fn fundamental(std::string_view name) -> bind<reflection::fundmental, void, Type> {
            auto accessor = implements::new_type_accessor_instance<Type>(name);
            return bind<fundmental, void, Type>{accessor, name, implements::global_type_accessor()};
        }

        template <typename Fx>
        static rain_fn method(std::string_view name, Fx &&fn) -> bind<reflection::method, utility::invalid_type, Fx> {
            return bind<reflection::method, utility::invalid_type, Fx>{implements::global_type_accessor(), name,
                                                                       utility::forward<Fx>(fn)};
        }

    protected:
        registration() = default;
        registration(core::internal_construct_tag_t, implements::type_accessor *) {
        }
    };
}

#define RAINY_REFLECTION_REGISTRATION                                                                                                 \
    static void rainytoolkit_auto_register_reflection_function();                                                                     \
    struct rainytoolkit_auto_register_reflection_ {                                                                                   \
        rainytoolkit_auto_register_reflection_() {                                                                                    \
            rainytoolkit_auto_register_reflection_function();                                                                         \
        }                                                                                                                             \
    };                                                                                                                                \
    static const rainytoolkit_auto_register_reflection_ rainy_toolkit_auto_register;                                                  \
    static void rainytoolkit_auto_register_reflection_function()

namespace rainy::annotations::runtime_assembly {
    template <typename Ty>
    class reflect_lunar_for_class {
    public:
        virtual ~reflect_lunar_for_class() = default;
        using lunar_current_register_type = Ty;

    private:
        struct reflect_register {
            reflect_register() {
                meta::reflection::registration::class_<Ty> instance(Ty::impl_get_reflect_lunar_name());
                Ty::impl_reflect_lunar_for_class(instance);
            }
        };

        virtual void *rainy_toolkit_touch_register() {
            return &register_; // 避免TU链接剪切优化和ODR
        }

        static reflect_register register_;
    };

    template <typename Ty>
    typename reflect_lunar_for_class<Ty>::reflect_register reflect_lunar_for_class<Ty>::register_;
}

namespace rainy::meta::reflection {
    using annotations::runtime_assembly::reflect_lunar_for_class;
}

/**
 * @brief 用于对类进行侵入式注册
 * @param name 要侵入的类的名称
 * @param reg_param 注册器名称
 */
#define RAINY_INTRUSIVE_REFLECTION_REGISTRATION(name, reg_param)                                                                      \
    static std::string_view impl_get_reflect_lunar_name() {                                                                           \
        return name;                                                                                                                  \
    }                                                                                                                                 \
    static void impl_reflect_lunar_for_class(::rainy::meta::reflection::registration::class_<lunar_current_register_type> &reg_param)

namespace rainy::meta::reflection::implements {
    template <typename Ty>
    using registration_derived_t =
        type_traits::other_trans::conditional_t<type_traits::type_relations::is_void_v<Ty>, registration, registration::class_<Ty>>;
}

namespace rainy::meta::reflection {
    template <typename... Args>
    RAINY_NODISCARD std::tuple<Args...> default_arguments(Args &&...args) {
        return std::make_tuple(utility::forward<Args>(args)...);
    }

    template <typename ClassType, typename Fx>
    class registration::bind<method, ClassType, Fx> : public implements::registration_derived_t<ClassType> {
    public:
        bind(implements::type_accessor *type,annotations::lifetime::in<std::string_view> name, Fx &&f) :
            implements::registration_derived_t<ClassType>(core::internal_construct_tag, type), type_accessor(type),
            fn(utility::forward<Fx>(f)), name(name), meth_{} {
        }

        ~bind() {
            if (type_accessor) {
                if (meth_.empty()) {
                    static collections::array<metadata, 0> empty{};
                    static std::tuple<> a;
                    meth_ = method::make(name, utility::move(fn), a, empty);
                }
                if (implements::check_method_field(type_accessor, name, meth_)) {
                    type_accessor->methods().emplace(name, utility::move(meth_));
                }
            }
        }

        template <typename... Args>
        rain_fn operator()(Args &&...args)->implements::registration_derived_t<ClassType> {
            static constexpr std::size_t metadata_count = implements::metadata_count<Args...>;
            collections::array<metadata, metadata_count> metadatas =
                utility::extract_args_to_array<metadata>(utility::forward<Args>(args)...);
            using tuple_type = typename implements::extract_unique_tuple<Args...>::type;
            if constexpr (!type_traits::type_relations::is_void_v<tuple_type>) {
                tuple_type arguments = implements::extract_tuple_from_args<tuple_type>(utility::forward<Args>(args)...);
                meth_ = reflection::method::make(name, utility::forward<Fx>(fn), arguments, metadatas);
            } else {
                static std::tuple<> empty_arguments;
                meth_ = reflection::method::make(name, utility::forward<Fx>(fn), empty_arguments, metadatas);
            }
            return implements::registration_derived_t<ClassType>(core::internal_construct_tag, type_accessor);
        }

    private:
        implements::type_accessor *type_accessor;
        Fx fn;
        std::string_view name;
        reflection::method meth_;
    };

    template <typename ClassType, typename Fx>
    class registration::bind<constructor, ClassType, Fx> : public implements::registration_derived_t<ClassType> {
    public:
        bind(implements::type_accessor *type, annotations::lifetime::in<std::string_view> name, Fx &&f) :
            implements::registration_derived_t<ClassType>(core::internal_construct_tag, type), type_accessor(type),
            fn(utility::forward<Fx>(f)), name(name) {
        }

        ~bind() {
            if (type_accessor) {
                if (ctor_.empty()) {
                    static collections::array<metadata, 0> empty{};
                    static std::tuple<> a;
                    ctor_ = constructor::make(name, utility::move(fn), a, empty);
                }
                if (implements::check_ctor_field(type_accessor, ctor_)) {
                    type_accessor->ctors().emplace_back(utility::move(ctor_));
                }
            }
        }

        template <typename... Args>
        rain_fn operator()(Args &&...args)->implements::registration_derived_t<ClassType> {
            static constexpr std::size_t metadata_count = implements::metadata_count<Args...>;
            collections::array<metadata, metadata_count> metadatas =
                utility::extract_args_to_array<metadata>(utility::forward<Args>(args)...);
            using tuple_type = typename implements::extract_unique_tuple<Args...>::type;
            if constexpr (!type_traits::type_relations::is_void_v<tuple_type>) {
                tuple_type arguments = implements::extract_tuple_from_args<tuple_type>(utility::forward<Args>(args)...);
                ctor_ = reflection::constructor::make(name, utility::forward<Fx>(fn), arguments, metadatas);
            } else {
                static std::tuple<> empty_arguments;
                ctor_ = reflection::constructor::make(name, utility::forward<Fx>(fn), empty_arguments, metadatas);
            }
            return implements::registration_derived_t<ClassType>(core::internal_construct_tag, type_accessor);
        }

    private:
        implements::type_accessor *type_accessor;
        Fx fn;
        std::string_view name;
        constructor ctor_;
    };

    template <typename ClassType, typename Field>
    class registration::bind<property, ClassType, Field> : public implements::registration_derived_t<ClassType> {
    public:
        bind(implements::type_accessor *type, annotations::lifetime::in<std::string_view> name, Field &&field) :
            implements::registration_derived_t<ClassType>(core::internal_construct_tag, type), type_accessor(type),
            field(utility::forward<Field>(field)), name(name), prop_{} {
        }

        ~bind() {
            if (type_accessor) {
#if RAINY_HAS_CXX20
                if (type_accessor->properties().contains(name)) {
                    return;
                }
#else
                if (type_accessor->properties().find(name) != type_accessor->properties().end()) {
                    return;
                }
#endif
                if (prop_.empty()) {
                    static collections::array<metadata, 0> empty{};
                    prop_ = property::make(name, utility::forward<Field>(field), empty);
                }
                type_accessor->properties().emplace(name, utility::move(prop_));
            }
        }

        template <typename... Args>
        rain_fn operator()(Args &&...args) -> implements::registration_derived_t<ClassType> {
            static constexpr std::size_t metadata_count = implements::metadata_count<Args...>;
            collections::array<metadata, metadata_count> metadatas =
                utility::extract_args_to_array<metadata>(utility::forward<Args>(args)...);
            using tuple_type = typename implements::extract_unique_tuple<Args...>::type;
            if constexpr (!type_traits::type_relations::is_void_v<tuple_type>) {
                tuple_type arguments = implements::extract_tuple_from_args<tuple_type>(utility::forward<Args>(args)...);
                prop_ = property::make(name, utility::forward<Field>(field), metadatas);
            } else {
                prop_ = property::make(name, utility::forward<Field>(field), metadatas);
            }
            return implements::registration_derived_t<ClassType>(core::internal_construct_tag, type_accessor);
        }

    private:
        implements::type_accessor *type_accessor;
        Field field;
        std::string_view name;
        reflection::property prop_;
    };

    template <typename ClassType, typename EnumType>
    class registration::bind<enumeration, ClassType, EnumType> : public implements::registration_derived_t<ClassType> {
    public:
        bind(implements::type_accessor *type, annotations::lifetime::in<std::string_view> name, implements::type_accessor* class_t) :
            implements::registration_derived_t<ClassType>(core::internal_construct_tag, class_t), name{name},
            enumeration_type_accessor{type}, class_t{class_t} {
        }

        ~bind() {
            // 如果用户未进行指定，由moon提供注册源（不保证一定返回全部数据）
            if (rainy_let enum_type_storage = implements::new_enum_type_storage_instance<EnumType>(enumeration_type_accessor);
                enum_type_storage->enum_count() == 0) {
                auto enums = meta::moon::enum_entries<EnumType>();
                if constexpr (meta::moon::enum_count<EnumType>() != 0) {
                    enum_type_storage->enums_.reserve(enums.size());
                    enum_type_storage->items_.reserve(enums.size());
                    enum_type_storage->names_.reserve(enums.size());
                    for (const auto &item: enums) {
                        enum_type_storage->enums_.emplace_back(item.first);
                        enum_type_storage->items_.emplace_back(item.first);
                        enum_type_storage->names_.emplace_back(item.second);
                    }
                }
            }
        }

        template <typename... Args>
        rain_fn operator()(Args &&...args)->implements::registration_derived_t<ClassType> {
            rainy_let enum_type_storage = implements::new_enum_type_storage_instance<EnumType>(enumeration_type_accessor);
            static constexpr std::size_t enum_values_count =
                type_traits::other_trans::count_type_v<implements::enum_data<EnumType>, type_traits::other_trans::type_list<Args...>>;
            if constexpr (enum_values_count != -1u) {
                collections::array<implements::enum_data<EnumType>, enum_values_count> enumerations =
                    utility::extract_args_to_array<implements::enum_data<EnumType>>(utility::forward<Args>(args)...);
                enum_type_storage->enums_.reserve(enum_values_count);
                enum_type_storage->items_.reserve(enum_values_count);
                enum_type_storage->names_.reserve(enum_values_count);
                for (const implements::enum_data<EnumType> &item: enumerations) {
                    enum_type_storage->enums_.emplace_back(item.get_value());
                    enum_type_storage->items_.emplace_back(item.get_value());
                    enum_type_storage->names_.emplace_back(item.get_name());
                }
            }
            return implements::registration_derived_t<ClassType>(core::internal_construct_tag, class_t);
        }

    private:
        std::string_view name;
        implements::type_accessor *enumeration_type_accessor;
        implements::type_accessor *class_t;
    };

    template <typename ClassType, typename FundType>
    class registration::bind<fundmental, ClassType, FundType> : public implements::registration_derived_t<ClassType> {
    public:
        bind(implements::type_accessor *type, annotations::lifetime::in<std::string_view> name, implements::type_accessor *class_t) :
            implements::registration_derived_t<ClassType>(core::internal_construct_tag, class_t), name{name},
            fundmental_type_accessor{type}, class_t{class_t} {
        }

        template <typename... Args>
        rain_fn operator()(Args &&...args)->implements::registration_derived_t<ClassType> {
            static constexpr std::size_t metadata_count = implements::metadata_count<Args...>;
            if constexpr (metadata_count != 0) {
                collections::array<metadata, metadata_count> metadatas =
                    utility::extract_args_to_array<metadata>(utility::forward<Args>(args)...);
                auto &metadata_ref = fundmental_type_accessor->metadatas();
                for (const auto &metadata: metadatas) {
                    metadata_ref.emplace_back(utility::move(metadata));
                }
            }
            return implements::registration_derived_t<ClassType>(core::internal_construct_tag, class_t);
        }

        ~bind() = default;

    private:
        std::string_view name;
        implements::type_accessor *fundmental_type_accessor;
        implements::type_accessor *class_t;
    };
}

#endif
