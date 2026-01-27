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

#include <bitset>

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

namespace rainy::core::builtin {
    class instruction_set_internal {
    public:
        static constexpr std::size_t max_ids = 32;

        instruction_set_internal() :
            is_intel(false), is_amd(false), f_1_ecx(0), f_1_edx(0), f_7_ebx(0), f_7_ecx(0), f_81_ecx(0), f_81_edx(0) {
            std::array<int, 4> cpui{};
            std::array<std::array<int, 4>, max_ids> data{};
            cpuid(cpui.data(), 0);
            n_ids = cpui[0];
            for (int i = 0; i <= n_ids; ++i) {
                cpuidex(cpui.data(), i, 0);
                data[i] = cpui;
            }
            char vendor[0x20] = {0};
            if (std::strcmp(vendor, "GenuineIntel") == 0) {
                is_intel = true;
            } else if (std::strcmp(vendor, "AuthenticAMD") == 0) {
                is_amd = true;
            }
            if (n_ids >= 1) {
                f_1_ecx = data[1][2];
                f_1_edx = data[1][3];
            }
            if (n_ids >= 7) {
                f_7_ebx = data[7][1];
                f_7_ecx = data[7][2];
            }
            cpuid(cpui.data(), 0x80000000);
            n_ex_ids = cpui[0];

        }

        bool is_intel;
        bool is_amd;
        std::bitset<32> f_1_ecx;
        std::bitset<32> f_1_edx;
        std::bitset<32> f_7_ebx;
        std::bitset<32> f_7_ecx;
        std::bitset<32> f_81_ecx;
        std::bitset<32> f_81_edx;
        int n_ids;
        int n_ex_ids;
    };

    class instruction_set_impl {
    public:
        static bool sse3() {
            return cpu_rep.f_1_ecx[0];
        }

        static bool pclmulqdq() {
            return cpu_rep.f_1_ecx[1];
        }
        
        static bool monitor() {
            return cpu_rep.f_1_ecx[3];
        }
        
        static bool ssse3() {
            return cpu_rep.f_1_ecx[9];
        }
        
        static bool fma() {
            return cpu_rep.f_1_ecx[12];
        }
        
        static bool cmpxchg16b() {
            return cpu_rep.f_1_ecx[13];
        }
        
        static bool sse41() {
            return cpu_rep.f_1_ecx[19];
        }
        
        static bool sse42() {
            return cpu_rep.f_1_ecx[20];
        }
        
        static bool movbe() {
            return cpu_rep.f_1_ecx[22];
        }
        
        static bool popcnt() {
            return cpu_rep.f_1_ecx[23];
        }
        
        static bool aes() {
            return cpu_rep.f_1_ecx[25];
        }
        
        static bool xsave() {
            return cpu_rep.f_1_ecx[26];
        }
        
        static bool osxsave() {
            return cpu_rep.f_1_ecx[27];
        }
        
        static bool avx() {
            return cpu_rep.f_1_ecx[28];
        }
        
        static bool f16c() {
            return cpu_rep.f_1_ecx[29];
        }
        
        static bool rdrand() {
            return cpu_rep.f_1_ecx[30];
        }

        static bool msr() {
            return cpu_rep.f_1_edx[5];
        }
        
        static bool cx8() {
            return cpu_rep.f_1_edx[8];
        }
        
        static bool sep() {
            return cpu_rep.f_1_edx[11];
        }
        
        static bool cmov() {
            return cpu_rep.f_1_edx[15];
        }
        
        static bool clfsh() {
            return cpu_rep.f_1_edx[19];
        }
        
        static bool mmx() {
            return cpu_rep.f_1_edx[23];
        }
        
        static bool fxsr() {
            return cpu_rep.f_1_edx[24];
        }

        static bool sse() {
            return cpu_rep.f_1_edx[25];
        }
        
        static bool sse2() {
            return cpu_rep.f_1_edx[26];
        }

        static bool fsgsbase() {
            return cpu_rep.f_7_ebx[0];
        }
        
        static bool bmi1() {
            return cpu_rep.f_7_ebx[3];
        }
        
        static bool hle() {
            return cpu_rep.is_intel && cpu_rep.f_7_ebx[4];
        }
        
