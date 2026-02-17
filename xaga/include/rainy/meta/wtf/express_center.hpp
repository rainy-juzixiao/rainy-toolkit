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
#ifndef RAINY_META_WTF_EXPRESS_CENTER_HPP
#define RAINY_META_WTF_EXPRESS_CENTER_HPP // NOLINT

#include <atomic>
#include <condition_variable>
#include <future>
#include <mutex>
#include <queue>
#include <rainy/collections/dense_map.hpp>
#include <rainy/collections/list.hpp>
#include <rainy/foundation/typeinfo.hpp>
#include <rainy/meta/reflection/function.hpp>
#include <shared_mutex>
#include <thread>

namespace rainy::meta::wtf::implements {
    /**
     * @brief 监听器结构，表示一个事件订阅者
     *
     * 封装了可调用对象及其唯一标识符，用于事件分发
     */
    struct listener {
        listener() noexcept = default;

        /**
         * @brief 从可调用对象构造监听器
         * @tparam Fx 可调用对象类型
         * @param recipient 接收事件的可调用对象
         */
        template <typename Fx,
                  type_traits::other_trans::enable_if_t<
                      !type_traits::type_relations::is_same_v<type_traits::other_trans::decay_t<Fx>, meta::reflection::function> &&
                          !type_traits::type_relations::is_same_v<type_traits::other_trans::decay_t<Fx>, listener>,
                      int> = 0>
        explicit listener(Fx &&recipient) : // NOLINT
            recipient(utility::forward<Fx>(recipient)),
            identifier(reinterpret_cast<std::uintptr_t>(this->recipient.target<type_traits::other_trans::decay_t<Fx>>())) {
        }

        /**
         * @brief 从函数对象和标识符构造监听器
         * @param fn 反射函数对象
         * @param id 唯一标识符
         */
        listener(reflection::function fn, const std::uintptr_t id) noexcept : recipient(utility::move(fn)), identifier(id) { // NOLINT
        }

        listener(listener &&right) noexcept :
            recipient(utility::move(right.recipient)), identifier(right.identifier), enabled(right.enabled) {
        }

        listener &operator=(listener &&o) noexcept {
            if (this != &o) {
                recipient = utility::move(o.recipient);
                identifier = o.identifier;
                enabled = o.enabled;
            }
            return *this;
        }

        listener(const listener &o) noexcept = default;

        listener &operator=(const listener &o) noexcept {
            if (this != &o) {
                recipient = o.recipient;
                identifier = o.identifier;
                enabled = o.enabled;
            }
            return *this;
        }

        reflection::function recipient;
        std::uintptr_t identifier{0};
        bool enabled{true};
    };

    /**
     * @brief 拦截条目，用于事件拦截链
     *
     * 包含谓词和处理函数，决定事件是否被拦截以及如何处理
     */
    struct interception_entry {
        interception_entry() = default;

        /**
         * @brief 构造拦截条目
         * @param pred 谓词函数，决定是否应用处理
         * @param handler 处理函数
         */
        interception_entry(reflection::function pred, reflection::function handler) :
            predicate(utility::move(pred)), handler(utility::move(handler)), enabled{true} { // NOLINT
        }

        interception_entry(const interception_entry &o) noexcept = default;

        interception_entry(interception_entry &&o) noexcept :
            predicate(utility::move(o.predicate)), handler(utility::move(o.handler)), enabled(o.enabled) {
        }

        interception_entry &operator=(const interception_entry &o) noexcept {
            if (this != &o) {
                predicate = o.predicate;
                handler = o.handler;
                enabled = o.enabled;
            }
            return *this;
        }

        interception_entry &operator=(interception_entry &&o) noexcept {
            if (this != &o) {
                predicate = utility::move(o.predicate);
                handler = utility::move(o.handler);
                enabled = o.enabled;
            }
            return *this;
        }

        reflection::function predicate;
        reflection::function handler;
        bool enabled{true};
    };
}

