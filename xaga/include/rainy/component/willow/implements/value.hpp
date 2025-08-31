#ifndef RAINY_COMPONENT_WILLOW_IMPLEMENTS_VALUE_HPP
#define RAINY_COMPONENT_WILLOW_IMPLEMENTS_VALUE_HPP
#include <variant>
#include <rainy/core/core.hpp>
#include <rainy/component/willow/implements/config.hpp>
#include <rainy/component/willow/implements/exceptions.hpp>

namespace rainy::component::willow::implements {
    template <typename basic_json>
    struct value {
        using string_type = typename basic_json::string_type;
        using char_type = typename basic_json::char_type;
        using integer_type = typename basic_json::integer_type;
        using float_type = typename basic_json::float_type;
        using boolean_type = typename basic_json::boolean_type;
        using array_type = typename basic_json::array_type;
        using object_type = typename basic_json::object_type;

        json_type type;
        union {
            object_type *object;
            array_type *vector;
            string_type *string;
            integer_type number_integer;
            float_type number_float;
            boolean_type boolean;
        } data;

        value() {
            type = json_type::null;
            data.object = nullptr;
        }

        value(std::nullptr_t) {
            type = json_type::null;
            data.object = nullptr;
        }

        value(const object_type &value) {
            type = json_type::object;
            data.object = create<object_type>(value);
        }

        value(const array_type &value) {
            type = json_type::array;
            data.vector = create<array_type>(value);
        }

        value(const string_type &value) {
            type = json_type::string;
            data.string = create<string_type>(value);
        }

        template <typename _CharT>
        value(const _CharT *str) {
            type = json_type::string;
            data.string = create<string_type>(str);
        }

        value(const integer_type value) {
            type = json_type::number_integer;
            data.number_integer = value;
        }

        value(const float_type value) {
            type = json_type::number_float;
            data.number_float = value;
        }

        value(const boolean_type value) {
            type = json_type::boolean;
            data.boolean = value;
        }

        value(const json_type value_type) {
            type = value_type;
            switch (type) {
                case json_type::object:
                    data.object = create<object_type>();
                    break;
                case json_type::array:
                    data.vector = create<array_type>();
                    break;
                case json_type::string:
                    data.string = create<string_type>();
                    break;
                case json_type::number_integer:
                    data.number_integer = integer_type(0);
                    break;
                case json_type::number_float:
                    data.number_float = float_type(0.0);
                    break;
                case json_type::boolean:
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
                case json_type::object:
                    data.object = create<object_type>(*other.data.object);
                    break;
                case json_type::array:
                    data.vector = create<array_type>(*other.data.vector);
                    break;
                case json_type::string:
                    data.string = create<string_type>(*other.data.string);
                    break;
                case json_type::number_integer:
                    data.number_integer = other.data.number_integer;
                    break;
                case json_type::number_float:
                    data.number_float = other.data.number_float;
                    break;
                case json_type::boolean:
                    data.boolean = other.data.boolean;
                    break;
                default:
                    data.object = nullptr;
                    break;
            }
        }

        value(value &&other) noexcept {
            type = other.type;
            data = other.data;
            other.type = json_type::null;
            other.data.object = nullptr;
        }

        ~value() {
            clear();
        }

        void swap(value &other) noexcept {
            std::swap(type, other.type);
            std::swap(data, other.data);
        }

        void clear() {
            switch (type) {
                case json_type::object:
                    destroy<object_type>(data.object);
                    break;
                case json_type::array:
                    destroy<array_type>(data.vector);
                    break;
                case json_type::string:
                    destroy<string_type>(data.string);
                    break;
                default:
                    break;
            }
        }

