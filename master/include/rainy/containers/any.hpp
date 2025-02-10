#ifndef RAINY_ANY_HPP
#define RAINY_ANY_HPP
#include <rainy/core/core.hpp>
#include <rainy/foundation/system/basic_exception.hpp>
#include <rainy/meta/type_traits.hpp>
#include <any>
#include <utility>

/*
 * any的基础utils和异常实现
 */
namespace rainy::containers::internals {
    inline constexpr size_t any_trivial_space_size = (information::small_object_num_ptrs - 1) * sizeof(void *);

    template <typename Ty>
    inline constexpr bool any_is_trivial =
        alignof(Ty) <= alignof(max_align_t) && std::is_trivially_copyable_v<Ty> && sizeof(Ty) <= any_trivial_space_size;

    inline constexpr size_t any_small_space_size = (information::small_object_num_ptrs - 2) * sizeof(void *);

    template <typename Ty>
    inline constexpr bool any_is_small =
        alignof(Ty) <= alignof(max_align_t) && std::is_nothrow_move_constructible_v<Ty> && sizeof(Ty) <= any_small_space_size;

    enum class any_representation : uintptr_t {
        trivial,
        big,
        _small
    };

    struct big_any_sti_manager {
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

    struct small_any_sti_manager {
        using destroy_fn = void(void *target) noexcept;
        using copy_fn = void(void *target, const void *source);
        using move_fn = void(void *target, void *source) noexcept;

        template <typename Ty>
        static void destroy(void *const target) noexcept {
            std::destroy_at(static_cast<Ty *>(target));
        }

        template <typename Ty>
        static void copy(void *const target, const void *const source) {
            utility::construct_at(static_cast<Ty *>(target), *static_cast<const Ty *>(source));
        }

        template <typename Ty>
        static void move(void *const target, void *const source) noexcept {
            utility::construct_at(static_cast<Ty *>(target), std::move(*static_cast<Ty *>(source)));
        }

        destroy_fn *destroy_;
        copy_fn *copy_;
        move_fn *move_;
    };

    template <typename Ty>
    inline constexpr big_any_sti_manager any_big_sti_manager_object = {&big_any_sti_manager::destory<Ty>,
                                                                       &big_any_sti_manager::copy<Ty>};

    template <typename Ty>
    inline constexpr small_any_sti_manager any_small_sti_manager_object = {
        &small_any_sti_manager::destroy<Ty>, &small_any_sti_manager::copy<Ty>, &small_any_sti_manager::move<Ty>};
}

namespace rainy::foundation::system::exceptions::cast {
    class bad_any_cast final : public bad_cast {
    public:
        using base = bad_cast;

        explicit bad_any_cast(const source &location = source::current()) : base(location.to_string() + " : Bad any_cast : ") {
        }
    };

    void throw_bad_any_cast() {
        utility::throw_exception(bad_any_cast{});
    }
}

/*
 * any实现
 */
namespace rainy::containers {
    class any { // storage for any (CopyConstructible) type
    public:
        constexpr any() noexcept {
        }

        any(const any &right) {
            using namespace internals;
            storage.type_data = right.storage.type_data;
            switch (get_representation()) {
                case any_representation::_small:
                    storage._SmallStorage.sti_manager = right.storage._SmallStorage.sti_manager;
                    storage._SmallStorage.sti_manager->copy_(&storage._SmallStorage._Data, &right.storage._SmallStorage._Data);
                    break;
                case any_representation::big:
                    storage._BigStorage.sti_manager = right.storage._BigStorage.sti_manager;
                    storage._BigStorage._Ptr = storage._BigStorage.sti_manager->copy_(right.storage._BigStorage._Ptr);
                    break;
                case any_representation::trivial:
                default:
                    std::memcpy(storage._TrivialData, right.storage._TrivialData, sizeof(storage._TrivialData));
                    break;
            }
        }