namespace rainy::meta::wtf {
    enum class emit_policy {
        fire_and_forget,
        wait_for_all
    };
}

namespace rainy::meta::wtf::execution {
    struct immediate_t {
        explicit immediate_t() = default;
    };

    struct locked_t {
        explicit locked_t() = default;
    };

    struct async_t {
        explicit constexpr async_t() = default;
        explicit constexpr async_t(const emit_policy p) noexcept : policy{p} { // NOLINT
        }

        emit_policy policy{emit_policy::fire_and_forget};
    };

    inline constexpr immediate_t immediate{};
    inline constexpr locked_t locked{};
    inline constexpr async_t async{};
    inline constexpr async_t async_wait{emit_policy::wait_for_all};
}

namespace rainy::meta::wtf {
    class express_center;
    class dispatcher;

    /**
     * @brief 订阅令牌，用于管理事件订阅
     *
     * 标识一个特定的事件订阅，可用于取消订阅或修改状态
     */
    class subscription {
    public:
        friend class express_center;
        friend class dispatcher;

        subscription() = default;

        RAINY_NODISCARD bool empty() const noexcept {
            return identifier == 0;
        }

        RAINY_NODISCARD std::size_t event_id() const noexcept {
            return event_id_;
        }

        bool operator==(const subscription &o) const noexcept {
            return event_id_ == o.event_id_ && identifier == o.identifier;
        }

        bool operator!=(const subscription &o) const noexcept {
            return !(*this == o);
        }

    private:
        subscription(const std::size_t eid, const std::uintptr_t id) : event_id_{eid}, identifier{id} {
        }

        std::size_t event_id_{};
        std::uintptr_t identifier{};
    };

    enum class priority_level : std::size_t {
        the_root = 0,
        system = 1,
        admin_mode = 2,
        high_priv_user = 3,
        user_mode = 4
    };

    inline constexpr std::size_t priority_count = 5;

    enum class interception_result {
        continue_running,
        continue_to_event_handler,
        reject_event,
        next_priority_handler
    };

    /**
     * @brief 事件中心核心类
     *
     * 管理事件订阅和拦截，支持多优先级拦截链和多种执行策略。
     * 线程安全，支持并发订阅和发射。
     */
    class express_center {
    public:
        express_center() {
            instance.reserve(16);
            interceptions.reserve(16);
        }

        express_center(const express_center &) = delete;
        express_center &operator=(const express_center &) = delete;
        express_center(express_center &&) = delete;
        express_center &operator=(express_center &&) = delete;

        ~express_center() {
            shutdown();
        }

        /**
         * @brief 订阅事件
         * @tparam Fx 可调用对象类型，第一个参数必须是事件类型（const引用）
         * @param slot 事件处理函数
         * @return 订阅令牌
         *
         * 事件类型通过函数第一个参数自动推导
         */
        template <typename Fx,
                  type_traits::other_trans::enable_if_t<
                      !type_traits::type_relations::is_same_v<type_traits::other_trans::decay_t<Fx>, reflection::function>, int> = 0>
        subscription subscribe(Fx &&slot) {
            using namespace type_traits;
            using traits = primary_types::function_traits<other_trans::decay_t<Fx>>;
            using type_list = typename other_trans::tuple_like_to_type_list<typename traits::tuple_like_type>::type; // NOLINT

            static_assert(traits::arity != 0, "You pass an empty slot. It can't proceed your event!");
            static_assert(type_relations::is_convertible_v<const typename other_trans::type_at<0, type_list>::type &, // NOLINT
                                                           typename other_trans::type_at<0, type_list>::type>,
                          "The first parameter (Event) must be taken by const ref.");

            using event_type = type_traits::other_trans::decay_t<typename other_trans::type_at<0, type_list>::type>;
            constexpr std::size_t event_id = foundation::ctti::typeinfo::get_type_hash<event_type>();
            std::unique_lock lock{rw_mutex_};
            auto &list = instance[event_id]; // NOLINT
            reflection::function fn{utility::forward<Fx>(slot)};
            auto *raw = fn.target<other_trans::decay_t<Fx>>();
            auto id = reinterpret_cast<std::uintptr_t>(raw);
            list.emplace_back(utility::move(fn), id); // NOLINT
            return subscription{event_id, id};
        }

