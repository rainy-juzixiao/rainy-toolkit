#ifndef RAINY_META_REFLECTION_REGISTRATION_HPP
#define RAINY_META_REFLECTION_REGISTRATION_HPP
#include <rainy/meta/meta_method.hpp>
#include <rainy/meta/reflection/moon/reflect.hpp>
#include <rainy/meta/reflection/type.hpp>
#include <string_view>

namespace rainy::meta::reflection::implements {
    class register_table {
    public:
        template <typename Type>
        static void register_type(std::string_view name, type_accessor *type) {
            auto *this_ = &instance();
            constexpr auto rtti = foundation::rtti::typeinfo::create<Type>();
#if RAINY_HAS_CXX20
            if (!this_->data.contains(rtti))
#else
            if (this_->data.find(rtti) == this_->data.end())
#endif
            {
                auto [iter, success] = this_->data.emplace(rtti, type);
                utility::ensures(success, "Cannot register type.");
                this_->index.emplace(name, rtti);
                (void) iter;
            }
        }

        template <typename Type>
        static type_accessor *get_accessor() {
            auto *this_ = &instance();
            if (auto iter = this_->data.find(rainy_typeid(Type)); iter != this_->data.end()) {
                return iter->second;
            }
            return nullptr;
        }

        static type_accessor *get_accessor_by_name(std::string_view name);
    private:
        RAINY_TOOLKIT_API static register_table &instance();

        std::unordered_map<std::string_view, foundation::rtti::typeinfo> index;
        std::unordered_map<foundation::rtti::typeinfo, type_accessor *> data;
    };
}

namespace rainy::meta::reflection::implements {
    struct shared_object_accessor {
        virtual ~shared_object_accessor() = default;
        RAINY_NODISCARD virtual const foundation::rtti::typeinfo& type() const noexcept = 0;
        virtual void* clone(void* soo_buffer) const noexcept = 0;
        virtual void *target() noexcept = 0;
        
    };
}

namespace rainy::meta::reflection {
    class type {
    public:
        using type_id = std::size_t;

        template <typename Type>
        static type get() noexcept {
            type instance{};
            instance.accessor = implements::register_table::get_accessor<Type>();
            return instance;
        }

        RAINY_TOOLKIT_API static type get_by_name(std::string_view name) noexcept;

        RAINY_NODISCARD std::string_view get_name() const noexcept {
            if (!accessor) {
                return {};
            }
            return accessor->name();
        }

        RAINY_NODISCARD type_id get_id() const noexcept {
            if (!accessor) {
                return 0;
            }
            return accessor->type().typeinfo().hash_code();
        }

        RAINY_NODISCARD std::size_t get_sizeof() const noexcept {
            if (!accessor) {
                return 0;
            }
            return accessor->type().get_sizeof();
        }

        RAINY_NODISCARD collections::views::array_view<foundation::rtti::typeinfo> get_template_arguments() const noexcept {
            if (!accessor) {
                return {};
            }
            return accessor->type().template_arguemnts();
        }

        RAINY_NODISCARD const method& get_method(const std::string_view name) const noexcept {
            static const method empty;
            if (!accessor) {
                return empty;
            }
            const auto &cont = accessor->methods();
            const auto iter = cont.find(name);
            return iter != cont.end() ? iter->second : empty;
        }

        RAINY_NODISCARD const method &get_method(const std::string_view name, const collections::views::array_view<foundation::rtti::typeinfo> overload_version_paramlist,
                                                 const method_flags filter_item = method_flags::none) const noexcept {
            static const method empty;
            if (!accessor) {
                return empty;
            }
            const auto [fst, snd] = accessor->methods().equal_range(name);
            if (fst == snd) {
                errno = EACCES;
                return empty;
            }
            auto match_method_type = [](method_flags candidate, method_flags filter) -> bool {
                if (filter == method_flags::none) {
                    return true;
                }
                candidate &= ~(method_flags::noexcept_specified);
                filter &= ~(method_flags::noexcept_specified);
                return candidate == filter;
            };
            for (auto iter = fst; iter != snd; ++iter) {
                if (const auto &method = iter->second; method.type() == filter_item) {
                    if (method.is_invocable(overload_version_paramlist)) {
                        return method;
                    }
                }
            }
            for (auto iter = fst; iter != snd; ++iter) {
                if (const auto &method = iter->second;
                    method.is_invocable(overload_version_paramlist) && match_method_type(method.type(), filter_item)) {
                    return method;
                }
            }
            if (filter_item != method_flags::none) {
                for (auto iter = fst; iter != snd; ++iter) {
                    if (const auto &method = iter->second; method.is_invocable(overload_version_paramlist)) {
                        return method;
                    }
                }
            }
            errno = EACCES;
            return empty;
        }

