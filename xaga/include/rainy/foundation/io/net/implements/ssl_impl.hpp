#ifndef RAINY_FOUNDATION_IO_NET_IMPLEMENTS_SSL_IMPL_HPP
#define RAINY_FOUNDATION_IO_NET_IMPLEMENTS_SSL_IMPL_HPP
#include <rainy/collections/vector.hpp>
#include <rainy/foundation/io/net/implements/sock.hpp>
#include <rainy/foundation/io/net/io_context.hpp>
#include <system_error>

namespace rainy::foundation::io::net::ssl {
    /**
     * \lang chinese
     * 
       TLS 连接方法枚举，用于指定 TLS 协议版本与角色。
     * @note 在 Windows 平台上，若使用 SChannel 作为 TLS 后端，
     *       部分协议版本可能受操作系统限制而无法使用。
     *       例如：Windows 10 默认不支持 TLS 1.3，Windows 10/11 需额外更新方可支持 TLS 1.3。
     *
     * \lang english
     * 
     * Enumeration of TLS connection methods, used to specify the TLS protocol version and role.
     * @note On Windows, if the SChannel backend is used for TLS,
     *       some protocol versions may be unavailable depending on the operating system.
     *       For example: Windows 10 does not support TLS 1.3 by default,
     *       and Windows 10/11 require additional updates to enable TLS 1.3.
     */
    enum class method {
        /**
         * \lang chinese
         * TLS 客户端（自动协商版本）
         *
         * \lang english
         * TLS client (auto-negotiate version)
         */
        tls_client,

        /**
         * \lang chinese
         * TLS 服务端（自动协商版本）
         *
         * \lang english
         * TLS server (auto-negotiate version)
         */
        tls_server,

        /**
         * \lang chinese
         * TLS 1.2 客户端
         *
         * \lang english
         * TLS 1.2 client
         */
        tlsv12_client,

        /**
         * \lang chinese
         * TLS 1.2 服务端
         *
         * \lang english
         * TLS 1.2 server
         */
        tlsv12_server,

        /**
         * \lang chinese
         * TLS 1.3 客户端
         *
         * \lang english
         * TLS 1.3 client
         */
        tlsv13_client,

        /**
         * \lang chinese
         * TLS 1.3 服务端
         *
         * \lang english
         * TLS 1.3 server
         */
        tlsv13_server
    };

    /**
     * \lang chinese
     * 证书验证模式枚举，用于控制 TLS 握手过程中对端证书的验证行为。
     * 各枚举值可通过位或运算组合使用，以实现更严格的验证策略。
     *
     * \lang english
     * Certificate verification mode enumeration, used to control peer certificate
     * validation behavior during the TLS handshake.
     * Enumeration values can be combined using bitwise OR to apply stricter validation policies.
     */
    enum class verify_mode {
        /**
         * \lang chinese
         * 不执行任何证书验证。
         *
         * \lang english
         * Perform no certificate verification.
         */
        none = 0,

        /**
         * \lang chinese
         * 验证对端证书的有效性（包括证书链、有效期、签名等）。
         *
         * \lang english
         * Verify the validity of the peer certificate (including chain, expiration, signature, etc.).
         */
        peer = 1 << 0,

        /**
         * \lang chinese
         * 若对端未提供证书，则握手立即失败。
         * 通常与 @c peer 组合使用。
         *
         * \lang english
         * Fail the handshake immediately if the peer does not present a certificate.
         * Typically combined with @c peer.
         */
        fail_if_no_peer_cert = 1 << 1,

        /**
         * \lang chinese
         * 仅当作为 TLS 客户端时执行一次证书验证请求。
         * 在会话重用场景下，不再重复要求对端证书。
         *
         * \lang english
         * Request certificate verification only once when acting as a TLS client.
         * In session resumption scenarios, the peer certificate is not requested again.
         */
        client_once = 1 << 2
    };
}

namespace rainy::foundation::io::net::implements {
    struct ssl_context_params {
        void *native_credentials;
        int verify_mode;
        int verify_depth;
        collections::vector<text::string> alpn_protos;
        bool is_server;
    };

    class ssl_context_impl_base {
    public:
        virtual ~ssl_context_impl_base() = default;

