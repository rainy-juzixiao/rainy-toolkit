#ifndef RAINY_META_REFLECTION_REGISTRATION_HPP
#define RAINY_META_REFLECTION_REGISTRATION_HPP
#include <rainy/meta/meta_method.hpp>
#include <rainy/meta/reflection/moon/reflect.hpp>
#include <rainy/meta/reflection/type.hpp>
#include <rainy/foundation/pal/threading.hpp>
#include <rainy/meta/reflection/metadata.hpp>
#include <string_view>

namespace rainy::meta::reflection::implements {
    class register_table {
    public:
        template <typename Type>
        static void register_type(std::string_view name, type_accessor *type) {
            auto *this_ = &instance();
            constexpr auto ctti = foundation::ctti::typeinfo::create<Type>();
            // 从此处开始，进入同步块
            foundation::pal::threading::create_synchronized_task(this_->lock, [this_,&ctti,&type,name]() {
#if RAINY_HAS_CXX20
                if (!this_->data.contains(ctti))
#else
                if (this_->data.find(ctti) == this_->data.end())
#endif
                {
                    auto [iter, success] = this_->data.emplace(ctti, type);
                    utility::ensures(success, "Cannot register type.");
                    this_->index.emplace(name, ctti);
                    (void) iter;
                }
            });
            // 同步结束
        }

        template <typename Type>
        static type_accessor *get_accessor() {
            auto *this_ = &instance();
            if (auto iter = this_->data.find(rainy_typeid(Type)); iter != this_->data.end()) {
                return iter->second;
            }
            return nullptr;
        }

        RAINY_TOOLKIT_API static void unregister(std::string_view name, foundation::ctti::typeinfo ctti);

        static type_accessor *get_accessor_by_name(std::string_view name);
    private:
        RAINY_TOOLKIT_API static register_table &instance();

        std::unordered_map<std::string_view, foundation::ctti::typeinfo> index;
        std::unordered_map<foundation::ctti::typeinfo, type_accessor *> data;
        std::mutex lock;
    };
}

namespace rainy::meta::reflection::implements {
    class injector {
    public:
        template <typename Type>
        static void register_type(std::string_view name, type_accessor *type) {
            register_table::register_type<Type>(name, type);
            auto *this_ = &instance();
            constexpr auto ctti = foundation::ctti::typeinfo::create<Type>();
            std::lock_guard<std::mutex> guard(this_->lock);
            this_->registered.emplace_back(registration_entry{name, ctti});
        }

        static void unregister_all() {
            auto *this_ = &instance();
            std::lock_guard<std::mutex> guard(this_->lock);
            for (const auto &entry: this_->registered) {
                register_table::unregister(entry.name, entry.ctti);
            }
            this_->registered.clear();
        }

    private:
        struct registration_entry {
            std::string_view name;
            foundation::ctti::typeinfo ctti;
        };

        static injector &instance() {
            static injector inst;
            return inst;
        }

        std::mutex lock;
        std::vector<registration_entry> registered;
    };
}

namespace rainy::meta::reflection {
    template <typename Any,
              type_traits::other_trans::enable_if_t<
                  type_traits::type_relations::is_same_v<type_traits::cv_modify::remove_cvref_t<Any>, utility::any>, int> = 0>
    RAINY_INLINE object_view as_object_view(Any&& any) {
        return object_view{const_cast<void*>(any.target_as_void_ptr()), any.type()};
    }

    class RAINY_TOOLKIT_API type {
    public:
        using type_id = std::size_t;

        template <typename Type>
        static type get() noexcept {
            type instance{};
            instance.accessor = implements::register_table::get_accessor<Type>();
            return instance;
        }

        static type get_by_name(std::string_view name) noexcept;

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

        RAINY_NODISCARD collections::views::array_view<foundation::ctti::typeinfo> get_template_arguments() const noexcept {
            if (!accessor) {
                return {};
            }
            return accessor->type().template_arguemnts();
        }

        RAINY_NODISCARD const method &get_method(const std::string_view name) const noexcept;

        RAINY_NODISCARD const method &get_method(
            const std::string_view name, const collections::views::array_view<foundation::ctti::typeinfo> overload_version_paramlist,
            const method_flags filter_item = method_flags::none) const noexcept;

        RAINY_NODISCARD auto get_methods() const {
            static implements::method_storage_t empty;
            if (!accessor) {
                return utility::mapped_range(empty);
            }
            return utility::mapped_range(accessor->methods());
        }

        RAINY_NODISCARD const property &get_property(const std::string_view name) const noexcept {
            static const property empty;
            if (!accessor) {
                return empty;
            }
            const auto &cont = accessor->properties();
            const auto iter = cont.find(name);
            return iter != cont.end() ? iter->second : empty;
        }

        RAINY_NODISCARD auto get_properties() const noexcept {
            static implements::property_storage_t empty;
            if (!accessor) {
                return utility::mapped_range(empty);
            }
            return utility::mapped_range(accessor->properties());
        }

