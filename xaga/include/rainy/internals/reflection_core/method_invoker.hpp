#ifndef RAINY_REFLECTION_CORE_METHOD_INVOKER_HPP
#define RAINY_REFLECTION_CORE_METHOD_INVOKER_HPP
#include <rainy/foundation/functional/function_pointer.hpp>
#include <rainy/utility/any.hpp>

namespace rainy::foundation::reflection {
    enum class method_type {
        static_method,
        static_method_noexcept,
        normal_method,
        normal_method_noexcept,
        normal_method_volatile,
        normal_method_volatile_noexcept,
        normal_method_left,
        normal_method_left_noexcept,
        normal_method_left_volatile,
        normal_method_left_volatile_noexcept,
        normal_method_right,
        normal_method_right_noexcept,
        normal_method_right_volatile,
        normal_method_right_volatile_noexcept,
        const_method,
        const_method_noexcept,
        const_method_volatile,
        const_method_volatile_noexcept,
        const_method_left,
        const_method_left_noexcept,
        const_method_left_volatile,
        const_method_left_volatile_noexcept,
        const_method_right,
        const_method_right_noexcept,
        const_method_right_volatile,
        const_method_right_volatile_noexcept,
        placeholder_value
    };
}

// 避免静态函数封装时出现错误
#define RAINY_METHOD_INVOKER_VOID(CLASSNAME, QUAL)                                                                                    \
    template <typename ReturnType, typename... Args>                                                                                  \
    struct CLASSNAME<void, ReturnType, Args...> {                                                                                     \
        using any = utility::any;                                                                                                  \
        CLASSNAME() = default;                                                                                                        \
        template <typename Class>                                                                                                     \
        CLASSNAME(ReturnType (Class::*invoker)(Args...) QUAL) noexcept {                                                              \
        }                                                                                                                             \
        any invoke(void *, const std::vector<any> &) const {                                                                          \
            return {};                                                                                                                \
        }                                                                                                                             \
    };

#define RAINY_CONST_METHOD_INVOKER(CLASSNAME, QUAL)                                                                                   \
    template <typename Class, typename ReturnType, typename... Args>                                                                  \
    struct CLASSNAME {                                                                                                                \
        using any = utility::any;                                                                                                  \
        CLASSNAME() = default;                                                                                                        \
        CLASSNAME(ReturnType (Class::*invoker)(Args...) QUAL) noexcept : invoker(invoker) {                                           \
        }                                                                                                                             \
        any invoke(void *obj, const std::vector<utility::any> &params) const {                                                     \
            return invoke_impl(obj, invoker, params, std::index_sequence_for<Args...>{});                                             \
        }                                                                                                                             \
        template <std::size_t... I>                                                                                                   \
        static any invoke_impl(void *obj, ReturnType (Class::*method)(Args...) QUAL, const std::vector<any> &params,                  \
                               std::index_sequence<I...>) {                                                                           \
            if (sizeof...(Args) != params.size()) {                                                                                   \
                rainy::foundation::system::exceptions::runtime::throw_runtime_error(                                                  \
                    RAINY_STRINGIZE(CLASSNAME) " in invoke_impl(): Parameter count mismatch");                                        \
            }                                                                                                                         \
            if constexpr (std::is_void_v<ReturnType>) {                                                                               \
                (static_cast<const Class *>(obj)->*method)(utility::any_cast<Args>(params[I])...);                                    \
                return {};                                                                                                            \
            } else {                                                                                                                  \
                return any((static_cast<const Class *>(obj)->*method)(utility::any_cast<Args>(params[I])...));                        \
            }                                                                                                                         \
        }                                                                                                                             \
        ReturnType (Class::*invoker)(Args...) QUAL;                                                                                   \
    };                                                                                                                                \
    RAINY_METHOD_INVOKER_VOID(CLASSNAME, QUAL)


