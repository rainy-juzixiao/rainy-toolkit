#ifndef RAINY_META_WTF_EXPRESS_CENTER_HPP
#define RAINY_META_WTF_EXPRESS_CENTER_HPP
#include <deque>
#include <list>
#include <rainy/collections/dense_map.hpp>
#include <rainy/core/core.hpp>
#include <rainy/meta/reflection/function.hpp>

namespace rainy::meta::wtf::implements {
    struct listener {
        listener() noexcept = default;

        template <typename Fx, type_traits::other_trans::enable_if_t<
                                   type_traits::type_properties::is_constructible_v<meta::reflection::function, Fx>, int> = 0>
        listener(Fx &&recipient) : recipient(utility::forward<Fx>(recipient)), identifier{0} {
            this->identifier = reinterpret_cast<std::size_t>(this->recipient.template target<meta::reflection::function>());
        }

        meta::reflection::function recipient;
        std::size_t identifier;
    };
}

namespace rainy::meta::wtf {
    struct recipient_token {
    public:
        friend class express_center;

        recipient_token() = default;
    
        bool empty() const noexcept {
            return identifier == 0;
        }

        std::size_t event_id() const noexcept {
            return event_id_;
        }

    private:
        recipient_token(std::size_t event_id, std::uintptr_t identifier) : event_id_{event_id}, identifier{identifier} {
        }

        std::size_t event_id_{};
        std::uintptr_t identifier;
    };

    enum class priority_level : std::size_t {
        the_root,
        system,
        admin_mode,
        high_priv_user,
        user_mode
    };

    enum class interception_result {
        continue_running,
        continue_to_event_handler,
        reject_event,
        next_priority_handler
    };
    
    struct test_event {};

    class express_center {
    public:
        express_center() noexcept {
            this->instance.reserve(10);
        }

        template <typename Event, typename Fx,
                  type_traits::other_trans::enable_if_t<
                      !type_traits::type_relations::is_same_v<type_traits::cv_modify::remove_cvref_t<Fx>, meta::reflection::function>,
                      int> = 0>
        recipient_token accept_package(Fx &&recipient) noexcept {
            using namespace type_traits;
            using traits = primary_types::function_traits<Fx>;
            using type_list = typename other_trans::tuple_like_to_type_list<typename traits::tuple_like_type>::type;
            static_assert(traits::arity != 0, "You pass a empty recipient, It can't proceed your package!");
            static_assert(type_relations::is_convertible_v<const cv_modify::remove_cvref_t<Event> &,
                                                           typename other_trans::type_at<0, type_list>::type>,
                          "No, irrelevant recipients cannot appear and your Event, cannot be a non-const!");
            constexpr std::size_t event_id = foundation::ctti::typeinfo::get_type_hash<other_trans::decay_t<Event>>();
            if (auto iter = instance.find(event_id); iter != instance.end()) {
                meta::reflection::function &recipient_object = iter->second.emplace_back(utility::forward<Fx>(recipient));
                return {event_id, reinterpret_cast<std::uintptr_t>(recipient_object.target<meta::reflection::function>())};
            } else {
                std::vector<meta::reflection::function> list(5);
                list.emplace_back(utility::forward<Fx>(recipient));
                auto res = instance.insert({event_id, utility::move(list)});
                return {event_id, reinterpret_cast<std::uintptr_t>(res.first->second.back().target<meta::reflection::function>())};
            }
        }

        template <typename Event, typename... Args>
        RAINY_INLINE void urgent_delivery(Event &&event, Args &&...args) const {
            using namespace type_traits;
            using namespace type_traits::type_relations;
            constexpr std::size_t event_id = foundation::ctti::typeinfo::get_type_hash<other_trans::decay_t<Event>>();
            auto iter = instance.find(event_id);
            if (iter != instance.end()) {
                for (const auto &item: iter->second) {
                    if constexpr (is_any_of_v<utility::any, Args...> || is_any_of_v<meta::reflection::object_view, Args...>) {
                        if (item.is_invocable<const Event &, Args...>()) {
                            item.static_invoke(utility::forward<Event>(event), utility::forward<Args>(args)...);
                        }
                    } else if (item.is_invocable_with(utility::forward<Event>(event), utility::forward<Args>(args)...)) {
                        item.static_invoke(utility::forward<Event>(event), utility::forward<Args>(args)...);
                    }
                }
            }
        }

        template <typename Mutex, typename Event, typename... Args>
        void normal_delivery(Mutex &mutex, Event &&event, Args &&...args) const {
            std::lock_guard lock{mutex};
            urgent_delivery(utility::forward<Event>(event), utility::forward<Args>(args)...);
        }

        template <typename Event, typename Fx, typename Pred = int,
                  type_traits::other_trans::enable_if_t<
                      !type_traits::type_relations::is_same_v<type_traits::cv_modify::remove_cvref_t<Fx>, meta::reflection::function>,
                      int> = 0>
        recipient_token set_interception(priority_level level, Fx &&handler, Pred &&predicate = {}) noexcept {
            using namespace type_traits;
            using traits = primary_types::function_traits<Fx>;
            using type_list = typename other_trans::tuple_like_to_type_list<typename traits::tuple_like_type>::type;
            static_assert(type_properties::is_invocable_r_v<interception_result, Fx, const Event &>, "Invalid interception handler");
            meta::reflection::function predicate_;
            if constexpr (type_traits::primary_types::function_traits<Fx>::valid) {
                static_assert(type_properties::is_invocable_r_v<bool, Pred, const Event &>, "Invalid predicate");
                predicate_.rebind(utility::forward<Pred>(predicate));
            }
            meta::reflection::function handler_{utility::forward<Fx>(handler)};
            constexpr std::size_t event_id = foundation::ctti::typeinfo::get_type_hash<other_trans::decay_t<Event>>();
            auto iter = interceptions.find(event_id);
            if (iter == interceptions.end()) {
                return {};
            }
            std::size_t storage_level{static_cast<std::size_t>(level)};
            auto &cont = iter->second.at(storage_level);
            cont.emplace_back(utility::move(predicate_), utility::move(handler_));
            return {};
        }


        void remove_interception(std::size_t event_id) noexcept {
        }

    private:
        std::mutex mtx_;
        collections::dense_map<
            std::size_t,
            collections::array<
                std::list<utility::pair<meta::reflection::function /* pred */, meta::reflection::function /* handler */>>, 5>>
            interceptions;
        collections::dense_map<std::size_t, std::vector<meta::reflection::function>> instance;
    };

    class dispatcher {
    public:
    private:
    };
}

#endif