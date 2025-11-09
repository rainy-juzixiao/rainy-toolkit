#ifndef RAINY_COMPONENT_WILLOW_BASIC_JSON_HPP
#define RAINY_COMPONENT_WILLOW_BASIC_JSON_HPP
#include <rainy/component/willow/json_impl.hpp>

namespace rainy::component::willow {
    template <template <typename Key, typename Ty, typename... Args> typename ObjectType,
              template <typename Key, typename... Args> typename ArrayType,
              typename StringType,
              typename IntegerType, typename FloatingType, typename BooleanType, template <typename Ty> typename Alloc>
    class basic_json {
    public:
        friend class iterators::json_iterator<basic_json>;
        friend class iterators::json_iterator<const basic_json>;
        friend struct implements::json_serializer<basic_json>;
        friend struct implements::json_parser<basic_json>;
        friend struct implements::value_getter<basic_json>;

        template <typename Ty>
        using allocator_type = Alloc<Ty>;
        using size_type = std::size_t;
        using difference_type = std::ptrdiff_t;
        using char_type = typename StringType::value_type; 
        using string_type = typename type_traits::extras::templates::replace_last_correct<StringType,allocator_type<char_type>>::type;
        using integer_type = IntegerType;
        using float_type = FloatingType;
        using boolean_type = BooleanType;
        using array_type = ArrayType<basic_json, allocator_type<basic_json>>;
        using object_type = typename type_traits::extras::templates::replace_last_correct<ObjectType<string_type, basic_json>,allocator_type<std::pair<const string_type, basic_json>>>::type;
        using initializer_list = std::initializer_list<basic_json>;
        using iterator = iterators::json_iterator<basic_json>;
        using const_iterator = iterators::json_iterator<const basic_json>;
        using reverse_iterator = utility::reverse_iterator<iterator>;
        using const_reverse_iterator = utility::reverse_iterator<const_iterator>;
        using dump_arguments = serializer_args<basic_json>;

        basic_json() noexcept = default;

        basic_json(std::nullptr_t) {
        }

        basic_json(const json_type type) : value_(type) {
        }

        basic_json(basic_json const &other) : value_(other.value_) {
        }

        basic_json(basic_json &&other) noexcept : value_(utility::move(other.value_)) {        }

        basic_json(string_type const &value) : value_(value) {
        }

        template <typename StringViewLike, type_traits::other_trans::enable_if_t<
                                               type_traits::type_properties::is_constructible_v<string_type, StringViewLike>, int> = 0>
        basic_json(const StringViewLike &value) : value_{string_type{value}} {
        }

        basic_json(array_type const &arr) : value_(arr) {
        }

        basic_json(object_type const &object) : value_(object) {
        }

        basic_json(integer_type value) : value_(value) {
        }

        template <typename IntegerUTy, typename std::enable_if<std::is_integral<IntegerUTy>::value, int>::type = 0>
        basic_json(IntegerUTy value) : value_(static_cast<integer_type>(value)) {
        }

        basic_json(float_type value) : value_(value) {
        }

        template <typename FloatingUTy, typename std::enable_if<std::is_floating_point<FloatingUTy>::value, int>::type = 0>
        basic_json(FloatingUTy value) : value_(static_cast<float_type>(value)) {
        }

        basic_json(boolean_type value) : value_(value) {
        }

        basic_json(initializer_list const &init_list, json_type exact_type = json_type::null) {
            bool is_an_object = std::all_of(init_list.begin(), init_list.end(), [](const basic_json &json) {
                return (json.is_array() && json.size() == 2 && json[0].is_string());
            });
            if (exact_type != json_type::object && exact_type != json_type::array) {
                exact_type = is_an_object ? json_type::object : json_type::array;
            }
            if (exact_type == json_type::object) {
                assert(is_an_object);
                value_ = json_type::object;
                for (const basic_json &item: init_list) {
                    object_type &object = (*value_.data.object);
                    array_type &values = (*item.value_.data.vector);
                    object.emplace(values[0].as_string(), values[1]);
                }
            } else {
                value_ = json_type::array;
                auto &vec = (*value_.data.vector);
                vec.reserve(init_list.size());
                vec.assign(init_list.begin(), init_list.end());
            }
        }

        static basic_json object(initializer_list const &init_list) {
            return basic_json(init_list, json_type::object);
        }

        static basic_json array(initializer_list const &init_list) {
            return basic_json(init_list, json_type::array);
        }

        bool is_object() const {
            return value_.type == json_type::object;
        }

        bool is_array() const {
            return value_.type == json_type::array;
        }

        bool is_string() const {
            return value_.type == json_type::string;
        }

        bool is_bool() const {
            return value_.type == json_type::boolean;
        }

        bool is_integer() const {
            return value_.type == json_type::number_integer;
        }

