#include <cstring>
#include <gnutls/gnutls.h>
#include <gnutls/x509.h>
#include <rainy/foundation/io/net/implements/ssl_context.hpp>
#include <sys/socket.h>
#include <sys/stat.h>
#include <vector>

namespace rainy::foundation::io::net::implements {
    struct gnutls_global_guard {
        gnutls_global_guard() {
            gnutls_global_init();
        }
        ~gnutls_global_guard() {
            gnutls_global_deinit();
        }
    };

    void ensure_gnutls_init() {
        // 局部静态，线程安全（C++11 起保证）
        static gnutls_global_guard guard;
        (void) guard;
    }

    std::error_code gnutls_ec(int ret) noexcept {
        if (ret == GNUTLS_E_SUCCESS) {
            return {};
        }
        switch (ret) {
            case GNUTLS_E_AGAIN:
                return std::make_error_code(std::errc::resource_unavailable_try_again);
            case GNUTLS_E_INTERRUPTED:
                return std::make_error_code(std::errc::interrupted);
            case GNUTLS_E_EXPIRED:
                return std::make_error_code(std::errc::timed_out);
            case GNUTLS_E_PUSH_ERROR:
            case GNUTLS_E_PULL_ERROR:
                return std::make_error_code(std::errc::io_error);
            case GNUTLS_E_PREMATURE_TERMINATION:
                return std::make_error_code(std::errc::connection_reset);
            case GNUTLS_E_INVALID_REQUEST:
            case GNUTLS_E_INVALID_SESSION:
                return std::make_error_code(std::errc::invalid_argument);
            case GNUTLS_E_MEMORY_ERROR:
                return std::make_error_code(std::errc::not_enough_memory);
            case GNUTLS_E_CERTIFICATE_VERIFICATION_ERROR:
            case GNUTLS_E_NO_CERTIFICATE_FOUND:
            case GNUTLS_E_CERTIFICATE_ERROR:
                // 没有标准 errc 对应，落回 system_category，用绝对值
                return {-ret, std::system_category()};
            default:
                return {-ret, std::system_category()};
        }
    }

    struct transport_ctx {
        native_socket_t fd{invalid_socket_value};
    };

    ssize_t gnutls_push_cb(gnutls_transport_ptr_t ptr, const void *buf, size_t len) {
        auto *ctx = static_cast<transport_ctx *>(ptr);
        return ::send(ctx->fd, buf, len, MSG_NOSIGNAL);
    }

    ssize_t gnutls_pull_cb(gnutls_transport_ptr_t ptr, void *buf, size_t len) {
        auto *ctx = static_cast<transport_ctx *>(ptr);
        return ::recv(ctx->fd, buf, len, 0);
    }

    int gnutls_pull_timeout_cb(gnutls_transport_ptr_t ptr, unsigned int ms) {
        auto *ctx = static_cast<transport_ctx *>(ptr);
        fd_set rfds;
        FD_ZERO(&rfds);
        FD_SET(ctx->fd, &rfds);
        struct timeval tv{};
        tv.tv_sec = ms / 1000;
        tv.tv_usec = (ms % 1000) * 1000;
        return ::select(ctx->fd + 1, &rfds, nullptr, nullptr, &tv);
    }

    class ssl_context_impl final : public ssl_context_impl_base {
    public:
        ssl_context_impl() {
            ensure_gnutls_init();
        }

        ~ssl_context_impl() override {
            if (cred_) {
                gnutls_certificate_free_credentials(cred_);
            }
            if (priority_cache_) {
                gnutls_priority_deinit(priority_cache_);
            }
        }

        std::error_code init(ssl::method mtd) noexcept override {
            method_ = mtd;
            switch (mtd) {
                case ssl::method::tls_client:
                case ssl::method::tls_server:
                    priority_str_ = "NORMAL";
                    break;
                case ssl::method::tlsv12_client:
                case ssl::method::tlsv12_server:
                    // 强制只允许 TLS 1.2，禁止 1.3
                    priority_str_ = "NORMAL:-VERS-TLS1.3";
                    break;
                case ssl::method::tlsv13_client:
                case ssl::method::tlsv13_server:
                    // 只允许 TLS 1.3
                    priority_str_ = "NORMAL:-VERS-TLS1.2:-VERS-TLS1.1:-VERS-TLS1.0";
                    break;
            }
            if (const int ret = gnutls_certificate_allocate_credentials(&cred_); ret != GNUTLS_E_SUCCESS) {
                return gnutls_ec(ret);
            }
            return rebuild_priority_cache();
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
            priority_str_ = ciphers;
            return rebuild_priority_cache();
        }

