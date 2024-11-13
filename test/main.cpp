#include <iostream>

#include <cstdio>
#include <rainy/experimental/format.hpp>
#include <rainy/system/exceptions.hpp>
#include <span>

template <typename Fn, typename... Args>
void benchmark(const char *const label, Fn &&func, Args &&...args) {
    const auto start = std::chrono::high_resolution_clock::now();
    (void) std::forward<Fn>(func)(std::forward<Args>(args)...);
    const auto end = std::chrono::high_resolution_clock::now();
    auto time = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
    std::cout << label << " took " << time.count() << " nanosecond\n";
}

#include <utility>
#include <variant>
#include <bitset>
#include <sstream>
#include <unordered_set>
#include <rainy/containers/any.hpp>
#include <rainy/meta/hash.hpp>
#include <rainy/utility.hpp>
#include <rainy/containers/stack.hpp>

// declare
namespace rainy::foundation::reflection {
    class lunar;
    class shared_object;
    class const_reflclass;
    class reflclass;
}

#include <rainy/meta/reflection_core/field.hpp>
#include <rainy/meta/reflection_core/method.hpp>

namespace rainy::foundation::reflection {
    template <typename Ty>
    struct method_traits {
        static constexpr bool is_const = false;
        static constexpr bool is_static = false;
        static constexpr bool valid = false;
        using _type = Ty;
        using return_type = void;
    };

    template <typename Class, typename ReturnType, typename... Args>
    struct method_traits<ReturnType (Class::*)(Args...)> {
        static constexpr bool is_const = false;
        static constexpr bool is_static = false;
        static constexpr bool valid = true;
        using return_type = ReturnType;
    };

    template <typename Class, typename ReturnType, typename... Args>
    struct method_traits<ReturnType (Class::*)(Args...) const> {
        static constexpr bool is_const = true;
        static constexpr bool is_static = false;
        static constexpr bool valid = true;
        using return_type = ReturnType;
    };

    template <typename ReturnType, typename... Args>
    struct method_traits<ReturnType (*)(Args...)> {
        static constexpr bool is_const = false;
        static constexpr bool is_static = true;
        static constexpr bool valid = true;
        using return_type = ReturnType;
    };

    template <typename Ty>
    struct member_pointer_class {
        using type = Ty;
        static constexpr bool valid = false;
    };

    template <typename Class, typename Ty>
    struct member_pointer_class<Ty Class::*> {
        using type = Class;
        static constexpr bool valid = true;
    };

    enum class type {
        enum_type,
        class_type,
        union_type,
        interface_type
    };

    enum class info_type {
        field,
        method_info,
        class_info,
        enum_field,
        unknown
    };

    struct info_collection {
        info_type type;
        void *resouces;
    };

    struct class_info {
        class_info() = default;
        virtual ~class_info() = default;
        class_info(const class_info &) = default;
        class_info(class_info &&) = default;
        class_info &operator=(const class_info &) = default;
        class_info &operator=(class_info &&) = default;

        RAINY_NODISCARD virtual const utility::type_info &info() const noexcept = 0;
        RAINY_NODISCARD virtual type get_type() const noexcept = 0;
        RAINY_NODISCARD virtual std::string_view name() const noexcept = 0;
        RAINY_NODISCARD virtual info_collection get_info(std::string_view name) noexcept = 0;
        RAINY_NODISCARD virtual std::size_t hash_code() const noexcept = 0;
    };

    template <typename Ty>
    class enum_type final : public class_info {
    public:
        enum_type() : _info(utility::type_info::create<Ty>()) {
        }

        RAINY_NODISCARD const utility::type_info &info() const noexcept override {
            return _info;
        }

        RAINY_NODISCARD type get_type() const noexcept override {
            return type::enum_type;
        }

        RAINY_NODISCARD std::string_view name() const noexcept override {
            return _info.name();
        }

        RAINY_NODISCARD info_collection get_info(const std::string_view name) noexcept override {
            if (name == "field") {
                return {info_type::enum_field, static_cast<void *>(&field)};
            }
            return {info_type::unknown, nullptr};
        }

        RAINY_NODISCARD std::size_t hash_code() const noexcept override {
            return _info.hash_code();
        }

    private:
        utility::type_info _info{};
        std::vector<enum_field> field;
    };

    template <typename Ty>
    class union_type final : public class_info {
    public:
        union_type() : _info(utility::type_info::create<Ty>()) {
        }

        RAINY_NODISCARD const utility::type_info &info() const noexcept override {
            return _info;
        }

        RAINY_NODISCARD type get_type() const noexcept override {
            return type::union_type;
        }

        RAINY_NODISCARD std::string_view name() const noexcept override {
            return _info.name();
        }

        RAINY_NODISCARD info_collection get_info(const std::string_view name) noexcept override {
            if (name == "field") {
                return {info_type::field, static_cast<void *>(&_field)};
            }
            return {info_type::unknown, nullptr};
        }

        RAINY_NODISCARD std::size_t hash_code() const noexcept override {
            return _info.hash_code();
        }

    private:
        utility::type_info _info;
        std::vector<field> _field;
    };

    template <typename Ty>
    class base_type : public class_info {
    public:
        base_type(const type type_kind) : _info(utility::type_info::create<Ty>()), _type_kind(type_kind) {
        }

        RAINY_NODISCARD const utility::type_info &info() const noexcept override {
            return _info;
        }

        RAINY_NODISCARD type get_type() const noexcept override {
            return _type_kind;
        }

        RAINY_NODISCARD std::string_view name() const noexcept override {
            return _info.name();
        }

        RAINY_NODISCARD std::size_t hash_code() const noexcept override {
            return _info.hash_code();
        }

    protected:
        utility::type_info _info;
        type _type_kind;
        std::vector<field> field_;
        std::vector<method> method_field_;
        std::vector<class_info *> derived;
    };

    template <typename Ty>
    class class_type final : public base_type<Ty> {
    public:
        class_type() : base_type<Ty>(type::class_type) {
        }

        info_collection get_info(const std::string_view name) noexcept override {
            if (name == "field") {
                return {info_type::field, static_cast<void *>(&this->field_)};
            }
            if (name == "method") {
                return {info_type::method_info, static_cast<void *>(&this->method_field_)};
            }
            if (name == "base") {
                return {info_type::class_info, static_cast<void *>(&base)};
            }
            if (name == "derived") {
                return {info_type::class_info, static_cast<void *>(&this->derived)};
            }
            return {info_type::unknown, nullptr};
        }

    private:
        std::vector<class_info *> base;
    };

    template <typename Ty>
    class interface_type final : public base_type<Ty> {
    public:
        interface_type() : base_type<Ty>(type::interface_type) {
        }

        info_collection get_info(const std::string_view name) override {
            if (name == "field") {
                return {info_type::field, static_cast<void *>(&this->field)};
            }
            if (name == "method") {
                return {info_type::method_info, static_cast<void *>(&this->method_field)};
            }
            if (name == "derived") {
                return {info_type::class_info, static_cast<void *>(&this->derived)};
            }
            return {info_type::unknown, nullptr};
        }
    };
}

namespace rainy::foundation::reflection {
    class const_reflclass {
    public:
        friend class reflclass;

        const_reflclass() = delete;
        const_reflclass(const const_reflclass &) = default;
        const_reflclass(const_reflclass &&) = default;
        const_reflclass &operator=(const const_reflclass &) = delete;
        const_reflclass &operator=(const_reflclass &&) = delete;
        ~const_reflclass() noexcept = default;

        const_reflclass(const std::unique_ptr<class_info> &instance) noexcept : instance(instance) {
        }

        RAINY_NODISCARD bool valid() const noexcept {
            return static_cast<bool>(instance);
        }

        explicit operator bool() const noexcept {
            return static_cast<bool>(instance);
        }

        RAINY_NODISCARD const utility::type_info &info() const noexcept {
            return instance->info();
        }

        RAINY_NODISCARD type get_type() const noexcept {
            return instance->get_type();
        }

        RAINY_NODISCARD std::string_view name() const noexcept {
            return instance->name();
        }

        RAINY_NODISCARD std::size_t hash_code() const noexcept {
            return instance->hash_code();
        }

