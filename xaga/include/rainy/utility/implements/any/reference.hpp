#ifndef RAINY_UTILITY_IMPLEMENTS_ANY_REFERENCE_HPP
#define RAINY_UTILITY_IMPLEMENTS_ANY_REFERENCE_HPP
#include <rainy/core/core.hpp>
#include <rainy/utility/implements/any/fwd.hpp>

namespace rainy::utility::implements {
    template <typename BasicAny>
    class any_reference : public BasicAny {
    public:
        friend struct any_execution_policy;

        using basic_any = BasicAny;

        any_reference() : basic_any{} {
        }

        template <typename ValueType,
                  type_traits::other_trans::enable_if_t<!type_traits::type_relations::is_any_of_v<
                                                            type_traits::other_trans::decay_t<ValueType>, basic_any, any_reference>,
                                                        int> = 0>
        any_reference(ValueType &&value, basic_any *this_pointer) :
            basic_any{std::in_place_type<decltype(value)>, utility::forward<ValueType>(value)}, this_pointer{this_pointer},
            the_type{&this_pointer->type()} {
        }

        any_reference(const any_reference &) = default;
        any_reference(any_reference &&) = default;

        template <typename ValueType>
        any_reference &operator=(ValueType &&value) {
#if RAINY_ENABLE_DEBUG
            assert((*this->the_type) == this->this_pointer->type() && "Can't accept a illegal reference to modify");
#else
            if (*this->the_type != this->this_pointer->type()) {
                return *this;
            }
#endif
            auto tuple = std::make_tuple(this, BasicAny{utility::forward<ValueType>(value)});
            this->storage.executer->invoke(any_operation::assign, &tuple);
            return *this;
        }

        any_reference &operator=(const any_reference &right) {
            if (utility::addressof(right) == this) {
                return *this;
            }
            auto tuple = std::make_tuple(this, right.as_value());
            this->storage.executer->invoke(any_operation::assign, &tuple);
            return *this;
        }

        any_reference &operator=(any_reference &&right) { // NOLINT
            if (utility::addressof(right) == this) {
                return *this;
            }
            auto tuple = std::make_tuple(this, right.as_value());
            this->storage.executer->invoke(any_operation::assign, &tuple);
            return *this;
        }

        void swap(any_reference &right) noexcept {
            this->swap_value(right);
        }

        friend void swap(any_reference left, any_reference right) noexcept {
            left.swap(right);
        }

        basic_any &as_value() noexcept {
            return static_cast<basic_any &>(*this);
        }

        const basic_any &as_value() const noexcept {
            return static_cast<const basic_any &>(*this);
        }

        basic_any construct_from_this() {
            basic_any any{};
            auto tuple = std::make_tuple(false, this, &any);
            basic_any::storage.executer->invoke(implements::any_operation::construct_from, &tuple);
            return any;
        }

        basic_any construct_from_this() const {
            basic_any any{};
            auto tuple = std::make_tuple(true, this, &any);
            basic_any::storage.executer->invoke(implements::any_operation::construct_from, &tuple);
            return any;
        }

    private:
        basic_any *this_pointer{nullptr};
        const foundation::ctti::typeinfo *the_type{nullptr};
    };
}

#endif