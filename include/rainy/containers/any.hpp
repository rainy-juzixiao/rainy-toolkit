#ifndef RAINY_CONTAINERS_ANY_HPP
#define RAINY_CONTAINERS_ANY_HPP
#include <rainy/core/core.hpp>
#include <rainy/foundation/functional/function_pointer.hpp>
#include <rainy/foundation/typeinfo.hpp>
#include <rainy/foundation/system/basic_exception.hpp>
#include <rainy/meta/type_traits.hpp>
#include <utility>

/*
 * any的基础utils和异常实现
 */
namespace rainy::containers::internals {
    inline constexpr size_t any_trivial_space_size = (core::small_object_num_ptrs - 1) * sizeof(void *);

    template <typename Ty>
    inline constexpr bool any_is_trivial =
        alignof(Ty) <= alignof(max_align_t) && type_traits::type_properties::is_trivially_copyable_v<Ty> && sizeof(Ty) <= any_trivial_space_size;

    inline constexpr size_t any_small_space_size = (core::small_object_num_ptrs - 2) * sizeof(void *);

    template <typename Ty>
    inline constexpr bool any_is_small =
        alignof(Ty) <= alignof(max_align_t) && type_traits::type_properties::is_nothrow_move_constructible_v<Ty> && sizeof(Ty) <= any_small_space_size;

    enum class any_representation : uintptr_t {
        trivial,
        big,
        _small
    };

    struct big_any_rtti_manager {
        using destory_fn = void(void *target) noexcept;
        using copy_fn = void *(const void *source);

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
}

namespace rainy::foundation::system::exceptions::cast {
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

namespace rainy::containers::internals {
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
namespace rainy::containers {
    class any {
    public:
        constexpr any() noexcept {
        }