#define RAINY_CONST_NOEXCEPT_METHOD_INVOKER(CLASSNAME, QUAL)                                                                                \
    template <typename Class, typename ReturnType, typename... Args>                                                                  \
    struct CLASSNAME {                                                                                                                \
        using any = utility::any;                                                                                                  \
        CLASSNAME() = default;                                                                                                        \
        CLASSNAME(ReturnType (Class::*invoker)(Args...) QUAL noexcept) noexcept : invoker(invoker) {                                  \
        }                                                                                                                             \
        any invoke(void *obj, const std::vector<utility::any> &params) const {                                                     \
            return invoke_impl(obj, invoker, params, std::index_sequence_for<Args...>{});                                             \
        }                                                                                                                             \
        template <std::size_t... I>                                                                                                   \
        static any invoke_impl(void *obj, ReturnType (Class::*method)(Args...) QUAL noexcept, const std::vector<any> &params,         \
                               std::index_sequence<I...>) noexcept {                                                                  \
            utility::expects(sizeof...(Args) == params.size(),                                                                        \
                             RAINY_STRINGIZE(CLASSNAME) " in invoke_impl(): Parameter count mismatch");                               \
            if constexpr (std::is_void_v<ReturnType>) {                                                                               \
                (static_cast<const Class *>(obj)->*method)(utility::any_cast<Args>(params[I])...);                                    \
                return {};                                                                                                            \
            } else {                                                                                                                  \
                return any((static_cast<const Class *>(obj)->*method)(utility::any_cast<Args>(params[I])...));                        \
            }                                                                                                                         \
        }                                                                                                                             \
        ReturnType (Class::*invoker)(Args...) QUAL noexcept;                                                                          \
    };\
    RAINY_METHOD_INVOKER_VOID(CLASSNAME, QUAL)

#define RAINY_CONST_RIGHT_METHOD_INVOKER(CLASSNAME, QUAL)                                                                             \
    template <typename Class, typename ReturnType, typename... Args>                                                                  \
    struct CLASSNAME {                                                                                                                \
        using any = utility::any;                                                                                                  \
        CLASSNAME() = default;                                                                                                        \
        CLASSNAME(ReturnType (Class::*invoker)(Args...) QUAL) noexcept : invoker(invoker) {                                           \
        }                                                                                                                             \
        any invoke(void *obj, const std::vector<utility::any> &params) const {                                                     \
            return invoke_impl(obj, invoker, params, std::index_sequence_for<Args...>{});                                             \
        }                                                                                                                             \
        template <std::size_t... I>                                                                                                   \
        static any invoke_impl(void *obj, ReturnType (Class::*method)(Args...) QUAL, const std::vector<any> &params,                  \
                               std::index_sequence<I...>) {                                                                           \
            if (sizeof...(Args) != params.size()) {                                                                                   \
                rainy::foundation::system::exceptions::runtime::throw_runtime_error(                                                  \
                    RAINY_STRINGIZE(CLASSNAME) " in invoke_impl(): Parameter count mismatch");                                        \
            }                                                                                                                         \
            if constexpr (std::is_void_v<ReturnType>) {                                                                               \
                (static_cast<const Class &&>(*(static_cast<Class *>(obj))).*method)(utility::any_cast<Args>(params[I])...);                \
                return {};                                                                                                            \
            } else {                                                                                                                  \
                return any((static_cast<const Class &&>(*(static_cast<Class *>(obj))).*method)(utility::any_cast<Args>(params[I])...));    \
            }                                                                                                                         \
        }                                                                                                                             \
        ReturnType (Class::*invoker)(Args...) QUAL;                                                                                   \
    };                                                                                                                                \
    RAINY_METHOD_INVOKER_VOID(CLASSNAME, QUAL)

