#ifndef RAINY_ANY_HPP
#define RAINY_ANY_HPP
#include <rainy/core.hpp>
#include <rainy/system/basic_exceptions.hpp>
#include <rainy/meta/type_traits.hpp>

/*
 * any的基础utils和异常实现
 */
namespace rainy {
    namespace containers::internals {
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

        struct big_any_rtti {
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

        struct small_any_rtti {
            using destroy_fn = void(void *target) noexcept;
            using copy_fn = void(void *target, const void *source);
            using move_fn = void(void *target, void *source) noexcept;

            template <typename Ty>
            static void destroy(void *const target) noexcept {
                std::destroy_at(*static_cast<Ty *>(target));
            }

            template <typename Ty>
            static void copy(void *const target, const void *const source) {
                utility::construct_at(*static_cast<Ty *>(target), *static_cast<const Ty *>(source));
            }

            template <typename Ty>
            static void move(void *const target, void *const source) noexcept {
                utility::construct_at(*static_cast<Ty *>(target), std::move(*static_cast<Ty *>(source)));
            }

            destroy_fn *destroy_;
            copy_fn *copy_;
            move_fn *move_;
        };

        template <typename Ty>
        inline constexpr big_any_rtti any_big_rtti_object = {&big_any_rtti::destory<Ty>, &big_any_rtti::copy<Ty>};

        template <typename Ty>
        inline constexpr small_any_rtti any_small_rtti_object = {&small_any_rtti::destroy<Ty>, &small_any_rtti::copy<Ty>,
                                                                &small_any_rtti::move<Ty>};
    }

    namespace foundation::system::exceptions::cast {
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
}

/*
 * any实现
 */
namespace rainy::containers {
    class any {
    public:
        template <typename Ty>
        using _enable_if = std::enable_if_t<
            std::conjunction_v<std::negation<std::is_same<std::decay_t<Ty>, any>>,
                               std::negation<type_traits::primary_types::has_specialization<std::decay_t<Ty>, std::in_place_type_t>>,
                               std::is_copy_constructible<std::decay_t<Ty>>>,
            int>;

        constexpr any() noexcept {
        }

        any(any &&right) noexcept {
            move_(right);
        }

        any(const any &right) {
            copy_(right);
        }

        template <typename Ty, _enable_if<Ty> = 0>
        any(Ty &&right) {
            emplace_<std::decay_t<Ty>>(std::forward<Ty>(right));
        }

        template <typename Ty, typename... Args,
                  typename = std::enable_if_t<
                      std::conjunction_v<std::is_constructible<std::decay_t<Ty>, Args...>, std::is_copy_constructible<std::decay_t<Ty>>>>>
        explicit any(std::in_place_type_t<Ty>, Args &&...args) {
            emplace_<std::decay_t<Ty>>(std::forward<Args...>(args)...);
        }

        template <typename Ty, typename... Args,
                  typename = std::enable_if_t<
                      std::conjunction_v<std::is_constructible<std::decay_t<Ty>, Args...>, std::is_copy_constructible<std::decay_t<Ty>>>>>
        explicit any(utility::placeholder_type_t<Ty>, Args &&...args) {
            emplace_<std::decay_t<Ty>>(std::forward<Args...>(args)...);
        }

        template <
            typename Ty, typename U, typename... Args,
            typename = std::enable_if_t<std::conjunction_v<std::is_copy_constructible<std::decay_t<Ty>>,
                                                           std::is_constructible<std::decay_t<Ty>, std::initializer_list<U> &, Args...>>>>
        explicit any(std::in_place_type_t<Ty>, std::initializer_list<U> initializer_list, Args &&...args) {
            emplace_<std::decay_t<Ty>>(initializer_list, std::forward<Args...>(args)...);
        }

        template <
            typename Ty, typename U, typename... Args,
            typename = std::enable_if_t<std::conjunction_v<std::is_copy_constructible<std::decay_t<Ty>>,
                                                           std::is_constructible<std::decay_t<Ty>, std::initializer_list<U> &, Args...>>>>
        explicit any(utility::placeholder_type_t<Ty>, std::initializer_list<U> initializer_list, Args &&...args) {
            emplace_<std::decay_t<Ty>>(initializer_list, std::forward<Args...>(args)...);
        }

        ~any() {
            reset();
        }

        void reset() noexcept {
            switch (rep_()) {
                case internals::any_representation::big:
                    this->storage.big_storage.rtti->destory_(this->storage.big_storage.ptr);
                    break;
                case internals::any_representation::_small:
                    this->storage.small_storage.rtti->destroy_(this->storage.small_storage.data);
                    break;
                case internals::any_representation::trivial:
                default:
                    break;
            }
            storage.type_data = 0;
        }

        any &operator=(const any &right) {
            copy_(right);
            return *this;
        }

        any &operator=(any &&right) noexcept {
            move_(right);
            return *this;
        }

        template <typename Ty, _enable_if<Ty> = 0>
        any &operator=(Ty &&right) {
            assign_(right);
            return *this;
        }

        template <typename decayed>
        RAINY_NODISCARD decayed *cast_to_pointer() noexcept {
            return const_cast<decayed *>(static_cast<const any *>(this)->cast_to_pointer<decayed>());
        }

        template <typename decayed>
        RAINY_NODISCARD const decayed *cast_to_pointer() const noexcept {
            if (const std::type_info *type_info = type_info_(); !type_info || *type_info != typeid(decayed)) {
                return nullptr;
            }
            using export_type = const decayed *;
            if constexpr (internals::any_is_small<decayed>) {
                return reinterpret_cast<export_type>(&storage.small_storage.data);
            } else if (internals::any_is_trivial<decayed>) {
                return reinterpret_cast<export_type>(&storage.trivial_data);
            } else {
                return static_cast<export_type>(storage.big_storage.ptr);
            }
        }

