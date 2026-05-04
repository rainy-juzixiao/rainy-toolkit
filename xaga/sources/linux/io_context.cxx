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
#include <rainy/core/core.hpp>
#if RAINY_USING_WINDOWS
#include <rainy/foundation/pal/asio/io_context.hpp>
#include <cstring>
#include <sys/epoll.h>
#include <unistd.h>

namespace rainy::foundation::pal::async_io::implements {
    struct epoll_context_t {
        int epoll_fd;
    };

    static epoll_context_t *get_ctx(io_ctx ctx) {
        return reinterpret_cast<epoll_context_t *>(ctx);
    }

    io_ctx create_io_context() {
        int epfd = epoll_create1(0);
        if (epfd < 0)
            return nullptr;
        auto *ctx = new epoll_context_t{epfd};
        return reinterpret_cast<io_ctx>(ctx);
    }

    void destroy_io_context(io_ctx ctx) {
        auto *context = get_ctx(ctx);
        if (!context)
            return;
        close(context->epoll_fd);
        delete context;
    }

    int register_io(io_ctx ctx, core::handle sock, uint32_t events, void *user_data) {
        auto *context = get_ctx(ctx);
        epoll_event ev{};
        ev.events = events;
        ev.data.ptr = user_data ? user_data : reinterpret_cast<void *>(static_cast<uintptr_t>(sock));
        return epoll_ctl(context->epoll_fd, EPOLL_CTL_ADD, sock, &ev);
    }

    int unregister_io(io_ctx ctx, core::handle sock) {
        auto *context = get_ctx(ctx);
        return epoll_ctl(context->epoll_fd, EPOLL_CTL_DEL, sock, nullptr);
    }

    int wait_io(io_ctx ctx, io_event *events, int max_events, int timeout_ms) {
        auto *context = get_ctx(ctx);
        epoll_event ep_events[max_events];
        int n = epoll_wait(context->epoll_fd, ep_events, max_events, timeout_ms);
        if (n <= 0)
            return n;

        for (int i = 0; i < n; ++i) {
            events[i].events = ep_events[i].events;
            events[i].user_data = ep_events[i].data.ptr;
            events[i].sock = 0;
        }
        return n;
    }
}

#endif
