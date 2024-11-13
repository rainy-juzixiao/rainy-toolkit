#include <variant>
#include <unordered_set>
#include <sstream>

#include <rainy/base.hpp>
#include <rainy/containers/any.hpp>
#include <rainy/meta/type_traits.hpp>

namespace rainy::type_traits::type_properties {
    template <typename T>
    struct method_traits {
        static constexpr bool is_const = false;
        static constexpr bool is_static = false;
        using return_type = void;
    };

    template <typename Class, typename ReturnType, typename... Args>
    struct method_traits<ReturnType (Class::*)(Args...)> {
        static constexpr bool is_const = false;
        static constexpr bool is_static = false;
        using return_type = ReturnType;
    };

    template <typename Class, typename ReturnType, typename... Args>
    struct method_traits<ReturnType (Class::*)(Args...) const> {
        static constexpr bool is_const = true;
        static constexpr bool is_static = false;
        using return_type = ReturnType;
    };

    template <typename ReturnType, typename... Args>
    struct method_traits<ReturnType (*)(Args...)> {
        static constexpr bool is_const = false;
        static constexpr bool is_static = true;
        using return_type = ReturnType;
    };

    template <typename Ty>
    struct member_pointer_class;

    // 偏特化用于成员指针（例如 T C::*）
    template <typename Class, typename Ty>
    struct member_pointer_class<Ty Class::*> {
        using type = Class;
    };
}

namespace rainy::experimental::reflection {
    struct property_info {
        property_info() = default;

        property_info(const property_info &other) = default;

        property_info(const std::string_view name, const std::string_view type, const bool is_const, const bool is_static,
                      std::function<rainy::containers::any(const void *)> getter,
                      std::function<void(void *, const rainy::containers::any &)> setter) :
            name(name),
            type(type), is_const(is_const), is_static(is_static), getter(std::move(getter)), setter(std::move(setter)) {
        }

        std::string_view name;
        std::string_view type;
        bool is_const{};
        bool is_static{};
        std::function<rainy::containers::any(const void *)> getter;
        std::function<void(void *, const rainy::containers::any &)> setter;
    };

    struct method_info {
        explicit operator bool() const noexcept {
            return static_cast<bool>(invoker);
        }

        std::string_view name;
        std::string_view return_type;
        std::vector<std::string_view> param_types;
        bool is_const;
        bool is_static;
        std::function<rainy::containers::any(void *, const std::vector<rainy::containers::any> &)> invoker;
    };

    struct enum_info {
        constexpr enum_info() = default;

        template <typename Enum>
        enum_info(std::string_view name, Enum value) : name(name), value(value) {
        }

        std::string_view name;
        std::variant<int, unsigned int, long, unsigned long, long long, unsigned long long> value;
    };

    class class_typeid {
    public:
        friend class reflection;
        friend class shared_object;

        class_typeid(const std::type_info &info, const bool is_polymorphic, const std::size_t size, const std::size_t align,
                     class_typeid *parent_class_typeid = nullptr) :
            _class_name(info.name()),
            _is_base(parent_class_typeid == nullptr), _is_polymorphic(is_polymorphic), _size(size), _align(align),
            _hash_code(info.hash_code()), _parent_class_typeid({}) {
            if (parent_class_typeid) {
                _parent_class_typeid.emplace_back(parent_class_typeid);
            }
        }

        RAINY_NODISCARD std::string_view class_name() const noexcept {
            return _class_name;
        }

        RAINY_NODISCARD bool is_base() const noexcept {
            return _is_base;
        }

        RAINY_NODISCARD bool is_polymorphic() const noexcept {
            return _is_polymorphic;
        }

        RAINY_NODISCARD std::size_t size() const noexcept {
            return _size;
        }

        RAINY_NODISCARD std::size_t align() const noexcept {
            return _align;
        }

        RAINY_NODISCARD std::size_t hash_code() const noexcept {
            return _hash_code;
        }

        RAINY_NODISCARD bool equals(const class_typeid &right) const noexcept {
            return _size == right._size && _align == right._align && _hash_code == right._hash_code;
        }

        bool operator==(const class_typeid &right) const noexcept {
            return equals(right);
        }

        bool operator!=(const class_typeid &right) const noexcept {
            return !equals(right);
        }

    private:
        std::vector<property_info> _property_info;
        std::vector<method_info> _method_info;
        std::vector<method_info> _constructer_info;
        std::vector<enum_info> _enum_info;
        std::string_view _class_name;
        bool _is_base;
        bool _is_polymorphic;
        std::size_t _size;
        std::size_t _align;
        std::size_t _hash_code;
        std::vector<class_typeid *> _parent_class_typeid;
    };
}

namespace rainy::experimental::reflection::internals {

    template <typename ReturnType, typename Class, typename... Args, size_t... I>
    rainy::containers::any invoke_helper_impl(void *obj, ReturnType (Class::*method)(Args...),
                                                             const std::vector<rainy::containers::any> &params,
                                                             std::index_sequence<I...>) {
        if constexpr (std::is_void_v<ReturnType>) {
            (static_cast<Class *>(obj)->*method)(rainy::containers::any_cast<Args>(params[I])...);
            return {}; // 返回空的 any 对象
        } else {
            return rainy::containers::any(
                (static_cast<Class *>(obj)->*method)(rainy::containers::any_cast<Args>(params[I])...));
        }
    }

    template <typename ReturnType, typename Class, typename... Args, size_t... I>
    rainy::containers::any invoke_helper_impl(const void *obj, ReturnType (Class::*method)(Args...) const,
                                                             const std::vector<rainy::containers::any> &params,
                                                             std::index_sequence<I...>) {
        if constexpr (std::is_void_v<ReturnType>) {
            (static_cast<const Class *>(obj)->*method)(rainy::containers::any_cast<Args>(params[I])...);
            return {}; // 返回空的 any 对象
        } else {
            return rainy::containers::any(
                (static_cast<const Class *>(obj)->*method)(rainy::containers::any_cast<Args>(params[I])...));
        }
    }

    
template <typename ReturnType, typename Class, typename... Args>
    rainy::containers::any invoke_helper(void *obj, ReturnType (Class::*method)(Args...),
                                                        const std::vector<rainy::containers::any> &params) {
        return invoke_helper_impl(obj, method, params, std::index_sequence_for<Args...>{});
    }

    template <typename ReturnType, typename Class, typename... Args>
    rainy::containers::any invoke_helper(const void *obj, ReturnType (Class::*method)(Args...) const,
                                                        const std::vector<rainy::containers::any> &params) {
        return invoke_helper_impl(obj, method, params, std::index_sequence_for<Args...>{});
    }

    template <typename ReturnType, typename... Args, size_t... I>
    rainy::containers::any invoke_static_helper_impl(ReturnType (*method)(Args...),
                                                                    const std::vector<rainy::containers::any> &params,
                                                                    std::index_sequence<I...>) {
        if constexpr (std::is_void_v<ReturnType>) {
            method(rainy::containers::any_cast<Args>(params[I])...);
            return {};
        } else {
            return rainy::containers::any(method(rainy::containers::any_cast<Args>(params[I])...));
        }
    }

    template <typename ReturnType, typename... Args>
    rainy::containers::any invoke_static_helper(ReturnType (*method)(Args...),
                                                               const std::vector<rainy::containers::any> &params) {
        return invoke_static_helper_impl(method, params, std::index_sequence_for<Args...>{});
    }

