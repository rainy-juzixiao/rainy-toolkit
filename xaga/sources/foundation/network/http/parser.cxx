#include <rainy/foundation/network/http/parser.hpp>

#if RAINY_USING_MSVC
#pragma warning(push)
#pragma warning(disable : 6387 26819)
#endif

namespace rainy::foundation::network::http::implements {
    static bool parse_digit(const char *&buf, const char *buf_end, int multiplier, int &out, int *ret) {
        if (buf == buf_end || *buf < '0' || *buf > '9') {
            ++buf;
            *ret = -1;
            return false;
        }
        out = multiplier * (*buf++ - '0');
        return true;
    }

    static const char *parse_int3(const char *&buf, const char *buf_end, int *val, int *ret) {
        int result = 0;
        int part = 0;
        if (!parse_digit(buf, buf_end, 100, part, ret)) {
            return nullptr;
        }
        result += part;
        if (!parse_digit(buf, buf_end, 10, part, ret)) {
            return nullptr;
        }
        result += part;
        if (!parse_digit(buf, buf_end, 1, part, ret)) {
            return nullptr;
        }
        result += part;
        *val = result;
        return buf;
    }

    inline bool check_eof(const char *&buf, const char *buf_end, int *ret) {
        if (buf == buf_end) {
            *ret = -2;
            return false;
        }
        return true;
    }

    inline bool expect_char(const char *&buf, const char *buf_end, char ch, int *ret) {
        if (!check_eof(buf, buf_end, ret))
            return false;
        if (*buf++ != ch) {
            *ret = -1;
            return false;
        }
        return true;
    }

    inline const char *advance_token(const char *&buf, const char *buf_end, const char **tok, std::size_t *toklen, int *ret) {
        if (!check_eof(buf, buf_end, ret))
            return nullptr;

        const char *tok_start = buf;
        while (true) {
            if (*buf == ' ') {
                break;
            } else if (rainy_unlikely(!is_printable_ascii(*buf))) {
                if (static_cast<unsigned char>(*buf) < '\040' || *buf == '\177') {
                    *ret = -1;
                    return nullptr;
                }
            }

            ++buf;
            if (!check_eof(buf, buf_end, ret))
                return nullptr;
        }

        *tok = tok_start;
        *toklen = buf - tok_start;
        return buf;
    }

    inline const char *advance_path(const char *&buf, const char *buf_end, const char **tok, std::size_t *toklen, int *ret,
                                    bool &has_query) {
        if (!check_eof(buf, buf_end, ret))
            return nullptr;

        const char *tok_start = buf;
        while (true) {
            if (*buf == ' ') {
                break;
            } else if (rainy_unlikely(!is_printable_ascii(*buf))) {
                if (static_cast<unsigned char>(*buf) < '\040' || *buf == '\177') {
                    *ret = -1;
                    return nullptr;
                }
            } else if (rainy_unlikely(*buf == '?')) {
                has_query = true;
            }

            ++buf;
            if (!check_eof(buf, buf_end, ret))
                return nullptr;
        }

        *tok = tok_start;
        *toklen = buf - tok_start;
        return buf;
    }

    const char *get_token_to_eol(const char *buf, const char *buf_end, const char **token, std::size_t *token_len, int *ret) {
        const char *const token_start = buf;
        while (rainy_likely(buf_end - buf >= 8)) {
            bool found_ctl = false;
            for (int i = 0; i < 8; ++i) {
                const unsigned char ch = static_cast<unsigned char>(*buf);
                if (rainy_unlikely(!is_printable_ascii(ch))) {
                    if ((rainy_likely(ch < '\040') && rainy_likely(ch != '\011')) || rainy_unlikely(ch == '\177')) {
                        found_ctl = true;
                        break;
                    }
                }
                ++buf;
            }
            if (found_ctl) {
                break;
            }
        }
        while (buf < buf_end) {
            const unsigned char ch = static_cast<unsigned char>(*buf);
            if (rainy_unlikely(!is_printable_ascii(ch))) {
                if ((rainy_likely(ch < '\040') && rainy_likely(ch != '\011')) || rainy_unlikely(ch == '\177')) {
                    break;
                }
            }
            ++buf;
        }
        if (buf >= buf_end) {
            *ret = -1;
            return nullptr;
        }
        if (rainy_likely(*buf == '\015')) {
            ++buf;
            if (buf >= buf_end || *buf != '\012') {
                *ret = -1;
                return nullptr;
            }
            ++buf;
            *token_len = static_cast<std::size_t>(buf - token_start - 2);
        } else if (*buf == '\012') {
            ++buf;
            *token_len = static_cast<std::size_t>(buf - token_start - 1);
        } else {
            *ret = -1;
            return nullptr;
        }
        *token = token_start;
        return buf;
    }

