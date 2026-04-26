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

#define SECURITY_WIN32
#define SCHANNEL_USE_BLACKLISTS
#include <rainy/foundation/io/net/implements/ssl_impl.hpp>
#include <rainy/foundation/io/implements/io_context.hpp>

// clang-format off
#include <winsock2.h>
#include <windows.h>
#include <winternl.h>
#include <schannel.h>
#include <security.h>
// clang-format on

#include <iostream>
#include <memory>
#include <string>
#include <system_error>
#include <vector>

namespace rainy::foundation::io::net::implements {
    using io::implements::completion_op;
    using io::implements::io_context_impl_base;
    using io::implements::op_result;
    
    static std::error_code schannel_ec(SECURITY_STATUS ss) noexcept {
        if (ss == SEC_E_OK) {
            return {};
        }
        switch (ss) {
            case SEC_E_INSUFFICIENT_MEMORY:
                return std::make_error_code(std::errc::not_enough_memory);
            case SEC_E_INVALID_HANDLE:
            case SEC_E_INVALID_TOKEN:
                return std::make_error_code(std::errc::invalid_argument);
            case SEC_E_UNSUPPORTED_FUNCTION:
                return std::make_error_code(std::errc::not_supported);
            case SEC_E_TARGET_UNKNOWN:
            case SEC_E_WRONG_PRINCIPAL:
                return std::make_error_code(std::errc::no_such_file_or_directory);
            case SEC_E_INTERNAL_ERROR:
                return std::make_error_code(std::errc::io_error);
            case SEC_E_NO_CREDENTIALS:
            case SEC_E_UNKNOWN_CREDENTIALS:
                return std::make_error_code(std::errc::permission_denied);
            case SEC_I_CONTINUE_NEEDED:
            case SEC_I_INCOMPLETE_CREDENTIALS:
                return std::make_error_code(std::errc::resource_unavailable_try_again);
            case SEC_E_CERT_EXPIRED:
                return std::make_error_code(std::errc::timed_out);
            case SEC_I_COMPLETE_NEEDED:
                return std::make_error_code(std::errc::connection_reset);
            default:
                return {static_cast<int>(ss), std::system_category()};
        }
    }

    static DWORD method_to_protocol_flags(ssl::method mtd) noexcept {
        switch (mtd) {
            case ssl::method::tls_client:
            case ssl::method::tls_server:
                return SP_PROT_TLS1_2 | SP_PROT_TLS1_3;
            case ssl::method::tlsv12_client:
            case ssl::method::tlsv12_server:
                return SP_PROT_TLS1_2;
            case ssl::method::tlsv13_client:
            case ssl::method::tlsv13_server:
                return SP_PROT_TLS1_3;
            default:
                return SP_PROT_TLS1_2 | SP_PROT_TLS1_3;
        }
    }

    static bool is_server_method(ssl::method mtd) noexcept {
        return mtd == ssl::method::tls_server || mtd == ssl::method::tlsv12_server || mtd == ssl::method::tlsv13_server;
    }

    class ssl_context_impl final : public ssl_context_impl_base {
    public:
        ssl_context_impl() = default;

        ~ssl_context_impl() override {
            cleanup_cred();
            if (cert_store_) {
                CertCloseStore(cert_store_, 0);
            }
            if (cert_ctx_) {
                CertFreeCertificateContext(cert_ctx_);
            }
        }

        std::error_code init(ssl::method mtd) noexcept override {
            method_ = mtd;
            is_server_ = is_server_method(mtd);
            protocol_flags_ = method_to_protocol_flags(mtd);
            return rebuild_cred();
        }

        std::error_code set_options(uint64_t options) noexcept override {
            options_ = options;
            return {};
        }

        std::error_code set_verify_mode(ssl::verify_mode mode) noexcept override {
            verify_mode_ = mode;
            return {};
        }

        std::error_code set_verify_depth(int depth) noexcept override {
            verify_depth_ = depth;
            return {};
        }

        std::error_code set_cipher_list(const char *ciphers) noexcept override {
            if (!ciphers) {
                return std::make_error_code(std::errc::invalid_argument);
            }
            cipher_list_ = ciphers;
            return {};
        }

        std::error_code set_alpn_protos(const collections::vector<text::string> &protos) noexcept override {
            alpn_protos_ = protos;
            return {};
        }

        std::error_code use_certificate_file(const char *path, const char *password) noexcept override {
            if (!path) {
                return std::make_error_code(std::errc::invalid_argument);
            }
            cert_file_ = path;
            key_password_ = password ? password : "";
            if (!key_file_.empty()) {
                return load_cert_and_key();
            }
            return {};
        }

        std::error_code use_certificate_chain_file(const char *path) noexcept override {
            return use_certificate_file(path, nullptr);
        }

        std::error_code use_private_key_file(const char *path, const char *password) noexcept override {
            if (!path) {
                return std::make_error_code(std::errc::invalid_argument);
            }
            key_file_ = path;
            key_password_ = password ? password : "";
            if (!cert_file_.empty()) {
                return load_cert_and_key();
            }
            return {};
        }

        std::error_code use_certificate_buffer(const void *data, std::size_t size) noexcept override {
            if (!data || size == 0) {
                return std::make_error_code(std::errc::invalid_argument);
            }
            ensure_ca_store();
            DWORD der_size = 0;
            std::vector<BYTE> der_buf;
            // 先试 PEM base64
            if (CryptStringToBinaryA(static_cast<LPCSTR>(data), static_cast<DWORD>(size), CRYPT_STRING_BASE64HEADER, nullptr,
                                     &der_size, nullptr, nullptr)) {
                der_buf.resize(der_size);
                CryptStringToBinaryA(static_cast<LPCSTR>(data), static_cast<DWORD>(size), CRYPT_STRING_BASE64HEADER, der_buf.data(),
                                     &der_size, nullptr, nullptr);
            } else {
                // 已经是 DER
                der_buf.assign(static_cast<const BYTE *>(data), static_cast<const BYTE *>(data) + size);
                der_size = static_cast<DWORD>(size);
            }
            PCCERT_CONTEXT ctx = CertCreateCertificateContext(X509_ASN_ENCODING | PKCS_7_ASN_ENCODING, der_buf.data(), der_size);
            if (!ctx) {
                return {static_cast<int>(GetLastError()), std::system_category()};
            }
            if (!CertAddCertificateContextToStore(ca_store_, ctx, CERT_STORE_ADD_REPLACE_EXISTING, nullptr)) {
                CertFreeCertificateContext(ctx);
                return {static_cast<int>(GetLastError()), std::system_category()};
            }
            CertFreeCertificateContext(ctx);
            return {};
        }