        any(any &&right) noexcept {
            _Move_from(right);
        }

        template <typename _ValueType, std::enable_if_t<std::conjunction_v<std::negation<std::is_same<std::decay_t<_ValueType>, any>>,
                                                              std::negation<type_traits::primary_types::is_specialization<std::decay_t<_ValueType>, std::in_place_type_t>>,
                                                              std::is_copy_constructible<std::decay_t<_ValueType>>>,
                                                int> = 0>
        any(_ValueType &&_Value) {
            emplace_<std::decay_t<_ValueType>>(utility:: forward<_ValueType>(_Value));
        }

        template <
            class _ValueType, typename... Types,
            std::enable_if_t<std::conjunction_v<std::is_constructible<std::decay_t<_ValueType>, Types...>, std::is_copy_constructible<std::decay_t<_ValueType>>>,
                        int> = 0>
        explicit any(std::in_place_type_t<_ValueType>, Types &&..._Args) {
            emplace_<std::decay_t<_ValueType>>(utility:: forward<Types>(_Args)...);
        }

        template <typename _ValueType, class _Elem, typename... Types, std::enable_if_t < std::conjunction_v<std::is_constructible<std::decay_t<_ValueType>, std::initializer_list<_Elem> &, Types...>,
                                            std::is_copy_constructible<std::decay_t<_ValueType>>>,
                              int> = 0>
        explicit any(std::in_place_type_t<_ValueType>, std::initializer_list<_Elem> _Ilist, Types &&..._Args) {
            emplace_<std::decay_t<_ValueType>>(_Ilist, utility:: forward<Types>(_Args)...);
        }

        ~any() noexcept {
            reset();
        }

        // Assignment [any.assign]
        any &operator=(const any &right) {
            assign_(right);
            return *this;
        }

        any &operator=(any &&right) noexcept {
            assign_(utility:: move(right));
            return *this;
        }

        template <typename _ValueType, std::enable_if_t<std::conjunction_v<std::negation<std::is_same<std::decay_t<_ValueType>, any>>,
                                                                        std::is_copy_constructible<std::decay_t<_ValueType>>>,
                              int> = 0>
        any &operator=(_ValueType &&_Value) {
            // replace contained value with an object of type decay_t<_ValueType> initialized from _Value
            assign_(utility:: forward<_ValueType>(_Value));
            return *this;
        }

        // Modifiers [any.modifiers]
        template <
            class _ValueType, typename... Types,
                  std::enable_if_t<std::conjunction_v<std::is_constructible<std::decay_t<_ValueType>, Types...>,
                                                      std::is_copy_constructible<std::decay_t<_ValueType>>>,
                        int> = 0>
        std::decay_t<_ValueType> &emplace(Types &&..._Args) {
            // replace contained value with an object of type decay_t<_ValueType> initialized from _Args...
            reset();
            return emplace_<std::decay_t<_ValueType>>(utility:: forward<Types>(_Args)...);
        }
        template <typename _ValueType, class _Elem, typename... Types,
                  std::enable_if_t<
                      std::conjunction_v<std::is_constructible<std::decay_t<_ValueType>, std::initializer_list<_Elem> &, Types...>,
                                         std::is_copy_constructible<std::decay_t<_ValueType>>>,
                              int> = 0>
        std::decay_t<_ValueType> &emplace(std::initializer_list<_Elem> _Ilist, Types &&..._Args) {
            // replace contained value with an object of type decay_t<_ValueType> initialized from _Ilist and _Args...
            reset();
            return emplace_<std::decay_t<_ValueType>>(_Ilist, utility:: forward<Types>(_Args)...);
        }

        void reset() noexcept { // transition to the empty state
            using namespace internals;
            switch (get_representation()) {
                case any_representation::_small:
                    storage._SmallStorage.sti_manager->destroy_(&storage._SmallStorage._Data);
                    break;
                case any_representation::big:
                    storage._BigStorage.sti_manager->destory_(storage._BigStorage._Ptr);
                    break;
                case any_representation::trivial:
                default:
                    break;
            }
            storage.type_data = 0;
        }

