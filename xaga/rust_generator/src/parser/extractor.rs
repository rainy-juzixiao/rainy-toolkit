// Copyright 2026 rainy-juzixiao
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

use tree_sitter::Node;

use crate::model::{cpp_ctor::CppCtor, cpp_function::CppFunction};

pub fn extract_class_name(node: Node, source: &str) -> Option<String> {
    for i in 0..node.child_count() {
        let child = node.child(i as u32)?;
        if child.kind() == "type_identifier" {
            return child
                .utf8_text(source.as_bytes())
                .ok()
                .map(|s| s.to_string());
        }
    }
    None
}

pub fn extract_functions_and_ctors(
    node: Node,
    source: &str,
    functions: &mut Vec<CppFunction>,
    ctors: &mut Vec<CppCtor>,
) {
    fn helper(
        node: Node,
        source: &str,
        functions: &mut Vec<CppFunction>,
        ctors: &mut Vec<CppCtor>,
        current_class: Option<&str>,
    ) {
        // 如果当前节点是 class_specifier，提取类名用于子节点
        let class_for_children = if node.kind() == "class_specifier" {
            extract_class_name(node, source)
        } else {
            current_class.map(|s| s.to_string())
        };

        if node.kind() == "function_definition" {
            let mut return_type: Option<String> = None;
            let mut params = Vec::new();
            let mut is_static = false;
            let mut ctor_name: Option<String> = None;
            let mut func_name: Option<String> = None;

            for i in 0..node.child_count() {
                if let Some(child) = node.child(i as u32) {
                    match child.kind() {
                        "primitive_type" | "type_identifier" => {
                            return_type =
                                Some(child.utf8_text(source.as_bytes()).unwrap_or("").to_string());
                        }
                        "storage_class_specifier" => {
                            if child.utf8_text(source.as_bytes()).unwrap_or("") == "static" {
                                is_static = true;
                            }
                        }
                        "function_declarator" => {
                            for j in 0..child.child_count() {
                                if let Some(grand) = child.child(j as u32) {
                                    match grand.kind() {
                                        "identifier" => {
                                            ctor_name = Some(grand.utf8_text(source.as_bytes()).unwrap_or("").to_string());
                                        }
                                        "field_identifier" => {
                                            func_name = Some(grand.utf8_text(source.as_bytes()).unwrap_or("").to_string());
                                        }
                                        "parameter_list" => {
                                            for k in 0..grand.child_count() {
                                                if let Some(param) = grand.child(k as u32) {
                                                    if param.kind() == "parameter_declaration" {
                                                        if let Some(typ) = param.child_by_field_name("type") {
                                                            params.push(
                                                                typ.utf8_text(source.as_bytes()).unwrap_or("").to_string(),
                                                            );
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                        _ => {}
                                    }
                                }
                            }
                        }
                        _ => {}
                    }
                }
            }

            // 判断构造函数：没有返回类型 && ctor_name == current_class
            if return_type.is_none() && ctor_name.as_deref() == current_class {
                ctors.push(CppCtor {
                    params,
                });
            }
            // 判断普通函数：有 field_identifier 名称
            else if func_name.is_some() {
                functions.push(CppFunction {
                    name: func_name.unwrap(),
                    return_type: return_type.unwrap_or_default(),
                    params,
                    is_static,
                });
            }
        }

        // 遍历子节点，使用当前确定的类名
        for i in 0..node.child_count() {
            if let Some(child) = node.child(i as u32) {
                helper(child, source, functions, ctors, class_for_children.as_deref());
            }
        }
    }

    helper(node, source, functions, ctors, None);
}

pub fn parse_global_function(node: Node, source: &str) -> Option<CppFunction> {
    let mut return_type: Option<String> = None;
    let mut params = Vec::new();
    let mut is_static = false;
    let mut func_name: Option<String> = None;

    for i in 0..node.child_count() {
        let child = node.child(i as u32)?;

        match child.kind() {
            "primitive_type" | "type_identifier" => {
                return_type =
                    Some(child.utf8_text(source.as_bytes()).unwrap_or("").to_string());
            }
            "storage_class_specifier" => {
                if child.utf8_text(source.as_bytes()).unwrap_or("") == "static" {
                    is_static = true;
                }
            }
            "function_declarator" => {
                for j in 0..child.child_count() {
                    let grand = child.child(j as u32)?;

                    match grand.kind() {
                        "identifier" | "field_identifier" => {
                            func_name =
                                Some(grand.utf8_text(source.as_bytes()).unwrap_or("").to_string());
                        }
                        "parameter_list" => {
                            for k in 0..grand.child_count() {
                                if let Some(param) = grand.child(k as u32) {
                                    if param.kind() == "parameter_declaration" {
                                        if let Some(typ) =
                                            param.child_by_field_name("type")
                                        {
                                            params.push(
                                                typ.utf8_text(source.as_bytes())
                                                    .unwrap_or("")
                                                    .to_string(),
                                            );
                                        }
                                    }
                                }
                            }
                        }
                        _ => {}
                    }
                }
            }
            _ => {}
        }
    }

    Some(CppFunction {
        name: func_name?,
        return_type: return_type.unwrap_or_default(),
        params,
        is_static,
    })
}