        template <typename TestClass>
        RAINY_NODISCARD bool is_base_from_this() const noexcept {
            switch (get_type()) {
                case type::class_type: {
                    auto *base_class_list = static_cast<std::vector<class_info *> *>(instance->get_info("base").resouces);
                    const std::size_t test_class_hashcode = rainy::utility::get_typeid<TestClass>().hash_code();
                    const auto iter =
                        std::find_if(base_class_list->cbegin(), base_class_list->cend(),
                                     [&test_class_hashcode](const auto &info) { return info.hash_code() == test_class_hashcode; });
                    return iter != base_class_list->cend();
                }
                case type::enum_type:
                    RAINY_FALLTHROUGH;
                case type::interface_type:
                    RAINY_FALLTHROUGH;
                case type::union_type:
                    break;
            }
            return false;
        }

        template <typename TestClass>
        RAINY_NODISCARD bool is_derived_from_this() const noexcept {
            switch (get_type()) {
                case type::interface_type:
                    RAINY_FALLTHROUGH;
                case type::class_type: {
                    auto *derived_class_list = static_cast<std::vector<class_info *> *>(instance->get_info("derived").resouces);
                    const std::size_t test_class_hashcode = rainy::utility::get_typeid<TestClass>().hash_code();
                    const auto iter =
                        std::find_if(derived_class_list->cbegin(), derived_class_list->cend(),
                                     [&test_class_hashcode](const auto &info) { return info.hash_code() == test_class_hashcode; });
                    return iter != derived_class_list->cend();
                }
                case type::enum_type:
                    RAINY_FALLTHROUGH;
                case type::union_type:
                    break;
            }
            return false;
        }

        RAINY_NODISCARD bool has_base() const noexcept {
            switch (get_type()) {
                case type::class_type: {
                    auto *base_class_list = static_cast<std::vector<class_info *> *>(instance->get_info("base").resouces);
                    return !base_class_list->empty();
                }
                case type::enum_type:
                    RAINY_FALLTHROUGH;
                case type::interface_type:
                    RAINY_FALLTHROUGH;
                case type::union_type:
                    break;
            }
            return false;
        }

        RAINY_NODISCARD bool has_derived() const noexcept {
            switch (get_type()) {
                case type::interface_type:
                    RAINY_FALLTHROUGH;
                case type::class_type: {
                    auto *base_class_list = static_cast<std::vector<class_info *> *>(instance->get_info("derived").resouces);
                    return !base_class_list->empty();
                }
                case type::enum_type:
                    RAINY_FALLTHROUGH;
                case type::union_type:
                    break;
            }
            return false;
        }

    private:
        const std::unique_ptr<class_info> &instance;
    };

    class reflclass final : public const_reflclass {
    public:
        reflclass(const std::unique_ptr<class_info> &instance) : const_reflclass(instance) {
        }

        template <auto Field, bool NoReturn = false,
                  std::enable_if_t<std::is_member_object_pointer_v<decltype(Field)> || std::is_enum_v<decltype(Field)>, int> = 0>
        void add_field(const std::string_view name = rainy::utility::variable_name<Field>(), const attributes &attr = {}) noexcept
             {
            //return lunar::instance()->add_field<Field, NoReturn>(name, attr);
        }

    private:
        template <typename Container>
        static bool no_same_name_field(const Container &container, std::string_view name) noexcept {
            return std::find_if(container.cbegin(), container.cend(), [&name](const auto &info) { return info.name() == name; }) ==
                   container.cend();
        }
    };

    class shared_object {
    public:
        template <typename Class>
        shared_object(const const_reflclass instance, const std::shared_ptr<Class> object_ptr) :
            instance(instance), object(std::make_unique<object_impl<Class>>(object_ptr)) {
        }

        shared_object &operator=(const shared_object &) = delete;
        shared_object &operator=(shared_object &&) = delete;
        shared_object(const shared_object &) = delete;
        shared_object(shared_object &&) = delete;

        std::string_view name() const noexcept {
            return instance.name();
        }

        void set_var() {
        }

    private:
        struct object {
            virtual ~object() = default;
            RAINY_NODISCARD virtual std::size_t use_count() const noexcept = 0;
            RAINY_NODISCARD virtual void *get() const noexcept = 0;
        };

        template <typename Class>
        struct object_impl final : object {
            explicit object_impl(const std::shared_ptr<Class> &ptr) : instance(ptr) {
            }

            ~object_impl() override = default;

            RAINY_NODISCARD std::size_t use_count() const noexcept override {
                return instance.use_count();
            }

            RAINY_NODISCARD void *get() const noexcept override {
                return instance.get();
            }

            std::shared_ptr<Class> instance;
        };

        const const_reflclass instance;
        std::unique_ptr<object> object;
    };
}


namespace rainy::foundation::reflection {
    template <typename ClassType, typename ReturnType, typename... Args>
    constexpr auto overload_method(ReturnType (ClassType::*method)(Args...)) {
        return method;
    }

    template <typename ClassType, typename ReturnType, typename... Args>
    constexpr auto overload_cmethod(ReturnType (ClassType::*method)(Args...) const) {
        return method;
    }

    template <typename ClassType, typename ReturnType, typename... Args>
    constexpr auto overload_static_method(ReturnType (*method)(Args...)) {
        return method;
    }

    class lunar final {
    public:
        static lunar *instance() noexcept {
            static lunar lunar_instance;
            return &lunar_instance;
        }

        lunar(const lunar &) = delete;
        lunar(lunar &&) = delete;
        lunar &operator=(const lunar &) = delete;
        lunar &operator=(lunar &&) = delete;

        void initializer() {
            static std::once_flag once_flag;
            std::call_once(once_flag, [this]() {
                this->storage.max_load_factor(0.80);
                this->storage.reserve(10);
            });
        }

        template <typename Class, typename... Args,
                  std::enable_if_t<std::is_constructible_v<Class, Args...> && type_traits::support_type<Class>, int> = 0>
        std::shared_ptr<shared_object> make_shared_object(Args &&...args) {
            return is_registered<Class>()
                       ? std::make_shared<shared_object>(get_classinfo<Class>(), std::make_shared<Class>(utility::forward<Args>(args)...))
                       : nullptr;
        }

        template <typename Class, typename Base = void, std::enable_if_t<type_traits::support_type<Class, Base>, int> = 0>
        void register_class() {
            if (is_registered<Class>()) {
                return;
            }
            auto &class_type_info = rainy::utility::get_typeid<Class>();
            if constexpr (std::is_union_v<Class>) {
                storage.insert({class_type_info.hash_code(), std::make_unique<union_type<Class>>()});
            } else if constexpr (std::is_class_v<Class>) {
                storage.insert({class_type_info.hash_code(), std::make_unique<class_type<Class>>()});
            } else if constexpr (std::is_enum_v<Class>) {
                storage.insert({class_type_info.hash_code(), std::make_unique<enum_type<Class>>()});
            } else {
                storage.insert({class_type_info.hash_code(), std::make_unique<interface_type<Class>>()});
            }
            if constexpr (!std::is_void_v<Base> && std::is_base_of_v<Base, Class>) {
                const bool is_base_registerd = is_registered<Base>();
                register_class<Base, void>();
                const auto &derived_class_info = storage.at(class_type_info.hash_code());
                const auto &base_class_info = storage.at(rainy::utility::get_typeid<Base>().hash_code());
                const std::size_t type_hash = derived_class_info->hash_code();
                const std::size_t base_hash = base_class_info->hash_code();
                // 建立继承树
                // 父类添加子类信息
                auto *derived_class_list = static_cast<std::vector<class_info *> *>(base_class_info->get_info("derived").resouces);
                if (!derived_class_list) {
                    std::terminate(); // 不应当发生
                }
                if (is_base_registerd) {
                    const auto find = std::find_if(derived_class_list->begin(), derived_class_list->end(),
                                                   [&base_hash](const class_info *info) { return info->hash_code() == base_hash; });
                    if (find == derived_class_list->end()) {
                        derived_class_list->emplace_back(derived_class_info.get());
                    }
                } else {
                    derived_class_list->emplace_back(derived_class_info.get());
                }
                if constexpr (!std::is_abstract_v<Base>) {
                    // 子类添加父类信息
                    auto *base_class_list = static_cast<std::vector<class_info *> *>(derived_class_info->get_info("base").resouces);
                    if (!base_class_list) {
                        std::terminate(); // 不应当发生
                    }
                    const auto find = std::find_if(base_class_list->begin(), base_class_list->end(),
                                                   [&type_hash](const class_info *info) { return info->hash_code() == type_hash; });
                    if (find == base_class_list->end()) {
                        base_class_list->emplace_back(base_class_info.get());
                    }
                }
            }
        }

