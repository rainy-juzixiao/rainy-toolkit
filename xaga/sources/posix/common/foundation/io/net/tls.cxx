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
#include <rainy/foundation/io/net/implements/ssl_impl.hpp>

#if RAINY_HAS_OPENSSL
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <openssl/err.h>
#include <openssl/ssl.h>
#include <openssl/x509v3.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <vector>

#if RAINY_USING_MACOS
#include <sys/select.h> // NOLINT
#endif

namespace rainy::foundation::io::net::implements {
    struct openssl_global_guard {
        openssl_global_guard() {
            SSL_library_init();
            SSL_load_error_strings();
            OpenSSL_add_all_algorithms();
        }
        ~openssl_global_guard() {
            ERR_free_strings();
            EVP_cleanup();
        }
    };

    void ensure_openssl_init() {
        static openssl_global_guard guard;
        (void) guard;
    }

    static std::error_code openssl_ec(const int ret) noexcept {
        if (ret > 0) {
            return {};
        }
        switch (const int err = SSL_get_error(nullptr, ret)) {
            case SSL_ERROR_WANT_READ:
            case SSL_ERROR_WANT_WRITE:
                return std::make_error_code(std::errc::resource_unavailable_try_again);
            case SSL_ERROR_SYSCALL:
                if (ERR_peek_error() == 0) {
                    return std::make_error_code(std::errc::connection_reset);
                }
                return std::make_error_code(std::errc::io_error);
            case SSL_ERROR_SSL:
                return {static_cast<int>(-ERR_get_error()), std::system_category()};
            case SSL_ERROR_ZERO_RETURN:
                return {};
            default:
                return {-err, std::system_category()};
        }
    }

    static std::error_code openssl_ec(SSL *ssl, const int ret) noexcept {
        if (ret > 0)
            return {};
        const int err = SSL_get_error(ssl, ret);
        switch (err) {
            case SSL_ERROR_WANT_READ:
            case SSL_ERROR_WANT_WRITE:
                return std::make_error_code(std::errc::resource_unavailable_try_again);
            case SSL_ERROR_ZERO_RETURN:
                return {}; // 正常 close_notify
            case SSL_ERROR_SYSCALL:
                if (ERR_peek_error() == 0) {
                    return std::make_error_code(std::errc::connection_reset);
                }
                return std::make_error_code(std::errc::io_error);
            case SSL_ERROR_SSL: {
                return {static_cast<int>(-ERR_get_error()), std::system_category()};
            }
            default:
                return {-err, std::system_category()};
        }
    }

    class ssl_context_impl : public ssl_context_impl_base {
    public:
        ssl_context_impl() {
            ensure_openssl_init();
        }

        ~ssl_context_impl() override {
            if (ssl_ctx_) {
                SSL_CTX_free(ssl_ctx_);
            }
        }

        std::error_code init(const ssl::method mtd) noexcept override {
            method_ = mtd;
            const SSL_METHOD *ssl_method = nullptr;
            switch (mtd) {
                case ssl::method::tls_client:
                case ssl::method::tlsv13_client:
                case ssl::method::tlsv12_client:
                    ssl_method = TLS_client_method();
                    break;
                case ssl::method::tls_server:
                case ssl::method::tlsv13_server:
                case ssl::method::tlsv12_server:
                    ssl_method = TLS_server_method();
                    break;
                default:
                    return std::make_error_code(std::errc::invalid_argument);
            }
            ssl_ctx_ = SSL_CTX_new(ssl_method);
            if (!ssl_ctx_) {
                return openssl_ec(-1);
            }
            SSL_CTX_set_options(ssl_ctx_, SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3 | SSL_OP_NO_COMPRESSION);
            if (mtd == ssl::method::tlsv13_client || mtd == ssl::method::tlsv13_server) {
                SSL_CTX_set_options(ssl_ctx_, SSL_OP_NO_TLSv1_2);
                SSL_CTX_set_min_proto_version(ssl_ctx_, TLS1_3_VERSION);
                SSL_CTX_set_max_proto_version(ssl_ctx_, TLS1_3_VERSION);
            } else if (mtd == ssl::method::tlsv12_client || mtd == ssl::method::tlsv12_server) {
                SSL_CTX_set_options(ssl_ctx_, SSL_OP_NO_TLSv1_3);
                SSL_CTX_set_min_proto_version(ssl_ctx_, TLS1_2_VERSION);
                SSL_CTX_set_max_proto_version(ssl_ctx_, TLS1_2_VERSION);
            }
            return {};
        }