#define RAINY_CONST_RIGHT_NOEXCEPT_METHOD_INVOKER(CLASSNAME, QUAL)                                                                          \
    template <typename Class, typename ReturnType, typename... Args>                                                                  \
    struct CLASSNAME {                                                                                                                \
        using any = utility::any;                                                                                                  \
        CLASSNAME() = default;                                                                                                        \
        CLASSNAME(ReturnType (Class::*invoker)(Args...) QUAL noexcept) noexcept : invoker(invoker) {                                  \
        }                                                                                                                             \
        any invoke(void *obj, const std::vector<utility::any> &params) const {                                                     \
            return invoke_impl(obj, invoker, params, std::index_sequence_for<Args...>{});                                             \
        }                                                                                                                             \
        template <std::size_t... I>                                                                                                   \
        static any invoke_impl(void *obj, ReturnType (Class::*method)(Args...) QUAL noexcept, const std::vector<any> &params,         \
                               std::index_sequence<I...>) noexcept {                                                                  \
            utility::expects(sizeof...(Args) == params.size(),                                                                        \
                             RAINY_STRINGIZE(CLASSNAME) " in invoke_impl(): Parameter count mismatch");                               \
            if constexpr (std::is_void_v<ReturnType>) {                                                                               \
                (static_cast<const Class &&>(*(static_cast<Class *>(obj))).*method)(utility::any_cast<Args>(params[I])...);                \
                return {};                                                                                                            \
            } else {                                                                                                                  \
                return any((static_cast<const Class &&>(*(static_cast<Class *>(obj))).*method)(utility::any_cast<Args>(params[I])...));    \
            }                                                                                                                         \
        }                                                                                                                             \
        ReturnType (Class::*invoker)(Args...) QUAL noexcept;                                                                          \
    };                                                                                                                                \
    RAINY_METHOD_INVOKER_VOID(CLASSNAME, QUAL)


// nolr-qual [cmethod]
namespace rainy::foundation::reflection::implements {
    RAINY_CONST_METHOD_INVOKER(cmethod_invoker, const)
    RAINY_CONST_METHOD_INVOKER(cmethod_volatile_invoker,const volatile)
    RAINY_CONST_NOEXCEPT_METHOD_INVOKER(cmethod_noexcept_invoker,const)
    RAINY_CONST_NOEXCEPT_METHOD_INVOKER(cmethod_volatile_noexcept_invoker,const volatile)
}

// l-qual [cmethod]
namespace rainy::foundation::reflection::implements {
    RAINY_CONST_METHOD_INVOKER(cmethod_left_invoker,const &)
    RAINY_CONST_METHOD_INVOKER(cmethod_left_volatile_invoker,const volatile &)
    RAINY_CONST_NOEXCEPT_METHOD_INVOKER(cmethod_left_noexcept_invoker, const &)
    RAINY_CONST_NOEXCEPT_METHOD_INVOKER(cmethod_left_volatile_noexcept_invoker, const volatile &)
}

// r-qual [cmethod]
namespace rainy::foundation::reflection::implements {
    RAINY_CONST_RIGHT_METHOD_INVOKER(cmethod_right_invoker, const &&)
    RAINY_CONST_RIGHT_METHOD_INVOKER(cmethod_right_volatile_invoker, const volatile &&)
    RAINY_CONST_RIGHT_NOEXCEPT_METHOD_INVOKER(cmethod_right_noexcept_invoker, const &&)
    RAINY_CONST_RIGHT_NOEXCEPT_METHOD_INVOKER(cmethod_right_volatile_noexcept_invoker, const volatile &&)
}

#define RAINY_METHOD_INVOKER(CLASSNAME, QUAL)                                                                                         \
    template <typename Class, typename ReturnType, typename... Args>                                                                  \
    struct CLASSNAME {                                                                                                                \
        using any = utility::any;                                                                                                  \
        CLASSNAME() = default;                                                                                                        \
        CLASSNAME(ReturnType (Class::*invoker)(Args...) QUAL) noexcept : invoker(invoker) {                                           \
        }                                                                                                                             \
        any invoke(void *obj, const std::vector<utility::any> &params) const {                                                     \
            return invoke_impl(obj, invoker, params, std::index_sequence_for<Args...>{});                                             \
        }                                                                                                                             \
        template <std::size_t... I>                                                                                                   \
        static any invoke_impl(void *obj, ReturnType (Class::*method)(Args...) QUAL, const std::vector<any> &params,                  \
                               std::index_sequence<I...>) {                                                                           \
            if (sizeof...(Args) != params.size()) {                                                                                   \
                rainy::foundation::system::exceptions::runtime::throw_runtime_error(                                                  \
                    RAINY_STRINGIZE(CLASSNAME) " in invoke_impl(): Parameter count mismatch");                                        \
            }                                                                                                                         \
            if constexpr (std::is_void_v<ReturnType>) {                                                                               \
                (static_cast<Class *>(obj)->*method)(utility::any_cast<Args>(params[I])...);                                          \
                return {};                                                                                                            \
            } else {                                                                                                                  \
                return any((static_cast<Class *>(obj)->*method)(utility::any_cast<Args>(params[I])...));                              \
            }                                                                                                                         \
        }                                                                                                                             \
        ReturnType (Class::*invoker)(Args...) QUAL;                                                                                   \
    };                                                                                                                                \
    RAINY_METHOD_INVOKER_VOID(CLASSNAME, QUAL)


