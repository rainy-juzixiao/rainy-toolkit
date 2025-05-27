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
#ifndef RAINY_USER_SHA_HPP
#define RAINY_USER_SHA_HPP
#include <iomanip>
#include <iostream>
#include <rainy/core/core.hpp>
#include <rainy/text/format_wrapper.hpp>
#include <sstream>
#include <fstream>
#include <string>
#include <vector>
#include <array>

namespace rainy::user::hash::implements {
    enum class sha_type {
        sha256,
        sha512
    };

    template <sha_type,typename Ty>
    RAINY_INLINE std::string to_hex_string(const Ty (&)[8]) {
        return {};
    }

    template <sha_type>
    std::string make_sha(const std::string &input);

    template <sha_type>
    std::string make_sha_from_file(const std::string_view file_path);

    template <>
    RAINY_INLINE std::string to_hex_string<sha_type::sha256>(const std::uint32_t (&hash)[8]) {
        std::stringstream ss;
        ss << std::hex << std::setfill('0');
        for (int i = 0; i < 8; ++i) {
            // 使用 8 位宽度确保每个数字都是 8 个十六进制字符
            ss << std::setw(8) << hash[i];
        }
        return ss.str();
    }

    template <>
    RAINY_INLINE std::string to_hex_string<sha_type::sha512>(const std::uint64_t (&hash)[8]) {
        std::stringstream ss;
        ss << std::hex << std::setfill('0');
        for (int i = 0; i < 8; ++i) {
            // 使用 16 位宽度确保每个数字都是 8 个十六进制字符
            ss << std::setw(16) << hash[i];
        }
        return ss.str();
    }
}

namespace rainy::user::hash::implements::sha256 {
    static const std::uint32_t constants[64] = {
        0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5, 0xd807aa98, 0x12835b01,
        0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174, 0x49b40821, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
        0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85, 0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624,
        0xf40e3585, 0x106aa070, 0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b1d8e6a, 0x6d9d6122,
        0x758e5f76, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2};

    RAINY_INLINE std::uint32_t right_rotate(std::uint32_t x, std::uint32_t n) {
        if (n == 0)
            return x;
        return (x >> n) | (x << (32 - n));
    }
}

namespace rainy::user::hash::implements::sha512 {
    static const std::uint64_t constants[80] = {
        0x428a2f98d728ae22, 0x7137449123ef65cd, 0xb5c0fbcfec4d3b2f, 0xe9b5dba58189dbbc, 0x3956c25bf348b538, 0x59f111f1b605d019,
        0x923f82a4af194f9b, 0xab1c5ed5da6d8118, 0xd807aa98a3030242, 0x12835b0145706fbe, 0x243185be4ee4b28c, 0x550c7dc3d5ffb4e2,
        0x72be5d74f27b896f, 0x80deb1fe3b1696b1, 0x9bdc06a725c71235, 0xc19bf174cf692694, 0x49b40821f83d93da, 0x2e1b21385c26c926,
        0x4d2c6dfc5ac42aed, 0x53380d139d95b3df, 0x650a73548baf63de, 0x766a0abb3c77b2a8, 0x81c2c92e47edaee6, 0x92722c85d8baf7d9,
        0xa2bfe8a14cf10364, 0xa81a664bbc423001, 0xc24b8b70d0f89791, 0xc76c51a3d2c6f539, 0xd192e819d6ef5218, 0xd69906245565a910,
        0xf40e35855771202a, 0x106aa07032bbd1b8, 0x19a4c116b8d2d0c8, 0x1e376c085141ab53, 0x2748774cdf8eeb99, 0x34b0bcb5e19b48a8,
        0x391c0cb3c5c95a63, 0x4ed8aa4ae9137d48, 0x5b1d8e6aa94f8e24, 0x6d9d6122c7e3a17d, 0x758e5f76c3e47d84, 0x78a5636f43172f60,
        0x84c87814a1f0ab72, 0x8cc702081a6439ec, 0x90befffa23631e28, 0xa4506cebde82bde9, 0xbef9a3f7b2c67915, 0xc67178f2e372532b,
        0xca273eceea26619c, 0xd186b8c721c0c207, 0xeada7dd6cde0eb1e, 0xf57d4f7fee6ed178, 0x06f067aa72176fba, 0x0a637dc5a2c898a6,
        0x113f9804bef90dae, 0x1b710b35131c471b, 0x28db77f523047d84, 0x32caab7b40c72493, 0x3c9ebe0a15c9bebc, 0x431d67c49c100d4c,
        0x4cc5d4becb3e42b6, 0x597f299cfc657e2a, 0x5fcb6fab3ad6faec, 0x6c44198c4a475817};

