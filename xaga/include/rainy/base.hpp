#ifndef RAINY_BASE_HPP
#define RAINY_BASE_HPP
/*
文件名: base.hpp
此头文件用于存放基础设施实现，提供对C语言的部分函数封装以及少量模块
*/
#include <rainy/core/core.hpp>
#include <rainy/foundation/io/stream_print.hpp>
#include <rainy/text/format_wrapper.hpp>
#include <rainy/foundation/functional/function_pointer.hpp>
#include <rainy/foundation/diagnostics/contract.hpp>
#include <rainy/utility/iterator.hpp>
#include <rainy/text/char_traits.hpp>
#include <rainy/foundation/system/memory/allocator.hpp>

/* standard-libray header */
#include <algorithm>
#include <array>
#include <bitset>
#include <cerrno>
#include <cstdarg>
#include <cstring>
#include <deque>
#include <functional>
#include <future>
#include <map>
#include <memory>
#include <mutex>
#include <list>
#include <queue>
#include <shared_mutex>
#include <stdexcept>
#include <string>
#include <string_view>
#include <thread>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>
#include <memory_resource>

#if RAINY_HAS_CXX20
#include <format>
#include <source_location>
#endif

#if RAINY_USING_WINDOWS
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

#if RAINY_HAS_CXX20
template <typename Ty>
class std::formatter<rainy::utility::reference_wrapper<Ty>, char> // NOLINT
{
public:
    explicit formatter() noexcept = default;

    auto parse(format_parse_context &ctx) const noexcept {
        return ctx.begin();
    }

    auto format(const rainy::utility::reference_wrapper<Ty> &value, std::format_context fc) const noexcept {
        return std::format_to(fc.out(), "{}", value.get());
    }
};
#endif

namespace rainy::component {
    template <typename Class>
    class object {
    public:
        using object_type = Class;
        using super_type = object<Class>;

        static_assert(std::is_class_v<object_type> && !std::is_array_v<object_type>);

        RAINY_NODISCARD const std::type_info &type_info() const noexcept {
            return typeid(object_type);
        }

        RAINY_NODISCARD std::size_t hash_code() const noexcept {
            rainy_let object_address = reinterpret_cast<const unsigned char *>(&object_dummy_unused);
            return utility::implements::fnv1a_append_bytes(utility::implements::fnv_offset_basis, object_address,
                                                          sizeof(object<object_type>));
        }

        RAINY_NODISCARD const void *addressof() const noexcept {
            return static_cast<const void *>(this);
        }

        RAINY_NODISCARD virtual const super_type &as_super() const noexcept {
            return *this;
        }

        RAINY_NODISCARD virtual std::shared_ptr<object_type> clone() const noexcept(std::is_nothrow_copy_constructible_v<object_type>) {
            if (std::is_copy_constructible_v<object_type>) {
                return std::shared_ptr<object_type>(new object_type(*static_cast<const object_type *>(this)));
            }
            foundation::system::exceptions::runtime::throw_runtime_error("We can't create a copy from this class");
            return nullptr;
        }

    protected:
        constexpr object() noexcept = default;
        virtual RAINY_CONSTEXPR20 ~object() = default;

    private:
        union {
            std::max_align_t object_dummy_unused{};
        };
    };
}

namespace rainy::utility {
    

    template <>
    struct hash<type_index> {
        using argument_type = type_index;
        using result_type = std::size_t;

        static std::size_t hash_this_val(const argument_type &val) noexcept {
            return val.hash_code();
        }

        RAINY_NODISCARD std::size_t operator()(const type_index &val) const noexcept {
            return val.hash_code();
        }
    };
}











/*
仿函数实现
*/
namespace rainy::foundation::functional {        

    namespace predicate {
        
    }
}

