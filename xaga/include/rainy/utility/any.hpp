#ifndef RAINY_UTILITY_ANY_HPP
#define RAINY_UTILITY_ANY_HPP
#include <rainy/core/core.hpp>
#include <rainy/foundation/diagnostics/contract.hpp>
#include <rainy/foundation/functional/function_pointer.hpp>
#include <rainy/foundation/typeinfo.hpp>
#include <utility>

#if RAINY_USING_MSVC
#pragma warning(push)
#pragma warning(disable : 4715 4702)
#endif

namespace rainy::utility {
    class any;

    template <typename TargetType, typename = void>
    struct any_converter {
        static constexpr bool convertible = false;

        static bool is_convertible(const foundation::rtti::typeinfo &) {
            return false;
        }
    };
}

/*
 * any的基础utils和异常实现
 */
namespace rainy::utility::implements {
    inline constexpr size_t any_trivial_space_size = (core::small_object_num_ptrs - 1) * sizeof(void *);

    template <typename Ty>
    inline constexpr bool any_is_trivial =
        alignof(Ty) <= alignof(max_align_t) && type_traits::type_properties::is_trivially_copyable_v<Ty> &&
        sizeof(Ty) <= any_trivial_space_size;

    inline constexpr size_t any_small_space_size = (core::small_object_num_ptrs - 2) * sizeof(void *);

    template <typename Ty>
    inline constexpr bool any_is_small =
        alignof(Ty) <= alignof(max_align_t) && type_traits::type_properties::is_nothrow_move_constructible_v<Ty> &&
        sizeof(Ty) <= any_small_space_size;

    enum class any_representation : uintptr_t {
        trivial,
        big,
        _small,
        reference
    };

    struct big_any_rtti_manager {
        using destory_fn = void(void *target) noexcept;
        using copy_fn = void *(const void *source);
        using size_fn = std::size_t() noexcept;

        template <typename Ty>
        static void destory(void *const target) noexcept {
            ::delete static_cast<Ty *>(target);
        }

        template <typename Ty>
        RAINY_NODISCARD static void *copy(const void *const source) {
            return ::new Ty(*static_cast<const Ty *>(source));
        }

        destory_fn *destory_;
        copy_fn *copy_;
    };

    struct small_any_rtti_manager {
        using destroy_fn = void(void *target) noexcept;
        using copy_fn = void(void *target, const void *source);
        using move_fn = void(void *target, void *source) noexcept;

        template <typename Ty>
        static void destroy(void *const target) noexcept {
            std::destroy_at(static_cast<Ty *>(target));
        }

        template <typename Ty>
        static void copy(void *const target, const void *const source) {
            ::new (static_cast<Ty *>(target)) Ty(*static_cast<const Ty *>(source));
        }

        template <typename Ty>
        static void move(void *const target, void *const source) noexcept {
            ::new (static_cast<Ty *>(target)) Ty(utility::move(*static_cast<const Ty *>(source)));
        }

        destroy_fn *destroy_;
        copy_fn *copy_;
        move_fn *move_;
    };

    template <typename Ty>
    inline constexpr big_any_rtti_manager any_big_rtti_manager_object = {&big_any_rtti_manager::destory<Ty>,
                                                                         &big_any_rtti_manager::copy<Ty>};

    template <typename Ty>
    inline constexpr small_any_rtti_manager any_small_rtti_manager_object = {
        &small_any_rtti_manager::destroy<Ty>, &small_any_rtti_manager::copy<Ty>, &small_any_rtti_manager::move<Ty>};

    struct any_operater_policy {
        enum operation {
            compare_less,
            compare_equal,
            compare_less_equal,
            eval_hash
        };

        using invoke_fn = bool(operation op, void * data) noexcept;

        template <typename Ty>
        static bool invoke(operation op, void * data);
        
        invoke_fn* fn;
    };

    template <typename Ty>
    inline const any_operater_policy any_operater_policy_object = {
        +[](const any_operater_policy::operation op, void * const data) noexcept -> bool { return any_operater_policy::invoke<Ty>(op, data); }
    };
}

namespace rainy::foundation::exceptions::cast {
    class bad_any_cast final : public bad_cast {
    public:
        using base = bad_cast;

        explicit bad_any_cast(const source &location = source::current()) : base(location.to_string() + " : Bad any_cast : ") {
        }
    };

    RAINY_INLINE void throw_bad_any_cast() {
        utility::throw_exception(bad_any_cast{});
    }
}

namespace rainy::utility::implements {
    class any_ostream_converter_register {
    public:
        RAINY_DECLARE_SIGNLE_INSTANCE(any_ostream_converter_register);

        void call_converter(std::ostream &ostream, const foundation::rtti::typeinfo &rtti, const void *res) const {
            const auto find = converters.find(rtti);
            if (find != converters.end()) {
                try {
                    find->second(ostream, rtti, res);
                } catch (std::exception &e) {
                    throw e;
                }
            }
        }

    private:
        std::unordered_map<foundation::rtti::typeinfo, foundation::functional::function_pointer<void (*)(
                                                           std::ostream &, const foundation::rtti::typeinfo &rtti, const void *res)>>
            converters;
    };
}

/*
 * any实现
 */
namespace rainy::utility {
    class any {
    public:
        constexpr any() noexcept {
        }

        any(const any &right) {
            using namespace implements;
            storage.type_data = right.storage.type_data;
            storage.policy = right.storage.policy;
            switch (get_representation()) {
                case any_representation::_small:
                    storage.small_storage.rtti_manager = right.storage.small_storage.rtti_manager;
                    storage.small_storage.rtti_manager->copy_(&storage.small_storage.data, &right.storage.small_storage.data);
                    break;
                case any_representation::big:
                    storage.big_storage.rtti_manager = right.storage.big_storage.rtti_manager;
                    storage.big_storage.ptr = right.storage.big_storage.rtti_manager->copy_(right.storage.big_storage.ptr);
                    break;
                case any_representation::reference:
                    storage.reference_storage.ptr = right.storage.reference_storage.ptr;
                    break;
                case any_representation::trivial:
                default:
                    std::memcpy(storage.trivial_data, right.storage.trivial_data, sizeof(storage.trivial_data));
                    break;
            }
        }