        template <typename Ty, typename... Args>
        static Ty *create(Args &&...args) {
            using allocator_type = typename basic_json::template allocator_type<Ty>;
            using allocator_traits = std::allocator_traits<allocator_type>;
            Ty *ptr{nullptr};
            if constexpr (type_traits::type_properties::is_constructible_v<allocator_type, std::pmr::polymorphic_allocator<void>>) {
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
            using allocator_type = typename basic_json::template allocator_type<Ty>;
            using allocator_traits = std::allocator_traits<allocator_type>;
            if constexpr (type_traits::type_properties::is_constructible_v<allocator_type, std::pmr::polymorphic_allocator<void>>) {
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
            value{other}.swap(*this);
            return (*this);
        }

        value &operator=(value &&other) noexcept {
            clear();
            type = other.type;
            data = std::move(other.data);
            // invalidate payload
            other.type = json_type::null;
            other.data.object = nullptr;
            return (*this);
        }

        friend bool operator==(const value &lhs, const value &rhs) {
            if (lhs.type == rhs.type) {
                switch (lhs.type) {
                    case json_type::array:
                        return (*lhs.data.vector == *rhs.data.vector);

                    case json_type::object:
                        return (*lhs.data.object == *rhs.data.object);

                    case json_type::null:
                        return true;

                    case json_type::string:
                        return (*lhs.data.string == *rhs.data.string);

                    case json_type::boolean:
                        return (lhs.data.boolean == rhs.data.boolean);

                    case json_type::number_integer:
                        return (lhs.data.number_integer == rhs.data.number_integer);

                    case json_type::number_float:
                        return utils::nearly_equal(lhs.data.number_float, rhs.data.number_float);

                    default:
                        return false;
                }
            } else if (lhs.type == json_type::number_integer && rhs.type == json_type::number_float) {
                return utils::nearly_equal<float_type>(static_cast<float_type>(lhs.data.number_integer), rhs.data.number_float);
            } else if (lhs.type == json_type::number_float && rhs.type == json_type::number_integer) {
                return utils::nearly_equal<float_type>(lhs.data.number_float, static_cast<float_type>(rhs.data.number_integer));
            }
            return false;
        }
    };

    template <typename BasicJson>
    struct value_getter {
        using string_type = typename BasicJson::string_type;
        using char_type = typename BasicJson::char_type;
        using integer_type = typename BasicJson::integer_type;
        using float_type = typename BasicJson::float_type;
        using boolean_type = typename BasicJson::boolean_type;
        using array_type = typename BasicJson::array_type;
        using object_type = typename BasicJson::object_type;

        static void assign(const BasicJson &json, object_type &value) {
            if (!json.is_object()) {
                foundation::exceptions::willow::throw_json_type_error("json value type must be object");
            }
            value = std::get<object_type>(json.value_.data);
        }

        static void assign(const BasicJson &json, array_type &value) {
            if (!json.is_array()) {
                foundation::exceptions::willow::throw_json_type_error("json value type must be array");
            }
            value = std::get<array_type>(json.value_.data);
        }

        static void assign(const BasicJson &json, string_type &value) {
            if (!json.is_string()) {
                foundation::exceptions::willow::throw_json_type_error("json value type must be string");
            }
            value = std::get<string_type>(json.value_.data);
        }

        static void assign(const BasicJson &json, boolean_type &value) {
            if (!json.is_bool()) {
                foundation::exceptions::willow::throw_json_type_error("json value type must be boolean");
            }
            value = std::get<boolean_type>(json.value_.data);
        }

        static void assign(const BasicJson &json, integer_type &value) {
            if (!json.is_integer()) {
                foundation::exceptions::willow::throw_json_type_error("json value type must be integer");
            }
            value = std::get<integer_type>(json.value_.data);
        }

        template <typename IntegerType, typename std::enable_if_t<std::is_integral_v<IntegerType>, int> = 0>
        static void assign(const BasicJson &json, IntegerType &value) {
            if (!json.is_integer()) {
                foundation::exceptions::willow::throw_json_type_error("json value type must be integer");
            }
            value = static_cast<IntegerType>(std::get<integer_type>(json.value_.data));
        }

        static void assign(const BasicJson &json, float_type &value) {
            if (!json.is_float()) {
                foundation::exceptions::willow::throw_json_type_error("json value type must be float");
            }
            value = std::get<float_type>(json.value_.data);
        }

        template <typename FloatingType, typename std::enable_if_t<std::is_floating_point<FloatingType>::value, int> = 0>
        static void assign(const BasicJson &json, FloatingType &value) {
            if (!json.is_float()) {
                foundation::exceptions::willow::throw_json_type_error("json value type must be float");
            }
            value = static_cast<FloatingType>(std::get<float_type>(json.value_.data));
        }
    };
}

#endif