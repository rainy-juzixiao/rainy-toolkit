#ifndef RAINY_FOUNDATION_WILLOW_IMPLEMENTS_COMMON_VALUE_HPP
#define RAINY_FOUNDATION_WILLOW_IMPLEMENTS_COMMON_VALUE_HPP
#include <rainy/core/platform.hpp>
#include <rainy/foundation/collections/unordered_map.hpp>
#include <rainy/foundation/willow/implements/common/config.hpp>
#include <rainy/foundation/willow/implements/common/exceptions.hpp>
#include <rainy/foundation/willow/implements/common/nearly_equal.hpp>
#include <variant>

namespace rainy::foundation::willow::implements {
    template <typename BasicDocument>
    struct value_state {
        using key = core::text::string;
        using properties_type = collections::unordered_map<key, typename BasicDocument::string_type>;

        typename BasicDocument::string_type &operator[](const key &key) {
            return properties[key];
        }

        bool contains(const key &key) const {
            return properties.find(key) != properties.end();
        }

        bool empty() const noexcept {
            return properties.empty();
        }

        void clear() {
            properties.clear();
        }

        properties_type properties;
    };

    template <typename BasicDocument>
    struct value {
        using string_type = typename BasicDocument::string_type;
        using char_type = typename BasicDocument::char_type;
        using integer_type = typename BasicDocument::integer_type;
        using float_type = typename BasicDocument::float_type;
        using boolean_type = typename BasicDocument::boolean_type;
        using array_type = typename BasicDocument::array_type;
        using object_type = typename BasicDocument::object_type;
        using state_type = value_state<BasicDocument>;

        value() {
            type = document_type::null;
            data.object = nullptr;
        }

        value(std::nullptr_t) {
            type = document_type::null;
            data.object = nullptr;
        }

        value(const object_type &value) {
            type = document_type::object;
            data.object = create<object_type>(value);
        }

        value(const array_type &value) {
            type = document_type::array;
            data.vector = create<array_type>(value);
        }

        value(const string_type &value) {
            type = document_type::string;
            data.string = create<string_type>(value);
        }

        template <typename CharType>
        value(const CharType *str) {
            type = document_type::string;
            data.string = create<string_type>(str);
        }

        value(const integer_type value) {
            type = document_type::number_integer;
            data.number_integer = value;
        }

        value(const float_type value) {
            type = document_type::number_float;
            data.number_float = value;
        }

        value(const boolean_type value) {
            type = document_type::boolean;
            data.boolean = value;
        }

        value(const document_type value_type) {
            type = value_type;
            switch (type) {
                case document_type::object:
                    data.object = create<object_type>();
                    break;
                case document_type::array:
                    data.vector = create<array_type>();
                    break;
                case document_type::string:
                    data.string = create<string_type>();
                    break;
                case document_type::number_integer:
                    data.number_integer = integer_type(0);
                    break;
                case document_type::number_float:
                    data.number_float = float_type(0.0);
                    break;
                case document_type::boolean:
                    data.boolean = boolean_type(false);
                    break;
                default:
                    data.object = nullptr;
                    break;
            }
        }

        value(value const &other) {
            type = other.type;

            switch (other.type) {
                case document_type::object:
                    data.object = create<object_type>(*other.data.object);
                    break;
                case document_type::array:
                    data.vector = create<array_type>(*other.data.vector);
                    break;
                case document_type::string:
                    data.string = create<string_type>(*other.data.string);
                    break;
                case document_type::number_integer:
                    data.number_integer = other.data.number_integer;
                    break;
                case document_type::number_float:
                    data.number_float = other.data.number_float;
                    break;
                case document_type::boolean:
                    data.boolean = other.data.boolean;
                    break;
                default:
                    data.object = nullptr;
                    break;
            }
            state_ = other.state_ ? create<state_type>(*other.state_) : nullptr;
        }