        /**
         * @brief 取消订阅
         * @param token 订阅令牌
         * @return 成功返回true
         */
        bool unsubscribe(const subscription &token) {
            if (token.empty()) {
                return false;
            }
            std::unique_lock lock{rw_mutex_};
            const auto iter = instance.find(token.event_id_);
            if (iter == instance.end()) {
                return false;
            }
            auto &list = iter->second;
            const auto it = core::algorithm::find_if(list.begin(), list.end(),
                                                     [&](const implements::listener &l) { return l.identifier == token.identifier; });
            if (it == list.end()) {
                return false;
            }
            list.erase(it);
            return true;
        }

        /**
         * @brief 设置订阅启用状态
         * @param token 订阅令牌
         * @param enabled 是否启用
         * @return 成功返回true
         */
        bool set_enabled(const subscription &token, const bool enabled) {
            if (token.empty()) {
                return false;
            }
            std::shared_lock lock{rw_mutex_};
            const auto iter = instance.find(token.event_id_);
            if (iter == instance.end()) {
                return false;
            }
            for (auto &l: iter->second) {
                if (l.identifier == token.identifier) {
                    l.enabled = enabled;
                    return true;
                }
            }
            return false;
        }

        /**
         * @brief 立即发射事件
         * @tparam Event 事件类型
         * @tparam Args 附加参数类型
         * @param event 事件对象
         * @param args 附加参数
         *
         * 在当前线程同步执行所有匹配的订阅者
         */
        template <typename Event, typename... Args>
        void emit(execution::immediate_t, Event &&event, Args &&...args) const {
            using event_type = type_traits::other_trans::decay_t<Event>;
            constexpr std::size_t event_id = foundation::ctti::typeinfo::get_type_hash<event_type>();
            if (!run_interception_chain<event_type>(event_id, event)) {
                return;
            }
            dispatch_to_listeners(event_id, utility::forward<Event>(event), utility::forward<Args>(args)...);
        }

        /**
         * @brief 在外部锁保护下发射事件
         * @tparam Mutex 互斥量类型
         * @tparam Event 事件类型
         * @tparam Args 附加参数类型
         * @param mutex 外部互斥量
         * @param event 事件对象
         * @param args 附加参数
         */
        template <typename Mutex, typename Event, typename... Args>
        void emit(execution::locked_t, Mutex &mutex, Event &&event, Args &&...args) const {
            std::scoped_lock lock{mutex};
            emit(execution::immediate, utility::forward<Event>(event), utility::forward<Args>(args)...);
        }

        /**
         * @brief 在内部锁保护下发射事件
         * @tparam Event 事件类型
         * @tparam Args 附加参数类型
         * @param event 事件对象
         * @param args 附加参数
         */
        template <typename Event, typename... Args>
        void emit(execution::locked_t, Event &&event, Args &&...args) const {
            std::shared_lock lock{rw_mutex_};
            emit(execution::immediate, utility::forward<Event>(event), utility::forward<Args>(args)...);
        }

