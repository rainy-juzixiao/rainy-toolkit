#include <rainy/core/platform.hpp>
#include <rainy/core/layer.hpp>

#include <sys/sysctl.h>
#include <mach/mach_host.h>
#include <mach/mach_init.h>

namespace rainy::core::layer {
    rain_fn query_memory_capacity(std::size_t *total, std::size_t *available) noexcept -> bool {
        if (!total || !available) {
            return false;
        }
        std::uint64_t total_bytes = 0;
        std::size_t length = sizeof(total_bytes);
        if (sysctlbyname("hw.memsize", &total_bytes, &length, nullptr, 0) != 0) {
            return false;
        }
        vm_statistics64_data_t statistics{};
        mach_msg_type_number_t count = HOST_VM_INFO64_COUNT;
        if (host_statistics64(mach_host_self(), HOST_VM_INFO64, reinterpret_cast<host_info64_t>(&statistics), &count) !=
            KERN_SUCCESS) {
            return false;
        }
        *total = static_cast<std::size_t>(total_bytes);
        *available = static_cast<std::size_t>(statistics.free_count + statistics.inactive_count) * vm_page_size;
        return true;
    }

    rain_fn system_random(void *buffer, std::size_t length) noexcept -> bool {
        if (!buffer || length == 0) {
            return false;
        }
        ::arc4random_buf(buffer, length);
        return true;
    }
}