        void swap(any &right) noexcept {
            right = utility:: exchange(*this, utility:: move(right));
        }

        // Observers [any.observers]
        RAINY_NODISCARD bool has_value() const noexcept {
            return storage.type_data != 0;
        }

        RAINY_NODISCARD const foundation::typeinfo &type() const noexcept {
            // if *this contains a value of type T, return sti_typeid(T); otherwise sti_typeid(void)
            const foundation::typeinfo *const info = type_info();
            if (info) {
                return *info;
            }
            return sti_typeid(void);
        }

        template <typename Decayed>
        RAINY_NODISCARD const Decayed *cast_to_pointer() const noexcept {
            // if *this contains a value of type Decayed, return a pointer to it
            const foundation::typeinfo *const info = type_info();
            if (!info || *info != sti_typeid(Decayed)) {
                return nullptr;
            }
            if constexpr (internals::any_is_trivial<Decayed>) {
                // get a pointer to the contained _Trivial value of type Decayed
                return reinterpret_cast<const Decayed *>(&storage._TrivialData);
            } else if constexpr (internals::any_is_small<Decayed>) {
                // get a pointer to the contained _Small value of type Decayed
                return reinterpret_cast<const Decayed *>(&storage._SmallStorage._Data);
            } else {
                // get a pointer to the contained _Big value of type Decayed
                return static_cast<const Decayed *>(storage._BigStorage._Ptr);
            }
        }

        template <typename Decayed>
        RAINY_NODISCARD const Decayed &as() const noexcept {
            return cast_to_pointer<Decayed>();
        }

        template <typename Decayed>
        RAINY_NODISCARD Decayed *cast_to_pointer() noexcept { // if *this contains a value of type Decayed, return a pointer to it
            return const_cast<Decayed *>(static_cast<const any *>(this)->cast_to_pointer<Decayed>());
        }

    private:
        static constexpr std::uintptr_t rep_mask = 3;

        RAINY_NODISCARD internals::any_representation get_representation()
            const noexcept { // extract the representation format from type_data
            return static_cast<internals::any_representation>(storage.type_data & rep_mask);
        }
        
        RAINY_NODISCARD const foundation::typeinfo *type_info() const noexcept { // extract the foundation::typeinfo from type_data
            return reinterpret_cast<const foundation::typeinfo *>(storage.type_data & ~rep_mask);
        }

        void _Move_from(any &right) noexcept {
            using namespace internals;
            storage.type_data = right.storage.type_data;
            switch (get_representation()) {
                case any_representation::_small:
                    storage._SmallStorage.sti_manager = right.storage._SmallStorage.sti_manager;
                    storage._SmallStorage.sti_manager->move_(&storage._SmallStorage._Data, &right.storage._SmallStorage._Data);
                    break;
                case any_representation::big:
                    storage._BigStorage.sti_manager = right.storage._BigStorage.sti_manager;
                    storage._BigStorage._Ptr = right.storage._BigStorage._Ptr;
                    right.storage.type_data = 0;
                    break;
                case any_representation::trivial:
                default:
                    std::memcpy(storage._TrivialData, right.storage._TrivialData, sizeof(storage._TrivialData));
                    break;
            }
        }

        void assign_(any right) noexcept { // intentionally pass by value
            reset();
            _Move_from(right);
        }

