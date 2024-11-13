#include <rainy/base.hpp>

namespace rainy::containers::internals {
    template <typename Ty>
    struct tuple_val { // stores each value in a tuple
        constexpr tuple_val() : value() {
        }

        template <typename Other>
        constexpr tuple_val(Other &&arg) : value(forward<Other>(arg)) {
        }

        template <typename Alloc, typename... Args,
                  type_traits::other_transformations::enable_if_t<!std::uses_allocator_v<Ty, Alloc>, int> = 0>
        constexpr tuple_val(const Alloc &, utility::allocator_arg_t, Args &&...args) : value(utility::forward<Args>(args)...) {
        }

        template <typename Alloc, typename... Args,
                  type_traits::other_transformations::enable_if_t<
                      type_traits::logical_traits::conjunction_v<
                          std::uses_allocator<Ty, Alloc>, std::is_constructible<Ty, std::allocator_arg_t, const Alloc &, Args...>>,
                      int> = 0>
        constexpr tuple_val(const Alloc &allocator, utility::allocator_arg_t, Args &&...args) :
            value(utility::allocator_arg, allocator, std::forward<Args>(args)...) {
        }

        template <typename Alloc, typename... Args,
                  type_traits::other_transformations::enable_if_t<
                      type_traits::logical_traits::conjunction_v<
                          std::uses_allocator<Ty, Alloc>, type_traits::logical_traits::negation<
                                                              std::is_constructible<Ty, utility::allocator_arg_t, const Alloc &, Args...>>>,
                      int> = 0>
        constexpr tuple_val(const Alloc &allocator, utility::allocator_arg_t, Args &&..._Arg) :
            value(utility::forward<Args>(_Arg)..., allocator) {
        }

        Ty value;
    };
}