        std::error_code use_system_store() noexcept override {
            use_system_store_ = true;
            return {};
        }

        std::error_code use_windows_store(const char *store_name) noexcept override {
            const char *name = (store_name && store_name[0]) ? store_name : "ROOT";
            HCERTSTORE h = CertOpenSystemStoreA(0, name);
            if (!h) {
                return {static_cast<int>(GetLastError()), std::system_category()};
            }
            if (cert_store_) {
                CertCloseStore(cert_store_, 0);
            }
            cert_store_ = h;
            use_system_store_ = true;
            return {};
        }

        std::error_code use_linux_store(const char * /*path*/) noexcept override {
            return use_system_store();
        }

        std::error_code add_verify_path(const char *path) noexcept override {
            if (!path) {
                return std::make_error_code(std::errc::invalid_argument);
            }
            return load_pem_file_to_ca_store(path);
        }

        std::error_code set_session_cache_size(std::size_t size) noexcept override {
            session_cache_size_ = size;
            return {};
        }

        void *native_handle() noexcept override {
            return &cred_handle_;
        }

        void *native_credentials() noexcept override {
            return &cred_handle_;
        }
       
        ssl::verify_mode verify_mode() const noexcept override {
            return verify_mode_;
        }
       
        int verify_depth() noexcept override {
            return verify_depth_;
        }
        
        const collections::vector<text::string> &alpn_protos() const noexcept override {
            return alpn_protos_;
        }
        
        bool is_server() const noexcept override {
            return is_server_;
        }

        CredHandle &cred_handle() noexcept {
            return cred_handle_;
        }

        bool cred_valid() const noexcept {
            return cred_valid_;
        }

        DWORD protocol_flags() const noexcept {
            return protocol_flags_;
        }

        PCCERT_CONTEXT client_cert() const noexcept {
            return cert_ctx_;
        }

        HCERTSTORE ca_store() const noexcept {
            return ca_store_;
        }

    private:
        void cleanup_cred() noexcept {
            if (cred_valid_) {
                FreeCredentialsHandle(&cred_handle_);
                cred_valid_ = false;
            }
        }

        void ensure_ca_store() noexcept {
            if (!ca_store_) {
                ca_store_ = CertOpenStore(CERT_STORE_PROV_MEMORY, 0, 0, CERT_STORE_CREATE_NEW_FLAG, nullptr);
            }
        }

        std::error_code rebuild_cred() noexcept {
            cleanup_cred();
            TLS_PARAMETERS tls_params{};
            tls_params.grbitDisabledProtocols = ~protocol_flags_; // 禁用其余协议

            SCH_CREDENTIALS sch_cred{};
            sch_cred.dwVersion = SCH_CREDENTIALS_VERSION;
            sch_cred.dwFlags = SCH_USE_STRONG_CRYPTO;
            sch_cred.pTlsParameters = &tls_params;
            sch_cred.cTlsParameters = 1;

            if (cert_ctx_) {
                sch_cred.cCreds = 1;
                sch_cred.paCred = const_cast<PCCERT_CONTEXT *>(&cert_ctx_);
            }

            if (!is_server_) {
                sch_cred.dwFlags |= SCH_CRED_AUTO_CRED_VALIDATION;
                if (verify_mode_ == ssl::verify_mode::none) {
                    sch_cred.dwFlags |= SCH_CRED_NO_SERVERNAME_CHECK | SCH_CRED_MANUAL_CRED_VALIDATION;
                    sch_cred.dwFlags &= ~SCH_CRED_AUTO_CRED_VALIDATION;
                }
            }

            TimeStamp ts{};

            SECURITY_STATUS ss = AcquireCredentialsHandleA(nullptr, const_cast<SEC_CHAR *>(UNISP_NAME_A),
                                                           is_server_ ? SECPKG_CRED_INBOUND : SECPKG_CRED_OUTBOUND, nullptr, &sch_cred,
                                                           nullptr, nullptr, &cred_handle_, &ts);

            if (ss != SEC_E_OK) {
                return schannel_ec(ss);
            }
            cred_valid_ = true;
            return {};
        }

