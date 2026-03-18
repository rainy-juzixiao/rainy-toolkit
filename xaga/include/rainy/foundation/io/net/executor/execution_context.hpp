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
#ifndef RAINY_FOUNDATION_IO_NET_EXECUTOR_EXECUTION_CONTEXT_HPP
#define RAINY_FOUNDATION_IO_NET_EXECUTOR_EXECUTION_CONTEXT_HPP
#include <rainy/core/core.hpp>
#include <rainy/foundation/concurrency/mutex.hpp>
#include <rainy/foundation/typeinfo.hpp>

namespace rainy::foundation::exceptions::net {
    class service_already_exists : public logic::logic_error {
    public:
        service_already_exists(const char *message, const source &location) : logic_error(message, location) {
        }
        service_already_exists(const std::string &message, const source &location) : logic_error(message, location) {
        }
    };

    RAINY_INLINE void throw_service_already_exists(
        const std::string &message, const diagnostics::source_location &location = diagnostics::source_location::current()) {
        throw_exception(service_already_exists{message, location});
    }
}

namespace rainy::foundation::io::net {
    enum class fork_event {
        prepare,
        parent,
        child
    };

    class execution_context {
    public:
        class service { // NOLINT
        public:
            struct key {
                key() = default;

                const ctti::typeinfo *type_info_{nullptr};
                const void *id_{nullptr};
            };

            execution_context &context() noexcept;

        protected:
            explicit service(execution_context &owner);

            // NOLINTBEGIN
            service(const service &) = delete;
            service &operator=(const service &) = delete;
            // NOLINTEND

            virtual ~service() = default;

        private:
            virtual void shutdown() noexcept = 0;
            virtual void notify_fork(fork_event e) {
            }

            friend class execution_context;

            key key_;
            execution_context &owner_;
            service *next_;
        };

        execution_context();
        execution_context(const execution_context &) = delete;
        execution_context &operator=(const execution_context &) = delete;
        virtual ~execution_context();

        void notify_fork(fork_event e);

    protected:
        void shutdown() noexcept;
        void destroy() noexcept;

    private:
        using factory_type = service *(*) (void *);

        service *do_use_service(const service::key &key, factory_type factory, void *owner_ctx);

        void do_make_service(const service::key &key, service *new_svc);

        bool do_has_service(const service::key &key) const noexcept;

        static bool keys_match(const service::key &a, const service::key &b) noexcept;

        template <typename Service>
        static service *create_service(void *owner_ctx) {
            return new Service(*static_cast<execution_context *>(owner_ctx));
        }

        template <typename Service>
        static service::key make_key() {
            service::key k;
            if constexpr (requires { typename Service::key_type; }) {
                k.type_info_ = &rainy_typeid(typename Service::key_type);
            } else {
                k.id_ = static_cast<const void *>(&Service::id);
            }
            return k;
        }

        template <typename Service>
        friend typename Service::key_type &use_service(execution_context &ctx);

        template <typename Service, typename... Args>
        friend Service &make_service(execution_context &ctx, Args &&...args);

        template <typename Service>
        friend bool has_service(const execution_context &ctx) noexcept;

        mutable concurrency::mutex mutex_;
        service *first_service_;
    };

    /**
     * use_service<Service>(ctx)
     *
     * 若 Service 尚未注册，自动构造并注册；返回对该 service 的引用。
     * 注意：返回类型是 Service::key_type&（Networking TS 规范）。
     */
    template <class Service>
    typename Service::key_type &use_service(execution_context &ctx) {
        execution_context::service::key k = execution_context::make_key<Service>();
        auto *svc = ctx.do_use_service(k, &execution_context::create_service<Service>, &ctx);
        return static_cast<typename Service::key_type &>(*svc);
    }

    /**
     * make_service<Service>(ctx, args...)
     *
     * 显式构造 Service 并注册；若已存在则抛出 service_already_exists。
     */
    template <class Service, class... Args>
    Service &make_service(execution_context &ctx, Args &&...args) {
        execution_context::service::key k = execution_context::make_key<Service>();
        // 先检查是否已存在
        if (ctx.do_has_service(k)) {
            exceptions::net::throw_service_already_exists("service already exists in execution_context");
        }
        rainy_let new_svc = new Service(ctx, utility::forward<Args>(args)...);
        // NOLINTBEGIN
        try {
            ctx.do_make_service(k, new_svc);
        } catch (...) {
            delete new_svc;
            throw;
        }
        // NOLINTEND
        return *new_svc;
    }

    /**
     * has_service<Service>(ctx)
     *
     * 返回 true 当且仅当 Service::key_type 对应的 service 已存在于 ctx 中。
     */
    template <class Service>
    bool has_service(const execution_context &ctx) noexcept {
        execution_context::service::key k = execution_context::make_key<Service>();
        return ctx.do_has_service(k);
    }
}

#endif