    template <typename Class, typename... Args, size_t... I>
    static void construct_helper_impl(void *obj, const std::vector<rainy::containers::any> &params,
                                      std::index_sequence<I...>) {
        new (obj) Class(rainy::containers::any_cast<Args>(params[I])...);
    }

    template <typename Class, typename... Args>
    static void construct_helper(void *obj, const std::vector<rainy::containers::any> &params) {
        construct_helper_impl<Class, Args...>(obj, params, std::index_sequence_for<Args...>{});
    }

    template <typename Class, typename... Args, size_t... I>
    static void construct_helper_impl(void *obj, const std::vector<rainy::containers::any> &params,
                                      std::index_sequence<I...>) {
        new (obj) Class(rainy::containers::any_cast<Args>(params[I])...);
    }

    template <typename Class, typename... Args>
    static void construct_helper(void *obj, const std::vector<rainy::containers::any> &params) {
        construct_helper_impl<Class, Args...>(obj, params, std::index_sequence_for<Args...>{});
    }
}

namespace rainy::experimental::reflection {
    template <typename Class, typename Type>
    property_info make_property(std::string_view name, Type Class::*member) {
        return property_info(
            name, typeid(Type).name(), std::is_const_v<Type>, false,
            [member](const void *obj) -> rainy::containers::any {
                return rainy::containers::any(static_cast<const Class *>(obj)->*member);
            },
            [member](void *obj, const rainy::containers::any &value) {
                if constexpr (!std::is_const_v<Type>) {
                    if (typeid(Type) != value.type()) {
                        return;
                    }
                    static_cast<Class *>(obj)->*member = rainy::containers::any_cast<Type>(value);
                } else {
                    rainy::foundation::system::exceptions::runtime::throw_runtime_error("Modify constant value is illegal!");
                }
            });
    }

    template <typename, typename Type>
    property_info make_property(std::string_view name, Type *member) {
        return property_info(
            name, typeid(Type).name(), std::is_const_v<Type>, true,
            [member](const void *) -> rainy::containers::any { return rainy::containers::any(*member); },
            [member](void *, const rainy::containers::any &value) {
                if constexpr (!std::is_const_v<Type>) {
                    if (typeid(Type) != value.type()) {
                        return;
                    }
                    *member = rainy::containers::any_cast<Type>(value);
                } else {
                    rainy::foundation::system::exceptions::runtime::throw_runtime_error("Modify constant value is illegal!");
                }
            });
    }

    template <typename Class, typename ReturnType, typename... Args>
    method_info make_method(const std::string_view name, ReturnType (Class::*method)(Args...)) {
        return method_info{
            name,
            typeid(ReturnType).name(),
            {typeid(Args).name()...},
            false,
            false,
            [method](void *obj, const std::vector<rainy::containers::any> &params) -> rainy::containers::any {
                if (sizeof...(Args) != params.size()) {
                    throw std::runtime_error("Parameter count mismatch");
                }
                return invoke_helper<ReturnType, Class, Args...>(obj, method, params);
            }};
    }

    template <typename Class, typename ReturnType, typename... Args>
    method_info make_const_method(const std::string_view name, ReturnType (Class::*method)(Args...) const) {
        return method_info{
            name,
            typeid(ReturnType).name(),
            {typeid(Args).name()...},
            true,
            false,
            [method](void *obj, const std::vector<rainy::containers::any> &params) -> rainy::containers::any {
                if (sizeof...(Args) != params.size()) {
                    throw std::runtime_error("Parameter count mismatch");
                }
                return invoke_helper(obj, method, params);
            }};
    }

    template <typename, typename ReturnType, typename... Args>
    method_info make_static_method(const std::string_view name, ReturnType (*method)(Args...)) {
        return method_info{
            name,
            typeid(ReturnType).name(),
            {typeid(Args).name()...},
            true,
            false,
            [method](void * /* 由于method_info要求此处必须有参数，因此此处要作为占位，仅填入nullptr即可 */,
                     const std::vector<rainy::containers::any> &params) -> rainy::containers::any {
                return invoke_static_helper<ReturnType, Args...>(method, params);
            }};
    }

    template <typename Class>
    static method_info make_default_constructor() {
        return method_info{
            "default_constructor",
            typeid(void).name(),
            {},
            false,
            false,
            [](void *obj, const std::vector<rainy::containers::any> &params) -> rainy::containers::any {
                if (!params.empty()) {
                    throw std::runtime_error("Default constructor doesn't accept parameters");
                }
                new (obj) Class();
                return {};
            }};
    }

    template <typename Class>
    method_info make_copy_constructor() {
        return method_info{
            "copy_constructor",
            typeid(void).name(),
            {typeid(const Class &).name()},
            false,
            false,
            [](void *obj, const std::vector<rainy::containers::any> &params) -> rainy::containers::any {
                if (params.size() != 1) {
                    throw std::runtime_error("Copy constructor requires exactly one parameter");
                }
                const Class *other = rainy::containers::any_cast<const Class *>(params[0]);
                new (obj) Class(*other);
                return {};
            }};
    }

    template <typename Class>
    method_info make_move_constructor() {
        return method_info{
            "move_constructor",
            typeid(void).name(),
            {typeid(Class &&).name()},
            false,
            false,
            [](void *obj, const std::vector<rainy::containers::any> &params) -> rainy::containers::any {
                if (params.size() != 1) {
                    throw std::runtime_error("Move constructor requires exactly one parameter");
                }
                Class *other = rainy::containers::any_cast<Class *>(params[0]);
                new (obj) Class(std::move(*other));
                return {};
            }};
    }

    template <typename Class>
    method_info make_copy_assignment_operator() {
        return method_info{
            "copy_assignment_operator",
            typeid(Class &).name(),
            {typeid(const Class &).name()},
            false,
            false,
            [](void *obj, const std::vector<rainy::containers::any> &params) -> rainy::containers::any {
                if (params.size() != 1) {
                    throw std::runtime_error("Copy assignment operator requires exactly one parameter");
                }
                rainy_let self = static_cast<Class *>(obj);
                const Class *other = rainy::containers::any_cast<const Class *>(params[0]);
                *self = *other;
                return rainy::containers::any(self);
            }};
    }

    template <typename Class>
    method_info make_move_assignment_operator() {
        return method_info{
            "move_assignment_operator",
            typeid(Class &).name(),
            {typeid(Class &&).name()},
            false,
            false,
            [](void *obj, const std::vector<rainy::containers::any> &params) -> rainy::containers::any {
                if (params.size() != 1) {
                    throw std::runtime_error("Move assignment operator requires exactly one parameter");
                }
                rainy_let self = static_cast<Class *>(obj);
                Class *other = rainy::containers::any_cast<Class *>(params[0]);
                *self = std::move(*other);
                return rainy::containers::any(self);
            }};
    }

    template <typename Class, typename... Args>
    static method_info make_custom_constructor(const std::string_view name) {
        return method_info{
            name,
            typeid(void).name(),
            {typeid(Args).name()...},
            false,
            false,
            [](void *obj, const std::vector<rainy::containers::any> &params) -> rainy::containers::any {
                if (sizeof...(Args) != params.size()) {
                    throw std::runtime_error("Parameter count mismatch for custom constructor");
                }
                construct_helper<Class, Args...>(obj, params);
                return {};
            }};
    }
}