        any(any &&right) noexcept {
            move_from(right);
        }

        template <typename ValueType,
                  type_traits::other_trans::enable_if_t<
                      type_traits::logical_traits::conjunction_v<
                          type_traits::logical_traits::negation<
                              type_traits::type_relations::is_same<type_traits::other_trans::decay_t<ValueType>, any>>,
                          type_traits::logical_traits::negation<type_traits::primary_types::is_specialization<
                              type_traits::other_trans::decay_t<ValueType>, std::in_place_type_t>>,
                          type_traits::type_properties::is_copy_constructible<type_traits::other_trans::decay_t<ValueType>>>,
                      int> = 0>
        any(ValueType &&value) {
            emplace_<ValueType>(utility::forward<ValueType>(value));
        }

        template <typename ValueType, typename... Types,
                  type_traits::other_trans::enable_if_t<
                      type_traits::logical_traits::conjunction_v<
                          type_traits::type_properties::is_constructible<type_traits::other_trans::decay_t<ValueType>, Types...>,
            type_traits::type_properties::is_copy_constructible<type_traits::other_trans::decay_t<ValueType>>>,
                      int> = 0>
        any(std::in_place_type_t<ValueType>, Types &&...args) {
            emplace_<ValueType>(utility::forward<Types>(args)...);
        }

        template <typename ValueType, typename Elem, typename... Types,
                  type_traits::other_trans::enable_if_t<
                      type_traits::logical_traits::conjunction_v<
                          type_traits::type_properties::is_constructible<type_traits::other_trans::decay_t<ValueType>,
                                                                         std::initializer_list<Elem> &, Types...>,
                          type_traits::type_properties::is_copy_constructible<type_traits::other_trans::decay_t<ValueType>>>,
                      int> = 0>
        explicit any(std::in_place_type_t<ValueType>, std::initializer_list<Elem> ilist, Types &&...args) {
            emplace_<ValueType>(ilist, utility::forward<Types>(args)...);
        }

        ~any() noexcept {
            reset();
        }

        any &operator=(const any &right) {
            assign_(right);
            return *this;
        }

        any &operator=(any &&right) noexcept {
            assign_(utility::move(right));
            return *this;
        }

        template <typename ValueType,
                  type_traits::other_trans::enable_if_t<
                      type_traits::logical_traits::conjunction_v<
                          type_traits::logical_traits::negation<
                              type_traits::type_relations::is_same<type_traits::other_trans::decay_t<ValueType>, any>>,
                          type_traits::type_properties::is_copy_constructible<type_traits::other_trans::decay_t<ValueType>>>,
                      int> = 0>
        any &operator=(ValueType &&value) {
            assign_(utility::forward<ValueType>(value));
            return *this;
        }

        template <typename ValueType, typename... Types,
                  type_traits::other_trans::enable_if_t<
                      type_traits::logical_traits::conjunction_v<
                          type_traits::type_properties::is_constructible<type_traits::other_trans::decay_t<ValueType>, Types...>,
                          type_traits::type_properties::is_copy_constructible<type_traits::other_trans::decay_t<ValueType>>>,
                      int> = 0>
        type_traits::other_trans::decay_t<ValueType> &emplace(Types &&...args) {
            reset();
            return emplace_<type_traits::other_trans::decay_t<ValueType>>(utility::forward<Types>(args)...);
        }

        template <typename ValueType, typename Elem, typename... Types,
                  type_traits::other_trans::enable_if_t<
                      type_traits::logical_traits::conjunction_v<
                          type_traits::type_properties::is_constructible<type_traits::other_trans::decay_t<ValueType>,
                                                                         std::initializer_list<Elem> &, Types...>,
                          type_traits::type_properties::is_copy_constructible<type_traits::other_trans::decay_t<ValueType>>>,
                      int> = 0>
        type_traits::other_trans::decay_t<ValueType> &emplace(std::initializer_list<Elem> ilist, Types &&...args) {
            reset();
            return emplace_<type_traits::other_trans::decay_t<ValueType>>(ilist, utility::forward<Types>(args)...);
        }

        void reset() noexcept {
            using namespace implements;
            if (!has_value()) {
                return;
            }
            switch (get_representation()) {
                case any_representation::_small:
                    storage.small_storage.rtti_manager->destroy_(&storage.small_storage.data);
                    break;
                case any_representation::big:
                    storage.big_storage.rtti_manager->destory_(storage.big_storage.ptr);
                    break;
                case any_representation::trivial:
                default:
                    break;
            }
            storage.type_data = 0;
        }

        void swap(any &right) noexcept {
            right = utility::exchange(*this, utility::move(right));
        }

        RAINY_NODISCARD bool has_value() const noexcept {
            return storage.type_data != 0;
        }

        RAINY_NODISCARD const foundation::rtti::typeinfo &type() const noexcept {
            const foundation::rtti::typeinfo *const info = type_info();
            if (info) {
                return *info;
            }
            return rainy_typeid(void);
        }

        template <typename Type>
        RAINY_NODISCARD auto as() noexcept -> decltype(auto) {
#if RAINY_USING_MSVC
#pragma warning(push)
#pragma warning(disable : 6011)
#endif
            rainy_let ptr = cast_to_pointer<type_traits::other_trans::decay_t<Type>>();
            if (!ptr) {
                std::terminate();
            }
            if constexpr (std::is_lvalue_reference_v<Type>) {
                if constexpr (std::is_const_v<std::remove_reference_t<Type>>) {
                    // 返回 const lvalue 引用
                    return *static_cast<const std::remove_reference_t<Type> *>(ptr);
                } else {
                    // 返回非 const lvalue 引用
                    return *static_cast<std::remove_reference_t<Type> *>(ptr);
                }
            } else if constexpr (std::is_rvalue_reference_v<Type>) {
                if constexpr (std::is_const_v<std::remove_reference_t<Type>>) {
                    // 返回 const rvalue 引用
                    return *static_cast<const std::remove_reference_t<Type> *>(ptr);
                } else {
                    // 返回非 const rvalue 引用
                    return utility::move(*static_cast<std::remove_reference_t<Type> *>(ptr));
                }
            } else {
                return *static_cast<std::remove_reference_t<Type> *>(ptr);
            }
#if RAINY_USING_MSVC
#pragma warning(pop)
#endif
        }