        std::error_code set_options(const uint64_t options) noexcept override {
            options_ = options;
            if (ssl_ctx_) {
                SSL_CTX_set_options(ssl_ctx_, options);
            }
            return {};
        }

        std::error_code set_verify_mode(ssl::verify_mode mode) noexcept override {
            verify_mode_ = mode;
            if (ssl_ctx_) {
                int ssl_mode = SSL_VERIFY_NONE;
                if (static_cast<int>(mode) & static_cast<int>(ssl::verify_mode::peer)) {
                    ssl_mode = SSL_VERIFY_PEER;
                    if (static_cast<int>(mode) & static_cast<int>(ssl::verify_mode::fail_if_no_peer_cert)) {
                        ssl_mode |= SSL_VERIFY_FAIL_IF_NO_PEER_CERT;
                    }
                }
                SSL_CTX_set_verify(ssl_ctx_, ssl_mode, nullptr);
            }
            return {};
        }

        std::error_code set_verify_depth(const int depth) noexcept override {
            verify_depth_ = depth;
            if (ssl_ctx_) {
                SSL_CTX_set_verify_depth(ssl_ctx_, depth);
            }
            return {};
        }

        RAINY_NODISCARD ssl::verify_mode verify_mode() const noexcept override {
            return verify_mode_;
        }

        int verify_depth() noexcept override {
            return verify_depth_;
        }

        std::error_code set_cipher_list(const char *ciphers) noexcept override { // NOLINT
            if (!ciphers || !ssl_ctx_) {
                return std::make_error_code(std::errc::invalid_argument);
            }
            cipher_list_ = ciphers;
            if (SSL_CTX_set_cipher_list(ssl_ctx_, ciphers) != 1) {
                return openssl_ec(-1);
            }
            return {};
        }

        std::error_code set_alpn_protos(const collections::vector<text::string> &protos) noexcept override { // NOLINT
            alpn_protos_ = protos;
            if (ssl_ctx_ && !protos.empty()) {
                std::vector<unsigned char> alpn_data;
                for (const auto &proto: protos) {
                    alpn_data.push_back(static_cast<unsigned char>(proto.size()));
                    alpn_data.insert(alpn_data.end(), proto.begin(), proto.end());
                }
                if (SSL_CTX_set_alpn_protos(ssl_ctx_, alpn_data.data(), alpn_data.size()) != 0) {
                    return openssl_ec(-1);
                }
            }
            return {};
        }

        std::error_code use_certificate_file(const char *path, const char * /*password*/) noexcept override { // NOLINT
            cert_file_ = path ? path : "";
            return {};
        }

        std::error_code use_certificate_chain_file(const char *path) noexcept override {
            if (!path || !ssl_ctx_) {
                return std::make_error_code(std::errc::invalid_argument);
            }
            if (SSL_CTX_use_certificate_chain_file(ssl_ctx_, path) != 1) {
                return openssl_ec(-1);
            }
            return {};
        }

        std::error_code use_private_key_file(const char *path, const char *password) noexcept override { // NOLINT
            key_file_ = path ? path : "";
            key_password_ = password ? password : "";

            if (!cert_file_.empty() && !key_file_.empty() && ssl_ctx_) {
                return load_cert_key();
            }
            return {};
        }