        RAINY_NODISCARD auto get_ctors() const noexcept {
            static implements::ctor_storage_t empty;
            if (!accessor) {
                return utility::mapped_range(empty);
            }
            return utility::mapped_range(accessor->ctors());
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
                    return cur_ctor.static_invoke(utility::forward<Args>(args)...);
                }
            }
            return {};
        }

        RAINY_NODISCARD bool is_valid() const noexcept {
            return static_cast<bool>(accessor);
        }

        template <typename... Args>
        utility::any invoke_method(std::string_view name, object_view instance, Args &&...args) const {
            using namespace foundation::ctti;

            auto flag = method_flags::none;
            if (instance.ctti().has_traits(traits::is_const)) {
                flag = flag | method_flags::const_qualified;
            }
            if (instance.ctti().has_traits(traits::is_volatile)) {
                flag = flag | method_flags::volatile_qualified;
            }
            if (instance.ctti().has_traits(traits::is_rref)) {
                flag = flag | method_flags::rvalue_qualified;
            }
            return invoke_method(flag, name, instance, utility::forward<Args>(args)...);
        }

        template <typename... Args>
        utility::any invoke_method(method_flags flag, std::string_view name, object_view instance, Args &&...args) const {
            using namespace type_traits::other_trans;
            using namespace type_traits::type_relations;
            using namespace foundation::ctti;
            const method *invoker{nullptr};
            if constexpr (is_any_of_v<utility::any, decay_t<Args>...> || is_any_of_v<object_view, decay_t<Args>...>) {
                implements::make_paramlist paramlist{utility::forward<Args>(args)...};
                invoker = &get_method(name, paramlist, flag);
            } else {
                static collections::array<foundation::ctti::typeinfo, sizeof...(Args)> paramlist = {
                    foundation::ctti::typeinfo::create<Args>()...};
                invoker = &get_method(name, paramlist, flag);
            }
            if (invoker->empty()) {
                errno = EINVAL;
                return {};
            }
            if (invoker->is_static()) {
                return invoker->static_invoke(utility::forward<Args>(args)...);
            }
            return invoker->invoke(instance, utility::forward<Args>(args)...);
        }

    private:
        implements::type_accessor *accessor{nullptr};
    };
}

namespace rainy::meta::reflection::implements {
    RAINY_TOOLKIT_API bool check_method_field(type_accessor *type, std::string_view name, method &meth);
    RAINY_TOOLKIT_API void register_method_helper(type_accessor *type, std::string_view name, method &&meth);
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

            explicit class_(const std::string_view name) : type(instance(name)) {
            }

            explicit class_(core::internal_construct_tag_t, type_accessor *type) : type(type) {
            }

            class_ &reflect_moon() {
                using namespace moon::implements;
                using reflect = moon::reflect<Type>;
                if constexpr (has_ctors<Type>) {
                    //reflect::ctors.for_each([this](auto pack) {
                    //    /*std::unordered_map<std::string_view, utility::any> metadata;
                    //    if constexpr (!type_traits::type_relations::is_same_v<decltype(pack.metas), moon::meta_list<>>) {
                    //        pack.metas.for_each([&metadata](auto meta) {
                    //            metadata.emplace(meta.name, type_traits::other_trans::decay_t<decltype(meta.value)>{meta.value});
                    //        });
                    //    }
                    //    this->constructor(pack.value, utility::move(metadata));*/
                    //});
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

            template <typename Fx,
                      type_traits::other_trans::enable_if_t<type_traits::type_properties::is_constructible_v<function, Fx>, int> = 0>
            bind<reflection::method, Type, Fx> method(std::string_view name, Fx &&fn) {
                return bind<reflection::method, Type, Fx>{this->type, name, utility::forward<Fx>(fn)};
            }

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

            template <typename... Args, type_traits::other_trans::enable_if_t<type_traits::type_properties::is_constructible_v<Type,Args...>,int> = 0>
            class_ &constructor(std::unordered_map<std::string_view, utility::any> metadata = {}) {
                static constexpr auto ctor_name = implements::make_ctor_name<Type, Args...>();
                // reflection::method ctor{"ctor", rainy::meta::method::get_ctor_fn<Type, Args...>()};
                // if (implements::check_method_field(type, "ctor", ctor)) {
                //     type->ctors().emplace(std::string_view{"ctor"}, utility::move(ctor));
                // }
                return *this;
            }

            template <typename Fx, type_traits::other_trans::enable_if_t<type_traits::type_properties::is_constructible_v<reflection::function,Fx> && !type_traits::primary_types::is_member_function_pointer_v<Fx>,int> = 0>
            class_ &constructor(Fx &&fn) {
                // reflection::method ctor{"ctor", utility::forward<Fx>(fn)};
                // utility::expects(ctor.is_static(), "Cannot use this constructor fn");
                // if (implements::check_method_field(type, "ctor", ctor)) {
                //     type->ctors().emplace(std::string_view{"ctor"}, utility::move(ctor));
                // }
                return *this;
            }

        private:
            static type_accessor *instance(std::string_view name) noexcept {
                static std::once_flag flag;
                static implements::type_accessor_impl_class<Type> instance{name};
                std::call_once(flag, [&]() { implements::injector::register_type<Type>(name, &instance); });
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

namespace rainy::meta::reflection::implements {
    template <typename Ty>
    using registration_derived_t = type_traits::other_trans::conditional_t<type_traits::type_relations::is_void_v<Ty>, registration, registration::class_<Ty>>;

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

        using type = std::conditional_t<current_is_tuple, std::decay_t<T>, typename next::type>;
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
        return {utility::forward<Args>(args)...};
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
                    std::tuple<> a;
                    meth_ = method::make(name, utility::move(fn), a, empty);
                }
                type_accessor->methods().emplace(name, utility::move(meth_));
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
}

#endif