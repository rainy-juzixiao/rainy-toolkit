#ifndef RAINY_FOUNDATION_IO_NET_SSL_CONTEXT_HPP
#define RAINY_FOUNDATION_IO_NET_SSL_CONTEXT_HPP

#include <rainy/collections/vector.hpp>
#include <rainy/core/core.hpp>
#include <rainy/foundation/io/net/implements/ssl_context.hpp>

namespace rainy::foundation::io::net::ssl {
    RAINY_INLINE verify_mode operator|(verify_mode a, verify_mode b) {
        return static_cast<verify_mode>(static_cast<int>(a) | static_cast<int>(b));
    }

    class certificate_options {
    public:
        friend class context;

        certificate_options &add_certificate_file(const text::string &path, const text::string &password = "");
        certificate_options &add_certificate_chain_file(const text::string &path);
        certificate_options &add_private_key_file(const text::string &path, const text::string &password = "");
        certificate_options &add_certificate_buffer(const void *data, std::size_t size);
        certificate_options &use_system_store();
        certificate_options &add_verify_path(const text::string &path);
        certificate_options &use_windows_store(const text::string &store_name = "MY");
        certificate_options &use_linux_store(const text::string &store_path = "/etc/ssl/certs");

    private:
        collections::vector<utility::pair<text::string, text::string>> certificate_files;
        collections::vector<text::string> certificate_chain_files;
        collections::vector<utility::pair<text::string, text::string>> private_key_files;
        collections::vector<utility::pair<const void *, std::size_t>> certificate_buffers;
        bool use_system_store_flag = false;
        collections::vector<text::string> verify_paths;
        text::string windows_store_name;
        text::string linux_store_path;
        bool use_windows_store_flag = false;
        bool use_linux_store_flag = false;
    };

    class context {
    public:
        template <typename>
        friend class ssl_stream;

        explicit context(method mtd);
        ~context();

        context(const context &) = delete;
        context &operator=(const context &) = delete;
        context(context &&other) noexcept;
        context &operator=(context &&other) noexcept;
        void set_options(uint64_t options);
        void set_verify_mode(verify_mode mode);
        void set_verify_depth(int depth);
        void set_cipher_list(const text::string &ciphers);
        void set_alpn_protos(const collections::vector<text::string> &protos);
        std::error_code load_certificate(const certificate_options &opts);
        std::error_code use_private_key(const certificate_options &opts);

        void *native_handle();
        void set_session_cache_size(std::size_t size);

    private:
        implements::ssl_context_params get_params() noexcept {
            return {impl_->native_handle(), impl_->priority_cache(), static_cast<int>(impl_->verify_mode()),
                    impl_->verify_depth(),  impl_->alpn_protos(),    impl_->is_server()};
        }

        memory::nebula_ptr<implements::ssl_context_impl_base> impl_;
    };
}

#endif