        value(value &&other) noexcept {
            type = other.type;
            data = other.data;
            state_ = other.state_;
            other.type = document_type::null;
            other.data.object = nullptr;
            other.state_ = nullptr;
        }

        ~value() {
            if (state_) {
                destroy<state_type>(state_);
                state_ = nullptr;
            }
            switch (type) {
                case document_type::object:
                    destroy<object_type>(data.object);
                    break;
                case document_type::array:
                    destroy<array_type>(data.vector);
                    break;
                case document_type::string:
                    destroy<string_type>(data.string);
                    break;
                default:
                    break;
            }
        }

        void swap(value &other) noexcept {
            std::swap(type, other.type);
            std::swap(data, other.data);
            std::swap(state_, other.state_);
        }

        void clear() {
            switch (type) {
                case document_type::object:
                    destroy<object_type>(data.object);
                    break;
                case document_type::array:
                    destroy<array_type>(data.vector);
                    break;
                case document_type::string:
                    destroy<string_type>(data.string);
                    break;
                default:
                    break;
            }
            if (state_) {
                destroy<state_type>(state_);
                state_ = nullptr;
            }
            type = document_type::null;
            data.object = nullptr;
        }

        template <typename Ty, typename... Args>
        static Ty *create(Args &&...args) {
            using allocator_type = typename BasicDocument::template allocator_type<Ty>;
            using allocator_traits = std::allocator_traits<allocator_type>;
            Ty *ptr{nullptr};
            if constexpr (type_traits::properties::is_constructible_v<allocator_type, std::pmr::polymorphic_allocator<void>>) {
                allocator_type base{get_memory_resource()};
                ptr = allocator_traits::allocate(base, 1);
                allocator_traits::construct(base, ptr, utility::forward<Args>(args)...);
            } else {
                allocator_type allocator{};
                ptr = allocator_traits::allocate(allocator, 1);
                allocator_traits::construct(allocator, ptr, utility::forward<Args>(args)...);
            }
            return ptr;
        }

        template <typename Ty>
        static void destroy(Ty *ptr) {
            using allocator_type = typename BasicDocument::template allocator_type<Ty>;
            using allocator_traits = std::allocator_traits<allocator_type>;
            if constexpr (type_traits::properties::is_constructible_v<allocator_type, std::pmr::polymorphic_allocator<void>>) {
                std::pmr::polymorphic_allocator<void> base{get_memory_resource()};
                allocator_type allocator{base};
                allocator_traits::destroy(allocator, ptr);
                allocator_traits::deallocate(allocator, ptr, 1);
            } else {
                allocator_type allocator{};
                allocator_traits::destroy(allocator, ptr);
                allocator_traits::deallocate(allocator, ptr, 1);
            }
        }

        value &operator=(value const &other) {
            if (other.state_) {
                value{other}.swap(*this);
            } else {
                state_type *keep = state_;
                state_ = nullptr;
                value{other}.swap(*this);
                state_ = keep;
            }
            return (*this);
        }

        value &operator=(value &&other) noexcept {
            state_type *keep = state_;
            state_ = nullptr;
            clear();
            type = other.type;
            data = std::move(other.data);
            if (other.state_) {
                state_ = other.state_;
                other.state_ = nullptr;
                if (keep) {
                    destroy<state_type>(keep);
                }
            } else {
                state_ = keep;
            }

            other.type = document_type::null;
            other.data.object = nullptr;
            return (*this);
        }

        bool has_state() const noexcept {
            return state_ != nullptr;
        }

        state_type &state() {
            if (!state_) {
                state_ = create<state_type>();
            }
            return *state_;
        }

        const state_type &state() const noexcept {
            static const state_type empty{};
            return state_ ? *state_ : empty;
        }