        any(const any &right) {
            using namespace internals;
            storage.type_data = right.storage.type_data;
            switch (get_representation()) {
                case any_representation::_small:
                    storage.small_storage.rtti_manager = right.storage.small_storage.rtti_manager;
                    storage.small_storage.rtti_manager->copy_(&storage.small_storage.data, &right.storage.small_storage.data);
                    break;
                case any_representation::big:
                    storage.big_storage.rtti_manager = right.storage.big_storage.rtti_manager;
                    storage.big_storage.ptr = storage.big_storage.rtti_manager->copy_(right.storage.big_storage.ptr);
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
            emplace_<type_traits::other_trans::decay_t<ValueType>>(utility::forward<ValueType>(value));
        }

        template <typename ValueType, typename... Types,
                  type_traits::other_trans::enable_if_t<
                      type_traits::logical_traits::conjunction_v<
                          type_traits::type_properties::is_constructible<type_traits::other_trans::decay_t<ValueType>, Types...>,
                          type_traits::type_properties::is_copy_constructible<type_traits::other_trans::decay_t<ValueType>>>,
                      int> = 0>
        explicit any(std::in_place_type_t<ValueType>, Types &&...args) {
            emplace_<type_traits::other_trans::decay_t<ValueType>>(utility::forward<Types>(args)...);
        }

        template <typename ValueType, typename Elem, typename... Types,
                  type_traits::other_trans::enable_if_t<
                      type_traits::logical_traits::conjunction_v<
                          type_traits::type_properties::is_constructible<type_traits::other_trans::decay_t<ValueType>,
                                                                         std::initializer_list<Elem> &, Types...>,
                          type_traits::type_properties::is_copy_constructible<type_traits::other_trans::decay_t<ValueType>>>,
                      int> = 0>
        explicit any(std::in_place_type_t<ValueType>, std::initializer_list<Elem> ilist, Types &&...args) {
            emplace_<type_traits::other_trans::decay_t<ValueType>>(ilist, utility::forward<Types>(args)...);
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
            using namespace internals;
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

        template <typename Decayed>
        RAINY_NODISCARD Decayed &as() noexcept {
            using remove_ref_t = type_traits::reference_modify::remove_reference_t<Decayed>;
            return *cast_to_pointer<remove_ref_t>();
        }

        template <typename Decayed>
        RAINY_NODISCARD const Decayed &as() const noexcept {
            using remove_ref_t = type_traits::reference_modify::remove_reference_t<Decayed>;
            return *cast_to_pointer<remove_ref_t>();
        }

        template <typename Decayed>
        RAINY_NODISCARD const Decayed *cast_to_pointer() const noexcept {
            using remove_ref_t = type_traits::reference_modify::remove_reference_t<Decayed>;
            const foundation::rtti::typeinfo *const info = type_info();
            if (!info || *info != rainy_typeid(remove_ref_t)) {
                return nullptr;
            }
            if constexpr (internals::any_is_trivial<remove_ref_t>) {
                return reinterpret_cast<const remove_ref_t *>(&storage.trivial_data);
            } else if constexpr (internals::any_is_small<remove_ref_t>) {
                return reinterpret_cast<const remove_ref_t *>(&storage.small_storage.data);
            } else {
                return static_cast<const remove_ref_t *>(storage.big_storage.ptr);
            }
        }

        template <typename Decayed>
        RAINY_NODISCARD Decayed *cast_to_pointer() noexcept {
            using remove_ref_t = std::remove_reference_t<Decayed>;
            return const_cast<remove_ref_t *>(static_cast<const any *>(this)->cast_to_pointer<remove_ref_t>());
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
        RAINY_NODISCARD type_traits::other_trans::decay_t<TargetType> convert() const {
            using Decayed = type_traits::other_trans::decay_t<TargetType>;
            if (!has_value()) {
                foundation::system::exceptions::cast::throw_bad_cast("Cannot cast a empty 'any' object");
            }
            std::size_t hash_code = type().hash_code();
            if (hash_code == rainy_typehash(TargetType)) {
                return as<TargetType>();
            }
            if constexpr (type_traits::composite_types::is_arithmetic_v<Decayed>) {
                switch (hash_code) {
                    case rainy_typehash(int):
                        return to_int();
                    case rainy_typehash(char):
                        return to_char();
                    case rainy_typehash(float):
                        return to_float();
                    case rainy_typehash(double):
                        return to_double();
                    case rainy_typehash(long):
                        return to_long();
                    case rainy_typehash(bool):
                        return to_bool();
                    case rainy_typehash(long long):
                        return to_long_long();
                    case rainy_typehash(short):
                        return to_short();
                    case rainy_typehash(std::int8_t):
                        return to_int8();
                    case rainy_typehash(std::uint8_t):
                        return to_uint8();
                    case rainy_typehash(std::uint16_t):
                        return to_uint16();
                    case rainy_typehash(std::uint32_t):
                        return to_uint32();
                    case rainy_typehash(std::uint64_t):
                        return to_uint64();
                    default:
                        break;
                }
            } else if constexpr (type_traits::type_relations::is_same_v<Decayed, std::string>) {
                return hash_code == rainy_typehash(std::string) ? to_string() : std::string{};
            } else if constexpr (type_traits::type_relations::is_same_v<Decayed, std::string_view>) {
                return hash_code == rainy_typehash(std::string_view) ? to_string_view() : std::string_view{};
            } else {
                utility::type_converter<TargetType> converter;
                return converter.convert(*this);
            }
            return TargetType{};
        }

        template <typename CharType, typename Traits>
        friend std::basic_ostream<CharType, Traits> &operator<<(std::basic_ostream<CharType, Traits> &left, const any &right) {
            if (!right.has_value()) {
                return left;
            }
            std::size_t typehash = right.type().hash_code();
            switch (typehash) {
                case rainy_typehash(char *):
                case rainy_typehash(wchar_t *):
                case rainy_typehash(const char *):
                case rainy_typehash(const wchar_t *):
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
                        return left << (typehash == rainy_typehash(char *) ? right.as<char *>() : right.as<const char *>());
                    } else {
                        if constexpr (type_traits::helper::is_wchar_t<CharType>) {
                            return left << (typehash == rainy_typehash(wchar_t *) ? right.as<wchar_t *>()
                                                                                  : right.as<const wchar_t *>());
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
                        internals::any_ostream_converter_register::instance().call_converter(left, right.type(), &right);
                    }
                    break;
            }
            return left;
        }

    private:
        static constexpr std::uintptr_t rep_mask = 3;

        RAINY_NODISCARD internals::any_representation get_representation() const noexcept {
            return static_cast<internals::any_representation>(storage.type_data & rep_mask);
        }

        RAINY_NODISCARD const foundation::rtti::typeinfo *type_info() const noexcept {
            return reinterpret_cast<const foundation::rtti::typeinfo *>(storage.type_data & ~rep_mask);
        }

        void move_from(any &right) noexcept {
            using namespace internals;
            storage.type_data = right.storage.type_data;
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
        Decayed &emplace_(Types &&...args) {
            if constexpr (internals::any_is_trivial<Decayed>) {
                auto &object = reinterpret_cast<Decayed &>(storage.trivial_data);
                ::new (utility::addressof(object)) Decayed(utility::forward<Types>(args)...);
                storage.type_data = reinterpret_cast<std::uintptr_t>(&rainy_typeid(Decayed)) |
                                    static_cast<std::uintptr_t>(internals::any_representation::trivial);
                return object;
            } else if constexpr (internals::any_is_small<Decayed>) {
                auto &object = reinterpret_cast<Decayed &>(storage.small_storage.data);
                ::new (utility::addressof(object)) Decayed(utility::forward<Types>(args)...);
                storage.small_storage.rtti_manager = &internals::any_small_rtti_manager_object<Decayed>;
                storage.type_data = reinterpret_cast<std::uintptr_t>(&rainy_typeid(Decayed)) |
                                    static_cast<std::uintptr_t>(internals::any_representation::_small);
                return object;
            } else {
                Decayed *const new_alloc_ptr = ::new Decayed(utility::forward<Types>(args)...);
                storage.big_storage.ptr = new_alloc_ptr;
                storage.big_storage.rtti_manager = &internals::any_big_rtti_manager_object<Decayed>;
                storage.type_data = reinterpret_cast<std::uintptr_t>(&rainy_typeid(Decayed)) |
                                    static_cast<std::uintptr_t>(internals::any_representation::big);
                return *new_alloc_ptr;
            }
        }

        struct small_storage_t {
            unsigned char data[internals::any_small_space_size];
            const internals::small_any_rtti_manager *rtti_manager;
        };
        static_assert(sizeof(small_storage_t) == internals::any_trivial_space_size);

        struct bigstorage_t {
            unsigned char padding[internals::any_small_space_size - sizeof(void *)];
            void *ptr;
            const internals::big_any_rtti_manager *rtti_manager;
        };
        static_assert(sizeof(bigstorage_t) == internals::any_trivial_space_size);

        struct storage_t {
            union {
                unsigned char trivial_data[internals::any_trivial_space_size];
                small_storage_t small_storage;
                bigstorage_t big_storage;
            };
            std::uintptr_t type_data;
        };
        static_assert(sizeof(storage_t) == internals::any_trivial_space_size + sizeof(void *));
        static_assert(type_traits::type_properties::is_standard_layout_v<storage_t>);

        union {
            storage_t storage{};
            std::max_align_t dummy;
        };
    };
}

namespace rainy::containers {
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
}

namespace rainy::utility {
    template <typename ValueType>
    RAINY_NODISCARD const ValueType *any_cast(const containers::any *const any) noexcept {
        using namespace foundation;
        using namespace type_traits;
        static_assert(!primary_types::is_void_v<ValueType>, "any cannot contain void.");
        if constexpr (primary_types::is_function_v<ValueType> || primary_types::is_array_v<ValueType>) {
            return nullptr;
        } else {
            if (!any) {
                return nullptr;
            }
            return any->cast_to_pointer<cv_modify::remove_cvref_t<ValueType>>();
        }
    }

    template <typename ValueType>
    RAINY_NODISCARD ValueType *any_cast(containers::any *const any) noexcept {
        using namespace foundation;
        using namespace type_traits;
        static_assert(!primary_types::is_void_v<ValueType>, "any cannot contain void.");
        if constexpr (primary_types::is_function_v<ValueType> || primary_types::is_array_v<ValueType>) {
            return nullptr;
        } else {
            if (!any) {
                return nullptr;
            }
            return any->cast_to_pointer<cv_modify::remove_cvref_t<ValueType>>();
        }
    }

    template <typename Ty>
    RAINY_NODISCARD type_traits::cv_modify::remove_cv_t<Ty> any_cast(const containers::any &any) {
        using namespace foundation;
        using namespace type_traits;
        static_assert(type_properties::is_constructible_v<cv_modify::remove_cv_t<Ty>, const cv_modify::remove_cvref_t<Ty> &>,
                      "any_cast<T>(const any&) requires remove_cv_t<T> to be constructible from "
                      "const remove_cv_t<remove_reference_t<T>>&");
        const auto ptr = any_cast<cv_modify::remove_cvref_t<Ty>>(&any);
        if (!ptr) {
            system::exceptions::cast::throw_bad_any_cast();
        } else {
            return static_cast<cv_modify::remove_cv_t<Ty>>(*ptr);
        }
        std::terminate();
    }

    template <typename Ty>
    RAINY_NODISCARD type_traits::cv_modify::remove_cv_t<Ty> any_cast(containers::any &any) {
        using namespace foundation;
        using namespace type_traits;
        static_assert(type_properties::is_constructible_v<cv_modify::remove_cv_t<Ty>, cv_modify::remove_cvref_t<Ty> &>,
                      "any_cast<T>(any&) requires remove_cv_t<T> to be constructible from remove_cv_t<remove_reference_t<T>>&");
        const auto ptr = any_cast<cv_modify::remove_cvref_t<Ty>>(&any);
        if (!ptr) {
            system::exceptions::cast::throw_bad_any_cast();
        } else {
            return static_cast<cv_modify::remove_cv_t<Ty>>(*ptr);
        }
        std::terminate();
    }

    template <typename Ty>
    RAINY_NODISCARD type_traits::cv_modify::remove_cv_t<Ty> any_cast(containers::any &&any) {
        using namespace foundation;
        using namespace type_traits;
        static_assert(type_properties::is_constructible_v<cv_modify::remove_cv_t<Ty>, cv_modify::remove_cvref_t<Ty>>,
                      "any_cast<T>(any&&) requires remove_cv_t<T> to be constructible from remove_cv_t<remove_reference_t<T>>");

        const auto ptr = any_cast<cv_modify::remove_cvref_t<Ty>>(&any);
        if (!ptr) {
            system::exceptions::cast::throw_bad_any_cast();
        } else {
            return static_cast<cv_modify::remove_cv_t<Ty>>(utility::move(*ptr));
        }
        std::terminate();
    }

    template <typename Ty, typename AnyType,
              type_traits::other_trans::enable_if_t<
                  type_traits::type_relations::is_same_v<type_traits::other_trans::decay_t<AnyType>, containers::any>, int> = 0>
    RAINY_NODISCARD decltype(auto) get(AnyType &&any) noexcept {
        return any_cast<Ty>(forward<AnyType>(any));
    }
}

#endif