        std::error_code load_cert_and_key() noexcept {
            {
                auto data = read_file(cert_file_.c_str());
                if (data.empty()) {
                    return std::make_error_code(std::errc::no_such_file_or_directory);
                }

                std::wstring wpwd = to_wstring(key_password_);
                CRYPT_DATA_BLOB blob{static_cast<DWORD>(data.size()), data.data()};
                HCERTSTORE pfx_store =
                    PFXImportCertStore(&blob, key_password_.empty() ? nullptr : wpwd.c_str(), CRYPT_MACHINE_KEYSET | CRYPT_EXPORTABLE);

                if (pfx_store) {
                    // 找第一个有私钥的证书
                    PCCERT_CONTEXT ctx = nullptr;
                    while ((ctx = CertEnumCertificatesInStore(pfx_store, ctx)) != nullptr) {
                        DWORD kspec = 0;
                        HCRYPTPROV_OR_NCRYPT_KEY_HANDLE hkey = 0;
                        BOOL caller_free = FALSE;
                        if (CryptAcquireCertificatePrivateKey(ctx, CRYPT_ACQUIRE_SILENT_FLAG, nullptr, &hkey, &kspec, &caller_free)) {
                            if (caller_free) {
                                if (kspec == CERT_NCRYPT_KEY_SPEC) {
                                    NCryptFreeObject(hkey);
                                }
                                else {
                                    CryptReleaseContext(hkey, 0);
                                }
                            }
                            if (cert_ctx_) {
                                CertFreeCertificateContext(cert_ctx_);
                            }
                            cert_ctx_ = CertDuplicateCertificateContext(ctx);
                            break;
                        }
                    }
                    CertCloseStore(pfx_store, 0);

                    if (cert_ctx_) {
                        return rebuild_cred();
                    }
                }
            }
            {
                auto cert_data = read_file(cert_file_.c_str());
                if (cert_data.empty()) {
                    return std::make_error_code(std::errc::no_such_file_or_directory);
                }

                DWORD der_size = 0;
                std::vector<BYTE> der;
                if (CryptStringToBinaryA(reinterpret_cast<LPCSTR>(cert_data.data()), static_cast<DWORD>(cert_data.size()),
                                         CRYPT_STRING_BASE64HEADER, nullptr, &der_size, nullptr, nullptr)) {
                    der.resize(der_size);
                    CryptStringToBinaryA(reinterpret_cast<LPCSTR>(cert_data.data()), static_cast<DWORD>(cert_data.size()),
                                         CRYPT_STRING_BASE64HEADER, der.data(), &der_size, nullptr, nullptr);
                } else {
                    der = cert_data;
                    der_size = static_cast<DWORD>(der.size());
                }

                PCCERT_CONTEXT ctx = CertCreateCertificateContext(X509_ASN_ENCODING | PKCS_7_ASN_ENCODING, der.data(), der_size);
                
                if (!ctx) {
                    return {static_cast<int>(GetLastError()), std::system_category()};
                }

                if (cert_ctx_) {
                    CertFreeCertificateContext(cert_ctx_);
                }
                cert_ctx_ = ctx;
            }
            return rebuild_cred();
        }

        std::error_code load_pem_file_to_ca_store(const char *path) noexcept {
            ensure_ca_store();
            auto data = read_file(path);
            if (data.empty()) {
                return std::make_error_code(std::errc::no_such_file_or_directory);
            }

            DWORD der_size = 0;
            std::vector<BYTE> der;

            if (CryptStringToBinaryA(reinterpret_cast<LPCSTR>(data.data()), static_cast<DWORD>(data.size()), CRYPT_STRING_BASE64HEADER,
                                     nullptr, &der_size, nullptr, nullptr)) {
                der.resize(der_size);
                CryptStringToBinaryA(reinterpret_cast<LPCSTR>(data.data()), static_cast<DWORD>(data.size()), CRYPT_STRING_BASE64HEADER,
                                     der.data(), &der_size, nullptr, nullptr);
            } else {
                der = data;
                der_size = static_cast<DWORD>(data.size());
            }

            PCCERT_CONTEXT ctx = CertCreateCertificateContext(X509_ASN_ENCODING | PKCS_7_ASN_ENCODING, der.data(), der_size);
            if (!ctx) {
                return {static_cast<int>(GetLastError()), std::system_category()};
            }

            CertAddCertificateContextToStore(ca_store_, ctx, CERT_STORE_ADD_REPLACE_EXISTING, nullptr);
            CertFreeCertificateContext(ctx);
            return {};
        }

        static std::vector<BYTE> read_file(const char *path) noexcept {
            FILE *f = fopen(path, "rb");
            if (!f) {
                return {};
            }
            fseek(f, 0, SEEK_END);
            const long sz = ftell(f);
            fseek(f, 0, SEEK_SET);
            if (sz <= 0) {
                fclose(f);
                return {};
            }
            std::vector<BYTE> buf(static_cast<std::size_t>(sz));
            fread(buf.data(), 1, buf.size(), f);
            fclose(f);
            return buf;
        }