        friend bool operator==(const value &lhs, const value &rhs) {
            if (lhs.type == rhs.type) {
                switch (lhs.type) {
                    case document_type::array:
                        return (*lhs.data.vector == *rhs.data.vector);

                    case document_type::object:
                        return (*lhs.data.object == *rhs.data.object);

                    case document_type::null:
                        return true;

                    case document_type::string:
                        return (*lhs.data.string == *rhs.data.string);

                    case document_type::boolean:
                        return (lhs.data.boolean == rhs.data.boolean);

                    case document_type::number_integer:
                        return (lhs.data.number_integer == rhs.data.number_integer);

                    case document_type::number_float:
                        return willow::implements::nearly_equal(lhs.data.number_float, rhs.data.number_float);

                    default:
                        return false;
                }
            } else if (lhs.type == document_type::number_integer && rhs.type == document_type::number_float) {
                return willow::implements::nearly_equal<float_type>(static_cast<float_type>(lhs.data.number_integer),
                                                                    rhs.data.number_float);
            } else if (lhs.type == document_type::number_float && rhs.type == document_type::number_integer) {
                return willow::implements::nearly_equal<float_type>(lhs.data.number_float,
                                                                    static_cast<float_type>(rhs.data.number_integer));
            }
            return false;
        }

        document_type type;
        union {
            object_type *object;
            array_type *vector;
            string_type *string;
            integer_type number_integer;
            float_type number_float;
            boolean_type boolean;
        } data;

    private:
        state_type *state_{nullptr};
    };

    template <typename BasicDocument>
    struct value_getter {
        using string_type = typename BasicDocument::string_type;
        using char_type = typename BasicDocument::char_type;
        using integer_type = typename BasicDocument::integer_type;
        using float_type = typename BasicDocument::float_type;
        using boolean_type = typename BasicDocument::boolean_type;
        using array_type = typename BasicDocument::array_type;
        using object_type = typename BasicDocument::object_type;

        static void assign(const BasicDocument &doc, object_type &value) {
            if (!doc.is_object()) {
                exceptions::willow::throw_willow_type_error("document value type must be object");
            }
            value = *doc.value_.data.object;
        }

        static void assign(const BasicDocument &doc, array_type &value) {
            if (!doc.is_array()) {
                exceptions::willow::throw_willow_type_error("document value type must be array");
            }
            value = *doc.value_.data.vector;
        }

        static void assign(const BasicDocument &doc, string_type &value) {
            if (!doc.is_string()) {
                exceptions::willow::throw_willow_type_error("document value type must be string");
            }
            value = *doc.value_.data.string;
        }

        static void assign(const BasicDocument &doc, boolean_type &value) {
            if (!doc.is_bool()) {
                exceptions::willow::throw_willow_type_error("document value type must be boolean");
            }
            value = doc.value_.data.boolean;
        }

        static void assign(const BasicDocument &doc, integer_type &value) {
            if (!doc.is_integer()) {
                exceptions::willow::throw_willow_type_error("document value type must be integer");
            }
            value = doc.value_.data.number_integer;
        }

        template <typename IntegerType, typename type_traits::other_trans::enable_if_t<std::is_integral_v<IntegerType>, int> = 0>
        static void assign(const BasicDocument &doc, IntegerType &value) {
            if (!doc.is_integer()) {
                exceptions::willow::throw_willow_type_error("document value type must be integer");
            }
            value = static_cast<IntegerType>(doc.value_.data.number_integer);
        }

        static void assign(const BasicDocument &doc, float_type &value) {
            if (!doc.is_float()) {
                exceptions::willow::throw_willow_type_error("document value type must be float");
            }
            value = doc.value_.data.number_float;
        }

        template <typename FloatingType,
                  typename type_traits::other_trans::enable_if_t<std::is_floating_point<FloatingType>::value, int> = 0>
        static void assign(const BasicDocument &doc, FloatingType &value) {
            if (!doc.is_float()) {
                exceptions::willow::throw_willow_type_error("document value type must be float");
            }
            value = static_cast<FloatingType>(doc.value_.data.number_float);
        }
    };
}

#endif
