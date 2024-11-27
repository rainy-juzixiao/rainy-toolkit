#ifndef RAINY_REFLECTION_CORE_METHOD_HPP
#define RAINY_REFLECTION_CORE_METHOD_HPP
#include <rainy/base.hpp>
#include <rainy/internals/reflection_core/field.hpp>
#include <rainy/system/nebula_ptr.hpp>
#include <rainy/internals/reflection_core/method_invoker.hpp>

// 定义普通的类成员的method构造函数
#define RAINY_METHOD_CONSTRUCTER_SPEC_DECLARE(SPEC) \
template <typename Class, typename ReturnType, typename... Args>\
method(std::string_view name, const std::type_info& return_type, const std::vector<utility::dynamic_type_info> &param_types,\
       const attributes &attrs, ReturnType (Class::*method)(Args...) SPEC) :\
    _name(name),\
    _return_type(return_type), _param_types(param_types),\
    invoker(std::make_shared<invoker_helper_impl<ReturnType (Class::*)(Args...) SPEC, Class, ReturnType, Args...>>(method)), _attrs(attrs) {             \
    }

namespace rainy::foundation::reflection {
    class method {
    public:
        method() noexcept : _name(), _return_type(typeid(void)), _param_types({}), invoker(), _attrs({}){};

        template <typename Class, typename ReturnType, typename... Args>
        method(const std::string_view &name, const std::type_info &return_type, const std::vector<utility::dynamic_type_info> &param_types,
               const attributes &attrs, ReturnType (Class::*method)(Args...)) :
            _name(name),
            _return_type(return_type), _param_types(param_types),
            invoker(std::make_shared<invoker_helper_impl<ReturnType (Class::*)(Args...), Class, ReturnType, Args...>>(method)),
            _attrs(attrs) {
        }

        template <typename ReturnType, typename... Args>
        method(const std::string_view& name, const std::type_info& return_type,
               const std::vector<utility::dynamic_type_info> &param_types,
               const attributes &attrs, ReturnType (*static_method)(Args...)) :
            _name(name),
            _return_type(return_type), _param_types(param_types),
            invoker(std::make_shared<invoker_helper_impl<ReturnType(Args...), void, ReturnType, Args...>>(static_method)), _attrs(attrs) {
        }

        template <typename ReturnType, typename... Args>
        method(const std::string_view &name, const std::type_info &return_type, const std::vector<utility::dynamic_type_info> &param_types,
               const attributes &attrs, ReturnType (*static_method)(Args...) noexcept) :
            _name(name),
            _return_type(return_type), _param_types(param_types),
            invoker(std::make_shared<invoker_helper_impl<ReturnType(Args...) noexcept, void, ReturnType, Args...>>(static_method)),
            _attrs(attrs) {
        }

        RAINY_METHOD_CONSTRUCTER_SPEC_DECLARE(&)
        RAINY_METHOD_CONSTRUCTER_SPEC_DECLARE(&&)
        RAINY_METHOD_CONSTRUCTER_SPEC_DECLARE(const)
        RAINY_METHOD_CONSTRUCTER_SPEC_DECLARE(const &)
        RAINY_METHOD_CONSTRUCTER_SPEC_DECLARE(const &&)
        RAINY_METHOD_CONSTRUCTER_SPEC_DECLARE(noexcept)
        RAINY_METHOD_CONSTRUCTER_SPEC_DECLARE(& noexcept)
        RAINY_METHOD_CONSTRUCTER_SPEC_DECLARE(&& noexcept)
        RAINY_METHOD_CONSTRUCTER_SPEC_DECLARE(const noexcept)
        RAINY_METHOD_CONSTRUCTER_SPEC_DECLARE(const & noexcept)
        RAINY_METHOD_CONSTRUCTER_SPEC_DECLARE(const && noexcept)
        RAINY_METHOD_CONSTRUCTER_SPEC_DECLARE(volatile)
        RAINY_METHOD_CONSTRUCTER_SPEC_DECLARE(volatile &)
        RAINY_METHOD_CONSTRUCTER_SPEC_DECLARE(volatile &&)
        RAINY_METHOD_CONSTRUCTER_SPEC_DECLARE(volatile noexcept)
        RAINY_METHOD_CONSTRUCTER_SPEC_DECLARE(volatile & noexcept)
        RAINY_METHOD_CONSTRUCTER_SPEC_DECLARE(volatile && noexcept)
        RAINY_METHOD_CONSTRUCTER_SPEC_DECLARE(const volatile)
        RAINY_METHOD_CONSTRUCTER_SPEC_DECLARE(const volatile &)
        RAINY_METHOD_CONSTRUCTER_SPEC_DECLARE(const volatile &&)
        RAINY_METHOD_CONSTRUCTER_SPEC_DECLARE(const volatile noexcept)
        RAINY_METHOD_CONSTRUCTER_SPEC_DECLARE(const volatile & noexcept)
        RAINY_METHOD_CONSTRUCTER_SPEC_DECLARE(const volatile && noexcept)