namespace rainy::experimental::reflection {
    struct qualifier_type {
        const bool is_const;
        const bool is_static;
    };

    class const_property {
    public:
        using any = rainy::containers::any;

        const_property(const property_info &info, void *object = nullptr) noexcept : info(info), object(object), unused(0) {
        }

        explicit operator any() const noexcept {
            if (!object) {
                return {};
            }
            return info.getter(object);
        }

        RAINY_NODISCARD any get() const noexcept {
            if (!object) {
                return {};
            }
            return info.getter(object);
        }

        RAINY_NODISCARD std::string_view name() const noexcept {
            return info.name;
        }

        RAINY_NODISCARD std::string_view type() const noexcept {
            return info.type;
        }

        RAINY_NODISCARD qualifier_type qualifier() const noexcept {
            return qualifier_type{info.is_const, info.is_static};
        }

    protected:
        const property_info &info;
        void *object;
        int unused;
    };

    class property : public const_property {
    public:
        property(const property_info &info, void *object = nullptr) noexcept : const_property(info, object) {
        }

        void set(const any &val) noexcept {
            if (!object) {
                return;
            }
            unused += 0;
            // 此处做无意义的操作。此处让静态分析工具认为无法作为const函数。以保证const安全。
            // 此处编译器会自动优化
            const_cast<property_info &>(info).setter(object, val);
        }

        property &operator=(const any &val) {
            if (this->object) {
                rainy::foundation::utility::invoke(const_cast<property_info &>(this->info).setter, object, val);
            }
            return *this;
        }
    };

    class method {
    public:
        using any = rainy::containers::any;

        method(method_info &info, void *object = nullptr) noexcept : info(info), object(object) {
        }

        RAINY_NODISCARD any invoke(const std::vector<any> &params) const {
            if (!info.is_static) {
                rainy::foundation::utility::expects(info.is_const, "You're invoking a non-const method");
            }
            return info.invoker(info.is_static ? nullptr : object, params);
        }

        RAINY_NODISCARD any invoke(const std::vector<any> &params) {
            return info.invoker(info.is_static ? nullptr : object, params);
        }

        RAINY_NODISCARD std::string_view name() const noexcept {
            return info.name;
        }

        RAINY_NODISCARD std::string_view return_type() const noexcept {
            return info.return_type;
        }

        RAINY_NODISCARD std::vector<std::string_view> param_type() const noexcept {
            return info.param_types;
        }

        RAINY_NODISCARD std::string to_string() const noexcept {
            std::string params;
            std::stringstream ss(params);
            for (const auto &param: info.param_types) {
                ss << param << ',';
            }


            std::string buf;
            auto [is_const, is_static] = qualifier();
#if RAINY_HAS_CXX20
            rainy::foundation::utility::format(buf, "{}{} {}({}){}", qualifier_.is_static ? "static " : "", return_type(), info.name,
                                                  params, info.is_const ? " const" : "");
#else
            rainy::foundation::utility::cstyle_format(buf, "%s%s %s(%s)%s", is_static ? "static " : "", return_type().data(),
                                                         info.name.data(), params.data(), is_const ? " const" : "");
#endif
            return buf;
        }

        RAINY_NODISCARD qualifier_type qualifier() const noexcept {
            return qualifier_type{info.is_const, info.is_static};
        }

    private:
        method_info &info;
        void *object;
    };
}

namespace rainy::foundation::utitily {
    template <typename Class>
    class object {
    public:
        friend class type_info_impl;

        using super_type = object;
        using class_type = Class;

        constexpr object() = default;

        virtual RAINY_CONSTEXPR20 ~object() = default;

        RAINY_NODISCARD virtual const std::type_info &type_info() const noexcept {
            return typeid(class_type);
        }

        RAINY_NODISCARD virtual std::size_t hash_code() const noexcept {
            return type_info().hash_code();
        }

        RAINY_NODISCARD virtual const void *addressof() const noexcept {
            return static_cast<const void *>(rainy::foundation::utility::addressof(*this));
        }

        virtual super_type &as_super() noexcept {
            return *this;
        }

        RAINY_NODISCARD virtual const super_type &as_super() const noexcept {
            return *this;
        }

        RAINY_NODISCARD virtual bool equals(const class_type &object) const noexcept {
            if constexpr (std::is_standard_layout_v<class_type> && std::is_trivial_v<class_type>) {
                // POD类型采用memcmp比较
                return std::memcmp(this, rainy::foundation::utility::addressof(object), sizeof(class_type)) == 0;
            } else {
                std::terminate(); // 若希望equals拥有独立的比较逻辑，则需要重写
            }
        }

        RAINY_NODISCARD virtual std::string to_string() const noexcept {
            std::string buffer;
            rainy::foundation::utility::cstyle_format(buffer, "%s:%p", type_info().name(), static_cast<const void *>(this));
            return buffer;
        }

        template <typename Derived>
        RAINY_NODISCARD Derived &as() noexcept {
            static_assert(std::is_base_of_v<class_type, Derived> || std::is_same_v<class_type, Derived>);
            rainy_let ptr = dynamic_cast<Derived *>(this);
            if (!ptr) {
                rainy::foundation::utility::throw_exception(rainy::foundation::system::exceptions::cast::bad_cast());
                std::terminate();
            }
            return *ptr;
        }

        template <typename Derived>
        RAINY_NODISCARD const Derived &as() const noexcept {
            static_assert(std::is_base_of_v<class_type, Derived> || std::is_same_v<class_type, Derived>);
            rainy_const ptr = dynamic_cast<const Derived *>(this);
            if (!ptr) {
                rainy::foundation::utility::throw_exception(rainy::foundation::system::exceptions::cast::bad_cast());
                std::terminate();
            }
            return *ptr;
        }

        RAINY_NODISCARD virtual class_type *clone() const noexcept(std::is_nothrow_copy_constructible_v<class_type>) {
            return new class_type(*static_cast<const class_type *>(this));
        }

        RAINY_NODISCARD virtual bool operator==(const class_type &object) const noexcept {
            return equals(object);
        }

        RAINY_NODISCARD virtual const experimental::reflection::object_type_info &refl_type_info() const {
            static const type_info_impl type_info; // lazy loading
            return type_info;
        }

    protected:
        static Class *construct_object(Class *ptr) noexcept(std::is_nothrow_constructible_v<Class>) {
            static_assert(std::is_default_constructible_v<Class>, "调用此函数必须支持默认构造");
            return rainy::foundation::utility::construct_at(ptr);
        }

        template <typename... Args>
        static Class *construct_object(Class *ptr, Args &&...args) noexcept(std::is_nothrow_constructible_v<Class>) {
            static_assert(std::is_constructible_v<Class, Args...>, "调用此函数必须支持默认构造");
            return rainy::foundation::utility::construct_at(ptr, std::forward<Args>(args)...);
        }

        static void delete_object(Class *ptr) {
            if (ptr) {
                destruct_object(ptr);
                delete ptr;
            }
        }

        static void destruct_object(Class *ptr) noexcept(std::is_nothrow_destructible_v<Class>) {
            if (ptr) {
                ptr->~Class();
            }
        }

