#ifndef RAINY_FOUNDATION_NETWORK_HTTP_PARSER_HPP
#define RAINY_FOUNDATION_NETWORK_HTTP_PARSER_HPP
#include <rainy/foundation/network/config.hpp>
#include <charconv>


#ifndef RAINY_MAX_HTTP_HEADER_FIELD_SIZE
#define RAINY_MAX_HTTP_HEADER_FIELD_SIZE 100
#endif

namespace rainy::foundation::network::http {
    struct http_header {
        std::string_view name;
        std::string_view value;
    };
}

namespace rainy::foundation::network::http::implements {
    static const char *token_char_map = "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
                                        "\0\1\0\1\1\1\1\1\0\0\1\1\0\1\1\0\1\1\1\1\1\1\1\1\1\1\0\0\0\0\0\0"
                                        "\0\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\0\0\0\1\1"
                                        "\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\0\1\0\1\0"
                                        "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
                                        "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
                                        "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
                                        "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0";

    struct phr_chunked_decoder {
        std::size_t bytes_left_in_chunk;
        char consume_trailer;
        char _hex_count;
        char _state;
    };

    constexpr bool is_printable_ascii(unsigned char c) noexcept {
        return c - 040u < 0137u;
    }

    RAINY_TOOLKIT_API const char *get_token_to_eol(const char *buf, const char *buf_end, const char **token, std::size_t *token_len,
                                                   int *ret);
    RAINY_TOOLKIT_API const char *is_complete(const char *buf, const char *buf_end, std::size_t last_len, int *ret);
    RAINY_TOOLKIT_API const char *parse_http_version(const char *buf, const char *buf_end, int *minor_version, int *ret);
    RAINY_TOOLKIT_API const char *parse_headers(const char *buf, const char *buf_end, http_header *headers, std::size_t *num_headers,
                                                std::size_t max_headers, int *ret, bool &has_connection, bool &has_close,
                                                bool &has_upgrade);
    RAINY_TOOLKIT_API const char *parse_request(const char *buf, const char *buf_end, const char **method, std::size_t *method_len,
                                                const char **path, std::size_t *path_len, int *minor_version, http_header *headers,
                                                std::size_t *num_headers, std::size_t max_headers, int *ret, bool &has_connection,
                                                bool &has_close, bool &has_upgrade, bool &has_query);
    RAINY_TOOLKIT_API int phr_parse_request(const char *buf_start, std::size_t len, const char **method, std::size_t *method_len,
                                            const char **path, std::size_t *path_len, int *minor_version, http_header *headers,
                                            std::size_t *num_headers, std::size_t last_len, bool &has_connection, bool &has_close,
                                            bool &has_upgrade, bool &has_query);
    RAINY_TOOLKIT_API const char *parse_response(const char *buf, const char *buf_end, int *minor_version, int *status,
                                                 const char **msg, std::size_t *msg_len, http_header *headers, std::size_t *num_headers,
                                                 std::size_t max_headers, int *ret);
    RAINY_TOOLKIT_API int phr_parse_response(const char *buf_start, std::size_t len, int *minor_version, int *status, const char **msg,
                                             std::size_t *msg_len, http_header *headers, std::size_t *num_headers, std::size_t last_len);
    RAINY_TOOLKIT_API int phr_parse_headers(const char *buf_start, std::size_t len, http_header *headers, std::size_t *num_headers,
                                            std::size_t last_len);
    static int decode_hex(int ch) noexcept {
        if ('0' <= ch && ch <= '9') {
            return ch - '0';
        } else if ('A' <= ch && ch <= 'F') {
            return ch - 'A' + 0xa;
        } else if ('a' <= ch && ch <= 'f') {
            return ch - 'a' + 0xa;
        } else {
            return -1;
        }
    }

    RAINY_TOOLKIT_API core::ssize_t phr_decode_chunked(struct phr_chunked_decoder *decoder, char *buf, std::size_t *_bufsz);
    RAINY_TOOLKIT_API bool phr_decode_chunked_is_in_data(struct phr_chunked_decoder *decoder);

    inline bool iequal0(std::string_view a, std::string_view b) {
        return std::equal(a.begin(), a.end(), b.begin(), b.end(), [](char a, char b) { return tolower(a) == tolower(b); });
    }

    std::vector<std::string_view> split_sv(std::string_view s, std::string_view delimiter) {
        std::size_t start = 0;
        std::size_t end = s.find_first_of(delimiter);

        std::vector<std::string_view> output;

        while (end <= std::string_view::npos) {
            output.emplace_back(s.substr(start, end - start));

            if (end == std::string_view::npos)
                break;

            start = end + 1;
            end = s.find_first_of(delimiter, start);
        }

        return output;
    }
}

namespace rainy::foundation::network::http {
    class http_parser {
    public:
        void parse_body_len() {
            auto header_value = this->get_header_value("content-length");
            if (header_value.empty()) {
                body_len_ = 0;
            } else {
                auto [ptr, ec] = std::from_chars(header_value.data(), header_value.data() + header_value.size(), body_len_, 10);
                if (ec != std::errc{}) {
                    body_len_ = -1;
                }
            }
        }

        int parse_response(const char *data, std::size_t size, int last_len) {
            num_headers_ = RAINY_MAX_HTTP_HEADER_FIELD_SIZE;
            const char *msg;
            int minor_version;
            std::size_t msg_len;
            header_len_ = implements::phr_parse_response(data, size, &minor_version, &status_, &msg, &msg_len, headers_.data(),
                                                         &num_headers_, last_len);
            msg_ = {msg, msg_len};
            parse_body_len();
            if (header_len_ < 0) {
                // CINATRA_LOG_WARNING << "parse http head failed";
                if (num_headers_ == RAINY_MAX_HTTP_HEADER_FIELD_SIZE) {
                    output_error();
                }
            }
            return header_len_;
        }

