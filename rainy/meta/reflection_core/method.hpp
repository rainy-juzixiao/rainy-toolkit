#ifndef RAINY_REFLECTION_CORE_METHOD_HPP
#define RAINY_REFLECTION_CORE_METHOD_HPP
#include <rainy/base.hpp>
#include <rainy/meta/reflection_core/field.hpp>

namespace rainy::foundation::reflection {
    template <typename ReturnType, typename... Args>
    struct static_invoker {
        using any = containers::any;

        static_invoker() = default;

        static_invoker(ReturnType (*invoker)(Args...)) noexcept : invoker(invoker) {
        }

        any invoke(const std::vector<any> &params) const {
            return invoke_impl(invoker, params, std::index_sequence_for<Args...>{});
        }

        template <size_t... I>
        static any invoke_impl(ReturnType (*method)(Args...), const std::vector<any> &params, std::index_sequence<I...>) {
            if (sizeof...(Args) != params.size()) {
                rainy::foundation::system::exceptions::runtime::throw_runtime_error("Parameter count mismatch");
            }
            if constexpr (std::is_void_v<ReturnType>) {
                method(rainy::utility::any_cast<Args>(params[I])...);
                return {};
            } else {
                return containers::make_any<ReturnType>(method(rainy::utility::any_cast<Args>(params[I])...));
            }
        }

        ReturnType (*invoker)(Args...);
    };

    template <typename Class, typename ReturnType, typename... Args>
    struct method_invoker {
        using any = rainy::containers::any;

        method_invoker() = default;

        method_invoker(ReturnType (Class::*invoker)(Args...)) noexcept : invoker(invoker) {
        }

        any invoke(void *obj, const std::vector<any> &params) const {
            return invoke_impl(obj, invoker, params, std::index_sequence_for<Args...>{});
        }

        template <std::size_t... I>
        static any invoke_impl(void *obj, ReturnType (Class::*method)(Args...), const std::vector<any> &params,
                               std::index_sequence<I...>) {
            if (sizeof...(Args) != params.size()) {
                rainy::foundation::system::exceptions::runtime::throw_runtime_error("Parameter count mismatch");
            }
            if constexpr (std::is_void_v<ReturnType>) {
                rainy::utility::invoke(method, static_cast<Class *>(obj), rainy::utility::any_cast<Args>(params[I])...);
                return {}; // 返回空的 any 对象
            } else {
                return containers::make_any<ReturnType>(
                    rainy::utility::invoke(method, static_cast<Class *>(obj), rainy::utility::any_cast<Args>(params[I])...));
            }
        }

        ReturnType (Class::*invoker)(Args...);
    };

    template <typename ReturnType, typename... Args>
    struct method_invoker<void, ReturnType, Args...> {
        using any = rainy::containers::any;

        method_invoker() = default;

        template <typename Class>
        method_invoker(ReturnType (Class::*)(Args...)) noexcept {
        }

        any invoke(void *, const std::vector<any> &) const {
            return {};
        }
    };

    template <typename Class, typename ReturnType, typename... Args>
    struct cmethod_invoker {
        using any = rainy::containers::any;

        cmethod_invoker() = default;

        cmethod_invoker(ReturnType (Class::*invoker)(Args...) const) noexcept : invoker(invoker) {
        }

        any invoke(void *obj, const std::vector<rainy::containers::any> &params) const {
            return invoke_impl(obj, invoker, params, std::index_sequence_for<Args...>{});
        }

        template <std::size_t... I>
        static any invoke_impl(void *obj, ReturnType (Class::*method)(Args...) const, const std::vector<any> &params,
                               std::index_sequence<I...>) {
            if (sizeof...(Args) != params.size()) {
                rainy::foundation::system::exceptions::runtime::throw_runtime_error("Parameter count mismatch");
            }
            if constexpr (std::is_void_v<ReturnType>) {
                (static_cast<const Class *>(obj)->*method)(rainy::utility::any_cast<Args>(params[I])...);
                return {}; // 返回空的 any 对象
            } else {
                return any((static_cast<const Class *>(obj)->*method)(rainy::utility::any_cast<Args>(params[I])...));
            }
        }

        ReturnType (Class::*invoker)(Args...) const;
    };

    template <typename ReturnType, typename... Args>
    struct cmethod_invoker<void, ReturnType, Args...> {
        using any = rainy::containers::any;

        cmethod_invoker() = default;

        template <typename Class>
        cmethod_invoker(ReturnType (Class::*)(Args...)) noexcept {
        }

        any invoke(void *, const std::vector<any> &) const {
            return {};
        }
    };

    class method {
    public:
        enum class method_type {
            static_method,
            normal_method,
            const_method,
            placeholder_value
        };

        method() noexcept : _name(), _return_type(), _param_types({}), invoker(), _attrs({}){};

        template <typename Class, typename ReturnType, typename... Args>
        method(const std::string_view name, const std::string_view return_type, const std::vector<std::string_view> &param_types,
               const attributes &attrs, ReturnType (Class::*method)(Args...)) :
            _name(name),
            _return_type(return_type), _param_types(param_types),
            invoker(std::make_unique<invoker_helper_impl<Class, ReturnType, Args...>>(method)), _attrs(attrs) {
        }