        RAINY_NODISCARD const std::type_info &type() const noexcept {
            return *type_info_();
        }

        RAINY_NODISCARD bool has_value() const noexcept {
            return static_cast<bool>(this->storage.type_data);
        }

        void swap(any &that) noexcept {
            that = std::exchange(*this, std::move(that));
        }

        template <typename decayed>
        RAINY_NODISCARD decayed &as() {
            decayed *extract_data = cast_to_pointer<decayed>();
            if (extract_data) {
                return *extract_data;
            }
            static char placeholder = '\0'; // 占位，此值永远不会被返回
            return foundation::system::exceptions::cast::throw_bad_any_cast(), reinterpret_cast<decayed &>(placeholder);
        }

        template <typename decayed>
        RAINY_NODISCARD const decayed &as() const {
            const decayed *extract_data = cast_to_pointer<const decayed>();
            if (extract_data) {
                return *extract_data;
            }
            static char placeholder = '\0';
            return foundation::system::exceptions::cast::throw_bad_any_cast(), reinterpret_cast<const decayed &>(placeholder);
        }

    private:
        static constexpr uintptr_t rep_mask = 3;

        void copy_(const any &that) {
            /* 复制要拷贝的any的type_info数据 */
            this->storage.type_data = that.storage.type_data;
            switch (rep_()) {
                case internals::any_representation::big:
                    this->storage.big_storage.rtti = that.storage.big_storage.rtti;
                    this->storage.big_storage.ptr = storage.big_storage.rtti->copy_(that.storage.big_storage.ptr);
                    break;
                case internals::any_representation::_small:
                    this->storage.small_storage.rtti = that.storage.small_storage.rtti;
                    this->storage.small_storage.rtti->copy_(&storage.small_storage.data, &that.storage.small_storage.data);
                    break;
                case internals::any_representation::trivial:
                default:
                    std::memcpy(storage.trivial_data, that.storage.trivial_data, sizeof(storage.trivial_data));
                    break;
            }
        }

        void move_(any &that) noexcept {
            this->storage.type_data = that.storage.type_data;
            switch (rep_()) {
                case internals::any_representation::big:
                    this->storage.big_storage.rtti = that.storage.big_storage.rtti;
                    this->storage.big_storage.ptr = that.storage.big_storage.ptr;
                    that.storage.type_data = 0;
                    // 在设定中，析构函数往往调用reset，reset检查type_data，如果type_data为0，那么，reset将会跳过回收，因此，这里设置type_data为0
                    break;
                case internals::any_representation::_small:
                    this->storage.small_storage.rtti = that.storage.small_storage.rtti;
                    this->storage.small_storage.rtti->move_(this->storage.small_storage.data, &this->storage.small_storage.data);
                    break;
                case internals::any_representation::trivial:
                default:
                    std::memcpy(storage.trivial_data, that.storage.trivial_data, sizeof(storage.trivial_data));
                    break;
            }
        }

        void assign_(any right) {
            reset();
            move_(right);
        }

        template <typename decayed, typename... Args>
        decayed &emplace_(Args &&...args) {
            if constexpr (internals::any_is_trivial<decayed>) {
                auto &obj = reinterpret_cast<decayed &>(storage.trivial_data);
                utility::construct_at(&obj, std::forward<Args>(args)...);
                storage.type_data =
                    reinterpret_cast<uintptr_t>(&typeid(decayed)) | static_cast<uintptr_t>(internals::any_representation::trivial);
                return obj;
            } else if constexpr (internals::any_is_small<decayed>) {
                auto &obj = reinterpret_cast<decayed &>(storage.small_storage.data);
                utility::construct_at(&obj, std::forward<Args>(args)...);
                storage.small_storage.rtti = &internals::any_small_rtti_object<decayed>;
                storage.type_data =
                    reinterpret_cast<uintptr_t>(&typeid(decayed)) | static_cast<uintptr_t>(internals::any_representation::_small);
                return obj;
            } else {
                rainy_let ptr = ::new decayed(std::forward<Args>(args)...);
                storage.big_storage.ptr = ptr;
                storage.big_storage.rtti = &internals::any_big_rtti_object<decayed>;
                storage.type_data =
                    reinterpret_cast<uintptr_t>(&typeid(decayed)) | static_cast<uintptr_t>(internals::any_representation::big);
                return *ptr;
            }
        }

        RAINY_NODISCARD internals::any_representation rep_() const noexcept {
            return static_cast<internals::any_representation>(storage.type_data & rep_mask);
        }

        RAINY_NODISCARD const std::type_info *type_info_() const noexcept {
            return storage.type_data == 0 ? &typeid(void) : reinterpret_cast<const std::type_info *>(storage.type_data & ~rep_mask);
        }

        struct small_storage_t {
            unsigned char data[internals::any_small_space_size];
            const internals::small_any_rtti *rtti;
        };

        static_assert(sizeof(small_storage_t) == internals::any_trivial_space_size);

        struct big_storage_t {
            unsigned char padding[internals::any_small_space_size - sizeof(void *)];
            void *ptr;
            const internals::big_any_rtti *rtti;
        };

        static_assert(sizeof(big_storage_t) == internals::any_trivial_space_size);

        struct storage_t {
            union {
                unsigned char trivial_data[internals::any_trivial_space_size];
                small_storage_t small_storage;
                big_storage_t big_storage;
            };

            uintptr_t type_data;
        };

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
