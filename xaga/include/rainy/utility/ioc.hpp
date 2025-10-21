#ifndef RAINY_UTILITY_IOC_HPP
#define RAINY_UTILITY_IOC_HPP
#include <rainy/core/core.hpp>
#include <rainy/foundation/fact.hpp>
#include <rainy/utility/any.hpp>
#include <rainy/foundation/pal/threading.hpp>

namespace rainy::utility::ioc::implements {
    struct inject_item {
        virtual void *get() = 0;
        virtual const foundation::ctti::typeinfo &impl_type() = 0;
        virtual std::string_view userdef_name() = 0;
    };

    using inject_items_t = std::vector<type_traits::other_trans::pointer<implements::inject_item>>;
}

namespace rainy::utility::ioc {
    using factory_key = std::string_view;
    using factory =
        foundation::fact::unsynchronized_factory<implements::inject_items_t, factory_key, type_traits::other_trans::pointer>;

    template <typename Type>
    struct factory_inject {
        using is_not_for_inject = void;

        void inject(factory &) {
            // do nothing...
        }
    };
}

namespace rainy::utility::ioc::implements {
    using signle_instance_storage_tuple = utility::pair<any *, std::once_flag *>;

    class RAINY_TOOLKIT_API injection_context : type_traits::helper::non_copyable, type_traits::helper::non_moveable {
    public:
        static rain_fn request_a_new_singleton(const foundation::ctti::typeinfo &new_resources_type) -> signle_instance_storage_tuple;
        static rain_fn has_init(const foundation::ctti::typeinfo &new_resources_type) -> bool;
        static rain_fn get_mutex() -> std::mutex&;

        template <typename Type, typename... Args>
        static rain_fn construct(Args &&...args) -> Type * {
            auto resources = request_a_new_singleton(rainy_typeid(Type));
            any *value = resources.first;
            if (!value->has_value()) {
                if constexpr (sizeof...(Args) == 0 && type_traits::type_properties::is_default_constructible_v<Type>) {
                    value->emplace<Type>();
                    return static_cast<Type *>(const_cast<void *>(value->target_as_void_ptr()));
                } else {
                    std::once_flag &once_flag = *resources.second;
                    std::call_once(
                        once_flag,
                        [value](auto &&...args) {
                            if constexpr (type_traits::type_properties::is_constructible_v<Type, decltype(args)...>) {
                                value->emplace<Type>(utility::forward<decltype(args)>(args)...);
                            }
                        },
                        utility::forward<Args>(args)...);
                    return value->has_value() ? static_cast<Type *>(const_cast<void *>(value->target_as_void_ptr())) : nullptr;
                }
                return nullptr;
            }
            return nullptr;
        }

        template <typename Type>
        static rain_fn get_singleton() -> Type * {
            return has_init(rainy_typeid(Type)) ? static_cast<Type *>(const_cast<void *>(
                                                      request_a_new_singleton(rainy_typeid(Type)).first->target_as_void_ptr()))
                                                : construct<Type>();
        }

        template <typename Type, typename... Args>
        static rain_fn get_singleton_with_args(Args &&...args) -> Type * {
            return has_init(rainy_typeid(Type)) ? static_cast<Type *>(const_cast<void *>(
                                                      request_a_new_singleton(rainy_typeid(Type)).first->target_as_void_ptr()))
                                                : construct<Type>(utility::forward<Args>(args)...);
        }

        template <typename Type, typename... Args>
        static rain_fn construct_new_instance(Args&&... args) -> Type* {
            return new Type(utility::forward<Args>(args)...);
        }

    private:
        injection_context() noexcept = default;
    };

    template <typename Type, typename... Args>
    struct inject_item_layer : public inject_item {
        virtual foundation::system::memory::nebula_ptr<Type> construct_new(Args... args) = 0;
        virtual Type *get_signleton_with_args(Args... args) = 0;

        const foundation::ctti::typeinfo &impl_type() override {
            return rainy_typeid(Type(Args...));
        }
    };

    template <typename Type ,typename Impl, typename... Args>
    struct inject_item_implement : inject_item_layer<Type, Args...> {
        inject_item_implement(std::string_view name = {}) : name{name} {
        }

        std::string_view userdef_name() {
            return name;
        }

        void *get() override {
            return injection_context::get_singleton<Impl>();
        }

        Type *get_signleton_with_args(Args... args) override {
            return injection_context::get_singleton_with_args<Impl>(utility::forward<Args>(args)...);
        }

        foundation::system::memory::nebula_ptr<Type> construct_new(Args... args) override {
            return injection_context::construct_new_instance<Impl>(utility::forward<Args>(args)...);
        }
    
        std::string_view name;
    };

    template <typename Type, typename Impl, typename... Args>
    inject_item_implement<Type, Impl, Args...> inject_item_object{};

    template <typename Type>
    static std::vector<type_traits::other_trans::pointer<inject_item>> inject_items;

    template <typename Type, typename = void>
    RAINY_CONSTEXPR_BOOL is_factory_inject_available = true;

    template <typename Type>
    RAINY_CONSTEXPR_BOOL
        is_factory_inject_available<Type, type_traits::other_trans::void_t<typename factory_inject<Type>::is_not_for_inject>> = false;
}