        bool is_float() const {
            return value_.type == json_type::number_float;
        }

        bool is_number() const {
            return is_integer() || is_float();
        }

        bool is_null() const {
            return value_.type == json_type::null;
        }

        bool is_primitive() const {
            switch (value_.type) {
                case json_type::number_integer:
                case json_type::number_float:
                case json_type::array: 
                case json_type::boolean:
                case json_type::null:
                    return true;
                default:
                    break;
            }
            return false;
        }

        json_type type() const {
            return value_.type;
        }

        std::string_view type_name() const {
            switch (type()) {
                case json_type::object:
                    return "object";
                case json_type::array:
                    return "array";
                case json_type::string:
                    return "string";
                case json_type::number_integer:
                    return "integer";
                case json_type::number_float:
                    return "float";
                case json_type::boolean:
                    return "boolean";
                case json_type::null:
                    return "null";
            }
            return {};
        }

        void swap(basic_json &rhs) noexcept {
            value_.swap(rhs.value_);
        }

        iterator begin() {
            iterator iter(this);
            iter.set_begin();
            return iter;
        }

        const_iterator begin() const {
            return cbegin();
        }

        const_iterator cbegin() const {
            const_iterator iter(this);
            iter.set_begin();
            return iter;
        }

        iterator end() {
            iterator iter(this);
            iter.set_end();
            return iter;
        }

        const_iterator end() const {
            return cend();
        }

        const_iterator cend() const {
            const_iterator iter(this);
            iter.set_end();
            return iter;
        }

        reverse_iterator rbegin() {
            return reverse_iterator(end());
        }

        const_reverse_iterator rbegin() const {
            return const_reverse_iterator(end());
        }

        const_reverse_iterator crbegin() const {
            return rbegin();
        }

        reverse_iterator rend() {
            return reverse_iterator(begin());
        }

        const_reverse_iterator rend() const {
            return const_reverse_iterator(begin());
        }

        const_reverse_iterator crend() const {
            return rend();
        }

        size_type size() const {
            switch (type()) {
                case json_type::null:
                    return 0;
                case json_type::array:
                    return value_.data.vector->size();
                case json_type::object:
                    return value_.data.object->size();
                default:
                    return 1;
            }
        }

        bool empty() const {
            if (is_object()) {
                return value_.data.object->empty();
            }
            if (is_array()) {
                return value_.data.vector->empty();
            }
            return is_null();
        }

        template <typename Key>
        const_iterator find(Key &&key) const {
            if (is_object()) {
                const_iterator iter(this);
                iter.object_it_ = value_.data.object->find(std::forward<Key>(key));
                return iter;
            }
            return cend();
        }

        template <typename Key>
        size_type count(Key &&key) const {
            return is_object() ? value_.data.object->count(std::forward<Key>(key)) : 0;
        }

        size_type erase(const typename object_type::key_type &key) {
            if (!is_object()) {
                foundation::exceptions::willow::json_invalid_key("cannot use erase() with non-object value");
            }
            return value_.data.object->erase(key);
        }

        void erase(const size_type index) {
            if (!is_array()) {
                foundation::exceptions::willow::json_invalid_key("cannot use erase() with non-array value");
            }
            value_.data.vector->erase(value_.data.vector->begin() + static_cast<difference_type>(index));
        }

        template <class Iterator,
                  typename std::enable_if<std::is_same<Iterator, iterator>::value || std::is_same<Iterator, const_iterator>::value,
                                          int>::type = 0>
        Iterator erase(Iterator pos) {
            Iterator result = end();

            switch (type()) {
                case json_type::object: {
                    result.it_.object_iter = value_.data.object->erase(pos.it_.object_iter);
                    break;
                }

                case json_type::array: {
                    result.it_.array_iter = value_.data.vector->erase(pos.it_.array_iter);
                    break;
                }

                default:
                    throw implements::json_invalid_iterator("cannot use erase() with non-object & non-array value");
            }

            return result;
        }

        template <class Iterator,
                  typename std::enable_if<std::is_same<Iterator, iterator>::value || std::is_same<Iterator, const_iterator>::value,
                                          int>::type = 0>
        Iterator erase(Iterator first, Iterator last) {
            Iterator result = end();
            switch (type()) {
                case json_type::object: {
                    result.it_.object_iter = value_.data.object->erase(first.it_.object_iter, last.it_.object_iter);
                    break;
                }
                case json_type::array: {
                    result.it_.array_iter = value_.data.vector->erase(first.it_.array_iter, last.it_.array_iter);
                    break;
                }
                default:
                    foundation::exceptions::willow::throw_json_invalid_iterator(
                        "cannot use erase() with non-object & non-array value");
            }
            return result;
        }

