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
#ifndef RAINY_FOUNDATION_IO_NET_BUFFER_HPP
#define RAINY_FOUNDATION_IO_NET_BUFFER_HPP
#include <rainy/core/core.hpp>
#include <rainy/collections/vector.hpp>
#include <rainy/foundation/io/net/executor/async_result.hpp>
#include <rainy/foundation/io/net/fwd.hpp>

namespace rainy::foundation::io::net {
    enum class stream_errc : int {
        eof = 1,
        not_found = 2
    };

    class stream_error_category : public std::error_category {
    public:
        const char *name() const noexcept override {
            return "stream";
        }

        std::string message(int ev) const override {
            switch (static_cast<stream_errc>(ev)) {
                case stream_errc::eof:
                    return "end of file";
                case stream_errc::not_found:
                    return "not found";
                default:
                    return "unknown stream error";
            }
        }

        bool equivalent(const std::error_code &code, int condition) const noexcept override {
            return code.category() == *this && code.value() == condition;
        }
    };

    RAINY_INLINE const std::error_category &stream_category() noexcept {
        static stream_error_category instance;
        return instance;
    }

    RAINY_INLINE std::error_code make_error_code(stream_errc e) noexcept {
        return {static_cast<int>(e), stream_category()};
    }

    RAINY_INLINE std::error_condition make_error_condition(stream_errc e) noexcept {
        return {static_cast<int>(e), stream_category()};
    }

    class mutable_buffer {
    public:
        mutable_buffer() noexcept : data_(nullptr), size_(0) {
        }

        mutable_buffer(void *p, std::size_t count) noexcept : data_(p), size_(count) {
        }

        void *data() const noexcept {
            return data_;
        }
        std::size_t size() const noexcept {
            return size_;
        }

        mutable_buffer &operator+=(std::size_t count) noexcept {
            std::size_t offset = count < size_ ? count : size_;
            data_ = static_cast<char *>(data_) + offset;
            size_ -= offset;
            return *this;
        }

    private:
        void *data_;
        std::size_t size_;
    };

    class const_buffer {
    public:
        const_buffer() noexcept : data_(nullptr), size_(0) {
        }

        const_buffer(const void *p, std::size_t count) noexcept : data_(p), size_(count) {
        }

        const_buffer(const mutable_buffer &b) noexcept : data_(b.data()), size_(b.size()) {
        }

        const void *data() const noexcept {
            return data_;
        }
        std::size_t size() const noexcept {
            return size_;
        }

        const_buffer &operator+=(std::size_t count) noexcept {
            std::size_t offset = count < size_ ? count : size_;
            data_ = static_cast<const char *>(data_) + offset;
            size_ -= offset;
            return *this;
        }

    private:
        const void *data_;
        std::size_t size_;
    };
}

namespace rainy::foundation::io::net {
    template <typename Ty>
    struct is_mutable_buffer_sequence : type_traits::type_relations::is_same<Ty, mutable_buffer> {};

    template <typename Ty>
    struct is_const_buffer_sequence
        : type_traits::logical_traits::disjunction<type_traits::type_relations::is_same<Ty, const_buffer>,
                                                   type_traits::type_relations::is_same<Ty, mutable_buffer>> {};

    template <typename Ty, typename = void>
    struct is_dynamic_buffer : type_traits::helper::false_type {};

    template <typename Ty>
    struct is_dynamic_buffer<
        Ty, type_traits::other_trans::void_t<decltype(utility::declval<Ty>().size()), decltype(utility::declval<Ty>().max_size()),
                                             decltype(utility::declval<Ty>().capacity()), decltype(utility::declval<Ty>().data()),
                                             decltype(utility::declval<Ty>().prepare(0)), decltype(utility::declval<Ty>().commit(0)),
                                             decltype(utility::declval<Ty>().consume(0))>> : type_traits::helper::true_type {};

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_mutable_buffer_sequence_v = is_mutable_buffer_sequence<Ty>::value;

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_const_buffer_sequence_v = is_const_buffer_sequence<Ty>::value;

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_dynamic_buffer_v = is_dynamic_buffer<Ty>::value;

    inline const mutable_buffer *buffer_sequence_begin(const mutable_buffer &b) noexcept {
        return &b;
    }

    inline const const_buffer *buffer_sequence_begin(const const_buffer &b) noexcept {
        return &b;
    }

    inline const mutable_buffer *buffer_sequence_end(const mutable_buffer &b) noexcept {
        return &b + 1;
    }

    inline const const_buffer *buffer_sequence_end(const const_buffer &b) noexcept {
        return &b + 1;
    }

    template <typename Cont>
    rain_fn buffer_sequence_begin(Cont &cont) noexcept -> decltype(cont.begin()) {
        return cont.begin();
    }

    template <typename Cont>
    rain_fn buffer_sequence_begin(const Cont &cont) noexcept -> decltype(cont.begin()) {
        return cont.begin();
    }

