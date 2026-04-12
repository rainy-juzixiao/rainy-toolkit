#include <rainy/foundation/io/net/ssl/context.hpp>

namespace rainy::foundation::io::net::ssl {
    certificate_options& certificate_options::add_certificate_file(
        const text::string& path, const text::string& password) {
        certificate_files.emplace_back(path, password);
        return *this;
    }

    certificate_options& certificate_options::add_certificate_chain_file(const text::string& path) {
        certificate_chain_files.push_back(path);
        return *this;
    }

    certificate_options& certificate_options::add_private_key_file(
        const text::string& path, const text::string& password) {
        private_key_files.emplace_back(path, password);
        return *this;
    }

    certificate_options& certificate_options::add_certificate_buffer(
        const void* data, std::size_t size) {
        certificate_buffers.emplace_back(data, size);
        return *this;
    }

    certificate_options& certificate_options::use_system_store() {
        use_system_store_flag = true;
        return *this;
    }

    certificate_options& certificate_options::add_verify_path(const text::string& path) {
        verify_paths.push_back(path);
        return *this;
    }

    certificate_options& certificate_options::use_windows_store(const text::string& store_name) {
        use_windows_store_flag = true;
        windows_store_name = store_name;
        return *this;
    }

    certificate_options& certificate_options::use_linux_store(const text::string& store_path) {
        use_linux_store_flag = true;
        linux_store_path = store_path;
        return *this;
    }

    // context 实现
    context::context(method mtd) : impl_(implements::create_ssl_context_impl()) {
        if (!impl_) {
            throw std::runtime_error("Failed to create SSL context implementation");
        }
        auto ec = impl_->init(mtd);
        if (ec) {
            throw std::system_error(ec, "Failed to initialize SSL context");
        }
    }

    context::~context() = default;
    context::context(context&& other) noexcept = default;
    context& context::operator=(context&& other) noexcept = default;

    void context::set_options(uint64_t options) {
        auto ec = impl_->set_options(options);
        if (ec) {
            throw std::system_error(ec, "Failed to set SSL options");
        }
    }

    void context::set_verify_mode(verify_mode mode) {
        auto ec = impl_->set_verify_mode(mode);
        if (ec) {
            throw std::system_error(ec, "Failed to set verify mode");
        }
    }

    void context::set_verify_depth(int depth) {
        auto ec = impl_->set_verify_depth(depth);
        if (ec) {
            throw std::system_error(ec, "Failed to set verify depth");
        }
    }

    void context::set_cipher_list(const text::string& ciphers) {
        auto ec = impl_->set_cipher_list(ciphers.c_str());
        if (ec) {
            throw std::system_error(ec, "Failed to set cipher list");
        }
    }

    void context::set_alpn_protos(const collections::vector<text::string>& protos) {
        const auto ec = impl_->set_alpn_protos(protos);
        if (ec) {
            throw std::system_error(ec, "Failed to set ALPN protocols");
        }
    }

    std::error_code context::load_certificate(const certificate_options& opts) {
        // 1. 加载单个证书文件
        for (const auto& [path, password] : opts.certificate_files) {
            auto ec = impl_->use_certificate_file(path.c_str(), password.c_str());
            if (ec) return ec;
        }
        // 2. 加载证书链文件
        for (const auto& path : opts.certificate_chain_files) {
            auto ec = impl_->use_certificate_chain_file(path.c_str());
            if (ec) return ec;
        }
        // 3. 从内存缓冲区加载证书
        for (const auto& [data, size] : opts.certificate_buffers) {
            auto ec = impl_->use_certificate_buffer(data, size);
            if (ec) return ec;
        }
        // 4. 使用系统证书存储（跨平台：Linux /etc/ssl/certs，Windows Cert Store）
        if (opts.use_system_store_flag) {
            auto ec = impl_->use_system_store();
            if (ec) return ec;
        }
        // 5. 添加自定义验证路径（Linux目录或Windows证书存储路径）
        for (const auto& path : opts.verify_paths) {
            auto ec = impl_->add_verify_path(path.c_str());
            if (ec) return ec;
        }
        // 6. Windows专用证书存储
        if (opts.use_windows_store_flag) {
            auto ec = impl_->use_windows_store(opts.windows_store_name.c_str());
            if (ec) return ec;
        }
        // 7. Linux专用证书存储
        if (opts.use_linux_store_flag) {
            auto ec = impl_->use_linux_store(opts.linux_store_path.c_str());
            if (ec) return ec;
        }
        return std::error_code();
    }

    std::error_code context::use_private_key(const certificate_options& opts) {
        for (const auto& [path, password] : opts.private_key_files) {
            auto ec = impl_->use_private_key_file(path.c_str(), password.c_str());
            if (ec) return ec;
        }

        return std::error_code();
    }

    void* context::native_handle() {
        return impl_->native_handle();
    }

    void context::set_session_cache_size(std::size_t size) {
        auto ec = impl_->set_session_cache_size(size);
        if (ec) {
            throw std::system_error(ec, "Failed to set session cache size");
        }
    }
}