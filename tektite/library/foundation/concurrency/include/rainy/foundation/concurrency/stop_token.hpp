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
#ifndef RAINY_FOUNDATION_CONCURRENCY_STOP_TOKEN_HPP
#define RAINY_FOUNDATION_CONCURRENCY_STOP_TOKEN_HPP
#include <rainy/core/concurrency/atomic.hpp>
#include <rainy/core/memory/shared_ptr.hpp>
#include <rainy/core/platform.hpp>
#include <rainy/foundation/concurrency/mutex.hpp>

namespace rainy::foundation::concurrency {
    class stop_source;
    class stop_token;

    template <typename Callback>
    class stop_callback;
}

namespace rainy::foundation::concurrency::implements {
    class stop_state;

    class stop_callback_base {
    public:
        using state_pointer = core::memory::shared_ptr<stop_state>;

        stop_callback_base(const stop_callback_base &) = delete;
        stop_callback_base &operator=(const stop_callback_base &) = delete;

        virtual ~stop_callback_base();

        virtual void invoke() noexcept = 0;

    protected:
        explicit stop_callback_base(state_pointer state) noexcept : state_(utility::move(state)) {
        }

        state_pointer state_;
        stop_callback_base *prev_{nullptr};
        stop_callback_base *next_{nullptr};
        bool registered_{false};

        friend class stop_state;
    };

    class stop_state {
    public:
        using mutex_type = concurrency::mutex;
        using lock_type = concurrency::unique_lock<mutex_type>;

        RAINY_NODISCARD rain_fn stop_requested() const noexcept -> bool {
            return flag_.load() != 0;
        }

        RAINY_NODISCARD rain_fn stop_possible() const noexcept -> bool {
            return flag_.load() != 0 || source_refs_.load() != 0;
        }

        rain_fn request_stop() noexcept -> bool {
            const lock_type lock(mtx_);
            if (flag_.load() != 0) {
                return false;
            }
            flag_.store(1);
            stop_callback_base *cb = head_;
            head_ = nullptr;
            while (cb != nullptr) {
                stop_callback_base *const next = cb->next_;
                cb->next_ = nullptr;
                cb->prev_ = nullptr;
                cb->registered_ = false;
                cb->invoke(); // NOLINT
                cb = next;
            }
            return true;
        }

        rain_fn register_callback(stop_callback_base *cb) -> bool {
            const lock_type lock(mtx_);
            if (flag_.load() != 0) {
                return false;
            }
            cb->prev_ = nullptr;
            cb->next_ = head_;
            cb->registered_ = true;
            if (head_ != nullptr) {
                head_->prev_ = cb;
            }
            head_ = cb;
            return true;
        }

        rain_fn remove_callback(stop_callback_base *cb) noexcept -> void {
            {
                const lock_type lock(mtx_);
                if (cb->registered_) {
                    if (cb->prev_ != nullptr) {
                        cb->prev_->next_ = cb->next_;
                    } else {
                        head_ = cb->next_;
                    }
                    if (cb->next_ != nullptr) {
                        cb->next_->prev_ = cb->prev_;
                    }
                    cb->prev_ = nullptr;
                    cb->next_ = nullptr;
                    cb->registered_ = false;
                }
            }
        }

        rain_fn add_source_ref() noexcept -> void {
            source_refs_.fetch_add(1);
        }

        rain_fn remove_source_ref() noexcept -> void {
            source_refs_.fetch_sub(1);
        }

    private:
        mutex_type mtx_{};
        stop_callback_base *head_{nullptr};
        rainy::core::concurrency::atomic<unsigned int> flag_{0};
        rainy::core::concurrency::atomic<unsigned int> source_refs_{1};
    };

    inline stop_callback_base::~stop_callback_base() {
        if (state_ != nullptr) {
            stop_state *const state = state_.get();
            state->remove_callback(this);
        }
    }
}

namespace rainy::foundation::concurrency {
    class nostopstate_t {
    public:
        nostopstate_t() = default;
        nostopstate_t(const nostopstate_t &) = default;
        nostopstate_t &operator=(const nostopstate_t &) = default;
    };

