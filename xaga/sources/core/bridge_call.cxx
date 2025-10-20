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
#include <cstdarg>
#include <mutex>
#include <rainy/core/platform.hpp>
#include <unordered_map>

using namespace rainy;
using namespace rainy::core;
using namespace rainy::core::abi;

static constexpr inline std::size_t bridge_calltable_size = 128;
static inline constexpr std::size_t fnv_offset_basis = static_cast<std::size_t>(14695981039346656037ULL);
static inline constexpr std::size_t fnv_prime = static_cast<std::size_t>(1099511628211ULL);

std::once_flag once_flag;
std::mutex rainy_toolkit_abi_lock;

struct library_context {
    struct detail_impl {
        const char *version_name{rainy_toolkit_version_name};
        const char *build_date{build_date};
        int major{RAINY_TOOLKIT_PROJECT_MAJOR};
        int minor{RAINY_TOOLKIT_PROJECT_MINOR};
        int patch{RAINY_TOOLKIT_PROJECT_PATCH};
    } detail;

    static constexpr inline std::size_t total_member_count{4};

    std::size_t hash{0};
};

static std::size_t eval_for_library_hash(const library_context *context) {
    auto hasher = [](const core::byte_t *cur, std::size_t length) -> std::size_t {
        std::size_t hash = fnv_offset_basis;
        for (std::size_t idx = 0; idx != length; ++idx) {
            hash ^= static_cast<std::size_t>(cur[idx]);
            hash *= fnv_prime;
            ++cur;
        }
        return hash;
    };
    std::size_t unique_hash{0};
    {
        unique_hash +=
            hasher(reinterpret_cast<const core::byte_t *>(context->detail.version_name), sizeof(rainy_toolkit_version_name));
        unique_hash += hasher(reinterpret_cast<const core::byte_t *>(context->detail.build_date), sizeof(build_date));
    }
    {
        const int hash_seq[3] = {context->detail.major, context->detail.minor, context->detail.patch};
        unique_hash += hasher(reinterpret_cast<const core::byte_t *>(hash_seq), sizeof(hash_seq));
    }
    unique_hash += sizeof(library_context::detail) * library_context::total_member_count;
    return unique_hash;
}

// RAINY_ABI_BRIDGE_CALL_HANDLER_TOTAL_COUNT
static long rainy_toolkit_abi_bridge_call_handler_count_impl(std::intptr_t pointer, std::intptr_t, std::intptr_t, std::intptr_t,
                                                             std::intptr_t, std::intptr_t) {
    rainy_let p = reinterpret_cast<long *>(pointer);
    (*p) = bridge_calltable_size;
    return 0;
}

// RAINY_ABI_BRIDGE_CALL_GET_VERSION
static long rainy_toolkit_abi_bridge_get_version_impl(std::intptr_t request, std::intptr_t recv, std::intptr_t, std::intptr_t,
                                                      std::intptr_t, std::intptr_t) {
    rainy_let bridge_ver = reinterpret_cast<bridge_version *>(request);
    rainy_let recv_var = reinterpret_cast<int *>(recv);
    switch (*bridge_ver) {
        case bridge_version::major:
            *recv_var = RAINY_TOOLKIT_PROJECT_MAJOR;
            break;
        case bridge_version::minor:
            *recv_var = RAINY_TOOLKIT_PROJECT_MINOR;
            break;
        case bridge_version::patch:
            *recv_var = RAINY_TOOLKIT_PROJECT_PATCH;
            break;
    }
    return 0;
}

// RAINY_ABI_BRIDGE_CALL_GET_FULLVERSION
static long rainy_toolkit_abi_bridge_get_fullversion_impl(std::intptr_t recv, std::intptr_t, std::intptr_t, std::intptr_t,
                                                          std::intptr_t, std::intptr_t) {
    rainy_let recv_var = reinterpret_cast<version *>(recv);
    if (!recv_var) {
        return EINVAL;
    }
    recv_var->major = RAINY_TOOLKIT_PROJECT_MAJOR;
    recv_var->minor = RAINY_TOOLKIT_PROJECT_MINOR;
    recv_var->patch = RAINY_TOOLKIT_PROJECT_PATCH;
    return 0;
}

// RAINY_ABI_BRIDGE_CALL_GET_COMPILE_STANDARD
static long rainy_toolkit_abi_bridge_get_compile_standard_impl(std::intptr_t recv, std::intptr_t, std::intptr_t, std::intptr_t,
                                                               std::intptr_t, std::intptr_t) {
    rainy_let recv_var = reinterpret_cast<standard *>(recv);
    if (!recv_var) {
        return EINVAL;
    }
#if RAINY_HAS_CXX23

#if RAINY_CURRENT_STANDARD_VERSION > 202302L
    *recv_var = standard::cxxlatest;
#else
    *recv_var = standard::cxx23;
#endif

#elif RAINY_HAS_CXX20
    *recv_var = standard::cxx20;
#elif RAINY_HAS_CXX17
    *recv_var = standard::cxx17;
#endif
    return 0;
}