        std::error_code use_certificate_buffer(const void *data, const std::size_t size) noexcept override {
            if (!data || size == 0 || !ssl_ctx_) {
                return std::make_error_code(std::errc::invalid_argument);
            }

            BIO *bio = BIO_new_mem_buf(data, static_cast<int>(size));
            if (!bio) {
                return openssl_ec(-1);
            }

            X509 *cert = PEM_read_bio_X509(bio, nullptr, nullptr, nullptr);
            BIO_free(bio);

            if (!cert) {
                return openssl_ec(-1);
            }

            if (SSL_CTX_use_certificate(ssl_ctx_, cert) != 1) {
                X509_free(cert);
                return openssl_ec(-1);
            }

            X509_free(cert);
            return {};
        }

        std::error_code use_system_store() noexcept override {
            if (!ssl_ctx_) {
                return std::make_error_code(std::errc::invalid_argument);
            }
            if (SSL_CTX_set_default_verify_paths(ssl_ctx_) != 1) {
                return openssl_ec(-1);
            }
            return {};
        }

        std::error_code set_session_cache_size(const std::size_t size) noexcept override {
            session_cache_size_ = size;
            if (ssl_ctx_) {
                SSL_CTX_sess_set_cache_size(ssl_ctx_, size);
            }
            return {};
        }

        void *native_handle() noexcept override {
            return ssl_ctx_;
        }

        RAINY_NODISCARD SSL_CTX *ssl_ctx() const noexcept {
            return ssl_ctx_;
        }

        RAINY_NODISCARD ssl::method method() const noexcept {
            return method_;
        }

        RAINY_NODISCARD int verify_depth() const noexcept {
            return verify_depth_;
        }

        RAINY_NODISCARD const collections::vector<text::string> &alpn_protos() const noexcept override {
            return alpn_protos_;
        }

        std::error_code add_verify_path(const char *path) noexcept override {
            if (!path || !ssl_ctx_) {
                return std::make_error_code(std::errc::invalid_argument);
            }

            struct stat st{};
            if (stat(path, &st) != 0) {
                return std::make_error_code(std::errc::no_such_file_or_directory);
            }

            int ret = 0;
            if (S_ISDIR(st.st_mode)) {
                ret = SSL_CTX_load_verify_locations(ssl_ctx_, nullptr, path);
            } else {
                ret = SSL_CTX_load_verify_locations(ssl_ctx_, path, nullptr);
            }

            if (ret != 1) {
                return openssl_ec(-1);
            }

            verify_paths_.push_back(text::string(path));
            return {};
        }

        std::error_code use_windows_store(const char *store_name) noexcept override {
            // OpenSSL 不直接支持 Windows 证书存储，需要手动实现
            return std::make_error_code(std::errc::not_supported);
        }

        std::error_code use_linux_store(const char *store_path) noexcept override {
            if (!ssl_ctx_) {
                return std::make_error_code(std::errc::invalid_argument);
            }

            const char *ca_path = store_path;
            if (!store_path || store_path[0] == '\0') {
                constexpr const char *default_paths[] = {"/etc/ssl/certs", "/etc/pki/tls/certs", "/usr/local/share/ca-certificates",
                                                         "/usr/share/ca-certificates"};

                for (const char *default_path: default_paths) {
                    struct stat st{};
                    if (stat(default_path, &st) == 0 && S_ISDIR(st.st_mode)) {
                        ca_path = default_path;
                        break;
                    }
                }
            }

            if (SSL_CTX_set_default_verify_paths(ssl_ctx_) == 1) {
                return {};
            }

            if (SSL_CTX_load_verify_locations(ssl_ctx_, nullptr, ca_path) != 1) {
                return openssl_ec(-1);
            }

            verify_paths_.push_back(text::string(ca_path));
            return {};
        }

        void *native_credentials() noexcept override {
            return ssl_ctx_;
        }