        std::error_code set_alpn_protos(const collections::vector<text::string> &protos) noexcept override {
            alpn_protos_ = protos;
            return {};
        }

        std::error_code use_certificate_file(const char *path, const char * /*password*/) noexcept override {
            cert_file_ = path ? path : "";
            return {};
        }

        std::error_code use_certificate_chain_file(const char *path) noexcept override {
            cert_file_ = path ? path : "";
            return {};
        }

        std::error_code use_private_key_file(const char *path, const char *password) noexcept override {
            key_file_ = path ? path : "";
            key_password_ = password ? password : "";

            // 如果证书也已经设置了，立即加载到凭据
            if (!cert_file_.empty() && !key_file_.empty()) {
                return load_cert_key();
            }
            return {};
        }

        std::error_code use_certificate_buffer(const void *data, std::size_t size) noexcept override {
            if (!data || size == 0) {
                return std::make_error_code(std::errc::invalid_argument);
            }
            gnutls_datum_t datum{};
            // const_cast 是安全的：gnutls_certificate_set_x509_trust_mem 不会修改数据
            datum.data = const_cast<unsigned char *>(static_cast<const unsigned char *>(data));
            datum.size = static_cast<unsigned int>(size);

            int ret = gnutls_certificate_set_x509_trust_mem(cred_, &datum, GNUTLS_X509_FMT_PEM);
            if (ret < 0) {
                return gnutls_ec(ret);
            }
            return {};
        }

        std::error_code use_system_store() noexcept override {
            int ret = gnutls_certificate_set_x509_system_trust(cred_);
            if (ret < 0) {
                return gnutls_ec(ret);
            }
            return {};
        }

        std::error_code set_session_cache_size(std::size_t size) noexcept override {
            session_cache_size_ = size;
            return {};
        }

        void *native_handle() noexcept override {
            return cred_;
        }

        gnutls_certificate_credentials_t cred() const noexcept {
            return cred_;
        }

        gnutls_priority_t priority_cache() const noexcept {
            return priority_cache_;
        }

        ssl::method method() const noexcept {
            return method_;
        }

        ssl::verify_mode verify_mode() const noexcept {
            return verify_mode_;
        }

        int verify_depth() const noexcept {
            return verify_depth_;
        }

        const collections::vector<text::string> &alpn_protos() const noexcept override {
            return alpn_protos_;
        }

        std::error_code add_verify_path(const char *path) noexcept override {
            if (!path || !cred_) {
                return std::make_error_code(std::errc::invalid_argument);
            }
            struct stat st;
            if (stat(path, &st) != 0) {
                return std::make_error_code(std::errc::no_such_file_or_directory);
            }
            int ret;
            if (S_ISDIR(st.st_mode)) {
                // 目录模式：加载目录中的所有 PEM 文件
                ret = gnutls_certificate_set_x509_trust_dir(cred_, path, GNUTLS_X509_FMT_PEM);
            } else {
                // 文件模式：加载单个 PEM 文件
                ret = gnutls_certificate_set_x509_trust_file(cred_, path, GNUTLS_X509_FMT_PEM);
            }
            if (ret < 0) {
                return gnutls_ec(ret);
            }
            verify_paths_.push_back(text::string(path));
            return {};
        }

        std::error_code use_windows_store(const char *store_name) noexcept override {
            return std::make_error_code(std::errc::not_supported);
        }

