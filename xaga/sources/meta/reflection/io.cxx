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
#include <rainy/meta/reflection/io.hpp>

namespace rainy::meta::reflection::io::implements {
    void tojson_recursively(const meta::reflection::shared_object &obj, component::willow::json &json_object);
    bool write_variant(utility::any &var, component::willow::json &json_value);
    void fromjson_recursively(meta::reflection::shared_object &obj, const component::willow::json &json_object);

    bool write_atomic_types_to_json(const foundation::ctti::typeinfo &t, const utility::any &var,
                                    component::willow::json &json_value) {
        if (t.is_arithmetic()) {
            switch (t.hash_code()) {
                case rainy_typehash(bool):
                    json_value = var.convert<bool>();
                    break;
                case rainy_typehash(char):
                    json_value = static_cast<int>(var.convert<char>());
                    break;
                case rainy_typehash(int8_t):
                    json_value = static_cast<int>(var.convert<int8_t>());
                    break;
                case rainy_typehash(int16_t):
                    json_value = var.convert<int16_t>();
                    break;
                case rainy_typehash(int32_t):
                    json_value = var.convert<int32_t>();
                    break;
                case rainy_typehash(int64_t):
                    json_value = var.convert<int64_t>();
                    break;
                case rainy_typehash(uint8_t):
                    json_value = static_cast<unsigned int>(var.convert<uint8_t>());
                    break;
                case rainy_typehash(uint16_t):
                    json_value = var.convert<uint16_t>();
                    break;
                case rainy_typehash(uint32_t):
                    json_value = var.convert<uint32_t>();
                    break;
                case rainy_typehash(uint64_t):
                    json_value = var.convert<uint64_t>();
                    break;
                case rainy_typehash(float):
                    json_value = var.convert<float>();
                    break;
                case rainy_typehash(double):
                    json_value = var.convert<double>();
                    break;
                default:
                    return false;
            }
            return true;
        } else {
            switch (t.hash_code()) {
                case rainy_typehash(std::string):
                case rainy_typehash(std::string_view):
                case rainy_typehash(const char *):
                    json_value = var.convert<std::string>();
                    return true;
                default:
                    break;
            }
        }
        return false;
    }

    void write_array(const utility::any &view, component::willow::json &json_array) {
        json_array = component::willow::json::array({});

        const std::size_t size = view.size();
        for (std::size_t i = 0; i < size; ++i) {
            utility::any item = view[i];
            component::willow::json json_item;
            if (item.type().is_sequential_container()) {
                write_array(item, json_item);
            } else {
                // 获取包装后的值
                utility::any wrapped_var = item;
                foundation::ctti::typeinfo value_type = wrapped_var.type().remove_cvref();

                if (value_type.is_arithmetic() || value_type == rainy_typeid(std::string) || value_type.has_traits(foundation::ctti::traits::is_enum)) {
                    write_atomic_types_to_json(value_type, wrapped_var, json_item);
                } else { // object
                    shared_object temp_obj(utility::move(wrapped_var));
                    json_item = component::willow::json::object({});
                    tojson_recursively(temp_obj, json_item);
                }
            }

            json_array.push_back(json_item);
        }
    }

    void write_associative_container(const utility::any &view, component::willow::json &json_array) {
        json_array = component::willow::json::array({});
        const foundation::ctti::typeinfo key_type = view.inner_decleartion_type(utility::any_inner_declaertion::key_type);
        const foundation::ctti::typeinfo value_type = view.inner_decleartion_type(utility::any_inner_declaertion::value_type);
        if (value_type != rainy_typeid(void)) {
            // key-value 类型 (如 map)
            for (auto it = view.begin(); it != view.end(); ++it) {
                component::willow::json json_obj = component::willow::json::object({});
                component::willow::json key_json;
                component::willow::json value_json;
                utility::any key_var = it.key();
                utility::any value_var = it.value();
                if (!write_variant(key_var, key_json)) {
                    continue;
                }
                json_obj["key"] = key_json;
                if (!write_variant(value_var, value_json)) {
                    continue;
                }
                json_obj["value"] = value_json;
                json_array.push_back(json_obj);
            }
        } else {
            for (auto it = view.begin(); it != view.end(); ++it) {
                component::willow::json key_json;
                utility::any key_var = it.key();

                if (write_variant(key_var, key_json)) {
                    json_array.push_back(key_json);
                }
            }
        }
    }

    bool write_variant(utility::any &var, component::willow::json &json_value) {
        auto value_type = var.type().remove_cvref();
        if (write_atomic_types_to_json(value_type, var, json_value)) {
            return true;
        } else if (var.type().is_sequential_container()) {
            write_array(var, json_value);
            return true;
        } else if (var.type().is_associative_container()) {
            write_associative_container(var, json_value);
            return true;
        } else {
            // 尝试作为对象处理
            auto prop_list = meta::reflection::type::get_by_typeinfo(value_type).get_properties();
            if (!prop_list.empty()) {
                json_value = component::willow::json::object({});
                shared_object temp_obj(utility::move(var));
                tojson_recursively(temp_obj, json_value);
                return true;
            } 
            return false;
        }
    }

    void tojson_recursively(const meta::reflection::shared_object &obj, component::willow::json &json_object) {
        if (!json_object.is_object()) {
            json_object = component::willow::json::object({});
        }
        auto prop_list = obj.get_properties();
        for (const auto &prop: prop_list) {
            utility::any prop_value = prop(obj);
            if (!prop_value.has_value()) {
                continue;
            }
            const auto name = prop.get_name();
            component::willow::json json_prop_value;
            if (!write_variant(prop_value, json_prop_value)) {
                continue;
            }
            json_object[name.data()] = json_prop_value;
        }
    }