        RAINY_NODISCARD bool is_server() const noexcept override {
            return method_ == ssl::method::tls_server || method_ == ssl::method::tlsv12_server ||
                   method_ == ssl::method::tlsv13_server;
        }

    private:
        std::error_code load_cert_key() noexcept { // NOLINT
            // NOLINTBEGIN
            if (!ssl_ctx_) {
                return std::make_error_code(std::errc::invalid_argument);
            }
            // NOLINTEND
            // 加载证书
            if (SSL_CTX_use_certificate_file(ssl_ctx_, cert_file_.c_str(), SSL_FILETYPE_PEM) != 1) {
                return openssl_ec(-1);
            }
            // 加载私钥
            if (!key_password_.empty()) {
                SSL_CTX_set_default_passwd_cb_userdata(ssl_ctx_, const_cast<char *>(key_password_.c_str()));
            }
            if (SSL_CTX_use_PrivateKey_file(ssl_ctx_, key_file_.c_str(), SSL_FILETYPE_PEM) != 1) {
                return openssl_ec(-1);
            }
            // 验证私钥
            if (SSL_CTX_check_private_key(ssl_ctx_) != 1) {
                return openssl_ec(-1);
            }
            return {};
        }

        SSL_CTX *ssl_ctx_{nullptr};
        ssl::method method_{ssl::method::tls_client};
        ssl::verify_mode verify_mode_{ssl::verify_mode::none};
        int verify_depth_{9};
        uint64_t options_{0};
        std::size_t session_cache_size_{0};
        text::string cipher_list_;
        text::string cert_file_;
        text::string key_file_;
        text::string key_password_;
        collections::vector<text::string> alpn_protos_;
        collections::vector<text::string> verify_paths_;
    };

    class ssl_stream_impl final : public ssl_stream_impl_base {
    public:
        ssl_stream_impl() = default;

        ~ssl_stream_impl() override {
            destroy_session();
        }

        std::error_code attach(const native_socket_t sock, const bool is_server) noexcept override {
            if (sock == invalid_socket_value) {
                return std::make_error_code(std::errc::bad_file_descriptor);
            }
            destroy_session();
            socket_fd_ = sock;
            is_server_ = is_server;

            SSL_CTX *raw_ctx = nullptr;
            if (pending_ctx_) {
                raw_ctx = pending_ctx_->ssl_ctx();
            } else {
                owned_ctx_.reset(SSL_CTX_new(TLS_method()));
                if (!owned_ctx_) {
                    return openssl_ec(ssl_, -1);
                }
                SSL_CTX_set_default_verify_paths(owned_ctx_.get());
                raw_ctx = owned_ctx_.get();
            }

            ssl_ = SSL_new(raw_ctx);
            if (!ssl_) {
                return openssl_ec(ssl_, -1);
            }
            if (is_server) {
                SSL_set_accept_state(ssl_);
            } else {
                SSL_set_connect_state(ssl_);
            }
            SSL_set_fd(ssl_, static_cast<int>(sock));

            if (pending_ctx_) {
                return apply_context_internal(pending_ctx_);
            }
            return {};
        }

        std::error_code apply_context(ssl_context_impl *ctx) noexcept {
            pending_ctx_ = ctx;
            if (ssl_) {
                return apply_context_internal(ctx);
            }
            return {};
        }

        RAINY_NODISCARD native_socket_t underlying_socket() const noexcept override {
            return socket_fd_;
        }

        std::error_code handshake() noexcept override {
            if (!ssl_) {
                return std::make_error_code(std::errc::not_connected);
            }

            const int flags = ::fcntl(socket_fd_, F_GETFL, 0);
            ::fcntl(socket_fd_, F_SETFL, flags & ~O_NONBLOCK);

            int ret = 0;
            // NOLINTBEGIN
            do {
                ret = is_server_ ? SSL_accept(ssl_) : SSL_connect(ssl_);
            } while (ret < 0 && SSL_get_error(ssl_, ret) == SSL_ERROR_WANT_READ);
            // NOLINTEND
            ::fcntl(socket_fd_, F_SETFL, flags);

            if (ret == 1) {
                handshaked_ = true;
                reset_operation();
                return {};
            }
            return openssl_ec(ssl_, ret);
        }