        std::error_code use_linux_store(const char *store_path) noexcept override {
            if (!store_path || !cred_) {
                return std::make_error_code(std::errc::invalid_argument);
            }
            const char *ca_path = store_path;
            if (!store_path || store_path[0] == '\0') { // NOLINT
                bool use_default = false;
                use_default = true;
                constexpr const char *default_paths[] = {"/etc/ssl/certs", "/etc/pki/tls/certs", "/usr/local/share/ca-certificates",
                                                         "/usr/share/ca-certificates"};
                for (const char *default_path: default_paths) {
                    struct stat st;
                    if (stat(default_path, &st) == 0 && S_ISDIR(st.st_mode)) {
                        ca_path = default_path;
                        use_default = false;
                        break;
                    }
                }

                if (use_default) {
                    return std::make_error_code(std::errc::no_such_file_or_directory);
                }
            }
            int ret = gnutls_certificate_set_x509_system_trust(cred_);
            if (ret >= 0) {
                // 成功加载系统信任存储
                return {};
            }
            struct stat st;
            if (stat(ca_path, &st) != 0) {
                return std::make_error_code(std::errc::no_such_file_or_directory);
            }
            if (S_ISDIR(st.st_mode)) {
                ret = gnutls_certificate_set_x509_trust_dir(cred_, ca_path, GNUTLS_X509_FMT_PEM);
            } else {
                ret = gnutls_certificate_set_x509_trust_file(cred_, ca_path, GNUTLS_X509_FMT_PEM);
            }
            if (ret < 0) {
                return gnutls_ec(ret);
            }
            verify_paths_.push_back(text::string(ca_path));
            return {};
        }

        void *native_credentials() noexcept override {
            return &cred_;
        }

        void *priority_cache() noexcept override {
            return priority_cache_;
        }

        ssl::verify_mode verify_mode() noexcept override {
            return verify_mode_;
        }

        int verify_depth() noexcept override {
            return verify_depth_;
        }

        bool is_server() noexcept override {
            return method_ == ssl::method::tls_server || method_ == ssl::method::tlsv12_server ||
                   method_ == ssl::method::tlsv13_server;
        }

    private:
        std::error_code rebuild_priority_cache() noexcept {
            if (priority_cache_) {
                gnutls_priority_deinit(priority_cache_);
                priority_cache_ = nullptr;
            }
            const char *err_pos = nullptr;
            int ret = gnutls_priority_init(&priority_cache_, priority_str_.c_str(), &err_pos);
            return gnutls_ec(ret);
        }

        std::error_code load_cert_key() noexcept {
            // 如果没有密码，走简单路径
            if (key_password_.empty()) {
                int ret = gnutls_certificate_set_x509_key_file(cred_, cert_file_.c_str(), key_file_.c_str(), GNUTLS_X509_FMT_PEM);
                return gnutls_ec(ret);
            }
            // 有密码：手动加载私钥
            gnutls_x509_privkey_t privkey{};
            gnutls_x509_privkey_init(&privkey);
            // 读取私钥文件内容（简单实现，实际应考虑大文件和错误处理）
            FILE *f = fopen(key_file_.c_str(), "rb");
            if (!f) {
                gnutls_x509_privkey_deinit(privkey);
                return std::make_error_code(std::errc::no_such_file_or_directory);
            }
            fseek(f, 0, SEEK_END);
            long fsz = ftell(f);
            fseek(f, 0, SEEK_SET);
            std::vector<unsigned char> buf(static_cast<std::size_t>(fsz));
            fread(buf.data(), 1, buf.size(), f);
            fclose(f);

            gnutls_datum_t key_datum{buf.data(), static_cast<unsigned int>(buf.size())};
            int ret = gnutls_x509_privkey_import2(privkey, &key_datum, GNUTLS_X509_FMT_PEM, key_password_.c_str(), 0);
            if (ret != GNUTLS_E_SUCCESS) {
                gnutls_x509_privkey_deinit(privkey);
                return gnutls_ec(ret);
            }

            // 加载证书
            gnutls_x509_crt_t cert{};
            gnutls_x509_crt_init(&cert);
            FILE *fc = fopen(cert_file_.c_str(), "rb");
            if (!fc) {
                gnutls_x509_privkey_deinit(privkey);
                gnutls_x509_crt_deinit(cert);
                return std::make_error_code(std::errc::no_such_file_or_directory);
            }
            fseek(fc, 0, SEEK_END);
            long csz = ftell(fc);
            fseek(fc, 0, SEEK_SET);
            std::vector<unsigned char> cbuf(static_cast<std::size_t>(csz));
            fread(cbuf.data(), 1, cbuf.size(), fc);
            fclose(fc);

            gnutls_datum_t cert_datum{cbuf.data(), static_cast<unsigned int>(cbuf.size())};
            ret = gnutls_x509_crt_import(cert, &cert_datum, GNUTLS_X509_FMT_PEM);
            if (ret != GNUTLS_E_SUCCESS) {
                gnutls_x509_privkey_deinit(privkey);
                gnutls_x509_crt_deinit(cert);
                return gnutls_ec(ret);
            }

            ret = gnutls_certificate_set_x509_key(cred_, &cert, 1, privkey);
            gnutls_x509_privkey_deinit(privkey);
            gnutls_x509_crt_deinit(cert);
            return gnutls_ec(ret);
        }