namespace rainy::utility {
    /**
     * @brief 用于将结果和资源进行绑定以便快捷返回.
     * @tparam _error 函数返回的结果，此处应当尽可能使用枚举或整型数据
     * @tparam _type 用于提供实际返回资源
     * @note 该模板结构体根据 _type 是否具有默认构造函数提供不同的实现。
     */
    template <typename Result, typename Type, bool = std::is_default_constructible_v<Type>>
    struct result_collection {
        Result result;
        Type data;
    };

    /**
     * @brief 特化版本，用于 _type 不具有默认构造函数的情况。
     * @tparam _error 函数返回的结果，应当使用枚举或整型数据。
     * @tparam _type 用于提供实际返回资源。
     */
    template <typename Result, typename Type>
    struct result_collection<Result, Type, false> {
        template <typename... Args>
        explicit result_collection(Result result,
                                   Args &&...construct_args) noexcept(std::conjunction_v<std::is_nothrow_default_constructible<Type>>) :
            result(result), data(utility::forward<Args>(construct_args)...) {
        }

        Result result;
        Type data;
    };
}

namespace rainy::utility {
    template <typename Ty>
    struct not_null {
    public:
        using pointer = Ty;

        // static_assert(type_traits::primary_types::is_pointer_v<Ty>, "Ty must be a pointer!");

        not_null() = delete;

        constexpr not_null(std::nullptr_t) = delete;

        constexpr not_null(pointer resource) noexcept : resource(resource) {
            ensures(resource != nullptr, "resource cannot be a null pointer!");
        }

        constexpr decltype(auto) operator->() const {
            return resource;
        }

        constexpr decltype(auto) operator*() const {
            return *resource;
        }

    private:
        pointer resource;
    };
}


namespace rainy::utility {
    template <typename Fx>
    class mem_fn_impl;

    template <typename Rx, typename Class, typename... Args>
    class mem_fn_impl<Rx (Class::*)(Args...)> {
    public:
        using return_type = Rx;
        using class_type = Class;
        using pointer = return_type (class_type::*)(Args...);

        mem_fn_impl() = delete;

        mem_fn_impl(Rx (Class::*mem_fn)(Args...)) noexcept : fn(mem_fn) {
        }

        return_type invoke(class_type &object, Args &&...args) const {
            return (object.*fn)(std::forward<Args>(args)...);
        }

        return_type operator()(class_type &object, Args &&...args) const {
            return (object.*fn)(std::forward<Args>(args)...);
        }

        RAINY_NODISCARD std::type_info &type() const noexcept {
            return get_typeid<pointer>();
        }

        RAINY_NODISCARD std::size_t arity() const noexcept {
            return sizeof...(Args);
        }

    private:
        union {
            pointer fn;
            std::max_align_t dummy;
        };
    };

    template <typename Rx, typename Class, typename... Args>
    class mem_fn_impl<Rx (Class::*)(Args...) const> {
    public:
        using return_type = Rx;
        using class_type = Class;
        using pointer = return_type (class_type::*)(Args...) const;

        mem_fn_impl() = delete;

        mem_fn_impl(Rx (Class::*mem_fn)(Args...) const) noexcept : fn(mem_fn) {
        }

        return_type invoke(const class_type &object, Args &&...args) const {
            return (object.*fn)(std::forward<Args>(args)...);
        }

        return_type operator()(class_type &object, Args &&...args) const {
            return (object.*fn)(std::forward<Args>(args)...);
        }

        RAINY_NODISCARD std::type_info &type() const noexcept {
            return get_typeid<pointer>();
        }

        RAINY_NODISCARD std::size_t arity() const noexcept {
            return sizeof...(Args);
        }

    private:
        union {
            pointer fn;
            std::max_align_t dummy;
        };
    };

    template <typename Rx, typename Class, typename... Args>
    auto mem_fn(Rx (Class::*memptr)(Args...)) -> mem_fn_impl<Rx (Class::*)(Args...)> {
        return {memptr};
    }

    template <typename Rx, typename Class, typename... Args>
    auto mem_fn(Rx (Class::*memptr)(Args...) const) -> mem_fn_impl<Rx (Class::*)(Args...) const> {
        return {memptr};
    }
}



