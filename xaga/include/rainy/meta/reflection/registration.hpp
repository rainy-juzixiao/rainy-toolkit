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
#include <rainy/meta/reflection/metadata.hpp>
#include <rainy/meta/reflection/moon/reflect.hpp>
#include <rainy/meta/reflection/type.hpp>
#include <string_view>

namespace rainy::meta::reflection {
    template <typename Any,
              type_traits::other_trans::enable_if_t<
                  type_traits::type_relations::is_same_v<type_traits::cv_modify::remove_cvref_t<Any>, utility::any>, int> = 0>
    RAINY_INLINE object_view as_object_view(Any &&any) {
        return object_view{const_cast<void *>(any.target_as_void_ptr()), any.type()};
    }
}

namespace rainy::meta::reflection::implements {
    RAINY_TOOLKIT_API bool check_method_field(type_accessor *type, std::string_view name, method &meth);
    RAINY_TOOLKIT_API bool check_ctor_field(type_accessor *type, method &ctor);
    RAINY_TOOLKIT_API void register_method_helper(type_accessor *type, std::string_view name, method &&meth);

    template <typename Type>
    static type_accessor *new_type_accessor_instance(std::string_view name) noexcept {
        static std::once_flag flag;
        static type_accessor_impl_class<Type> instance{name};
        std::call_once(flag, [&]() { injector::register_type<Type>(name, &instance); });
        return &instance;
    }
}

namespace rainy::meta::reflection {
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
            class_ &property(std::string_view name, Property &&property) {
#if RAINY_HAS_CXX20
                if (type->properties().contains(name)) {
                    return *this;
                }
#else
                if (type->properties().find(name) != type->properties().end()) {
                    return *this;
                }
#endif
                type->properties().emplace(name, property);
                return *this;
            }

            /**
             * @brief 向当前类注册构造函数
             * @tparam Args 构造函数参数类型列表
             * @return 一个绑定构造函数的注册器对象
             */
            template <typename... Args,
                      type_traits::other_trans::enable_if_t<type_traits::type_properties::is_constructible_v<Type, Args...>, int> = 0>
            bind<reflection::constructor, Type, Type (*)(Args...)> constructor() {
                static constexpr auto name_var = implements::make_ctor_name<Type, Args...>();
                std::string_view name{name_var.data(), name_var.size()};
                return bind<reflection::constructor, Type, Type (*)(Args...)>{this->type, name, utility::get_ctor_fn<Type, Args...>()};
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

            /**
             * @brief 向当前类注册基类
             * @tparam Base 基类的类型
             * @tparam  要求Base必须是目标类型的基类
             * @param name Base类的名称，默认为类型名称，由CTTI生成
             * @param reflect_moon 保留参数，暂未使用（未来用于自动反射基类的moon信息）
             * @return 当前注册器实例的引用
             */
            template <typename Base,
                      type_traits::other_trans::enable_if_t<type_traits::type_relations::is_base_of_v<Base, Type>, int> = 0>
            class_ &base(std::string_view name = rainy_typeid(Base).name(), bool reflect_moon = false) {
                auto &bases = this->type->bases();
                if (bases.find(name) == bases.end()) {
                    if (!implements::register_table::has_register<Base>()) {
                        class_<Base>(name).template derive<Type>(type->name()); // 注册一个新类型，并将该类直接注入到表中
                    }
                    foundation::ctti::register_base<Type, Base>();
                    bases.emplace(name, type::get<Base>());
                }
                return *this;
            }

            /**
             * @brief 向当前类注册派生类
             * @tparam Derive 派生类的类型
             * @tparam  要求Derive必须是目标类型的派生类
             * @param name Derive类的名称，默认为类型名称，由CTTI生成
             * @param reflect_moon 保留参数，暂未使用（未来用于自动反射派生类的moon信息）
             * @return 当前注册器实例的引用
             */
            template <typename Derive,
                      type_traits::other_trans::enable_if_t<type_traits::type_relations::is_base_of_v<Type, Derive>, int> = 0>
            class_ &derive(std::string_view name = rainy_typeid(Derive).name(), bool reflect_moon = false) {
                auto &deriveds = this->type->deriveds();
                if (deriveds.find(name) == deriveds.end()) {
                    if (!implements::register_table::has_register<Derive>()) {
                        class_<Derive>(name).template base<Type>(type->name()); // 注册一个新类型，并将该类直接注入到表中
                    }
                    foundation::ctti::register_base<Derive, Type>();
                    deriveds.emplace(name, type::get<Derive>());
                }
                return *this;
            }

        private:
            type_accessor *type;
        };

    private:
    };
}

#define RAINY_REFLECTION_REGISTRATION                                                                                                 \
    static void RAINY_CAT(RAINY_CAT(rainytoolkit_auto_register_reflection_, __LINE__), _function_());                                 \
    namespace rainy::meta::implements::auto_regsiter {                                                                                \
        struct RAINY_CAT(rainytoolkit_reflection_auto_register, __LINE__) {                                                           \
            RAINY_CAT(rainytoolkit_reflection_auto_register, __LINE__)() {                                                            \
                RAINY_CAT(RAINY_CAT(rainytoolkit_auto_register_reflection_, __LINE__), _function_());                                 \
            }                                                                                                                         \
        };                                                                                                                            \
    }                                                                                                                                 \
    static const RAINY_CAT(rainy::meta::implements::auto_regsiter::rainytoolkit_reflection_auto_register, __LINE__)                   \
        RAINY_CAT(rainy_toolkit_reflect_auto_register_, __LINE__);                                                                    \
    static void RAINY_CAT(RAINY_CAT(rainytoolkit_auto_register_reflection_, __LINE__), _function_())

namespace rainy::meta::reflection::implements {
    template <typename Ty>
    using registration_derived_t =
        type_traits::other_trans::conditional_t<type_traits::type_relations::is_void_v<Ty>, registration, registration::class_<Ty>>;

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
    public:
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
    template <typename... Args>
    RAINY_NODISCARD std::tuple<Args...> default_arguments(Args &&...args) {
        return std::make_tuple(utility::forward<Args>(args)...);
    }

    template <typename ClassType, typename Fx>
    class registration::bind<method, ClassType, Fx> : public implements::registration_derived_t<ClassType> {
    public:
        bind(implements::type_accessor *type, std::string_view name, Fx &&f) :
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
        implements::registration_derived_t<ClassType> operator()(Args &&...args) {
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
        bind(implements::type_accessor *type, std::string_view name, Fx &&f) :
            implements::registration_derived_t<ClassType>(core::internal_construct_tag, type), type_accessor(type),
            fn(utility::forward<Fx>(f)), name(name), ctor_{} {
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
        implements::registration_derived_t<ClassType> operator()(Args &&...args) {
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
}

#endif