        static std::vector<experimental::reflection::property_info> _properties() noexcept {
            return {};
        }

        static std::vector<experimental::reflection::method_info> _methods() noexcept {
            return {};
        }

        class type_info_impl final : public object_type_info {
        public:
            type_info_impl() : _method_info(Class::_methods()), _properties_info(Class::_properties()) {
            }

            RAINY_NODISCARD std::string name() const override {
                return typeid(class_type).name();
            }

            RAINY_NODISCARD std::vector<property_info> properties() const override {
                return _properties_info;
            }

            RAINY_NODISCARD std::vector<method_info> methods() const override {
                return _method_info;
            }

            std::vector<method_info> _method_info;
            std::vector<property_info> _properties_info;
        };
    };

    class avoid_final_warning_for_object_class final : public object<avoid_final_warning_for_object_class> {};
}

class method {
public:
    using any = rainy::containers::any;

    method(method_info &info, void *object = nullptr) noexcept : info(info), object(object) {
    }

    RAINY_NODISCARD any invoke(const std::vector<any> &params) const {
        if (!info.is_static) {
            rainy::foundation::utility::expects(info.is_const, "You're invoking a non-const method");
        }
        return info.invoker(info.is_static ? nullptr : object, params);
    }

    RAINY_NODISCARD any invoke(const std::vector<any> &params) {
        return info.invoker(info.is_static ? nullptr : object, params);
    }

    RAINY_NODISCARD std::string_view name() const noexcept {
        return info.name;
    }

    RAINY_NODISCARD std::string_view return_type() const noexcept {
        return info.return_type;
    }

    RAINY_NODISCARD std::vector<std::string_view> param_type() const noexcept {
        return info.param_types;
    }

    RAINY_NODISCARD std::string to_string() const noexcept {
        std::string params;
        std::stringstream ss(params);
        for (const auto &param: info.param_types) {
            ss << param << ',';
        }


        std::string buf;
        auto [is_const, is_static] = qualifier();
#if RAINY_HAS_CXX20
        rainy::foundation::utility::format(buf, "{}{} {}({}){}", qualifier_.is_static ? "static " : "", return_type(), info.name, params,
                                              info.is_const ? " const" : "");
#else
        rainy::foundation::utility::cstyle_format(buf, "%s%s %s(%s)%s", is_static ? "static " : "", return_type().data(),
                                                     info.name.data(), params.data(), is_const ? " const" : "");
#endif
        return buf;
    }

    RAINY_NODISCARD qualifier_type qualifier() const noexcept {
        return qualifier_type{info.is_const, info.is_static};
    }

private:
    method_info &info;
    void *object;
};

template <typename Class>
class object {
public:
    friend class type_info_impl;

    using super_type = object;
    using class_type = Class;

    constexpr object() = default;

    virtual RAINY_CONSTEXPR20 ~object() = default;

    RAINY_NODISCARD virtual const std::type_info &type_info() const noexcept {
        return typeid(class_type);
    }

    RAINY_NODISCARD virtual std::size_t hash_code() const noexcept {
        return type_info().hash_code();
    }

    RAINY_NODISCARD virtual const void *addressof() const noexcept {
        return static_cast<const void *>(rainy::foundation::utility::addressof(*this));
    }

    virtual super_type &as_super() noexcept {
        return *this;
    }

    RAINY_NODISCARD virtual const super_type &as_super() const noexcept {
        return *this;
    }

    RAINY_NODISCARD virtual bool equals(const class_type &object) const noexcept {
        if constexpr (std::is_standard_layout_v<class_type> && std::is_trivial_v<class_type>) {
            // POD类型采用memcmp比较
            return std::memcmp(this, rainy::foundation::utility::addressof(object), sizeof(class_type)) == 0;
        } else {
            std::terminate(); // 若希望equals拥有独立的比较逻辑，则需要重写
        }
    }

    RAINY_NODISCARD virtual std::string to_string() const noexcept {
        std::string buffer;
        rainy::foundation::utility::cstyle_format(buffer, "%s:%p", type_info().name(), static_cast<const void *>(this));
        return buffer;
    }

    template <typename Derived>
    RAINY_NODISCARD Derived &as() noexcept {
        static_assert(std::is_base_of_v<class_type, Derived> || std::is_same_v<class_type, Derived>);
        rainy_let ptr = dynamic_cast<Derived *>(this);
        if (!ptr) {
            rainy::foundation::utility::throw_exception(rainy::foundation::system::exceptions::cast::bad_cast());
            std::terminate();
        }
        return *ptr;
    }

    template <typename Derived>
    RAINY_NODISCARD const Derived &as() const noexcept {
        static_assert(std::is_base_of_v<class_type, Derived> || std::is_same_v<class_type, Derived>);
        rainy_const ptr = dynamic_cast<const Derived *>(this);
        if (!ptr) {
            rainy::foundation::utility::throw_exception(rainy::foundation::system::exceptions::cast::bad_cast());
            std::terminate();
        }
        return *ptr;
    }

    RAINY_NODISCARD virtual class_type *clone() const noexcept(std::is_nothrow_copy_constructible_v<class_type>) {
        return new class_type(*static_cast<const class_type *>(this));
    }

    RAINY_NODISCARD virtual bool operator==(const class_type &object) const noexcept {
        return equals(object);
    }

    RAINY_NODISCARD virtual const object_type_info &refl_type_info() const {
        static const type_info_impl type_info; // lazy loading
        return type_info;
    }

protected:
    static Class *construct_object(Class *ptr) noexcept(std::is_nothrow_constructible_v<Class>) {
        static_assert(std::is_default_constructible_v<Class>, "调用此函数必须支持默认构造");
        return rainy::foundation::utility::construct_at(ptr);
    }

    template <typename... Args>
    static Class *construct_object(Class *ptr, Args &&...args) noexcept(std::is_nothrow_constructible_v<Class>) {
        static_assert(std::is_constructible_v<Class, Args...>, "调用此函数必须支持默认构造");
        return rainy::foundation::utility::construct_at(ptr, std::forward<Args>(args)...);
    }

    static void delete_object(Class *ptr) {
        if (ptr) {
            destruct_object(ptr);
            delete ptr;
        }
    }

    static void destruct_object(Class *ptr) noexcept(std::is_nothrow_destructible_v<Class>) {
        if (ptr) {
            ptr->~Class();
        }
    }

    static std::vector<property_info> _properties() noexcept {
        return {};
    }

    static std::vector<method_info> _methods() noexcept {
        return {};
    }

    class type_info_impl final : public object_type_info {
    public:
        type_info_impl() : _method_info(Class::_methods()), _properties_info(Class::_properties()) {
        }

        RAINY_NODISCARD std::string name() const override {
            return typeid(class_type).name();
        }

        RAINY_NODISCARD std::vector<property_info> properties() const override {
            return _properties_info;
        }

        RAINY_NODISCARD std::vector<method_info> methods() const override {
            return _method_info;
        }

        std::vector<method_info> _method_info;
        std::vector<property_info> _properties_info;
    };
};

class avoid_final_warning_for_object_class final : public object<avoid_final_warning_for_object_class> {};

template <typename Ty>
struct member_pointer_class;

// 偏特化用于成员指针（例如 T C::*）
template <typename Class, typename Ty>
struct member_pointer_class<Ty Class::*> {
    using type = Class;
};