        /**
         * @brief 异步发射事件
         * @tparam Event 事件类型
         * @tparam Args 附加参数类型
         * @param policy 异步策略
         * @param event 事件对象
         * @param args 附加参数
         *
         * 根据策略可能立即返回或等待处理完成
         */
        template <typename Event, typename... Args>
        void emit(const execution::async_t policy, Event &&event, Args &&...args) {
            ensure_worker_running();

            if (policy.policy == emit_policy::fire_and_forget) {
                std::unique_lock lock{queue_mutex_};
                task_queue_.emplace([this, e = type_traits::other_trans::decay_t<Event>{utility::forward<Event>(event)},
                                     tpl = std::make_tuple(utility::forward<Args>(args)...)]() mutable {
                    // NOLINTBEGIN
                    std::apply([&](auto &&...a) { emit(execution::locked, utility::move(e), utility::forward<decltype(a)>(a)...); },
                               utility::move(tpl));
                    // NOLINTEND
                });
                queue_cv_.notify_one();
            } else {
                auto promise = std::make_shared<std::promise<void>>();
                const auto future = promise->get_future();
                {
                    std::unique_lock lock{queue_mutex_};
                    task_queue_.emplace([this, promise, e = type_traits::other_trans::decay_t<Event>{utility::forward<Event>(event)},
                                         tpl = std::make_tuple(utility::forward<Args>(args)...)]() mutable {
                        // NOLINTBEGIN
                        std::apply(
                            [&](auto &&...a) { emit(execution::locked, utility::move(e), utility::forward<decltype(a)>(a)...); },
                            utility::move(tpl));
                        // NOLINTEND
                        promise->set_value();
                    });
                    queue_cv_.notify_one();
                }
                future.wait();
            }
        }

        /**
         * @brief 拦截事件
         * @tparam Event 事件类型
         * @tparam Fx 处理函数类型
         * @tparam Pred 谓词函数类型
         * @param level 优先级级别
         * @param handler 处理函数，签名：interception_result(const Event&)
         * @param predicate 谓词函数，签名：bool(const Event&)，可选
         * @return 订阅令牌
         *
         * 拦截器按优先级顺序执行，可通过谓词条件触发
         */
        template <typename Event, typename Fx, typename Pred = std::nullptr_t>
        subscription intercept(priority_level level, Fx &&handler, Pred &&predicate = {}) {
            using namespace type_traits;
            static_assert(type_properties::is_invocable_r_v<interception_result, other_trans::decay_t<Fx>, const Event &>,
                          "Interception handler must be: interception_result(const Event &)");

            constexpr std::size_t event_id = foundation::ctti::typeinfo::get_type_hash<Event>();
            const auto lvl = static_cast<std::size_t>(level);

            reflection::function pred_fn{};
            if constexpr (!type_relations::is_same_v<other_trans::decay_t<Pred>, std::nullptr_t>) {
                static_assert(type_properties::is_invocable_r_v<bool, other_trans::decay_t<Pred>, const Event &>,
                              "Predicate must be: bool(const Event &)");
                pred_fn.rebind(utility::forward<Pred>(predicate));
            }

            reflection::function handler_fn{utility::forward<Fx>(handler)};
            auto *raw = handler_fn.target<other_trans::decay_t<Fx>>();
            const auto id = reinterpret_cast<std::uintptr_t>(raw);

            std::unique_lock lock{rw_mutex_};
            auto &chain = interceptions[event_id]; // NOLINT
            chain.at(lvl).emplace_back(utility::move(pred_fn), utility::move(handler_fn)); // NOLINT

            return subscription{event_id, id};
        }

        /**
         * @brief 移除指定优先级的所有拦截器
         * @param event_id 事件ID
         * @param level 优先级级别
         */
        void unintercept(const std::size_t event_id, priority_level level) noexcept {
            std::unique_lock lock{rw_mutex_};
            const auto iter = interceptions.find(event_id);
            if (iter == interceptions.end()) {
                return;
            }
            iter->second.at(static_cast<std::size_t>(level)).clear(); // NOLINT
        }

        /**
         * @brief 移除事件的所有拦截器
         * @param event_id 事件ID
         */
        void unintercept_all(const std::size_t event_id) noexcept {
            std::unique_lock lock{rw_mutex_};
            const auto iter = interceptions.find(event_id);
            if (iter == interceptions.end()) {
                return;
            }
            for (auto &bucket: iter->second) {
                bucket.clear();
            }
        }