// RAINY_ABI_BRIDGE_CALL_GET_COMPILE_IDENTIFIER
static long rainy_toolkit_abi_bridge_get_compile_identifer_impl(std::intptr_t recv, std::intptr_t, std::intptr_t, std::intptr_t,
                                                               std::intptr_t, std::intptr_t) {
    rainy_let recv_var = reinterpret_cast<compiler_identifier *>(recv);
    if (!recv_var) {
        return EINVAL;
    }
#if RAINY_USING_MSVC
    *recv_var = compiler_identifier::msvc;
#elif RAINY_USING_CLANG
    *recv_var = compiler_identifier::llvm_clang;
#elif RAINY_USING_GCC
    *recv_var = compiler_identifier::gcc;
#else
    std::terminate();
#endif
    return 0;
}

// RAINY_ABI_BRIDGE_CALL_GET_VERSION_NAME
static long rainy_toolkit_get_version_name_impl(std::intptr_t recv, std::intptr_t buffer_length, std::intptr_t, std::intptr_t,
                                                std::intptr_t, std::intptr_t) {
    rainy_let buffer = reinterpret_cast<char *>(recv);
    rainy_let length = *reinterpret_cast<std::size_t *>(buffer_length);
    std::size_t copy_length = sizeof(rainy_toolkit_version_name);
    if (buffer && copy_length <= length) {
        std::memcpy(buffer, rainy_toolkit_version_name, copy_length);
        buffer[copy_length] = '\0';
    }
    return static_cast<long>(copy_length);
}

abi_bridge_call_func_t rainy_toolkit_abi_bridge_calltable[bridge_calltable_size]{};

static void init_this_table() noexcept {
    rainy_toolkit_abi_bridge_calltable[RAINY_ABI_BRIDGE_CALL_HANDLER_TOTAL_COUNT] = &rainy_toolkit_abi_bridge_call_handler_count_impl;
    rainy_toolkit_abi_bridge_calltable[RAINY_ABI_BRIDGE_CALL_GET_VERSION] = &rainy_toolkit_abi_bridge_get_version_impl;
    rainy_toolkit_abi_bridge_calltable[RAINY_ABI_BRIDGE_CALL_GET_FULLVERSION] = &rainy_toolkit_abi_bridge_get_fullversion_impl;
    rainy_toolkit_abi_bridge_calltable[RAINY_ABI_BRIDGE_CALL_GET_COMPILE_STANDARD] =
        &rainy_toolkit_abi_bridge_get_compile_standard_impl;
    rainy_toolkit_abi_bridge_calltable[RAINY_ABI_BRIDGE_CALL_GET_COMPILE_IDENTIFIER] =
        &rainy_toolkit_abi_bridge_get_compile_identifer_impl;
    rainy_toolkit_abi_bridge_calltable[RAINY_ABI_BRIDGE_CALL_GET_VERSION_NAME] = &rainy_toolkit_get_version_name_impl;
}

namespace rainy::core::abi {
    long rainy_toolkit_abi_bridge_call(long number, ...) {
        std::call_once(once_flag, init_this_table);
        if (number >= 0 && number < bridge_calltable_size) {
            std::lock_guard guard{rainy_toolkit_abi_lock};
            if (abi_bridge_call_func_t fn = rainy_toolkit_abi_bridge_calltable[number]; fn != nullptr) {
                std::intptr_t param[6] = {0, 0, 0, 0, 0, 0};
                va_list paramlist;
                va_start(paramlist, number);
                for (int i = 0; i < 6; ++i) {
                    param[i] = va_arg(paramlist, std::intptr_t);
                }
                va_end(paramlist);
                return fn(param[0], param[1], param[2], param[3], param[4], param[5]);
            }
            errno = ENOEXEC;
            return -1;
        }
        errno = EINVAL;
        return -1;
    }

    long rainy_toolkit_call_handler_total_count(long *total) {
        return rainy_toolkit_abi_bridge_call(RAINY_ABI_BRIDGE_CALL_HANDLER_TOTAL_COUNT, reinterpret_cast<std::intptr_t>(total));
    }

    long rainy_toolkit_get_version(bridge_version bridge_version, int *recv) {
        return rainy_toolkit_abi_bridge_call(RAINY_ABI_BRIDGE_CALL_GET_VERSION, reinterpret_cast<std::intptr_t>(&bridge_version),
                                             reinterpret_cast<std::intptr_t>(recv));
    }

    long rainy_toolkit_get_fullversion(version *recv) {
        return rainy_toolkit_abi_bridge_call(RAINY_ABI_BRIDGE_CALL_GET_FULLVERSION, reinterpret_cast<std::intptr_t>(recv));
    }

    long rainy_toolkit_get_compile_standard(standard *recv) {
        return rainy_toolkit_abi_bridge_call(RAINY_ABI_BRIDGE_CALL_GET_COMPILE_STANDARD, reinterpret_cast<std::intptr_t>(recv));
    }

    long rainy_toolkit_get_compile_identifier(compiler_identifier *recv) {
        return rainy_toolkit_abi_bridge_call(RAINY_ABI_BRIDGE_CALL_GET_COMPILE_IDENTIFIER, reinterpret_cast<std::intptr_t>(recv));
    }

    long rainy_toolkit_get_version_name(char *buffer, std::size_t buffer_length) {
        return rainy_toolkit_abi_bridge_call(RAINY_ABI_BRIDGE_CALL_GET_VERSION_NAME, reinterpret_cast<std::intptr_t>(buffer),
                                             reinterpret_cast<std::intptr_t>(&buffer_length));
    }
}