        int parse_request(const char *data, std::size_t size, int last_len) {
            int minor_version;
            num_headers_ = RAINY_MAX_HTTP_HEADER_FIELD_SIZE;
            const char *method;
            std::size_t method_len;
            const char *url;
            std::size_t url_len;
            bool has_query{};
            header_len_ =
                implements::phr_parse_request(data, size, &method, &method_len, &url, &url_len, &minor_version, headers_.data(),
                                              &num_headers_, last_len, has_connection_, has_close_, has_upgrade_, has_query);
            if (header_len_ < 0) {
                // CINATRA_LOG_WARNING << "parse http head failed";
                if (num_headers_ == RAINY_MAX_HTTP_HEADER_FIELD_SIZE) {
                    output_error();
                }
                return header_len_;
            }
            method_ = {method, method_len};
            url_ = {url, url_len};
            auto methd_type = method_type(method_);
            if (methd_type == http_method::get || methd_type == http_method::head) {
                body_len_ = 0;
            } else {
                parse_body_len();
            }
            full_url_ = url_;
            if (!queries_.empty()) {
                queries_.clear();
            }
            if (has_query) {
                std::size_t pos = url_.find('?');
                parse_query(url_.substr(pos + 1, url_len - pos - 1));
                url_ = {url, pos};
            }
            return header_len_;
        }

        bool has_connection() {
            return has_connection_;
        }

        bool has_close() {
            return has_close_;
        }

        bool has_upgrade() {
            return has_upgrade_;
        }

        std::string_view get_header_value(std::string_view key) const {
            for (std::size_t i = 0; i < num_headers_; i++) {
                if (implements::iequal0(headers_[i].name, key))
                    return headers_[i].value;
            }
            return {};
        }

        const auto &queries() const {
            return queries_;
        }

        std::string_view full_url() {
            return full_url_;
        }

        std::string_view get_query_value(std::string_view key) {
            if (auto it = queries_.find(key); it != queries_.end()) {
                return it->second;
            } else {
                return "";
            }
        }

        bool is_chunked() const {
            auto transfer_encoding = this->get_header_value("transfer-encoding");
            if (transfer_encoding == "chunked") {
                return true;
            }

            return false;
        }

        bool is_multipart() {
            auto content_type = get_header_value("Content-Type");
            if (content_type.empty()) {
                return false;
            }
            if (content_type.find("multipart") == std::string_view::npos) {
                return false;
            }
            return true;
        }

        std::string_view get_boundary() {
            auto content_type = get_header_value("Content-Type");
            std::size_t pos = content_type.find("=--");
            if (pos == std::string_view::npos) {
                return {};
            }
            return content_type.substr(pos + 1);
        }

        bool is_resp_ranges() const {
            auto value = this->get_header_value("Accept-Ranges");
            return !value.empty();
        }

        bool is_websocket() const {
            auto upgrade = this->get_header_value("Upgrade");
            return upgrade == "WebSocket" || upgrade == "websocket";
        }

        bool keep_alive() const {
            if (is_websocket()) {
                return true;
            }
            auto val = this->get_header_value("connection");
            if (val.empty() || implements::iequal0(val, "keep-alive")) {
                return true;
            }
            return false;
        }

        int status() const {
            return status_;
        }

        int header_len() const {
            return header_len_;
        }

        int64_t body_len() const {
            return body_len_;
        }

        int64_t total_len() const {
            return header_len_ + body_len_;
        }

        bool is_location() {
            auto location = this->get_header_value("Location");
            return !location.empty();
        }

        std::string_view msg() const {
            return msg_;
        }

        std::string_view method() const {
            return method_;
        }

        std::string_view url() const {
            return url_;
        }

        collections::views::array_view<http_header> get_headers() {
            return {headers_.data(), num_headers_};
        }

        void parse_query(std::string_view str) {
            std::string_view key;
            std::string_view val;
            auto vec = implements::split_sv(str, "&");
            for (auto s: vec) {
                if (s.empty()) {
                    continue;
                }
                std::size_t pos = s.find('=');
                if (s.find('=') != std::string_view::npos) {
                    key = s.substr(0, pos);
                    if (key.empty()) {
                        continue;
                    }
                    val = s.substr(pos + 1, s.length() - pos);
                } else {
                    key = s;
                    val = "";
                }
                queries_.emplace(key, val);
            }
        }

    private:
        void output_error() {
            /*CINATRA_LOG_ERROR << "the field of http head is out of max limit " << RAINY_MAX_HTTP_HEADER_FIELD_SIZE
                              << ", you can define macro "
                                 "RAINY_MAX_HTTP_HEADER_FIELD_SIZE to expand it.";*/
        }

    private:
        int status_ = 0;
        std::string_view msg_;
        std::size_t num_headers_ = 0;
        int header_len_ = 0;
        std::int64_t body_len_ = 0;
        bool has_connection_{};
        bool has_close_{};
        bool has_upgrade_{};
        collections::array<http_header, RAINY_MAX_HTTP_HEADER_FIELD_SIZE> headers_;
        std::string_view method_;
        std::string_view url_;
        std::string_view full_url_;
        collections::dense_map<std::string_view, std::string_view> queries_;
    };
}

#endif