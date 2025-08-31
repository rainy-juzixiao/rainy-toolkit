#pragma once
#include <iterator>
#include <rainy/component/willow/utils.hpp>

namespace rainy::component::willow::iterators {
    using namespace foundation::exceptions::willow;

    class primitive_iterator {
    public:
        using difference_type = std::ptrdiff_t;

        explicit primitive_iterator(const difference_type it = 0) : it_(it) {
        }

        void set_begin() {
            it_ = 0;
        }

        void set_end() {
            it_ = 1;
        }

        primitive_iterator &operator++() {
            ++it_;
            return *this;
        }

        primitive_iterator operator++(int) {
            const primitive_iterator old(it_);
            ++(*this);
            return old;
        }

        primitive_iterator &operator--() {
            --it_;
            return (*this);
        }

        primitive_iterator operator--(int) {
            const primitive_iterator old = (*this);
            --(*this);
            return old;
        }

        bool operator==(const primitive_iterator &other) const {
            return it_ == other.it_;
        }

        bool operator!=(const primitive_iterator &other) const {
            return !(*this == other);
        }

        primitive_iterator operator+(const difference_type off) const {
            return primitive_iterator(it_ + off);
        }

        primitive_iterator operator-(const difference_type off) const {
            return primitive_iterator(it_ - off);
        }

        primitive_iterator &operator+=(const difference_type off) {
            it_ += off;
            return (*this);
        }

        primitive_iterator &operator-=(const difference_type off) {
            it_ -= off;
            return (*this);
        }

        difference_type operator-(const primitive_iterator &other) const {
            return it_ - other.it_;
        }

        bool operator<(const primitive_iterator &other) const {
            return it_ < other.it_;
        }

        bool operator<=(const primitive_iterator &other) const {
            return it_ <= other.it_;
        }

        bool operator>(const primitive_iterator &other) const {
            return it_ > other.it_;
        }

        bool operator>=(const primitive_iterator &other) const {
            return it_ >= other.it_;
        }

    private:
        difference_type it_;
    };

    template <typename BasicJson>
    struct iterator_value {
        using value_type = BasicJson;
        using object_type = typename BasicJson::object_type;
        using key_type = typename object_type::key_type;

        explicit iterator_value(value_type *value) : key_(&dummy_key_), value_(value) {
        }
        explicit iterator_value(const key_type &key, value_type *value) : key_(&key), value_(value) {
        }

        const key_type &key() const {
            return key_ == *dummy_key_ ? throw_exception(json_invalid_iterator("cannot use key() with non-object type")) : *key_;
        }

        value_type &value() const {
            return *value_;
        }

        explicit operator value_type &() const {
            return *value_;
        }

    private:
        static key_type dummy_key_;
        const key_type *key_;
        value_type *value_;
    };

    template <typename BasicJson>
    typename iterator_value<BasicJson>::key_type iterator_value<BasicJson>::dummy_key_;

    template <typename BasicJson>
    class json_iterator {
    public:
        friend BasicJson;

        using string_type = typename BasicJson::string_type;
        using char_type = typename BasicJson::char_type;
        using integer_type = typename BasicJson::integer_type;
        using float_type = typename BasicJson::float_type;
        using boolean_type = typename BasicJson::boolean_type;
        using array_type = typename BasicJson::array_type;
        using object_type = typename BasicJson::object_type;

        using value_type = BasicJson;
        using difference_type = std::ptrdiff_t;
        using iterator_category = std::bidirectional_iterator_tag;
        using pointer = iterator_value<value_type> *;
        using reference = iterator_value<value_type> &;

        explicit json_iterator(value_type *json) : data_(json), it_value_(json) {
        }

        reference operator*() const {
            check_data();
            check_iterator();
            switch (data_->type()) {
                case json_type::object:
                    it_value_ = iterator_value<value_type>(object_it_->first, &(object_it_->second));
                    break;
                case json_type::array:
                    it_value_ = iterator_value<value_type>(&(*array_it_));
                    break;
                default:
                    it_value_ = iterator_value<value_type>(data_);
                    break;
            }
            return it_value_;
        }

        pointer operator->() const {
            return &(operator*());
        }

        json_iterator operator++(int) {
            json_iterator old = (*this);
            ++(*this);
            return old;
        }

        json_iterator &operator++() {
            check_data();

            switch (data_->type()) {
                case json_type::object: {
                    std::advance(object_it_, 1);
                    break;
                }
                case json_type::array: {
                    std::advance(array_it_, 1);
                    break;
                }
                default: {
                    if (data_->type() != json_type::null) {
                        ++original_it_;
                    }
                    break;
                }
            }
            return *this;
        }