        template <typename Class, std::enable_if_t<rainy::foundation::reflection::type_traits::support_type<Class>, int> = 0>
        bool is_registered() const noexcept {
            return is_registered(utility::get_typeid<Class>());
        }

        bool is_registered(const std::type_info& info) const noexcept {
#if RAINY_HAS_CXX20
            return storage.contains(info.hash_code());
#else
            return storage.find(info.hash_code()) != storage.end();
#endif
        }

        template <typename Class>
        void load_class() {
            rainy_let class_hashcode = rainy::utility::get_typeid<Class>().hash_code();
            static std::unordered_set<std::string_view> seen_field;
            static std::unordered_set<std::string_view> seen_method;
            seen_field.clear();
            seen_method.clear();
            std::vector<field> field_vector = Class::lunar_get_field();
            std::vector<method> method_vector = Class::lunar_get_method();
            for (const auto &field_vector_it: field_vector) {
                rainy::utility::ensures(seen_field.find(field_vector_it.name()) == seen_field.end());
                seen_field.insert(field_vector_it.name());
            }
            for (const auto &method_vector_it: method_vector) {
                rainy::utility::ensures(seen_method.find(method_vector_it.name()) == seen_method.end());
                seen_method.insert(method_vector_it.name());
            }
            storage.insert({class_hashcode, std::make_unique<class_type<Class>>()});
            std::unique_ptr<class_info> &class_instance = storage[class_hashcode];
            rainy_let instance_field_ptr = static_cast<std::vector<field> *>(class_instance->get_info("field").resouces);
            rainy_let instance_method_ptr = static_cast<std::vector<method> *>(class_instance->get_info("method").resouces);
            if (!(instance_field_ptr && instance_method_ptr)) {
                std::terminate();
            }
            (*instance_field_ptr) = field_vector;
            (*instance_method_ptr) = method_vector;
        }

        template <typename Class, std::enable_if_t<rainy::foundation::reflection::type_traits::support_type<Class, void>, int> = 0>
        void unregister_class() {
            if (!is_registered<Class>()) {
                return;
            }
            /* 接口类和正常的类需要移除继承树 */
            const std::size_t remove_key_hash = rainy::utility::get_typeid<Class>().hash_code();
            auto &remove_class = storage.at(remove_key_hash);
            const type class_type = remove_class->get_type();
            switch (class_type) {
                case type::interface_type:
                    RAINY_FALLTHROUGH;
                case type::class_type: {
                    /* 接口类和正常类都具有derived继承树，所以此处合并逻辑，统一处理 */
                    auto *derived_class_list = static_cast<std::vector<class_info *> *>(remove_class->get_info("derived").resouces);
                    if (!derived_class_list) {
                        std::terminate(); // 不应该发生
                    }
                    derived_remove_base(derived_class_list, remove_key_hash); // 删除派生类继承树
                    if (class_type == type::class_type) {
                        /* 因为正常类具有base继承，我们需要从base中安全删除 */
                        auto *base_class_list = static_cast<std::vector<class_info *> *>(remove_class->get_info("base").resouces);
                        for (auto base: *base_class_list) {
                            auto *_derived_class_list = static_cast<std::vector<class_info *> *>(base->get_info("derived").resouces);
                            /* 获取派生列表 */
                            if (!_derived_class_list) {
                                std::terminate(); // 不应该发生
                            }
                            const auto remove_iter =
                                std::find_if(_derived_class_list->begin(), _derived_class_list->end(),
                                             [&remove_key_hash](const class_info *info) { return info->hash_code() == remove_key_hash; });
                            if (remove_iter != _derived_class_list->end()) {
                                _derived_class_list->erase(remove_iter); // 找到就移除
                            }
                        }
                    }
                    break;
                }
                case type::enum_type:
                    RAINY_FALLTHROUGH;
                case type::union_type:
                    RAINY_FALLTHROUGH;
                default:
                    break;
            }
            storage.erase(remove_key_hash);
            invalidate_cache(remove_key_hash); // 避免缓存不一致（我们只需要让特定类的缓存失效即可）
        }

        template <typename Class, typename Base, std::enable_if_t<std::is_class_v<Class> && std::is_base_of_v<Base, Class>, int> = 0>
        void add_base() {
            if (!is_registered<Class>()) {
                return;
            }
            const std::size_t type_hash = utility::get_typeid<Class>().hash_code();
            const std::size_t base_hash = utility::get_typeid<Base>().hash_code();
            auto &add_base_class = storage.at(type_hash);
            const type class_type = add_base_class->get_type();
#if RAINY_ENABLE_DEBUG
            utility::expects(class_type == type::class_type, "Only class_type have base"); // 只有正常的类才拥有父类
#else
            if (class_type != type::class_type) {
                return;
            }
#endif
            const bool is_base_registerd = is_registered<Base>(); // 提前获取状态
            register_class<Base>();
            const auto &derived_class_info = storage.at(type_hash);
            const auto &base_class_info = storage.at(base_hash);
            // 建立继承树
            // 父类添加子类信息
            auto *derived_class_list = static_cast<std::vector<class_info *> *>(base_class_info->get_info("derived").resouces);
            if (!derived_class_list) {
                std::terminate(); // 不应当发生
            }
            if (is_base_registerd) /* 确保不产生冲突 */ {
                // 如果base在此注册过，那么需要检查，否则我们并不需要
                const auto find = std::find_if(derived_class_list->begin(), derived_class_list->end(),
                                               [&type_hash](const class_info *info) { return info->hash_code() == type_hash; });
                if (find == derived_class_list->end()) {
                    // 如果已经注册，需要确保父类的列表不能出现两个Class代表的类
                    derived_class_list->emplace_back(derived_class_info.get());
                }
            } else {
                // 如果没有注册，这个操作是完全合法的
                derived_class_list->emplace_back(derived_class_info.get());
            }
            // 子类添加父类信息
            auto *base_class_list = static_cast<std::vector<class_info *> *>(derived_class_info->get_info("base").resouces);
            if (!base_class_list) {
                std::terminate(); // 不应当发生
            }
            /* 此处我们默认该类是注册过的。所以添加的时候，必须确保没有冲突 */
            const auto find = std::find_if(base_class_list->begin(), base_class_list->end(),
                                           [&base_hash](const class_info *info) { return info->hash_code() == base_hash; });
            if (find == base_class_list->end()) {
                base_class_list->emplace_back(base_class_info.get());
            }
        }

        template <typename Class, typename Base, std::enable_if_t<std::is_class_v<Class> && std::is_base_of_v<Base, Class>, int> = 0>
        void remove_base() {
            /* 两个必须被注册 */
            if (!is_registered<Class>() || !is_registered<Base>()) {
                return;
            }
            const std::size_t type_hash = utility::get_typeid<Class>().hash_code();
            const std::size_t base_hash = utility::get_typeid<Base>().hash_code();
            auto &remove_base_class = storage.at(type_hash);
            const type class_type = remove_base_class->get_type();
            utility::expects(class_type == type::class_type, "Only class_type have base");
            const auto &derived_class_info = storage.at(type_hash);
            const auto &base_class_info = storage.at(base_hash);
            auto *derived_class_list = static_cast<std::vector<class_info *> *>(base_class_info->get_info("derived").resouces);
            auto *base_class_list = static_cast<std::vector<class_info *> *>(derived_class_info->get_info("base").resouces);
            if (!derived_class_list || !base_class_list) {
                std::terminate(); // 不应当发生
            }
            {
                const auto find = std::find_if(derived_class_list->begin(), derived_class_list->end(),
                                               [&type_hash](const class_info *info) { return info->hash_code() == type_hash; });
                if (find != derived_class_list->end()) {
                    // 如果已经注册，需要确保父类的列表不能出现两个Class代表的类
                    derived_class_list->erase(find);
                }
            }
            {
                const auto find = std::find_if(base_class_list->begin(), base_class_list->end(),
                                               [&base_hash](const class_info *info) { return info->hash_code() == base_hash; });
                if (find != base_class_list->end()) {
                    base_class_list->erase(find);
                }
            }
        }

        RAINY_NODISCARD bool empty() const noexcept {
            return storage.empty();
        }

        RAINY_NODISCARD std::size_t size() const noexcept {
            return storage.size();
        }