    const char *is_complete(const char *buf, const char *buf_end, std::size_t last_len, int *ret) {
        int ret_cnt = 0;
        if (last_len >= 3) {
            buf += last_len - 3;
        }
        while (true) {
            if (buf == buf_end) {
                *ret = -2;
                return nullptr;
            }
            if (*buf == '\015') { // '\r'
                ++buf;
                if (buf == buf_end) {
                    *ret = -2;
                    return nullptr;
                }
                if (*buf != '\012') { // '\n'
                    *ret = -1;
                    return nullptr;
                }
                ++buf;
                ++ret_cnt;
            } else if (*buf == '\012') { // '\n'
                ++buf;
                ++ret_cnt;
            } else {
                ++buf;
                ret_cnt = 0;
            }
            if (ret_cnt == 2) {
                return buf;
            }
        }
        *ret = -2;
        return nullptr;
    }

    const char *parse_http_version(const char *buf, const char *buf_end, int *minor_version, int *ret) {
        /* [HTTP/1.<two chars>] */
        if (buf_end - buf < 9) {
            *ret = -2;
            return nullptr;
        }
        constexpr const char prefix[] = {'H', 'T', 'T', 'P', '/', '1', '.'};
        for (const char c: prefix) {
            if (*buf++ != c) {
                *ret = -1;
                return nullptr;
            };
        }
        parse_digit(buf, buf_end, 1, *minor_version, ret);
        return buf;
    }

    const char *parse_request(const char *buf, const char *buf_end, const char **method, std::size_t *method_len, const char **path,
                              std::size_t *path_len, int *minor_version, http_header *headers, std::size_t *num_headers,
                              std::size_t max_headers, int *ret, bool &has_connection, bool &has_close, bool &has_upgrade,
                              bool &has_query) {
        if (!check_eof(buf, buf_end, ret)) {
            return nullptr;
        }
        if (*buf == '\015') { // \r
            ++buf;
            if (!expect_char(buf, buf_end, '\012', ret)) {
                return nullptr; // \n
            }
        } else if (*buf == '\012') {
            ++buf;
        }
        if (!(buf = advance_token(buf, buf_end, method, method_len, ret))) {
            return nullptr;
        }
        ++buf;
        if (!(buf = advance_path(buf, buf_end, path, path_len, ret, has_query))) {
            return nullptr;
        }
        ++buf;
        buf = parse_http_version(buf, buf_end, minor_version, ret);
        if (buf == nullptr) {
            return nullptr;
        }
        if (*buf == '\015') {
            ++buf;
            if (!expect_char(buf, buf_end, '\012', ret))
                return nullptr;
        } else if (*buf == '\012') {
            ++buf;
        } else {
            *ret = -1;
            return nullptr;
        }
        return parse_headers(buf, buf_end, headers, num_headers, max_headers, ret, has_connection, has_close, has_upgrade);
    }

    const char *parse_headers(const char *buf, const char *buf_end, http_header *headers, std::size_t *num_headers,
                              std::size_t max_headers, int *ret, bool &has_connection, bool &has_close, bool &has_upgrade) {
        while (true) {
            const char *name = nullptr;
            std::size_t name_len = 0;
            const char *value = nullptr;
            std::size_t value_len = 0;

            if (!check_eof(buf, buf_end, ret))
                return nullptr;

            if (*buf == '\015') {
                ++buf;
                if (!expect_char(buf, buf_end, '\012', ret))
                    return nullptr;
                break;
            } else if (*buf == '\012') {
                ++buf;
                break;
            }

            if (*num_headers == max_headers) {
                *ret = -1;
                return nullptr;
            }

            // 非折行情况，解析 name
            if (!(*num_headers != 0 && (*buf == ' ' || *buf == '\t'))) {
                name = buf;

                if (!check_eof(buf, buf_end, ret))
                    return nullptr;

                while (true) {
                    if (*buf == ':')
                        break;
                    if (!token_char_map[static_cast<unsigned char>(*buf)]) {
                        *ret = -1;
                        return nullptr;
                    }
                    ++buf;
                    if (!check_eof(buf, buf_end, ret))
                        return nullptr;
                }

                name_len = buf - name;
                if (name_len == 0) {
                    *ret = -1;
                    return nullptr;
                }

                ++buf; // skip ':'

                // skip leading whitespace before value
                while (true) {
                    if (!check_eof(buf, buf_end, ret))
                        return nullptr;
                    if (*buf != ' ' && *buf != '\t') {
                        break;
                    }
                    ++buf;
                }
            }
            // parse value
            buf = get_token_to_eol(buf, buf_end, &value, &value_len, ret);
            if (buf == nullptr) {
                return nullptr;
            }
            // header name: Connection
            if (name_len == 10 && name && std::memcmp(name + 1, "onnection", name_len - 1) == 0) {
                has_connection = true;
                if (value_len >= 1) {
                    char ch = *value;
                    if (ch == 'U' || ch == 'u') {
                        has_upgrade = true;
                    } else if (ch == 'c' || ch == 'C') {
                        has_close = true;
                    }
                }
            }

            headers[*num_headers] = {std::string_view{name, name_len}, std::string_view{value, value_len}}; // NOLINT

            ++(*num_headers);
        }
        return buf;
    }
    int phr_parse_request(const char *buf_start, std::size_t len, const char **method, std::size_t *method_len, const char **path,
                          std::size_t *path_len, int *minor_version, http_header *headers, std::size_t *num_headers,
                          std::size_t last_len, bool &has_connection, bool &has_close, bool &has_upgrade, bool &has_query) {
        const char *buf = buf_start, *buf_end = buf_start + len;
        std::size_t max_headers = *num_headers;
        int res;
        *method = nullptr;
        *method_len = 0;
        *path = nullptr;
        *path_len = 0;
        *minor_version = -1;
        *num_headers = 0;
        if (last_len != 0 && is_complete(buf, buf_end, last_len, &res) == nullptr) {
            return res;
        }
        if ((buf = parse_request(buf + last_len, buf_end, method, method_len, path, path_len, minor_version, headers, num_headers,
                                 max_headers, &res, has_connection, has_close, has_upgrade, has_query)) == nullptr) {
            return res;
        }
        return (int) (buf - buf_start - last_len);
    }