    inline constexpr nostopstate_t nostopstate{};

    class stop_source;

    class stop_token {
    public:
        stop_token() noexcept = default;

        RAINY_NODISCARD rain_fn stop_possible() const noexcept -> bool {
            return state_ != nullptr && state_->stop_possible();
        }

        RAINY_NODISCARD rain_fn stop_requested() const noexcept -> bool {
            return state_ != nullptr && state_->stop_requested();
        }

        rain_fn swap(stop_token &right) noexcept -> void {
            state_.swap(right.state_);
        }

        friend bool operator==(const stop_token &left, const stop_token &right) noexcept {
            return left.state_.get() == right.state_.get();
        }

        friend bool operator!=(const stop_token &left, const stop_token &right) noexcept {
            return !(left == right);
        }

    private:
        friend class stop_source;
        template <typename Callback>
        friend class stop_callback;

        explicit stop_token(const implements::stop_callback_base::state_pointer &state) noexcept : state_(state) {
        }

        implements::stop_callback_base::state_pointer state_;
    };

    inline void swap(stop_token &left, stop_token &right) noexcept {
        left.swap(right);
    }

    class stop_source {
    public:
        stop_source() : state_(rainy::core::memory::make_shared<implements::stop_state>()) {
        }

        explicit stop_source(nostopstate_t) noexcept : state_() {
        }

        ~stop_source() {
            if (state_ != nullptr) {
                state_->remove_source_ref();
            }
        }

        stop_source(const stop_source &other) noexcept : state_(other.state_) {
            if (state_ != nullptr) {
                state_->add_source_ref();
            }
        }

        stop_source(stop_source &&other) noexcept = default;

        stop_source &operator=(const stop_source &other) noexcept {
            if (this != &other) {
                stop_source tmp(other);
                swap(tmp);
            }
            return *this;
        }

        stop_source &operator=(stop_source &&other) noexcept {
            if (this != &other) {
                if (state_ != nullptr) {
                    state_->remove_source_ref();
                }
                state_ = utility::move(other.state_);
            }
            return *this;
        }

        RAINY_NODISCARD rain_fn get_token() const noexcept -> stop_token {
            return stop_token(state_);
        }

        RAINY_NODISCARD rain_fn stop_requested() const noexcept -> bool {
            return state_ != nullptr && state_->stop_requested();
        }

        RAINY_NODISCARD rain_fn stop_possible() const noexcept -> bool {
            return state_ != nullptr;
        }

        rain_fn request_stop() noexcept -> bool {
            return state_ != nullptr && state_->request_stop();
        }

        rain_fn swap(stop_source &right) noexcept -> void {
            state_.swap(right.state_);
        }

        friend bool operator==(const stop_source &left, const stop_source &right) noexcept {
            return left.state_.get() == right.state_.get();
        }

        friend bool operator!=(const stop_source &left, const stop_source &right) noexcept {
            return !(left == right);
        }

    private:
        implements::stop_callback_base::state_pointer state_;
    };

    inline void swap(stop_source &left, stop_source &right) noexcept {
        left.swap(right);
    }

    template <typename Callback>
    class stop_callback final : public implements::stop_callback_base {
    public:
        using callback_type = Callback;

        stop_callback(const stop_token &token, Callback cb) : stop_callback_base(token.state_), func_(utility::move(cb)) {
            init();
        }

        stop_callback(stop_token &&token, Callback cb) : stop_callback_base(utility::move(token.state_)), func_(utility::move(cb)) {
            init();
        }

        ~stop_callback() override = default;

        stop_callback(const stop_callback &) = delete;
        stop_callback &operator=(const stop_callback &) = delete;
        stop_callback(stop_callback &&) = delete;
        stop_callback &operator=(stop_callback &&) = delete;

    private:
        void init() noexcept {
            if (state_ != nullptr && state_->register_callback(this)) {
                return;
            }
            invoke();
        }

        void invoke() noexcept override {
            return static_cast<void>(func_());
        }

        Callback func_;
    };
}

#endif