        template <auto Field, bool NoReturn = false,
                  std::enable_if_t<std::is_member_object_pointer_v<decltype(Field)> || std::is_enum_v<decltype(Field)>, int> = 0>
        auto add_field(const std::string_view name = rainy::utility::variable_name<Field>(), const attributes &attr = {}) noexcept
            -> rainy::type_traits::other_transformations::conditional_t<
                NoReturn, void,
                rainy::type_traits::other_transformations::conditional_t<
                    !member_pointer_class<decltype(Field)>::valid, utility::pair<bool, enum_field *>, utility::pair<bool, field *>>> {
            // 根据Field的类型决定返回，如果Field是一个枚举，则返回rainy::utility::pair<bool,enum_field*>，否则rainy::utility::pair<bool,field*>
            using field_type = decltype(Field);
            using traits = member_pointer_class<field_type>;
            using trait_type = typename traits::type;
#if RAINY_ENABLE_DEBUG
            utility::expects(is_registered<trait_type>(),
                             "You must register Derived class before invoking this method."); // 检查注册
#else
            if (!is_registered<trait_type>()) {
                if constexpr (!NoReturn) {
                    return {false, nullptr};
                } else {
                    return;
                }
            }
#endif
            const auto &types = storage.at(typeid(trait_type).hash_code());
            if constexpr (!traits::valid) {
                // 为枚举类注册
                if constexpr (std::is_enum_v<trait_type>) {
                    rainy::utility::expects(types->get_type() == type::enum_type, "Expected enum_type, but not enum_type");
                    auto *enum_fields = static_cast<std::vector<enum_field> *>(types->get_info("field").resouces);
                    if (!enum_fields) {
                        std::terminate();
                    }
#if RAINY_ENABLE_DEBUG
                    rainy::utility::ensures(no_same_name_field(*enum_fields, name), "Add a samename field is a invalid operation!");
#else
                    if (!no_same_name_field(*enum_fields, name)) {
                        if constexpr (!NoReturn) {
                            return;
                        } else {
                            return {false, nullptr};
                        }
                    }
#endif
                    enum_fields->emplace_back(name, types->name(), Field);
                    if constexpr (!NoReturn) {
                        return {true, &this->get_field<trait_type>(name)};
                    }
                }
            } else {
                auto *field_infos = static_cast<std::vector<field> *>(types->get_info("field").resouces);
                if (!field_infos) {
                    std::terminate();
                }
#if RAINY_ENABLE_DEBUG
                rainy::utility::ensures(no_same_name_field(*field_infos, name), "Add a samename field is a invalid operation!");
#else
                if (!no_same_name_field(*field_infos, name)) {
                    if constexpr (!NoReturn) {
                        return {false, nullptr};
                    } else {
                        return;
                    }
                }
#endif
                field_infos->emplace_back(make_field<trait_type>(name, Field, attr));
                if constexpr (!NoReturn) {
                    return {true, &this->get_field<trait_type>(name)};
                }
            }
        }

        template <auto Method, bool NoReturn = false, std::enable_if_t<std::is_member_function_pointer_v<decltype(Method)>, int> = 0>
        auto add_method(const std::string_view name = rainy::utility::variable_name<Method>(), const attributes &attr = {}) noexcept
            -> rainy::type_traits::other_transformations::conditional_t<NoReturn, void, utility::pair<bool, method *>> {
            using method_type = decltype(Method);
            using traits = method_traits<method_type>;
            using trait_type = typename member_pointer_class<method_type>::type;
#if RAINY_ENABLE_DEBUG
            utility::expects(is_registered<trait_type>(),
                             "You must register Derived class before invoking this method."); // 检查注册
#else
            if (!is_registered<trait_type>()) {
                if constexpr (!NoReturn) {
                    return {false, nullptr};
                } else {
                    return;
                }
            }
#endif
            utility::pair<bool, method *> pair;
            switch (const auto &types = storage.at(typeid(trait_type).hash_code()); types->get_type()) {
                case type::interface_type:
                    RAINY_FALLTHROUGH;
                case type::class_type: {
                    auto *method_infos = static_cast<std::vector<method> *>(types->get_info("method").resouces);
                    if (!method_infos) {
                        std::terminate();
                    }
#if RAINY_ENABLE_DEBUG
                    utility::ensures(no_same_name_field(*method_infos, name), "Add a samename method is a invalid operation!");
#else
                    if (!no_same_name_field(*method_infos, name)) {
                        if constexpr (!NoReturn) {
                            return {false, nullptr};
                        } else {
                            return;
                        }
                    }
#endif
                    if constexpr (traits::is_const) {
                        method_infos->emplace_back(make_const_method<trait_type>(name, Method));
                    } else if constexpr (!(traits::is_const && traits::is_static)) {
                        method_infos->emplace_back(make_method<trait_type>(name, Method));
                    } else {
                        static_assert(rainy::type_traits::internals::always_false<void>);
                    }
                    break;
                }
                case type::union_type:
                    RAINY_FALLTHROUGH;
                case type::enum_type:
                    RAINY_FALLTHROUGH;
                default:
#if RAINY_ENABLE_DEBUG
                    /* 枚举和联合体是不允许拥有方法的，所以此处将报错（DEBUG模式） */
                    utility::ensures(false, "Not supported types !");
#else
                    if constexpr (!NoReturn) {
                        return {false, nullptr};
                    } else {
                        return;
                    }
#endif
            }
            if constexpr (!NoReturn) {
                return {true, &get_method<trait_type>(name)};
            }
        }

        template <typename Class, auto Method, std::enable_if_t<std::is_class_v<Class>, int> = 0>
        void add_static_method(const std::string_view name, const attributes &attr = {}) noexcept {
            utility::expects(is_registered<Class>(),
                                                   "You must register Derived class before invoking this method."); // 检查注册
            switch (const auto &types = storage.at(typeid(Class).hash_code()); types->get_type()) {
                case type::interface_type:
                    RAINY_FALLTHROUGH;
                case type::class_type: {
                    auto *method_infos = static_cast<std::vector<method> *>(types->get_info("method").resouces);
                    if (!method_infos) {
                        std::terminate();
                    }
#if RAINY_ENABLE_DEBUG
                    rainy::utility::ensures(no_same_name_field(*method_infos, name));
#else
                    if (!no_same_name_field(*method_infos, name)) {
                        return;
                    }
#endif
                    method_infos->emplace_back(make_static_method(name, Method, attr));
                    break;
                }
                case type::union_type:
                    RAINY_FALLTHROUGH;
                case type::enum_type:
                    RAINY_FALLTHROUGH;
                default:
                    rainy::utility::ensures(false, "Not supported types!");
                    /* 枚举和联合体是不允许拥有方法的，所以此处将报错 */
            }
        }

        template <typename Class,
                  std::enable_if_t<std::is_enum_v<Class> || std::is_class_v<Class> || std::is_union_v<Class> || std::is_abstract_v<Class>,
                                   int> = 0>
        const auto &get_field(const std::string_view name) const {
            const std::size_t hash_code = typeid(Class).hash_code();
            auto &types = storage.at(hash_code);
            if constexpr (std::is_enum_v<Class>) {
                static const enum_field enum_empty;
                if (!is_registered<Class>()) {
                    return enum_empty;
                }
                rainy::utility::expects(types->get_type() == type::enum_type, "Expected enum_type, but not enum_type");
                auto *enum_fields = static_cast<std::vector<enum_field> *>(types->get_info("field").resouces);
                if (const auto iter = find_in_container(*enum_fields, name); iter != enum_fields->end()) {
                    return *iter;
                }
                return enum_empty;
            } else {
                static const field empty;
                if (!is_registered<Class>()) {
                    return empty;
                }
                if (const auto find = field_cache_storage.find(types->hash_code()); find != field_cache_storage.end()) {
                    if (const field *info = find->second.get(name); info != nullptr) {
                        return *info;
                    }
                } else {
                    field_cache_storage.insert({types->hash_code(), cache<field>()});
                    field_cache_storage[types->hash_code()].preheat();
                }
                auto *field_infos = static_cast<std::vector<field> *>(types->get_info("field").resouces);
                if (!field_infos) {
                    std::terminate();
                }
                if (const auto iter = find_in_container(*field_infos, name); iter != field_infos->end()) {
                    return *iter;
                }
                std::size_t found_hash{};
                field *found_info = nullptr;
                /* 如果没有查找到，我们可以尝试查找父类 */
                if (types->get_type() != type::union_type && types->get_type() != type::interface_type) {
                    // 联合体和接口是不允许拥有父类的。因此此处进行判断
                    auto *base_class_list_ptr = static_cast<std::vector<class_info *> *>(types->get_info("base").resouces);
                    if (!base_class_list_ptr) {
                        std::terminate();
                    }
                    field *search_result = nullptr;
                    if (search_result = search_in_parent(field_cache_storage, found_info, name, base_class_list_ptr, found_hash);
                        search_result != nullptr) {
                        found_info = search_result;
                    }
                }
                if (found_info) {
                    return *found_info;
                }
                return empty;
            }
        }