        void async_handshake(io_context::executor_type executor, completion_op *op) noexcept override {
            if (!ssl_ || !op) {
                return;
            }
            pending_op_ = ssl_operation_type::handshake;

            if (const int ret = is_server_ ? SSL_accept(ssl_) : SSL_connect(ssl_); ret == 1) {
                handshaked_ = true;
                wants_retry_ = false;
                wants_read_ = false;
                wants_write_ = false;
                op->complete(io::implements::op_result{}, false);
            } else {
                if (const int err = SSL_get_error(ssl_, ret); err == SSL_ERROR_WANT_READ) {
                    wants_read_ = true;
                    wants_write_ = false;
                    wants_retry_ = true;
                } else if (err == SSL_ERROR_WANT_WRITE) {
                    wants_read_ = false;
                    wants_write_ = true;
                    wants_retry_ = true;
                } else {
                    wants_retry_ = false;
                    const std::error_code ec = openssl_ec(ssl_, err);
                    op->complete(io::implements::op_result{nullptr, 0, ec.value()}, false); // NOLINT
                }
            }
        }

        std::error_code shutdown() noexcept override {
            if (!ssl_) {
                return {};
            }
            int ret = SSL_shutdown(ssl_);
            if (ret == 0) {
                ret = SSL_shutdown(ssl_);
            }
            handshaked_ = false;
            return ret == 1 ? std::error_code{} : openssl_ec(ssl_, ret);
        }

        void async_shutdown(io_context::executor_type executor, completion_op *op) noexcept override {
            if (!ssl_ || !op) {
                return;
            }
            pending_op_ = ssl_operation_type::shutdown;

            if (const int ret = SSL_shutdown(ssl_); ret == 1) {
                handshaked_ = false;
                wants_retry_ = false;
                op->complete(io::implements::op_result{}, false);
            } else if (ret == 0) {
                wants_retry_ = true;
                wants_write_ = true;
            } else {
                if (const int err = SSL_get_error(ssl_, ret); err == SSL_ERROR_WANT_READ) {
                    wants_read_ = true;
                    wants_retry_ = true;
                } else if (err == SSL_ERROR_WANT_WRITE) {
                    wants_write_ = true;
                    wants_retry_ = true;
                } else {
                    op->complete(io::implements::op_result{nullptr, 0, -err}, false); // NOLINT
                }
            }
        }

        std::ptrdiff_t write_some(const void *buf, const std::size_t len, std::error_code &ec) noexcept override {
            if (!ssl_ || !handshaked_) {
                ec = std::make_error_code(std::errc::not_connected);
                return -1;
            }
            pending_op_ = ssl_operation_type::write;
            const int ret = SSL_write(ssl_, buf, static_cast<int>(len));

            if (ret > 0) {
                ec.clear();
                reset_operation();
                return ret;
            }
            if (const int err = SSL_get_error(ssl_, ret); err == SSL_ERROR_WANT_READ) {
                wants_read_ = true;
                wants_write_ = false;
                wants_retry_ = true;
            } else if (err == SSL_ERROR_WANT_WRITE) {
                wants_read_ = false;
                wants_write_ = true;
                wants_retry_ = true;
            } else {
                wants_retry_ = false;
            }
            ec = openssl_ec(ssl_, ret);
            return -1;
        }

