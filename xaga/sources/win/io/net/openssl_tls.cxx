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
#include <iostream>
#include <vector>
#include <system_error>

#include <openssl/err.h>
#include <openssl/ssl.h>
#include <openssl/x509v3.h>
#include <openssl/pem.h>
#include <openssl/pkcs12.h>

#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include <wincrypt.h>

namespace rainy::foundation::io::net::implements {
    
    struct common_ssl_socket_proxy : io_context::executor_type {
        using executor_type::associate_handle;
        using executor_type::post_immediate_completion;
    };

    struct openssl_global_guard {
        openssl_global_guard() {
            OPENSSL_init_ssl(OPENSSL_INIT_SSL_DEFAULT, nullptr);
        }
        ~openssl_global_guard() {
            // OpenSSL 3.0+ 不需要显式清理
        }
    };

    void ensure_openssl_init() {
        static openssl_global_guard guard;
        (void)guard;
    }

    static std::error_code openssl_ec_ctx_level() noexcept {
        const unsigned long err = ERR_get_error();
        if (err == 0) {
            return std::make_error_code(std::errc::io_error);
        }
        char buf[256];
        ERR_error_string_n(err, buf, sizeof(buf));
        std::cerr << "OpenSSL ctx error: " << buf << "\n";
        ERR_clear_error();
        return std::make_error_code(std::errc::io_error);
    }


    static std::error_code openssl_ec(const SSL *ssl, const int ret) noexcept {
        if (ret > 0) {
            return {};
        }

        // 必须先 peek，SSL_get_error 内部可能会影响队列
        const unsigned long ssl_err_code = ERR_peek_error();
        const int err = SSL_get_error(ssl, ret); // 这里 ssl=nullptr 的重载是 UB，见下

        switch (err) {
            case SSL_ERROR_WANT_READ:
            case SSL_ERROR_WANT_WRITE:
                ERR_clear_error();
                return std::make_error_code(std::errc::resource_unavailable_try_again);
            case SSL_ERROR_ZERO_RETURN:
                ERR_clear_error();
                return {};
            case SSL_ERROR_SYSCALL: {
                ERR_clear_error();
                int sys_err = WSAGetLastError();
                if (ssl_err_code == 0 && sys_err == 0) {
                    return std::make_error_code(std::errc::connection_reset);
                }
                if (sys_err != 0) {
                    return {sys_err, std::system_category()};
                }
                return std::make_error_code(std::errc::io_error);
            }
            case SSL_ERROR_SSL: {
                // 用 peek 到的码，不要再 get 一次
                char buf[256];
                ERR_error_string_n(ssl_err_code, buf, sizeof(buf)); // ← 用 ssl_err_code
                std::cerr << "OpenSSL error: " << buf << "\n";
                ERR_clear_error();
                // 不要放进 system_category，用 generic 或直接返回 io_error
                return std::make_error_code(std::errc::io_error);
            }
            default:
                ERR_clear_error();
                return {-err, std::system_category()};
        }
    }

