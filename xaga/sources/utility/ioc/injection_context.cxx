#include <rainy/utility/ioc.hpp>

namespace rainy::utility::ioc::implements {
    std::mutex injection_mutex;

    using inject_impl_t =
        collections::dense_map<foundation::ctti::typeinfo, utility::pair<foundation::memory::nebula_ptr<any>,
                                                                         foundation::memory::nebula_ptr<std::once_flag>>>;

    static rain_fn inject_impl() -> inject_impl_t* {
        static inject_impl_t injection_context_storage;
        return &injection_context_storage;
    }

    rain_fn injection_context::request_a_new_singleton(const foundation::ctti::typeinfo &new_resources_type)
        -> signle_instance_storage_tuple {
        if (has_init(new_resources_type)) {
            return {};
        }
        {
            using inject_instance =
                utility::pair<foundation::memory::nebula_ptr<any>, foundation::memory::nebula_ptr<std::once_flag>>;
            auto &p = foundation::pal::threading::create_synchronized_task(injection_mutex, [&new_resources_type]() -> decltype(auto) {
                return inject_impl()
                    ->emplace(new_resources_type, inject_instance{foundation::memory::make_nebula<any>(),
                                                                  foundation::memory::make_nebula<std::once_flag>()})
                    .first->second;
            });
            return {p.first.get(), p.second.get()};
        }
    }

    rain_fn injection_context::has_init(const foundation::ctti::typeinfo &new_resources_type) -> bool {
        return inject_impl()->contains(new_resources_type);
    }

    rain_fn injection_context::get_mutex() -> std::mutex& {
        return injection_mutex;
    }
}

namespace rainy::utility::ioc {
    static factory ioc_storage_factory;

    rain_fn container::impl() noexcept -> container * {
        static container instance{&ioc_storage_factory};
        return &instance;
    }
}