        template <typename Type>
        RAINY_NODISCARD auto as() const noexcept -> decltype(auto) {
            using namespace type_traits::cv_modify;
            using ret_type = decltype(utility::declval<any &>().template as<Type>());
            rainy_let nonconst = const_cast<any *>(this);
            if constexpr (type_traits::primary_types::is_rvalue_reference_v<ret_type>) {
                return nonconst->as<type_traits::reference_modify::add_const_rvalue_ref_t<Type>>();
            } else {
                return nonconst->as<type_traits::reference_modify::add_const_lvalue_ref_t<Type>>();
            }
        }

        template <typename Decayed>
        RAINY_NODISCARD const Decayed *cast_to_pointer() const noexcept {
            using namespace foundation::rtti;
            static constexpr std::size_t target_hashcode = foundation::rtti::typeinfo::create<Decayed>().hash_code();
            const foundation::rtti::typeinfo *const info = type_info();
            if (!info) {
                return nullptr;
            }
            if (info->has_traits(traits::is_lref)) {
                static constexpr std::size_t add_ref_hash =
                    foundation::rtti::typeinfo::create<type_traits::reference_modify::add_lvalue_reference_t<Decayed>>().hash_code();
                return info->hash_code() == add_ref_hash ? static_cast<const Decayed *>(target_as_void_ptr()) : nullptr;
            } else if (info->has_traits(traits::is_rref)) {
                static constexpr std::size_t add_ref_hash =
                    foundation::rtti::typeinfo::create<type_traits::reference_modify::add_rvalue_reference_t<Decayed>>().hash_code();
                return info->hash_code() == add_ref_hash ? static_cast<const Decayed *>(target_as_void_ptr()) : nullptr;
            }
            return info->hash_code() == target_hashcode ? static_cast<const Decayed *>(target_as_void_ptr()) : nullptr;
        }

        template <typename Decayed>
        RAINY_NODISCARD Decayed *cast_to_pointer() noexcept {
            using remove_ref_t = type_traits::reference_modify::remove_reference_t<Decayed>;
            return const_cast<remove_ref_t *>(static_cast<const any *>(this)->cast_to_pointer<Decayed>());
        }

#define RAINY_ANY_CAST_TO_DECLARATION(NAME, TYPE)                                                                                     \
    TYPE &to_##NAME() noexcept {                                                                                                      \
        return as<TYPE>();                                                                                                            \
    }                                                                                                                                 \
    const TYPE &to_##NAME() const noexcept {                                                                                          \
        return as<TYPE>();                                                                                                            \
    }

        RAINY_ANY_CAST_TO_DECLARATION(int, int);
        RAINY_ANY_CAST_TO_DECLARATION(char, char);
        RAINY_ANY_CAST_TO_DECLARATION(float, float);
        RAINY_ANY_CAST_TO_DECLARATION(double, double);
        RAINY_ANY_CAST_TO_DECLARATION(long, long);
        RAINY_ANY_CAST_TO_DECLARATION(bool, bool);
        RAINY_ANY_CAST_TO_DECLARATION(long_long, long);
        RAINY_ANY_CAST_TO_DECLARATION(short, short);
        RAINY_ANY_CAST_TO_DECLARATION(int8, std::int8_t);
        RAINY_ANY_CAST_TO_DECLARATION(int16, std::int16_t);
        RAINY_ANY_CAST_TO_DECLARATION(int32, std::int32_t);
        RAINY_ANY_CAST_TO_DECLARATION(int64, std::int64_t);
        RAINY_ANY_CAST_TO_DECLARATION(uint8, std::uint8_t);
        RAINY_ANY_CAST_TO_DECLARATION(uint16, std::uint16_t);
        RAINY_ANY_CAST_TO_DECLARATION(uint32, std::uint64_t);
        RAINY_ANY_CAST_TO_DECLARATION(uint64, std::uint64_t);
        RAINY_ANY_CAST_TO_DECLARATION(string, std::string);
        RAINY_ANY_CAST_TO_DECLARATION(wstring, std::wstring);
        RAINY_ANY_CAST_TO_DECLARATION(string_view, std::string_view);
        RAINY_ANY_CAST_TO_DECLARATION(wstring_view, std::wstring_view);
        RAINY_ANY_CAST_TO_DECLARATION(char16, char16_t);
        RAINY_ANY_CAST_TO_DECLARATION(char32, char32_t);
#if RAINY_HAS_CXX20
        RAINY_ANY_CAST_TO_DECLARATION(char8, char8_t);