        json_iterator operator--(int) {
            json_iterator old = (*this);
            --(*this);
            return old;
        }

        json_iterator &operator--() {
            check_data();
            switch (data_->type()) {
                case json_type::object: {
                    std::advance(object_it_, -1);
                    break;
                }
                case json_type::array: {
                    std::advance(array_it_, -1);
                    break;
                }
                default: {
                    if (data_->type() != json_type::null) {
                        --original_it_;
                    }
                    break;
                }
            }
            return *this;
        }

        json_iterator operator-(const difference_type off) const {
            return operator+(-off);
        }

        json_iterator operator+(const difference_type off) const {
            json_iterator ret(*this);
            ret += off;
            return ret;
        }

        json_iterator &operator-=(const difference_type off) {
            return operator+=(-off);
        }

        json_iterator &operator+=(const difference_type off) {
            check_data();
            switch (data_->type()) {
                case json_type::object: {
                    throw_exception(json_invalid_iterator("cannot use offsets with object type"));
                }
                case json_type::array: {
                    std::advance(array_it_, off);
                    break;
                }
                default: {
                    if (data_->type() != json_type::null) {
                        original_it_ += off;
                    }
                    break;
                }
            }
            return *this;
        }

        bool operator!=(const json_iterator &other) const {
            return !(*this == other);
        }

        bool operator==(const json_iterator &other) const {
            if (data_ == nullptr) {
                return false;
            }
            if (data_ != other.data_) {
                return false;
            }
            switch (data_->type()) {
                case json_type::object: {
                    return object_it_ == other.object_it_;
                }
                case json_type::array: {
                    return array_it_ == other.array_it_;
                }
                default: {
                    return original_it_ == other.original_it_;
                }
            }
        }

        bool operator>(const json_iterator &other) const {
            return other.operator<(*this);
        }

        bool operator>=(const json_iterator &other) const {
            return !operator<(other);
        }

        bool operator<=(const json_iterator &other) const {
            return !other.operator<(*this);
        }

        bool operator<(const json_iterator &other) const {
            check_data();
            other.check_data();
            if (data_ != other.data_)
                throw json_invalid_iterator("cannot compare iterators of different objects");
            switch (data_->type()) {
                case json_type::object:
                    throw json_invalid_iterator("cannot compare iterators with object type");
                case json_type::array:
                    return array_it_ < other.array_it_;
                default:
                    return original_it_ < other.original_it_;
            }
        }

    private:
        void set_begin() {
            check_data();

            switch (data_->type()) {
                case json_type::object: {
                    object_it_ = data_->value_.data.object->begin();
                    break;
                }
                case json_type::array: {
                    array_it_ = data_->value_.data.vector->begin();
                    break;
                }
                default: {
                    if (data_->type() != json_type::null) {
                        original_it_.set_begin();
                    }
                    break;
                }
            }
        }

        void set_end() {
            check_data();

            switch (data_->type()) {
                case json_type::object: {
                    object_it_ = data_->value_.data.object->end();
                    break;
                }
                case json_type::array: {
                    array_it_ = data_->value_.data.vector->end();
                    break;
                }
                default: {
                    if (data_->type() != json_type::null) {
                        original_it_.set_end();
                    }
                    break;
                }
            }
        }

        void check_data() const {
            if (data_ == nullptr) {
                throw json_invalid_iterator("iterator contains an empty object");
            }
        }

        void check_iterator() const {
            switch (data_->type()) {
                case json_type::object:
                    if (object_it_ == data_->value_.data.object->end()) {
                        throw std::out_of_range("iterator out of range");
                    }
                    break;
                case json_type::array:
                    if (array_it_ == data_->value_.data.vector->end()) {
                        throw std::out_of_range("iterator out of range");
                    }
                    break;
                case json_type::null: {
                    throw std::out_of_range("iterator out of range");
                }
                default:
                    if (original_it_ != primitive_iterator{0}) {
                        throw std::out_of_range("iterator out of range");
                    }
                    break;
            }
        }

        using array_iter = typename BasicJson::array_type::iterator;
        using object_iter = typename BasicJson::object_type::iterator;

        value_type *data_;
        mutable iterator_value<value_type> it_value_;
        array_iter array_it_;
        object_iter object_it_;
        primitive_iterator original_it_; // for other types
    };
}