        template <typename Class>
        auto &get_field(const std::string_view name) {
            const auto &ret = static_cast<const lunar *>(this)->get_field<Class>(name);
            if constexpr (std::is_same_v<decltype(ret), const field &>) {
                return const_cast<field &>(ret);
            } else {
                return ret;
            }
        }

        template <typename Class, std::enable_if_t<std::is_class_v<Class> || std::is_abstract_v<Class>, int> = 0>
        const method &get_method(const std::string_view name) const {
            static const method empty;
            const std::size_t hash_code = rainy::utility::get_typeid<Class>().hash_code();
            auto &types = storage.at(hash_code);
            rainy::utility::expects(types->get_type() == type::class_type || types->get_type() == type::interface_type);
            if (!is_registered<Class>()) {
                return empty;
            }
            if (const auto find = method_cache_storage.find(types->hash_code()); find != method_cache_storage.end()) {
                if (const method *info = find->second.get(name); info != nullptr) {
                    return *info;
                }
            } else {
                method_cache_storage.insert({types->hash_code(), cache<method>()});
                method_cache_storage[types->hash_code()].preheat();
            }
            const auto *method_infos = static_cast<std::vector<method> *>(types->get_info("method").resouces);
            if (!method_infos) {
                std::terminate();
            }
            if (const auto iter = find_in_container(*method_infos, name); iter != method_infos->end()) {
                return *iter;
            }
            std::size_t found_hash{};
            method *found_info = nullptr;
            /* 如果没有查找到，我们可以尝试查找父类 */
            if (types->get_type() != type::interface_type) {
                // 联合体和接口是不允许拥有父类的。因此此处进行判断
                auto *base_class_list_ptr = static_cast<std::vector<class_info *> *>(types->get_info("base").resouces);
                if (!base_class_list_ptr) {
                    std::terminate();
                }
                if (method *search_result = search_in_parent(method_cache_storage, found_info, name, base_class_list_ptr, found_hash);
                    search_result != nullptr) {
                    found_info = search_result;
                }
            }
            if (found_info) {
                method_cache_storage[found_hash].push_cache(name, *found_info);
                return *found_info;
            }
            return empty;
        }

        template <typename Class>
        method &get_method(const std::string_view name) {
            return const_cast<method &>(static_cast<const lunar *>(this)->get_method<Class>(name));
        }

        template <typename Class,
                  std::enable_if_t<std::is_enum_v<Class> || std::is_class_v<Class> || std::is_union_v<Class> || std::is_abstract_v<Class>,
                                   int> = 0>
        void remove_field(const std::string_view name) {
            if (!is_registered<Class>()) {
                return;
            }
            const std::size_t hash_code = rainy::utility::get_typeid<Class>().hash_code();
            const auto &types = storage.at(hash_code);
            auto *field_infos = static_cast<std::vector<field> *>(types->get_info("field").resouces);
            if (!field_infos) {
                std::terminate();
            }
            const auto iter = find_in_container(*field_infos, name);
            if (iter != field_infos->end()) {
                field_infos->erase(iter);
            }
        }

        template <typename Class, std::enable_if_t<std::is_class_v<Class> || std::is_abstract_v<Class>, int> = 0>
        void remove_method(const std::string_view name) {
            if (!is_registered<Class>()) {
                return;
            }
            const std::size_t hash_code = rainy::utility::get_typeid<Class>().hash_code();
            const auto &types = storage.at(hash_code);
            auto *method_infos = static_cast<std::vector<field> *>(types->get_info("method").resouces);
            if (!method_infos) {
                std::terminate();
            }
            const auto iter = find_in_container(*method_infos, name);
            if (iter != method_infos->end()) {
                method_infos->erase(iter);
            }
        }

        template <typename Class>
        reflclass get_classinfo() noexcept {
            static std::unique_ptr<class_info> nullpointer;
            static reflclass empty(nullpointer);
            return is_registered<Class>() ? reflclass(storage.at(rainy::utility::get_typeid<Class>().hash_code())) : empty;
        }

        void invalidate_all_cache() noexcept {
            method_cache_storage.clear();
            field_cache_storage.clear();
        }

        void invalidate_method_cache() noexcept {
            method_cache_storage.clear();
        }

        void invalidate_field_cache() noexcept {
            field_cache_storage.clear();
        }

        void invalidate_cache(const std::size_t hash) noexcept {
            if (const auto find = method_cache_storage.find(hash); find != method_cache_storage.end()) {
                find->second.invalidate();
            }
            if (const auto find = field_cache_storage.find(hash); find != field_cache_storage.end()) {
                find->second.invalidate();
            }
        }

    private:
        lunar() noexcept = default;
        ~lunar() noexcept = default;

        template <typename Info>
        class cache {
        public:
            class cache_data {
            public:
                friend class cache<Info>;

                cache_data() = default;

                cache_data(const unsigned int ref_count, Info *data) : ref_count(ref_count), data(data) {
                }

            private:
                unsigned int ref_count;
                Info *data;
            };

            void push_cache(const std::string_view name, Info &info_) noexcept {
                std::size_t hash = std::hash<std::string_view>{}(name);
                cold_segment.insert({hash, {3, &info_}}); // 初次插入时进入冷段
                if (cold_segment.size() > cold_limit) {
                    evict_cold();
                }
            }

            Info *get(const std::string_view name) noexcept {
                const std::size_t hash = std::hash<std::string_view>{}(name);
                if (auto find = hot_segment.find(hash); find != hot_segment.end()) {
                    return find->second.data;
                }
                if (auto find = warm_segment.find(hash); find != warm_segment.end()) {
                    promote_to_hot(hash, find->second);
                    return find->second.data;
                }
                if (auto find = cold_segment.find(hash); find != cold_segment.end()) {
                    promote_to_warm(hash, find->second);
                    return find->second.data;
                }
                return nullptr;
            }

            void invalidate() noexcept {
                hot_segment.clear();
                warm_segment.clear();
                cold_segment.clear();
            }

            void preheat() {
                hot_segment.reserve(hot_limit);
                warm_segment.reserve(warm_limit);
                cold_segment.reserve(cold_limit);
            }

        private:
            void promote_to_warm(const std::size_t hash, cache_data &data) {
                warm_segment[hash] = data;
                cold_segment.erase(hash);
                if (warm_segment.size() > warm_limit) {
                    evict_warm();
                }
            }

            void promote_to_hot(const std::size_t hash, cache_data &data) {
                hot_segment[hash] = data;
                warm_segment.erase(hash);
                if (hot_segment.size() > hot_limit) {
                    evict_hot();
                }
            }

            void evict_cold() {
                if (!cold_segment.empty()) {
                    cold_segment.erase(cold_segment.begin());
                }
            }

            void evict_warm() {
                if (!warm_segment.empty()) {
                    warm_segment.erase(warm_segment.begin());
                }
            }

            void evict_hot() {
                if (!hot_segment.empty()) {
                    hot_segment.erase(hot_segment.begin());
                }
            }

            static inline constexpr size_t hot_limit = 10; // 热段的最大大小
            static inline constexpr size_t warm_limit = 20; // 中段的最大大小
            static inline constexpr size_t cold_limit = 30; // 冷段的最大大小

            std::unordered_map<std::size_t, cache_data> hot_segment; // 频繁访问段
            std::unordered_map<std::size_t, cache_data> warm_segment; // 中等访问段
            std::unordered_map<std::size_t, cache_data> cold_segment; // 冷段
        };

        static void derived_remove_base(std::vector<class_info *> *derived_class_list, const std::size_t remove_key_hash) {
            for (auto derived: *derived_class_list) {
                auto *base_class_list = static_cast<std::vector<class_info *> *>(derived->get_info("base").resouces);
                if (!base_class_list) {
                    std::terminate(); // 不应该发生
                }
                const auto remove_iter =
                    std::find_if(base_class_list->begin(), base_class_list->end(),
                                 [&remove_key_hash](const class_info *info) { return info->hash_code() == remove_key_hash; });
                if (remove_iter != base_class_list->end()) {
                    base_class_list->erase(remove_iter);
                }
            }
        }