        bool empty() const noexcept {
            return static_cast<bool>(!invoker);
        }

        explicit operator bool() const noexcept {
            return empty();
        }

        template <typename ReturnType = void>
        auto invoke(void *object = nullptr, const std::vector<containers::any> &args = {}) const {
            if (empty()) {
                rainy::foundation::system::exceptions::runtime::throw_runtime_error("internal object is error!");
            }
            if constexpr (std::is_void_v<ReturnType>) {
                return invoker->invoke(object, args);
            } else {
                return invoker->invoke(object, args).as<ReturnType>();
            }
        }

        template <typename ReturnType = void>
        auto invoke(const void *object = nullptr, const std::vector<containers::any> &args = {}) const {
            if (empty()) {
                rainy::foundation::system::exceptions::runtime::throw_runtime_error("internal object is error!");
            }
            if constexpr (std::is_void_v<ReturnType>) {
                return invoker->invoke(object, args);
            } else {
                return invoker->invoke(object, args).as<ReturnType>();
            }
        }

        template <typename... Args>
        auto invoke_with_paramspack(void *object = nullptr, Args &&...args) const {
            if (empty()) {
                rainy::foundation::system::exceptions::runtime::throw_runtime_error("internal object is error!");
            }
            std::vector<containers::any> params_pack = {rainy::utility::forward<Args>(args)...};
            return invoker->invoke(object, params_pack);
        }

        containers::any operator()(void *object, const std::vector<containers::any> &params) const {
            return this->invoke(object, params);
        }

        RAINY_NODISCARD std::string_view name() const noexcept {
            return _name;
        }

        RAINY_NODISCARD utility::dynamic_type_info return_type() const noexcept {
            return _return_type;
        }

        RAINY_NODISCARD std::string_view return_type_as_stringview() const noexcept {
            return return_type().name();
        }

        RAINY_NODISCARD const std::vector<utility::dynamic_type_info> &param_types() const noexcept {
            return _param_types;
        }

        RAINY_NODISCARD std::vector<std::string_view> param_types_as_stringview() const noexcept {
            std::size_t transform_length = _param_types.size();
            std::vector<std::string_view> sv_array(transform_length, {}); // 确保transform能成功
            algorithm::container_operater::transform(_param_types.begin(), _param_types.end(), sv_array.begin(),
                                                     [](const utility::dynamic_type_info &element) { return element.name(); });
            return sv_array;
        }

        attributes &attrs() noexcept {
            return _attrs;
        }

        const attributes &attrs() const noexcept {
            return _attrs;
        }

        RAINY_NODISCARD bool is_const() const noexcept {
            return invoker->is_const();
        }

        RAINY_NODISCARD bool is_static() const noexcept {
            return invoker->is_static();
        }

        utility::dynamic_type_info signature() const noexcept {
            return invoker->signature();
        }