class shared_object;

/* 这是一个实验性质的反射系统。当前正在测试，若没有必要，请勿使用 */
class reflection {
public:
    friend class shared_object;

    template <typename ClassType, typename ReturnType, typename... Args>
    static constexpr auto make_method_helper(ReturnType (ClassType::*method)(Args...)) {
        return method;
    }

    template <typename ClassType, typename ReturnType, typename... Args>
    static constexpr auto make_cmethod_helper(ReturnType (ClassType::*method)(Args...) const) {
        return method;
    }

    template <typename ClassType, typename ReturnType, typename... Args>
    static constexpr auto make_method_helper(ReturnType (*method)(Args...)) {
        return method;
    }

    struct collection {
        bool success;

        property_info *property;
        method_info *method;
    };

    static reflection *instance() noexcept {
        static reflection _instance;
        return &_instance;
    }

    template <typename Class, typename Base = void, std::enable_if_t<std::is_class_v<Class>, int> = 0>
    void register_class(const std::string_view class_name) {
        rainy::foundation::utility::expects(
            !class_name.empty(), "you must pass a non-empty string_view to this method,or consider using register_class_auto");
        if (has_register<Class>()) {
            return;
        }
        rainy_ref eval_typeid = rainy::foundation::utility::get_typeid<Class>();

        if constexpr (std::is_void_v<Base>) {
            reflection_storage.insert({eval_typeid.hash_code(), make_class_typeid<Class>(eval_typeid)}); // 注册基类
        } else {
            static_assert(std::is_base_of_v<Base, Class> && std::is_class_v<Base>);
            rainy_ref base_typeid = rainy::foundation::utility::get_typeid<Base>();

            if (!has_register<Base>()) /* 因为没有注册基类。我们需要帮助用户注册 */ {
                reflection_storage.insert({base_typeid.hash_code(), make_class_typeid<Class>(base_typeid)});
                rainy_ref base = reflection_storage.at(base_typeid.hash_code());
                register_constructors<Class>(base);
                register_assignment_operators<Class>(base);
                reflection_storage.insert({eval_typeid.hash_code(), make_class_typeid<Class>(eval_typeid, &base)});
            } else {
                rainy_ref base = reflection_storage.at(base_typeid.hash_code());
                reflection_storage.insert({eval_typeid.hash_code(), make_class_typeid<Class>(eval_typeid, &base)});
            }
        }

        rainy_ref added = reflection_storage.at(eval_typeid.hash_code());
        register_constructors<Class>(added);
        register_assignment_operators<Class>(added);
    }

    template <typename EnumClass, std::enable_if_t<std::is_enum_v<EnumClass>, int> = 0>
    void register_enum(const std::string_view class_name) noexcept {
        rainy::foundation::utility::expects(!class_name.empty(), "you must pass a non-empty string_view to this method");
        if (has_register<EnumClass>()) {
            return;
        }
        rainy_ref eval_typeid = rainy::foundation::utility::get_typeid<EnumClass>();
        reflection_storage.insert({eval_typeid.hash_code(), make_class_typeid<EnumClass>(eval_typeid)}); // 注册枚举类
    }

    template <typename Class, typename Base = void, std::enable_if_t<std::is_class_v<Class>, int> = 0>
    void register_class_auto() {
        register_class<Class, Base>(typeid(Class).name());
    }

    template <typename Class>
    void unregister_class() {
        rainy::foundation::utility::ensures(has_register<Class>(), "You must register a class instance then invoke this method");
        rainy_ref eval_typeid = rainy::foundation::utility::get_typeid<Class>();
        reflection_storage.erase(eval_typeid.hash_code());
    }

    template <typename Class, typename = std::enable_if_t<std::is_class_v<Class> || std::is_enum_v<Class>>>
    bool has_register() const noexcept {
        return reflection_storage.find(typeid(Class).hash_code()) != reflection_storage.end();
    }

    template <typename Class, typename = std::enable_if_t<std::is_class_v<Class>>>
    bool is_derived() const noexcept {
        if (has_register<Class>()) {
            const std::size_t hash_code = typeid(Class).hash_code();
            rainy_cref register_class = reflection_storage.at(hash_code);
            return !register_class._is_base;
        }
        return false;
    }

    template <typename Class, typename... Args>
    void add_custom_constrcuter(const std::string_view name) {
        rainy::foundation::utility::expects(
            !name.empty() && has_register<Class>(),
            "you must pass a non-empty string_view to this method and also you must register this class");
        rainy_ref eval_typeid = typeid(Class);
        rainy_ref constrcuter_info_ = reflection_storage.at(eval_typeid.hash_code())._constructer_info;
        rainy_let iter = std::find_if(constrcuter_info_.begin(), constrcuter_info_.end(),
                                         [&name](const method_info &info) { return info.name == name; });
        rainy::foundation::utility::ensures(iter == constrcuter_info_.end()); // 检查到内部已经存在相同的constructer，因此不能插入
        constrcuter_info_.emplace_back(make_custom_constructor<Class, Args...>(name));
    }

    template <auto field, std::enable_if_t<std::is_member_pointer_v<decltype(field)>, int> = 0>
    void add_field(const std::string_view property_name) {
        rainy::foundation::utility::expects(!property_name.empty(), "property_name should not be empty!");
        using class_type = typename member_pointer_class<decltype(field)>::type;
        rainy_ref class_typeid = typeid(class_type);
        rainy::foundation::utility::ensures(has_register<class_type>(), "You must register a class instance then invoke this method");
        rainy_ref property_info_ = reflection_storage.at(class_typeid.hash_code())._property_info;
        rainy_let iter = std::find_if(property_info_.begin(), property_info_.end(),
                                         [&property_name](const property_info &info) { return info.name == property_name; });
        rainy::foundation::utility::ensures(iter == property_info_.end()); // 检查到内部已经存在相同的field，因此不能插入
        property_info_.emplace_back(make_property<class_type>(property_name, field));
    }

    template <auto field, typename Class, std::enable_if_t<std::is_pointer_v<decltype(field)> && std::is_class_v<Class>, int> = 0>
    void add_static_field(const std::string_view property_name) {
        rainy::foundation::utility::expects(!property_name.empty(), "property_name should not be empty!");
        using class_type = Class;
        rainy_ref class_typeid = typeid(class_type);
        rainy::foundation::utility::ensures(has_register<class_type>(), "You must register a class instance then invoke this method");
        rainy_ref property_info_ = reflection_storage.at(class_typeid.hash_code())._property_info;
        rainy_let iter = std::find_if(property_info_.begin(), property_info_.end(),
                                         [&property_name](const property_info &info) { return info.name == property_name; });
        rainy::foundation::utility::ensures(iter == property_info_.end()); // 检查到内部已经存在相同的field，因此不能插入
        property_info_.emplace_back(make_property<class_type>(property_name, field));
    }