    static std::error_code openssl_ec(const int ret) noexcept {
        return openssl_ec(nullptr, ret);
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
            if (cert_store_) {
                CertCloseStore(cert_store_, 0);
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
                SSL_CTX_set_options(ssl_ctx_, SSL_OP_NO_TLSv1 | SSL_OP_NO_TLSv1_1 | SSL_OP_NO_TLSv1_2);
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

        std::error_code set_cipher_list(const char *ciphers) noexcept override {
            if (!ciphers || !ssl_ctx_) {
                return std::make_error_code(std::errc::invalid_argument);
            }
            cipher_list_ = ciphers;
            if (SSL_CTX_set_cipher_list(ssl_ctx_, ciphers) != 1) {
                return openssl_ec(-1);
            }
            return {};
        }

        std::error_code set_alpn_protos(const collections::vector<text::string> &protos) noexcept override {
            alpn_protos_ = protos;
            if (ssl_ctx_ && !protos.empty()) {
                std::vector<unsigned char> alpn_data;
                for (const auto &proto : protos) {
                    alpn_data.push_back(static_cast<unsigned char>(proto.size()));
                    alpn_data.insert(alpn_data.end(), proto.begin(), proto.end());
                }
                if (SSL_CTX_set_alpn_protos(ssl_ctx_, alpn_data.data(), static_cast<unsigned int>(alpn_data.size())) != 0) {
                    return openssl_ec(-1);
                }
            }
            return {};
        }

        std::error_code use_certificate_file(const char *path, const char *password) noexcept override {
            if (!path || !ssl_ctx_) {
                return std::make_error_code(std::errc::invalid_argument);
            }

            cert_file_ = path;
            key_password_ = password ? password : "";

            ERR_clear_error();

            // 尝试 PEM 证书链
            if (SSL_CTX_use_certificate_chain_file(ssl_ctx_, path) == 1) {
                // 只加载证书，私钥由 use_private_key_file 单独处理
                return {};
            }

            ERR_clear_error();

            // 尝试 PKCS#12（同时含证书和私钥）
            return load_pkcs12(path, key_password_.c_str());
        }

        std::error_code use_certificate_chain_file(const char *path) noexcept override {
            return use_certificate_file(path, nullptr);
        }

        std::error_code use_private_key_file(const char *path, const char *password) noexcept override {
            if (!path || !ssl_ctx_) {
                return std::make_error_code(std::errc::invalid_argument);
            }
            key_file_ = path;
            key_password_ = password ? password : "";
            ERR_clear_error();
            if (!key_password_.empty()) {
                SSL_CTX_set_default_passwd_cb_userdata(ssl_ctx_, const_cast<char *>(key_password_.c_str()));
            }
            if (SSL_CTX_use_PrivateKey_file(ssl_ctx_, path, SSL_FILETYPE_PEM) != 1) {
                return openssl_ec_ctx_level();
            }
            // 证书已加载的情况下才 check，否则 check 会因为没有证书而误报
            if (!cert_file_.empty()) {
                if (SSL_CTX_check_private_key(ssl_ctx_) != 1) {
                    return openssl_ec_ctx_level();
                }
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
            if (!cert) {
                BIO_reset(bio);
                cert = d2i_X509_bio(bio, nullptr);
            }
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
            
            // Windows 系统证书存储
            HCERTSTORE system_store = CertOpenSystemStoreA(0, "ROOT");
            if (!system_store) {
                return {static_cast<int>(GetLastError()), std::system_category()};
            }
            
            X509_STORE *store = SSL_CTX_get_cert_store(ssl_ctx_);
            PCCERT_CONTEXT cert_ctx = nullptr;
            while ((cert_ctx = CertEnumCertificatesInStore(system_store, cert_ctx)) != nullptr) {
                const unsigned char *cert_data = cert_ctx->pbCertEncoded;
                X509 *x509 = d2i_X509(nullptr, &cert_data, cert_ctx->cbCertEncoded);
                if (x509) {
                    X509_STORE_add_cert(store, x509);
                    X509_free(x509);
                }
            }
            CertCloseStore(system_store, 0);
            
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

        RAINY_NODISCARD const collections::vector<text::string> &alpn_protos() const noexcept override {
            return alpn_protos_;
        }

        std::error_code add_verify_path(const char *path) noexcept override {
            if (!path || !ssl_ctx_) {
                return std::make_error_code(std::errc::invalid_argument);
            }

            // Windows 上，path 可能是证书文件或目录
            DWORD attrs = GetFileAttributesA(path);
            if (attrs == INVALID_FILE_ATTRIBUTES) {
                return std::make_error_code(std::errc::no_such_file_or_directory);
            }

            int ret = 0;
            if (attrs & FILE_ATTRIBUTE_DIRECTORY) {
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
            if (!ssl_ctx_) {
                return std::make_error_code(std::errc::invalid_argument);
            }
            
            const char *name = (store_name && store_name[0]) ? store_name : "ROOT";
            HCERTSTORE store = CertOpenSystemStoreA(0, name);
            if (!store) {
                return {static_cast<int>(GetLastError()), std::system_category()};
            }
            
            if (cert_store_) {
                CertCloseStore(cert_store_, 0);
            }
            cert_store_ = store;
            
            X509_STORE *x509_store = SSL_CTX_get_cert_store(ssl_ctx_);
            PCCERT_CONTEXT cert_ctx = nullptr;
            while ((cert_ctx = CertEnumCertificatesInStore(store, cert_ctx)) != nullptr) {
                const unsigned char *cert_data = cert_ctx->pbCertEncoded;
                X509 *x509 = d2i_X509(nullptr, &cert_data, cert_ctx->cbCertEncoded);
                if (x509) {
                    X509_STORE_add_cert(x509_store, x509);
                    X509_free(x509);
                }
            }
            
            return {};
        }

        std::error_code use_linux_store(const char *store_path) noexcept override {
            return use_system_store();
        }

        void *native_credentials() noexcept override {
            return ssl_ctx_;
        }

        RAINY_NODISCARD bool is_server() const noexcept override {
            return method_ == ssl::method::tls_server || method_ == ssl::method::tlsv12_server ||
                   method_ == ssl::method::tlsv13_server;
        }

    private:
        std::error_code load_private_key() noexcept {
            if (!ssl_ctx_ || key_file_.empty()) {
                return std::make_error_code(std::errc::invalid_argument);
            }
            
            if (!key_password_.empty()) {
                SSL_CTX_set_default_passwd_cb_userdata(ssl_ctx_, const_cast<char *>(key_password_.c_str()));
            }
            
            if (SSL_CTX_use_PrivateKey_file(ssl_ctx_, key_file_.c_str(), SSL_FILETYPE_PEM) != 1) {
                return openssl_ec(-1);
            }
            
            if (SSL_CTX_check_private_key(ssl_ctx_) != 1) {
                return openssl_ec(-1);
            }
            
            return {};
        }
        
        std::error_code load_pkcs12(const char *path, const char *password) noexcept {
            FILE *fp = nullptr;
            if (fopen_s(&fp, path, "rb") != 0 || !fp) {
                return std::make_error_code(std::errc::no_such_file_or_directory);
            }
            
            PKCS12 *p12 = d2i_PKCS12_fp(fp, nullptr);
            fclose(fp);
            
            if (!p12) {
                return openssl_ec(-1);
            }
            
            EVP_PKEY *pkey = nullptr;
            X509 *cert = nullptr;
            STACK_OF(X509) *ca = nullptr;
            
            if (PKCS12_parse(p12, password, &pkey, &cert, &ca) != 1) {
                PKCS12_free(p12);
                return openssl_ec(-1);
            }
            
            PKCS12_free(p12);
            
            if (cert) {
                if (SSL_CTX_use_certificate(ssl_ctx_, cert) != 1) {
                    EVP_PKEY_free(pkey);
                    X509_free(cert);
                    return openssl_ec(-1);
                }
                X509_free(cert);
            }
            
            if (pkey) {
                if (SSL_CTX_use_PrivateKey(ssl_ctx_, pkey) != 1) {
                    EVP_PKEY_free(pkey);
                    return openssl_ec(-1);
                }
                EVP_PKEY_free(pkey);
            }
            
            if (ca) {
                X509_STORE *store = SSL_CTX_get_cert_store(ssl_ctx_);
                for (int i = 0; i < sk_X509_num(ca); ++i) {
                    X509_STORE_add_cert(store, sk_X509_value(ca, i));
                }
                sk_X509_pop_free(ca, X509_free);
            }
            
            return {};
        }

        SSL_CTX *ssl_ctx_{nullptr};
        HCERTSTORE cert_store_{nullptr};
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
        struct iocp_op : completion_op {
            OVERLAPPED overlapped{};
            HANDLE associated_handle{INVALID_HANDLE_VALUE};
            DWORD transferred{0};
            completion_op *linked_op{nullptr};

            explicit iocp_op(fn_type f) noexcept : completion_op(f) {
            }

            static iocp_op *from_overlapped(OVERLAPPED *ov) noexcept {
                return reinterpret_cast<iocp_op *>(reinterpret_cast<char *>(ov) - offsetof(iocp_op, overlapped));
            }
        };

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
            
            // 设置底层 socket
            SSL_set_fd(ssl_, static_cast<int>(sock));

            if (pending_ctx_) {
                return apply_context_internal(pending_ctx_);
            }
            return {};
        }

        std::error_code apply_context(ssl_context_impl_base *ctx) noexcept override {
            pending_ctx_ = static_cast<ssl_context_impl *>(ctx);
            if (ssl_) {
                return apply_context_internal(static_cast<ssl_context_impl *>(ctx));
            }
            return {};
        }

        RAINY_NODISCARD native_socket_t underlying_socket() const noexcept override {
            return socket_fd_;
        }

        std::error_code handshake() noexcept override {
            u_long mode = 0;
            ioctlsocket(socket_fd_, FIONBIO, &mode); // 切阻塞

            int ret = 0;
            int err = 0;
            do {
                ret = is_server_ ? SSL_accept(ssl_) : SSL_connect(ssl_);
                if (ret == 1)
                    break;
                err = SSL_get_error(ssl_, ret);
            } while (err == SSL_ERROR_WANT_READ || err == SSL_ERROR_WANT_WRITE);
            // 啊呀! 这里先不恢复非阻塞
            if (ret == 1) {
                handshaked_ = true;
                reset_operation();
                return {};
            }
            return openssl_ec(ssl_, ret);
        }

        void async_handshake(io_context::executor_type executor, completion_op *op) noexcept override {
            if (!ssl_ || !op) {
                if (op) {
                    op->complete(io::implements::op_result{nullptr, 0, WSAENOTCONN}, false);
                }
                return;
            }
            
            pending_op_ = ssl_operation_type::handshake;
            const int ret = is_server_ ? SSL_accept(ssl_) : SSL_connect(ssl_);
            
            if (ret == 1) {
                handshaked_ = true;
                reset_operation();
                op->complete({}, false);
                return;
            }
            
            if (const int err = SSL_get_error(ssl_, ret); err == SSL_ERROR_WANT_READ || err == SSL_ERROR_WANT_WRITE) {
                wants_retry_ = true;
                wants_read_ = (err == SSL_ERROR_WANT_READ);
                wants_write_ = (err == SSL_ERROR_WANT_WRITE);
                
                auto *wait_op = new ssl_wait_op(this, executor, op, ssl_operation_type::handshake);
                common_ssl_socket_proxy{executor}.associate_handle(wait_op, static_cast<std::uintptr_t>(socket_fd_), nullptr);
                
                if (!wait_op->io_handle) {
                    delete wait_op;
                    op->complete(io::implements::op_result{nullptr, 0, WSAEBADF}, false);
                    return;
                }
                
                // 使用 IOCP 等待事件
                auto *iocp_op_ = static_cast<iocp_op *>(wait_op);
                ZeroMemory(&iocp_op_->overlapped, sizeof(OVERLAPPED));
                
                HANDLE iocp = reinterpret_cast<HANDLE>(wait_op->io_handle);
                PostQueuedCompletionStatus(iocp, 0, socket_fd_, &iocp_op_->overlapped);
            } else {
                op->complete(io::implements::op_result{nullptr, 0, openssl_ec(ssl_, ret).value()}, false);
            }
        }

       std::error_code shutdown() noexcept override {
            if (!ssl_) {
                return {};
            }

            int ret;
            int err;
            do {
                ret = SSL_shutdown(ssl_);
                if (ret == 1)
                    break;
                if (ret == 0) {
                    ret = SSL_shutdown(ssl_);
                    if (ret == 1)
                        break;
                }
                err = SSL_get_error(ssl_, ret);
            } while (err == SSL_ERROR_WANT_READ || err == SSL_ERROR_WANT_WRITE);

            handshaked_ = false;

            if (ret == 1) {
                return {};
            }
            return openssl_ec(ssl_, ret);
        }

        void async_shutdown(const io_context::executor_type executor, completion_op *op) noexcept override {
            u_long mode = 1;
            ioctlsocket(static_cast<SOCKET>(socket_fd_), FIONBIO, &mode);
            if (!ssl_ || !op) {
                if (op) {
                    op->complete(io::implements::op_result{nullptr, 0, WSAENOTCONN}, false);
                }
                return;
            }

            pending_op_ = ssl_operation_type::shutdown;
            const int ret = SSL_shutdown(ssl_);

            if (ret == 1) {
                handshaked_ = false;
                op->complete({}, false);
                return;
            }
            static thread_local char peek_buf[1];

            if (ret == 0) {
                wants_retry_ = true;
                wants_write_ = true;
                wants_read_ = false;

                auto *wait_op = new ssl_wait_op(this, executor, op, ssl_operation_type::shutdown);
                common_ssl_socket_proxy{executor}.associate_handle(nullptr, static_cast<std::uintptr_t>(socket_fd_), nullptr);

                auto *iocp_op_ = static_cast<iocp_op *>(wait_op);
                ZeroMemory(&iocp_op_->overlapped, sizeof(OVERLAPPED));
                WSABUF wb{1, peek_buf};
                DWORD flags = 0;
                DWORD recvd = 0;
                int r = WSARecv(static_cast<SOCKET>(socket_fd_), &wb, 1, &recvd, &flags, &iocp_op_->overlapped, nullptr);
                if (r == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING) {
                    delete wait_op;
                    handshaked_ = false;
                    op->complete({}, false);
                }
                return;
            }

            if (const int err = SSL_get_error(ssl_, ret); err == SSL_ERROR_WANT_READ || err == SSL_ERROR_WANT_WRITE) {
                wants_retry_ = true;
                wants_read_ = (err == SSL_ERROR_WANT_READ);
                wants_write_ = (err == SSL_ERROR_WANT_WRITE);
                
                auto *wait_op = new ssl_wait_op(this, executor, op, ssl_operation_type::shutdown);
                common_ssl_socket_proxy{executor}.associate_handle(wait_op, static_cast<std::uintptr_t>(socket_fd_), nullptr);
    
                if (!wait_op->io_handle) {
                    delete wait_op;
                    op->complete(io::implements::op_result{nullptr, 0, WSAEBADF}, false);
                    return;
                }
                
                auto *iocp_op_ = static_cast<iocp_op *>(wait_op);
                ZeroMemory(&iocp_op_->overlapped, sizeof(OVERLAPPED));
                HANDLE iocp = reinterpret_cast<HANDLE>(wait_op->io_handle);
                PostQueuedCompletionStatus(iocp, 0, socket_fd_, &iocp_op_->overlapped);
            } else {
                op->complete(io::implements::op_result{nullptr, 0, openssl_ec(ssl_, ret).value()}, false);
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
                    return 0;
                }
                /*
                 * workaround for openssl win32
                 * 并不是所有TLS服务器的实现都标准，例如谷歌的服务器，可能是服务器节点的问题，也可能是服务器本身为了优化，不会走标准行为
                 * 但无论如何，此处，我们则进行完整的一次判断，来把SSL_R_DECRYPTION_FAILED_OR_BAD_RECORD_MAC的情况完全区分开来，避免走错
                 * 或是默许错误的通信流程
                 *
                 * 详细帖子：
                 * http://erlang.org/pipermail/erlang-questions/2010-July/052319.html
                 * https://hachyderm.io/@harrysintonen@infosec.exchange/111857045361610169
                 */
                if (reason == SSL_R_DECRYPTION_FAILED_OR_BAD_RECORD_MAC) {
                    char tmp = 0;
                    int recv_ret = recv(socket_fd_, &tmp, 1, MSG_PEEK);
                    if (recv_ret == 0) {
                        ERR_clear_error();
                        ec.clear();
                        handshaked_ = false;
                        return 0;
                    }
                    if (recv_ret == SOCKET_ERROR) {
                        int wsa_err = WSAGetLastError();
                        if (wsa_err == WSAECONNRESET || wsa_err == WSAESHUTDOWN) {
                            ERR_clear_error();
                            ec.clear();
                            handshaked_ = false;
                            return 0;
                        }
                    }
                    wants_retry_ = false;
                } else {
                    wants_retry_ = false;
                }
            } else {
                wants_retry_ = false;
            }
            
            ec = openssl_ec(ssl_, ret);
            return -1;
        }

        void async_write_some(const void *buf, std::size_t len, io_context::executor_type executor,
                              completion_op *op) noexcept override {
            if (!op || !ssl_ || !handshaked_) {
                if (op) {
                    op->complete(io::implements::op_result{nullptr, 0, WSAENOTCONN}, false);
                }
                return;
            }
            
            pending_op_ = ssl_operation_type::write;
            const int ret = SSL_write(ssl_, buf, static_cast<int>(len));
            
            if (ret > 0) {
                reset_operation();
                op->complete(io::implements::op_result{nullptr, static_cast<std::size_t>(ret), 0}, false);
                return;
            }
            
            if (const int err = SSL_get_error(ssl_, ret); err == SSL_ERROR_WANT_READ || err == SSL_ERROR_WANT_WRITE) {
                wants_retry_ = true;
                wants_read_ = (err == SSL_ERROR_WANT_READ);
                wants_write_ = (err == SSL_ERROR_WANT_WRITE);

                auto *data = new async_io_data{buf, len, op};
                auto *wait_op = new ssl_io_wait_op(this, executor, data, ssl_operation_type::write);
                common_ssl_socket_proxy{executor}.associate_handle(wait_op, static_cast<std::uintptr_t>(socket_fd_), nullptr);
                
                if (!wait_op->io_handle) {
                    delete wait_op;
                    delete data;
                    op->complete(io::implements::op_result{nullptr, 0, WSAEBADF}, false);
                    return;
                }
                
                auto *iocp_op_ = static_cast<iocp_op *>(wait_op);
                ZeroMemory(&iocp_op_->overlapped, sizeof(OVERLAPPED));
                
                HANDLE iocp = reinterpret_cast<HANDLE>(wait_op->io_handle);
                PostQueuedCompletionStatus(iocp, 0, socket_fd_, &iocp_op_->overlapped);
            } else {
                op->complete(io::implements::op_result{nullptr, 0, openssl_ec(ssl_, ret).value()}, false);
            }
        }

        void async_read_some(void *buf, const std::size_t len, io_context::executor_type executor,
                             completion_op *op) noexcept override {
            if (!op || !ssl_ || !handshaked_) {
                if (op) {
                    op->complete(io::implements::op_result{nullptr, 0, WSAENOTCONN}, false);
                }
                return;
            }

            pending_op_ = ssl_operation_type::read;
            const int ret = SSL_read(ssl_, buf, static_cast<int>(len));

            if (ret > 0) {
                reset_operation();
                op->complete(io::implements::op_result{nullptr, static_cast<std::size_t>(ret), 0}, false);
                return;
            }

            if (ret == 0) {
                handshaked_ = false;
                reset_operation();
                op->complete(io::implements::op_result{nullptr, 0, 0}, false);
                return;
            }

            const int err = SSL_get_error(ssl_, ret);
            if (err == SSL_ERROR_WANT_READ || err == SSL_ERROR_WANT_WRITE) {
                wants_retry_ = true;
                wants_read_ = (err == SSL_ERROR_WANT_READ);
                wants_write_ = (err == SSL_ERROR_WANT_WRITE);

                auto *data = new async_io_data{buf, len, op};
                auto *wait_op = new ssl_io_wait_op(this, executor, data, ssl_operation_type::read);
                common_ssl_socket_proxy{executor}.associate_handle(wait_op, static_cast<std::uintptr_t>(socket_fd_), nullptr);
                
                if (!wait_op->io_handle) {
                    delete wait_op;
                    delete data;
                    op->complete(io::implements::op_result{nullptr, 0, WSAEBADF}, false);
                    return;
                }
                
                auto *iocp_op_ = static_cast<iocp_op *>(wait_op);
                ZeroMemory(&iocp_op_->overlapped, sizeof(OVERLAPPED));
                
                HANDLE iocp = reinterpret_cast<HANDLE>(wait_op->io_handle);
                PostQueuedCompletionStatus(iocp, 0, socket_fd_, &iocp_op_->overlapped);
                return;
            }

            if (err == SSL_ERROR_SSL) {
                const unsigned long err_code = ERR_peek_error();
                const int reason = ERR_GET_REASON(err_code);

                if (reason == SSL_R_UNEXPECTED_EOF_WHILE_READING) {
                    ERR_clear_error();
                    handshaked_ = false;
                    op->complete(io::implements::op_result{nullptr, 0, 0}, false);
                    return;
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
                    char tmp = 0;
                    int recv_ret = recv(socket_fd_, &tmp, 1, MSG_PEEK);
                    if (recv_ret == 0) {
                        ERR_clear_error();
                        handshaked_ = false;
                        op->complete(io::implements::op_result{nullptr, 0, 0}, false);
                        return;
                    }
                    if (recv_ret == SOCKET_ERROR) {
                        int wsa_err = WSAGetLastError();
                        if (wsa_err == WSAECONNRESET || wsa_err == WSAESHUTDOWN) {
                            ERR_clear_error();
                            handshaked_ = false;
                            op->complete(io::implements::op_result{nullptr, 0, 0}, false);
                            return;
                        }
                    }
                }
            }

            op->complete(io::implements::op_result{nullptr, 0, openssl_ec(ssl_, ret).value()}, false);
        }

        RAINY_NODISCARD bool is_handshaked() const noexcept override {
            return handshaked_;
        }

        RAINY_NODISCARD bool is_open() const noexcept override {
            return ssl_ != nullptr && socket_fd_ != invalid_socket_value;
        }

        RAINY_NODISCARD text::string negotiated_protocol() const noexcept override {
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

        void set_server_name(const char *hostname) noexcept override {
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

        struct async_io_data {
            const void *buf;
            std::size_t len;
            completion_op *user_op;

            async_io_data(const void *b, const std::size_t l, completion_op *op) : buf(b), len(l), user_op(op) {}
            async_io_data(void *b, const std::size_t l, completion_op *op) : buf(b), len(l), user_op(op) {}
        };

        class ssl_wait_op : public iocp_op {
        public:
            ssl_wait_op(ssl_stream_impl *stream, io_context::executor_type exec, completion_op *user, 
                        const ssl_operation_type type) :
                iocp_op(&do_complete), stream_(stream), executor_(utility::move(exec)), user_op_(user), op_type_(type) {
            }

            static void do_complete(completion_op *self, const io::implements::op_result &result, const bool cancelled) noexcept {
                auto *me = static_cast<ssl_wait_op *>(self);
                if (cancelled) {
                    me->user_op_->complete(io::implements::op_result{nullptr, 0, WSA_E_CANCELLED}, true);
                    delete me;
                    return;
                }
                switch (me->op_type_) {
                    case ssl_operation_type::handshake:
                        me->stream_->async_handshake(me->executor_, me->user_op_);
                        break;
                    case ssl_operation_type::shutdown: { // 在Windows平台，必须这么处理
                        if (result.error_code != 0) {
                            me->stream_->handshaked_ = false;
                            me->user_op_->complete({}, false);
                        } else {
                            SSL *ssl = static_cast<SSL *>(me->stream_->native_handle());
                            const int ret = SSL_shutdown(ssl);
                            me->stream_->handshaked_ = false;
                            if (ret >= 0) {
                                // ret==1 完成，ret==0 对端还没回，但我们不再等了
                                ERR_clear_error();
                                me->user_op_->complete({}, false);
                            } else {
                                const int err = SSL_get_error(ssl, ret);
                                if (err == SSL_ERROR_SYSCALL || err == SSL_ERROR_SSL) {
                                    // 对端已关闭连接，视为正常完成
                                    ERR_clear_error();
                                    me->user_op_->complete({}, false);
                                } else {
                                    // 真正的错误
                                    auto ec = openssl_ec(ssl, ret);
                                    me->user_op_->complete(io::implements::op_result{nullptr, 0, ec.value()}, false);
                                }
                            }
                        }
                        break;
                    }
                    default:
                        me->user_op_->complete(io::implements::op_result{nullptr, 0, WSAEINVAL}, false);
                        break;
                }
                delete me;
            }

        private:
            ssl_stream_impl *stream_;
            io_context::executor_type executor_;
            completion_op *user_op_;
            ssl_operation_type op_type_;
        };

        class ssl_io_wait_op : public iocp_op {
        public:
            ssl_io_wait_op(ssl_stream_impl *stream, io_context::executor_type exec, async_io_data *data,
                           const ssl_operation_type type) :
                iocp_op(&do_complete), stream_(stream), executor_(utility::move(exec)), data_(data), op_type_(type) {
            }

            static void do_complete(completion_op *self, const io::implements::op_result &, const bool cancelled) noexcept {
                auto *me = static_cast<ssl_io_wait_op *>(self);
                if (cancelled) {
                    me->user_op()->complete(io::implements::op_result{nullptr, 0, WSA_E_CANCELLED}, true);
                    delete me->data_;
                    delete me;
                    return;
                }
                
                if (me->op_type_ == ssl_operation_type::write) {
                    me->stream_->async_write_some(me->data_->buf, me->data_->len, me->executor_, me->user_op());
                } else if (me->op_type_ == ssl_operation_type::read) {
                    me->stream_->async_read_some(const_cast<void *>(me->data_->buf), me->data_->len, me->executor_, me->user_op());
                }
                delete me->data_;
                delete me;
            }

        private:
            RAINY_NODISCARD completion_op *user_op() const {
                return data_->user_op;
            }

            ssl_stream_impl *stream_;
            io_context::executor_type executor_;
            async_io_data *data_;
            ssl_operation_type op_type_;
        };

        std::error_code apply_context_internal(ssl_context_impl *ctx) noexcept {
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
                for (const auto &p : protos) {
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

        RAINY_NODISCARD text::string get_cert_field(const bool subject) const noexcept {
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
                        "Option 1: Install OpenSSL via vcpkg (Windows):\n"
                        "  - Install vcpkg: git clone https://github.com/Microsoft/vcpkg.git\n"
                        "  - Bootstrap vcpkg: .\\vcpkg\\bootstrap-vcpkg.bat\n"
                        "  - Install OpenSSL: .\\vcpkg\\vcpkg install openssl-windows\n"
                        "  - Integrate with your build system: .\\vcpkg\\vcpkg integrate install\n\n"
                        "Option 2: Download OpenSSL binaries from:\n"
                        "  - https://slproweb.com/products/Win32OpenSSL.html\n"
                        "  - https://github.com/openssl/openssl\n\n"
                        "Option 3: Build OpenSSL from source:\n"
                        "  - Download from https://www.openssl.org/source/\n"
                        "  - Follow Windows build instructions in INSTALL file\n\n"
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