    template <typename Cont>
    rain_fn buffer_sequence_end(Cont &cont) noexcept -> decltype(cont.end()) {
        return cont.end();
    }

    template <typename Cont>
    rain_fn buffer_sequence_end(const Cont &cont) noexcept -> decltype(cont.end()) {
        return cont.end();
    }

    template <typename ConstBufferSequence>
    std::size_t buffer_size(const ConstBufferSequence &buffers) noexcept {
        std::size_t total = 0;
        auto it = buffer_sequence_begin(buffers);
        auto end = buffer_sequence_end(buffers);
        for (; it != end; ++it) {
            total += const_buffer(*it).size();
        }
        return total;
    }

    template <typename MutableBufferSequence, typename ConstBufferSequence>
    std::size_t buffer_copy(const MutableBufferSequence &dest, const ConstBufferSequence &source) noexcept {
        return buffer_copy(dest, source, (std::numeric_limits<std::size_t>::max)());
    }

    template <typename MutableBufferSequence, typename ConstBufferSequence>
    std::size_t buffer_copy(const MutableBufferSequence &dest, const ConstBufferSequence &source, std::size_t max_size) noexcept {
        std::size_t copied = 0;

        auto dit = buffer_sequence_begin(dest);
        auto dend = buffer_sequence_end(dest);
        auto sit = buffer_sequence_begin(source);
        auto send = buffer_sequence_end(source);

        mutable_buffer cur_dest;
        const_buffer cur_src;

        if (dit != dend)
            cur_dest = mutable_buffer(*dit);
        if (sit != send)
            cur_src = const_buffer(*sit);

        while (copied < max_size && cur_dest.size() > 0 && cur_src.size() > 0) {
            std::size_t count = (core::min)({cur_dest.size(), cur_src.size(), max_size - copied});
            std::memcpy(cur_dest.data(), cur_src.data(), count);
            copied += count;
            cur_dest += count;
            cur_src += count;

            if (cur_dest.size() == 0) {
                if (++dit != dend)
                    cur_dest = mutable_buffer(*dit);
                else
                    break;
            }
            if (cur_src.size() == 0) {
                if (++sit != send)
                    cur_src = const_buffer(*sit);
                else
                    break;
            }
        }
        return copied;
    }

    inline mutable_buffer operator+(const mutable_buffer &b, std::size_t count) noexcept {
        mutable_buffer tmp = b;
        tmp += count;
        return tmp;
    }

    inline mutable_buffer operator+(std::size_t count, const mutable_buffer &b) noexcept {
        return b + count;
    }

    inline const_buffer operator+(const const_buffer &b, std::size_t count) noexcept {
        const_buffer tmp = b;
        tmp += count;
        return tmp;
    }

    inline const_buffer operator+(std::size_t count, const const_buffer &b) noexcept {
        return b + count;
    }
}

namespace rainy::foundation::io::net {
    inline mutable_buffer buffer(void *p, std::size_t count) noexcept {
        return {p, count};
    }

    inline const_buffer buffer(const void *p, std::size_t count) noexcept {
        return {p, count};
    }

    inline mutable_buffer buffer(const mutable_buffer &b) noexcept {
        return b;
    }

    inline mutable_buffer buffer(const mutable_buffer &b, std::size_t count) noexcept {
        return {b.data(), (std::min)(b.size(), count)};
    }

    inline const_buffer buffer(const const_buffer &b) noexcept {
        return b;
    }

    inline const_buffer buffer(const const_buffer &b, std::size_t count) noexcept {
        return {b.data(), (std::min)(b.size(), count)};
    }

    template <typename Ty, std::size_t N>
    mutable_buffer buffer(Ty (&data)[N]) noexcept {
        return {data, N * sizeof(Ty)};
    }

    template <typename Ty, std::size_t N>
    const_buffer buffer(const Ty (&data)[N]) noexcept {
        return {data, N * sizeof(Ty)};
    }

    template <typename Ty, std::size_t N>
    mutable_buffer buffer(collections::array<Ty, N> &data) noexcept {
        return {data.data(), N * sizeof(Ty)};
    }

    template <typename Ty, std::size_t N>
    const_buffer buffer(collections::array<const Ty, N> &data) noexcept {
        return {data.data(), N * sizeof(Ty)};
    }

    template <typename Ty, std::size_t N>
    const_buffer buffer(const collections::array<Ty, N> &data) noexcept {
        return {data.data(), N * sizeof(Ty)};
    }

    template <typename Ty, typename Allocator>
    mutable_buffer buffer(collections::vector<Ty, Allocator> &data) noexcept {
        return {data.data(), data.size() * sizeof(Ty)};
    }

    template <typename Ty, typename Allocator>
    const_buffer buffer(const collections::vector<Ty, Allocator> &data) noexcept {
        return {data.data(), data.size() * sizeof(Ty)};
    }

    template <typename CharType, typename Traits, typename Allocator>
    mutable_buffer buffer(foundation::text::basic_string<CharType, Traits, Allocator> &data) noexcept {
        return {data.data(), data.size() * sizeof(CharType)};
    }