    template <auto method, std::enable_if_t<std::is_member_function_pointer_v<decltype(method)>, int> = 0>
    void add_method(const std::string_view method_name, void (*_success_func)(const std::string_view name) = nullptr) {
        rainy::foundation::utility::expects(!method_name.empty(), "Property_name should not be empty!");

        using decltype_method = decltype(method);
        using class_type = typename member_pointer_class<decltype_method>::type;
        rainy_ref class_typeid = typeid(class_type);
        rainy::foundation::utility::ensures(has_register<class_type>(), "You must register a class instance then invoke this method");
        rainy_ref method_info_ = reflection_storage.at(class_typeid.hash_code())._method_info;
        rainy_let iter = std::find_if(method_info_.begin(), method_info_.end(),
                                         [&method_name](const method_info &info) { return info.name == method_name; });
        rainy::foundation::utility::ensures(iter == method_info_.end()); // 检查到内部已经存在相同的method_name，因此不能插入
        using traits = method_traits<decltype(method)>;
        if constexpr (traits::is_const) {
            if (_success_func) {
                _success_func(typeid(decltype_method).name());
            }
            method_info_.emplace_back(make_const_method<class_type>(method_name, method));
        } else if constexpr (!(traits::is_const && traits::is_static)) {
            if (_success_func) {
                _success_func(typeid(decltype_method).name());
            }
            method_info_.emplace_back(make_method<class_type>(method_name, method));
        } else {
            static_assert(rainy::type_traits::internals::always_false<void>,
                          "If you want add a static method,please invoke add_static_method");
        }
    }

    template <typename Class, auto method, std::enable_if_t<std::is_class_v<Class>, int> = 0>
    void add_static_method(const std::string_view method_name, void (*_success_func)(const std::string_view name) = nullptr) {
        rainy::foundation::utility::expects(!method_name.empty(), "method_name should not be empty!");
        using decltype_method = decltype(method);
        rainy_ref class_typeid = typeid(Class);
        rainy::foundation::utility::ensures(has_register<Class>(), "You must register a class instance then invoke this method");
        rainy_ref method_info_ = reflection_storage.at(class_typeid.hash_code())._method_info;
        rainy_let iter = std::find_if(method_info_.begin(), method_info_.end(),
                                         [&method_name](const method_info &info) { return info.name == method_name; });
        rainy::foundation::utility::ensures(iter == method_info_.end()); // 检查到内部已经存在相同的method_name，因此不能插入
        using traits = method_traits<decltype_method>;
        if constexpr (!(traits::is_const && traits::is_static)) {
            if (_success_func) {
                _success_func(typeid(decltype_method).name());
            }
            method_info_.emplace_back(make_static_method<Class>(method_name, method));
        } else {
            static_assert(rainy::type_traits::internals::always_false<void>,
                          "If you want add a method,please invoke add_method");
        }
    }

    template <typename Derived, typename Base,
              std::enable_if_t<std::is_base_of_v<Base, Derived> && !std::is_same_v<Derived, Base>, int> = 0>
    void add_base() {
        // 检查 Derived 是否已注册
        rainy::foundation::utility::expects(has_register<Derived>(), "You must register Derived class before invoking this method.");
        // 获取 Derived 类的信息
        auto &derived_class = reflection_storage.at(typeid(Derived).hash_code());
        rainy_ref base_typeid = typeid(Base);
        // 获取 Base 类的 typeid
        std::size_t base_hash = base_typeid.hash_code();
        // 查找 Derived 类的父类列表中是否已包含 Base 类
        auto &parent_class_typeids = derived_class._parent_class_typeid;
        // 不为空就意味着我们需要检查了
        if (!parent_class_typeids.empty()) {
            auto find = std::find_if(parent_class_typeids.begin(), parent_class_typeids.end(),
                                     [base_hash](const class_typeid *iter) { return iter->_hash_code == base_hash; });
            // 确保没有找到相同的基类
            rainy::foundation::utility::ensures(find == parent_class_typeids.end(), "Base class already exists");
        }
        // 标记派生类不再是基类
        derived_class._is_base = false;
        if (has_register<Base>()) {
            // Base 类已注册，直接添加
            parent_class_typeids.emplace_back(&reflection_storage.at(base_hash));
        } else {
            // Base 类未注册，先注册基类，再添加到派生类的父类列表
            reflection_storage.insert({base_hash, make_class_typeid<Base>(base_typeid)});
            rainy_ref base = reflection_storage.at(base_typeid.hash_code());
            register_constructors<Base>(base);
            register_assignment_operators<Base>(base);
            parent_class_typeids.emplace_back(&reflection_storage.at(base_hash));
        }
    }

    template <typename Derived, typename Base, std::enable_if_t<std::is_base_of_v<Base, Derived>, int> = 0>
    void remove_base() {
        rainy::foundation::utility::expects(has_register<Derived>(), "You must register Derived class before invoking this method.");
        auto &derived_class = reflection_storage.at(typeid(Derived).hash_code());
        // 获取 Base 类的 typeid
        std::size_t base_hash = typeid(Base).hash_code();
        auto &parent_class_typeids = derived_class._parent_class_typeid;
        if (!parent_class_typeids.empty()) /* 如果不为空，则检查 */ {
            auto find = std::find_if(parent_class_typeids.begin(), parent_class_typeids.end(),
                                     [base_hash](const class_typeid *iter) { return iter->_hash_code == base_hash; });
            if (find != parent_class_typeids.end()) {
                // 找到了，直接删除
                parent_class_typeids.erase(find);
            }
        }
        derived_class._is_base = parent_class_typeids.empty(); // 如果为空，则意味着没添加父类关系。可以直接假定其为父类
    }

    template <typename Class>
    const property_info &get_field(const std::string_view field_name) const {
        static const property_info empty{};
        if (!has_register<Class>()) {
            return empty;
        }
        const std::size_t hash_code = typeid(Class).hash_code();
        if (const property_info *cache_data = check_cache(hash_code, field_name, property_cache_storage); cache_data != nullptr) {
            return *cache_data;
        }
        const auto &class_data = reflection_storage.at(hash_code);
        const auto &property_info_vec = class_data._property_info;
        auto iter = std::find_if(property_info_vec.begin(), property_info_vec.end(),
                                 [&field_name](const property_info &info) { return info.name == field_name; });
        if (iter != property_info_vec.end()) {
            property_cache_storage[hash_code].push_cache(field_name, *iter);
            return *iter;
        }
        property_info *found_info = nullptr;
        std::size_t found_hash_code{};
        for (auto *parent: class_data._parent_class_typeid) {
            if (const property_info *cache_data = check_cache(parent->_hash_code, field_name, property_cache_storage);
                cache_data != nullptr) {
                return *cache_data;
            }
            if (property_info *parent_info = get_field_in_parent(parent, field_name); parent_info) {
                if (found_info) {
                    std::ostringstream oss;
                    oss << "Ambiguous field '" << field_name << "' found in multiple parent classes.";
                    rainy::foundation::utility::ensures(false, oss.str());
                }
                found_hash_code = parent->_hash_code;
                found_info = parent_info;
            }
        }
        if (found_info) {
            property_cache_storage[found_hash_code].push_cache(field_name, *found_info);
            return *found_info;
        }
        return empty;
    }

    template <typename Class>
    const method_info &get_method(const std::string_view method_name) const noexcept {
        static const method_info empty{};
        if (!has_register<Class>()) {
            return empty;
        }
        const std::size_t hash_code = typeid(Class).hash_code();

        const auto &method_info_vec = reflection_storage.at(hash_code)._method_info;
        auto iter = std::find_if(method_info_vec.begin(), method_info_vec.end(),
                                 [&method_name](const method_info &info) { return info.name == method_name; });
        return (iter != method_info_vec.end()) ? (*iter) : empty;
    }