        virtual std::error_code init(ssl::method mtd) noexcept = 0;

        virtual std::error_code set_options(uint64_t options) noexcept = 0;
        virtual std::error_code set_verify_mode(ssl::verify_mode mode) noexcept = 0;
        virtual std::error_code set_verify_depth(int depth) noexcept = 0;
        virtual std::error_code set_cipher_list(const char *ciphers) noexcept = 0;
        virtual std::error_code set_alpn_protos(const collections::vector<text::string> &protos) noexcept = 0;

        virtual std::error_code use_certificate_file(const char *path, const char *password) noexcept = 0;
        virtual std::error_code use_certificate_chain_file(const char *path) noexcept = 0;
        virtual std::error_code use_private_key_file(const char *path, const char *password) noexcept = 0;
        virtual std::error_code use_certificate_buffer(const void *data, std::size_t size) noexcept = 0;
        virtual std::error_code use_system_store() noexcept = 0;

        virtual void *native_credentials() noexcept = 0;
        virtual ssl::verify_mode verify_mode() const noexcept = 0;
        virtual int verify_depth() noexcept = 0;
        virtual const collections::vector<text::string> &alpn_protos() const noexcept = 0;
        virtual bool is_server() const noexcept = 0;

        virtual std::error_code add_verify_path(const char *path) noexcept = 0;
        virtual std::error_code use_windows_store(const char *store_name) noexcept = 0;
        virtual std::error_code use_linux_store(const char *store_path) noexcept = 0;

        virtual std::error_code set_session_cache_size(std::size_t size) noexcept = 0;
        virtual void *native_handle() noexcept = 0;
    };

    enum class ssl_operation_type {
        handshake,
        shutdown,
        read,
        write
    };

    class ssl_stream_impl_base {
    public:
        virtual ~ssl_stream_impl_base() = default;

        virtual std::error_code attach(native_socket_t sock, bool is_server) noexcept = 0;
        virtual native_socket_t underlying_socket() const noexcept = 0;

        virtual std::error_code handshake() noexcept = 0;
        virtual void async_handshake(io_context_impl_base &ctx_impl, completion_op *op) noexcept = 0;

        virtual std::error_code shutdown() noexcept = 0;
        virtual void async_shutdown(io_context_impl_base &ctx_impl, completion_op *op) noexcept = 0;

        virtual std::ptrdiff_t write_some(const void *buf, std::size_t len, std::error_code &ec) noexcept = 0;
        virtual std::ptrdiff_t read_some(void *buf, std::size_t len, std::error_code &ec) noexcept = 0;
        virtual void async_write_some(const void *buf, std::size_t len, io_context_impl_base &ctx_impl,
                                      completion_op *op) noexcept = 0;

        virtual void async_read_some(void *buf, std::size_t len, io_context_impl_base &ctx_impl, completion_op *op) noexcept = 0;

        RAINY_NODISCARD virtual bool is_handshaked() const noexcept = 0;
        RAINY_NODISCARD virtual bool is_open() const noexcept = 0;

        RAINY_NODISCARD virtual text::string negotiated_protocol() const noexcept = 0;
        RAINY_NODISCARD virtual text::string peer_certificate_subject() const noexcept = 0;
        RAINY_NODISCARD virtual text::string peer_certificate_issuer() const noexcept = 0;

        virtual void set_server_name(const char *hostname) noexcept = 0;
        RAINY_NODISCARD virtual const char *server_name() const noexcept = 0;

        virtual void *native_handle() noexcept = 0;
        RAINY_NODISCARD virtual ssl_operation_type pending_operation() const noexcept = 0;

        RAINY_NODISCARD virtual bool wants_retry() const noexcept = 0;
        RAINY_NODISCARD virtual bool wants_read() const noexcept = 0;
        RAINY_NODISCARD virtual bool wants_write() const noexcept = 0;
        virtual void reset_operation() noexcept = 0;

        virtual std::error_code apply_context(const ssl_context_params &params) noexcept = 0;
    };

    RAINY_TOOLKIT_API memory::nebula_ptr<ssl_context_impl_base> create_ssl_context_impl();
    RAINY_TOOLKIT_API memory::nebula_ptr<ssl_stream_impl_base> create_ssl_stream_impl();

}

#endif