        /**
         * @brief 获取订阅者数量
         * @param event_id 事件ID
         * @return 订阅者数量
         */
        RAINY_NODISCARD std::size_t subscriber_count(const std::size_t event_id) const {
            std::shared_lock lock{rw_mutex_};
            const auto iter = instance.find(event_id);
            if (iter == instance.end()) {
                return 0;
            }
            return iter->second.size();
        }

        /**
         * @brief 检查事件是否有订阅者
         * @param event_id 事件ID
         * @return 有订阅者返回true
         */
        RAINY_NODISCARD bool has_subscribers(const std::size_t event_id) const {
            return subscriber_count(event_id) > 0;
        }

        /**
         * @brief 清空所有订阅和拦截
         */
        void clear() noexcept {
            std::unique_lock lock{rw_mutex_};
            instance.clear();
            interceptions.clear();
        }

        /**
         * @brief 清空指定事件的所有订阅
         * @param event_id 事件ID
         */
        void clear_event(const std::size_t event_id) noexcept {
            std::unique_lock lock{rw_mutex_};
            instance.erase(event_id);
        }

        /**
         * @brief 关闭事件中心，停止工作线程
         */
        void shutdown() noexcept {
            {
                std::unique_lock lock{queue_mutex_};
                stop_worker_ = true;
            }
            queue_cv_.notify_all();
            if (worker_thread_.joinable()) {
                worker_thread_.join();
            }
        }

    private:
        template <typename Event, typename... Args>
        bool run_interception_chain(const std::size_t event_id, const Event &event, Args &&...args) const {
            const auto iter = interceptions.find(event_id);
            if (iter == interceptions.end()) {
                return true;
            }
            for (std::size_t lvl = 0; lvl < priority_count; ++lvl) {
                const auto &bucket = iter->second.at(lvl); // NOLINT
                bool skip_to_next_priority = false;

                for (const auto &entry: bucket) {
                    if (!entry.enabled) {
                        continue;
                    }
                    if (entry.predicate.is_invocable_with(event)) {
                        if (const bool pass = entry.predicate.static_invoke(event).template convert<bool>(); !pass) {
                            continue;
                        }
                    }
                    switch (auto result = entry.handler.static_invoke(event, utility::forward<Args>(args)...)
                                              .template convert<interception_result>()) {
                        case interception_result::continue_running:
                            break;
                        case interception_result::continue_to_event_handler:
                            return true;
                        case interception_result::reject_event:
                            return false;
                        case interception_result::next_priority_handler:
                            skip_to_next_priority = true;
                            break;
                        default:
                            break;
                    }
                    if (skip_to_next_priority) {
                        break;
                    }
                }
            }
            return true;
        }

        template <typename Event, typename... Args>
        void dispatch_to_listeners(const std::size_t event_id, Event &&event, Args &&...args) const {
            const auto iter = instance.find(event_id);
            if (iter == instance.end()) {
                return;
            }
            for (const auto &item: iter->second) {
                if (!item.enabled) {
                    continue;
                }
                if (item.recipient.is_invocable_with(utility::forward<Event>(event), utility::forward<Args>(args)...)) {
                    item.recipient.static_invoke(utility::forward<Event>(event), utility::forward<Args>(args)...);
                }
            }
        }

        void ensure_worker_running() {
            std::call_once(worker_once_, [this] {
                stop_worker_ = false;
                worker_thread_ = std::thread([this] {
                    while (true) {
                        std::function<void()> task;
                        {
                            std::unique_lock lock{queue_mutex_};
                            queue_cv_.wait(lock, [this] { return stop_worker_ || !task_queue_.empty(); });
                            if (stop_worker_ && task_queue_.empty()) {
                                return;
                            }
                            task = utility::move(task_queue_.front());
                            task_queue_.pop();
                        }
                        task();
                    }
                });
            });
        }

        mutable std::shared_mutex rw_mutex_;
        collections::dense_map<std::size_t, std::vector<implements::listener>> instance;
        collections::dense_map<std::size_t, collections::array<collections::list<implements::interception_entry>, priority_count>>
            interceptions;