        RAINY_NODISCARD auto get_methods() const {
            static implements::method_storage_t empty;
            if (!accessor) {
                return utility::mapped_range(empty);
            }
            return utility::mapped_range(accessor->methods());
        }

        RAINY_NODISCARD const property& get_property(const std::string_view name) const noexcept {
            static const property empty;
            if (!accessor){
                return empty;
            }
            const auto& cont = accessor->properties();
            const auto iter = cont.find(name);
            return iter != cont.end() ? iter->second : empty;
        }

        RAINY_NODISCARD auto get_properties() const noexcept {
            static implements::property_storage_t empty;
            if (!accessor){
                return utility::mapped_range(empty);
            }
            return utility::mapped_range(accessor->properties());
        }

        template <typename... Args>
        utility::any create(Args &&...args) const {
            for (const auto &item: utility::mapped_range(accessor->ctors())) {
                const function &cur_ctor = item;
                bool invocable{};
                if constexpr (type_traits::type_relations::is_any_of_v<utility::any, type_traits::other_trans::decay_t<Args>...> ||
                              type_traits::type_relations::is_any_of_v<object_view, type_traits::other_trans::decay_t<Args>...>) {
                    invocable = cur_ctor.is_invocable_with(utility::forward<Args>(args)...);
                } else {
                    static implements::make_nondynamic_paramlist<Args...> paramlist;
                    invocable = cur_ctor.is_invocable(paramlist.get());
                }
                if (invocable) {
                    return cur_ctor.invoke_static(utility::forward<Args>(args)...);
                }
            }
            return {};
        }

        RAINY_NODISCARD bool is_valid() const noexcept {
            return static_cast<bool>(accessor);
        }

        template <typename... Args>
        utility::any invoke_method(std::string_view name, object_view instance, Args &&...args) const {
            using namespace foundation::rtti;
            auto flag = method_flags::none;
            if (instance.rtti().has_traits(traits::is_const)) {
                flag = flag | method_flags::const_qualified;
            }
            if (instance.rtti().has_traits(traits::is_volatile)) {
                flag = flag | method_flags::volatile_qualified;
            }
            if (instance.rtti().has_traits(traits::is_rref)) {
                flag = flag | method_flags::rvalue_qualified;
            }
            return invoke_method(flag, name, instance, utility::forward<Args>(args)...);
        }

        template <typename... Args>
        utility::any invoke_method(method_flags flag, std::string_view name, object_view instance, Args &&...args) const {
            using namespace type_traits::other_trans;
            using namespace type_traits::type_relations;
            using namespace foundation::rtti;
            const method *invoker{nullptr};
            if constexpr (is_any_of_v<utility::any, decay_t<Args>...> || is_any_of_v<object_view, decay_t<Args>...>) {
                implements::make_paramlist paramlist{utility::forward<Args>(args)...};
                invoker = &get_method(name, paramlist, flag);
            } else {
                static collections::array<foundation::rtti::typeinfo, sizeof...(Args)> paramlist = {
                    foundation::rtti::typeinfo::create<Args>()...};
                invoker = &get_method(name, paramlist, flag);
            }
            if (invoker->empty()) {
                errno = EINVAL;
                return {};
            }
            if (invoker->is_static()) {
                return invoker->invoke_static(utility::forward<Args>(args)...);
            }
            return invoker->invoke(instance, utility::forward<Args>(args)...);
        }

    private:
        implements::type_accessor *accessor{nullptr};
    };

    class registration {
    public:
        template <typename Type>
        class class_ {
        public:
            using type_accessor = implements::type_accessor;

            explicit class_(const std::string_view name) : type(instance(name)) {
            }