#define RAINY_NOEXCEPT_METHOD_INVOKER(CLASSNAME, QUAL)                                                                                \
    template <typename Class, typename ReturnType, typename... Args>                                                                  \
    struct CLASSNAME {                                                                                                                \
        using any = utility::any;                                                                                                  \
        CLASSNAME() = default;                                                                                                        \
        CLASSNAME(ReturnType (Class::*invoker)(Args...) QUAL noexcept) noexcept : invoker(invoker) {                                  \
        }                                                                                                                             \
        any invoke(void *obj, const std::vector<utility::any> &params) const {                                                     \
            return invoke_impl(obj, invoker, params, std::index_sequence_for<Args...>{});                                             \
        }                                                                                                                             \
        template <std::size_t... I>                                                                                                   \
        static any invoke_impl(void *obj, ReturnType (Class::*method)(Args...) QUAL noexcept, const std::vector<any> &params,         \
                               std::index_sequence<I...>) noexcept {                                                                  \
            utility::expects(sizeof...(Args) == params.size(),                                                                        \
                             RAINY_STRINGIZE(CLASSNAME) " in invoke_impl(): Parameter count mismatch");                               \
            if constexpr (std::is_void_v<ReturnType>) {                                                                               \
                (static_cast<Class *>(obj)->*method)(utility::any_cast<Args>(params[I])...);                                          \
                return {};                                                                                                            \
            } else {                                                                                                                  \
                return any((static_cast<Class *>(obj)->*method)(utility::any_cast<Args>(params[I])...));                              \
            }                                                                                                                         \
        }                                                                                                                             \
        ReturnType (Class::*invoker)(Args...) QUAL noexcept;                                                                          \
    };                                                                                                                                \
    RAINY_METHOD_INVOKER_VOID(CLASSNAME, QUAL)


#define RAINY_RIGHT_METHOD_INVOKER(CLASSNAME, QUAL)                                                                             \
    template <typename Class, typename ReturnType, typename... Args>                                                                  \
    struct CLASSNAME {                                                                                                                \
        using any = utility::any;                                                                                                  \
        CLASSNAME() = default;                                                                                                        \
        CLASSNAME(ReturnType (Class::*invoker)(Args...) QUAL) noexcept : invoker(invoker) {                                           \
        }                                                                                                                             \
        any invoke(void *obj, const std::vector<utility::any> &params) const {                                                     \
            return invoke_impl(obj, invoker, params, std::index_sequence_for<Args...>{});                                             \
        }                                                                                                                             \
        template <std::size_t... I>                                                                                                   \
        static any invoke_impl(void *obj, ReturnType (Class::*method)(Args...) QUAL, const std::vector<any> &params,                  \
                               std::index_sequence<I...>) {                                                                           \
            if (sizeof...(Args) != params.size()) {                                                                                   \
                rainy::foundation::system::exceptions::runtime::throw_runtime_error(                                                  \
                    RAINY_STRINGIZE(CLASSNAME) " in invoke_impl(): Parameter count mismatch");                                        \
            }                                                                                                                         \
            if constexpr (std::is_void_v<ReturnType>) {                                                                               \
                (static_cast<Class &&>(*(static_cast<Class *>(obj))).*method)(utility::any_cast<Args>(params[I])...);           \
                return {};                                                                                                            \
            } else {                                                                                                                  \
                return any(                                                                                                           \
                    (static_cast<Class &&>(*(static_cast<Class *>(obj))).*method)(utility::any_cast<Args>(params[I])...));      \
            }                                                                                                                         \
        }                                                                                                                             \
        ReturnType (Class::*invoker)(Args...) QUAL;                                                                                   \
    };                                                                                                                                \
    RAINY_METHOD_INVOKER_VOID(CLASSNAME, QUAL)

