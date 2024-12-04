#ifndef RAINY_EXCEPTION_HANDLER
#define RAINY_EXCEPTION_HANDLER
#include <random>
#include <unordered_map>
#include <rainy/functional/function_pointer.hpp>
#include <rainy/diagnostics/source_location.hpp>

namespace rainy::information {
    class exception_handler {
    public:
        RAINY_DECLARE_SIGNLE_INSTANCE(exception_handler)

        static std::size_t add_process() {
            
        }



    private:


        exception_handler() = default;
        exception_handler(const exception_handler&) = default;
        exception_handler(exception_handler&&) = default;


        std::unordered_map<std::size_t,
                           foundation::functional::function_pointer<void(const std::string_view, const utility::source_location &)>>
            process_queue;
    };
}

#endif