            class_ &reflect_moon() {
                moon::reflect<Type>::methods.for_each([this](auto pack) {
                    std::unordered_map<utility::any, utility::any> metadata;
                    if constexpr (!type_traits::type_relations::is_same_v<decltype(pack.metas), moon::meta_list<>>) {
                        pack.metas.for_each([&metadata](auto meta) {
                            metadata.emplace(meta.name, type_traits::other_trans::decay_t<decltype(meta.value)>{meta.value});
                        });
                    }
                    this->method(pack.name, pack.value, utility::move(metadata));
                });
                return *this;
            }

            template <typename Fx,
                      type_traits::other_trans::enable_if_t<
                          type_traits::type_properties::is_constructible_v<function,Fx>, int> = 0>
            class_ &method(std::string_view name, Fx &&fn, std::unordered_map<std::string_view, utility::any> metadata = {}) {
                reflection::method meth{name, utility::forward<Fx>(fn), utility::move(metadata)};
#if RAINY_HAS_CXX20
                if (type->methods().contains(name))
#else
                if (type->methods().find(name) != type->methods().end())
#endif
                    {
                    auto [fst, snd] = type->methods().equal_range(name);
                    for (auto &it = fst; it != snd; ++it) {
                        const auto &existing_params = it->second.paramlists();
                        const auto &wiat_for_emplace_params = meth.paramlists();
                        if (existing_params.size() != wiat_for_emplace_params.size()) {
                            continue;
                        }
                        const bool same = core::algorithm::all_of(
                            wiat_for_emplace_params.begin(), wiat_for_emplace_params.end(),
                            [&, i = std::size_t{0}](const auto &param) mutable { return param == existing_params[static_cast<std::ptrdiff_t
                                >(i++)]; });
                        if (same && it->second.function_signature() == meth.function_signature()) {
                            return *this;
                        }
                    }
                }
                type->methods().emplace(name, utility::move(meth));
                return *this;
            }

            template <typename Property,
                      type_traits::other_trans::enable_if_t<type_traits::primary_types::is_pointer_v<Property> ||
                                                                (type_traits::primary_types::is_member_object_pointer_v<Property>),
                                                            int> = 0>
            class_ &property(std::string_view name, Property &&property,
                             std::unordered_map<std::string_view, utility::any> metadata = {}) {
#if RAINY_HAS_CXX20
                if (type->ctors().contains(name)) {
                    return *this;
                }
#else
                if (type->ctors().find(name) != type->ctors().end()) {
                    return *this;
                }
#endif
                type->properties().emplace(name, property);
                if (!metadata.empty()) {
                    type->metadatas().emplace(name, utility::move(metadata));
                }
                return *this;
            }

            template <typename... Args, type_traits::other_trans::enable_if_t<type_traits::type_properties::is_constructible_v<Type,Args...>,int> = 0>
            class_ &constructor(std::unordered_map<std::string_view, utility::any> metadata = {}) {
                static constexpr auto ctor_name = implements::make_ctor_name<Type, Args...>();
                std::string_view name{ctor_name.data(), ctor_name.size()};
#if RAINY_HAS_CXX20
                if (type->ctors().contains(name)) {
                    return *this;
                }
#else
                if (type->ctors().find(name) != type->ctors().end()) {
                    return *this;
                }
#endif
                type->ctors().emplace(name, meta::method::get_ctor_fn<Type, Args...>());
                if (!metadata.empty()) {
                    type->metadatas().emplace(name, utility::move(metadata));
                }
                return *this;
            }

            template <typename Fx, type_traits::other_trans::enable_if_t<type_traits::type_properties::is_constructible_v<reflection::function,Fx> && !type_traits::primary_types::is_member_function_pointer_v<Fx>,int> = 0>
            class_ &constructor(Fx &&fn, const std::string_view ctor_name = {},
                                std::unordered_map<std::string_view, utility::any> metadata = {}) {
                function ctor{utility::forward<Fx>(fn)};
                utility::expects(ctor.is_static(), "Ctor must be static");
                std::string_view name = ctor_name.empty() ? ctor.function_signature().name() : ctor_name;
#if RAINY_HAS_CXX20
                if (type->ctors().contains(name)) {
                    return *this;
                }
#else
                if (type->ctors().find(name) != type->ctors().end()) {
                    return *this;
                }
#endif
                type->ctors().emplace(name, ctor);
                if (!metadata.empty()) {
                    type->metadatas().emplace(name, utility::move(metadata));
                }
                return *this;
            }

