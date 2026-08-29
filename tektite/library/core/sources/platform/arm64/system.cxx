#include <rainy/core/platform.hpp>
#include <rainy/core/layer.hpp>

#include <sys/auxv.h>

namespace rainy::core::layer {
    rain_fn arm64_hwcap() noexcept -> unsigned long {
        return ::getauxval(AT_HWCAP);
    }
}