    utility::any extract_basic_types(const component::willow::json &json_value) {
        using component::willow::json_type;
        utility::any var;
        switch (json_value.type()) {
            case json_type::string:
                var = json_value.as_string_view();
                break;
            case json_type::boolean:
                var = json_value.as_bool(); // NOLINT
                break;
            case json_type::number_integer:
                var = json_value.as_integer();
                break;
            case json_type::number_float:
                var = json_value.as_float();
                break;
            case json_type::null:
            case json_type::object:
            case json_type::array:
                break;
        }
        return var;
    }

    void write_array_recursively(meta::reflection::shared_object &obj, utility::any &view,
                                 const component::willow::json &json_array_value) { // NOLINT
        // NOLINTBEGIN
        view.resize(json_array_value.size());
        const foundation::ctti::typeinfo array_value_type = view[0].type().remove_cvref();
        for (std::size_t i = 0; i < json_array_value.size(); ++i) {
            if (auto &json_index_value = json_array_value[i]; json_index_value.is_array()) {
                auto sub_array_view = view[0].construct_from_this();
                write_array_recursively(obj, sub_array_view, json_index_value);
            } else if (json_index_value.is_object()) {
                utility::any var = view[i].construct_from_this();
                shared_object object{utility::move(var)};
                fromjson_recursively(object, json_index_value);
                view[i] = var;
            } else {
                utility::any extracted_value = extract_basic_types(json_index_value);
                if (extracted_value.is_convertible(array_value_type)) {
                    view[i] = extracted_value;
                }
            }
        }
        // NOLINTEND
    }

    utility::any extract_value(component::willow::json::const_iterator &itr, const meta::reflection::type &t) {
        auto &json_value = itr->value();
        utility::any extracted_value = extract_basic_types(json_value);
        if (const bool could_convert = extracted_value.is_convertible(t.get_typeinfo()); !could_convert) {
            if (json_value.is_object()) {
                meta::reflection::constructor ctor = t.get_constructor();
                for (auto &item: t.get_constructors()) {
                    if (item.instantiated_type() == t.get_typeinfo())
                        ctor = item;
                }
                extracted_value = ctor.invoke();
                shared_object tmp_object(utility::move(extracted_value));
                fromjson_recursively(tmp_object, json_value);
            }
        }

        return extracted_value;
    }

    void write_associative_view_recursively(utility::any &view, const component::willow::json &json_array_value) {
        for (std::size_t i = 0; i < json_array_value.size(); ++i) {
            auto &json_index_value = json_array_value[i];
            if (json_index_value.is_object())  { // a key-value associative view
                auto key_itr = json_index_value.find("key");
                auto value_itr = json_index_value.find("value");
                if (key_itr != json_index_value.cend() && value_itr != json_index_value.cend()) {
                    auto key_var = extract_value(key_itr, meta::reflection::type::get_by_typeinfo(
                                                              view.inner_decleartion_type(utility::any_inner_declaertion::key_type)));
                    auto value_var = extract_value(value_itr, meta::reflection::type::get_by_typeinfo(view.inner_decleartion_type(
                                                                  utility::any_inner_declaertion::value_type)));
                    if (key_var.has_value() && value_var.has_value()) {
                        view.insert_as_maplike(key_var, value_var);
                    }
                }
            } else {
                utility::any extracted_value = extract_basic_types(json_index_value);
                if (extracted_value.has_value() &&
                    extracted_value.is_convertible(view.inner_decleartion_type(utility::any_inner_declaertion::key_type))) {
                    view.insert_as_maplike(extracted_value);
                }
            }
        }
    }

    void fromjson_recursively(meta::reflection::shared_object &obj, const component::willow::json &json_object) {
        const auto prop_list = obj.get_properties();
        for (const auto &prop: prop_list) {
            component::willow::json::const_iterator ret = json_object.find(prop.get_name());
            if (ret == json_object.end()) {
                continue;
            }
            const foundation::ctti::typeinfo value_t = prop.field_ctti_type();
            auto &json_value = ret->value();
            switch (json_value.type()) {
                case component::willow::json_type::array: {
                    utility::any var;
                    if (value_t.is_sequential_container()) {
                        var = prop(obj);
                        write_array_recursively(obj, var, json_value);
                    } else if (value_t.is_associative_container()) {
                        var = prop(obj);
                        write_associative_view_recursively(var, json_value);
                    }
                    if (var.has_value()) {
                        prop(obj) = var;
                    }
                    break;
                }
                case component::willow::json_type::object: {
                    meta::reflection::shared_object temp_object(prop(obj));
                    fromjson_recursively(temp_object, json_value);
                    prop.set_value(obj, temp_object.target());
                    break;
                }
                default: {
                    utility::any extracted_value = extract_basic_types(json_value);
                    if (extracted_value.is_convertible(value_t)) {
                        prop(obj) = extracted_value;
                    }
                }
            }
        }
    }
}

namespace rainy::meta::reflection::io {
    component::willow::json to_json(meta::reflection::shared_object obj) {
        component::willow::json json_object = component::willow::json::object({});
        implements::tojson_recursively(obj, json_object);
        return json_object;
    }

    void from_json(const component::willow::json &json, meta::reflection::shared_object obj) {
        implements::fromjson_recursively(obj, json);   
    }

    meta::reflection::shared_object from_json_and_make_shared(const component::willow::json &json, meta::reflection::type type,
                                              collections::views::array_view<utility::any> args) {
        auto object = type.create_object(args);
        implements::fromjson_recursively(object, json);
        return object;
    }
}