    template <typename CharType, typename Traits, typename Allocator>
    const_buffer buffer(const foundation::text::basic_string<CharType, Traits, Allocator> &data) noexcept {
        return {data.data(), data.size() * sizeof(CharType)};
    }

    template <typename CharType, typename Traits>
    const_buffer buffer(foundation::text::basic_string_view<CharType, Traits> data) noexcept {
        return {data.data(), data.size() * sizeof(CharType)};
    }

    template <typename Ty, std::size_t N>
    mutable_buffer buffer(Ty (&data)[N], std::size_t count) noexcept {
        return {data, (std::min)(N * sizeof(Ty), count)};
    }

    template <typename Ty, std::size_t N>
    const_buffer buffer(const Ty (&data)[N], std::size_t count) noexcept {
        return {data, (std::min)(N * sizeof(Ty), count)};
    }

    template <typename Ty, std::size_t N>
    mutable_buffer buffer(collections::array<Ty, N> &data, std::size_t count) noexcept {
        return {data.data(), (std::min)(N * sizeof(Ty), count)};
    }

    template <typename Ty, std::size_t N>
    const_buffer buffer(collections::array<const Ty, N> &data, std::size_t count) noexcept {
        return {data.data(), (std::min)(N * sizeof(Ty), count)};
    }

    template <typename Ty, std::size_t N>
    const_buffer buffer(const collections::array<Ty, N> &data, std::size_t count) noexcept {
        return {data.data(), (std::min)(N * sizeof(Ty), count)};
    }

    template <typename CharType, typename Traits>
    const_buffer buffer(foundation::text::basic_string_view<CharType, Traits> data, std::size_t count) noexcept {
        return {data.data(), (std::min)(data.size() * sizeof(CharType), count)};
    }
}

namespace rainy::foundation::io::net {
    template <typename Ty, typename Allocator>
    class dynamic_vector_buffer {
    public:
        using const_buffers_type = const_buffer;
        using mutable_buffers_type = mutable_buffer;

        explicit dynamic_vector_buffer(collections::vector<Ty, Allocator> &vec) noexcept :
            vec_(vec), size_(vec.size()), max_size_(vec.max_size()) {
        }

        dynamic_vector_buffer(collections::vector<Ty, Allocator> &vec, std::size_t maximum_size) noexcept :
            vec_(vec), size_(vec.size()), max_size_(maximum_size) {
        }

        dynamic_vector_buffer(dynamic_vector_buffer &&) = default;

        std::size_t size() const noexcept {
            return size_;
        }

        std::size_t max_size() const noexcept {
            return max_size_;
        }

        std::size_t capacity() const noexcept {
            return vec_.capacity() * sizeof(Ty);
        }

        const_buffers_type data() const noexcept {
            return {vec_.data(), size_ * sizeof(Ty)};
        }

        mutable_buffers_type prepare(std::size_t count) {
            if (size_ + count > max_size_) {
                throw std::length_error("dynamic_vector_buffer overflow");
            }
            vec_.resize(size_ + count);
            return {vec_.data() + size_, count * sizeof(Ty)};
        }

        void commit(std::size_t count) {
            size_ += (std::min)(count, vec_.size() - size_);
        }

        void consume(std::size_t count) {
            std::size_t consume_count = (std::min)(count / sizeof(Ty), size_);
            vec_.erase(vec_.begin(), vec_.begin() + static_cast<std::ptrdiff_t>(consume_count));
            size_ -= consume_count;
        }

    private:
        collections::vector<Ty, Allocator> &vec_;
        std::size_t size_;
        const std::size_t max_size_;
    };

    template <typename CharType, typename Traits, typename Allocator>
    class dynamic_string_buffer {
    public:
        using const_buffers_type = const_buffer;
        using mutable_buffers_type = mutable_buffer;

        explicit dynamic_string_buffer(foundation::text::basic_string<CharType, Traits, Allocator> &str) noexcept :
            str_(str), size_(str.size()), max_size_(str.max_size()) {
        }

        dynamic_string_buffer(foundation::text::basic_string<CharType, Traits, Allocator> &str, std::size_t maximum_size) noexcept :
            str_(str), size_(str.size()), max_size_(maximum_size) {
        }

        dynamic_string_buffer(dynamic_string_buffer &&) = default;

        std::size_t size() const noexcept {
            return size_;
        }

        std::size_t max_size() const noexcept {
            return max_size_;
        }

        std::size_t capacity() const noexcept {
            return str_.capacity() * sizeof(CharType);
        }

        const_buffers_type data() const noexcept {
            return {str_.data(), size_ * sizeof(CharType)};
        }

        mutable_buffers_type prepare(std::size_t count) {
            if (size_ + count > max_size_) {
                throw std::length_error("dynamic_string_buffer overflow");
            }
            str_.resize(size_ + count);
            return {&str_[size_], count * sizeof(CharType)};
        }

