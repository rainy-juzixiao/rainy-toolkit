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
#include <rainy/user/hash/sha.hpp>

namespace rainy::user::hash::implements::sha512 {
    RAINY_INLINE void preprocess_message(const std::string &input, std::vector<uint8_t> &message) {
        message.clear();
        // 原始消息转换
        for (char c: input) {
            message.push_back(static_cast<uint8_t>(c));
        }
        // 添加填充位
        std::uint64_t bit_length = input.length() * 8;
        message.push_back(0x80); // 添加单个 1 位
        // 计算需要填充的零字节数量
        std::size_t padding_zero_count = (128 - ((input.length() + 1 + 16) % 128)) % 128;
        message.insert(message.end(), padding_zero_count, 0x00);
        // 添加原始消息长度（大端序）
        for (int i = 7; i >= 0; --i) {
            message.push_back(static_cast<uint8_t>((bit_length >> (i * 8)) & 0xFF));
        }
        message.insert(message.end(), 8, 0x00);
    }

    void compress(uint64_t (&state)[8], const std::vector<std::uint8_t> &message_block) {
        std::uint64_t word[80]{}; // 确保大端序转换
        for (int i = 0; i < 16; ++i) {
            word[i] = 0;
            for (int j = 0; j < 8; ++j) {
                word[i] = (word[i] << 8) | static_cast<std::uint64_t>(message_block[i * 8 + j]);
            }
        }
        for (int i = 16; i < 80; ++i) { // 消息扩展
            uint64_t s0 = right_rotate(word[i - 15], 1) ^ right_rotate(word[i - 15], 8) ^ (word[i - 15] >> 7);
            uint64_t s1 = right_rotate(word[i - 2], 19) ^ right_rotate(word[i - 2], 61) ^ (word[i - 2] >> 6);
            word[i] = word[i - 16] + s0 + word[i - 7] + s1;
        }
        // 压缩函数工作变量
        std::uint64_t a = state[0];
        std::uint64_t b = state[1];
        std::uint64_t c = state[2];
        std::uint64_t d = state[3];
        std::uint64_t e = state[4];
        std::uint64_t f = state[5];
        std::uint64_t g = state[6];
        std::uint64_t h = state[7];
        for (int i = 0; i < 80; ++i) {
            std::uint64_t S1 = right_rotate(e, 14) ^ right_rotate(e, 18) ^ right_rotate(e, 41);
            std::uint64_t ch = (e & f) ^ ((~e) & g);
            std::uint64_t temp1 = h + S1 + ch + constants[i] + word[i];
            std::uint64_t S0 = right_rotate(a, 28) ^ right_rotate(a, 34) ^ right_rotate(a, 39);
            std::uint64_t maj = (a & b) ^ (a & c) ^ (b & c);
            std::uint64_t temp2 = S0 + maj;
            h = g;
            g = f;
            f = e;
            e = d + temp1;
            d = c;
            c = b;
            b = a;
            a = temp1 + temp2;
        }
        state[0] += a;
        state[1] += b;
        state[2] += c;
        state[3] += d;
        state[4] += e;
        state[5] += f;
        state[6] += g;
        state[7] += h;
    }

    RAINY_INLINE void preprocess_message_from_file(const std::string_view file_path, std::vector<uint8_t> &message) {
        std::ifstream file(file_path.data(), std::ios::binary);
        if (!file) {
            return;
        }
        message.assign(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
    }
}

namespace rainy::user::hash::implements::sha256 {
    RAINY_INLINE void preprocess_message(const std::string &input, std::vector<uint8_t> &message) {
        std::size_t input_len = input.size();
        std::size_t padded_len = ((input_len + 8 + 63) / 64) * 64;
        // 消息缓冲区填充
        message.resize(padded_len);
        std::memcpy(message.data(), input.data(), input_len);
        // 添加 '1' 位
        message[input_len] = 0x80;
        // 添加消息长度（64 位表示）
        std::uint64_t length_bits = input_len * 8;
        message[padded_len - 8] = static_cast<uint8_t>((length_bits >> 56) & 0xFF);
        message[padded_len - 7] = static_cast<uint8_t>((length_bits >> 48) & 0xFF);
        message[padded_len - 6] = static_cast<uint8_t>((length_bits >> 40) & 0xFF);
        message[padded_len - 5] = static_cast<uint8_t>((length_bits >> 32) & 0xFF);
        message[padded_len - 4] = static_cast<uint8_t>((length_bits >> 24) & 0xFF);
        message[padded_len - 3] = static_cast<uint8_t>((length_bits >> 16) & 0xFF);
        message[padded_len - 2] = static_cast<uint8_t>((length_bits >> 8) & 0xFF);
        message[padded_len - 1] = static_cast<uint8_t>((length_bits >> 0) & 0xFF);
    }

    RAINY_INLINE void compress(std::uint32_t (&state)[8], const std::vector<uint8_t> &message_block) {
        std::uint32_t w[64];
        for (int i = 0; i < 16; ++i) {
            w[i] = (message_block[i * 4] << 24) | (message_block[i * 4 + 1] << 16) | (message_block[i * 4 + 2] << 8) |
                   (message_block[i * 4 + 3]);
        }
        for (int i = 16; i < 64; ++i) {
            std::uint32_t s0 = right_rotate(w[i - 15], 7) ^ right_rotate(w[i - 15], 18) ^ (w[i - 15] >> 3);
            std::uint32_t s1 = right_rotate(w[i - 2], 17) ^ right_rotate(w[i - 2], 19) ^ (w[i - 2] >> 10);
            w[i] = w[i - 16] + s0 + w[i - 7] + s1;
        }
        std::uint32_t a = state[0];
        std::uint32_t b = state[1];
        std::uint32_t c = state[2];
        std::uint32_t d = state[3];
        std::uint32_t e = state[4];
        std::uint32_t f = state[5];
        std::uint32_t g = state[6];
        std::uint32_t h = state[7];
        for (int i = 0; i < 64; ++i) {
            std::uint32_t S1 = right_rotate(e, 6) ^ right_rotate(e, 11) ^ right_rotate(e, 25);
            std::uint32_t ch = (e & f) ^ (~e & g);
            std::uint32_t temp1 = h + S1 + ch + constants[i] + w[i];
            std::uint32_t S0 = right_rotate(a, 2) ^ right_rotate(a, 13) ^ right_rotate(a, 22);
            std::uint32_t maj = (a & b) ^ (a & c) ^ (b & c);
            std::uint32_t temp2 = S0 + maj;
            h = g;
            g = f;
            f = e;
            e = d + temp1;
            d = c;
            c = b;
            b = a;
            a = temp1 + temp2;
        }
        state[0] += a;
        state[1] += b;
        state[2] += c;
        state[3] += d;
        state[4] += e;
        state[5] += f;
        state[6] += g;
        state[7] += h;
    }

    RAINY_INLINE void preprocess_message_from_file(const std::string_view file_path, std::vector<uint8_t> &message) {
        std::ifstream file(file_path.data(), std::ios::binary);
        if (!file) {
            return;
        }
        message.assign(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
    }
}