        private:
            static type_accessor *instance(std::string_view name) noexcept {
                static std::once_flag flag;
                static implements::type_accessor_impl_class<Type> instance{name};
                std::call_once(flag, [&]() { implements::register_table::register_type<Type>(name, &instance); });
                return &instance;
            }

            type_accessor *type;
        };

    private:
    };
}

#define RAINY_REFLECTION_REGISTRATION                                                                                                             \
    static void RAINY_CAT(RAINY_CAT(rainytoolkit_auto_register_reflection_, __LINE__),_function_());                                                                            \
    namespace rainy::meta::implements::auto_regsiter {                                                                                                                       \
        struct RAINY_CAT(rainytoolkit_reflection_auto_register,__LINE__) {                                                                                               \
            RAINY_CAT(rainytoolkit_reflection_auto_register, __LINE__)() {                                                                                                \
                RAINY_CAT(RAINY_CAT(rainytoolkit_auto_register_reflection_, __LINE__),_function_());                                                                            \
            }                                                                                                                         \
        };                                                                                                                            \
    }                                                                                                                                 \
    static const RAINY_CAT(rainy::meta::implements::auto_regsiter::rainytoolkit_reflection_auto_register, __LINE__)                                                           \
        RAINY_CAT(rainy_toolkit_reflect_auto_register_, __LINE__);                                                          \
    static void RAINY_CAT(RAINY_CAT(rainytoolkit_auto_register_reflection_, __LINE__),_function_())



RAINY_REFLECTION_REGISTRATION {
    using std::string;
    using namespace rainy;
    using namespace rainy::meta;
    using namespace rainy::meta::method::cpp_methods;
    reflection::registration::class_<string>("std::string")
        .constructor<const char *>()
        .constructor<const char *, std::size_t>()
        .constructor<const std::string &>()
        .constructor<std::string &&>()
        .constructor([](const std::string_view str) {
            return std::string{str.data(),str.size()};
        })
        .method(method_append, utility::get_overloaded_memfn<string, string &(const char *)>(&string::append))
        .method(method_size, &string::size)
        .method(method_operator_assign, utility::get_overloaded_memfn<string, string &(char)>(&string::operator=))
        .method(method_operator_assign, utility::get_overloaded_memfn<string, string &(const char *)>(&string::operator=))
        .method(method_operator_assign, utility::get_overloaded_memfn<string, string &(char)>(&string::operator=))
        .method(method_at, utility::get_overloaded_memfn<string, const char &(std::size_t) const>(&string::at))
        .method(method_at, utility::get_overloaded_memfn<string, char &(std::size_t)>(&string::at))
        .method(method_operator_index, utility::get_overloaded_memfn<string, char &(std::size_t)>(&string::operator[]))
        .method(method_swap, utility::get_overloaded_memfn<string, void(string &)>(&string::swap))
        .method(method_clear, &string::clear)
        .method(method_begin, utility::get_overloaded_memfn<string,string::iterator()>(&string::begin))
        .method(method_end, utility::get_overloaded_memfn<string,string::iterator()>(&string::end))
        .method(method_begin, utility::get_overloaded_memfn<string,string::const_iterator() const>(&string::begin))
        .method(method_end, utility::get_overloaded_memfn<string,string::const_iterator() const>(&string::end))
        .method(method_rbegin,utility::get_overloaded_memfn<string,string::reverse_iterator()>(&string::rbegin))
        .method(method_rend,utility::get_overloaded_memfn<string,string::reverse_iterator()>(&string::rend))
        .method(method_rbegin,utility::get_overloaded_memfn<string,string::const_reverse_iterator() const>(&string::rbegin))
        .method(method_rend,utility::get_overloaded_memfn<string,string::const_reverse_iterator() const>(&string::rend))
        .method(method_push_back, &string::push_back)
        .method(method_pop_back, &string::pop_back)
        .method(method_length, &string::length)
        .method(method_erase, utility::get_overloaded_memfn<string,string&(string::size_type, string::size_type)>(&string::erase))
        .method("substr", &string::substr)
        .property("npos", &string::npos);
}

#endif