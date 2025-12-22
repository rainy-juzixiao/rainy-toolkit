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
#ifndef RAINY_META_REFLECTION_ENUMERATION_HPP
#define RAINY_META_REFLECTION_ENUMERATION_HPP
#include <rainy/core/core.hpp>
#include <rainy/meta/reflection/refl_impl/enumeration_accessor.hpp>

namespace rainy::meta::reflection {
    class RAINY_TOOLKIT_API enumeration {
    public:
        enumeration() noexcept = default;

        enumeration(core::internal_construct_tag_t, implements::type_accessor *type_accessor,
                    implements::enumeration_accessor *accessor) : type_accessor{type_accessor}, accessor{accessor} {
        }

        enumeration(annotations::lifetime::read_only<enumeration>) = default;

        RAINY_NODISCARD rain_fn get_names() const noexcept -> collections::views::array_view<std::string_view> {
            if (!accessor) {
                return {};
            }
            return accessor->names();
        }

        RAINY_NODISCARD rain_fn get_values() const noexcept -> collections::views::array_view<utility::any> {
            if (!accessor) {
                return {};
            }
            return accessor->values();
        }

        RAINY_NODISCARD rain_fn value_to_name(annotations::lifetime::in<utility::any> value) const noexcept -> std::string_view {
            if (!accessor) {
                return {};
            }
            return accessor->value_to_name(value);
        }

        RAINY_NODISCARD rain_fn name_to_value(const std::string_view name) const -> utility::any {
            if (!accessor) {
                return {};
            }
            return accessor->name_to_value(name);
        }

        RAINY_NODISCARD rain_fn type() const noexcept -> const foundation::ctti::typeinfo & {
            static constexpr foundation::ctti::typeinfo empty;
            if (!accessor) {
                return empty;
            }
            return type_accessor->typeinfo();
        }

        RAINY_NODISCARD rain_fn underlying_type() const noexcept -> const foundation::ctti::typeinfo & {
            static constexpr foundation::ctti::typeinfo empty;
            if (!accessor) {
                return empty;
            }
            return accessor->underlying_type();
        }

        RAINY_NODISCARD rain_fn get_name() const noexcept -> std::string_view {
            if (!accessor) {
                return {};
            }
            return type_accessor->name();
        }

        RAINY_NODISCARD rain_fn contains(const std::string_view name) const noexcept -> bool {
            if (!accessor) {
                return false;
            }
            auto names_view = accessor->names();
            return core::algorithm::find(names_view.begin(), names_view.end(), name) != names_view.end();
        }

        RAINY_NODISCARD rain_fn enum_count() const noexcept -> std::size_t {
            if (!accessor) {
                return {};
            }
            return accessor->enum_count();
        }

        friend bool operator==(const enumeration& left,const enumeration &right) noexcept {
            if (!left.type_accessor || !right.type_accessor) {
                return false;
            }
            return left.type_accessor->typeinfo() == right.type_accessor->typeinfo();
        }

        friend bool operator!=(const enumeration& left,const enumeration &right) noexcept {
            return !(left == right);
        }

    private:
        implements::type_accessor *type_accessor{nullptr};
        implements::enumeration_accessor *accessor{nullptr};
    };
}

#endif