        static bool avx2() {
            return cpu_rep.f_7_ebx[5];
        }
        
        static bool bmi2() {
            return cpu_rep.f_7_ebx[8];
        }
        
        static bool erms() {
            return cpu_rep.f_7_ebx[9];
        }
        
        static bool invpcid() {
            return cpu_rep.f_7_ebx[10];
        }
        
        static bool rtm() {
            return cpu_rep.is_intel && cpu_rep.f_7_ebx[11];
        }
        
        static bool avx512f() {
            return cpu_rep.f_7_ebx[16];
        }
        
        static bool rdseed() {
            return cpu_rep.f_7_ebx[18];
        }
        
        static bool adx() {
            return cpu_rep.f_7_ebx[19];
        }
        
        static bool avx512pf() {
            return cpu_rep.f_7_ebx[26];
        }
        static bool avx512er() {
            return cpu_rep.f_7_ebx[27];
        }
        static bool avx512cd() {
            return cpu_rep.f_7_ebx[28];
        }
        static bool sha() {
            return cpu_rep.f_7_ebx[29];
        }

        static bool prefetchwt1() {
            return cpu_rep.f_7_ecx[0];
        }

        static bool lahf() {
            return cpu_rep.f_81_ecx[0];
        }
        static bool lzcnt() {
            return cpu_rep.is_intel && cpu_rep.f_81_ecx[5];
        }
        static bool abm() {
            return cpu_rep.is_amd && cpu_rep.f_81_ecx[5];
        }
        static bool sse4a() {
            return cpu_rep.is_amd && cpu_rep.f_81_ecx[6];
        }
        static bool xop() {
            return cpu_rep.is_amd && cpu_rep.f_81_ecx[11];
        }
        static bool tbm() {
            return cpu_rep.is_amd && cpu_rep.f_81_ecx[21];
        }

        static bool syscall() {
            return cpu_rep.is_intel && cpu_rep.f_81_edx[11];
        }
        
        static bool mmxext() {
            return cpu_rep.is_amd && cpu_rep.f_81_edx[22];
        }
        
        static bool rdtscp() {
            return cpu_rep.is_intel && cpu_rep.f_81_edx[27];
        }
        
        static bool _3dnowext() {
            return cpu_rep.is_amd && cpu_rep.f_81_edx[30];
        }

        static bool _3dnow() {
            return cpu_rep.is_amd && cpu_rep.f_81_edx[31];
        }

        static bool hypervisor() {
            return cpu_rep.f_1_ecx[31];
        }

    private:
        static const instruction_set_internal cpu_rep;
    };

    const instruction_set_internal instruction_set_impl::cpu_rep;
}

// 硬件检查
namespace rainy::core::builtin {
    void cpuid(int query[4], int function_id) {
#if RAINY_USING_MSVC && !RAINY_IS_ARM64
        __cpuid(query, function_id);
#else
        (void) query;
        (void) function_id;
#endif
    }

    void cpuidex(int query[4], int function_id, int subfunction_id) {
#if RAINY_USING_MSVC && !RAINY_IS_ARM64
        __cpuidex(query, function_id, subfunction_id);
#else
        (void) query;
        (void) function_id;
        (void) subfunction_id;
#endif
    }


