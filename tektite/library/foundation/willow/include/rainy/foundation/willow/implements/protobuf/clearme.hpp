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
#ifndef RAINY_FOUNDATION_WILLOW_IMPLEMENTS_PROTOBUF_CLEARME_HPP
#define RAINY_FOUNDATION_WILLOW_IMPLEMENTS_PROTOBUF_CLEARME_HPP
#include <rainy/foundation/willow/implements/protobuf/config.hpp>
#include <rainy/foundation/willow/implements/protobuf/direct.hpp>
#include <rainy/foundation/willow/implements/protobuf/unknown.hpp>
#include <rainy/foundation/willow/implements/protobuf/version.hpp>
#include <cstddef>
#include <type_traits>

#if RAINY_WILLOW_PROTOBUF_AVAILABLE

namespace rainy::foundation::willow::protobuf {
    template <typename Concept>
    class add_clearme {
    public:
        void clearme() {
            clear_all(static_cast<Concept &>(*this),
                      type_traits::helper::make_index_sequence<field_count_v<Concept>>{});
            if constexpr (type_traits::type_relations::is_base_of_v<add_unknown_fields<Concept>, Concept>) {
                static_cast<Concept &>(*this).unknown().clear();
            }
        }

    private:
        template <std::size_t... Is>
        static void clear_all(Concept &obj, type_traits::helper::index_sequence<Is...>) {
            (clear_one<Is>(obj), ...);
        }

        template <std::size_t Index>
        static void clear_one(Concept &obj) {
            using field_type = direct_field_at_t<Concept, Index>;
            auto &slot = obj.*(field_type::member);
            using raw = type_traits::modifers::remove_cvref_t<decltype(slot)>;
            slot = raw{};
        }
    };
}

#endif

#endif