        template <typename Container>
        static bool no_same_name_field(const Container &container, std::string_view name) noexcept {
            return std::find_if(container.cbegin(), container.cend(), [&name](const auto &info) { return info.name() == name; }) ==
                   container.cend();
        }

        template <typename Container>
        static typename Container::const_iterator find_in_container(const Container &container, std::string_view name) noexcept {
            return std::find_if(container.begin(), container.end(), [&name](const auto &info) { return info.name() == name; });
        }

        template <typename Container>
        static typename Container::iterator find_in_container(Container &container, std::string_view name) noexcept {
            return std::find_if(container.begin(), container.end(), [&name](const auto &info) { return info.name() == name; });
        }

        template <typename Info, typename CacheStorage>
        static Info *search_in_parent(CacheStorage &cache_storage, Info *found_info, const std::string_view name,
                                      std::vector<class_info *> *base_class_list_ptr, std::size_t &found_hash) {
            auto &base_class_list = *base_class_list_ptr;
            for (auto *parent_class: base_class_list) {
                if (const auto find = cache_storage.find(parent_class->hash_code()); find != cache_storage.end()) {
                    if (Info *info = find->second.get(name); info != nullptr) {
                        return info;
                    }
                } else {
                    cache_storage.insert({parent_class->hash_code(), {}});
                    cache_storage[parent_class->hash_code()].preheat();
                }
                constexpr bool is_searching_method = std::is_same_v<Info, method>;
                if (Info *parent_info = find_data_in_parent<Info>(parent_class, name, is_searching_method ? "method" : "field");
                    parent_info != nullptr) {
                    found_info = parent_info;
                    found_hash = parent_class->hash_code();
                }
            }
            if (found_info) {
                cache_storage[found_hash].push_cache(name, *found_info);
                return found_info;
            }
            return nullptr;
        }

        template <typename info_type>
        static info_type *find_data_in_parent(class_info *parent_class, const std::string_view name, const std::string_view type) {
            if (!parent_class) {
                std::terminate(); // 不应该发生
            }
            info_type *found_info = nullptr;
            rainy::containers::stack_container<class_info *> class_stack;
            std::unordered_set<class_info *> visited_classes; // 用于跟踪已访问过的类，防止循环
            class_stack.push(parent_class);
            visited_classes.insert(parent_class); // 初始类标记为已访问
            while (!class_stack.empty()) {
                class_info *current_class = class_stack.top();
                class_stack.pop();
                auto *info_vec_ptr = static_cast<std::vector<info_type> *>(current_class->get_info(type).resouces);
                if (!info_vec_ptr) {
                    std::terminate();
                }
                auto &info_vec = *info_vec_ptr;
                if (auto iter = find_in_container(info_vec, name); iter != info_vec.end()) {
                    if (found_info) {
                        // 如果已经找到过一个匹配的字段，则表示出错了
                        std::string buffer;
#if RAINY_HAS_CXX20
                        rainy::utility::format(buffer, "Ambiguous field '{}' found in multiple parent classes.", name);
#else
                        rainy::utility::cstyle_format(buffer,"Ambiguous field '%s' found in multiple parent classes.", name.data());
#endif
                        rainy::utility::ensures(false, buffer);
                    }
                    found_info = &(*iter);
                }
                const auto *parent_class_info_ptr = static_cast<std::vector<class_info *> *>(current_class->get_info("base").resouces);
                if (!parent_class_info_ptr) {
                    std::terminate();
                }
                for (auto *parent: *parent_class_info_ptr) {
#if RAINY_HAS_CXX20
                    if (visited_classes.contains(parent)) {
                        class_stack.push(parent);
                        visited_classes.insert(parent); // 标记为已访问
                    }
#else
                    if (visited_classes.find(parent) == visited_classes.end()) { // 检查是否已访问
                        class_stack.push(parent);
                        visited_classes.insert(parent); // 标记为已访问
                    }
#endif
                }
            }
            return found_info;
        }

        mutable std::unordered_map<std::size_t, cache<field>> field_cache_storage;
        mutable std::unordered_map<std::size_t, cache<method>> method_cache_storage;
        std::unordered_map<std::size_t, std::unique_ptr<class_info>> storage;
    };
}

union unions {
    int a;
    int b;
};

template <typename Ty,std::size_t N>
class array {
public:
    using value_type = Ty;
    using size_type = std::size_t;

    constexpr array() noexcept(std::is_nothrow_constructible_v<Ty>) = default;

    RAINY_CONSTEXPR20 array(std::initializer_list<Ty> ilist) noexcept(std::is_nothrow_copy_assignable_v<Ty>) {
        _size = ilist.size();
#if RAINY_HAS_CXX20
        if (std::is_constant_evaluated()) {
            std::copy_n(ilist.begin(), _size, elems);
        } else
#endif
        {
            rainy::utility::expects(ilist.size() <= _size);
            if constexpr (rainy::type_traits::type_properties::is_pod_v<Ty>) {
                std::uninitialized_copy_n(ilist.begin(), _size, elems);
            } else {
                std::copy_n(ilist.begin(), _size, elems);
            }
        }
    }

    RAINY_CONSTEXPR20 array(const array &right) {
        
    }

    constexpr std::size_t size() const noexcept {
        return _size;
    }

    constexpr size_type length() const noexcept {
        return N;
    }

    constexpr std::size_t empty() const noexcept {
        return _size == 0;
    }

    template <typename... Args>
    RAINY_CONSTEXPR20 void emplace_back(Args... args) noexcept {
        range_check(++_size);
        
    }

private:
    void range_check(const size_type idx) {
        if (idx <= N) {
            rainy::foundation::system::exceptions::logic::throw_out_of_range("Invalid array subscript");
        }
    }

    static void check_zero_length_error() {
        if (N == 0) {
            std::terminate();
        }
    }

    std::size_t _size;
    value_type elems[N];
};

namespace static_reflection {
    template <typename Ty>
    struct field {
        using type = Ty;

        constexpr field(std::string_view name, type *pointer, const bool is_const, const bool is_static) :
            name(name), pointer(pointer), is_const(is_const), is_static(is_static) {
        }

        std::string_view name;
        type *pointer;
        bool is_const;
        bool is_static;
    };

    template <typename Class,typename Ty>
    struct field<Ty Class::*> {
        using type = Ty Class::*;

        constexpr field(std::string_view name, Ty Class::* pointer, const bool is_const, const bool is_static) :
            name(name), pointer(pointer), is_const(is_const), is_static(is_static) {
        }

        std::string_view name;
        type pointer;
        bool is_const;
        bool is_static;
    };

    template <typename... Es>
    struct element_list {
        constexpr element_list(Es... es) : elements(es...) {
        }

        template <typename Fx>
        constexpr void for_each(Fx && func) const {
            std::apply([&func](const auto &...e) {
                (func(e), ...); 
            }, elements);
        }

        template <typename Pred>
        constexpr std::size_t find_if(Pred &&pred) const {
            rainy_let index = static_cast<std::size_t>(-1);
            std::size_t i = 0;
            ((pred(std::get<Es>(elements)) ? (index = i, true) : (++i, false)) || ...);
            return index;
        }

        template <typename Name>
        constexpr auto find() const {
            return find_if([](const auto &e) {
                return std::is_same_v<typename decltype(e)::name_type, Name>; 
            });
        }

        template <std::size_t I>
        constexpr const auto &get() const {
            return std::get<I>(elements);
        }

        std::tuple<Es...> elements;
    };

    template <typename... Fields>
    struct field_list : element_list<Fields...> {
        constexpr field_list(Fields... fields) : element_list<Fields...>{fields...} {
        }
    };

    template <typename Class,typename Ty>
    constexpr auto make_field(std::string_view name, Ty Class::*member) {
        return field<Ty Class::*>(name, member, std::is_const_v<Ty Class::*>, false);
    }

    template <typename Ty>
    struct demo {
        using type = Ty;
        int a;

        void fun() {
            std::cout << "fun"
                      << "\n";
        }
    };

    template <typename Ty, typename... Fields>
    struct type_info {
        static constexpr std::string_view name = "unknown";
        using fields = std::tuple<Fields...>;
    };

    template <typename type>
    struct type_info<demo<type>> {
        static constexpr std::string_view name = "demo";
        static constexpr field_list fields = {
            make_field("a", &demo<int>::a), 
            make_field("fun",&demo<int>::fun)
        };
    };
}


