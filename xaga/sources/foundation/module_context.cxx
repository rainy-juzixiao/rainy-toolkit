/*
 * Copyright 2025 rainy-juzixiao
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
#include <rainy/foundation/dynamic_library/module_context.hpp>
#include <filesystem>

namespace rainy::foundation::dyanmic_library {
    struct module_context::module_context_private {
        core::handle handle{0};
        bool is_observe{false};
    };

    module_context::module_context_private *module_context::create_ctx() {
        return new module_context_private; // NOLINT
    }

    void module_context::destroy_ctx(module_context_private const *ctx) {
        if (ctx) { // NOLINT
            delete ctx;
        }
    }

    module_context::module_context(const std::string_view module_path, const bool load) noexcept {
        namespace fs = std::filesystem;
        rainy_const ctx = create_ctx();
        if (load) {
            ctx->handle = implements::try_to_get_module(module_path, true);
        } else {
            ctx->handle = implements::load_module(module_path, true);
            ctx->is_observe = true;
        }
        if (!ctx->handle) {
            destroy_ctx(ctx);
            return;
        }
        this->private_ = ctx;
    }

    module_context::module_context(module_context &&right) noexcept {
        this->private_ = utility::exchange(right.private_, {});
    }

    module_context &module_context::operator=(module_context &&right) noexcept {
        if (this == &right) {
            return *this;
        }
        release();
        if (this->private_) {
            private_->handle = utility::exchange(right.private_->handle, 0);
            private_->is_observe = private_->is_observe;
            right.private_->is_observe = false;
        } else {
            this->private_ = utility::exchange(right.private_, nullptr);
        }
        return *this;
    }

    module_context::~module_context() {
        release();
        if (private_) {
            destroy_ctx(private_);
            private_ = nullptr;
        }
    }

    bool module_context::release() noexcept { // NOLINT
        if (is_loaded()) {
            if (!is_observe()) {
                if (!implements::release_module(&private_->handle)) {
                    std::terminate(); // 不应当发生的情况
                }
            }
            private_->handle = {};
            private_->is_observe = false;
            return true;
        }
        return false;
    }

    bool module_context::load(const std::string_view module_path) noexcept {
        namespace fs = std::filesystem;
        release();
        if (!private_) {
            this->private_ = create_ctx();
        }
        private_->handle = implements::load_module(module_path, true);
        if (!private_->handle) {
            return false;
        }
        private_->is_observe = false;
        return true;
    }

    bool module_context::try_get_module(const std::string_view module_path) noexcept {
        namespace fs = std::filesystem;
        release();
        if (!private_) {
            this->private_ = create_ctx();
        }
        private_->handle = implements::try_to_get_module(module_path, true);
        if (!private_->handle) {
            return false;
        }
        private_->is_observe = true;
        return true;
    }

    farproc_fn module_context::load_symbol(const std::string_view symbol_name) const noexcept {
        if (is_loaded()) {
            return implements::load_symbol(private_->handle, symbol_name);
        }
        return nullptr;
    }

    bool module_context::is_loaded() const noexcept {
        if (!private_) {
            return false;
        }
        return private_->handle != 0;
    }

    bool module_context::is_observe() const noexcept {
        if (is_loaded()) {
            return private_->is_observe;
        }
        return false;
    }

    void *module_context::native_handle() const noexcept {
        if (is_loaded()) {
            return reinterpret_cast<void *>(private_->handle);
        }
        return nullptr;
    }
}