    template <typename Class>
    property_info &get_field(const std::string_view field_name) {
        unused += 0;
        return const_cast<property_info &>(static_cast<const reflection *>(this)->get_field<Class>(field_name));
    }

    template <typename Class>
    method_info &get_method(const std::string_view method_name) noexcept {
        unused += 0;
        return const_cast<method_info &>(static_cast<const reflection *>(this)->get_method<Class>(method_name));
    }

    template <typename EnumClass, std::enable_if_t<std::is_enum_v<EnumClass>, int> = 0>
    void add_efield(const std::string_view field_name, EnumClass value) noexcept {
        rainy::foundation::utility::expects(
            !field_name.empty() && has_register<EnumClass>(),
            "you must pass a non-empty string_view to this method and also you must register this class");
        rainy_ref eval_typeid = rainy::foundation::utility::get_typeid<EnumClass>();
        class_typeid &enum_class = reflection_storage.at(eval_typeid.hash_code());
        enum_class._enum_info.emplace_back(field_name, value);
    }

    template <typename EnumClass, std::enable_if_t<std::is_enum_v<EnumClass>, int> = 0>
    const enum_info &get_efield(const std::string_view field_name) const noexcept {
        rainy::foundation::utility::expects(
            !field_name.empty() && has_register<EnumClass>(),
            "you must pass a non-empty string_view to this method and also you must register this class");
        static const enum_info empty{};
        rainy_ref eval_typeid = rainy::foundation::utility::get_typeid<EnumClass>();
        const class_typeid &enum_class = reflection_storage.at(eval_typeid.hash_code());
        auto &_enum_info = enum_class._enum_info;
        const auto find =
            std::find_if(_enum_info.begin(), _enum_info.end(), [&field_name](const enum_info &info) { return info.name == field_name; });
        return find == _enum_info.end() ? empty : *find;
    }

    template <typename Class>
    std::vector<method_info> &get_methods() noexcept {
        rainy::foundation::utility::expects(has_register<Class>(), "You must register Class before invoking this method.");
        const std::size_t hash_code = typeid(Class).hash_code();
        return reflection_storage.at(hash_code)._method_info;
    }

    template <typename Class>
    std::vector<method_info> &get_constructers() noexcept {
        rainy::foundation::utility::expects(has_register<Class>(), "You must register Class before invoking this method.");
        const std::size_t hash_code = typeid(Class).hash_code();
        return reflection_storage.at(hash_code)._constructer_info;
    }

    template <typename Class, typename... Args>
    RAINY_NODISCARD std::shared_ptr<shared_object> make_shared(Args &&...args) {
        rainy::foundation::utility::expects(has_register<Class>(), "You must register Class before invoking this method.");
        rainy_ref class_typeid = typeid(Class);
        auto object = std::make_shared<shared_object>(reflection_storage.at(class_typeid.hash_code()),
                                                      std::make_shared<Class>(std::forward<Args>(args)...));
        return object;
    }

    template <typename Class, typename... Args>
    RAINY_NODISCARD std::shared_ptr<shared_object> make_shared_with_name(
        const std::string_view constructer_name, const std::vector<rainy::containers::any> &params) {
        rainy::foundation::utility::expects(
            has_register<Class>() && !constructer_name.empty(),
            "You must register Class before invoking this method and also your need input a vaild constructer_name.");
        static auto deleter = [](Class *ptr) {
            ptr->~Class();
            operator delete(ptr, static_cast<std::align_val_t>(alignof(Class)));
        };
        rainy_ref class_typeid = typeid(Class);
        rainy_ref constructers = reflection_storage.at(class_typeid.hash_code())._constructer_info;
        const auto find = std::find_if(constructers.begin(), constructers.end(),
                                       [&constructer_name](const method_info &info) { return info.name == constructer_name; });
        rainy::foundation::utility::ensures(find != constructers.end(), "Could not find invoke");
        rainy_let ptr = static_cast<Class *>(operator new(sizeof(Class), static_cast<std::align_val_t>(alignof(Class))));
        find->invoker(ptr, params);
        auto object =
            std::make_shared<shared_object>(reflection_storage.at(class_typeid.hash_code()), std::shared_ptr<Class>(ptr, deleter));
        return object;
    }

    void invalidate_all_cache() noexcept {
        property_cache_storage.clear();
        method_cache_storage.clear();
        unused += 0;
    }

    template <typename Class>
    void invalidate_property_cache() noexcept {
        const std::size_t hash = rainy::foundation::utility::get_typeid<Class>().hash_code();
        const auto find = property_cache_storage.find(hash);
        if (find == property_cache_storage.end()) {
            return;
        }
        find->second.invalidate();
        unused += 0;
    }

    template <typename Class>
    void invalidate_method_cache() noexcept {
        const std::size_t hash = rainy::foundation::utility::get_typeid<Class>().hash_code();
        const auto find = method_cache_storage.find(hash);
        if (find == method_cache_storage.end()) {
            return;
        }
        find->second.invalidate();
        unused += 0;
    }

private:
    template <typename info>
    struct cache {
        void push_cache(const std::string_view name, const info &info_) noexcept {
            static std::hash<std::string_view> hasher;
            const std::size_t cacl_result = hasher(name);
            mapping[cacl_result] = &info_;
        }

        RAINY_NODISCARD const info *get(const std::string_view name) const noexcept {
            static std::hash<std::string_view> hasher;
            const std::size_t cacl_result = hasher(name);
            if (const auto find = mapping.find(cacl_result); find != mapping.end()) {
                return find->second;
            }
            return nullptr;
        }

        void invalidate() noexcept {
            mapping.clear();
        }

        std::unordered_map<std::size_t, const info *> mapping;
    };

    template <typename Class>
    static class_typeid make_class_typeid(const std::type_info &info, class_typeid *parent_class_typeid = nullptr) {
        return {info, std::is_polymorphic_v<Class>, sizeof(Class), alignof(Class), parent_class_typeid};
    }

    template <typename Class>
    void register_constructors(class_typeid &class_type) {
        if constexpr (std::is_default_constructible_v<Class>) {
            class_type._constructer_info.emplace_back(make_default_constructor<Class>());
        }
        if constexpr (std::is_copy_constructible_v<Class>) {
            class_type._constructer_info.emplace_back(make_copy_constructor<Class>());
        }
        if constexpr (std::is_move_constructible_v<Class>) {
            class_type._constructer_info.emplace_back(make_move_constructor<Class>());
        }
    }

    template <typename Class>
    void register_assignment_operators(class_typeid &class_type) {
        // 拷贝赋值运算符
        if constexpr (std::is_copy_assignable_v<Class>) {
            class_type._method_info.emplace_back(make_copy_assignment_operator<Class>());
        }
        // 移动赋值运算符
        if constexpr (std::is_move_assignable_v<Class>) {
            class_type._method_info.emplace_back(make_move_assignment_operator<Class>());
        }
    }

    template <typename info>
    const info *check_cache(const std::size_t hash_code, const std::string_view name,
                            std::unordered_map<std::size_t, cache<info>> &cache_storage) const {
        if (const auto find = cache_storage.find(hash_code); find == cache_storage.end()) {
            cache_storage.insert({hash_code, cache<info>{}});
        } else {
            rainy_cref cache = find->second;
            if (const auto *cache_data = cache.get(name); cache_data != nullptr) {
                return cache_data;
            }
        }
        return nullptr;
    }

