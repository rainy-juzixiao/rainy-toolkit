#ifndef RAINY_CORE_SIMD_REGISTER_HPP
#define RAINY_CORE_SIMD_REGISTER_HPP

#include <rainy/core/simd/fwd.hpp>
#include <rainy/core/simd/native_abi.hpp>
#include <rainy/core/simd/implements/register_scalar.hpp>

#if RAINY_IS_ARM64
#include <rainy/core/simd/implements/register_arm64.hpp>
#elif RAINY_IS_X86_PLATFORM
#include <rainy/core/simd/implements/register_x86.hpp>
#endif

#endif