        std::thread worker_thread_;
        std::queue<std::function<void()>> task_queue_;
        std::mutex queue_mutex_;
        std::condition_variable queue_cv_;
        std::atomic<bool> stop_worker_{false};
        std::once_flag worker_once_;
    };

    /**
     * @brief 事件分发器
     */
    class dispatcher {
    public:
        explicit dispatcher(express_center &center) noexcept : center_{utility::addressof(center)} {
        }
        
        /**
         * @brief 订阅事件
         * @tparam Fx 可调用对象类型
         * @param slot 事件处理函数
         * @return 订阅令牌
         */
        template <typename Fx>
        RAINY_NODISCARD subscription subscribe(Fx &&slot) {
            return center_->subscribe(utility::forward<Fx>(slot));
        }
        
        /**
         * @brief 取消订阅
         * @param token 订阅令牌
         * @return 成功返回true
         */
        bool unsubscribe(const subscription &token) { // NOLINT
            return center_->unsubscribe(token);
        }

        /**
         * @brief 设置订阅启用状态
         * @param token 订阅令牌
         * @param on 是否启用
         * @return 成功返回true
         */
        bool set_enabled(const subscription &token, const bool on = true) { // NOLINT
            return center_->set_enabled(token, on);
        }

        /**
         * @brief 发射事件
         * @tparam Policy 执行策略类型
         * @tparam Event 事件类型
         * @tparam Args 附加参数类型
         * @param policy 执行策略
         * @param event 事件对象
         * @param args 附加参数
         */
        template <typename Policy, typename Event, typename... Args>
        void emit(Policy &&policy, Event &&event, Args &&...args) {
            center_->emit(utility::forward<Policy>(policy), utility::forward<Event>(event), utility::forward<Args>(args)...);
        }

        /**
         * @brief 拦截事件
         * @tparam Event 事件类型
         * @tparam Fx 处理函数类型
         * @tparam Pred 谓词函数类型
         * @param level 优先级级别
         * @param handler 处理函数
         * @param pred 谓词函数
         * @return 订阅令牌
         */
        template <typename Event, typename Fx, typename Pred = std::nullptr_t>
        subscription intercept(priority_level level, Fx &&handler, Pred &&pred = {}) {
            return center_->intercept<Event>(level, utility::forward<Fx>(handler), utility::forward<Pred>(pred));
        }

        /**
         * @brief 移除事件指定优先级的拦截器
         * @tparam Event 事件类型
         * @param level 优先级级别
         */
        template <typename Event>
        void unintercept(const priority_level level) { // NOLINT
            constexpr std::size_t eid = foundation::ctti::typeinfo::get_type_hash<Event>();
            center_->unintercept(eid, level);
        }

        /**
         * @brief 移除事件的所有拦截器
         * @tparam Event 事件类型
         */
        template <typename Event>
        void unintercept_all() { // NOLINT
            constexpr std::size_t eid = foundation::ctti::typeinfo::get_type_hash<Event>();
            center_->unintercept_all(eid);
        }

        /**
         * @brief 获取事件订阅者数量
         * @tparam Event 事件类型
         * @return 订阅者数量
         */
        template <typename Event>
        RAINY_NODISCARD std::size_t subscriber_count() const {
            constexpr std::size_t eid = foundation::ctti::typeinfo::get_type_hash<Event>();
            return center_->subscriber_count(eid);
        }

        /**
         * @brief 检查事件是否有订阅者
         * @tparam Event 事件类型
         * @return 有订阅者返回true
         */
        template <typename Event>
        RAINY_NODISCARD bool has_subscribers() const {
            return subscriber_count<Event>() > 0;
        }

        /**
         * @brief 获取底层事件中心
         * @return 事件中心引用
         */
        RAINY_NODISCARD express_center &center() noexcept { // NOLINT
            return *center_;
        }

    private:
        express_center *center_;
    };
}

#endif
