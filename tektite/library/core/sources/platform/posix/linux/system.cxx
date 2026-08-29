#include <rainy/core/platform.hpp>
#include <rainy/core/layer.hpp>

#include <sys/random.h>
#include <sys/sysinfo.h>
#include <unistd.h>

namespace rainy::core::layer {
    rain_fn query_memory_capacity(std::size_t *total, std::size_t *available) noexcept -> bool {
        if (!total || !available) {
            return false;
        }
        struct sysinfo info{};
        if (sysinfo(&info) != 0) {
            return false;
        }
        *total = static_cast<std::size_t>(info.totalram) * info.mem_unit;
        *available = static_cast<std::size_t>(info.freeram) * info.mem_unit;
        return true;
    }

    rain_fn system_random(void *buffer, std::size_t length) noexcept -> bool {
        if (!buffer || length == 0) {
            return false;
        }
        std::size_t filled = 0;
        while (filled < length) {
            const ssize_t chunk = ::getrandom(static_cast<char *>(buffer) + filled, length - filled, 0);
            if (chunk < 0) {
                if (errno == EINTR) {
                    continue;
                }
                return false;
            }
            filled += static_cast<std::size_t>(chunk);
        }
        return true;
    }
}