    RAINY_INLINE std::uint64_t right_rotate(std::uint64_t x, std::uint64_t n) {
        return (x >> n) | (x << (64 - n));
    }
}

namespace rainy::user::hash::implements::sha256 {
    RAINY_INLINE void preprocess_message(const std::string &input, std::vector<uint8_t> &message);
    RAINY_INLINE void compress(std::uint32_t (&state)[8], const std::vector<uint8_t> &message_block);
    RAINY_INLINE void preprocess_message_from_file(const std::string_view file_path, std::vector<uint8_t> &message);
}

namespace rainy::user::hash::implements::sha512 {
    RAINY_INLINE void preprocess_message(const std::string &input, std::vector<uint8_t> &message);
    RAINY_INLINE void compress(uint64_t (&state)[8], const std::vector<std::uint8_t> &message_block);
    RAINY_INLINE void preprocess_message_from_file(const std::string_view file_path, std::vector<uint8_t> &message);
}

namespace rainy::user::hash::implements {
    template <>
    RAINY_INLINE std::string make_sha<sha_type::sha256>(const std::string &input) {
        std::uint32_t state[8] = {0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a, 0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19};
        std::vector<std::uint8_t> message;
        sha256::preprocess_message(input, message);
        for (std::size_t i = 0; i < message.size(); i += 64) {
            std::vector<std::uint8_t> block(message.begin() + i, message.begin() + i + 64);
            sha256::compress(state, block);
        }
        return to_hex_string<sha_type::sha256>(state);
    }

    template <>
    RAINY_INLINE std::string make_sha<sha_type::sha512>(const std::string &input) {
        std::uint64_t state[8] = {0x6a09e667f3bcc908, 0xbb67ae8584caa73b, 0x3c6ef372fe94f82b, 0xa54ff53a5f1d36f1,
                                  0x510e527fade682d1, 0x9b05688c2b3e6c1f, 0x1f83d9abfb41bd6b, 0x5be0cd19137e2179};
        std::vector<std::uint8_t> message;
        sha512::preprocess_message(input, message);
        for (std::size_t i = 0; i < message.size(); i += 128) {
            std::vector<std::uint8_t> block(message.begin() + i, message.begin() + i + 128);
            sha512::compress(state, block);
        }
        return to_hex_string<sha_type::sha512>(state);
    }

    template <>
    RAINY_INLINE std::string make_sha_from_file<sha_type::sha256>(const std::string_view file_path) {
        std::uint32_t state[8] = {0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a, 0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19};
        std::vector<std::uint8_t> message;
        sha256::preprocess_message_from_file(file_path, message);
        if (message.empty()) {
            return {}; // 不进行处理
        }
        for (std::size_t i = 0; i < message.size(); i += 64) {
            std::vector<std::uint8_t> block(message.begin() + i, message.begin() + i + 64);
            sha256::compress(state, block);
        }
        return to_hex_string<sha_type::sha256>(state);
    }

    template <>
    RAINY_INLINE std::string make_sha_from_file<sha_type::sha512>(const std::string_view file_path) {
        std::uint64_t state[8] = {0x6a09e667f3bcc908, 0xbb67ae8584caa73b, 0x3c6ef372fe94f82b, 0xa54ff53a5f1d36f1,
                                  0x510e527fade682d1, 0x9b05688c2b3e6c1f, 0x1f83d9abfb41bd6b, 0x5be0cd19137e2179};
        std::vector<std::uint8_t> message;
        sha512::preprocess_message_from_file(file_path, message);
        if (message.empty()) {
            return {}; // 不进行处理
        }
        for (std::size_t i = 0; i < message.size(); i += 128) {
            std::vector<std::uint8_t> block(message.begin() + i, message.begin() + i + 128);
            sha512::compress(state, block);
        }
        return to_hex_string<sha_type::sha512>(state);
    }
}

#endif