namespace rainy::component::sync_event {
    class event {
    public:
        virtual ~event() = default;

        virtual const std::type_info &type_info() {
            return typeid(*this);
        }
    };

    template <typename Derived>
    class event_handler {
    public:
        event_handler() {
            static_assert(std::is_base_of_v<event, Derived>);
        }

        virtual ~event_handler() = default;

        virtual void on(event &user_event) = 0;

        RAINY_NODISCARD std::string_view handler_name() const noexcept {
            return typeid(*this).name();
        }

        RAINY_NODISCARD std::size_t handler_hash_code() const noexcept {
            return typeid(*this).hash_code();
        }

        void dispatch(event &user_event) {
            on(dynamic_cast<Derived &>(user_event));
        }
    };

    class handler_registration {
    public:
        virtual ~handler_registration() = default;

        virtual void remove_handler() = 0;
        RAINY_NODISCARD virtual std::string_view name() const noexcept = 0;
        RAINY_NODISCARD virtual std::size_t hash_code() const noexcept = 0;
        virtual void invoke_dispatch(event &) const = 0;
    };

    class registration : public handler_registration, public std::enable_shared_from_this<registration> {
    public:
        using registrations_t = std::list<std::shared_ptr<registration>>;

        registration(std::shared_ptr<registrations_t> registrations) :
                registrations(utility::move(registrations)) {
        }

        void remove_handler() override {
            if (registered) {
                auto self = shared_from_this();
                registrations->remove_if([self](const std::shared_ptr<registration> &reg) { return reg == self; });
                registered = false;
            }
        }

        RAINY_NODISCARD std::string_view name() const noexcept override {
            return instance->name();
        }

        RAINY_NODISCARD std::size_t hash_code() const noexcept override {
            return instance->hash_code();
        }

        void invoke_dispatch(event &user_event) const noexcept override {
            return instance->invoke(user_event);
        }

        void *get_handler() {
            return instance->get_handler();
        }

        template <typename EventType>
        void set(std::shared_ptr<event_handler<EventType>> handler) {
            instance = std::make_unique<impl<EventType>>(handler);
        }

    private:
        struct resource {
            virtual ~resource() = default;
            virtual void *get_handler() = 0;
            RAINY_NODISCARD virtual std::string_view name() const noexcept = 0;
            RAINY_NODISCARD virtual std::size_t hash_code() const noexcept = 0;
            virtual void invoke(event &) const = 0;
        };

        template <typename EventType>
        struct impl : resource {
            impl(std::shared_ptr<event_handler<EventType>> handler) : handler(std::move(handler)) {
            }

            void *get_handler() override {
                return handler.get();
            }

            RAINY_NODISCARD std::string_view name() const noexcept override {
                return handler->handler_name();
            }

            RAINY_NODISCARD std::size_t hash_code() const noexcept override {
                return handler->handler_hash_code();
            }

            void invoke(event &user_event) const override {
                handler->dispatch(user_event);
            }

            std::shared_ptr<event_handler<EventType>> handler;
        };

        std::shared_ptr<registrations_t> registrations;
        bool registered{true};
        std::unique_ptr<resource> instance;
    };

    class dispatcher {
    public:
        using registrations_t = std::list<std::shared_ptr<registration>>;
        using type_mapping = std::unordered_map<utility::type_index, std::shared_ptr<registrations_t>>;

        static dispatcher *instance() {
            static dispatcher instance;
            return &instance;
        }

        dispatcher() {
            handlers.reserve(16);
        }

        ~dispatcher() {
            clear();
        }

        void clear() {
            handlers.clear();
        }

        template <typename EventType, typename ListenerType, typename... Args>
        RAINY_NODISCARD std::shared_ptr<handler_registration> subscribe(Args &&...args) {
            return subscribe<EventType>(std::make_shared<ListenerType>(utility::forward<Args>(args)...));
        }