namespace rainy::utility::ioc {
    class RAINY_TOOLKIT_API container final : type_traits::helper::non_copyable, type_traits::helper::non_moveable {
    public:
        using key = factory_key;
        using factory = ioc::factory;

        template <typename Type, typename Impl = Type, typename... Args>
        static rain_fn register_type() -> void {
            foundation::pal::threading::create_synchronized_task(implements::injection_context::get_mutex(), []() {
                if constexpr (implements::is_factory_inject_available<Type>) {
                    factory_inject<Type>{}.inject(*impl()->storage);
                } else {
                    if (is_this_type_available<Type>() && !is_this_type_with_construct_args_type_available<Type(Args...)>) {
                        return;
                    }
                    impl()->storage->emplace(rainy_typeid(Type).name(), []() { return &implements::inject_items<Type>; });
                    auto &vec = implements::inject_items<Type>;
                    vec.empalce_back(&implements::inject_item_object<Type, Impl, Args...>);
                }
            });
        }

        template <typename Type>
        static rain_fn is_this_type_available() -> bool {
            return impl()->storage->contains(rainy_typeid(Type).name());
        }

        template <typename Fx, type_traits::other_trans::enable_if_t<type_traits::primary_types::function_traits<Fx>::valid &&
                                                                         !type_traits::primary_types::is_member_function_pointer_v<Fx>,
                                                                     int> = 0>
        static rain_fn is_this_type_with_construct_args_type_available() -> bool {
            return is_this_type_with_construct_args_type_available_impl<Fx>(nullptr);
        }

        template <typename Type, typename... Args>
        static rain_fn get_instance(Args &&...args) -> Type * {
            if constexpr (sizeof...(Args) == 0) {
                if (!is_this_type_available<Type>()) {
                    return nullptr;
                }
                factory_key name = rainy_typeid(Type).name();
                rainy_let vec = impl()->storage->make_product(name);
                foundation::exceptions::throw_exception_if<foundation::exceptions::runtime::runtime_error>(
                    vec->size() == 1, "Cannot get instance unless you use qual args");
                return static_cast<Type *>(static_cast<implements::inject_item_layer<Type> *>(vec->at(0))->get());
            } else {
                std::size_t index{0};
                if (!is_this_type_with_construct_args_type_available_impl<Type(Args...)>(&index)) {
                    return nullptr;
                }
                rainy_let vec = impl()->storage->make_product(rainy_typeid(Type).name());
                return static_cast<Type *>(static_cast<implements::inject_item_layer<Type, Args...> *>(vec->at(index))
                                               ->get_signleton_with_args(utility::forward<Args>(args)...));
            }
        }

        template <typename Type, typename... Args>
        static rain_fn get_new_instance(Args &&...args) -> foundation::system::memory::nebula_ptr<Type> {
            factory_key name = rainy_typeid(Type).name();
            if (is_this_type_with_construct_args_type_available_impl<Type(Args...)>()) {
                return static_cast<implements::inject_item_layer<Type, Args...> *>(impl()->storage->make_product(name))
                    ->construct_new(utility::forward<Args>(args)...);
            }
            return nullptr;
        }

        template <typename Type, typename... Args>
        friend class injector;

    private:
        template <typename Fx, type_traits::other_trans::enable_if_t<type_traits::primary_types::function_traits<Fx>::valid &&
                                                                         !type_traits::primary_types::is_member_function_pointer_v<Fx>,
                                                                     int> = 0>
        static rain_fn is_this_type_with_construct_args_type_available_impl(std::size_t* active_index_p = nullptr) -> bool {
            using ret_t = type_traits::primary_types::function_return_type<Fx>;
            if (!impl()->storage->contains(rainy_typeid(ret_t).name())) {
                return false;
            }
            rainy_let vec = impl()->storage->make_product(rainy_typeid(ret_t).name());
            auto iter = std::find_if(vec->begin(), vec->end(),
                         [](implements::inject_item *value) { 
                    return value->impl_type() == rainy_typeid(Fx); });
            bool contains = iter != vec->end();
            if (active_index_p) {
                *active_index_p = utility::distance(iter, vec->begin());
            }
            return contains;
        }

        container(factory * fact) noexcept : storage{fact} {
        }

        static rain_fn impl() noexcept -> container *;

        factory* storage;
    };

    template <typename Type, typename... Args>
    class injector : type_traits::helper::non_copyable, type_traits::helper::non_moveable {
    public:
        injector(factory &factory) : fact{factory} {
        }

        template <typename Impl>
        void operator()(std::in_place_type_t<Impl>) {
            auto vec = &implements::inject_items<Type>;
            fact.emplace(rainy_typeid(Type).name(), [vec]() { return vec; });
            vec->emplace_back(&implements::inject_item_object<Type, Impl, Args...>);
        }

    private:
        factory &fact;
    };

    template <typename Type, typename... Args>
    injector<Type, Args...> inject_to(factory &factory) {
        return {factory};
    }
}

#define RAINY_INJECT_SERVICE(Type, Impl, ...)                                                                                         \
    namespace rainy::utility::ioc {                                                                                                   \
        template <>                                                                                                                   \
        struct factory_inject<datasource> {                                                                                           \
            void inject(factory &fact) {                                                                                              \
                inject_to<datasource, __VA_ARGS__>(fact)(std::in_place_type<datasource_impl>);                                        \
            }                                                                                                                         \
        };                                                                                                                            \
    }

#endif