#ifndef RAINY_ANNOTATIONS_AUTO_WIRED_HPP
#define RAINY_ANNOTATIONS_AUTO_WIRED_HPP
#include <rainy/core/core.hpp>
#include <rainy/utility/ioc.hpp>

namespace rainy::annotations {
    template <typename Type>
    class auto_wired {
    public:
        using type = type_traits::cv_modify::remove_cvref_t<Type>;

        auto_wired() {
            if (utility::ioc::container::is_this_type_available<Type>()) {
                object = utility::ioc::container::get_instance<Type>();
            } else {
                utility::ioc::container::register_type<Type>();
                object = utility::ioc::container::get_instance<Type>();
            }
        }

        template <typename... Args>
        auto_wired(Args &&...args) {
            if (utility::ioc::container::is_this_type_with_construct_args_type_available<Type(Args...)>()) {
                object = utility::ioc::container::get_instance<Type>();
            } else {
                utility::ioc::container::register_type<Type, Type, Args...>();
                object = utility::ioc::container::get_instance<Type>(utility::forward<Args>(args)...);
            }
        }

        auto_wired(const auto_wired &) = default;
        auto_wired &operator=(const auto_wired &) noexcept = default;
        auto_wired(auto_wired &&) noexcept = default;
        auto_wired& operator=(auto_wired &&) noexcept = default;

        rain_fn has_inject_failed() -> bool {
            return object == nullptr;
        }

        rain_fn operator->() noexcept -> type * {
            return object;
        }

        rain_fn operator->() const noexcept -> const type * {
            return object;
        }

        rain_fn operator*() noexcept -> type & {
            return *object;
        }

        rain_fn operator*() const noexcept -> type & {
            return *object;
        }

    private:
        type *object{nullptr};
    };
}

#endif