    private:
        struct invoker_helper {
            virtual containers::any invoke(void *,
                                                                 const std::vector<containers::any> &) const = 0;
            virtual containers::any invoke(const void *,
                                                                 const std::vector<containers::any> &) const = 0;
            virtual method_type type() const noexcept = 0;
            virtual bool is_const() const noexcept = 0;
            virtual bool is_static() const noexcept = 0;
            virtual utility::dynamic_type_info signature() const noexcept = 0;
        };

        template <typename Fx,typename Class, typename ReturnType, typename... Args>
        class invoker_helper_impl : public invoker_helper {
        public:
            using function_signature = Fx;

            template <typename Method>
            invoker_helper_impl(Method method) {
                construct(method);
            }

            utility::dynamic_type_info signature() const noexcept override {
                return utility::dynamic_type_info::create<function_signature>();
            }

            containers::any invoke(void *object, const std::vector<containers::any> &params) const override {
                const method_type cur_type = type(); // 提前获取类型
                if (cur_type >= method_type::const_method && cur_type <= method_type::const_method_right_volatile_noexcept) {
                    // 处理const_method部分
                    if (cur_type >= method_type::const_method && cur_type <= method_type::const_method_volatile_noexcept) {
                        return internals::call_cmethod(storage.get(),cur_type, object, params);
                    } else if (cur_type >= method_type::const_method_left &&
                               cur_type <= method_type::const_method_left_volatile_noexcept) {
                        return internals::call_cmethod_left(storage.get(), cur_type, object, params);
                    } else {
                        return internals::call_cmethod_right(storage.get(), cur_type, object, params);
                    }
                } else if (cur_type >= method_type::normal_method && cur_type <= method_type::normal_method_right_volatile_noexcept) {
                    // 处理normal_method部分
                    if (cur_type >= method_type::normal_method && cur_type <= method_type::normal_method_left_volatile_noexcept) {
                        return internals::call_normal_method(storage.container, cur_type, object, params);
                    } else if (cur_type >= method_type::normal_method_left &&
                               cur_type <= method_type::normal_method_left_volatile_noexcept) {
                        return internals::call_normal_method_left(storage.get(), cur_type, object, params);
                    } else {
                        return internals::call_normal_method_right(storage.get(), cur_type, object, params);
                    }
                } else {
                    switch (cur_type) {
                        case method_type::static_method:
                            return storage.container.static_method.invoke(params);
                        case method_type::static_method_noexcept:
                            return storage.container.static_method_noexcept.invoke(params);
                        default:
                            system::exceptions::runtime::throw_runtime_error("Cannot find invoker!");
                    }
                }
                return {};
            }

            containers::any invoke(const void *object, const std::vector<containers::any> &params) const override {
                return invoke(const_cast<void *>(object), params);
            }

            method_type type() const noexcept override {
                return storage.type;
            }

            bool is_const() const noexcept override {
                return storage.type == method_type::const_method || storage.type == method_type::const_method_noexcept;
            }

            bool is_static() const noexcept override {
                return storage.type == method_type::static_method;
            }

            template <typename Method>
            void construct(Method method) {
                if constexpr (std::is_convertible_v<Method, ReturnType (*)(Args...)>) {
                    storage.type = method_type::static_method;
                    utility::construct_at(&storage.container.static_method, method);
                } else if constexpr (std::is_convertible_v<Method, ReturnType (*)(Args...) noexcept>) {
                    storage.type = method_type::static_method_noexcept;
                    utility::construct_at(&storage.container.static_method_noexcept, method);
                } else {
                    using traits = type_traits::primary_types::function_traits<function_signature>;
                    if constexpr (traits::is_const_member_function) {
                        construct_cmethod<traits>(method);
                    } else {
                        construct_method<traits>(method);
                    }
                }
            }

