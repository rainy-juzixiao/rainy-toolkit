#include <rainy/meta/reflection/registration.hpp>

namespace rainy::meta::reflection::implements {
    register_table &register_table::instance() {
        static register_table instance;
        return instance;
    }

    type_accessor *register_table::get_accessor_by_name(const std::string_view name) {
        auto *this_ = &instance();
        if (const auto idx_iter = this_->index.find(name); idx_iter != this_->index.end()) {
            if (const auto iter = this_->data.find(idx_iter->second); iter != this_->data.end()) {
                return iter->second;
            }
        }
        return nullptr;
    }
}

namespace rainy::meta::reflection {
    type type::get_by_name(const std::string_view name) noexcept{
        type instance{};
        instance.accessor = implements::register_table::get_accessor_by_name(name);
        return instance;
    }
}