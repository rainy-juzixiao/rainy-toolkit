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
#ifndef RAINY_META_REFLECTION_METADATA_HPP
#define RAINY_META_REFLECTION_METADATA_HPP
#include <rainy/core/core.hpp>
#include <rainy/utility/any.hpp>

namespace rainy::meta::reflection {
    class metadata {
    public:
        metadata() = default;

        metadata(metadata &&right) noexcept : key_{utility::move(right.key_)}, value_{utility::move(right.value_)} {
        }

        metadata(const metadata &right) = default;

        metadata &operator=(const metadata &) = default;
        metadata &operator=(metadata &&) = default;

        template <typename Ty1, typename Ty2>
        metadata(Ty1 &&key, Ty2 &&value) : value_(utility::forward<Ty2>(value)) {
            if (type_traits::type_relations::is_same_v<std::string_view, Ty1>) {
                this->key_.emplace<std::string_view>(utility::forward<Ty1>(key));
            } else if constexpr (type_traits::type_properties::is_constructible_v<std::string, Ty1>) {
                this->key_.emplace<std::string>(utility::forward<Ty1>(key));
            } else {
                this->key_.emplace<Ty1>(key);
            }
        }

        RAINY_NODISCARD const utility::any &key() const noexcept {
            return key_;
        }

        RAINY_NODISCARD const utility::any &value() const noexcept {
            return value_;
        }

    private:
        utility::any key_;
        utility::any value_;
    };
}

namespace rainy::meta::reflection::implements {
    template <typename... Args>
    RAINY_INLINE_CONSTEXPR std::size_t metadata_count =
        type_traits::other_trans::count_type_v<metadata, type_traits::other_trans::type_list<Args...>>;

    RAINY_INLINE const metadata &find_metadata(const std::vector<metadata> &view, const utility::any &key) {
        static const metadata empty;
        const auto it = core::algorithm::find_if(view.begin(), view.end(), [&key](const metadata &meta) { return meta.key() == key; });
        return it != view.end() ? *it : empty;
    }
}

#endif