        template <typename EventType>
        RAINY_NODISCARD std::shared_ptr<handler_registration> subscribe(std::shared_ptr<event_handler<EventType>> handler) {
            auto &registrations_instance = handlers[typeid(EventType)];
            if (!registrations_instance) {
                registrations_instance = std::make_shared<registrations_t>();
            }
            auto registration_instance = std::make_shared<registration>(registrations_instance);
            registration_instance->set<EventType>(handler);
            registrations_instance->emplace_back(registration_instance);
            return registration_instance;
        }

        template <typename EventType, typename Fx,
                  type_traits::other_trans::enable_if_t<
                      type_traits::type_properties::is_invocable_r_v<void, Fx, event &>, int> = 0>
        RAINY_NODISCARD std::shared_ptr<handler_registration> subscribe(Fx &&func) {
            /* 我们实际创建了一个virtual_listener实例，表示虚拟的监听器 */
            struct virtual_listener : public event_handler<EventType>, Fx {
                virtual_listener(Fx &&func) : Fx(std::forward<Fx>(func)) {
                }

                void on(event &user_event) override {
                    (*this)(user_event);
                }
            };
            return subscribe<EventType, virtual_listener>(std::forward<Fx>(func));
        }

        template <typename EventType>
        RAINY_NODISCARD std::shared_ptr<handler_registration> subscribe(
            foundation::functional::function_pointer<void(event &)> fptr) {
            struct virtual_listener : public event_handler<EventType> {
                virtual_listener(foundation::functional::function_pointer<void(event &)> fptr) : fptr(std::move(fptr)) {
                }

                void on(event &user_event) override {
                    fptr(user_event);
                    anti_jesus(); // 防止静态工具的const建议
                }

                void anti_jesus() {
                    jesus = 666;
                }

                foundation::functional::function_pointer<void(event &)> fptr;
                int jesus{33};
            };
            return subscribe<EventType, virtual_listener>(fptr);
        }

        template <typename EventType>
        void for_each_in_handlers(foundation::functional::function_pointer<void(event_handler<EventType> *)> pred) const {
            const auto it = handlers.find(typeid(EventType));
            if (it == handlers.end()) {
                return;
            }
            const auto &registrations_instance = it->second;
            if (!registrations_instance || registrations_instance->empty()) {
                return;
            }
            for (const auto &handler: *registrations_instance) {
                pred(static_cast<event_handler<EventType> *>(handler->get_handler()));
            }
        }

        template <typename EventType>
        void publish(EventType &event) const {
            const auto it = handlers.find(typeid(EventType));
            if (it == handlers.end()) {
                return;
            }
            const auto &registrations_instance = it->second;
            if (!registrations_instance || registrations_instance->empty()) {
                return;
            }
            for (const auto &handler: *registrations_instance) {
                try {
                    static_cast<event_handler<EventType> *>(handler->get_handler())->dispatch(event);
                } catch (foundation::system::exceptions::exception &RAINY_exception) {
                    foundation::io::stderr_println(
                        "found a error in :" RAINY_STRINGIZE(rainy::component::sync_event::dispatcher::publish),
                        "with addtional core from RAINY_exception: ", RAINY_exception.what());
                    utility::throw_exception(RAINY_exception);
                } catch (std::exception &e) {
                    foundation::io::stderr_println(
                        "found a error in :" RAINY_STRINGIZE(rainy::component::sync_event::dispatcher::publish),
                        "with addtional core from std::exception: ", e.what());
                    utility::throw_exception(utility::stdexcept_to_rexcept(e));
                }
            }
        }

        template <typename EventType>
        void publish_noexcept(EventType &event) const noexcept {
            const auto it = handlers.find(typeid(EventType));
            if (it == handlers.end()) {
                return;
            }
            const auto &registrations_instance = it->second;
            if (!registrations_instance || registrations_instance->empty()) {
                return;
            }
            for (const auto &handler: *registrations_instance) {
                handler->invoke_dispatch(event);
            }
        }
    private:
        type_mapping handlers;
    };
}


#endif // RAINY_BASE_HPP