        void commit(std::size_t count) noexcept {
            size_ += (std::min)(count, str_.size() - size_);
        }

        void consume(std::size_t count) {
            std::size_t erase_count = (std::min)(count / sizeof(CharType), size_);
            str_.erase(0, erase_count);
            size_ -= erase_count;
        }

    private:
        foundation::text::basic_string<CharType, Traits, Allocator> &str_;
        std::size_t size_;
        const std::size_t max_size_;
    };
}

namespace rainy::foundation::io::net {
    template <typename Ty, typename Allocator>
    dynamic_vector_buffer<Ty, Allocator> dynamic_buffer(collections::vector<Ty, Allocator> &vec) noexcept {
        return dynamic_vector_buffer<Ty, Allocator>{vec};
    }

    template <typename Ty, typename Allocator>
    dynamic_vector_buffer<Ty, Allocator> dynamic_buffer(collections::vector<Ty, Allocator> &vec, std::size_t count) noexcept {
        return dynamic_vector_buffer<Ty, Allocator>{vec, count};
    }

    template <typename CharType, typename Traits, typename Allocator>
    dynamic_string_buffer<CharType, Traits, Allocator> dynamic_buffer(
        foundation::text::basic_string<CharType, Traits, Allocator> &str) noexcept {
        return dynamic_string_buffer<CharType, Traits, Allocator>{str};
    }

    template <typename CharType, typename Traits, typename Allocator>
    dynamic_string_buffer<CharType, Traits, Allocator> dynamic_buffer(foundation::text::basic_string<CharType, Traits, Allocator> &str,
                                                                      std::size_t count) noexcept {
        return dynamic_string_buffer<CharType, Traits, Allocator>{str, count};
    }
}

namespace rainy::foundation::io::net {
    class transfer_all {
    public:
        std::size_t operator()(const std::error_code &ec, std::size_t) const {
            return ec ? 0 : std::numeric_limits<std::size_t>::max();
        }
    };

    class transfer_at_least {
    public:
        explicit transfer_at_least(std::size_t m) : minimum_(m) {
        }

        std::size_t operator()(const std::error_code &ec, std::size_t count) const {
            return (!ec && count < minimum_) ? std::numeric_limits<std::size_t>::max() : 0;
        }

    private:
        std::size_t minimum_;
    };

    class transfer_exactly {
    public:
        explicit transfer_exactly(std::size_t e) : exact_(e) {
        }

        std::size_t operator()(const std::error_code &ec, std::size_t count) const {
            return (!ec && count < exact_) ? (std::min)(exact_ - count, std::numeric_limits<std::size_t>::max()) : 0;
        }

    private:
        std::size_t exact_;
    };
}

namespace rainy::foundation::io::net {
    template <typename SyncReadStream, typename MutableBufferSequence, typename CompletionCondition,
              type_traits::other_trans::enable_if_t<!is_dynamic_buffer_v<MutableBufferSequence>, int> = 0>
    std::size_t read(SyncReadStream &stream, const MutableBufferSequence &buffers, CompletionCondition completion_condition,
                     std::error_code &ec) {
        ec.clear();
        std::size_t total = 0;
        std::size_t buf_size = buffer_size(buffers);
        rain_loop {
            std::size_t to_read = completion_condition(ec, total);
            if (to_read == 0 || total >= buf_size) {
                break;
            }
            to_read = (std::min)(to_read, buf_size - total);
            mutable_buffer slice = buffer(buffers) + total;
            std::size_t count = stream.read_some(slice, ec);
            total += count;
            if (ec || count == 0) {
                break;
            }
        }
        return total;
    }

    template <typename SyncReadStream, typename MutableBufferSequence,
              type_traits::other_trans::enable_if_t<!is_dynamic_buffer_v<MutableBufferSequence>, int> = 0>
    std::size_t read(SyncReadStream &stream, const MutableBufferSequence &buffers, std::error_code &ec) {
        return read(stream, buffers, transfer_all{}, ec);
    }

    template <typename SyncReadStream, typename MutableBufferSequence, typename CompletionCondition,
              type_traits::other_trans::enable_if_t<!is_dynamic_buffer_v<MutableBufferSequence>, int> = 0>
    std::size_t read(SyncReadStream &stream, const MutableBufferSequence &buffers, CompletionCondition completion_condition) {
        std::error_code ec;
        std::size_t count = read(stream, buffers, completion_condition, ec);
        if (ec && ec != make_error_code(stream_errc::eof)) {
            throw std::system_error(ec, "read");
        }
        return count;
    }

    template <typename SyncReadStream, typename MutableBufferSequence,
              type_traits::other_trans::enable_if_t<!is_dynamic_buffer_v<MutableBufferSequence>, int> = 0>
    std::size_t read(SyncReadStream &stream, const MutableBufferSequence &buffers) {
        return read(stream, buffers, transfer_all{});
    }