#define RAINY_RIGHT_NOEXCEPT_METHOD_INVOKER(CLASSNAME, QUAL)                                                                    \
    template <typename Class, typename ReturnType, typename... Args>                                                                  \
    struct CLASSNAME {                                                                                                                \
        using any = utility::any;                                                                                                  \
        CLASSNAME() = default;                                                                                                        \
        CLASSNAME(ReturnType (Class::*invoker)(Args...) QUAL noexcept) noexcept : invoker(invoker) {                                  \
        }                                                                                                                             \
        any invoke(void *obj, const std::vector<utility::any> &params) const {                                                     \
            return invoke_impl(obj, invoker, params, std::index_sequence_for<Args...>{});                                             \
        }                                                                                                                             \
        template <std::size_t... I>                                                                                                   \
        static any invoke_impl(void *obj, ReturnType (Class::*method)(Args...) QUAL noexcept, const std::vector<any> &params,         \
                               std::index_sequence<I...>) noexcept {                                                                  \
            utility::expects(sizeof...(Args) == params.size(),                                                                        \
                             RAINY_STRINGIZE(CLASSNAME) " in invoke_impl(): Parameter count mismatch");                               \
            if constexpr (std::is_void_v<ReturnType>) {                                                                               \
                (static_cast<Class &&>(*(static_cast<Class *>(obj))).*method)(utility::any_cast<Args>(params[I])...);           \
                return {};                                                                                                            \
            } else {                                                                                                                  \
                return any(                                                                                                           \
                    (static_cast<Class &&>(*(static_cast<Class *>(obj))).*method)(utility::any_cast<Args>(params[I])...));      \
            }                                                                                                                         \
        }                                                                                                                             \
        ReturnType (Class::*invoker)(Args...) QUAL noexcept;                                                                          \
    };                                                                                                                                \
    RAINY_METHOD_INVOKER_VOID(CLASSNAME, QUAL)



// nolr-qual [method]
namespace rainy::foundation::reflection::implements {
    RAINY_METHOD_INVOKER(method_invoker,)
    RAINY_METHOD_INVOKER(method_volatile_invoker, volatile)
    RAINY_NOEXCEPT_METHOD_INVOKER(method_noexcept_invoker,)
    RAINY_NOEXCEPT_METHOD_INVOKER(method_volatile_noexcept_invoker, volatile)
}

// l-qual [method]
namespace rainy::foundation::reflection::implements {
    RAINY_METHOD_INVOKER(method_left_invoker, &)
    RAINY_METHOD_INVOKER(method_left_volatile_invoker, volatile &)
    RAINY_NOEXCEPT_METHOD_INVOKER(method_left_noexcept_invoker, &)
    RAINY_NOEXCEPT_METHOD_INVOKER(method_left_volatile_noexcept_invoker, volatile &)
}

// r-qual [method]
namespace rainy::foundation::reflection::implements {
    RAINY_RIGHT_METHOD_INVOKER(method_right_invoker, &&)
    RAINY_RIGHT_METHOD_INVOKER(method_right_volatile_invoker, volatile &&)
    RAINY_RIGHT_NOEXCEPT_METHOD_INVOKER(method_right_noexcept_invoker, &&)
    RAINY_RIGHT_NOEXCEPT_METHOD_INVOKER(method_right_volatile_noexcept_invoker, volatile &&)
}