        template <typename Decayed, typename... Types>
        Decayed &emplace_(Types &&..._Args) { // emplace construct Decayed
            if constexpr (internals::any_is_trivial<Decayed>) {
                // using the _Trivial representation
                auto &object = reinterpret_cast<Decayed &>(storage._TrivialData);
                //::new (utility::addressof(object)) Decayed(utility::forward<Types>(_Args)...);
                rainy::utility::construct_in_place(object, utility::forward<Types>(_Args)...);
                storage.type_data =
                    reinterpret_cast<std::uintptr_t>(&sti_typeid(Decayed)) | static_cast<std::uintptr_t>(internals::any_representation::trivial);
                return object;
            } else if constexpr (internals::any_is_small<Decayed>) {
                // using the _Small representation
                auto &object = reinterpret_cast<Decayed &>(storage._SmallStorage._Data);
                //::new (utility::addressof(object)) Decayed(utility::forward<Types>(_Args)...);
                rainy::utility::construct_in_place(object, utility::forward<Types>(_Args)...);
                storage._SmallStorage.sti_manager = &internals::any_small_sti_manager_object<Decayed>;
                storage.type_data =
                    reinterpret_cast<std::uintptr_t>(&sti_typeid(Decayed)) | static_cast<std::uintptr_t>(internals::any_representation::_small);
                return object;
            } else {
                // using the _Big representation
                Decayed *const new_alloc_ptr = ::new Decayed(utility:: forward<Types>(_Args)...);
                storage._BigStorage._Ptr = new_alloc_ptr;
                storage._BigStorage.sti_manager = &internals::any_big_sti_manager_object<Decayed>;
                storage.type_data = reinterpret_cast<std::uintptr_t>(&sti_typeid(Decayed)) |
                                    static_cast<std::uintptr_t>(internals::any_representation::big);
                return *new_alloc_ptr;
            }
        }

        struct _Smallstorage_t {
            unsigned char _Data[internals::any_small_space_size];
            const internals::small_any_sti_manager *sti_manager;
        };
        static_assert(sizeof(_Smallstorage_t) == internals::any_trivial_space_size);

        struct _Bigstorage_t {
            // Pad so that _Ptr and sti_manager might share type_data's cache line
            unsigned char _Padding[internals::any_small_space_size - sizeof(void *)];
            void *_Ptr;
            const internals::big_any_sti_manager *sti_manager;
        };
        static_assert(sizeof(_Bigstorage_t) == internals::any_trivial_space_size);

        struct storage_t {
            union {
                unsigned char _TrivialData[internals::any_trivial_space_size];
                _Smallstorage_t _SmallStorage;
                _Bigstorage_t _BigStorage;
            };
            uintptr_t type_data;
        };
        static_assert(sizeof(storage_t) == internals::any_trivial_space_size + sizeof(void *));
        static_assert(std::is_standard_layout_v<storage_t>);

        union {
            storage_t storage{};
            std::max_align_t dummy;
        };
    };

    template <typename Ty, typename... Args,
              typename = std::enable_if_t<std::is_constructible_v<any, std::in_place_type_t<Ty>, Args...>, int>>
    RAINY_NODISCARD any make_any(Args &&...args) {
        return any{std::in_place_type<Ty>, std::forward<Args>(args)...};
    }

    template <typename Ty, typename U, typename... Args,
              typename = std::enable_if_t<std::is_constructible_v<any, std::in_place_type_t<Ty>, std::initializer_list<U> &, Args...>>>
    RAINY_NODISCARD any make_any(std::initializer_list<U> initializer_list, Args &&...args) {
        return any{std::in_place_type<Ty>, initializer_list, std::forward<Args>(args)...};
    }
}

namespace rainy::utility {
    template <typename ValueType>
    RAINY_NODISCARD const ValueType *any_cast(const containers::any *const any) noexcept {
        // retrieve a pointer to the ValueType contained in _Any, or null
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
        // retrieve a pointer to the ValueType contained in _Any, or null
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
                  type_traits::type_relations::is_same_v<std::decay_t<AnyType>, containers::any>, int> = 0>
    RAINY_NODISCARD decltype(auto) get(AnyType &&any) noexcept {
        return any_cast<Ty>(forward<AnyType>(any));
    }
}

#endif //RAINY_ANY_HPP
