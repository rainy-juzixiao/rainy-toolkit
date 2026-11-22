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
#include <rainy/core/layer.hpp>
#include <rainy/foundation/willow/implements/protobuf/descriptor.hpp>

namespace rainy::foundation::willow::protobuf {
    namespace { // 为了确保descriptor不依赖concurrency模块以及标准库，此处定义了一个mutex类，使用core::layer提供的线程同步功能避免重编
        class mutex {
        public:
            mutex() noexcept {
                core::layer::mtx_create(&mtx_, core::layer::mutex_types::plain_mtx);
            }

            explicit mutex(int flags) noexcept {
                core::layer::mtx_create(&mtx_, flags);
            }

            ~mutex() noexcept {
                if (mtx_ != nullptr) {
                    core::layer::mtx_destroy(&mtx_);
                }
            }

            mutex(const mutex &) = delete;
            mutex &operator=(const mutex &) = delete;

            void lock() noexcept {
                core::layer::mtx_lock(&mtx_);
            }

            void unlock() noexcept {
                core::layer::mtx_unlock(&mtx_);
            }

        private:
            core::layer::mtx_t mtx_{nullptr};
        };

        class lock_guard {
        public:
            explicit lock_guard(mutex &m) noexcept : mtx_(&m) {
                mtx_->lock();
            }

            ~lock_guard() noexcept {
                mtx_->unlock();
            }

            lock_guard(const lock_guard &) = delete;
            lock_guard &operator=(const lock_guard &) = delete;

        private:
            mutex *mtx_;
        };
    }

    struct descriptor_pool::impl {
        const descriptor_base *find_message(core::text::string_view name) const {
            lock_guard guard{mutex_};
            auto it = entries_.find({name.data(), name.size()});
            if (it == entries_.end()) {
                return nullptr;
            }
            return it->second;
        }

        std::size_t size() const {
            lock_guard guard{mutex_};
            return entries_.size();
        }

        void register_descriptor(const descriptor_base &desc) {
            lock_guard guard{mutex_};
            entries_.emplace(desc.message_name(), &desc);
        }

        mutable mutex mutex_;
        foundation::collections::unordered_map<core::text::string, const descriptor_base *> entries_;
    };

    descriptor_pool &descriptor_pool::instance() {
        static descriptor_pool pool;
        return pool;
    }

    const descriptor_base *descriptor_pool::find_message(core::text::string_view name) const {
        return impl_->find_message(name);
    }

    std::size_t descriptor_pool::size() const {
        return impl_->size();
    }

    void descriptor_pool::register_descriptor(const descriptor_base &desc) {
        impl_->register_descriptor(desc);
    }

    descriptor_pool::descriptor_pool() : impl_(core::memory::make_nebula<impl>()) {
    }
}
