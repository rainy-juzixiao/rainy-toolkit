#ifndef RAINY_UTILITY_ARG_EXTRACTOR_HPP
#define RAINY_UTILITY_ARG_EXTRACTOR_HPP
#include <rainy/core/core.hpp>

namespace rainy::utility {
    template <typename Ty,typename... Args>
    class arg_extractor {
    public:
        static auto extract_to_array(Args &&...args) {
            static constexpr std::size_t size =
                type_traits::other_trans::count_type_v<Ty, type_traits::other_trans::type_list<Args...>>;
            collections::array<Ty, size> result;
            extract_types_recursively(result, utility::forward<Args>(args)...);
            return result;
        }

    private:
        template <typename ContainerType>
        static void extract_types_recursively(ContainerType & container) {
            (void) container;
        }

        template <typename ContainerType, typename Uty, typename... Rest,
                  type_traits::other_trans::enable_if_t<
                      !type_traits::type_relations::is_same_v<Ty, type_traits::other_trans::decay_t<Uty>>, int> = 0>
        static void extract_types_recursively(ContainerType &container, Uty &&value, Rest &&...rest) {
            extract_types_recursively(container, utility::forward<Rest>(rest)...);
        }

        template <typename ContainerType, typename Uty, typename... Rest,
                  type_traits::other_trans::enable_if_t<
                      type_traits::type_relations::is_same_v<Ty, type_traits::other_trans::decay_t<Uty>>, int> = 0>
        static void extract_types_recursively(ContainerType &container, Uty &&value, Rest &&...tail) {
            static constexpr auto index = type_traits::other_trans::count_type_v<Ty, type_traits::other_trans::type_list<Args...>> -
                                          type_traits::other_trans::count_type_v<Ty, type_traits::other_trans::type_list<Rest...>> - 1;
            if constexpr (type_traits::type_properties::is_move_constructible_v<Ty>) {
                utility::construct_at(&container[index], utility::forward<Uty>(value));
            } else {
                container[index] = utility::forward<Uty>(value);
            }
            extract_types_recursively(container, utility::forward<Rest>(tail)...);
        }

        template <typename Uty, typename... Rest,
                  type_traits::other_trans::enable_if_t<
                      type_traits::type_relations::is_same_v<Ty, type_traits::other_trans::decay_t<Uty>>, int> = 0>
        static void extract_types_recursively(std::vector<Ty> &container, Uty &&value, Rest &&...tail) {
            container.emplace_back(utility::forward<Uty>(value));
            extract_types_recursively(container, utility::forward<Rest>(tail)...);
        }
    };

    template <typename Ty, typename... Args>
    auto extract_args_to_array(Args &&...args) {
        return arg_extractor<Ty, Args...>::extract_to_array(utility::forward<Args>(args)...);
    }
}

#endif