        void push_back(basic_json &&json) {
            if (!is_null() && !is_array()) {
                foundation::exceptions::willow::throw_json_type_error("cannot use push_back() with non-array value");
            }
            if (is_null()) {
                value_ = json_type::array;
            }
            value_.data.vector->push_back(std::move(json));
        }

        void push_back(const basic_json &json) {
            if (!is_null() && !is_array()) {
                foundation::exceptions::willow::throw_json_type_error("cannot use push_back() with non-array value");
            }
            if (is_null()) {
                value_ = json_type::array;
            }
            value_.data.vector->push_back(json);
        }

        template <typename... Args, type_traits::other_trans::enable_if_t<
                                        type_traits::type_properties::is_constructible_v<basic_json, Args...>, int> = 0>
        void emplace_back(Args &&...args) {
            if (!is_null() && !is_array()) {
                foundation::exceptions::willow::throw_json_type_error("cannot use emplace_back() with non-array value");
            }
            if (is_null()) {
                value_ = json_type::array;
            }
            value_.data.vector->emplace_back(utility::forward<Args>(args)...);
        }

        basic_json &operator+=(basic_json &&json) {
            push_back(utility::move(json));
            return *this;
        }

        void clear() {
            value_.clear();
        }

        boolean_type as_bool() const {
            if (!is_bool()) {
                foundation::exceptions::willow::throw_json_type_error("json value must be boolean");
            }
            return value_.data.boolean;
        }

        integer_type as_integer() const {
            if (!is_number()) {
                foundation::exceptions::willow::throw_json_type_error("json value must be integer");
            }
            if (is_float()) {
                return static_cast<integer_type>(value_.data.number_float);
            }
            return value_.data.number_integer;
        }

        float_type as_float() const {
            if (!is_number()) {
                foundation::exceptions::willow::throw_json_type_error("json value must be float");
            }
            if (is_integer()) {
                return static_cast<float_type>(value_.data.number_integer);
            }
            return value_.data.number_float;
        }

        array_type &as_array() {
            if (!is_array())
                foundation::exceptions::willow::throw_json_type_error("json value must be array");
            return (*value_.data.vector);
        }

        const array_type &as_array() const {
            if (!is_array())
                foundation::exceptions::willow::throw_json_type_error("json value must be array");
            return (*value_.data.vector);
        }

        const string_type &as_string() const {
            if (!is_string())
                foundation::exceptions::willow::throw_json_type_error("json value must be string");
            return (*value_.data.string);
        }

        object_type &as_object() {
            if (!is_object()) {
                foundation::exceptions::willow::throw_json_type_error("json value must be object");
            }
            return (*value_.data.object);
        }

        const object_type &as_object() const {
            if (!is_object()) {
                foundation::exceptions::willow::throw_json_type_error("json value must be object");
            }
            return (*value_.data.object);
        }

        template <typename Ty>
        Ty get() const {
            Ty value{};
            implements::value_getter<basic_json>::assign(*this, value);
            return value;
        }

        basic_json &operator=(basic_json const &other) {
            value_ = other.value_;
            return *this;
        }

        basic_json &operator=(basic_json &&other) noexcept {
            value_ = utility::move(other.value_);
            return *this;
        }

        basic_json &operator[](size_type index) {
            if (is_null())
                value_ = json_type::array;
            if (!is_array())
                foundation::exceptions::willow::throw_json_invalid_key("operator[] called on a non-array object");
            auto& vec = (*value_.data.vector);
            if (index >= vec.size())
                vec.insert(vec.end(), index - vec.size() + 1, basic_json{});
            return vec[index];
        }

        const basic_json &operator[](size_type index) const {
            if (!is_array())
                foundation::exceptions::willow::throw_json_invalid_key("operator[] called on a non-array type");
            if (index >= (*value_.data.vector).size())
                throw std::out_of_range("operator[] index out of range");
            return (*value_.data.vector)[index];
        }

        RAINY_INLINE basic_json &operator[](const typename object_type::key_type &key) {
            if (is_null()) {
                value_ = json_type::object;
            }
            if (!is_object()) {
                foundation::exceptions::willow::throw_json_invalid_key("operator[] called on a non-object type");
            }
            return (*value_.data.object)[key];
        }

        RAINY_INLINE basic_json &operator[](const typename object_type::key_type &key) const {
            if (!is_object()) {
                foundation::exceptions::willow::throw_json_invalid_key("operator[] called on a non-object object");
            }
            auto iter = value_.data.object->find(key);
            if (iter == value_.data.object->end()) {
                throw std::out_of_range("operator[] key out of range");
            }
            return iter->second;
        }

        template <typename _CharT>
        RAINY_INLINE basic_json &operator[](_CharT *key) {
            if (is_null()) {
                value_ = json_type::object;
            }
            if (!is_object()) {
                foundation::exceptions::willow::throw_json_invalid_key("operator[] called on a non-object object");
            }
            return (*value_.data.object)[key];
        }