    static property_info *get_field_in_parent(class_typeid *parent_class, const std::string_view field_name) {
        property_info *found_info = nullptr;
        rainy::containers::stack_container<class_typeid *> class_stack;
        std::unordered_set<class_typeid *> visited_classes; // 用于跟踪已访问过的类，防止循环
        class_stack.push(parent_class);
        visited_classes.insert(parent_class); // 初始类标记为已访问
        while (!class_stack.empty()) {
            class_typeid *current_class = class_stack.top();
            class_stack.pop();
            auto &property_info_vec = current_class->_property_info;
            if (auto iter = std::find_if(property_info_vec.begin(), property_info_vec.end(),
                                         [&field_name](const property_info &info) { return info.name == field_name; });
                iter != property_info_vec.end()) {
                if (found_info) {
                    // 如果已经找到过一个匹配的字段，则返回 nullptr 以避免多义性
                    return nullptr;
                }
                found_info = &(*iter);
            }
            for (auto *parent: current_class->_parent_class_typeid) {
                if (visited_classes.find(parent) == visited_classes.end()) { // 检查是否已访问
                    class_stack.push(parent);
                    visited_classes.insert(parent); // 标记为已访问
                }
            }
        }
        return found_info;
    }

    reflection() = default;
    ~reflection() = default;

    std::unordered_map<std::size_t, class_typeid> reflection_storage;
    mutable std::unordered_map<std::size_t, cache<property_info>> property_cache_storage;
    mutable std::unordered_map<std::size_t, cache<method_info>> method_cache_storage{};
    int unused{};
};

class shared_object {
public:
    friend class reflection;

    using any = rainy::containers::any;

    template <typename Class>
    shared_object(class_typeid &instance, const std::shared_ptr<Class> obj) :
        instance(instance), ptr(std::make_unique<object_impl<Class>>(obj)) {
    }

    ~shared_object() = default;

    shared_object &operator=(const shared_object &) = delete;
    shared_object &operator=(shared_object &&) = delete;
    shared_object(const shared_object &) = delete;
    shared_object(shared_object &&) = delete;

    RAINY_NODISCARD std::string_view class_name() const noexcept {
        return instance._class_name;
    }

    RAINY_NODISCARD auto invoke(const std::string_view name, const std::vector<any> &params) -> any {
        rainy_ref method_info_vec = instance._method_info;
        const auto iter =
            std::find_if(method_info_vec.begin(), method_info_vec.end(), [&name](const method_info &info) { return info.name == name; });
        rainy::foundation::utility::ensures(iter != method_info_vec.end(), "failed to find invoke");
        return iter->invoker(ptr->get(), params);
    }

    RAINY_NODISCARD auto invoke(const std::string_view name, const std::vector<any> &params) const -> any {
        rainy_ref method_info_vec = instance._method_info;
        const auto iter = std::find_if(method_info_vec.begin(), method_info_vec.end(),
                                       [&name](const method_info &info) { return info.name == name && info.is_const; });
        rainy::foundation::utility::ensures(iter != method_info_vec.end(),
                                               "failed to find invoke and also your invoke must be a const member-function");
        return iter->invoker(ptr->get(), params);
    }

    RAINY_NODISCARD auto get_vars(const std::string_view name) const -> any {
        rainy_ref property_info_vec = instance._property_info;
        const auto iter = std::find_if(property_info_vec.begin(), property_info_vec.end(),
                                       [&name](const property_info &info) { return info.name == name; });
        rainy::foundation::utility::ensures(iter != property_info_vec.end(), "failed to find invoke");
        return iter->getter(ptr->get());
    }

    void set_vars(const std::string_view name, const rainy::containers::any &val) {
        rainy_ref property_info_vec = instance._property_info;
        const auto iter = std::find_if(property_info_vec.begin(), property_info_vec.end(),
                                       [&name](const property_info &info) { return info.name == name; });
        rainy::foundation::utility::ensures(iter != property_info_vec.end(), "failed to find invoke");
        unused += 0;
        iter->setter(ptr->get(), val);
    }

    RAINY_NODISCARD property get_property(const std::string_view name) const {
        rainy_ref property_info_vec = instance._property_info;
        const auto iter = std::find_if(property_info_vec.begin(), property_info_vec.end(),
                                       [&name](const property_info &info) { return info.name == name; });
        rainy::foundation::utility::ensures(iter != property_info_vec.end(), "failed to find invoke");
        void *object_address = ptr->get();
        return {*iter, object_address};
    }

    RAINY_NODISCARD method get_method(const std::string_view name) const {
        rainy_ref method_info_vec = instance._method_info;
        const auto iter =
            std::find_if(method_info_vec.begin(), method_info_vec.end(), [&name](const method_info &info) { return info.name == name; });
        rainy::foundation::utility::ensures(iter != method_info_vec.end(), "failed to find invoke");
        void *object_address = ptr->get();
        return {*iter, object_address};
    }

    RAINY_NODISCARD std::vector<method> get_method_list() const {
        std::vector<method> list;
        void *object_address = ptr->get();
        for (auto &i: instance._method_info) {
            if (i.is_const) {
                list.emplace_back(i, object_address);
            }
        }
        return list;
    }

    RAINY_NODISCARD std::vector<const_property> get_property_list() const {
        std::vector<const_property> list;
        void *object_address = ptr->get();
        for (auto &i: instance._property_info) {
            if (i.is_const) {
                list.emplace_back(i, object_address);
            }
        }
        return list;
    }

    RAINY_NODISCARD std::vector<method> get_method_list() {
        std::vector<method> list;
        void *object_address = ptr->get();
        for (auto &i: instance._method_info) {
            list.emplace_back(i, object_address);
        }
        return list;
    }

    RAINY_NODISCARD std::vector<property> get_property_list() {
        std::vector<property> list;
        void *object_address = ptr->get();
        for (auto &i: instance._property_info) {
            list.emplace_back(i, object_address);
        }
        return list;
    }

    RAINY_NODISCARD property operator[](const std::string_view name) const {
        return get_property(name);
    }

    RAINY_NODISCARD shared_object &object_view() {
        return *this;
    }

    RAINY_NODISCARD const shared_object &object_view() const {
        return *this;
    }

#if RAINY_ENABLE_DEBUG
    RAINY_NODISCARD std::size_t _use_count() const noexcept {
        if (ptr) {
            return ptr->use_count();
        }
        return 0;
    }
#endif

private:
    class_typeid &instance;

    struct object {
        virtual ~object() = default;
        RAINY_NODISCARD virtual std::size_t use_count() const noexcept = 0;
        RAINY_NODISCARD virtual void *get() const noexcept = 0;
    };

    template <typename Class>
    struct object_impl final : object {
        explicit object_impl(const std::shared_ptr<Class> &ptr) : ptr(ptr) {
        }

        ~object_impl() override = default;

        RAINY_NODISCARD std::size_t use_count() const noexcept override {
            return ptr.use_count();
        }

        RAINY_NODISCARD void *get() const noexcept override {
            return ptr.get();
        }

        std::shared_ptr<Class> ptr;
    };

    char *unused{nullptr};
    std::unique_ptr<object> ptr;
};
