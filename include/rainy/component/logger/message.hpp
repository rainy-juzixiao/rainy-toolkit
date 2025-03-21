#ifndef RAINY_COMPONENT_LOGGER_MESSAGE_HPP
#define RAINY_COMPONENT_LOGGER_MESSAGE_HPP
#include <string>
#include <rainy/foundation/diagnostics/source_location.hpp>

namespace rainy::component::logger {
    template <typename Elem,typename Traits,typename Alloc>
    class basic_message {
    public:
        using string_type = std::basic_string<Elem,Traits,Alloc>;
        using value_type = Elem;
        using traits_type = Traits;
        using allocator_type = Alloc;
        using source_location = utility::source_location;

        basic_message() = default;

        basic_message(const string_type& message, const source_location& location)
            : message_(message), location_(location) {}

        basic_message(const basic_message&) = default;
        basic_message(basic_message&&) = default;

    private:
        string_type message_;
        source_location location_;
    };

}

#endif