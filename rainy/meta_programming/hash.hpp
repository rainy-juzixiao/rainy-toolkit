#ifndef RAINY_HASH_HPP
#define RAINY_HASH_HPP
#include <rainy/core.hpp>
#include <rainy/meta_programming/type_traits.hpp>

namespace rainy::utility {
    template <typename key>
    struct hash;

    namespace internals {
        template <typename Key>
        RAINY_AINLINE_NODISCARD std::size_t fnv1a_append_value(const std::size_t offset_basis, const Key &keyval) noexcept {
            static_assert(foundation::type_traits::type_properties::is_trivial_v<Key>, "Only trivial types can be directly hashed.");
            return fnv1a_append_bytes(offset_basis, &reinterpret_cast<const unsigned char &>(keyval), sizeof(Key));
        }

        template <typename Ty>
        RAINY_AINLINE_NODISCARD std::size_t fnv1a_append_range(const std::size_t offset_basis, const Ty *const first,
                                                                  const Ty *const last) {
            static_assert(foundation::type_traits::type_properties::is_trivial_v<Ty>, "Only trivial types can be directly hashed.");
            const auto first_binary = reinterpret_cast<const unsigned char *>(first);
            const auto last_binary = reinterpret_cast<const unsigned char *>(last);
            return fnv1a_append_bytes(offset_basis, first_binary, static_cast<size_t>(last_binary - first_binary));
        }

        template <typename Key>
        RAINY_AINLINE_NODISCARD std::size_t hash_representation(const Key &keyval) noexcept {
            return fnv1a_append_value(fnv_offset_basis, keyval);
        }

        template <typename Key>
        RAINY_AINLINE_NODISCARD std::size_t hash_array_representation(const Key *const first, const std::size_t count) noexcept {
            static_assert(foundation::type_traits::type_properties::is_trivial_v<Key>, "Only trivial types can be directly hashed.");
            return fnv1a_append_bytes(fnv_offset_basis, reinterpret_cast<const unsigned char *>(first), count * sizeof(Key));
        }

        /**
         *
         * @tparam key
         * @tparam check 如果为真，此模板将启用
         */
        template <typename key, bool check>
        struct hash_enable_if {
            using argument_type = key;
            using result_type = std::size_t;

            RAINY_AINLINE_NODISCARD result_type operator()(const argument_type &val) const
                noexcept(noexcept(hash<key>::hash_this_val(val))) {
                return hash<key>::hash_this_val(val);
            }
        };

        template <typename key>
        struct hash_enable_if<key, false> {
            hash_enable_if() = delete;
            hash_enable_if(const hash_enable_if &) = delete;
            hash_enable_if(hash_enable_if &&) = delete;
            hash_enable_if &operator=(const hash_enable_if &) = delete;
            hash_enable_if &operator=(hash_enable_if &&) = delete;
        };
    }

    template <typename key>
    struct hash : internals::hash_enable_if<key, !foundation::type_traits::type_properties::is_const_v<key> &&
                                                     !foundation::type_traits::type_properties::is_volatile_v<key> &&
                                                     (foundation::type_traits::primary_types::is_enum_v<key> ||
                                                      foundation::type_traits::primary_types::is_integral_v<key> ||
                        foundation::type_traits::primary_types::is_pointer_v<key>)> {
        static size_t hash_this_val(const key &keyval) noexcept {
            return internals::hash_representation(keyval);
        }
    };

    template <>
    struct hash<float> {
        using argument_type = float;
        using result_type = std::size_t;


        RAINY_AINLINE_NODISCARD result_type operator()(argument_type val) const {
            return internals::hash_representation(val == 0.0f ? 0.0f : val);
        }
    };

    template <>
    struct hash<double> {
        using argument_type = double;
        using result_type = std::size_t;

        RAINY_AINLINE_NODISCARD result_type operator()(argument_type val) const {
            return internals::hash_representation(val == 0.0 ? 0.0 : val);
        }
    };

    template <>
    struct hash<long double> {
        using argument_type = long double;
        using result_type = std::size_t;

        RAINY_AINLINE_NODISCARD result_type operator()(argument_type val) const {
            return internals::hash_representation(val == 0.0L ? 0.0L : val);
        }
    };

    template <>
    struct hash<std::nullptr_t> {
        using argument_type = std::nullptr_t;
        using result_type = std::size_t;

        RAINY_AINLINE_NODISCARD result_type operator()(std::nullptr_t) const {
            void *null_pointer{};
            return internals::hash_representation(null_pointer);
        }
    };

    template <>
    struct hash<std::string_view> {
        using argument_type = std::string_view;
        using result_type = std::size_t;

        static size_t hash_this_val(const argument_type &val) noexcept {
            return internals::hash_array_representation(val.data(), val.size());
        }

        RAINY_AINLINE_NODISCARD result_type operator()(argument_type val) const {
            return internals::hash_array_representation(val.data(), val.size());
        }
    };

    template <>
    struct hash<std::string> {
        using argument_type = std::string_view;
        using result_type = std::size_t;

        static size_t hash_this_val(const argument_type &val) noexcept {
            return internals::hash_array_representation(val.data(), val.size());
        }

        RAINY_AINLINE_NODISCARD result_type operator()(argument_type val) const {
            return internals::hash_array_representation(val.data(), val.size());
        }
    };

    template <>
    struct hash<type_index> {
        using argument_type = type_index;
        using result_type = std::size_t;

        static size_t hash_this_val(const argument_type &val) noexcept {
            return val.hash_code();
        }


        RAINY_NODISCARD size_t operator()(const type_index &val) const noexcept {
            return val.hash_code();
        }
    };
}

#endif
