/*
* Copyright 2026 rainy-juzixiao
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <rainy/core/platform.hpp>
#include <rainy/core/layer.hpp>

#if RAINY_USING_LINUX
#include <sys/auxv.h>
#elif RAINY_USING_MACOS
#include <sys/sysctl.h>
#include <mach/machine.h>
#elif RAINY_USING_WINDOWS
#include <windows.h>
#endif

namespace rainy::core::layer {
    rain_fn arm64_hwcap() noexcept -> unsigned long {
#if RAINY_USING_LINUX
        return ::getauxval(AT_HWCAP);
#elif RAINY_USING_MACOS
        unsigned long hwcap = 0;
        auto query = [&hwcap](const char *name, unsigned long bit) {
            int present = 0;
            size_t size = sizeof(present);
            if (::sysctlbyname(name, &present, &size, nullptr, 0) == 0 && present) {
                hwcap |= bit;
            }
        };
        query("hw.optional.advsimd", 1UL << 0);
        query("hw.optional.arm.FEAT_AES", 1UL << 3);
        query("hw.optional.arm.FEAT_PMULL", 1UL << 4);
        query("hw.optional.arm.FEAT_SHA1", 1UL << 5);
        query("hw.optional.arm.FEAT_SHA256", 1UL << 6);
        query("hw.optional.arm.FEAT_CRC32", 1UL << 7);
        return hwcap;
#elif RAINY_USING_WINDOWS
        unsigned long hwcap = 0;
        if (IsProcessorFeaturePresent(PF_ARM_V8_INSTRUCTIONS_AVAILABLE)) {
            hwcap |= (1UL << 0);
        }
        if (IsProcessorFeaturePresent(PF_ARM_NEON_INSTRUCTIONS_AVAILABLE)) {
            hwcap |= (1UL << 1);
        }
        if (IsProcessorFeaturePresent(PF_ARM_VFP_32_REGISTERS_AVAILABLE)) {
            hwcap |= (1UL << 2);
        }
        if (IsProcessorFeaturePresent(PF_ARM_V82_DP_INSTRUCTIONS_AVAILABLE)) {
            hwcap |= (1UL << 3);
        }
        if (IsProcessorFeaturePresent(PF_ARM_V81_ATOMIC_INSTRUCTIONS_AVAILABLE)) {
            hwcap |= (1UL << 4);
        }
        if (IsProcessorFeaturePresent(PF_ARM_V8_CRYPTO_INSTRUCTIONS_AVAILABLE)) {
            hwcap |= (1UL << 5);
        }
        if (IsProcessorFeaturePresent(PF_ARM_V8_CRC32_INSTRUCTIONS_AVAILABLE)) {
            hwcap |= (1UL << 6);
        }
        return hwcap;
#else
        return 0;
#endif
    }
}