/*
 * Copyright 2025 rainy-juzixiao
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
#include <rainy/core/core.hpp>

#if RAINY_USING_WINDOWS
#include <windows.h>
#include <intrin.h>
#include <VersionHelpers.h>
#else
#include <unistd.h>
#endif

#include <iostream>

#if RAINY_USING_CLANG
#pragma clang diagnostic ignored "-Wcast-function-type-mismatch"
#pragma clang diagnostic ignored "-Wsign-compare"
#endif

#if RAINY_USING_WINDOWS
namespace rainy::core::builtin::implements {
    struct rtl_osversion_info {
        ULONG dwOSVersionInfoSize;
        ULONG dwMajorVersion;
        ULONG dwMinorVersion;
        ULONG dwBuildNumber;
        ULONG dwPlatformId;
        WCHAR szCSDVersion[128];
    };

    long NTAPI get_version_impl(rtl_osversion_info *version_info) {
        using impl_type = long(NTAPI *)(rtl_osversion_info *);
        static impl_type impl_fn;
        if (impl_fn) {
            return impl_fn(version_info);
        }
        auto handle = GetModuleHandleA("ntdll.dll");
        if (!handle) {
            return ERROR_NOT_FOUND;
        }
        if (impl_fn = reinterpret_cast<impl_type>(GetProcAddress(handle, "RtlGetVersion")); impl_fn != nullptr) { // NOLINT
            return impl_fn(version_info);
        } else {
            return -1;
        }
    }
}
#endif

// 硬件检查
namespace rainy::core::builtin {
    RAINY_TOOLKIT_API void cpuid(int query[4], int function_id) {
#if RAINY_USING_MSVC
        __cpuid(query, function_id);
#else
        (void) query;
        (void) function_id;
#endif
    }

    bool has_instruction(instruction_set check) {
        constexpr int eax = 0;
        constexpr int ebx = 1;
        constexpr int ecx = 2;

        int cpu_info[4]{};
        int leaf = 1;
        int reg_idx = ecx; // 默认使用ecx寄存器
        int bit_idx = 0;

        switch (check) {
            case instruction_set::sse3:
                bit_idx = 0;
                break;
            case instruction_set::pclmulqdq:
                bit_idx = 1;
                break;
            case instruction_set::monitor:
                bit_idx = 3;
                break;
            case instruction_set::ssse3:
                bit_idx = 9;
                break;
            case instruction_set::fma:
                bit_idx = 12;
                break;
            case instruction_set::cmpxchg16b:
                bit_idx = 13;
                break;
            case instruction_set::sse41:
                bit_idx = 19;
                break;
            case instruction_set::sse42:
                bit_idx = 20;
                break;
            case instruction_set::movbe:
                bit_idx = 22;
                break;
            case instruction_set::popcnt:
                bit_idx = 23;
                break;
            case instruction_set::aes:
                bit_idx = 25;
                break;
            case instruction_set::xsave:
                bit_idx = 26;
                break;
            case instruction_set::osxsave:
                bit_idx = 27;
                break;
            case instruction_set::avx:
                bit_idx = 28;
                break;
            case instruction_set::f16c:
                bit_idx = 29;
                break;
            case instruction_set::rdrand:
                bit_idx = 30;
                break;
            case instruction_set::msr:
                bit_idx = 31;
                break;
            case instruction_set::cx8:
                bit_idx = 8;
                reg_idx = ebx; // 使用ebx
                break;
            case instruction_set::sep:
                bit_idx = 11;
                break;
            case instruction_set::cmov:
                bit_idx = 15;
                break;
            case instruction_set::clflush:
                bit_idx = 19;
                break;
            case instruction_set::mmx:
                bit_idx = 23;
                break;
            case instruction_set::fxsr:
                bit_idx = 24;
                break;
            case instruction_set::sse:
                bit_idx = 25;
                break;
            case instruction_set::sse2:
                bit_idx = 26;
                break;
            case instruction_set::fsgsbase:
                bit_idx = 0;
                reg_idx = eax; // 使用eax
                break;
            case instruction_set::bmi1:
                bit_idx = 3;
                break;
            case instruction_set::hle:
                bit_idx = 4;
                break;
            case instruction_set::avx2:
                bit_idx = 5;
                leaf = 7;
                reg_idx = ebx; // 使用ebx
                break;
            case instruction_set::bmi2:
                bit_idx = 8;
                break;
            case instruction_set::erms:
                bit_idx = 9;
                break;
            case instruction_set::invpcid:
                bit_idx = 10;
                break;
            case instruction_set::rtm:
                bit_idx = 11;
                break;
            case instruction_set::avx512f:
                bit_idx = 16;
                leaf = 7;
                break;
            case instruction_set::rdseed:
                bit_idx = 18;
                break;
            case instruction_set::adx:
                bit_idx = 19;
                break;
            case instruction_set::avx512pf:
                bit_idx = 26;
                leaf = 7;
                break;
            case instruction_set::avx512er:
                bit_idx = 27;
                leaf = 7;
                break;
            case instruction_set::avx512cd:
                bit_idx = 28;
                leaf = 7;
                break;
            case instruction_set::sha:
                bit_idx = 29;
                break;
            case instruction_set::prefetchwt1:
                bit_idx = 30;
                break;
            case instruction_set::lahf:
                bit_idx = 0;
                reg_idx = eax; // 使用eax
                break;
            case instruction_set::lzcnt:
                bit_idx = 5;
                break;
            case instruction_set::abm:
                bit_idx = 5;
                break;
            case instruction_set::sse4a:
                bit_idx = 6;
                break;
            case instruction_set::xop:
                bit_idx = 11;
                break;
            case instruction_set::tbm:
                bit_idx = 21;
                break;
            case instruction_set::syscall:
                bit_idx = 11;
                break;
            case instruction_set::mmxext:
                bit_idx = 22;
                break;
            case instruction_set::rdtscp:
                bit_idx = 27;
                break;
            case instruction_set::_3dnowext:
                bit_idx = 31;
                break;
            case instruction_set::_3dnow:
                bit_idx = 30;
                break;
            default:
                return false;
        }
        cpuid(cpu_info, leaf);
        int reg = cpu_info[reg_idx];
        return (reg & (1 << bit_idx)) != 0;
    }

    errno_t get_vendor(char *buffer) {
        if (!buffer) {
            return EINVAL;
        }
        int cpu_info[4]{0};
        cpuid(cpu_info, 0);
        char vendor[0x20]{};
        *reinterpret_cast<int *>(vendor) = cpu_info[1];
        *reinterpret_cast<int *>(vendor + 4) = cpu_info[3];
        *reinterpret_cast<int *>(vendor + 8) = cpu_info[2];
        builtin::copy_memory(buffer, vendor, sizeof(vendor));
        return 0;
    }

    errno_t get_brand(char *buffer) {
        if (!buffer) {
            return EINVAL;
        }
        int cpu_info[4] = {0};
        cpuid(cpu_info, 0x80000000);
        int exids = cpu_info[0];
        if (exids < 0x80000004) {
            return EACCES;
        }
        char brand[0x40]{};
        for (int i = 0x80000002; i <= 0x80000004; ++i) {
            cpuid(cpu_info, i);
            core::builtin::copy_memory(brand + (i - 0x80000002) * 16, cpu_info, sizeof(cpu_info));
        }
        core::builtin::copy_memory(buffer, brand, sizeof(brand));
        return 0;
    }

    RAINY_TOOLKIT_API std::size_t hardware_concurrency() {
#if RAINY_USING_WINDOWS
        SYSTEM_INFO sysinfo{};
        GetSystemInfo(&sysinfo);
        return sysinfo.dwNumberOfProcessors;
#else
        return sysconf(_SC_NPROCESSORS_ONLN);
#endif
    }

    version get_os_version() {
#if RAINY_USING_WINDOWS
        implements::rtl_osversion_info version_info{};
        implements::get_version_impl(&version_info);
        if (version_info.dwMajorVersion == 10) {
            if (version_info.dwBuildNumber >= 22000) {
                return version::windows11;
            } else {
                return version::windows10;
            }
        } else if (version_info.dwMajorVersion == 6) {
            if (version_info.dwMinorVersion == 3 && version_info.dwBuildNumber >= 9600) {
                return version::windows8_1;
            } else if (version_info.dwMinorVersion == 2 && version_info.dwBuildNumber >= 9200) {
                return version::windows8;
            } else if (version_info.dwMinorVersion == 1) {
                if (version_info.dwBuildNumber >= 7601) {
                    return version::windows7sp1;
                } else {
                    return version::windows7;
                }
            } else {
                if (IsWindowsServer()) {
                    return version::windows_server;
                }
            }
        }
        return version::unknown;
#elif RAINY_USING_LINUX
        return version::linux_like;
#endif
    }

    errno_t get_os_name(char *buffer) {
        if (!buffer) {
            return EINVAL;
        }
#if RAINY_USING_WINDOWS
        version ver = get_os_version();
        switch (ver) {
            case version::windows11: {
                constexpr char sysname[] = "Microsoft Windows 11";
                core::builtin::copy_memory(buffer, sysname, sizeof(sysname));
                break;
            }
            case version::windows10: {
                constexpr char sysname[] = "Microsoft Windows 10";
                core::builtin::copy_memory(buffer, sysname, sizeof(sysname));
                break;
            }
            case version::windows8_1: {
                constexpr char sysname[] = "Microsoft Windows 8_1";
                core::builtin::copy_memory(buffer, sysname, sizeof(sysname));
                break;
            }
            case version::windows8: {
                constexpr char sysname[] = "Microsoft Windows 8";
                core::builtin::copy_memory(buffer, sysname, sizeof(sysname));
                break;
            }
            case version::windows7sp1: {
                constexpr char sysname[] = "Microsoft Windows 7 SP1";
                core::builtin::copy_memory(buffer, sysname, sizeof(sysname));
                break;
            }
            case version::windows7: {
                constexpr char sysname[] = "Microsoft Windows 7";
                core::builtin::copy_memory(buffer, sysname, sizeof(sysname));
                break;
            }
            case version::windows_server: {
                constexpr char sysname[] = "Microsoft Windows Server";
                core::builtin::copy_memory(buffer, sysname, sizeof(sysname));
                break;
            }
            case version::unknown:
            default: {
                constexpr char sysname[] = "Microsoft Windows";
                core::builtin::copy_memory(buffer, sysname, sizeof(sysname));
                return EINVAL;
            }
        }
        return 0;
#else
        constexpr char sysname[] = "GNU/Linux";
        core::builtin::copy_memory(buffer, sysname, sizeof(sysname));
        return 0;
#endif
    }
}