        template <typename Class, typename ReturnType, typename... Args>
        method(const std::string_view name, const std::string_view return_type, const std::vector<std::string_view> &param_types,
               const attributes &attrs, ReturnType (Class::*method)(Args...) const) :
            _name(name),
            _return_type(return_type), _param_types(param_types),
            invoker(std::make_unique<invoker_helper_impl<Class, ReturnType, Args...>>(method)), _attrs(attrs) {
        }

        template <typename ReturnType, typename... Args>
        method(const std::string_view name, const std::string_view return_type, const std::vector<std::string_view> &param_types,
               const attributes &attrs, ReturnType (*static_method)(Args...)) :
            _name(name),
            _return_type(return_type), _param_types(param_types),
            invoker(std::make_unique<invoker_helper_impl<void, ReturnType, Args...>>(static_method)), _attrs(attrs) {
        }

        bool empty() const noexcept {
            return static_cast<bool>(!invoker);
        }

        explicit operator bool() const noexcept {
            return empty();
        }

        template <typename ReturnType = void>
        auto invoke(void *object, const std::vector<rainy::containers::any> &args = {}) const {
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
        auto invoke(const void *object, const std::vector<rainy::containers::any> &args = {}) const {
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
        auto invoke_with_paramspack(void *object, Args &&...args) const {
            if (empty()) {
                rainy::foundation::system::exceptions::runtime::throw_runtime_error("internal object is error!");
            }
            std::vector<rainy::containers::any> params_pack = {rainy::utility::forward<Args>(args)...};
            return invoker->invoke(object, params_pack);
        }

        rainy::containers::any operator()(void *object,
                                                         const std::vector<rainy::containers::any> &params) const {
            return this->invoke(object, params);
        }

        RAINY_NODISCARD std::string_view name() const noexcept {
            return _name;
        }

        RAINY_NODISCARD std::string_view return_type() const noexcept {
            return _return_type;
        }

        RAINY_NODISCARD const std::vector<std::string_view> &param_types() const noexcept {
            return _param_types;
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

    private:
        struct invoker_helper {
            virtual rainy::containers::any invoke(void *,
                                                                 const std::vector<rainy::containers::any> &) const = 0;
            virtual rainy::containers::any invoke(const void *,
                                                                 const std::vector<rainy::containers::any> &) const = 0;
            virtual method_type type() const noexcept = 0;
            virtual bool is_const() const noexcept = 0;
            virtual bool is_static() const noexcept = 0;
        };

        template <typename Class, typename ReturnType, typename... Args>
        struct invoker_helper_impl : public invoker_helper {
            template <typename Method>
            invoker_helper_impl(Method method) {
                construct(method);
            }

            rainy::containers::any invoke(
                void *object, const std::vector<rainy::containers::any> &params) const override {
                switch (storage.type) {
                    case method_type::normal_method:
                        return storage.normal_method.invoke(object, params);
                    case method_type::const_method:
                        return storage.const_method.invoke(object, params);

                    default:
                        break;
                }
                return {};
            }

            rainy::containers::any invoke(
                const void *object, const std::vector<rainy::containers::any> &params) const override {
                return invoke(const_cast<void *>(object), params);
            }

            method_type type() const noexcept override {
                return storage.type;
            }

            bool is_const() const noexcept override {
                return storage.type == method_type::const_method;
            }

            bool is_static() const noexcept override {
                return storage.type == method_type::static_method;
            }

            template <typename Method>
            void construct(Method method) {
                if constexpr (std::is_same_v<Method, ReturnType (*)(Args...)>) {
                    storage.type = method_type::static_method;
                    rainy::utility::construct_at(&storage.static_method, method);
                } else {
                    if constexpr (std::is_same_v<Method, ReturnType (Class::*)(Args...)>) {
                        storage.type = method_type::normal_method;
                        rainy::utility::construct_at(&storage.normal_method, method);
                    } else if constexpr (std::is_same_v<Method, ReturnType (Class::*)(Args...) const>) {
                        storage.type = method_type::const_method;
                        rainy::utility::construct_at(&storage.const_method, method);
                    } else {
                        static_assert(rainy::type_traits::internals::always_false<void>, "unsupported type");
                    }
                }
            }

            struct storage_t {
                union {
                    method_invoker<Class, ReturnType, Args...> normal_method;
                    cmethod_invoker<Class, ReturnType, Args...> const_method;
                    static_invoker<ReturnType, Args...> static_method;
                };
                method_type type;
            };

            union {
                storage_t storage{};
                std::max_align_t dummy;
            };
        };

        std::string_view _name;
        std::string_view _return_type;
        std::vector<std::string_view> _param_types;
        std::shared_ptr<invoker_helper> invoker{nullptr};
        attributes _attrs;
    };

    template <typename ReturnType, typename... Args>
    method make_static_method(const std::string_view name, ReturnType (*static_method)(Args...), const attributes &attrs = {}) {
        return method(name, typeid(ReturnType).name(), {typeid(Args).name()...}, attrs, static_method);
    }

    template <typename Class, typename ReturnType, typename... Args>
    method make_const_method(const std::string_view name, ReturnType (Class::*const_method)(Args...) const,
                             const attributes &attrs = {}) {
        return method(name, typeid(ReturnType).name(), {typeid(Args).name()...}, attrs, const_method);
    }

    template <typename Class, typename ReturnType, typename... Args>
    method make_method(const std::string_view name, ReturnType (Class::*normal_method)(Args...), const attributes &attrs = {}) {
        return method(name, typeid(ReturnType).name(), {typeid(Args).name()...}, attrs, normal_method);
    }
}

#endif