#endif
#undef RAINY_ANY_CAST_TO_DECLARATION

        template <typename TargetType>
        RAINY_NODISCARD TargetType convert() {
            using foundation::rtti::traits;
            static foundation::rtti::typeinfo target_type = foundation::rtti::typeinfo::create<TargetType>();
            std::size_t hash_code = type().hash_code();
            if (hash_code == rainy_typehash(TargetType)) {
                return as<TargetType>();
            } else if (type().has_traits(traits::is_lref) || type().has_traits(traits::is_rref)) {
                return as<TargetType>();
            }
            if constexpr (utility::any_converter<TargetType>::convertible) {
                return utility::any_converter<TargetType>::convert(*this);
            } else {
                std::terminate();
            }
        }

        template <typename TargetType>
        RAINY_NODISCARD TargetType convert() const {
            return const_cast<any *>(this)->convert<TargetType>();
        }

        const void *target_as_void_ptr() const {
            switch (get_representation()) {
                case implements::any_representation::big:
                    return reinterpret_cast<const void *>(storage.big_storage.ptr);
                case implements::any_representation::reference:
                    return storage.reference_storage.ptr;
                case implements::any_representation::trivial:
                    return reinterpret_cast<const void *>(&storage.trivial_data);
                case implements::any_representation::_small:
                    return reinterpret_cast<const void *>(&storage.small_storage.data);
            }
            return nullptr;
        }
        
        bool operator<(const utility::any &right) const {
            std::tuple<const utility::any *,const utility::any&> tuple{this, right};
            return storage.policy->fn(implements::any_operater_policy::compare_less, &tuple);
        }

        bool operator>(const utility::any &right) const {
            std::tuple<const utility::any *, const utility::any &> tuple{this, right};
            return (!storage.policy->fn(implements::any_operater_policy::compare_less, &tuple) &&
                    !storage.policy->fn(implements::any_operater_policy::compare_equal, &tuple));
        }

        bool operator>=(const utility::any &right) const {
            std::tuple<const utility::any *, const utility::any &> tuple{this, right};
            return ((storage.policy->fn(implements::any_operater_policy::compare_equal, &tuple)) ||
                    (!storage.policy->fn(implements::any_operater_policy::compare_less, &tuple)));
        }

        bool operator<=(const utility::any &right) const {
            std::tuple<const utility::any *, const utility::any &> tuple{this, right};
            return (storage.policy->fn(implements::any_operater_policy::compare_equal, &tuple) ||
                    storage.policy->fn(implements::any_operater_policy::compare_less, &tuple));
        }

        bool operator==(const utility::any& right) const {
            std::tuple<const utility::any *, const utility::any &> tuple{this, right};
            return storage.policy->fn(implements::any_operater_policy::compare_equal, &tuple);
        }

        bool operator!=(const utility::any &right) const {
            return !(*this == right);
        }

        std::size_t hash_code() const noexcept {
            std::size_t ret{};
            std::tuple<const utility::any *, std::size_t &> tuple{this, ret};
            if (storage.policy->fn(implements::any_operater_policy::eval_hash, &tuple)) {
                return ret;
            }
            std::terminate();
        }

    private:
        static constexpr std::uintptr_t rep_mask = 3;

        RAINY_NODISCARD implements::any_representation get_representation() const noexcept {
            return static_cast<implements::any_representation>(storage.type_data & rep_mask);
        }

        RAINY_NODISCARD const foundation::rtti::typeinfo *type_info() const noexcept {
            return reinterpret_cast<const foundation::rtti::typeinfo *>(storage.type_data & ~rep_mask);
        }

        void move_from(any &right) noexcept {
            using namespace implements;
            storage.type_data = right.storage.type_data;
            storage.policy = right.storage.policy;
            switch (get_representation()) {
                case any_representation::_small:
                    storage.small_storage.rtti_manager = right.storage.small_storage.rtti_manager;
                    storage.small_storage.rtti_manager->move_(&storage.small_storage.data, &right.storage.small_storage.data);
                    break;
                case any_representation::big:
                    storage.big_storage.rtti_manager = right.storage.big_storage.rtti_manager;
                    storage.big_storage.ptr = right.storage.big_storage.ptr;
                    right.storage.type_data = 0;
                    break;
                case any_representation::reference:
                    storage.reference_storage.ptr = right.storage.reference_storage.ptr;
                    right.storage.type_data = 0;
                    break;
                case any_representation::trivial:
                default:
                    std::memcpy(storage.trivial_data, right.storage.trivial_data, sizeof(storage.trivial_data));
                    break;
            }
        }

        void assign_(any right) noexcept {
            reset();
            move_from(right);
        }

        template <typename Decayed, typename... Types>
        decltype(auto) emplace_(Types &&...args) {
            using decayed = type_traits::other_trans::decay_t<Decayed>;
            if constexpr (type_traits::composite_types::is_reference_v<Decayed>) {
                if constexpr (!type_traits::primary_types::is_array_v<type_traits::reference_modify::remove_reference_t<Decayed>>) {
                    return emplace_ref<Decayed>(utility::forward<Types>(args)...);
                } else {
                    return emplace_<decayed>(utility::forward<Types>(args)...);
                }
            } else if constexpr (implements::any_is_trivial<decayed>) {
                auto &object = reinterpret_cast<decayed &>(storage.trivial_data);
                ::new (utility::addressof(object)) decayed(utility::forward<Types>(args)...);
                storage.type_data = reinterpret_cast<std::uintptr_t>(&rainy_typeid(decayed)) |
                                    static_cast<std::uintptr_t>(implements::any_representation::trivial);
                storage.policy = &implements::any_operater_policy_object<decayed>;
                return object;
            } else if constexpr (implements::any_is_small<decayed>) {
                auto &object = reinterpret_cast<decayed &>(storage.small_storage.data);
                ::new (utility::addressof(object)) decayed(utility::forward<Types>(args)...);
                storage.small_storage.rtti_manager = &implements::any_small_rtti_manager_object<decayed>;
                storage.type_data = reinterpret_cast<std::uintptr_t>(&rainy_typeid(decayed)) |
                                    static_cast<std::uintptr_t>(implements::any_representation::_small);
                storage.policy = &implements::any_operater_policy_object<decayed>;
                return object;
            } else {
                decayed *const new_alloc_ptr = ::new decayed(utility::forward<Types>(args)...);
                storage.big_storage.ptr = new_alloc_ptr;
                storage.big_storage.rtti_manager = &implements::any_big_rtti_manager_object<decayed>;
                storage.type_data = reinterpret_cast<std::uintptr_t>(&rainy_typeid(decayed)) |
                                    static_cast<std::uintptr_t>(implements::any_representation::big);
                storage.policy = &implements::any_operater_policy_object<decayed>;
                return *new_alloc_ptr;
            }
        }

        template <typename Decayed, typename Type>
        decltype(auto) emplace_ref(Type &&reference) {
            storage.reference_storage.ptr = utility::addressof(reference);
            storage.type_data = reinterpret_cast<std::uintptr_t>(&rainy_typeid(Decayed)) |
                                static_cast<std::uintptr_t>(implements::any_representation::reference);
            storage.policy = &implements::any_operater_policy_object<Decayed>;
            return reference;
        }

        struct small_storage_t {
            unsigned char data[implements::any_small_space_size];
            const implements::small_any_rtti_manager *rtti_manager;
        };

        static_assert(sizeof(small_storage_t) == implements::any_trivial_space_size);

        struct bigstorage_t {
            unsigned char padding[implements::any_small_space_size - sizeof(void *)];
            void *ptr;
            const implements::big_any_rtti_manager *rtti_manager;
        };

        static_assert(sizeof(bigstorage_t) == implements::any_trivial_space_size);

        struct reference_storage_t {
            const void *ptr;
        };

        struct storage_t {
            union {
                unsigned char trivial_data[implements::any_trivial_space_size];
                small_storage_t small_storage;
                bigstorage_t big_storage;
                reference_storage_t reference_storage;
            };
            const implements::any_operater_policy *policy;
            std::uintptr_t type_data;
        };

        static_assert(sizeof(storage_t) == implements::any_trivial_space_size + (sizeof(void *) * 2));
        static_assert(type_traits::type_properties::is_standard_layout_v<storage_t>);

        union {
            storage_t storage{};
            std::max_align_t dummy;
        };
    };
}