        static std::wstring to_wstring(const text::string &s) noexcept {
            if (s.empty()) {
                return {};
            }
            int n = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), static_cast<int>(s.size()), nullptr, 0);
            std::wstring ws(n, L'\0');
            MultiByteToWideChar(CP_UTF8, 0, s.c_str(), static_cast<int>(s.size()), ws.data(), n);
            return ws;
        }

        CredHandle cred_handle_{};
        bool cred_valid_ = false;

        ssl::method method_ = ssl::method::tls_client;
        ssl::verify_mode verify_mode_ = ssl::verify_mode::none;
        int verify_depth_ = 9;
        uint64_t options_ = 0;
        std::size_t session_cache_size_ = 0;
        DWORD protocol_flags_ = SP_PROT_TLS1_2 | SP_PROT_TLS1_3;
        bool is_server_ = false;
        bool use_system_store_ = false;

        text::string cert_file_;
        text::string key_file_;
        text::string key_password_;
        text::string cipher_list_;

        PCCERT_CONTEXT cert_ctx_ = nullptr; // 本端证书
        HCERTSTORE cert_store_ = nullptr; // Windows store（use_windows_store）
        HCERTSTORE ca_store_ = nullptr; // 自定义 CA 信任 store

        collections::vector<text::string> alpn_protos_;
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

        struct ssl_iocp_op : iocp_op {
            enum class phase : uint8_t {
                send,
                send_final,
                recv
            } current_phase;
            completion_op *outer{nullptr};
            io_context_impl_base *ctx{nullptr};
            ssl_stream_impl *ssl{nullptr};

            explicit ssl_iocp_op(phase p, ssl_stream_impl *s, io_context_impl_base *c, completion_op *op) noexcept :
                iocp_op(&do_complete), current_phase(p), outer(op), ctx(c), ssl(s) {
            }

            static void do_complete(completion_op *self, const op_result &result, bool cancelled) {
                auto *iop = static_cast<ssl_iocp_op *>(self);

                ssl_stream_impl *ssl = iop->ssl;
                io_context_impl_base *ctx = iop->ctx;
                completion_op *outer = iop->outer;
                phase p = iop->current_phase;

                ctx->on_work_finished();

                if (cancelled || result.error_code) {
                    delete iop;
                    outer->complete(op_result{nullptr, 0, result.error_code}, cancelled);
                    return;
                }

                if (p == phase::recv) {
                    ssl->recv_used_ += result.bytes_transferred;
                    ssl->recv_used_total_ = ssl->recv_used_;
                    delete iop;
                    ssl->advance_handshake(*ctx, outer);
                } else if (p == phase::send_final) {
                    delete iop;
                    ssl->handshaked_ = true;
                    QueryContextAttributesA(&ssl->ctx_handle_, SECPKG_ATTR_STREAM_SIZES, &ssl->stream_sizes_);
                    outer->complete(op_result{nullptr, 0, 0}, false);
                } else {
                    delete iop;
                    ssl->async_raw_recv(*ctx, outer);
                }
            }
        };

        friend struct ssl_stream_impl::ssl_iocp_op;

        ssl_stream_impl() = default;

        ~ssl_stream_impl() override {
            destroy_session();
        }

        std::error_code attach(native_socket_t sock, bool is_server) noexcept override {
            static constexpr std::size_t recv_buf_size = 32 * 1024; // 32 K
            if (sock == invalid_socket_value) {
                return std::make_error_code(std::errc::bad_file_descriptor);
            }
            destroy_session();
            sock_ = sock;
            is_server_ = is_server;
            recv_buf_.resize(recv_buf_size);
            return {};
        }

        std::error_code apply_context(ssl_context_impl *ctx) noexcept {
            ctx_ = ctx;
            return {};
        }

        std::error_code apply_context(const ssl_context_params &params) noexcept override {
            p_cred_ = static_cast<CredHandle *>(params.native_credentials);
            ext_params_ = params;
            use_ext_params_ = true;
            return {};
        }

        native_socket_t underlying_socket() const noexcept override {
            return sock_;
        }

        std::error_code handshake() noexcept override {
            if (!ensure_cred()) {
                return std::make_error_code(std::errc::not_connected);
            }

            SECURITY_STATUS ss;
            if (is_server_) {
                ss = server_handshake_loop();
            } else {
                ss = client_handshake_loop();
            }

            if (ss == SEC_E_OK) {
                handshaked_ = true;
                reset_operation();
                QueryContextAttributesA(&ctx_handle_, SECPKG_ATTR_STREAM_SIZES, &stream_sizes_);
            } else {
                pending_op_ = ssl_operation_type::handshake;
                wants_retry_ = (ss == SEC_I_CONTINUE_NEEDED);
                wants_read_ = true;
                wants_write_ = false;
            }
            return schannel_ec(ss == SEC_I_CONTINUE_NEEDED ? SEC_E_OK : ss);
        }

        void async_handshake(io_context_impl_base &ctx, completion_op *op) noexcept override {
            if (!ensure_cred()) {
                op->complete(op_result{nullptr, 0, std::make_error_code(std::errc::not_connected).value()}, false);
                return;
            }

            SecInvalidateHandle(&ctx_handle_);
            session_valid_ = true;

            SecBuffer out_buf{};
            out_buf.BufferType = SECBUFFER_TOKEN;
            SecBufferDesc out_desc{SECBUFFER_VERSION, 1, &out_buf};
            ULONG ctx_flags =
                ISC_REQ_SEQUENCE_DETECT | ISC_REQ_REPLAY_DETECT | ISC_REQ_CONFIDENTIALITY | ISC_REQ_STREAM | ISC_REQ_ALLOCATE_MEMORY;
            ULONG out_flags = 0;
            const char *sni = server_name_.empty() ? nullptr : server_name_.c_str();
            SECURITY_STATUS ss =
                InitializeSecurityContextA(p_cred_, nullptr, const_cast<SEC_CHAR *>(sni), ctx_flags, 0, SECURITY_NATIVE_DREP, nullptr,
                                           0, &ctx_handle_, &out_desc, &out_flags, nullptr);
            if (ss != SEC_I_CONTINUE_NEEDED && ss != SEC_E_OK) {
                op->complete(op_result{nullptr, 0, static_cast<int>(ss)}, false);
                return;
            }
            recv_used_ = 0;
            recv_used_total_ = 0;
            if (out_buf.cbBuffer && out_buf.pvBuffer) {
                async_raw_send(out_buf.pvBuffer, out_buf.cbBuffer, ctx, op);
                FreeContextBuffer(out_buf.pvBuffer);
            } else {
                async_raw_recv(ctx, op);
            }
        }

        void async_raw_send(const void *buf, DWORD len, io_context_impl_base &ctx, completion_op *op) noexcept {
            send_buf_.assign(static_cast<const BYTE *>(buf), static_cast<const BYTE *>(buf) + len);
            auto *iop = new ssl_iocp_op(ssl_iocp_op::phase::send, this, &ctx, op);
            ::ZeroMemory(&iop->overlapped, sizeof(OVERLAPPED));
            WSABUF wb{len, reinterpret_cast<char *>(send_buf_.data())};
            DWORD sent = 0;
            int ret = ::WSASend(static_cast<SOCKET>(sock_), &wb, 1, &sent, 0, &iop->overlapped, nullptr);
            if (ret == SOCKET_ERROR && ::WSAGetLastError() != WSA_IO_PENDING) {
                int err = ::WSAGetLastError();
                delete iop;
                op->complete(op_result{nullptr, 0, err}, false);
                return;
            }
            ctx.on_work_started();
        }

        void async_raw_recv(io_context_impl_base &ctx, completion_op *op) noexcept {
            if (recv_used_ >= recv_buf_.size()) {
                recv_buf_.resize(recv_buf_.size() * 2);
            }
            auto *iop = new ssl_iocp_op(ssl_iocp_op::phase::recv, this, &ctx, op);
            ::ZeroMemory(&iop->overlapped, sizeof(OVERLAPPED));
            WSABUF wb{static_cast<ULONG>(recv_buf_.size() - recv_used_), reinterpret_cast<char *>(recv_buf_.data() + recv_used_)};
            DWORD recvd = 0;
            DWORD flags = 0;
            int ret = ::WSARecv(static_cast<SOCKET>(sock_), &wb, 1, &recvd, &flags, &iop->overlapped, nullptr);
            if (ret == SOCKET_ERROR && ::WSAGetLastError() != WSA_IO_PENDING) {
                int err = ::WSAGetLastError();
                delete iop;
                op->complete(op_result{nullptr, 0, err}, false);
                return;
            }
            ctx.on_work_started();
        }

        void advance_handshake(io_context_impl_base &ctx, completion_op *op) noexcept {
            SecBuffer in_bufs[2]{};

            in_bufs[0].BufferType = SECBUFFER_TOKEN;
            in_bufs[0].cbBuffer = static_cast<DWORD>(recv_used_);
            in_bufs[0].pvBuffer = recv_buf_.data();

            in_bufs[1].BufferType = SECBUFFER_EMPTY;

            SecBufferDesc in_desc{SECBUFFER_VERSION, 2, in_bufs};
            SecBuffer out_buf{};
            out_buf.BufferType = SECBUFFER_TOKEN;
            SecBufferDesc out_desc{SECBUFFER_VERSION, 1, &out_buf};
            ULONG ctx_flags = ISC_REQ_SEQUENCE_DETECT | ISC_REQ_REPLAY_DETECT | ISC_REQ_CONFIDENTIALITY | ISC_REQ_STREAM |
                              ISC_REQ_ALLOCATE_MEMORY;
            ULONG out_flags = 0;

            SECURITY_STATUS ss = InitializeSecurityContextA(p_cred_, &ctx_handle_, nullptr, ctx_flags, 0, SECURITY_NATIVE_DREP,
                                                            &in_desc, 0, nullptr, &out_desc, &out_flags, nullptr);
            DWORD extra = 0;
            for (int i = 0; i < 2; ++i) {
                if (in_bufs[i].BufferType == SECBUFFER_EXTRA) {
                    extra = in_bufs[i].cbBuffer;
                    break;
                }
            }
            if (extra > 0) {
                std::memmove(recv_buf_.data(), recv_buf_.data() + (recv_used_total_ - extra), extra);

                recv_used_ = extra;
            } else {
                recv_used_ = 0;
            }
            if (ss == SEC_E_OK) {
                if (out_buf.cbBuffer && out_buf.pvBuffer) {
                    async_raw_send_final(out_buf.pvBuffer, out_buf.cbBuffer, ctx, op);
                    FreeContextBuffer(out_buf.pvBuffer);
                } else {
                    handshaked_ = true;
                    QueryContextAttributesA(&ctx_handle_, SECPKG_ATTR_STREAM_SIZES, &stream_sizes_);
                    op->complete(op_result{nullptr, 0, 0}, false);
                }
                return;
            }

            if (ss == SEC_I_CONTINUE_NEEDED || ss == SEC_E_INCOMPLETE_MESSAGE) {
                if (out_buf.cbBuffer && out_buf.pvBuffer) {
                    async_raw_send(out_buf.pvBuffer, out_buf.cbBuffer, ctx, op);
                    FreeContextBuffer(out_buf.pvBuffer);
                } else {
                    async_raw_recv(ctx, op);
                }
                return;
            }

            if (out_buf.pvBuffer) {
                FreeContextBuffer(out_buf.pvBuffer);
            }

            op->complete(op_result{nullptr, 0, static_cast<int>(ss)}, false);
        }

        std::error_code shutdown() noexcept override {
            if (!session_valid_) {
                return {};
            }
            send_shutdown_alert();
            destroy_session();
            return {};
        }

        void async_shutdown(io_context_impl_base &, completion_op *op) noexcept override {
            if (!op) {
                return;
            }
            auto ec = shutdown();
            op->complete(op_result{nullptr, 0, ec.value()}, false);
        }

        std::ptrdiff_t write_some(const void *buf, std::size_t len, std::error_code &ec) noexcept override {
            if (!session_valid_ || !handshaked_) {
                ec = std::make_error_code(std::errc::not_connected);
                return -1;
            }
            pending_op_ = ssl_operation_type::write;
            const std::size_t max_msg = stream_sizes_.cbMaximumMessage;
            const std::size_t send_len = (len > max_msg) ? max_msg : len;
            const std::size_t total = stream_sizes_.cbHeader + send_len + stream_sizes_.cbTrailer;
            std::vector<BYTE> out(total);
            SecBuffer bufs[4]{};
            bufs[0].BufferType = SECBUFFER_STREAM_HEADER;
            bufs[0].cbBuffer = stream_sizes_.cbHeader;
            bufs[0].pvBuffer = out.data();

            bufs[1].BufferType = SECBUFFER_DATA;
            bufs[1].cbBuffer = static_cast<DWORD>(send_len);
            bufs[1].pvBuffer = out.data() + stream_sizes_.cbHeader;
            std::memcpy(bufs[1].pvBuffer, buf, send_len);

            bufs[2].BufferType = SECBUFFER_STREAM_TRAILER;
            bufs[2].cbBuffer = stream_sizes_.cbTrailer;
            bufs[2].pvBuffer = out.data() + stream_sizes_.cbHeader + send_len;

            bufs[3].BufferType = SECBUFFER_EMPTY;
            bufs[3].cbBuffer = 0;
            bufs[3].pvBuffer = nullptr;

            SecBufferDesc desc{SECBUFFER_VERSION, 4, bufs};
            SECURITY_STATUS ss = EncryptMessage(&ctx_handle_, 0, &desc, 0);
            if (ss != SEC_E_OK) {
                ec = schannel_ec(ss);
                return -1;
            }

            // 发送加密后的全部字节
            const DWORD enc_size = bufs[0].cbBuffer + bufs[1].cbBuffer + bufs[2].cbBuffer;
            if (!raw_send(out.data(), enc_size, ec)) {
                return -1;
            }

            reset_operation();
            return static_cast<std::ptrdiff_t>(send_len);
        }

        std::ptrdiff_t read_some(void *buf, std::size_t len, std::error_code &ec) noexcept override {
            if (!session_valid_ || !handshaked_) {
                ec = std::make_error_code(std::errc::not_connected);
                return -1;
            }
            pending_op_ = ssl_operation_type::read;

            // 先从 plain_buf_ 提取已解密数据
            if (!plain_buf_.empty()) {
                return drain_plain(buf, len, ec);
            }

            rain_loop {
                // 尝试解密已接收的密文
                if (recv_used_ > 0) {
                    auto [ss, extra_offset, extra_len, plain_data, plain_len] = decrypt_pending();

                    if (ss == SEC_E_OK) {
                        if (plain_len > 0) {
                            plain_buf_.insert(plain_buf_.end(), plain_data, plain_data + plain_len);
                        }
                        // 将 extra 数据移到 recv_buf_ 头部继续处理
                        if (extra_len > 0) {
                            std::memmove(recv_buf_.data(), recv_buf_.data() + extra_offset, extra_len);
                        }
                        recv_used_ = extra_len;

                        if (!plain_buf_.empty()) {
                            return drain_plain(buf, len, ec);
                        }
                        ec.clear();
                        handshaked_ = false;
                        return 0;
                    }
                    if (ss == SEC_E_INCOMPLETE_MESSAGE) {
                        // 数据不够，继续读
                    } else if (ss == SEC_I_CONTEXT_EXPIRED) {
                        handshaked_ = false;
                        ec.clear();
                        return 0;
                    } else {
                        ec = schannel_ec(ss);
                        return -1;
                    }
                }

                // 从 socket 读更多数据
                if (!raw_recv(ec)) {
                    return -1;
                }
            }
        }

        void async_write_some(const void *buf, std::size_t len, io_context_impl_base &, completion_op *op) noexcept override {
            if (!op) {
                return;
            }
            std::error_code ec;
            const std::ptrdiff_t n = write_some(buf, len, ec);
            if (!ec || !wants_retry_) {
                op->complete(op_result{nullptr, static_cast<std::size_t>(n > 0 ? n : 0), ec ? ec.value() : 0}, false);
            }
        }

        void async_read_some(void *buf, std::size_t len, io_context_impl_base &, completion_op *op) noexcept override {
            if (!op) {
                return;
            }
            std::error_code ec;
            const std::ptrdiff_t n = read_some(buf, len, ec);
            if (!ec || !wants_retry_) {
                op->complete(op_result{nullptr, static_cast<std::size_t>(n > 0 ? n : 0), ec ? ec.value() : 0}, false);
            }
        }

        bool is_handshaked() const noexcept override {
            return handshaked_;
        }

        bool is_open() const noexcept override {
            return sock_ != invalid_socket_value;
        }

        text::string negotiated_protocol() const noexcept override {
            if (!session_valid_) {
                return {};
            }
            SecPkgContext_ApplicationProtocol alpn{};
            if (QueryContextAttributesA(const_cast<PCtxtHandle>(&ctx_handle_), SECPKG_ATTR_APPLICATION_PROTOCOL, &alpn) == SEC_E_OK) {
                if (alpn.ProtoNegoStatus == SecApplicationProtocolNegotiationStatus_Success) {
                    return text::string(reinterpret_cast<const char *>(alpn.ProtocolId), alpn.ProtocolIdSize);
                }
            }
            return {};
        }

        text::string peer_certificate_subject() const noexcept override {
            return get_cert_name(true);
        }
        text::string peer_certificate_issuer() const noexcept override {
            return get_cert_name(false);
        }

        void set_server_name(const char *hostname) noexcept override {
            server_name_ = hostname ? hostname : "";
        }
        const char *server_name() const noexcept override {
            return server_name_.c_str();
        }

        void *native_handle() noexcept override {
            return &ctx_handle_;
        }

        ssl_operation_type pending_operation() const noexcept override {
            return pending_op_;
        }
        bool wants_retry() const noexcept override {
            return wants_retry_;
        }
        bool wants_read() const noexcept override {
            return wants_read_;
        }
        bool wants_write() const noexcept override {
            return wants_write_;
        }

        void reset_operation() noexcept override {
            pending_op_ = ssl_operation_type::read;
            wants_retry_ = false;
            wants_read_ = false;
            wants_write_ = false;
        }

    private:
        bool ensure_cred() noexcept {
            if (use_ext_params_) {
                p_cred_ = static_cast<CredHandle *>(ext_params_.native_credentials);
                return p_cred_ != nullptr;
            }
            if (ctx_) {
                p_cred_ = &ctx_->cred_handle();
                return ctx_->cred_valid();
            }
            return false;
        }

        SECURITY_STATUS client_handshake_loop() noexcept {
            std::vector<BYTE> alpn_buf = build_alpn_extension();
            SecBuffer out_buf{};
            SecBufferDesc out_desc{SECBUFFER_VERSION, 1, &out_buf};
            ULONG ctx_flags = ISC_REQ_SEQUENCE_DETECT | ISC_REQ_REPLAY_DETECT | ISC_REQ_CONFIDENTIALITY | ISC_REQ_STREAM;
            ULONG out_flags = 0;
            SECURITY_STATUS ss = InitializeSecurityContextA(
                p_cred_,
                nullptr, // 首次调用无 ctx
                server_name_.empty() ? nullptr : const_cast<SEC_CHAR *>(server_name_.c_str()), ctx_flags, 0, SECURITY_NATIVE_DREP,
                nullptr, // 首次无输入
                0, &ctx_handle_, &out_desc, &out_flags, nullptr);

            session_valid_ = true;

            if (ss != SEC_I_CONTINUE_NEEDED && ss != SEC_E_OK) {
                return ss;
            }

            // 发送 ClientHello
            if (out_buf.cbBuffer && out_buf.pvBuffer) {
                std::error_code ec;
                raw_send(out_buf.pvBuffer, out_buf.cbBuffer, ec);
                FreeContextBuffer(out_buf.pvBuffer);
                out_buf.pvBuffer = nullptr;
                if (ec) {
                    return SEC_E_INTERNAL_ERROR;
                }
            }

            // 握手循环
            while (ss == SEC_I_CONTINUE_NEEDED) {
                std::error_code ec;
                if (!raw_recv(ec)) {
                    return SEC_E_INTERNAL_ERROR;
                }

                SecBuffer in_bufs[2]{};
                in_bufs[0].BufferType = SECBUFFER_TOKEN;
                in_bufs[0].cbBuffer = static_cast<DWORD>(recv_used_);
                in_bufs[0].pvBuffer = recv_buf_.data();
                in_bufs[1].BufferType = SECBUFFER_EMPTY;

                SecBufferDesc in_desc{SECBUFFER_VERSION, 2, in_bufs};

                out_buf = {};
                out_desc = {SECBUFFER_VERSION, 1, &out_buf};

                ss = InitializeSecurityContextA(p_cred_, &ctx_handle_, nullptr, ctx_flags, 0, SECURITY_NATIVE_DREP, &in_desc, 0,
                                                nullptr, // ctx 已有
                                                &out_desc, &out_flags, nullptr);

                // 处理残留数据（SECBUFFER_EXTRA）
                recv_used_ = 0;
                for (int i = 0; i < 2; ++i) {
                    if (in_bufs[i].BufferType == SECBUFFER_EXTRA && in_bufs[i].cbBuffer > 0) {
                        std::memmove(recv_buf_.data(), recv_buf_.data() + (recv_used_total_ - in_bufs[i].cbBuffer),
                                     in_bufs[i].cbBuffer);
                        recv_used_ = in_bufs[i].cbBuffer;
                    }
                }

                if (out_buf.cbBuffer && out_buf.pvBuffer) {
                    raw_send(out_buf.pvBuffer, out_buf.cbBuffer, ec);
                    FreeContextBuffer(out_buf.pvBuffer);
                    if (ec) {
                        return SEC_E_INTERNAL_ERROR;
                    }
                }
            }
            return ss;
        }

        SECURITY_STATUS server_handshake_loop() noexcept {
            ULONG ctx_flags = ASC_REQ_SEQUENCE_DETECT | ASC_REQ_REPLAY_DETECT | ASC_REQ_CONFIDENTIALITY | ASC_REQ_STREAM;
            ULONG out_flags = 0;

            SECURITY_STATUS ss = SEC_I_CONTINUE_NEEDED;

            while (ss == SEC_I_CONTINUE_NEEDED || ss == SEC_E_INCOMPLETE_MESSAGE) {
                // 读取客户端数据
                std::error_code ec;
                if (!raw_recv(ec)) {
                    return SEC_E_INTERNAL_ERROR;
                }

                SecBuffer in_buf{};
                in_buf.BufferType = SECBUFFER_TOKEN;
                in_buf.cbBuffer = static_cast<DWORD>(recv_used_);
                in_buf.pvBuffer = recv_buf_.data();

                SecBuffer in_extra{SECBUFFER_EMPTY, 0, nullptr};
                SecBuffer in_arr[2] = {in_buf, in_extra};
                SecBufferDesc in_desc{SECBUFFER_VERSION, 2, in_arr};

                SecBuffer out_buf{};
                SecBufferDesc out_desc{SECBUFFER_VERSION, 1, &out_buf};

                ss = AcceptSecurityContext(p_cred_, session_valid_ ? &ctx_handle_ : nullptr, &in_desc, ctx_flags, SECURITY_NATIVE_DREP,
                                           session_valid_ ? nullptr : &ctx_handle_, &out_desc, &out_flags, nullptr);

                session_valid_ = true;

                // 处理 EXTRA
                recv_used_ = 0;
                if (in_arr[1].BufferType == SECBUFFER_EXTRA && in_arr[1].cbBuffer > 0) {
                    std::memmove(recv_buf_.data(), recv_buf_.data() + (recv_used_total_ - in_arr[1].cbBuffer), in_arr[1].cbBuffer);
                    recv_used_ = in_arr[1].cbBuffer;
                }

                if (out_buf.cbBuffer && out_buf.pvBuffer) {
                    std::error_code ec2;
                    raw_send(out_buf.pvBuffer, out_buf.cbBuffer, ec2);
                    FreeContextBuffer(out_buf.pvBuffer);
                    if (ec2) {
                        return SEC_E_INTERNAL_ERROR;
                    }
                }
            }
            return ss;
        }

        void async_raw_send_final(const void *buf, DWORD len, io_context_impl_base &ctx, completion_op *op) noexcept {
            send_buf_.assign(static_cast<const BYTE *>(buf), static_cast<const BYTE *>(buf) + len);

            auto *iop = new ssl_iocp_op(ssl_iocp_op::phase::send_final, this, &ctx, op);
            ::ZeroMemory(&iop->overlapped, sizeof(OVERLAPPED));

            WSABUF wb{len, reinterpret_cast<char *>(send_buf_.data())};
            DWORD sent = 0;
            int ret = ::WSASend(static_cast<SOCKET>(sock_), &wb, 1, &sent, 0, &iop->overlapped, nullptr);
            if (ret == SOCKET_ERROR && ::WSAGetLastError() != WSA_IO_PENDING) {
                int err = ::WSAGetLastError();
                delete iop;
                op->complete(op_result{nullptr, 0, err}, false);
                return;
            }
            ctx.on_work_started();
        }

        void send_shutdown_alert() noexcept {
            if (!session_valid_) {
                return;
            }
            
            DWORD type = SCHANNEL_SHUTDOWN;
            SecBuffer tok{sizeof(type), SECBUFFER_TOKEN, &type};
            SecBufferDesc desc{SECBUFFER_VERSION, 1, &tok};
            ApplyControlToken(&ctx_handle_, &desc);

            SecBuffer out_buf{};
            SecBufferDesc out_desc{SECBUFFER_VERSION, 1, &out_buf};
            ULONG flags = 0;

            if (is_server_) {
                AcceptSecurityContext(p_cred_, &ctx_handle_, nullptr, 0, 0, nullptr, &out_desc, &flags, nullptr);
            } else {
                InitializeSecurityContextA(p_cred_, &ctx_handle_, nullptr, 0, 0, 0, nullptr, 0, nullptr, &out_desc, &flags, nullptr);
            }
            if (out_buf.pvBuffer && out_buf.cbBuffer) {
                std::error_code ec;
                raw_send(out_buf.pvBuffer, out_buf.cbBuffer, ec);
                FreeContextBuffer(out_buf.pvBuffer);
            }
        }

        struct DecryptResult {
            SECURITY_STATUS ss;
            std::size_t extra_offset;
            std::size_t extra_len;
            BYTE *plain_data;
            std::size_t plain_len;
        };

        DecryptResult decrypt_pending() noexcept {
            SecBuffer bufs[4]{};
            bufs[0].BufferType = SECBUFFER_DATA;
            bufs[0].cbBuffer = static_cast<DWORD>(recv_used_);
            bufs[0].pvBuffer = recv_buf_.data();
            for (int i = 1; i < 4; ++i) {
                bufs[i].BufferType = SECBUFFER_EMPTY;
            }
            SecBufferDesc desc{SECBUFFER_VERSION, 4, bufs};
            SECURITY_STATUS ss = DecryptMessage(&ctx_handle_, &desc, 0, nullptr);

            DecryptResult r{ss, 0, 0, nullptr, 0};
            for (int i = 0; i < 4; ++i) {
                if (bufs[i].BufferType == SECBUFFER_DATA) {
                    r.plain_data = static_cast<BYTE *>(bufs[i].pvBuffer);
                    r.plain_len = bufs[i].cbBuffer;
                }
                if (bufs[i].BufferType == SECBUFFER_EXTRA) {
                    // extra 相对于 recv_buf_ 头部的偏移 = recv_used_ - cbBuffer
                    r.extra_offset = recv_used_ - bufs[i].cbBuffer;
                    r.extra_len = bufs[i].cbBuffer;
                }
            }
            return r;
        }

        std::ptrdiff_t drain_plain(void *buf, std::size_t len, std::error_code &ec) noexcept {
            const std::size_t copy = (len < plain_buf_.size()) ? len : plain_buf_.size();
            std::memcpy(buf, plain_buf_.data(), copy);
            plain_buf_.erase(plain_buf_.begin(), plain_buf_.begin() + static_cast<std::ptrdiff_t>(copy));
            ec.clear();
            reset_operation();
            return static_cast<std::ptrdiff_t>(copy);
        }

        bool raw_send(const void *buf, DWORD len, std::error_code &ec) noexcept {
            const char *p = static_cast<const char *>(buf);
            DWORD sent = 0;
            while (sent < len) {
                int n = ::send(sock_, p + sent, static_cast<int>(len - sent), 0);
                if (n == SOCKET_ERROR) {
                    ec = {WSAGetLastError(), std::system_category()};
                    return false;
                }
                sent += static_cast<DWORD>(n);
            }
            return true;
        }

        bool raw_recv(std::error_code &ec) noexcept {
            if (recv_used_ >= recv_buf_.size()) {
                recv_buf_.resize(recv_buf_.size() * 2);
            }
            const int n = ::recv(sock_, reinterpret_cast<char *>(recv_buf_.data() + recv_used_),
                                 static_cast<int>(recv_buf_.size() - recv_used_), 0);
            if (n == SOCKET_ERROR) {
                ec = {WSAGetLastError(), std::system_category()};
                return false;
            }
            if (n == 0) {
                // 对端关闭连接
                ec = std::make_error_code(std::errc::connection_reset);
                return false;
            }
            recv_used_ += static_cast<std::size_t>(n);
            recv_used_total_ = recv_used_;
            return true;
        }

        std::vector<BYTE> build_alpn_extension() noexcept {
            const collections::vector<text::string> *protos = nullptr;
            if (use_ext_params_) {
                protos = &ext_params_.alpn_protos;
            } else if (ctx_) {
                protos = &ctx_->alpn_protos();
            }
            if (!protos || protos->empty()) {
                return {};
            }
            return {};
        }

        text::string get_cert_name(bool subject) const noexcept {
            if (!session_valid_) {
                return {};
            }
            PCCERT_CONTEXT peer_cert = nullptr;
            QueryContextAttributesA(const_cast<PCtxtHandle>(&ctx_handle_), SECPKG_ATTR_REMOTE_CERT_CONTEXT, &peer_cert);
            if (!peer_cert) {
                return {};
            }

            char buf[512]{};
            DWORD flags = subject ? CERT_NAME_SIMPLE_DISPLAY_TYPE : CERT_NAME_SIMPLE_DISPLAY_TYPE;

            if (subject) {
                CertGetNameStringA(peer_cert, CERT_NAME_RDN_TYPE, 0, nullptr, buf, sizeof(buf));
            } else {
                CertGetNameStringA(peer_cert, CERT_NAME_RDN_TYPE, CERT_NAME_ISSUER_FLAG, nullptr, buf, sizeof(buf));
            }
            CertFreeCertificateContext(peer_cert);
            return text::string(buf);
        }

        void destroy_session() noexcept {
            if (session_valid_) {
                DeleteSecurityContext(&ctx_handle_);
                session_valid_ = false;
            }
            handshaked_ = false;
            recv_used_ = 0;
            recv_used_total_ = 0;
            plain_buf_.clear();
            reset_operation();
        }

        native_socket_t sock_ = invalid_socket_value;
        bool is_server_ = false;
        bool session_valid_ = false;
        bool handshaked_ = false;

        CtxtHandle ctx_handle_{};
        CredHandle *p_cred_ = nullptr;

        ssl_context_impl *ctx_ = nullptr;
        bool use_ext_params_ = false;
        ssl_context_params ext_params_{};

        SecPkgContext_StreamSizes stream_sizes_{};

        std::vector<BYTE> recv_buf_;
        std::size_t recv_used_ = 0;
        std::size_t recv_used_total_ = 0;
        std::vector<BYTE> plain_buf_; // 已解密、待消费的明文

        text::string server_name_;
        collections::vector<BYTE> send_buf_;
        ssl_operation_type pending_op_ = ssl_operation_type::read;
        bool wants_retry_ = false;
        bool wants_read_ = false;
        bool wants_write_ = false;
    };

    memory::nebula_ptr<ssl_context_impl_base> create_ssl_context_impl() {
        return memory::make_nebula<ssl_context_impl>();
    }

    memory::nebula_ptr<ssl_stream_impl_base> create_ssl_stream_impl() {
        return memory::make_nebula<ssl_stream_impl>();
    }

}
