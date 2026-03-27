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
#include <rainy/foundation/io/net/executor/execution_context.hpp>

namespace rainy::foundation::io::net {
    execution_context::service::service(execution_context &owner) : owner_(owner), next_(nullptr) {
    }

    execution_context &execution_context::service::context() noexcept { // NOLINT
        return owner_;
    }

    execution_context::execution_context() : first_service_(nullptr) {
    }

    execution_context::~execution_context() {}

    void execution_context::notify_fork(fork_event e) { // NOLINT
        // 收集所有 service 指针（链表头是最新插入的）
        std::vector<service *> services;
        {
            concurrency::lock_guard lock(mutex_);
            for (service *s = first_service_; s; s = s->next_) {
                services.push_back(s);
            }
        }
        // Networking TS：fork_prepare 按 "beginning of service lifetime" 顺序
        if (e == fork_event::prepare) {
            // 按创建顺序（链表逆序）通知
            for (auto it = services.rbegin(); it != services.rend(); ++it) {
                (*it)->notify_fork(e);
            }
        } else {
            // 按逆创建顺序（链表顺序）通知
            for (auto *s: services) {
                s->notify_fork(e);
            }
        }
    }

    void execution_context::shutdown() noexcept { // NOLINT
        concurrency::lock_guard lock(mutex_);
        // 逆创建顺序 = 链表正向遍历
        for (service *s = first_service_; s; s = s->next_) {
            s->shutdown();
        }
    }

    void execution_context::destroy() noexcept {
        concurrency::lock_guard lock(mutex_);
        while (first_service_) {
            service *next = first_service_->next_;
            delete first_service_;
            first_service_ = next;
        }
    }

    execution_context::service *execution_context::do_use_service(const service::key &key, const factory_type factory,
                                                                  void *owner_ctx) {
        concurrency::unique_lock lock(mutex_);

        // 先在已有链表中查找
        for (service *s = first_service_; s; s = s->next_) {
            if (keys_match(s->key_, key)) {
                return s;
            }
        }
        // 未找到，需要创建。释放锁以允许 service 构造函数中递归调用 use_service。
        lock.unlock();
        service *new_svc = factory(owner_ctx);
        new_svc->key_ = key;
        lock.lock();
        // 重新检查：在释放锁期间，可能有另一个线程也完成了创建
        for (service *s = first_service_; s; s = s->next_) {
            if (keys_match(s->key_, key)) {
                // 已被别的线程创建，丢弃我们自己创建的
                delete new_svc;
                return s;
            }
        }
        // 头插入链表
        new_svc->next_ = first_service_;
        first_service_ = new_svc;
        return new_svc;
    }

    void execution_context::do_make_service(const service::key &key, service *new_svc) {
        concurrency::lock_guard lock(mutex_);
        // 二次检查：加锁后再确认是否已存在
        for (const service *s = first_service_; s; s = s->next_) {
            if (keys_match(s->key_, key)) {
                exceptions::net::throw_service_already_exists("service already exists in execution_context");
            }
        }
        new_svc->key_ = key;
        new_svc->next_ = first_service_;
        first_service_ = new_svc;
    }

    bool execution_context::do_has_service(const service::key &key) const noexcept {
        concurrency::lock_guard lock(mutex_);
        for (const service *s = first_service_; s; s = s->next_) {
            if (keys_match(s->key_, key)) {
                return true;
            }
        }
        return false;
    }

    bool execution_context::keys_match(const service::key &a,
                                        const service::key &b) noexcept {
        if (a.id_ && b.id_) {
            if (a.id_ == b.id_) return true;
        }
        if (a.type_info_ && b.type_info_) {
            if (*a.type_info_ == *b.type_info_) return true;
        }
        return false;
    }
}