// [static_method]
namespace rainy::foundation::reflection::implements {
    template <typename ReturnType, typename... Args>
    struct static_invoker {
        using any = utility::any;

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
                return collections::make_any<ReturnType>(method(rainy::utility::any_cast<Args>(params[I])...));
            }
        }

        ReturnType (*invoker)(Args...);
    };

    template <typename ReturnType, typename... Args>
    struct static_noexcept_invoker {
        using any = utility::any;

        static_noexcept_invoker() = default;

        static_noexcept_invoker(ReturnType (*invoker)(Args...)) noexcept : invoker(invoker) {
        }

        any invoke(const std::vector<any> &params) const {
            return invoke_impl(invoker, params, std::index_sequence_for<Args...>{});
        }

        template <size_t... I>
        static any invoke_impl(ReturnType (*method)(Args...) noexcept, const std::vector<any> &params,
                               std::index_sequence<I...>) noexcept {
            utility::expects(sizeof...(Args) == params.size(), "static_noexcept_invoker in invoke_impl(): Parameter count mismatch");
            if constexpr (std::is_void_v<ReturnType>) {
                method(rainy::utility::any_cast<Args>(params[I])...);
                return {};
            } else {
                return collections::make_any<ReturnType>(method(rainy::utility::any_cast<Args>(params[I])...));
            }
        }

        ReturnType (*invoker)(Args...) noexcept;
    };
}

namespace rainy::foundation::reflection::implements {
    template <typename Class, typename ReturnType, typename... Args>
    union /* 预定义的invoker */ union_method_container {
        static_invoker<ReturnType, Args...> static_method; // static_method
        static_noexcept_invoker<ReturnType, Args...> static_method_noexcept;

        method_invoker<Class, ReturnType, Args...> normal_method; // normal_method
        method_noexcept_invoker<Class, ReturnType, Args...> normal_method_noexcept; // normal_method_noexcept
        method_volatile_invoker<Class, ReturnType, Args...> normal_method_volatile; // normal_method_volatile
        method_volatile_noexcept_invoker<Class, ReturnType, Args...>
            normal_method_volatile_noexcept; // normal_method_volatile_noexcept

        method_left_invoker<Class, ReturnType, Args...> normal_method_left; // normal_method_left
        method_left_noexcept_invoker<Class, ReturnType, Args...> normal_method_left_noexcept; // normal_method_left_noexcept
        method_left_volatile_invoker<Class, ReturnType, Args...> normal_method_left_volatile; // normal_method_left_volatile
        method_left_volatile_noexcept_invoker<Class, ReturnType, Args...>
            normal_method_left_volatile_noexcept; // normal_method_left_volatile_noexcept

        method_right_invoker<Class, ReturnType, Args...> normal_method_right; // normal_method_right
        method_right_noexcept_invoker<Class, ReturnType, Args...> normal_method_right_noexcept; // normal_method_right_noexcept
        method_right_volatile_invoker<Class, ReturnType, Args...> normal_method_right_volatile; // normal_method_right_volatile
        method_right_volatile_noexcept_invoker<Class, ReturnType, Args...>
            normal_method_right_volatile_noexcept; // normal_method_right_volatile_noexcept

        cmethod_invoker<Class, ReturnType, Args...> const_method; // const_method
        cmethod_noexcept_invoker<Class, ReturnType, Args...> const_method_noexcept; // const_method_noexcept
        cmethod_volatile_invoker<Class, ReturnType, Args...> const_method_volatile; // const_method_volatile
        cmethod_volatile_invoker<Class, ReturnType, Args...> const_method_volatile_noexcept; // const_volatile_noexcept_method

        cmethod_left_invoker<Class, ReturnType, Args...> const_method_left; // const_method_left
        cmethod_left_noexcept_invoker<Class, ReturnType, Args...> const_method_left_noexcept; // const_method_left_noexcept
        cmethod_left_volatile_invoker<Class, ReturnType, Args...> const_method_left_volatile; // const_method_left_volatile
        cmethod_left_volatile_noexcept_invoker<Class, ReturnType, Args...>
            const_method_left_volatile_noexcept; // const_method_left_noexcept_volatile

        cmethod_right_invoker<Class, ReturnType, Args...> const_method_right; // const_method_right
        cmethod_right_noexcept_invoker<Class, ReturnType, Args...> const_method_right_noexcept; // const_method_right_noexcept
        cmethod_right_volatile_invoker<Class, ReturnType, Args...> const_method_right_volatile; // const_method_right_volatile
        cmethod_right_volatile_noexcept_invoker<Class, ReturnType, Args...>
            const_method_right_volatile_noexcept; // const_method_right_noexcept_volatile
    };