    const char *parse_response(const char *buf, const char *buf_end, int *minor_version, int *status, const char **msg,
                               std::size_t *msg_len, http_header *headers, std::size_t *num_headers, std::size_t max_headers,
                               int *ret) {
        if ((buf = parse_http_version(buf, buf_end, minor_version, ret)) == nullptr) {
            return nullptr;
        }
        if (*buf++ != ' ') {
            *ret = -1;
            return nullptr;
        }
        /* status code : [:digit:][:digit:][:digit:] */
        if (buf_end - buf < 4) {
            *ret = -2;
            return nullptr;
        }
        parse_int3(buf, buf_end, status, ret);
        /* skip space */
        if (*buf++ != ' ') {
            *ret = -1;
            return nullptr;
        }
        /* get message */
        if ((buf = get_token_to_eol(buf, buf_end, msg, msg_len, ret)) == nullptr) {
            return nullptr;
        }
        bool has_connection, has_close, has_upgrade;
        return parse_headers(buf, buf_end, headers, num_headers, max_headers, ret, has_connection, has_close, has_upgrade);
    }

    int phr_parse_response(const char *buf_start, std::size_t len, int *minor_version, int *status, const char **msg,
                           std::size_t *msg_len, http_header *headers, std::size_t *num_headers, std::size_t last_len) {
        const char *buf = buf_start, *buf_end = buf + len;
        std::size_t max_headers = *num_headers;
        int res;
        *minor_version = -1;
        *status = 0;
        *msg = nullptr;
        *msg_len = 0;
        *num_headers = 0;
        if (last_len != 0 && is_complete(buf, buf_end, last_len, &res) == nullptr) {
            return res;
        }
        if ((buf = parse_response(buf, buf_end, minor_version, status, msg, msg_len, headers, num_headers, max_headers, &res)) ==
            nullptr) {
            return res;
        }
        return static_cast<int>(buf - buf_start);
    }

    int phr_parse_headers(const char *buf_start, std::size_t len, http_header *headers, std::size_t *num_headers,
                          std::size_t last_len) {
        const char *buf = buf_start, *buf_end = buf + len;
        std::size_t max_headers = *num_headers;
        int res;
        *num_headers = 0;
        if (last_len != 0 && is_complete(buf, buf_end, last_len, &res) == NULL) {
            return res;
        }
        bool has_connection, has_close, has_upgrade;
        if ((buf = parse_headers(buf, buf_end, headers, num_headers, max_headers, &res, has_connection, has_close, has_upgrade)) ==
            nullptr) {
            return res;
        }
        return static_cast<int>(buf - buf_start);
    }

    enum : char {
        CHUNKED_IN_CHUNK_SIZE,
        CHUNKED_IN_CHUNK_EXT,
        CHUNKED_IN_CHUNK_DATA,
        CHUNKED_IN_CHUNK_CRLF,
        CHUNKED_IN_TRAILERS_LINE_HEAD,
        CHUNKED_IN_TRAILERS_LINE_MIDDLE,
        CHUNKED_COMPLETE
    };

