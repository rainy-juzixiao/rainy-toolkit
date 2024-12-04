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

namespace rainy::user::hash::internals {
    enum class sha_type {
        sha256,
        sha512
    };

    template <sha_type,typename Ty>
    RAINY_INLINE std::string to_hex_string(const Ty (&hash)[8]) {
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

// 此部分详细实现请参见 rainy/sources/sha/impl_sha.cxx 
namespace rainy::user::hash::internals::sha256 {
    std::uint32_t right_rotate(std::uint32_t x, std::uint32_t n);
    void preprocess_message(const std::string &input, std::vector<uint8_t> &message);
    void compress(std::uint32_t (&state)[8], const std::vector<uint8_t> &message_block);
    void preprocess_message_from_file(const std::string_view file_path, std::vector<uint8_t> &message);
}

namespace rainy::user::hash::internals::sha512 {
    std::uint64_t right_rotate(std::uint64_t x, std::uint64_t n);
    void preprocess_message(const std::string &input, std::vector<uint8_t> &message);
    void compress(uint64_t (&state)[8], const std::vector<std::uint8_t> &message_block);
    void preprocess_message_from_file(const std::string_view file_path, std::vector<uint8_t> &message);
}

namespace rainy::user::hash::internals {
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