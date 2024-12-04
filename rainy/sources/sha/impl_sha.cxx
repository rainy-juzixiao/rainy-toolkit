#include <rainy/user/hash/sha.hpp>

namespace rainy::user::hash::internals::sha512 {
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

    std::uint64_t right_rotate(std::uint64_t x, std::uint64_t n) {
        return (x >> n) | (x << (64 - n));
    }

    void preprocess_message(const std::string &input, std::vector<uint8_t> &message) {
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

    void preprocess_message_from_file(const std::string_view file_path, std::vector<uint8_t> &message) {
        std::ifstream file(file_path.data(), std::ios::binary);
        if (!file) {
            return;
        }
        message.assign(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
    }
}

namespace rainy::user::hash::internals::sha256 {
    static const std::uint32_t constants[64] = {
        0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5, 0xd807aa98, 0x12835b01,
        0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174, 0x49b40821, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
        0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85, 0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624,
        0xf40e3585, 0x106aa070, 0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b1d8e6a, 0x6d9d6122,
        0x758e5f76, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2};

    std::uint32_t right_rotate(std::uint32_t x, std::uint32_t n) {
        if (n == 0)
            return x;
        return (x >> n) | (x << (32 - n));
    }


    // 消息预处理函数
    void preprocess_message(const std::string &input, std::vector<uint8_t> &message) {
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

    // 压缩函数
    void compress(std::uint32_t (&state)[8], const std::vector<uint8_t> &message_block) {
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

    void preprocess_message_from_file(const std::string_view file_path, std::vector<uint8_t> &message) {
        std::ifstream file(file_path.data(), std::ios::binary);
        if (!file) {
            return;
        }
        message.assign(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
    }
}