namespace rainy::utility {
    template <typename Ty, typename... Args,
              typename = type_traits::other_trans::enable_if_t<type_traits::type_properties::is_constructible_v<any, std::in_place_type_t<Ty>, Args...>, int>>
    RAINY_NODISCARD any make_any(Args &&...args) {
        return any{std::in_place_type<Ty>, utility::forward<Args>(args)...};
    }

    template <typename Ty, typename U, typename... Args,
              typename = type_traits::other_trans::enable_if_t<type_traits::type_properties::is_constructible_v<any, std::in_place_type_t<Ty>, std::initializer_list<U> &, Args...>>>
    RAINY_NODISCARD any make_any(std::initializer_list<U> initializer_list, Args &&...args) {
        return any{std::in_place_type<Ty>, initializer_list, utility::forward<Args>(args)...};
    }

    RAINY_INLINE void swap(utility::any& left,utility::any& right) {
        left.swap(right);
    }
}

#include <iostream>
namespace rainy::utility {
    template <typename TargetType>
    struct any_converter<TargetType, type_traits::other_trans::enable_if_t<
            type_traits::composite_types::is_reference_v<TargetType> ||
            type_traits::primary_types::is_pointer_reference_v<TargetType>
        >> {
        static constexpr bool convertible = true;

        template <typename Any,
                  type_traits::other_trans::enable_if_t<
                      type_traits::type_relations::is_same_v<type_traits::other_trans::decay_t<Any>, utility::any>, int> = 0>
        static decltype(auto) convert(Any &&any) {
            using namespace foundation::rtti;
            return basic_convert(any.target_as_void_ptr(), any.type(), true);
        }

        static decltype(auto) basic_convert(const void *target_pointer, const foundation::rtti::typeinfo &type, bool is_any) {
            using namespace foundation::rtti;
            using namespace foundation::exceptions::cast;
            using raw_type = std::remove_reference_t<TargetType>;
            static const auto target_type = typeinfo::create<TargetType>();
            if (!target_type.is_compatible(type)) {
                throw_bad_any_cast();
            }
            if constexpr (std::is_lvalue_reference_v<TargetType>) {
                if constexpr (std::is_const_v<raw_type>) {
                    return *static_cast<const raw_type *>(target_pointer);
                } else {
                    utility::expects(!type.has_traits(traits::is_const), "Cannot cast const value to non-const lvalue reference");
                    utility::expects(!type.has_traits(traits::is_rref), "Cannot cast rvalue reference to lvalue reference");
                    utility::expects(type.has_traits(traits::is_lref) || is_any, "Cannot cast non-lvalue to lvalue reference");
                    return *static_cast<raw_type *>(const_cast<void *>(target_pointer));
                }
            } else if constexpr (std::is_rvalue_reference_v<TargetType>) {
                if constexpr (std::is_const_v<raw_type>) {
                    return *static_cast<const raw_type *>(target_pointer);
                } else {
                    utility::expects(!type.has_traits(traits::is_const), "Cannot cast const value to non-const rvalue reference");
                    return utility::move(*static_cast<raw_type *>(const_cast<void *>(target_pointer)));
                }
            } else {
                return *static_cast<raw_type *>(const_cast<void *>(target_pointer));
            }
            throw_bad_any_cast();
        }

        static bool is_convertible(const foundation::rtti::typeinfo &type) {
            static const auto target_type = foundation::rtti::typeinfo::create<TargetType>();
            if (target_type.is_compatible(type)) {
                if constexpr (type_traits::composite_types::is_reference_v<TargetType>) {
                    if constexpr (type_traits::type_properties::is_const_v<std::remove_reference_t<TargetType>>) {
                        return true;
                    } else if constexpr (type_traits::primary_types::is_rvalue_reference_v<TargetType>) {
                        if (type.has_traits(foundation::rtti::traits::is_const)) {
                            return false;
                        }
                        return true;
                    } else {
                        if (type.has_traits(foundation::rtti::traits::is_const) ||
                            type.has_traits(foundation::rtti::traits::is_rref) ||
                            !type.has_traits(foundation::rtti::traits::is_lref)) {
                            return false;
                        }
                        return true;
                    }
                } else {
                    return true;
                }
            }
            return false;
        }
    };