    template <typename Class, typename ReturnType, typename... Args>
    utility::any call_cmethod(const union_method_container<Class, ReturnType, Args...> &storage, method_type type, void *object,
                                 const std::vector<utility::any> &params) {
        switch (type) {
            case method_type::const_method:
                return storage.const_method.invoke(object, params);
            case method_type::const_method_noexcept:
                return storage.const_method_noexcept.invoke(object, params);
            case method_type::const_method_volatile:
                return storage.const_method_volatile.invoke(object, params);
            case method_type::const_method_volatile_noexcept:
                return storage.const_method_volatile_noexcept.invoke(object, params);
            default:
                break;
        }
        return {};
    }

    template <typename Class, typename ReturnType, typename... Args>
    utility::any call_cmethod_left(const union_method_container<Class, ReturnType, Args...> &storage, method_type type,
                                      void *object, const std::vector<utility::any> &params) {
        switch (type) {
            case method_type::const_method_left:
                return storage.const_method_left.invoke(object, params);
            case method_type::const_method_left_noexcept:
                return storage.const_method_left_noexcept.invoke(object, params);
            case method_type::const_method_left_volatile:
                return storage.const_method_left_volatile.invoke(object, params);
            case method_type::const_method_left_volatile_noexcept:
                return storage.const_method_left_volatile_noexcept.invoke(object, params);
            default:
                break;
        }
        return {};
    }

    template <typename Class, typename ReturnType, typename... Args>
    utility::any call_cmethod_right(const union_method_container<Class, ReturnType, Args...> &storage, method_type type,
                                       void *object, const std::vector<utility::any> &params) {
        switch (type) {
            case method_type::const_method_left:
                return storage.const_method_left.invoke(object, params);
            case method_type::const_method_left_noexcept:
                return storage.const_method_left_noexcept.invoke(object, params);
            case method_type::const_method_left_volatile:
                return storage.const_method_left_volatile.invoke(object, params);
            case method_type::const_method_left_volatile_noexcept:
                return storage.const_method_left_volatile_noexcept.invoke(object, params);
            default:
                break;
        }
        return {};
    }

    template <typename Class, typename ReturnType, typename... Args>
    utility::any call_normal_method(const union_method_container<Class, ReturnType, Args...> &storage, method_type type,
                                       void *object, const std::vector<utility::any> &params) {
        switch (type) {
            case method_type::normal_method:
                return storage.normal_method.invoke(object, params);
            case method_type::normal_method_noexcept:
                return storage.normal_method_noexcept.invoke(object, params);
            case method_type::normal_method_volatile:
                return storage.normal_method_volatile.invoke(object, params);
            case method_type::normal_method_volatile_noexcept:
                return storage.normal_method_volatile_noexcept.invoke(object, params);
            default:
                break;
        }
        return {};
    }

    template <typename Class, typename ReturnType, typename... Args>
    utility::any call_normal_method_left(const union_method_container<Class, ReturnType, Args...> &storage, method_type type,
                                            void *object, const std::vector<utility::any> &params) {
        switch (type) {
            case method_type::normal_method_left:
                return storage.normal_method_left.invoke(object, params);
            case method_type::normal_method_left_noexcept:
                return storage.normal_method_left_noexcept.invoke(object, params);
            case method_type::normal_method_left_volatile:
                return storage.normal_method_left_volatile.invoke(object, params);
            case method_type::normal_method_left_volatile_noexcept:
                return storage.normal_method_left_volatile_noexcept.invoke(object, params);
            default:
                break;
        }
        return {};
    }

    template <typename Class, typename ReturnType, typename... Args>
    utility::any call_normal_method_right(const union_method_container<Class, ReturnType, Args...> &storage, method_type type,
                                             void *object, const std::vector<utility::any> &params) {
        switch (type) {
            case method_type::normal_method_right:
                return storage.normal_method_right.invoke(object, params);
            case method_type::normal_method_right_noexcept:
                return storage.normal_method_right_noexcept.invoke(object, params);
            case method_type::normal_method_right_volatile:
                return storage.normal_method_right_volatile.invoke(object, params);
            case method_type::normal_method_right_volatile_noexcept:
                return storage.normal_method_right_volatile_noexcept.invoke(object, params);
            default:
                break;
        }
        return {};
    }
}

#endif