    template <typename SyncReadStream, typename DynamicBuffer, typename CompletionCondition,
              type_traits::other_trans::enable_if_t<is_dynamic_buffer_v<DynamicBuffer>, int> = 0>
    std::size_t read(SyncReadStream &stream, DynamicBuffer &&b, CompletionCondition completion_condition, std::error_code &ec) {
        ec.clear();
        std::size_t total = 0;
        rain_loop {
            std::size_t to_read = completion_condition(ec, total);
            if (to_read == 0) {
                break;
            }
            std::size_t available = b.max_size() - b.size();
            if (available == 0) {
                break;
            }
            to_read = (std::min)(to_read, available);
            mutable_buffer mb = b.prepare(to_read);
            std::error_code read_ec;
            std::size_t count = stream.read_some(mb, read_ec);
            b.commit(count);
            total += count;
            if (ec || count == 0) {
                break;
            }
        }
        return total;
    }

    template <typename SyncReadStream, typename DynamicBuffer>
    std::size_t read(SyncReadStream &stream, DynamicBuffer &&b, std::error_code &ec) {
        return read(stream, utility::forward<DynamicBuffer>(b), transfer_all{}, ec);
    }

    template <typename SyncReadStream, typename DynamicBuffer, typename CompletionCondition>
    std::size_t read(SyncReadStream &stream, DynamicBuffer &&b, CompletionCondition completion_condition) {
        std::error_code ec;
        std::size_t count = read(stream, utility::forward<DynamicBuffer>(b), completion_condition, ec);
        if (ec && ec != make_error_code(stream_errc::eof)) {
            throw std::system_error(ec, "read");
        }
        return count;
    }

    template <typename SyncReadStream, typename DynamicBuffer>
    std::size_t read(SyncReadStream &stream, DynamicBuffer &&b) {
        return read(stream, utility::forward<DynamicBuffer>(b), transfer_all{});
    }
}

namespace rainy::foundation::io::net {
    template <typename SyncWriteStream, typename ConstBufferSequence, typename CompletionCondition,
              type_traits::other_trans::enable_if_t<!is_dynamic_buffer_v<ConstBufferSequence>, int> = 0>
    std::size_t write(SyncWriteStream &stream, const ConstBufferSequence &buffers, CompletionCondition completion_condition,
                      std::error_code &ec) {
        ec.clear();
        std::size_t total = 0;
        std::size_t buf_total = buffer_size(buffers);
        rain_loop {
            std::size_t to_write = completion_condition(ec, total);
            if (to_write == 0 || total >= buf_total) {
                break;
            }
            const_buffer cur = buffer(buffers) + total;
            std::size_t count = stream.write_some(cur, ec);
            total += count;
            if (ec || count == 0) {
                break;
            }
        }
        return total;
    }

    template <typename SyncWriteStream, typename ConstBufferSequence,
              type_traits::other_trans::enable_if_t<!is_dynamic_buffer_v<ConstBufferSequence>, int> = 0>
    std::size_t write(SyncWriteStream &stream, const ConstBufferSequence &buffers, std::error_code &ec) {
        return write(stream, buffers, transfer_all{}, ec);
    }

    template <typename SyncWriteStream, typename ConstBufferSequence, typename CompletionCondition,
              type_traits::other_trans::enable_if_t<!is_dynamic_buffer_v<ConstBufferSequence>, int> = 0>
    std::size_t write(SyncWriteStream &stream, const ConstBufferSequence &buffers, CompletionCondition completion_condition) {
        std::error_code ec;
        std::size_t count = write(stream, buffers, completion_condition, ec);
        if (ec) {
            throw std::system_error(ec, "write");
        }
        return count;
    }

    template <typename SyncWriteStream, typename ConstBufferSequence,
              type_traits::other_trans::enable_if_t<!is_dynamic_buffer_v<ConstBufferSequence>, int> = 0>
    std::size_t write(SyncWriteStream &stream, const ConstBufferSequence &buffers) {
        return write(stream, buffers, transfer_all{});
    }

    template <typename SyncWriteStream, typename DynamicBuffer, typename CompletionCondition,
              type_traits::other_trans::enable_if_t<is_dynamic_buffer_v<DynamicBuffer>, int> = 0>
    std::size_t write(SyncWriteStream &stream, DynamicBuffer &&b, CompletionCondition completion_condition, std::error_code &ec) {
        ec.clear();
        std::size_t total = 0;
        rain_loop {
            std::size_t to_write = completion_condition(ec, total);
            if (to_write == 0) {
                break;
            }
            auto buf = b.data();
            if (buf.size() == 0) {
                break;
            }
            std::size_t count = stream.write_some(buf, ec);
            b.consume(count);
            total += count;
            if (ec) {
                break;
            }
            if (count == 0) {
                break;
            }
        }
        return total;
    }