    template <typename TargetType>
    struct any_converter<TargetType,
                         type_traits::other_trans::enable_if_t<type_traits::composite_types::is_arithmetic_v<TargetType>>> {
        static constexpr bool convertible = true;

        template <typename Any,
                  type_traits::other_trans::enable_if_t<
                      type_traits::type_relations::is_same_v<type_traits::cv_modify::remove_cvref_t<Any>, utility::any>, int> = 0>
        static TargetType convert(Any &&any) {
            return basic_convert(any.target_as_void_ptr(), any.type());
        }

        static TargetType basic_convert(const void *target_pointer, const foundation::rtti::typeinfo &type, bool = false) {
            using namespace foundation::rtti;
            foundation::rtti::typeinfo target_type = type;
            if (target_type.has_traits(traits::is_const) || target_type.has_traits(traits::is_volatile)) {
                target_type = target_type.remove_cv();
            }
            if (target_type.has_traits(traits::is_lref) || target_type.has_traits(traits::is_rref)) {
                target_type = target_type.remove_reference();
            }
            switch (target_type.hash_code()) {
                case rainy_typehash(int):
                    return static_cast<TargetType>(*static_cast<const int *>(target_pointer));
                case rainy_typehash(char):
                    return static_cast<TargetType>(*static_cast<const char *>(target_pointer));
                case rainy_typehash(float):
                    return static_cast<TargetType>(*static_cast<const float *>(target_pointer));
                case rainy_typehash(double):
                    return static_cast<TargetType>(*static_cast<const double *>(target_pointer));
                case rainy_typehash(long):
                    return static_cast<TargetType>(*static_cast<const long *>(target_pointer));
                case rainy_typehash(bool):
                    return static_cast<TargetType>(*static_cast<const bool *>(target_pointer));
                case rainy_typehash(long long):
                    return static_cast<TargetType>(*static_cast<const long long *>(target_pointer));
                case rainy_typehash(short):
                    return static_cast<TargetType>(*static_cast<const short *>(target_pointer));
                case rainy_typehash(std::int8_t):
                    return static_cast<TargetType>(*static_cast<const std::int8_t *>(target_pointer));
                case rainy_typehash(std::uint8_t):
                    return static_cast<TargetType>(*static_cast<const std::uint8_t *>(target_pointer));
                case rainy_typehash(std::uint16_t):
                    return static_cast<TargetType>(*static_cast<const std::uint16_t *>(target_pointer));
                case rainy_typehash(std::uint32_t):
                    return static_cast<TargetType>(*static_cast<const std::uint32_t *>(target_pointer));
                case rainy_typehash(std::uint64_t):
                    return static_cast<TargetType>(*static_cast<const std::uint64_t *>(target_pointer));
                default:
                    break;
            }
            foundation::exceptions::cast::throw_bad_any_cast();
        }

        static bool is_convertible(const foundation::rtti::typeinfo &type) {
            using namespace foundation::rtti;
            foundation::rtti::typeinfo target_type = type;
            if (target_type.has_traits(traits::is_const) || target_type.has_traits(traits::is_volatile)) {
                target_type = target_type.remove_cv();
            }
            if (target_type.has_traits(traits::is_lref) || target_type.has_traits(traits::is_rref)) {
                target_type = target_type.remove_reference();
            }
            switch (target_type.hash_code()) {
                case rainy_typehash(int):
                case rainy_typehash(char):
                case rainy_typehash(float):
                case rainy_typehash(double):
                case rainy_typehash(long):
                case rainy_typehash(bool):
                case rainy_typehash(long long):
                case rainy_typehash(short):
                case rainy_typehash(std::int8_t):
                case rainy_typehash(std::uint8_t):
                case rainy_typehash(std::uint16_t):
                case rainy_typehash(std::uint32_t):
                case rainy_typehash(std::uint64_t):
                    return true;
                default:
                    return false;
            }
        }
    };

    template <typename CharType>
    struct any_converter<std::basic_string_view<CharType>, void> {
        static constexpr bool convertible = true;

        template <typename Any,
                  type_traits::other_trans::enable_if_t<
                      type_traits::type_relations::is_same_v<type_traits::cv_modify::remove_cvref_t<Any>, utility::any>, int> = 0>
        static std::basic_string_view<CharType> convert(Any &&any) {
            return basic_convert(any.target_as_void_ptr(), any.type(), true);
        }

        static std::basic_string_view<CharType> basic_convert(const void *target_pointer, const foundation::rtti::typeinfo &type,
                                                              bool is_any = false) {
            using namespace foundation::rtti;
            using namespace foundation::exceptions::cast;
            if (is_any) {
                auto ptr = *static_cast<const void *const *>(target_pointer);
                return basic_convert(ptr, type.remove_reference(), false);
            }
            if constexpr (type_traits::helper::is_wchar_t<CharType>) {
                // any -> std::wstring_view   
                switch (type.hash_code()) {
                    case rainy_typehash(const wchar_t *):
                        return std::wstring_view{static_cast<const wchar_t *const>(target_pointer)};
                    case rainy_typehash(wchar_t *):
                        return std::wstring_view{static_cast<wchar_t *const>(const_cast<void *const>(target_pointer))};
                    case rainy_typehash(std::wstring_view):
                        return *static_cast<const std::wstring_view *>(target_pointer);
                    case rainy_typehash(std::wstring):
                        return static_cast<std::wstring_view>(*static_cast<const std::wstring *>(target_pointer));
                    default:
                        break;
                }
                
            } else if constexpr (type_traits::type_relations::is_same_v<CharType, char>) {
                // any -> std::string
                switch (type.remove_reference().hash_code()) {
                    case rainy_typehash(const char *):
                        return std::string_view{static_cast<const char *const>(target_pointer)};
                    case rainy_typehash(char *):
                        return std::string_view{static_cast<const char *const>(target_pointer)};
                    case rainy_typehash(std::string_view):
                        return *static_cast<const std::string_view *>(target_pointer);
                    case rainy_typehash(std::string):
                        return static_cast<std::string_view>(*static_cast<const std::string *>(target_pointer));
                    default:
                        break;
                }
            } else if constexpr (type_traits::type_relations::is_same_v<CharType, char16_t>) {
                // any -> std::u16string_view
                switch (type.remove_reference().hash_code()) {
                    case rainy_typehash(const char16_t *):
                        return std::u16string_view{static_cast<const char16_t *const>(target_pointer)};
                    case rainy_typehash(char16_t *):
                        return std::u16string_view{static_cast<const char16_t *const>(const_cast<void *const>(target_pointer))};
                    case rainy_typehash(std::u16string_view):
                        return *static_cast<const std::u16string_view *>(target_pointer);
                    case rainy_typehash(std::u16string):
                        return static_cast<std::u16string_view>(*static_cast<const std::u16string *>(target_pointer));
                    default:
                        break;
                }
            } else if constexpr (type_traits::type_relations::is_same_v<CharType, char32_t>) {
                // any -> std::u32string_view
                switch (type.remove_reference().hash_code()) {
                    case rainy_typehash(const char32_t *):
                        return std::u32string_view{static_cast<const char32_t *const>(target_pointer)};
                    case rainy_typehash(char32_t *):
                        return std::u32string_view{static_cast<const char32_t *const>(target_pointer)};
                    case rainy_typehash(std::u32string_view):
                        return *static_cast<const std::u32string_view *>(target_pointer);
                    case rainy_typehash(std::u32string):
                        return static_cast<std::u32string_view>(*static_cast<const std::u32string *>(target_pointer));
                    default:
                        break;
                }
            }
#ifdef __cpp_lib_char8_t
            else if constexpr (type_traits::type_relations::is_same_v<CharType, char8_t>) {
                // any -> std::u8string_view
                switch (type.remove_reference().hash_code()) {
                    case rainy_typehash(const char8_t *):
                        return std::u8string_view{static_cast<const char8_t *const>(target_pointer)};
                    case rainy_typehash(char8_t *):
                        return std::u8string_view{static_cast<const char8_t *const>(target_pointer)};
                    case rainy_typehash(std::u8string_view):
                        return *static_cast<const std::u8string_view *>(target_pointer);
                    case rainy_typehash(std::u8string):
                        return static_cast<std::u8string_view>(*static_cast<const std::u8string *>(target_pointer));
                    default:
                        break;
                }
            }
#endif
            throw_bad_any_cast();
        }

