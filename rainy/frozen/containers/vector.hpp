#ifndef RAINY_FROZEN_FROZEN_VECTOR_HPP
#define RAINY_FROZEN_FROZEN_VECTOR_HPP
#include <rainy/core/core.hpp>
#include <rainy/system/basic_exceptions.hpp>
#include <rainy/utility/iterator.hpp>

namespace rainy::containers::frozen {
    template <typename Ty, std::size_t N>
    class frozen_vector {
    public:
        using value_type = Ty;
        using reference = value_type&;
        using const_reference = const value_type&;
        using pointer = value_type*;
        using const_pointer = const value_type*;
        using size_type = std::size_t;
        using difference_type = std::ptrdiff_t;
        using iterator = utility::iterator<pointer>;
        using const_iterator = utility::const_iterator<const_pointer>;
         
        constexpr frozen_vector() noexcept(std::is_nothrow_constructible_v<Ty>) = default;

        RAINY_CONSTEXPR20 frozen_vector(std::initializer_list<Ty> ilist) noexcept(std::is_nothrow_copy_assignable_v<Ty>) {
            _size = ilist.size();
#if RAINY_HAS_CXX20
            if (std::is_constant_evaluated()) {
                std::copy_n(ilist.begin(), _size, elems);
            } else
#endif
            {
                rainy::utility::expects(ilist.size() <= _size);
                if constexpr (type_traits::type_properties::is_pod_v<Ty>) {
                    std::uninitialized_copy_n(ilist.begin(), _size, elems);
                } else {
                    std::copy_n(ilist.begin(), _size, elems);
                }
            }
        }

        frozen_vector(const frozen_vector &right) {
            
            right.elems;
        }

        constexpr std::size_t size() const noexcept {
            return _size;
        }

        constexpr size_type length() const noexcept {
            return N;
        }

        constexpr std::size_t empty() const noexcept {
            return _size == 0;
        }

        template <typename... Args>
        void emplace_back(Args... args) noexcept {
            range_check(_size + 1);
        }

    private:
        void range_check(const size_type idx) {
            if (idx <= _size) {
                rainy::foundation::system::exceptions::logic::throw_out_of_range("Invalid frozen_vector subscript");
            }
        }

        static void check_zero_length_error() {
            if (N == 0) {
                std::terminate();
            }
        }

        std::size_t _size;
        value_type elems[N];
    };
}

#endif