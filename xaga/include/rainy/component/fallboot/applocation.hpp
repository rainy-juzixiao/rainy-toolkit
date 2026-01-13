#ifndef RAINY_COMPONENT_FALLBOOT_APPLICATION_HPP
#define RAINY_COMPONENT_FALLBOOT_APPLICATION_HPP
#include <optional>
#include <mutex>
#include <filesystem>
#include <rainy/core/core.hpp>
#include <rainy/foundation/typeinfo.hpp>
#include <rainy/foundation/memory/nebula_ptr.hpp>
#include <rainy/text/string.hpp>

namespace rainy::component::fallboot {
    class dispatcher;
}

namespace rainy::component::fallboot::implements {
    RAINY_TOOLKIT_API std::once_flag &once_flag() noexcept;
    RAINY_TOOLKIT_API dispatcher &global_dispatcher() noexcept;
}

namespace rainy::component::fallboot {
    enum class application_type {
        web_server,
        backend_rpc
    };

    class RAINY_TOOLKIT_API application : type_traits::helper::non_copyable {
    public:
        application() = default;
        application &app_name(std::string_view name);
        application &with_config_path(std::filesystem::path config_path);

        int run(application_type application_type, int argc, char *argv[]);

        static application &init();
        static void shutdown();

        virtual ~application() = default;

    private:
        virtual void start_application() = 0;
        virtual void stop_application() = 0;
        virtual void set_appname(std::string_view name) = 0;
        virtual void set_config_path(std::filesystem::path config_path) = 0;
    };
}

#endif