    bool has_instruction(instruction_set check) {
        switch (check) {
            case instruction_set::sse3:
                return instruction_set_impl::sse3();
            case instruction_set::pclmulqdq:
                return instruction_set_impl::pclmulqdq();
            case instruction_set::monitor:
                return instruction_set_impl::monitor();
            case instruction_set::ssse3:
                return instruction_set_impl::ssse3();
            case instruction_set::fma:
                return instruction_set_impl::fma();
            case instruction_set::cmpxchg16b:
                return instruction_set_impl::cmpxchg16b();
            case instruction_set::sse41:
                return instruction_set_impl::sse41();
            case instruction_set::sse42:
                return instruction_set_impl::sse42();
            case instruction_set::movbe:
                return instruction_set_impl::movbe();
            case instruction_set::popcnt:
                return instruction_set_impl::popcnt();
            case instruction_set::aes:
                return instruction_set_impl::aes();
            case instruction_set::xsave:
                return instruction_set_impl::xsave();
            case instruction_set::osxsave:
                return instruction_set_impl::osxsave();
            case instruction_set::avx:
                return instruction_set_impl::avx();
            case instruction_set::f16c:
                return instruction_set_impl::f16c();
            case instruction_set::rdrand:
                return instruction_set_impl::rdrand();
            case instruction_set::msr:
                return instruction_set_impl::msr();
            case instruction_set::cx8:
                return instruction_set_impl::cx8();
            case instruction_set::sep:
                return instruction_set_impl::sep();
            case instruction_set::cmov:
                return instruction_set_impl::cmov();
            case instruction_set::clflush:
                return instruction_set_impl::clfsh();
            case instruction_set::mmx:
                return instruction_set_impl::mmx();
            case instruction_set::fxsr:
                return instruction_set_impl::fxsr();
            case instruction_set::sse:
                return instruction_set_impl::sse();
            case instruction_set::sse2:
                return instruction_set_impl::sse2();
            case instruction_set::fsgsbase:
                return instruction_set_impl::fsgsbase();
            case instruction_set::bmi1:
                return instruction_set_impl::bmi1();
            case instruction_set::hle:
                return instruction_set_impl::hle();
            case instruction_set::avx2:
                return instruction_set_impl::avx2();                
            case instruction_set::bmi2:
                return instruction_set_impl::bmi2();
            case instruction_set::erms:
                return instruction_set_impl::erms();
            case instruction_set::invpcid:
                return instruction_set_impl::invpcid();
            case instruction_set::rtm:
                return instruction_set_impl::rtm();
            case instruction_set::avx512f:
                return instruction_set_impl::avx512f();
            case instruction_set::rdseed:
                return instruction_set_impl::rdseed();
            case instruction_set::adx:
                return instruction_set_impl::adx();
            case instruction_set::avx512pf:
                return instruction_set_impl::avx512pf();
            case instruction_set::avx512er:
                return instruction_set_impl::avx512er();
            case instruction_set::avx512cd:
                return instruction_set_impl::avx512cd();
            case instruction_set::sha:
                return instruction_set_impl::sha();
            case instruction_set::prefetchwt1:
                return instruction_set_impl::prefetchwt1();
            case instruction_set::lahf:
                return instruction_set_impl::lahf();
            case instruction_set::lzcnt:
                return instruction_set_impl::lzcnt();
            case instruction_set::abm:
                return instruction_set_impl::abm();
            case instruction_set::sse4a:
                return instruction_set_impl::sse4a();
            case instruction_set::xop:
                return instruction_set_impl::xop();
            case instruction_set::tbm:
                return instruction_set_impl::tbm();
            case instruction_set::syscall:
                return instruction_set_impl::syscall();
            case instruction_set::mmxext:
                return instruction_set_impl::mmxext();
            case instruction_set::rdtscp:
                return instruction_set_impl::rdtscp();
            case instruction_set::_3dnowext:
                return instruction_set_impl::_3dnowext();
            case instruction_set::_3dnow:
                return instruction_set_impl::_3dnow();
            case instruction_set::hypervisor:
                return instruction_set_impl::hypervisor();
            default:
                return false;
        }
    }

    bool is_hypervisor() {
        return has_instruction(instruction_set::hypervisor);
    }

    errno_t get_vendor(char *buffer, std::size_t length) {
        if (!buffer) {
            return EINVAL;
        }
        int cpu_info[4]{0};
        cpuid(cpu_info, 0);
        char vendor[0x20]{};
        *reinterpret_cast<int *>(vendor) = cpu_info[1];
        *reinterpret_cast<int *>(vendor + 4) = cpu_info[3];
        *reinterpret_cast<int *>(vendor + 8) = cpu_info[2];
        std::size_t brand_length = std::strlen(vendor);
        if (brand_length > length) {
            return E2BIG;
        }
        builtin::copy_memory(buffer, vendor, sizeof(vendor));
        return 0;
    }

    errno_t get_brand(char *buffer, std::size_t length) {
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
        std::size_t brand_length = std::strlen(brand);
        if (brand_length > length) {
            return E2BIG;
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