        gnutls_certificate_credentials_t cred_{nullptr};
        gnutls_priority_t priority_cache_{nullptr};
        ssl::method method_{ssl::method::tls_client};
        ssl::verify_mode verify_mode_{ssl::verify_mode::none};
        int verify_depth_{9};
        uint64_t options_{0};
        std::size_t session_cache_size_{0};
        text::string priority_str_;
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

        std::error_code attach(native_socket_t sock, bool is_server) noexcept override {
            if (sock == invalid_socket_value) {
                return std::make_error_code(std::errc::bad_file_descriptor);
            }
            destroy_session();
            transport_.fd = sock;
            is_server_ = is_server;
            unsigned int flags = is_server ? GNUTLS_SERVER : GNUTLS_CLIENT;
            if (const int ret = gnutls_init(&session_, flags); ret != GNUTLS_E_SUCCESS) {
                return gnutls_ec(ret);
            }
            // 注册自定义 transport 回调，传入 transport_ctx 指针
            gnutls_transport_set_ptr(session_, &transport_);
            gnutls_transport_set_push_function(session_, gnutls_push_cb);
            gnutls_transport_set_pull_function(session_, gnutls_pull_cb);
            gnutls_transport_set_pull_timeout_function(session_, gnutls_pull_timeout_cb);
            if (pending_ctx_) {
                return apply_context_internal(pending_ctx_);
            }
            return {};
        }

        std::error_code apply_context(ssl_context_impl *ctx) noexcept {
            pending_ctx_ = ctx;
            if (session_) {
                return apply_context_internal(ctx);
            }
            return {};
        }

        native_socket_t underlying_socket() const noexcept override {
            return transport_.fd;
        }

        std::error_code handshake() noexcept override {
            if (!session_) {
                return std::make_error_code(std::errc::not_connected);
            }
            int ret;
            do {
                ret = gnutls_handshake(session_);
            } while (ret == GNUTLS_E_AGAIN || ret == GNUTLS_E_INTERRUPTED);

            if (ret == GNUTLS_E_SUCCESS) {
                handshaked_ = true;
                reset_operation();
            } else {
                pending_op_ = ssl_operation_type::handshake;
                wants_retry_ = gnutls_error_is_fatal(ret) == 0;
                wants_read_ = (ret == GNUTLS_E_AGAIN); // NOLINT
                wants_write_ = false;
            }
            return gnutls_ec(ret);
        }

        void async_handshake(io_context_impl_base &ctx_impl, completion_op *op) noexcept override {
            if (!session_ || !op) {
                return;
            }
            pending_op_ = ssl_operation_type::handshake;

            if (const int ret = gnutls_handshake(session_); ret == GNUTLS_E_SUCCESS) {
                handshaked_ = true;
                wants_retry_ = false;
                wants_read_ = false;
                wants_write_ = false;
                op->complete(op_result{}, false);
            } else if (ret == GNUTLS_E_AGAIN || ret == GNUTLS_E_INTERRUPTED) {
                int dir = gnutls_record_get_direction(session_);
                wants_read_ = (dir == 0);
                wants_write_ = (dir == 1);
                wants_retry_ = true;
                // 不回调，等 io_context 重新投递
            } else {
                wants_retry_ = false;
                op->complete(op_result{nullptr, 0, -ret}, false);
            }
        }