    template <typename SyncWriteStream, typename DynamicBuffer,
              type_traits::other_trans::enable_if_t<is_dynamic_buffer_v<DynamicBuffer>, int> = 0>
    std::size_t write(SyncWriteStream &stream, DynamicBuffer &&b, std::error_code &ec) {
        return write(stream, utility::forward<DynamicBuffer>(b), transfer_all{}, ec);
    }

    template <typename SyncWriteStream, typename DynamicBuffer, typename CompletionCondition,
              type_traits::other_trans::enable_if_t<is_dynamic_buffer_v<DynamicBuffer>, int> = 0>
    std::size_t write(SyncWriteStream &stream, DynamicBuffer &&b, CompletionCondition completion_condition) {
        std::error_code ec;
        std::size_t count = write(stream, utility::forward<DynamicBuffer>(b), completion_condition, ec);
        if (ec)
            throw std::system_error(ec, "write");
        return count;
    }

    template <typename SyncWriteStream, typename DynamicBuffer,
              type_traits::other_trans::enable_if_t<is_dynamic_buffer_v<DynamicBuffer>, int> = 0>
    std::size_t write(SyncWriteStream &stream, DynamicBuffer &&b) {
        return write(stream, utility::forward<DynamicBuffer>(b), transfer_all{});
    }
}

namespace rainy::foundation::io::net {
    template <typename AsyncReadStream, typename MutableBufferSequence, typename CompletionCondition, typename CompletionToken,
              type_traits::other_trans::enable_if_t<!is_dynamic_buffer_v<MutableBufferSequence>, int> = 0>
    auto async_read(AsyncReadStream &stream, const MutableBufferSequence &buffers, CompletionCondition completion_condition,
                    CompletionToken &&token) ->
        typename async_result<type_traits::other_trans::decay_t<CompletionToken>, void(std::error_code, std::size_t)>::return_type {
        using token_t = type_traits::other_trans::decay_t<CompletionToken>;
        using result_t = async_result<token_t, void(std::error_code, std::size_t)>;
        async_completion<token_t, void(std::error_code, std::size_t)> init(token);
        auto handler = utility::move(init.completion_handler);
        auto do_read = [&stream, buffers, completion_condition, handler]() mutable {
            std::error_code ec;
            std::size_t total = 0;
            rain_loop {
                std::size_t to_read = completion_condition(ec, total);
                if (to_read == 0)
                    break;
                std::size_t count = stream.read_some(buffer(buffers) + total, ec);
                total += count;
                if (ec || count == 0)
                    break;
            }
            handler(ec, total);
        };

        stream.get_executor().post(utility::move(do_read), std::allocator<void>{});
        return init.result.get();
    }

    template <typename AsyncReadStream, typename MutableBufferSequence, typename CompletionToken,
              type_traits::other_trans::enable_if_t<!is_dynamic_buffer_v<MutableBufferSequence>, int> = 0>
    auto async_read(AsyncReadStream &stream, const MutableBufferSequence &buffers, CompletionToken &&token) ->
        typename async_result<type_traits::other_trans::decay_t<CompletionToken>, void(std::error_code, std::size_t)>::return_type {
        return async_read(stream, buffers, transfer_all{}, utility::forward<CompletionToken>(token));
    }

    template <typename AsyncReadStream, typename DynamicBuffer, typename CompletionCondition, typename CompletionToken,
              type_traits::other_trans::enable_if_t<is_dynamic_buffer_v<DynamicBuffer>, int> = 0>
    auto async_read(AsyncReadStream &stream, DynamicBuffer &&b, CompletionCondition completion_condition, CompletionToken &&token) ->
        typename async_result<type_traits::other_trans::decay_t<CompletionToken>, void(std::error_code, std::size_t)>::return_type {
        using token_t = type_traits::other_trans::decay_t<CompletionToken>;
        using result_t = async_result<token_t, void(std::error_code, std::size_t)>;
        async_completion<token_t, void(std::error_code, std::size_t)> init(token);

        auto handler = utility::move(init.completion_handler);
        auto buf = utility::forward<DynamicBuffer>(b);
        auto do_read = [&stream, buf = utility::move(buf), completion_condition, handler]() mutable {
            std::error_code ec;
            std::size_t total = 0;
            rain_loop {
                std::size_t to_read = completion_condition(ec, total);
                if (to_read == 0) {
                    break;
                }
                to_read = (core::min)(to_read, buf.max_size() - buf.size());
                if (to_read == 0) {
                    ec = make_error_code(stream_errc::eof);
                    break;
                }
                auto mb = buf.prepare(to_read);
                std::size_t count = stream.read_some(mb, ec);
                buf.commit(count);
                total += count;
                if (ec || count == 0) {
                    break;
                }
            }
            handler(ec, total);
        };

        stream.get_executor().post(utility::move(do_read), std::allocator<void>{});
        return init.result.get();
    }