        static bool is_convertible(const foundation::rtti::typeinfo &type) {
            using namespace foundation::rtti;
            if constexpr (type_traits::helper::is_wchar_t<CharType>) {
                switch (type.remove_reference().hash_code()) {
                    case rainy_typehash(const wchar_t *):
                    case rainy_typehash(wchar_t *):
                    case rainy_typehash(std::wstring_view):
                    case rainy_typehash(std::wstring):
                        return true;
                    default:
                        break;
                }
            } else if constexpr (type_traits::type_relations::is_same_v<CharType, char>) {
                switch (type.remove_reference().hash_code()) {
                    case rainy_typehash(const char *):
                    case rainy_typehash(char *):
                    case rainy_typehash(std::string_view):
                    case rainy_typehash(std::string):
                        return true;
                    default:
                        break;
                }
            } else if constexpr (type_traits::type_relations::is_same_v<CharType, char16_t>) {
                switch (type.remove_reference().hash_code()) {
                    case rainy_typehash(const char16_t *):
                    case rainy_typehash(char16_t *):
                    case rainy_typehash(std::u16string_view):
                    case rainy_typehash(std::u16string):
                        return true;
                    default:
                        break;
                }
            } else if constexpr (type_traits::type_relations::is_same_v<CharType, char32_t>) {
                switch (type.remove_reference().hash_code()) {
                    case rainy_typehash(const char32_t *):
                    case rainy_typehash(char32_t *):
                    case rainy_typehash(std::u32string_view):
                    case rainy_typehash(std::u32string):
                        return true;
                    default:
                        break;
                }
            }
#ifdef __cpp_lib_char8_t
            else if constexpr (type_traits::type_relations::is_same_v<CharType, char8_t>) {
                switch (type.remove_reference().hash_code()) {
                    case rainy_typehash(const char8_t *):
                    case rainy_typehash(char8_t *):
                    case rainy_typehash(std::u8string_view):
                    case rainy_typehash(std::u8string):
                        return true;
                    default:
                        break;
                }
            }
#endif
            return false;
        }
    };
}

namespace rainy::utility {
    template <typename CharType, typename Traits>
    std::basic_ostream<CharType, Traits> &operator<<(std::basic_ostream<CharType, Traits> &left, const any &right) {
        if (!right.has_value()) {
            return left;
        }
        std::size_t typehash = right.type().remove_reference().hash_code();
        switch (typehash) {
            case rainy_typehash(char *):
                return left << right.as<char *>();
            case rainy_typehash(wchar_t *):
                if constexpr (type_traits::helper::is_wchar_t<CharType>) {
                    return left << right.as<wchar_t *>();
                }
            case rainy_typehash(const char *):
                return left << right.as<const char *>();
            case rainy_typehash(const wchar_t *):
                if constexpr (type_traits::helper::is_wchar_t<CharType>) {
                    return left << right.as<const wchar_t *>();
                }
            case rainy_typehash(std::string):
            case rainy_typehash(std::wstring):
            case rainy_typehash(std::string_view):
            case rainy_typehash(std::wstring_view):
                if (typehash == rainy_typehash(std::string) || typehash == rainy_typehash(std::string_view)) {
                    if constexpr (!type_traits::helper::is_wchar_t<CharType>) {
                        return left << (typehash == rainy_typehash(std::string) ? right.to_string() : right.to_string_view());
                    }
                } else if (typehash == rainy_typehash(std::wstring) || typehash == rainy_typehash(std::wstring_view)) {
                    if constexpr (type_traits::helper::is_wchar_t<CharType>) {
                        return left << (typehash == rainy_typehash(std::wstring) ? right.to_wstring() : right.to_wstring_view());
                    }
                } else if (typehash == rainy_typehash(char *) || typehash == rainy_typehash(const char *)) {
                    return left << (typehash == rainy_typehash(const char *) ? right.as<const char *>() : right.as<char *>());
                } else {
                    if constexpr (type_traits::helper::is_wchar_t<CharType>) {
                        return left << (typehash == rainy_typehash(wchar_t *) ? right.as<wchar_t *>() : right.as<const wchar_t *>());
                    }
                }
                return left;
            default:
                break;
        }
        switch (typehash) {
            case rainy_typehash(int):
                return left << right.to_int();
            case rainy_typehash(char):
                return left << right.to_char();
            case rainy_typehash(float):
                return left << right.to_float();
            case rainy_typehash(double):
                return left << right.to_double();
            case rainy_typehash(long):
                return left << right.to_long();
            case rainy_typehash(bool):
                return left << right.to_bool();
            case rainy_typehash(long long):
                return left << right.to_long_long();
            case rainy_typehash(short):
                return left << right.to_short();
            case rainy_typehash(std::int8_t):
                return left << right.to_int8();
            case rainy_typehash(std::uint8_t):
                return left << right.to_uint8();
            case rainy_typehash(std::uint16_t):
                return left << right.to_uint16();
            case rainy_typehash(std::uint32_t):
                return left << right.to_uint32();
            case rainy_typehash(std::uint64_t):
                return left << right.to_uint64();
            default:
                if constexpr (!type_traits::helper::is_wchar_t<CharType>) {
                    implements::any_ostream_converter_register::instance().call_converter(left, right.type(), &right);
                }
                break;
        }
        return left;
    }
}