        std::error_code shutdown() noexcept override {
            if (!session_) {
                return {};
            }
            int ret;
            do {
                ret = gnutls_bye(session_, GNUTLS_SHUT_WR);
            } while (ret == GNUTLS_E_AGAIN || ret == GNUTLS_E_INTERRUPTED);

            handshaked_ = false;
            return gnutls_ec(ret);
        }

        void async_shutdown(io_context_impl_base &ctx_impl, completion_op *op) noexcept override {
            if (!session_ || !op) {
                return;
            }
            pending_op_ = ssl_operation_type::shutdown;
            // GNUTLS_SHUT_WR：单向关闭，只发 close_notify，不等对端回复
            if (const int ret = gnutls_bye(session_, GNUTLS_SHUT_WR); ret == GNUTLS_E_SUCCESS) {
                handshaked_ = false;
                wants_retry_ = false;
                op->complete(op_result{}, false);
            } else if (ret == GNUTLS_E_AGAIN || ret == GNUTLS_E_INTERRUPTED) {
                const int dir = gnutls_record_get_direction(session_);
                wants_read_ = (dir == 0);
                wants_write_ = (dir == 1);
                wants_retry_ = true;
            } else {
                op->complete(op_result{nullptr, 0, -ret}, false);
            }
        }

        std::ptrdiff_t write_some(const void *buf, std::size_t len, std::error_code &ec) noexcept override {
            if (!session_ || !handshaked_) {
                ec = std::make_error_code(std::errc::not_connected);
                return -1;
            }
            pending_op_ = ssl_operation_type::write;
            const ssize_t ret = gnutls_record_send(session_, buf, len);
            if (ret >= 0) {
                ec.clear();
                reset_operation();
                return static_cast<std::ptrdiff_t>(ret);
            }
            // AGAIN / INTERRUPTED：非致命
            if (ret == GNUTLS_E_AGAIN || ret == GNUTLS_E_INTERRUPTED) {
                int dir = gnutls_record_get_direction(session_);
                wants_read_ = (dir == 0);
                wants_write_ = (dir == 1);
                wants_retry_ = true;
            } else {
                wants_retry_ = false;
            }
            ec = gnutls_ec(static_cast<int>(ret));
            return -1;
        }

        std::ptrdiff_t read_some(void *buf, std::size_t len, std::error_code &ec) noexcept override {
            if (!session_ || !handshaked_) {
                ec = std::make_error_code(std::errc::not_connected);
                return -1;
            }
            pending_op_ = ssl_operation_type::read;
            const ssize_t ret = gnutls_record_recv(session_, buf, len);
            if (ret > 0) {
                ec.clear();
                reset_operation();
                return ret;
            }
            if (ret == 0) {
                // 对端发了 close_notify
                ec.clear();
                handshaked_ = false;
                return 0;
            }
            if (ret == GNUTLS_E_AGAIN || ret == GNUTLS_E_INTERRUPTED) {
                int dir = gnutls_record_get_direction(session_);
                wants_read_ = (dir == 0);
                wants_write_ = (dir == 1);
                wants_retry_ = true;
            } else {
                wants_retry_ = false;
            }
            ec = gnutls_ec(static_cast<int>(ret));
            return -1;
        }

        void async_write_some(const void *buf, std::size_t len, io_context_impl_base & /*ctx_impl*/,
                              completion_op *op) noexcept override {
            if (!op) {
                return;
            }
            std::error_code ec;
            const std::ptrdiff_t n = write_some(buf, len, ec);
            if (!ec || !wants_retry_) {
                op->complete(op_result{nullptr, static_cast<std::size_t>(n > 0 ? n : 0), ec ? ec.value() : 0}, false);
            }
        }

        void async_read_some(void *buf, std::size_t len, io_context_impl_base & /*ctx_impl*/, completion_op *op) noexcept override {
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
            return session_ != nullptr && transport_.fd != invalid_socket_value;
        }