    template <typename AsyncReadStream, typename DynamicBuffer, typename CompletionToken,
              type_traits::other_trans::enable_if_t<is_dynamic_buffer_v<DynamicBuffer>, int> = 0>
    auto async_read(AsyncReadStream &stream, DynamicBuffer &&b, CompletionToken &&token) ->
        typename async_result<type_traits::other_trans::decay_t<CompletionToken>, void(std::error_code, std::size_t)>::return_type {
        return async_read(stream, utility::forward<DynamicBuffer>(b), transfer_all{}, utility::forward<CompletionToken>(token));
    }

    template <typename AsyncWriteStream, typename ConstBufferSequence, typename CompletionCondition, typename CompletionToken,
              type_traits::other_trans::enable_if_t<!is_dynamic_buffer_v<ConstBufferSequence>, int> = 0>
    auto async_write(AsyncWriteStream &stream, const ConstBufferSequence &buffers, CompletionCondition completion_condition,
                     CompletionToken &&token) ->
        typename async_result<type_traits::other_trans::decay_t<CompletionToken>, void(std::error_code, std::size_t)>::return_type {
        using token_t = type_traits::other_trans::decay_t<CompletionToken>;
        async_completion<token_t, void(std::error_code, std::size_t)> init(token);
        auto handler = utility::move(init.completion_handler);
        auto do_write = [&stream, buffers, completion_condition, handler]() mutable {
            std::error_code ec;
            std::size_t total = 0;
            rain_loop {
                std::size_t to_write = completion_condition(ec, total);
                if (to_write == 0)
                    break;
                std::size_t count = stream.write_some(buffer(buffers) + total, ec);
                total += count;
                if (ec || count == 0)
                    break;
            }
            handler(ec, total);
        };
        stream.get_executor().post(utility::move(do_write), std::allocator<void>{});
        return init.result.get();
    }

    template <typename AsyncWriteStream, typename ConstBufferSequence, typename CompletionToken,
              type_traits::other_trans::enable_if_t<!is_dynamic_buffer_v<ConstBufferSequence>, int> = 0>
    auto async_write(AsyncWriteStream &stream, const ConstBufferSequence &buffers, CompletionToken &&token) ->
        typename async_result<type_traits::other_trans::decay_t<CompletionToken>, void(std::error_code, std::size_t)>::return_type {
        return async_write(stream, buffers, transfer_all{}, utility::forward<CompletionToken>(token));
    }

    template <typename AsyncWriteStream, typename DynamicBuffer, typename CompletionCondition, typename CompletionToken,
              type_traits::other_trans::enable_if_t<is_dynamic_buffer_v<DynamicBuffer>, int> = 0>
    auto async_write(AsyncWriteStream &stream, DynamicBuffer &&b, CompletionCondition completion_condition, CompletionToken &&token) ->
        typename async_result<type_traits::other_trans::decay_t<CompletionToken>, void(std::error_code, std::size_t)>::return_type {
        using token_t = type_traits::other_trans::decay_t<CompletionToken>;
        async_completion<token_t, void(std::error_code, std::size_t)> init(token);
        auto handler = utility::move(init.completion_handler);
        auto buf = utility::forward<DynamicBuffer>(b);
        auto do_write = [&stream, buf = utility::move(buf), completion_condition, handler]() mutable {
            std::error_code ec;
            std::size_t total = 0;
            rain_loop {
                std::size_t to_write = completion_condition(ec, total);
                if (to_write == 0) {
                    break;
                }
                auto cb = buf.data();
                if (cb.size() == 0) {
                    break;
                }
                std::size_t count = stream.write_some(cb, ec);
                buf.consume(count);
                total += count;
                if (ec || count == 0) {
                    break;
                }
            }
            handler(ec, total);
        };
        stream.get_executor().post(utility::move(do_write), std::allocator<void>{});
        return init.result.get();
    }

    template <typename AsyncWriteStream, typename DynamicBuffer, typename CompletionToken,
              type_traits::other_trans::enable_if_t<is_dynamic_buffer_v<DynamicBuffer>, int> = 0>
    auto async_write(AsyncWriteStream &stream, DynamicBuffer &&b, CompletionToken &&token) ->
        typename async_result<type_traits::other_trans::decay_t<CompletionToken>, void(std::error_code, std::size_t)>::return_type {
        return async_write(stream, utility::forward<DynamicBuffer>(b), transfer_all{}, utility::forward<CompletionToken>(token));
    }
}

