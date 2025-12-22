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
#ifndef RAINY_META_REFLECTION_FUNDMENTAL_HPP
#define RAINY_META_REFLECTION_FUNDMENTAL_HPP
#include <rainy/core/core.hpp>
#include <rainy/meta/reflection/refl_impl/fundmental_accessor.hpp>
#include <rainy/meta/reflection/refl_impl/type_register.hpp>
#include <rainy/utility/any.hpp>

namespace rainy::meta::reflection {
    class shared_object;
    class type;
}

namespace rainy::meta::reflection {
    class fundmental {
    public:
        fundmental() noexcept = default;

        fundmental(core::internal_construct_tag_t, implements::type_accessor *accessor,
                   implements::fundmental_type_accessor *fundmental_accessor) :
            accessor{accessor}, fundmental_accessor{fundmental_accessor} {
        }

        template <typename SharedObject = shared_object, typename Type = type>
        SharedObject create() {
            auto this_fund_t = Type(core::internal_construct_tag, accessor);
            return this_fund_t.create();
        }

        template <typename ValueType, typename SharedObject = shared_object, typename Type = type>
        SharedObject create(ValueType&& value) {
            auto this_fund_t = Type(core::internal_construct_tag, accessor);
            return this_fund_t.create(value);
        }

        RAINY_NODISCARD rain_fn is_valid() const noexcept -> bool {
            return accessor && fundmental_accessor;
        }

        RAINY_NODISCARD rain_fn type() const noexcept -> annotations::lifetime::read_only<foundation::ctti::typeinfo> {
            static constexpr foundation::ctti::typeinfo empty;
            if (!accessor) {
                return empty;
            }
            return fundmental_accessor->typeinfo();
        }

        RAINY_NODISCARD rain_fn get_metadata(const utility::any &key) const noexcept -> annotations::lifetime::read_only<metadata> {
            if (!is_valid()) {
                static metadata empty;
                return empty;
            }
            return implements::find_metadata(accessor->metadatas(), key);
        }

        RAINY_NODISCARD rain_fn get_metadatas() const noexcept -> collections::views::array_view<metadata> {
            if (!is_valid()) {
                return {};
            }
            return accessor->metadatas();
        }

    private:
        implements::type_accessor *accessor{nullptr};
        implements::fundmental_type_accessor *fundmental_accessor{nullptr};
    };
}

#endif