            template <typename Traits, typename Method>
            void construct_cmethod(Method method) {
                if constexpr (Traits::is_invoke_for_lvalue) {
                    if constexpr (Traits::is_volatile && Traits::is_noexcept) {
                        storage.type = method_type::const_method_left_volatile_noexcept;
                        utility::construct_at(&storage.container.const_method_left_volatile_noexcept, method);
                    } else if constexpr (Traits::is_volatile) {
                        storage.type = method_type::const_method_left_volatile;
                        utility::construct_at(&storage.container.const_method_left_volatile, method);
                    } else if constexpr (Traits::is_noexcept) {
                        storage.type = method_type::const_method_left_noexcept;
                        utility::construct_at(&storage.container.const_method_left_noexcept, method);
                    } else {
                        storage.type = method_type::const_method_left;
                        utility::construct_at(&storage.container.const_method_left, method);
                    }
                } else if constexpr (Traits::is_invoke_for_rvalue) {
                    if constexpr (Traits::is_volatile && Traits::is_noexcept) {
                        storage.type = method_type::const_method_right_volatile_noexcept;
                        utility::construct_at(&storage.container.const_method_right_volatile_noexcept, method);
                    } else if constexpr (Traits::is_volatile) {
                        storage.type = method_type::const_method_right_volatile;
                        utility::construct_at(&storage.container.const_method_right_volatile, method);
                    } else if constexpr (Traits::is_noexcept) {
                        storage.type = method_type::const_method_right_noexcept;
                        utility::construct_at(&storage.container.const_method_right_noexcept, method);
                    } else {
                        storage.type = method_type::const_method_right;
                        utility::construct_at(&storage.container.const_method_right, method);
                    }
                } else {
                    if constexpr (Traits::is_volatile && Traits::is_noexcept) {
                        storage.type = method_type::const_method_volatile_noexcept;
                        utility::construct_at(&storage.container.const_method_volatile_noexcept, method);
                    } else if constexpr (Traits::is_volatile) {
                        storage.type = method_type::const_method_volatile;
                        utility::construct_at(&storage.container.const_method_volatile, method);
                    } else if constexpr (Traits::is_noexcept) {
                        storage.type = method_type::const_method_right_noexcept;
                        utility::construct_at(&storage.container.const_method_noexcept, method);
                    } else {
                        storage.type = method_type::const_method;
                        utility::construct_at(&storage.container.const_method, method);
                    }
                }
            }

            template <typename Traits, typename Method>
            void construct_method(Method method) {
                if constexpr (Traits::is_invoke_for_lvalue) {
                    if constexpr (Traits::is_volatile && Traits::is_noexcept) {
                        storage.type = method_type::normal_method_left_volatile_noexcept;
                        utility::construct_at(&storage.container.normal_method_left_volatile_noexcept, method);
                    } else if constexpr (Traits::is_volatile) {
                        storage.type = method_type::normal_method_left_volatile;
                        utility::construct_at(&storage.container.normal_method_left_volatile, method);
                    } else if constexpr (Traits::is_noexcept) {
                        storage.type = method_type::normal_method_left_noexcept;
                        utility::construct_at(&storage.container.normal_method_left_noexcept, method);
                    } else {
                        storage.type = method_type::normal_method_left;
                        utility::construct_at(&storage.container.normal_method_left, method);
                    }
                } else if constexpr (Traits::is_invoke_for_rvalue) {
                    if constexpr (Traits::is_volatile && Traits::is_noexcept) {
                        storage.type = method_type::normal_method_right_volatile_noexcept;
                        utility::construct_at(&storage.container.normal_method_right_volatile_noexcept, method);
                    } else if constexpr (Traits::is_volatile) {
                        storage.type = method_type::normal_method_right_volatile;
                        utility::construct_at(&storage.container.normal_method_right_volatile, method);
                    } else if constexpr (Traits::is_noexcept) {
                        storage.type = method_type::normal_method_right_noexcept;
                        utility::construct_at(&storage.container.normal_method_right_noexcept, method);
                    } else {
                        storage.type = method_type::normal_method_right;
                        utility::construct_at(&storage.container.normal_method_right, method);
                    }
                } else {
                    if constexpr (Traits::is_volatile && Traits::is_noexcept) {
                        storage.type = method_type::normal_method_volatile_noexcept;
                        utility::construct_at(&storage.container.normal_method_volatile_noexcept, method);
                    } else if constexpr (Traits::is_volatile) {
                        storage.type = method_type::normal_method_volatile;
                        utility::construct_at(&storage.container.normal_method_volatile, method);
                    } else if constexpr (Traits::is_noexcept) {
                        storage.type = method_type::normal_method_right_noexcept;
                        utility::construct_at(&storage.container.normal_method_noexcept, method);
                    } else {
                        storage.type = method_type::normal_method;
                        utility::construct_at(&storage.container.normal_method, method);
                    }
                }
            }

