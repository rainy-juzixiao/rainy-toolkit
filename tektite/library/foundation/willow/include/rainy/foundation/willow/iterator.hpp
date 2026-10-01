/*
 * Copyright 2026 rainy-juzixiao
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef RAINY_FOUNDATION_WILLOW_ITERATOR_HPP
#define RAINY_FOUNDATION_WILLOW_ITERATOR_HPP
#include <iterator>
#include <rainy/foundation/willow/implements/common/config.hpp>
#include <rainy/foundation/willow/implements/common/exceptions.hpp>

namespace rainy::foundation::willow {
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

    template <typename BasicDocument>
    struct document_iterator_value {
        using value_type = BasicDocument;
        using object_type = typename BasicDocument::object_type;
        using key_type = typename object_type::key_type;

        explicit document_iterator_value(value_type *value) : key_(&dummy_key_), value_(value) {
        }
        explicit document_iterator_value(const key_type &key, value_type *value) : key_(&key), value_(value) {
        }

        const key_type &key() const {
            if (key_ == &dummy_key_) {
                exceptions::willow::throw_willow_invalid_iterator("cannot use key() with non-object type");
            }
            return *key_;
        }

        value_type &value() const {
            return *value_;
        }

        document_iterator_value *operator->() noexcept {
            return this;
        }

        explicit operator value_type &() const {
            return *value_;
        }

    private:
        static key_type dummy_key_;
        const key_type *key_;
        value_type *value_;
    };

    template <typename BasicDocument>
    typename document_iterator_value<BasicDocument>::key_type document_iterator_value<BasicDocument>::dummy_key_;

    template <typename BasicDocument>
    class document_iterator {
    public:
        friend BasicDocument;

        using string_type = typename BasicDocument::string_type;
        using char_type = typename BasicDocument::char_type;
        using integer_type = typename BasicDocument::integer_type;
        using float_type = typename BasicDocument::float_type;
        using boolean_type = typename BasicDocument::boolean_type;
        using array_type = typename BasicDocument::array_type;
        using object_type = typename BasicDocument::object_type;

        using value_type = BasicDocument;
        using difference_type = std::ptrdiff_t;
        using iterator_category = std::bidirectional_iterator_tag;
        using pointer = document_iterator_value<value_type>;
        using reference = document_iterator_value<value_type>;

        explicit document_iterator(value_type *doc) : data_(doc) {
        }

        reference operator*() const {
            check_data();
            check_iterator();
            switch (data_->type()) {
                case document_type::object:
                    return document_iterator_value<value_type>(object_it_->first, &(object_it_->second));
                case document_type::array:
                    return document_iterator_value<value_type>(&(*array_it_));
                default:
                    return document_iterator_value<value_type>(data_);
            }
        }

        pointer operator->() const {
            return operator*();
        }

        document_iterator operator++(int) {
            document_iterator old = (*this);
            ++(*this);
            return old;
        }

        document_iterator &operator++() {
            check_data();

            switch (data_->type()) {
                case document_type::object: {
                    std::advance(object_it_, 1);
                    break;
                }
                case document_type::array: {
                    std::advance(array_it_, 1);
                    break;
                }
                default: {
                    if (data_->type() != document_type::null) {
                        ++original_it_;
                    }
                    break;
                }
            }
            return *this;
        }

        document_iterator operator--(int) {
            document_iterator old = (*this);
            --(*this);
            return old;
        }

        document_iterator &operator--() {
            check_data();
            switch (data_->type()) {
                case document_type::object: {
                    std::advance(object_it_, -1);
                    break;
                }
                case document_type::array: {
                    std::advance(array_it_, -1);
                    break;
                }
                default: {
                    if (data_->type() != document_type::null) {
                        --original_it_;
                    }
                    break;
                }
            }
            return *this;
        }

        document_iterator operator-(const difference_type off) const {
            return operator+(-off);
        }

        document_iterator operator+(const difference_type off) const {
            document_iterator ret(*this);
            ret += off;
            return ret;
        }

        document_iterator &operator-=(const difference_type off) {
            return operator+=(-off);
        }

        document_iterator &operator+=(const difference_type off) {
            check_data();
            switch (data_->type()) {
                case document_type::object: {
                    exceptions::willow::throw_willow_invalid_iterator("cannot use offsets with object type");
                }
                case document_type::array: {
                    std::advance(array_it_, off);
                    break;
                }
                default: {
                    if (data_->type() != document_type::null) {
                        original_it_ += off;
                    }
                    break;
                }
            }
            return *this;
        }

        bool operator!=(const document_iterator &other) const {
            return !(*this == other);
        }

        bool operator==(const document_iterator &other) const {
            if (data_ == nullptr) {
                return false;
            }
            if (data_ != other.data_) {
                return false;
            }
            switch (data_->type()) {
                case document_type::object: {
                    return object_it_ == other.object_it_;
                }
                case document_type::array: {
                    return array_it_ == other.array_it_;
                }
                default: {
                    return original_it_ == other.original_it_;
                }
            }
        }

        bool operator>(const document_iterator &other) const {
            return other.operator<(*this);
        }

        bool operator>=(const document_iterator &other) const {
            return !operator<(other);
        }

        bool operator<=(const document_iterator &other) const {
            return !other.operator<(*this);
        }

        bool operator<(const document_iterator &other) const {
            check_data();
            other.check_data();
            if (data_ != other.data_)
                exceptions::willow::throw_willow_invalid_iterator("cannot compare iterators of different objects");
            switch (data_->type()) {
                case document_type::object:
                    exceptions::willow::throw_willow_invalid_iterator("cannot compare iterators with object type");
                case document_type::array:
                    return array_it_ < other.array_it_;
                default:
                    return original_it_ < other.original_it_;
            }
        }

    private:
        void set_begin() {
            check_data();

            switch (data_->type()) {
                case document_type::object: {
                    object_it_ = data_->value_.data.object->begin();
                    break;
                }
                case document_type::array: {
                    array_it_ = data_->value_.data.vector->begin();
                    break;
                }
                default: {
                    if (data_->type() != document_type::null) {
                        original_it_.set_begin();
                    }
                    break;
                }
            }
        }

        void set_end() {
            check_data();

            switch (data_->type()) {
                case document_type::object: {
                    object_it_ = data_->value_.data.object->end();
                    break;
                }
                case document_type::array: {
                    array_it_ = data_->value_.data.vector->end();
                    break;
                }
                default: {
                    if (data_->type() != document_type::null) {
                        original_it_.set_end();
                    }
                    break;
                }
            }
        }

        void check_data() const {
            if (data_ == nullptr) {
                exceptions::willow::throw_willow_invalid_iterator("iterator contains an empty object");
            }
        }

        void check_iterator() const {
            switch (data_->type()) {
                case document_type::object:
                    if (object_it_ == data_->value_.data.object->end()) {
                        throw std::out_of_range("iterator out of range");
                    }
                    break;
                case document_type::array:
                    if (array_it_ == data_->value_.data.vector->end()) {
                        throw std::out_of_range("iterator out of range");
                    }
                    break;
                case document_type::null: {
                    throw std::out_of_range("iterator out of range");
                }
                default:
                    if (original_it_ != primitive_iterator{0}) {
                        throw std::out_of_range("iterator out of range");
                    }
                    break;
            }
        }

        using array_iter = typename BasicDocument::array_type::iterator;
        using object_iter = typename BasicDocument::object_type::iterator;

        value_type *data_;
        array_iter array_it_;
        object_iter object_it_;
        primitive_iterator original_it_;
    };
}

#endif