        std::ptrdiff_t read_some(void *buf, const std::size_t len, std::error_code &ec) noexcept override {
            if (!ssl_ || !handshaked_) {
                ec = std::make_error_code(std::errc::not_connected);
                return -1;
            }
            pending_op_ = ssl_operation_type::read;
            const int ret = SSL_read(ssl_, buf, static_cast<int>(len));
            if (ret > 0) {
                ec.clear();
                reset_operation();
                return ret;
            }
            if (ret == 0) {
                ec.clear();
                handshaked_ = false;
                return 0;
            }
            if (const int err = SSL_get_error(ssl_, ret); err == SSL_ERROR_WANT_READ) {
                wants_read_ = true;
                wants_write_ = false;
                wants_retry_ = true;
            } else if (err == SSL_ERROR_WANT_WRITE) {
                wants_read_ = false;
                wants_write_ = true;
                wants_retry_ = true;
            } else if (err == SSL_ERROR_SSL) {
                const unsigned long err_code = ERR_peek_error();
                const int reason = ERR_GET_REASON(err_code);
                if (reason == SSL_R_UNEXPECTED_EOF_WHILE_READING) {
                    ERR_clear_error();
                    ec.clear();
                    handshaked_ = false;
                    return 0; // 当作 EOF
                }
                /*
                 * workaround for openssl posix/macOS
                 * 并不是所有TLS服务器的实现都标准，例如谷歌的服务器，可能是服务器节点的问题，也可能是服务器本身为了优化，不会走标准行为
                 * 但无论如何，此处，我们则进行完整的一次判断，来把SSL_R_DECRYPTION_FAILED_OR_BAD_RECORD_MAC的情况完全区分开来，避免走错
                 * 或是默许错误的通信流程
                 *
                 * 详细帖子：
                 * http://erlang.org/pipermail/erlang-questions/2010-July/052319.html
                 * https://hachyderm.io/@harrysintonen@infosec.exchange/111857045361610169
                */
                if (reason == SSL_R_DECRYPTION_FAILED_OR_BAD_RECORD_MAC) {
                    // 验证连接是否真的已关闭
                    char tmp = 0;
                    const ssize_t recv_ret = ::recv(socket_fd_, &tmp, 1, MSG_PEEK | MSG_DONTWAIT);
                    if (recv_ret == 0) {
                        // 连接已正常关闭
                        ERR_clear_error();
                        ec.clear();
                        handshaked_ = false;
                        return 0;
                    }
                    if (recv_ret == -1 && (errno == ECONNRESET || errno == EPIPE)) {
                        // 连接被 RST
                        ERR_clear_error();
                        ec.clear();
                        handshaked_ = false;
                        return 0;
                    }
                    wants_retry_ = false;
                }
                wants_retry_ = false;
            } else {
                wants_retry_ = false;
            }
            ec = openssl_ec(ssl_, ret);
            return -1;
        }

        void async_write_some(const void *buf, const std::size_t len, io_context::executor_type /* executor */,
                              completion_op *op) noexcept override {
            if (!op) {
                return;
            }
            std::error_code ec;
            const std::ptrdiff_t n = write_some(buf, len, ec);
            if (!ec || !wants_retry_) {
                op->complete(io::implements::op_result{nullptr, static_cast<std::size_t>(n > 0 ? n : 0), ec ? ec.value() : 0},
                             false); // NOLINT
            }
        }

        void async_read_some(void *buf, const std::size_t len, io_context::executor_type /* executor */,
                             completion_op *op) noexcept override {
            if (!op) {
                return;
            }
            std::error_code ec;
            const std::ptrdiff_t n = read_some(buf, len, ec);
            if (!ec || !wants_retry_) {
                op->complete(io::implements::op_result{nullptr, static_cast<std::size_t>(n > 0 ? n : 0), ec ? ec.value() : 0},
                             false); // NOLINT
            }
        }

        RAINY_NODISCARD bool is_handshaked() const noexcept override {
            return handshaked_;
        }

        RAINY_NODISCARD bool is_open() const noexcept override {
            return ssl_ != nullptr && socket_fd_ != invalid_socket_value;
        }