namespace rainy::utility::implements {
    RAINY_INLINE bool compare_equal_helpr(const utility::any &left, const utility::any &right) {
        if (left.type().is_floating_point() || right.type().is_floating_point()) {
            return core::builtin::almost_equal(left.convert<double>(), right.convert<double>());
        } else {
            if (any_converter<std::int64_t>::is_convertible(left.type()) && any_converter<std::int64_t>::is_convertible(right.type())) {
                return left.convert<std::int64_t>() == right.convert<std::int64_t>();
            }
            return false;
        }
    }

    template <typename Ty>
    bool any_operater_policy::invoke(operation op, void *const data) {
        switch (op) {
            case compare_less: {
                const auto *res = static_cast<const std::tuple<const utility::any *, const utility::any &> *const>(data);
                const utility::any *left = std::get<0>(*res);
                const utility::any &right = std::get<1>(*res);
                static constexpr auto left_type = foundation::rtti::typeinfo::create<Ty>();
                if constexpr (type_traits::composite_types::is_arithmetic_v<Ty>) {
                    if (right.type().is_arithmetic()) {
                        if (left_type == right.type()) {
                            return (left->as<Ty>() < right.as<Ty>());
                        }
                        if (left_type.is_floating_point() || right.type().is_floating_point()) {
                            return (left->convert<double>() < right.convert<double>());
                        } else {
                            return (left->convert<std::int64_t>() < right.convert<std::int64_t>());
                        }
                    }
                }
                if (!left_type.is_nullptr() && right.type().is_nullptr()) {
                    return false;
                }
                if constexpr (type_traits::type_relations::is_same_v<Ty, std::string_view>) {
                    if (left_type == right.type()) {
                        return (left->as<std::string_view>() < right.as<std::string_view>());
                    }
                    bool convertible = any_converter<std::string_view>::is_convertible(right.type());
                    if (convertible) {
                        return (left->as<std::string_view>() < any_converter<std::string_view>::convert(right));
                    }
                } else {
                    if (any_converter<std::string_view>::is_convertible(left->type()) &&
                        any_converter<std::string_view>::is_convertible(right.type())) {
                        return left->convert<std::string_view>() < right.convert<std::string_view>();
                    }
                }
                return false;
            }
            case compare_equal: {
                const auto *res = static_cast<const std::tuple<const utility::any *, const utility::any &> *const>(data);
                const utility::any *left = std::get<0>(*res);
                const utility::any &right = std::get<1>(*res);
                static constexpr auto left_type = foundation::rtti::typeinfo::create<Ty>();
                if constexpr (type_traits::composite_types::is_arithmetic_v<Ty>) {
                    if (right.type().is_arithmetic()) {
                        if (left_type == right.type()) {
                            return (left->as<Ty>() == right.as<Ty>());
                        }
                        return compare_equal_helpr(left, right);
                    }
                }
                if (!left_type.is_nullptr() && right.type().is_nullptr()) {
                    return false;
                }
                if constexpr (type_traits::type_relations::is_same_v<Ty, std::string_view>) {
                    if (left_type == right.type()) {
                        return (left->as<std::string_view>() == right.as<std::string_view>());
                    }
                    bool convertible = any_converter<std::string_view>::is_convertible(right.type());
                    if (convertible) {
                        return (left->as<std::string_view>() == any_converter<std::string_view>::convert(right));
                    }
                } else {
                    if (any_converter<std::string_view>::is_convertible(left->type()) &&
                        any_converter<std::string_view>::is_convertible(right.type())) {
                        return left->convert<std::string_view>() == right.convert<std::string_view>();
                    }
                }
                return false;
            }
            case operation::compare_less_equal: {
                const auto *res = static_cast<const std::tuple<const utility::any *, const utility::any &> *const>(data);
                const utility::any *left = std::get<0>(*res);
                const utility::any &right = std::get<1>(*res);
                static constexpr auto left_type = foundation::rtti::typeinfo::create<Ty>();
                if constexpr (type_traits::composite_types::is_arithmetic_v<Ty>) {
                    if (right.type().is_arithmetic()) {
                        if (left_type <= right.type()) {
                            return (left->as<Ty>() <= right.as<Ty>());
                        }
                        if (left_type.is_floating_point() || right.type().is_floating_point()) {
                            return (left->convert<double>() <= right.convert<double>());
                        } else {
                            return (left->convert<std::int64_t>() <= right.convert<std::int64_t>());
                        }
                    }
                }
                if (!left_type.is_nullptr() && right.type().is_nullptr()) {
                    return false;
                }
                if constexpr (type_traits::type_relations::is_same_v<Ty, std::string_view>) {
                    if (left_type == right.type()) {
                        return (left->as<std::string_view>() <= right.as<std::string_view>());
                    }
                    if (any_converter<std::string_view>::is_convertible(right.type())) {
                        return (left->as<std::string_view>() <= any_converter<std::string_view>::convert(right));
                    }
                } else {
                    if (any_converter<std::string_view>::is_convertible(left->type()) &&
                        any_converter<std::string_view>::is_convertible(right.type())) {
                        return left->convert<std::string_view>() <= right.convert<std::string_view>();
                    }
                }
                return false;
            }
            case any_operater_policy::eval_hash: {
                auto* res = static_cast<std::tuple<const utility::any *, std::size_t&>*>(data);
                const utility::any* any = std::get<0>(*res);
                std::size_t &hashcode = std::get<1>(*res);
                if constexpr (utility::is_support_standard_hasher_available<Ty>::value) {
                    hashcode = std::hash<Ty>{}(any->as<Ty>());
                    return true;
                } else {
                    return false;
                }
            }
        }
        return false;
    }
}

namespace std {
    template <>
    struct hash<rainy::utility::any> {
        RAINY_NODISCARD std::size_t operator()(const rainy::utility::any right) const {
            return right.hash_code();
        }
    };
}

#if RAINY_USING_MSVC
#pragma warning(pop)
#endif

#endif