        text::string negotiated_protocol() const noexcept override {
            if (!session_) {
                return {};
            }
            gnutls_datum_t proto{};
            if (gnutls_alpn_get_selected_protocol(session_, &proto) == GNUTLS_E_SUCCESS && proto.data) {
                return text::string(reinterpret_cast<const char *>(proto.data), proto.size);
            }
            return {};
        }

        text::string peer_certificate_subject() const noexcept override {
            return get_cert_field(true);
        }

        text::string peer_certificate_issuer() const noexcept override {
            return get_cert_field(false);
        }

        void set_server_name(const char *hostname) noexcept override {
            server_name_ = hostname ? hostname : "";
            if (session_ && !server_name_.empty()) {
                // 在握手前设置 SNI
                gnutls_server_name_set(session_, GNUTLS_NAME_DNS, server_name_.c_str(), server_name_.size());
            }
        }

        const char *server_name() const noexcept override {
            return server_name_.c_str();
        }

        void *native_handle() noexcept override {
            return session_;
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
            pending_op_ = ssl_operation_type::read; // 默认
            wants_retry_ = false;
            wants_read_ = false;
            wants_write_ = false;
        }

        std::error_code apply_context(const ssl_context_params &params) noexcept override {
            if (!session_) {
                return std::make_error_code(std::errc::invalid_argument);
            }

            // 设置优先级
            int ret = gnutls_priority_set(session_, static_cast<gnutls_priority_t>(params.priority_cache));
            if (ret != GNUTLS_E_SUCCESS) {
                return gnutls_ec(ret);
            }

            // 绑定证书凭据
            ret = gnutls_credentials_set(session_, GNUTLS_CRD_CERTIFICATE,
                                         static_cast<gnutls_certificate_credentials_t>(params.native_credentials));
            if (ret != GNUTLS_E_SUCCESS) {
                return gnutls_ec(ret);
            }

            // 设置验证模式
            if (params.verify_mode == static_cast<int>(ssl::verify_mode::none)) {
                gnutls_certificate_server_set_request(session_, GNUTLS_CERT_IGNORE);
            } else {
                unsigned int gnutls_flags = GNUTLS_VERIFY_ALLOW_X509_V1_CA_CRT;
                if (params.verify_mode & static_cast<int>(ssl::verify_mode::fail_if_no_peer_cert)) {
                    if (params.is_server) {
                        gnutls_certificate_server_set_request(session_, GNUTLS_CERT_REQUIRE);
                    }
                } else if (params.is_server) {
                    gnutls_certificate_server_set_request(session_, GNUTLS_CERT_REQUEST);
                }
                gnutls_certificate_set_verify_flags(static_cast<gnutls_certificate_credentials_t>(params.native_credentials),
                                                    gnutls_flags);
            }

            // 设置验证深度
            gnutls_certificate_set_verify_limits(static_cast<gnutls_certificate_credentials_t>(params.native_credentials),
                                                 8192, // max_bits
                                                 params.verify_depth);

            // 设置 ALPN
            if (!params.alpn_protos.empty()) {
                std::vector<gnutls_datum_t> datums;
                datums.reserve(params.alpn_protos.size());
                for (const auto &p: params.alpn_protos) {
                    gnutls_datum_t d{};
                    d.data = const_cast<unsigned char *>(reinterpret_cast<const unsigned char *>(p.c_str()));
                    d.size = static_cast<unsigned int>(p.size());
                    datums.push_back(d);
                }
                ret = gnutls_alpn_set_protocols(session_, datums.data(), static_cast<unsigned int>(datums.size()),
                                                GNUTLS_ALPN_SERVER_PRECEDENCE);
                if (ret != GNUTLS_E_SUCCESS) {
                    return gnutls_ec(ret);
                }
            }

            return {};
        }

    private:
        void destroy_session() noexcept {
            if (session_) {
                gnutls_deinit(session_);
                session_ = nullptr;
            }
            handshaked_ = false;
            reset_operation();
        }