        RAINY_NODISCARD text::string negotiated_protocol() const noexcept override { // NOLINT
            if (!ssl_) {
                return {};
            }
            const unsigned char *proto = nullptr;
            unsigned int proto_len = 0;
            SSL_get0_alpn_selected(ssl_, &proto, &proto_len);
            if (proto && proto_len > 0) {
                return {reinterpret_cast<const char *>(proto), proto_len};
            }
            return {};
        }

        RAINY_NODISCARD text::string peer_certificate_subject() const noexcept override {
            return get_cert_field(true);
        }

        RAINY_NODISCARD text::string peer_certificate_issuer() const noexcept override {
            return get_cert_field(false);
        }

        void set_server_name(const char *hostname) noexcept override { // NOLINT
            server_name_ = hostname ? hostname : "";
            if (ssl_ && !server_name_.empty() && !is_server_) {
                SSL_set_tlsext_host_name(ssl_, server_name_.c_str());
            }
        }

        RAINY_NODISCARD const char *server_name() const noexcept override {
            return server_name_.c_str();
        }

        void *native_handle() noexcept override {
            return ssl_;
        }

        RAINY_NODISCARD ssl_operation_type pending_operation() const noexcept override {
            return pending_op_;
        }

        RAINY_NODISCARD bool wants_retry() const noexcept override {
            return wants_retry_;
        }

        RAINY_NODISCARD bool wants_read() const noexcept override {
            return wants_read_;
        }

        RAINY_NODISCARD bool wants_write() const noexcept override {
            return wants_write_;
        }

        void reset_operation() noexcept override {
            pending_op_ = ssl_operation_type::read;
            wants_retry_ = false;
            wants_read_ = false;
            wants_write_ = false;
        }

        std::error_code apply_context(const ssl_context_params &params) noexcept override {
            if (!ssl_) {
                return std::make_error_code(std::errc::invalid_argument);
            }
            SSL_set_verify(ssl_, params.verify_mode, nullptr);
            SSL_set_verify_depth(ssl_, params.verify_depth);

            if (!params.alpn_protos.empty()) {
                std::vector<unsigned char> alpn_data;
                for (const auto &proto: params.alpn_protos) {
                    alpn_data.push_back(static_cast<unsigned char>(proto.size()));
                    alpn_data.insert(alpn_data.end(), proto.begin(), proto.end());
                }
                SSL_set_alpn_protos(ssl_, alpn_data.data(), static_cast<unsigned int>(alpn_data.size()));
            }
            return {};
        }

    private:
        void destroy_session() noexcept {
            if (ssl_) {
                SSL_shutdown(ssl_);
                SSL_free(ssl_);
                ssl_ = nullptr;
            }
            owned_ctx_.reset();
            handshaked_ = false;
            reset_operation();
        }

        std::error_code apply_context_internal(ssl_context_impl *ctx) noexcept { // NOLINT
            if (!ctx || !ssl_) {
                return std::make_error_code(std::errc::invalid_argument);
            }
            int mode = SSL_VERIFY_NONE;
            if (const auto vm = ctx->verify_mode(); static_cast<int>(vm) & static_cast<int>(ssl::verify_mode::peer)) {
                mode = SSL_VERIFY_PEER;
                if (static_cast<int>(vm) & static_cast<int>(ssl::verify_mode::fail_if_no_peer_cert)) {
                    mode |= SSL_VERIFY_FAIL_IF_NO_PEER_CERT;
                }
            }
            SSL_set_verify(ssl_, mode, nullptr);
            SSL_set_verify_depth(ssl_, ctx->verify_depth());

            if (const auto &protos = ctx->alpn_protos(); !protos.empty()) {
                std::vector<unsigned char> alpn_data;
                for (const auto &p: protos) {
                    alpn_data.push_back(static_cast<unsigned char>(p.size()));
                    alpn_data.insert(alpn_data.end(), p.begin(), p.end());
                }
                SSL_set_alpn_protos(ssl_, alpn_data.data(), static_cast<unsigned int>(alpn_data.size()));
            }

            if (!is_server_ && !server_name_.empty()) {
                SSL_set_tlsext_host_name(ssl_, server_name_.c_str());
            }
            return {};
        }