        template <typename _CharT>
        RAINY_INLINE const basic_json &operator[](_CharT *key) const {
            if (!is_object()) {
                foundation::exceptions::willow::throw_json_invalid_key("operator[] called on a non-object object");
            }
            auto iter = value_.data.object->find(key);
            if (iter == value_.data.object->end()) {
                throw std::out_of_range("operator[] key out of range");
            }
            return iter->second;
        }

        template <typename Ty>
        explicit operator Ty() const {
            return get<Ty>();
        }

        friend std::basic_ostream<char_type> &operator<<(std::basic_ostream<char_type> &out, const basic_json &j) {
            using char_type = typename std::basic_ostream<char_type>::char_type;

            dump_arguments args;
            args.indent = static_cast<unsigned int>(out.width());
            args.indent_char = out.fill();
            args.precision = static_cast<int>(out.precision());

            out.width(0);

            adapters::stream_output_adapter<char_type> adapter(out);
            j.dump(&adapter, args);
            return out;
        }

        string_type dump(const dump_arguments &args = dump_arguments{}) const {
            string_type result{};
            adapters::string_output_adapter<string_type> adapter(result);
            this->dump(&adapter, args);
            return result;
        }

        string_type dump(unsigned int indent, char_type indent_char = ' ', bool escape_unicode = false) const {
            dump_arguments args;
            args.indent = indent;
            args.indent_char = indent_char;
            args.escape_unicode = escape_unicode;
            return this->dump(args);
        }

        void dump(adapters::output_adapter<char_type> adapter, const dump_arguments &args = dump_arguments()) const {
            implements::json_serializer<basic_json>(adapter, args).dump(*this);
        }

        friend std::basic_istream<char_type> &operator>>(std::basic_istream<char_type> &in, basic_json &json) {
            adapters::stream_input_adapter<char_type> adapter(in);
            implements::json_parser<basic_json>(&adapter).parse(json);
            return in;
        }

        static basic_json parse(const std::basic_string<char_type> &str) {
            adapters::string_input_adapter<std::basic_string<char_type>> adapter(str);
            return parse(&adapter);
        }

        static basic_json parse(std::basic_string_view<char_type> str) {
            adapters::string_view_input_adapter<std::basic_string_view<char_type>> adapter(str.data());
            return parse(&adapter);
        }

        static basic_json parse(const char_type *str) {
            adapters::buffer_input_adapter<char_type> adapter(str);
            return parse(&adapter);
        }

        static basic_json parse(std::FILE *file) {
            adapters::file_input_adapter<char_type> adapter(file);
            return parse(&adapter);
        }

        static basic_json parse(adapters::input_adapter<char_type> adapter) {
            basic_json result;
            implements::json_parser<basic_json>(adapter).parse(result);
            return result;
        }

        friend bool operator==(const basic_json &lhs, const basic_json &rhs) {
            return lhs.value_ == rhs.value_;
        }

        friend bool operator!=(const basic_json &lhs, const basic_json &rhs) {
            return !(lhs == rhs);
        }

        friend bool operator<(const basic_json &lhs, const basic_json &rhs) {
            const auto lhs_type = lhs.type();
            const auto rhs_type = rhs.type();
            if (lhs_type == rhs_type) {
                switch (lhs_type) {
                    case json_type::array:
                        return (*lhs.value_.data.vector) < (*rhs.value_.data.vector);
                    case json_type::object:
                        return (*lhs.value_.data.object) < (*rhs.value_.data.object);
                    case json_type::string:
                        return (*lhs.value_.data.string) < (*rhs.value_.data.string);
                    case json_type::boolean:
                        return (lhs.value_.data.boolean < rhs.value_.data.boolean);
                    case json_type::number_integer:
                        return (lhs.value_.data.number_integer < rhs.value_.data.number_integer);
                    case json_type::number_float:
                        return (lhs.value_.data.number_float < rhs.value_.data.number_float);
                    case json_type::null:
                    default:
                        return false;
                }
            }
            if (lhs_type == json_type::number_integer && rhs_type == json_type::number_float) {
                return (static_cast<float_type>(lhs.value_.data.number_integer) < rhs.value_.data.number_float);
            }
            if (lhs_type == json_type::number_float && rhs_type == json_type::number_integer) {
                return (lhs.value_.data.number_float < static_cast<float_type>(rhs.value_.data.number_integer));
            }
            return false;
        }

        friend bool operator<=(const basic_json &lhs, const basic_json &rhs) {
            return !(rhs < lhs);
        }

        friend bool operator>(const basic_json &lhs, const basic_json &rhs) {
            return rhs < lhs;
        }

        friend bool operator>=(const basic_json &lhs, const basic_json &rhs) {
            return !(lhs < rhs);
        }

    private:
        implements::value<basic_json> value_;
    };
}

#endif