class low {
public:
    int low_field = 0;

    virtual void virtual_method() {
    }
};

class base : public low {
public:
    int base_field = 0;

    void virtual_method() override {
        std::cout << "Hello Virtual"
                  << "\n";
    }
};

class my_class : public rainy::component::object<my_class> {
public:
    my_class() {
        std::cout << "default constructer"
                  << "\n";
    };

    my_class(const my_class &) {
        std::cout << "copy constructer"
                  << "\n";
    };

    my_class(int, int) {
        std::cout << "constructer(int,int)"
                  << "\n";
    }

    ~my_class() {
        std::cout << "destructer"
                  << "\n";
    }

    void another() {
        std::cout << "another"
                  << "\n";
    }

    void another() const {
        std::cout << "const another"
                  << "\n";
    }

    void hello() {
        std::cout << "normal method"
                  << "\n";
    }

    void hello(const int a, const int b) const {
    }

    static void hello_static() {
        std::cout << "static method"
                  << "\n";
    }

    static void hello_static(int, int) {
        std::cout << "static method"
                  << "\n";
    }

    my_class &operator=(const my_class &) {
        return *this;
    }

    int a = 10;
    const int b = 10;
    static const int c;
};

const int my_class::c = 10;

#if RAINY_USING_WINDOWS
template <typename CharType>
LSTATUS reg_openkey(HKEY key,DWORD options,const CharType* sub_key, REGSAM desired,PHKEY result) {
    if (!result || !key) {
        return ERROR_BAD_ARGUMENTS;
    }
    if constexpr (rainy::type_traits::helper::is_wchar_t<CharType>) {
        return RegOpenKeyExW(key, sub_key, options, desired, result);
    } else {
        return RegOpenKeyExA(key, sub_key, options, desired, result);
    }
}

class registry {
public:
    enum class predefinekey {
        HKCR,
        HKCC,
        HKCU,
        HKLM,
        HKUSERS
    };

    constexpr registry() noexcept : context(HKEY{}){};

    ~registry() {
        close();
    }

    template <typename CharType>
    LSTATUS switch_context(predefinekey predefine_key, const std::basic_string_view<CharType> path) {
        HKEY key = get_predefine_key(predefine_key);
        if (LSTATUS status = reg_openkey(key, path.data(), 0, KEY_ALL_ACCESS, &context); status == ERROR_SUCCESS) {
            return status;
        } else if (status = reg_openkey(key, path.data(), 0, KEY_READ | KEY_WRITE, &context); status == ERROR_SUCCESS) {
            return status;
        } else if (status = reg_openkey(key, path.data(), 0, KEY_READ, &context); status == ERROR_SUCCESS) {
            return status;
        } else {
            return status;
        }
    }

    template <typename CharType>
    LSTATUS switch_context(HKEY key, const std::basic_string_view<CharType> path) {
        if (!key) {
            return ERROR_BAD_ARGUMENTS;
        }
        if (LSTATUS status = reg_openkey(key, path.data(), 0, KEY_ALL_ACCESS, &context); status == ERROR_SUCCESS) {
            return status;
        } else if (status = reg_openkey(key, path.data(), 0, KEY_READ | KEY_WRITE, &context); status == ERROR_SUCCESS) {
            return status;
        } else if (status = reg_openkey(key, path.data(), 0, KEY_READ, &context); status == ERROR_SUCCESS) {
            return status;
        } else {
            return status;
        }
    }

    void close() {
        if (!valid()) {
            return;
        }
        if (LSTATUS error_code = RegCloseKey(context); error_code != ERROR_SUCCESS) {
            std::string error_message = rainy::winapi::error_process::last_error_message<char>(error_code);
            (void)std::fwrite(error_message.c_str(), sizeof(char), sizeof(char) * error_message.length(), stderr);
        }
        context = nullptr;
    }

    bool valid() const noexcept {
        return static_cast<bool>(context);
    }

private:
    static HKEY get_predefine_key(predefinekey predefine_key) noexcept {
        HKEY open_key{nullptr};
        switch (predefine_key) {
            case predefinekey::HKCR:
                open_key = HKEY_CLASSES_ROOT;
                break;
            case predefinekey::HKCC:
                open_key = HKEY_CURRENT_CONFIG;
                break;
            case predefinekey::HKCU:
                open_key = HKEY_CURRENT_USER;
                break;
            case predefinekey::HKLM:
                open_key = HKEY_LOCAL_MACHINE;
                break;
            case predefinekey::HKUSERS:
                open_key = HKEY_USERS;
                break;
            default:
                break;
        }
        return open_key;
    }

    HKEY context{};
};
#endif

#include <rainy/utility.hpp>



#include <future>




constexpr int fooooo() {
    return 10 + 20;
}



#include <rainy/system/nebula_ptr.hpp>

class A {
public:
    virtual ~A() {
        std::puts("~A()\n");
    }

    int value = 0;

    virtual void hello() const = 0;
};

class B : public A {
public:
    ~B() {
        std::puts("~B()\n");
    }

    void hello() const {
        std::cout << "Hello World"
                  << "\n";
    }
};

#include <execution>

constexpr int test() {
    std::array<int, 5> a = {1, 2, 3, 4, 5};
    std::array<int, 5> a2{};
    //rainy::algorithm::container_operater::copy_n(rainy::algorithm::execution::par,a.begin(), a.size(), a2.begin());
    return 1;
}

//#include <rainy/functional.hpp>

#include <rainy/containers/vector.hpp>
#include <rainy/winapi/environment.hpp>

constexpr int test_fun() {
    /*int *p = rainy::foundation::system::memory::allocator<int>{}.allocate(1);
    rainy::utility::construct_at(p,10);
    int value = *p;
    rainy::utility::ensures(*p == 10);
    rainy::foundation::system::memory::allocator<int>{}.deallocate(p, 1);
    return value;*/
    return 0;
}

#include <rainy/containers/variant.hpp>

void run_visit_test() {
    using namespace rainy::containers;
    struct visitor {
        void operator()(int val) noexcept {
            std::cout << "int\n";
        }

        void operator()(char val) noexcept {
            std::cout << "char\n";
        }

        void operator()(std::string &val) noexcept {
            std::cout << "string\n";
        }
    };
    variant<std::string,int,char> variant1 = "Hello World";
    visit(visitor{}, variant1);
    variant<std::string, int, char> variant2 = 10;
    visit(visitor{}, variant2);
    variant<std::string, int, char> variant3 = 'c';
    visit(visitor{}, variant3);
    visit(visitor{}, variant1, variant2, variant3);

    variant<std::string, const char *, int> var(std::in_place_index<2>, 10);
}

struct demo_class {
    int x, y;
    int custom_field;

    int mult() {
        return x * y;
    }
};

struct derived_class : public demo_class {
};

struct foo {
    foo(int val) : val(val) {
    }

    // 这是前置递增
    int operator++() {
        val = val + 1;
        return val;
    }

    // 这是后置递增
    int operator++(int) {
        int old_val = val; // 保存了当前的值
        val = val + 1; // 然后我再自增
        return old_val; // 最后返回的是我保存的，而不是val
    }

    // 这是前置递减
    int operator--() {
        val = val - 1;
        return val;
    }

    // 这是后置递减
    int operator--(int) {
        int old_val = val; // 保存了当前的值
        val = val - 1; // 然后我再自增
        return old_val; // 最后返回的是我保存的，而不是val
    }

    int val;
};

#include <rainy/winapi/ui/window.hpp>

//class my_window : public rainy::winapi::ui::window<char> {
//public:
//    virtual LRESULT handle_message(UINT msg, WPARAM wparam, LPARAM lparam) override {
//        return DefWindowProcA(this->handle,msg,wparam,lparam);
//    }
//};

unsigned int my_function(void*) {
    std::cout << "Hello World"
              << "\n";
    return 0;
}

//#include <process.h>
#include <fstream>

class localization {
public:
    localization() = default;

    localization(std::string_view filename) {
        std::fstream fstream(filename.data(), std::ios::in);
        if (!fstream) {
            
        }
    }

private:
    std::unordered_map<std::string, std::string> data;
};

class user_event : public rainy::component::sync_event::event {
public:
    user_event() : msg(666) {
    }

    int message() const noexcept {
        return msg;
    }

    void set_message(int msg) noexcept {
        this->msg = msg;
    }

private:
    int msg;
};