        RAINY_NODISCARD text::string get_cert_field(const bool subject) const noexcept { // NOLINT
            if (!ssl_) {
                return {};
            }
            X509 *cert = SSL_get_peer_certificate(ssl_);
            if (!cert) {
                return {};
            }
            char buf[512];
            const X509_NAME *name = subject ? X509_get_subject_name(cert) : X509_get_issuer_name(cert);
            X509_NAME_oneline(name, buf, sizeof(buf));
            X509_free(cert);
            return {buf};
        }

        SSL *ssl_{nullptr};
        memory::nebula_ptr<SSL_CTX, decltype([](SSL_CTX *p) { SSL_CTX_free(p); })> owned_ctx_;
        native_socket_t socket_fd_{invalid_socket_value};
        ssl_context_impl *pending_ctx_{nullptr};
        bool is_server_{false};
        bool handshaked_{false};
        text::string server_name_;

        ssl_operation_type pending_op_{ssl_operation_type::read};
        bool wants_retry_{false};
        bool wants_read_{false};
        bool wants_write_{false};
    };

    memory::nebula_ptr<ssl_context_impl_base> create_ssl_context_impl() {
        return memory::make_nebula<ssl_context_impl>();
    }

    memory::nebula_ptr<ssl_stream_impl_base> create_ssl_stream_impl() {
        return memory::make_nebula<ssl_stream_impl>();
    }
}

#else

namespace rainy::foundation::io::net::implements {
    class openssl_not_implement_error : public exceptions::logic::logic_error {
    public:
        openssl_not_implement_error(const source &location) :
            logic_error("You see this exception it means that you cannot use tls crypto function without build with a tls backend.\n"
                        "If you don't have OpenSSL, you can follow these instructions:\n\n"
                        "Option 1: Install OpenSSL via system package manage (Unix-Like Platform)r:\n"
                        "  - Ubuntu/Debian/Mint: sudo apt install libssl-dev\n"
                        "  - Fedora/RHEL/CentOS: sudo dnf install openssl-devel\n"
                        "  - ArchLinux/Manjaro: sudo pacman -S openssl\n"
                        "  - Alpine Linux: sudo apk add openssl-dev\n"
                        "  - macOS (Homebrew): brew install openssl\n"
                        "  - macOS (MacPorts): sudo port install openssl\n\n"
                        "Option 2: Install OpenSSL via vcpkg (cross-platform):\n"
                        "  - Install vcpkg: git clone https://github.com/Microsoft/vcpkg.git\n"
                        "  - Bootstrap vcpkg: ./vcpkg/bootstrap-vcpkg.sh (Linux/macOS) or .\\vcpkg\\bootstrap-vcpkg.bat (Windows)\n"
                        "  - Install OpenSSL: ./vcpkg/vcpkg install openssl\n"
                        "  - Integrate with your build system: ./vcpkg/vcpkg integrate install\n\n"
                        "For more information, visit:\n"
                        "  - OpenSSL: https://www.openssl.org/\n"
                        "  - vcpkg: https://github.com/microsoft/vcpkg\n",
                        location) {
        }
    };

    static void throw_openssl_not_implement_error(
        const diagnostics::source_location &location = diagnostics::source_location::current()) {
        throw_exception(openssl_not_implement_error{location});
    }
}

namespace rainy::foundation::io::net::implements {
    memory::nebula_ptr<ssl_context_impl_base> create_ssl_context_impl() {
        throw_openssl_not_implement_error();
        return {};
    }

    memory::nebula_ptr<ssl_stream_impl_base> create_ssl_stream_impl() {
        throw_openssl_not_implement_error();
        return {};
    }
}

#endif