        private:
            struct storage_t {
                const auto &get() const noexcept {
                    return container;
                }

                internals::union_method_container<Class, ReturnType, Args...> container;
                method_type type;
            };

            union {
                storage_t storage{};
                std::max_align_t dummy;
            };
        };

        std::string_view _name;
        const std::type_info& _return_type;
        std::vector<utility::dynamic_type_info> _param_types;
        std::shared_ptr<invoker_helper> invoker{nullptr};
        attributes _attrs;
    };
}

#undef RAINY_METHOD_CONSTRUCTER_SPEC_DECLARE

#define RAINY_MAKE_METHOD(ARGNAME, QUAL) template <typename Class, typename ReturnType, typename... Args>\
method make_method(const std::string_view name, ReturnType (Class::*ARGNAME)(Args...) QUAL, const attributes &attrs = {}) {\
    return method(name, typeid(ReturnType), {typeid(Args)...}, attrs, ARGNAME);\
}

namespace rainy::foundation::reflection {
    template <typename Class, typename ReturnType, typename... Args>
    method make_method(const std::string_view name, ReturnType (Class::*normal_method)(Args...), const attributes &attrs = {}) {
        return method(name, typeid(ReturnType), {typeid(Args)...}, attrs, normal_method);
    }

    RAINY_MAKE_METHOD(const_method,const)
    RAINY_MAKE_METHOD(const_left_method,const &)
    RAINY_MAKE_METHOD(const_right_method,const &&)
    RAINY_MAKE_METHOD(const_noexcept_method, const noexcept)
    RAINY_MAKE_METHOD(const_noexcept_left_method, const & noexcept)
    RAINY_MAKE_METHOD(const_noexcept_right_method, const && noexcept)
    RAINY_MAKE_METHOD(const_volatile_method, const volatile)
    RAINY_MAKE_METHOD(const_volatile_left_method, const volatile &)
    RAINY_MAKE_METHOD(const_volatile_right_method, const volatile &&)
    RAINY_MAKE_METHOD(const_volatile_noexcept_method, const volatile noexcept)
    RAINY_MAKE_METHOD(const_volatile_noexcept_left_method, const volatile & noexcept)
    RAINY_MAKE_METHOD(const_volatile_noexcept_right_method, const volatile && noexcept)
    RAINY_MAKE_METHOD(normal_left_method, &)
    RAINY_MAKE_METHOD(normal_right_method, &&)
    RAINY_MAKE_METHOD(normal_noexcept_method, noexcept)
    RAINY_MAKE_METHOD(normal_noexcept_left_method, & noexcept)
    RAINY_MAKE_METHOD(normal_noexcept_right_method, && noexcept)
    RAINY_MAKE_METHOD(normal_volatile_method, volatile)
    RAINY_MAKE_METHOD(normal_volatile_left_method, volatile &)
    RAINY_MAKE_METHOD(normal_volatile_right_method, volatile &&)
    RAINY_MAKE_METHOD(normal_volatile_noexcept_method, volatile noexcept)
    RAINY_MAKE_METHOD(normal_volatile_noexcept_left_method, volatile & noexcept)
    RAINY_MAKE_METHOD(normal_volatile_noexcept_right_method, volatile && noexcept)

    template <typename ReturnType, typename... Args>
    method make_static_method(const std::string_view name, ReturnType (*static_method)(Args...), const attributes &attrs = {}) {
        return method(name, typeid(ReturnType), {typeid(Args)...}, attrs, static_method);
    }
}

#undef RAINY_MAKE_METHOD

#endif