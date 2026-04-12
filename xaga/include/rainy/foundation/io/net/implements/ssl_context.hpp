#ifndef RAINY_FOUNDATION_IO_NET_IMPLEMENTS_SSL_IMPL_HPP
#define RAINY_FOUNDATION_IO_NET_IMPLEMENTS_SSL_IMPL_HPP
#include <rainy/collections/vector.hpp>
#include <rainy/foundation/io/net/fwd.hpp>
#include <rainy/foundation/io/net/implements/sock.hpp>
#include <rainy/foundation/io/net/io_context.hpp>
#include <system_error>
#include <vector>

namespace rainy::foundation::io::net::ssl {
    enum class method {
        tls_client, // TLS 客户端
        tls_server, // TLS 服务端
        tlsv12_client, // TLS 1.2 客户端（兼容旧系统）
        tlsv12_server, // TLS 1.2 服务端
        tlsv13_client, // TLS 1.3 客户端
        tlsv13_server // TLS 1.3 服务端
    };

    enum class verify_mode {
        none = 0, // 不验证
        peer = 1 << 0, // 验证对端证书
        fail_if_no_peer_cert = 1 << 1, // 无证书则失败
        client_once = 1 << 2 // 仅客户端验证一次
    };
}

namespace rainy::foundation::io::net::implements {
    struct ssl_context_params {
        void *native_credentials;
        void *priority_cache;
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
        virtual void *priority_cache() noexcept = 0;
        virtual ssl::verify_mode verify_mode() noexcept = 0;
        virtual int verify_depth() noexcept = 0;
        virtual const collections::vector<text::string> &alpn_protos() const noexcept = 0;
        virtual bool is_server() noexcept = 0;

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

        virtual bool is_handshaked() const noexcept = 0;
        virtual bool is_open() const noexcept = 0;

        virtual text::string negotiated_protocol() const noexcept = 0;
        virtual text::string peer_certificate_subject() const noexcept = 0;
        virtual text::string peer_certificate_issuer() const noexcept = 0;

        virtual void set_server_name(const char *hostname) noexcept = 0;
        virtual const char *server_name() const noexcept = 0;

        virtual void *native_handle() noexcept = 0;
        virtual ssl_operation_type pending_operation() const noexcept = 0;

        virtual bool wants_retry() const noexcept = 0;
        virtual bool wants_read() const noexcept = 0;
        virtual bool wants_write() const noexcept = 0;
        virtual void reset_operation() noexcept = 0;

        virtual std::error_code apply_context(const ssl_context_params &params) noexcept = 0;
    };

    RAINY_TOOLKIT_API memory::nebula_ptr<ssl_context_impl_base> create_ssl_context_impl();
    RAINY_TOOLKIT_API memory::nebula_ptr<ssl_stream_impl_base> create_ssl_stream_impl();

}

#endif
