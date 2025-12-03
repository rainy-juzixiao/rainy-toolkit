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
#ifndef RAINY_META_REFL_IMPL_ENUMERATION_ACCESSOR_HPP
#define RAINY_META_REFL_IMPL_ENUMERATION_ACCESSOR_HPP
#include <rainy/core/core.hpp>
#include <rainy/meta/reflection/metadata.hpp>

namespace rainy::meta::reflection::implements {
    template <typename Enum>
    class enum_data {
    public:
        enum_data() noexcept = default;

        enum_data(std::string_view name, Enum value) : name(name), value(value) {
        }

        std::string_view get_name() const {
            return name;
        }

        Enum get_value() const {
            return value;
        }

    private:
        std::string_view name{};
        Enum value{};
    };

    template <typename Enum_Type>
    implements::enum_data<Enum_Type> value(std::string_view name, Enum_Type value) {
        return {name, value};
    }
}

namespace rainy::meta::reflection::implements {
    struct enumeration_accessor {
        virtual const foundation::ctti::typeinfo &underlying_type() const noexcept = 0;
        virtual std::size_t enum_count() const noexcept = 0;
        virtual collections::views::array_view<std::string_view> names() const = 0;
        virtual collections::views::array_view<utility::any> values() const = 0;
        virtual std::string_view value_to_name(const utility::any &value) const = 0;
        virtual utility::any name_to_value(std::string_view name) const = 0;
    };

    template <typename Enum>
    struct enumeration_accessor_impl : enumeration_accessor {
        enumeration_accessor_impl(type_accessor *accessor) : accessor{accessor} {
        }

        const foundation::ctti::typeinfo &underlying_type() const noexcept override {
            return rainy_typeid(type_traits::other_trans::underlying_type_t<Enum>);
        }

        collections::views::array_view<std::string_view> names() const override {
            return names_;
        }

        collections::views::array_view<utility::any> values() const override {
            return items_;
        }

        std::size_t enum_count() const noexcept {
            return enums_.size();
        }

        std::string_view value_to_name(const utility::any &value) const override {
            Enum enum_value{};
            if (value.is<Enum>()) {
                enum_value = value.as<Enum>();
            } else if (value.is_convertible<type_traits::other_trans::underlying_type_t<Enum>>()) {
                enum_value = Enum{value.convert<type_traits::other_trans::underlying_type_t<Enum>>()};
            } else {
                return {};
            }
            std::size_t index{};
            auto it = core::algorithm::find_if(enums_.begin(), enums_.end(),
                                               [enum_value, &index](const annotations::lifetime::in<Enum> enum_item) {
                                                   bool cond = enum_item == enum_value;
                                                   return cond ? true : (++index, false);
                                               });
            if (it != enums_.end()) {
                return names_[index];
            }
            return {};
        }

        utility::any name_to_value(std::string_view name) const override {
            std::size_t index{};
            auto it = core::algorithm::find_if(names_.begin(), names_.end(), [&name, &index](const std::string_view item_name) {
                bool cond = (item_name == name);
                return cond ? true : (++index, false);
            });
            if (it != names_.end()) {
                return Enum{enums_[index]};
            }
            return {};
        }

        std::vector<std::string_view> names_;
        std::vector<Enum> enums_;
        std::vector<utility::any> items_;
        type_accessor *accessor{nullptr};
    };
}

#endif