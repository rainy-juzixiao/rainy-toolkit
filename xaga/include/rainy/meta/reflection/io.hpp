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
#ifndef RAINY_META_REFLECTION_IO_HPP
#define RAINY_META_REFLECTION_IO_HPP

#include <rainy/meta/reflection/type.hpp>
#include <rainy/meta/reflection/shared_object.hpp>
#include <rainy/component/willow/json.hpp>

namespace rainy::meta::reflection::io {
    /**
     * @brief 将json对象反序列化到shared_object.
     * @param json 用于输入的json对象，必须是component::willow::json的类型
     * @param obj  要进行反序列化的shared_object对象
     */
    RAINY_TOOLKIT_API void from_json(const component::willow::json &json, meta::reflection::shared_object obj);

    /**
     * @brief 将json对象序列化到shared_object
     * @param json 用于输入的json对象，必须是component::willow::json的类型
     * @param type 要创建的shared_object的类型
     * @param args 用于创建shared_object的构造函数参数列表（如果没有默认构造则需提供）
     * @return 已反序列化完成的shared_object对象
     */
    RAINY_TOOLKIT_API meta::reflection::shared_object from_json_and_make_shared(
        const component::willow::json &json, meta::reflection::type type, collections::views::array_view<utility::any> args = {});

    /**
     * @brief 将shared_object序列化为json对象
     * @param obj 要进行序列化的shared_object对象
     * @return 已序列化完成的json对象
     */
    RAINY_TOOLKIT_API component::willow::json to_json(meta::reflection::shared_object obj);
}

#endif