    core::ssize_t phr_decode_chunked(phr_chunked_decoder *decoder, char *buf, std::size_t *_bufsz) {
        std::size_t dst = 0;
        std::size_t src = 0;
        std::size_t bufsz = *_bufsz;
        while (true) {
            switch (decoder->_state) {
                case CHUNKED_IN_CHUNK_SIZE: {
                    while (src < bufsz) {
                        const int v = decode_hex(buf[src]);
                        if (v == -1) {
                            if (decoder->_hex_count == 0) {
                                return -1;
                            }
                            break; // 结束 chunk size 解析
                        }
                        if (decoder->_hex_count == sizeof(std::size_t) * 2) {
                            return -1;
                        }
                        decoder->bytes_left_in_chunk = decoder->bytes_left_in_chunk * 16 + v;
                        ++decoder->_hex_count;
                        ++src;
                    }
                    if (src == bufsz) {
                        *_bufsz = dst;
                        return -2;
                    }
                    decoder->_hex_count = 0;
                    decoder->_state = CHUNKED_IN_CHUNK_EXT;
                    RAINY_FALLTHROUGH;
                }
                case CHUNKED_IN_CHUNK_EXT: {
                    while (src < bufsz && buf[src] != '\012') {
                        ++src;
                    }
                    if (src == bufsz) {
                        *_bufsz = dst;
                        return -2;
                    }
                    ++src;
                    if (decoder->bytes_left_in_chunk == 0) {
                        decoder->_state = decoder->consume_trailer ? CHUNKED_IN_TRAILERS_LINE_HEAD : CHUNKED_COMPLETE;
                        continue;
                    }
                    decoder->_state = CHUNKED_IN_CHUNK_DATA;
                    RAINY_FALLTHROUGH;
                }
                case CHUNKED_IN_CHUNK_DATA: {
                    std::size_t avail = bufsz - src;
                    if (avail < decoder->bytes_left_in_chunk) {
                        if (dst != src) {
                            memmove(buf + dst, buf + src, avail);
                        }
                        src += avail;
                        dst += avail;
                        decoder->bytes_left_in_chunk -= avail;
                        *_bufsz = dst;
                        return -2;
                    }
                    if (dst != src) {
                        memmove(buf + dst, buf + src, decoder->bytes_left_in_chunk);
                    }
                    src += decoder->bytes_left_in_chunk;
                    dst += decoder->bytes_left_in_chunk;
                    decoder->bytes_left_in_chunk = 0;
                    decoder->_state = CHUNKED_IN_CHUNK_CRLF;
                    RAINY_FALLTHROUGH;
                }
                case CHUNKED_IN_CHUNK_CRLF: {
                    while (src < bufsz && buf[src] == '\015') {
                        ++src;
                    }
                    if (src == bufsz) {
                        *_bufsz = dst;
                        return -2;
                    }

                    if (buf[src] != '\012') {
                        return -1;
                    }
                    ++src;
                    decoder->_state = CHUNKED_IN_CHUNK_SIZE;
                    break;
                }
                case CHUNKED_IN_TRAILERS_LINE_HEAD: {
                    if (src == bufsz) {
                        *_bufsz = dst;
                        return -2;
                    }
                    if (buf[src] == '\015') {
                        ++src;
                        if (src == bufsz) {
                            *_bufsz = dst;
                            return -2;
                        }
                        if (buf[src] == '\012') {
                            ++src;
                            decoder->_state = CHUNKED_COMPLETE;
                            continue;
                        }
                    }

                    decoder->_state = CHUNKED_IN_TRAILERS_LINE_MIDDLE;
                    RAINY_FALLTHROUGH;
                }
                case CHUNKED_IN_TRAILERS_LINE_MIDDLE: {
                    while (src < bufsz && buf[src] != '\012') {
                        ++src;
                    }
                    if (src == bufsz) {
                        *_bufsz = dst;
                        return -2;
                    }
                    ++src;
                    decoder->_state = CHUNKED_IN_TRAILERS_LINE_HEAD;
                    break;
                }
                case CHUNKED_COMPLETE: {
                    std::size_t remaining = bufsz - src;
                    if (dst != src) {
                        memmove(buf + dst, buf + src, remaining);
                    }
                    *_bufsz = dst;
                    return static_cast<core::ssize_t>(remaining);
                }
                default:
                    assert(!"decoder state corrupt");
                    return -1;
            }
        }
    }

    bool phr_decode_chunked_is_in_data(phr_chunked_decoder *decoder) {
        return decoder->_state == CHUNKED_IN_CHUNK_DATA;
    }
}

#if RAINY_USING_MSVC
#pragma warning(pop)
#endif