namespace rainy::foundation::io::net::implements {
    template <typename SyncReadStream, typename DynamicBuffer, typename Delim>
    std::size_t read_until_impl(SyncReadStream &s, DynamicBuffer &&b, Delim delim, std::error_code &ec) {
        ec.clear();
        std::size_t search_start = 0;
        rain_loop {
            // 在已提交数据中搜索分隔符
            auto committed = b.data();
            const char *begin = static_cast<const char *>(committed.data());
            const char *end = begin + committed.size();

            if constexpr (std::is_same_v<Delim, char>) {
                auto pos = std::find(begin + search_start, end, delim);
                if (pos != end) {
                    return static_cast<std::size_t>(pos - begin) + 1;
                }
            } else {
                // string_view 分隔符
                foundation::text::string_view sv{begin + search_start, static_cast<std::size_t>(end - begin - search_start)};
                auto idx = sv.find(delim);
                if (idx != foundation::text::string_view::npos) {
                    return search_start + idx + delim.size();
                }
            }
            search_start = committed.size();
            if (b.size() == b.max_size()) {
                ec = make_error_code(stream_errc::not_found);
                return b.size();
            }
            std::size_t avail = (std::min)(std::size_t{512}, b.max_size() - b.size());
            auto mb = b.prepare(avail);
            std::size_t count = s.read_some(mb, ec);
            b.commit(count);
            if (ec) {
                return b.size();
            }
            if (count == 0) {
                ec = make_error_code(stream_errc::eof);
                return b.size();
            }
        }
    }
}

namespace rainy::foundation::io::net {
    template <typename SyncReadStream, typename DynamicBuffer,
              type_traits::other_trans::enable_if_t<is_dynamic_buffer_v<DynamicBuffer>, int> = 0>
    std::size_t read_until(SyncReadStream &s, DynamicBuffer &&b, char delim, std::error_code &ec) {
        return implements::read_until_impl(s, utility::forward<DynamicBuffer>(b), delim, ec);
    }

    template <typename SyncReadStream, typename DynamicBuffer,
              type_traits::other_trans::enable_if_t<is_dynamic_buffer_v<DynamicBuffer>, int> = 0>
    std::size_t read_until(SyncReadStream &s, DynamicBuffer &&b, char delim) {
        std::error_code ec;
        std::size_t count = read_until(s, utility::forward<DynamicBuffer>(b), delim, ec);
        if (ec) {
            throw std::system_error(ec, "read_until");
        }
        return count;
    }

    template <typename SyncReadStream, typename DynamicBuffer,
              type_traits::other_trans::enable_if_t<is_dynamic_buffer_v<DynamicBuffer>, int> = 0>
    std::size_t read_until(SyncReadStream &s, DynamicBuffer &&b, foundation::text::string_view delim, std::error_code &ec) {
        return implements::read_until_impl(s, utility::forward<DynamicBuffer>(b), delim, ec);
    }

    template <typename SyncReadStream, typename DynamicBuffer,
              type_traits::other_trans::enable_if_t<is_dynamic_buffer_v<DynamicBuffer>, int> = 0>
    std::size_t read_until(SyncReadStream &s, DynamicBuffer &&b, foundation::text::string_view delim) {
        std::error_code ec;
        std::size_t count = read_until(s, utility::forward<DynamicBuffer>(b), delim, ec);
        if (ec) {
            throw std::system_error(ec, "read_until");
        }
        return count;
    }

    template <typename AsyncReadStream, typename DynamicBuffer, typename CompletionToken,
              type_traits::other_trans::enable_if_t<is_dynamic_buffer_v<DynamicBuffer>, int> = 0>
    auto async_read_until(AsyncReadStream &s, DynamicBuffer &&b, char delim, CompletionToken &&token) ->
        typename async_result<type_traits::other_trans::decay_t<CompletionToken>, void(std::error_code, std::size_t)>::return_type {
        using token_t = type_traits::other_trans::decay_t<CompletionToken>;
        async_completion<token_t, void(std::error_code, std::size_t)> init(token);
        auto handler = utility::move(init.completion_handler);
        auto buf = utility::forward<DynamicBuffer>(b);
        s.get_executor().post(
            [&s, buf = utility::move(buf), delim, handler]() mutable {
                std::error_code ec;
                std::size_t count = implements::read_until_impl(s, buf, delim, ec);
                handler(ec, count);
            },
            std::allocator<void>{});
        return init.result.get();
    }

    template <typename AsyncReadStream, typename DynamicBuffer, typename CompletionToken,
              type_traits::other_trans::enable_if_t<is_dynamic_buffer_v<DynamicBuffer>, int> = 0>
    auto async_read_until(AsyncReadStream &s, DynamicBuffer &&b, foundation::text::string_view delim, CompletionToken &&token) ->
        typename async_result<type_traits::other_trans::decay_t<CompletionToken>, void(std::error_code, std::size_t)>::return_type {
        using token_t = type_traits::other_trans::decay_t<CompletionToken>;
        async_completion<token_t, void(std::error_code, std::size_t)> init(token);
        auto handler = utility::move(init.completion_handler);
        auto buf = utility::forward<DynamicBuffer>(b);
        auto delim_s = foundation::text::string{delim};
        s.get_executor().post(
            [&s, buf = utility::move(buf), delim_s, handler]() mutable {
                std::error_code ec;
                std::size_t count = implements::read_until_impl(s, buf, foundation::text::string_view{delim_s}, ec);
                handler(ec, count);
            },
            std::allocator<void>{});
        return init.result.get();
    }
}

namespace std {
    template <>
    struct is_error_code_enum<rainy::foundation::io::net::stream_errc> : public true_type {};
}

#endif
