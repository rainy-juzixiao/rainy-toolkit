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
#ifndef RAINY_FOUNDATION_CONTAINER_ATOM_HPP
#define RAINY_FOUNDATION_CONTAINER_ATOM_HPP

#include <rainy/core/concurrency/atomic.hpp>
#include <rainy/core/platform.hpp>
#include <rainy/core/type_traits/properties.hpp>
#include <rainy/foundation/concurrency/mutex.hpp>

namespace rainy::foundation::container::implements {
    template <typename Ty, bool = type_traits::properties::is_trivially_copyable_v<Ty>>
    class atom_base;

    template <typename Ty>
    class atom_base<Ty, true> {
    public:
        using value_type = Ty;

        atom_base() noexcept : value_() {
        }

        RAINY_CONSTEXPR20 explicit atom_base(const Ty &value) noexcept : value_(value) {
        }

        atom_base(const atom_base &) = delete;

        atom_base(atom_base &&) = delete;

        atom_base &operator=(const atom_base &) = delete;

        atom_base &operator=(atom_base &&) = delete;

        RAINY_NODISCARD Ty load(core::concurrency::memory_order order = core::concurrency::memory_order::seq_cst) const noexcept {
            return value_.load(order);
        }

        void store(const Ty &value, core::concurrency::memory_order order = core::concurrency::memory_order::seq_cst) noexcept {
            value_.store(value, order);
        }

        Ty exchange(const Ty &value, core::concurrency::memory_order order = core::concurrency::memory_order::seq_cst) noexcept {
            return value_.exchange(value, order);
        }

        bool compare_exchange(Ty &expected, const Ty &desired,
                              core::concurrency::memory_order order = core::concurrency::memory_order::seq_cst) noexcept {
            return value_.compare_exchange_strong(expected, desired, order);
        }

        RAINY_NODISCARD static bool is_lock_free() noexcept {
            return true;
        }

    protected:
        core::concurrency::atomic<Ty> value_;
    };

    template <typename Ty>
    class atom_base<Ty, false> {
    public:
        using value_type = Ty;

        atom_base() : value_() {
        }

        explicit atom_base(const Ty &value) : value_(value) {
        }

        atom_base(const atom_base &) = delete;

        atom_base(atom_base &&) = delete;

        atom_base &operator=(const atom_base &) = delete;

        atom_base &operator=(atom_base &&) = delete;

        RAINY_NODISCARD Ty load() const {
            concurrency::lock_guard<concurrency::mutex> guard(mutex_);
            return value_;
        }

        void store(const Ty &value) {
            concurrency::lock_guard<concurrency::mutex> guard(mutex_);
            value_ = value;
        }

        Ty exchange(const Ty &value) {
            concurrency::lock_guard<concurrency::mutex> guard(mutex_);
            Ty old = utility::move(value_);
            value_ = value;
            return old;
        }

        bool compare_exchange(Ty &expected, const Ty &desired) {
            concurrency::lock_guard<concurrency::mutex> guard(mutex_);
            if (value_ == expected) {
                value_ = desired;
                return true;
            }
            expected = value_;
            return false;
        }

        RAINY_NODISCARD static bool is_lock_free() noexcept {
            return false;
        }

    protected:
        mutable Ty value_;
        mutable concurrency::mutex mutex_;
    };
}

namespace rainy::foundation::container {
    /**
     * \lang english
     * @brief A thread-safe holder for a single value.
     *
     * @brief atom chooses its synchronization strategy from the stored type. For
     * types that the platform can access atomically (trivially copyable types),
     * it delegates directly to core::concurrency::atomic, giving lock-free load,
     * store, exchange and compare-exchange. For any other type it embeds a
     * mutex from the concurrency module and guards every access with it.
     *
     * @tparam Ty The stored value type.
     *
     * \lang simp-chinese
     * @brief 单个值的线程安全持有者。
     *
     * @brief atom 依据所存类型选择同步策略。对平台可原子访问的类型（可平凡复制类型），
     * 它直接委托给 core::concurrency::atomic，提供无锁的 load、store、exchange 与
     * compare-exchange。对其他类型，它内嵌一个来自 concurrency 模块的互斥量，并以
     * 之保护每一次访问。
     *
     * @tparam Ty 所存储的值类型。
     */
    template <typename Ty>
    class atom : public implements::atom_base<Ty> {
    public:
        using base = implements::atom_base<Ty>;

        using value_type = Ty;
        using base::base;
        using base::compare_exchange;
        using base::exchange;
        using base::is_lock_free;
        using base::load;
        using base::store;

        RAINY_NODISCARD operator Ty() const {
            return this->load();
        }

        atom &operator=(const Ty &value) {
            this->store(value);
            return *this;
        }
    };
}

#endif
