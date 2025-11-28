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
#include <rainy/meta/reflection/type.hpp>
#include <rainy/meta/reflection/refl_impl/type_register.hpp>
#include <rainy/foundation/pal/threading.hpp>

namespace rainy::meta::reflection::implements {
    class register_table::myimpl {
    public:
        type_accessor *get_accessor(annotations::lifetime::in<foundation::ctti::typeinfo> type) {
            if (auto iter = this->data.find(type); iter != this->data.end()) {
                return iter->second;
            }
            return nullptr;
        }

        void register_type(std::string_view name, type_accessor *type) {
            // 同步块开始
            foundation::pal::threading::create_synchronized_task(this->lock, [this, &type, name]() {
                const auto &typeinfo = type->typeinfo();
#if RAINY_HAS_CXX20
                if (!this->data.contains(typeinfo))
#else
                if (this_->data.find(typeinfo) == this_->data.end())
#endif
                {
                    auto [iter, success] = this->data.emplace(typeinfo, type);
                    utility::ensures(success, "Cannot register type.");
                    this->index.emplace(name, typeinfo);
                    (void) iter;
                }
            });
            // 同步结束
        }

        type_accessor *get_accessor_by_name(const std::string_view name) {
            if (const auto idx_iter = this->index.find(name); idx_iter != this->index.end()) {
                if (const auto iter = this->data.find(idx_iter->second); iter != this->data.end()) {
                    return iter->second;
                }
            }
            return nullptr;
        }

        void unregister(std::string_view name, foundation::ctti::typeinfo ctti) {
            this->data.erase(ctti);
            this->index.erase(name);
        }

        bool has_register(annotations::lifetime::in<foundation::ctti::typeinfo> type) noexcept {
            bool has_reg = this->data.find(type) != this->data.end();
            return has_reg;
        }

    private:
        std::unordered_map<std::string_view, foundation::ctti::typeinfo> index;
        std::unordered_map<foundation::ctti::typeinfo, type_accessor *> data;
        std::mutex lock;
    };

    type_accessor *register_table::get_accessor(annotations::lifetime::in<foundation::ctti::typeinfo> type) {
        return instance().global_ptr->get_accessor(type);
    }

    void register_table::register_type(std::string_view name, type_accessor *type) {
        instance().global_ptr->register_type(name, type);
    }

    register_table &register_table::instance() {
        static register_table instance{};
        static register_table::myimpl impl;
        instance.global_ptr = &impl;
        return instance;
    }

    type_accessor *register_table::get_accessor_by_name(const std::string_view name) {
        return instance().global_ptr->get_accessor_by_name(name);
    }

    void register_table::unregister(std::string_view name, foundation::ctti::typeinfo ctti) {
        return instance().global_ptr->unregister(name, ctti);
    }

    bool register_table::has_register(annotations::lifetime::in<foundation::ctti::typeinfo> type) noexcept {
        return instance().global_ptr->has_register(type);
    }
}

namespace rainy::meta::reflection::implements {
    class injector::myimpl {
    public:
        void unregister_all() {
            std::lock_guard<std::mutex> guard(this->lock);
            for (const auto &entry: this->registered) {
                register_table::unregister(entry.name, entry.ctti);
            }
            this->registered.clear();
        }

        void register_type(std::string_view name, type_accessor *type) {
            register_table::register_type(name, type);
            std::lock_guard<std::mutex> guard(this->lock);
            this->registered.emplace_back(registration_entry{name, type->typeinfo()});
        }

    private:
        std::mutex lock;
        std::vector<registration_entry> registered;
    };

    void injector::unregister_all() {
        return instance().global_ptr->unregister_all();
    }

    void injector::register_type(std::string_view name, type_accessor *type) {
        return instance().global_ptr->register_type(name, type);
    }

    injector &injector::instance() {
        static injector inst;
        static myimpl impl;
        inst.global_ptr = &impl;
        return inst;
    }
}