class listener : public rainy::component::sync_event::event_handler<user_event> {
public:
    void on(rainy::component::sync_event::event &e) override {
    }
};

struct pod {
    int a{};
    char b{};
    std::string c;
    std::vector<int> d;
public:
    static constexpr auto __refl_struct_members() noexcept {
        return std::make_tuple(&pod::a, &pod::b, &pod::c, &pod::d);
    }
};

class secret {
    int m_i = 3;
    int m_f(int p) {
        return 14 * p;
    }
    int m_2 = 10;
    std::string m_3;
};

class secret2 {
    int m_i = 3;
    int m_f(int p) {
        return 14 * p;
    }
};

struct test_class {
    void noexcept_fun() noexcept {
        std::cout << "Hello test_class"
                  << "\n";
    }

    int a;
};

#include <set>
#include <rainy/utility/invoke.hpp>

int main() {
    test_class object_test{};
    std::reference_wrapper<test_class> my_test = object_test;
    rainy::utility::internals::select_invoker<decltype(&test_class::a), decltype(my_test)>::value;
    rainy::utility::invoke(&test_class::a, my_test);
    std::cout << "Hello World"
              << "\n";
    std::cout << "Hello World\n";
    std::random_device random_device;
    std::mt19937 gen(random_device());
    std::uniform_int_distribution<> dist(1, 27);
    std::set<int> my_set;
    while (my_set.size() < 10) {
        my_set.insert(dist(gen));
    }
    for (const auto &i: my_set) {
        std::cout << i << "\n";
    }
    std::uniform_int_distribution<> rc_dist(1, 11);
    std::cout << "random choice = " << rc_dist(gen) << "\n";
    //rainy::foundation::system::async::internals::init_async_moudle_abi();
    //auto thread_instance = rainy::foundation::comint::create_instance<rainy::foundation::system::async::internals::native_thread>();



    //thread_instance->start(nullptr, 0, my_function, nullptr, 0);
    //rainy::foundation::system::async::internals::abi::thread_sleep_for(1);
    //std::cout << thread_instance.use_count() << "\n";
    //std::cout << thread_instance->get_id() << "\n";
    //std::shared_ptr<rainy::foundation::comint::the_unknown> move_ptr;
    //thread_instance->move(move_ptr);
    //std::cout << thread_instance->get_id() << "\n";
    //std::cout << std::dynamic_pointer_cast<rainy::foundation::system::async::internals::native_thread>(move_ptr)->get_id() << "\n";
    //std::dynamic_pointer_cast<rainy::foundation::system::async::internals::native_thread>(move_ptr)->join();
    //std::cout << thread_instance.use_count() << "\n";
    auto reg = rainy::component::sync_event::dispatcher::instance()->subscribe<user_event, listener>();
    std::cout << reg->name() << "\n";
    (void) rainy::component::sync_event::dispatcher::instance()->subscribe<user_event>(
        [](rainy::component::sync_event::event &e) {});
    user_event e;
    std::cout << e.type_info().name() << "\n";
    for (int i = 0; i < 10; ++i) {
        benchmark("sync_event", [&e]() {
            for (int i = 0; i < 10000; ++i) {
                rainy::component::sync_event::dispatcher::instance()->publish(e);
            }
        });
    }

    rainy::component::sync_event::dispatcher::instance()->for_each_in_handlers<user_event>(
        +[](rainy::component::sync_event::event_handler<user_event> *handler) { 
            std::cout << handler->handler_name() << "\n";
        });
    rainy::component::sync_event::dispatcher::instance()->clear();
    std::tuple<int,char,std::string> t;
    using namespace rainy::type_traits::extras::reflection;
    //rainy::winapi::ui::window<> main_window;
    //RECT window_rect = {100, 100, 800, 600};
    //main_window.create(NULL, "Main Window", WS_OVERLAPPEDWINDOW, 0, window_rect);
    //main_window.show_window();
    //main_window.message_loop();
    system("pause");
    foo a = 2;
    foo b = ++a;
    std::cout << a.val << "\n";
    std::cout << b.val << "\n";

    auto *instance = rainy::foundation::reflection::lunar::instance();
    instance->register_class<demo_class>();
    instance->add_field<&demo_class::x>();
    instance->add_field<&demo_class::y>();
    instance->add_method<&demo_class::mult>();

    auto &x_field = instance->get_field<demo_class>(rainy::utility::variable_name<&demo_class::x>());
    auto &y_field = instance->get_field<demo_class>(rainy::utility::variable_name<&demo_class::y>());

    std::cout << x_field.name() << "\n";
    std::cout << y_field.name() << "\n";

    demo_class sample_object{};

    x_field.set(&sample_object, 10);
    y_field.set(&sample_object, 20); // 使用setter功能设置值

    std::cout << x_field.get(&sample_object).as<int>() << "\n"; // 通过获取any并通过类型还原获取x
    std::cout << sample_object.x << "\n";

    auto &mult_method = instance->get_method<demo_class>(rainy::utility::variable_name<&demo_class::mult>());
    std::cout << "mult_method name: " << mult_method.name() << "\n";
    std::cout << mult_method.invoke_with_paramspack(&sample_object).as<int>() << "\n";
    // 或者...
    std::cout << mult_method(&sample_object, {}).as<int>() << "\n";


    auto object = instance->make_shared_object<demo_class>();

    std::cout << object->name() << "\n";
    
    instance->register_class<derived_class>();
    instance->add_base<derived_class, demo_class>();
    instance->remove_base<derived_class, demo_class>();

    std::cout << "Is demo_class registered? " << std::boolalpha << instance->is_registered(typeid(demo_class)) << "\n";

    run_visit_test();
    rainy::containers::any any_object = 10;
    std::cout << rainy::utility::get<int&>(any_object) << "\n";
    rainy::utility::get<int &>(any_object) = 666;
    std::cout << rainy::utility::get<int &>(any_object) << "\n";


    /*rainy::winapi::environment::environment_manager manager;
    for (const auto &i: manager.get_environment_var_by_list("PATH").vector_list) {
        std::cout << i << "\n";
    }*/
    constexpr int v = test_fun();
    std::cout << v << "\n";

    rainy::foundation::system::memory::allocator<int,
                                                    rainy::foundation::system::memory::allocation_method::RAINY_allocator_no_check>
        allocator;

    int *get = allocator.allocate(10);
    std::tuple<int, char, int> tuple(1, 'a', 3);

    my_class o2;
    o2.another();

    o2.a = 666;

    auto o3 = o2.clone();

    std::cout << o3->a << "\n";

    o3->another();

    std::cout << std::get<2>(tuple) << "\n";
    rainy::foundation::system::memory::nebula_ptr<int[]> smart_ptr(get, 10);
    smart_ptr.fill_with_ilist({1, 2, 3, 4, 5, 6, 7, 8, 9, 10});
    for (const int &i: smart_ptr) {
        std::cout << i << ' ';
    }
    allocator.deallocate(smart_ptr.release(), 10);

    std::pmr::memory_resource* std_memres = new rainy::foundation::system::memory::pmr::std_memory_resource;
    smart_ptr = static_cast<int*>(std_memres->allocate(sizeof(int) * 10, alignof(int)));
    smart_ptr.fill_with_ilist({1, 2, 3, 4, 5, 6, 7, 8, 9, 10});
    for (const int &i: smart_ptr) {
        std::cout << i << ' ';
    }
    std_memres->deallocate(smart_ptr.release(),sizeof(int) * 10, alignof(int));

    constexpr int i = test();
    auto p = rainy::foundation::system::memory::make_nebula<int[]>(10, {1, 2, 3, 4, 5, 6, 7, 8, 9, 10});
    for (const int &i: p) {
        std::cout << i << ' ';
    }
    std::cout.put('\n');
    p.reset();
    int array[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    p = &array;
    rainy::foundation::system::output::stdout_print("p.length = ", p.length(), " \n");
    for (const int &i: p) {
        std::cout << i << ' ';
    }
    std::cout.put('\n');
    p.reset();
    rainy::foundation::system::output::stdout_print("p.length = ", p.length(), " \n");

    int array1[5] = {1, 2, 3, 4, 5};
    p = &array1;

    rainy::foundation::system::output::stdout_print("p.length = ", p.length(), " \n");
    for (const int &i: p) {
        std::cout << i << ' ';
    }
    std::cout.put('\n');

    rainy::foundation::system::output::stdout_print("p.length = ", p.length(), " \n");

    return 0;
}