        std::error_code apply_context_internal(ssl_context_impl *ctx) noexcept { // NOLINT
            if (!ctx || !session_) {
                return std::make_error_code(std::errc::invalid_argument);
            }
            // 设置优先级
            int ret = gnutls_priority_set(session_, static_cast<gnutls_priority_t>(ctx->priority_cache()));
            if (ret != GNUTLS_E_SUCCESS) {
                return gnutls_ec(ret);
            }
            // 绑定证书凭据
            ret = gnutls_credentials_set(session_, GNUTLS_CRD_CERTIFICATE, ctx->cred());
            if (ret != GNUTLS_E_SUCCESS)
                return gnutls_ec(ret);
            if (const auto vm = ctx->verify_mode(); vm == ssl::verify_mode::none) {
                // 不验证对端（客户端匿名，或服务端不要求客户端证书）
                gnutls_certificate_server_set_request(session_, GNUTLS_CERT_IGNORE);
            } else {
                unsigned int gnutls_flags = 0;
                const auto vm_int = static_cast<int>(vm);
                if (vm_int & static_cast<int>(ssl::verify_mode::peer)) {
                    gnutls_flags |= GNUTLS_VERIFY_ALLOW_X509_V1_CA_CRT;
                }
                // GNUTLS_CERT_REQUIRE：无证书则握手失败
                if (vm_int & static_cast<int>(ssl::verify_mode::fail_if_no_peer_cert)) {
                    if (is_server_) {
                        gnutls_certificate_server_set_request(session_, GNUTLS_CERT_REQUIRE);
                    }
                } else if (is_server_) {
                    gnutls_certificate_server_set_request(session_, GNUTLS_CERT_REQUEST);
                }
                gnutls_certificate_set_verify_flags(ctx->cred(), gnutls_flags);
            }
            // GnuTLS 通过 gnutls_certificate_set_verify_limits 设置链深度
            gnutls_certificate_set_verify_limits(ctx->cred(),
                                                 8192, // max_bits
                                                 ctx->verify_depth()); // max_depth
            // ALPN
            const auto &protos = ctx->alpn_protos();
            if (!protos.empty()) {
                std::vector<gnutls_datum_t> datums;
                datums.reserve(protos.size());
                for (const auto &p: protos) {
                    gnutls_datum_t d{};
                    // GnuTLS 在 gnutls_alpn_set_protocols 调用期间只读，不会修改
                    d.data = const_cast<unsigned char *>(reinterpret_cast<const unsigned char *>(p.c_str()));
                    d.size = static_cast<unsigned int>(p.size());
                    datums.push_back(d);
                }
                ret = gnutls_alpn_set_protocols(session_, datums.data(), static_cast<unsigned int>(datums.size()),
                                                GNUTLS_ALPN_SERVER_PRECEDENCE);
                if (ret != GNUTLS_E_SUCCESS)
                    return gnutls_ec(ret);
            }
            // SNI（客户端）
            if (!is_server_ && !server_name_.empty()) {
                gnutls_server_name_set(session_, GNUTLS_NAME_DNS, server_name_.c_str(), server_name_.size());
            }
            return {};
        }

        text::string get_cert_field(bool subject) const noexcept {
            if (!session_) {
                return {};
            }
            unsigned int list_size = 0;
            const gnutls_datum_t *certs = gnutls_certificate_get_peers(session_, &list_size);
            if (!certs || list_size == 0) {
                return {};
            }
            gnutls_x509_crt_t cert{};
            if (gnutls_x509_crt_init(&cert) != GNUTLS_E_SUCCESS)
                return {};
            if (gnutls_x509_crt_import(cert, &certs[0], GNUTLS_X509_FMT_DER) != GNUTLS_E_SUCCESS) {
                gnutls_x509_crt_deinit(cert);
                return {};
            }
            char buf[512]{};
            std::size_t buf_sz = sizeof(buf);
            int ret;
            if (subject) {
                ret = gnutls_x509_crt_get_dn(cert, buf, &buf_sz);
            } else {
                ret = gnutls_x509_crt_get_issuer_dn(cert, buf, &buf_sz);
            }
            gnutls_x509_crt_deinit(cert);
            if (ret != GNUTLS_E_SUCCESS) {
                return {};
            }
            return text::string(buf);
        }

        gnutls_session_t session_{nullptr};
        transport_ctx transport_{};
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
