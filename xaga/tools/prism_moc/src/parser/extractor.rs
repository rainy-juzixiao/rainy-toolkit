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

use crate::model::cpp_class::AccessLevel;
use crate::model::cpp_enumeration::{CppEnumeration, CppEnumerationItem};
use crate::model::cpp_property::CppProperty;
use crate::model::{cpp_ctor::CppCtor, cpp_function::CppFunction};
use crate::parser::node::find_param_identifier;

pub fn extract_type_name(node: Node, source: &str) -> Option<String> {
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

pub fn extract_decl_type(node: Node, source: &str) -> Option<String> {
    match node.kind() {
        "primitive_type" | "type_identifier" | "qualified_identifier" => {
            Some(node.utf8_text(source.as_bytes()).ok()?.to_string())
        }
        _ => None,
    }
}

pub fn extract_functions_and_ctors(
    node: Node,
    source: &str,
    functions: &mut Vec<CppFunction>,
    ctors: &mut Vec<CppCtor>,
) {
    // 内部递归
    fn inner_visit(
        node: Node,
        source: &str,
        functions: &mut Vec<CppFunction>,
        ctors: &mut Vec<CppCtor>,
        current_class: Option<&str>,
    ) {
        // 如果当前节点是 class_specifier 或 struct_specifier，需要特殊处理
        if node.kind() == "class_specifier" || node.kind() == "struct_specifier" {
            // 提取当前类名
            let class_name = extract_type_name(node, source);

            // 只处理直接子节点，不要深入递归到嵌套类
            for i in 0..node.child_count() {
                if let Some(child) = node.child(i as u32) {
                    // 如果是 field_declaration_list，只处理其直接成员
                    if child.kind() == "field_declaration_list" {
                        // 内联 process_field_declaration_list
                        for j in 0..child.child_count() {
                            if let Some(grandchild) = child.child(j as u32) {
                                match grandchild.kind() {
                                    "function_definition" | "declaration" => {
                                        if let Some((func, ctor)) = parse_function_or_ctor(
                                            grandchild,
                                            source,
                                            class_name.as_deref(),
                                        ) {
                                            if let Some(ctor) = ctor {
                                                ctors.push(ctor);
                                            }
                                            if let Some(func) = func {
                                                functions.push(func);
                                            }
                                        }
                                    }
                                    "field_declaration" => {
                                        if let Some((func, ctor)) = parse_function_or_ctor(
                                            grandchild,
                                            source,
                                            class_name.as_deref(),
                                        ) {
                                            if is_function_or_ctor(grandchild) {
                                                if let Some(ctor) = ctor {
                                                    ctors.push(ctor);
                                                }
                                                if let Some(func) = func {
                                                    functions.push(func);
                                                }
                                            }
                                        }
                                    }
                                    "class_specifier" | "struct_specifier" => {
                                        // 遇到嵌套类，不深入处理，让外层的 visit 函数处理
                                    }
                                    _ => {
                                        // 其他节点继续递归（比如 access_specifier 等）
                                    }
                                }
                            }
                        }
                    }
                }
            }
            return;
        }

        // 对于非类节点，继续递归
        for i in 0..node.child_count() {
            if let Some(child) = node.child(i as u32) {
                inner_visit(child, source, functions, ctors, current_class);
            }
        }
    }

    inner_visit(node, source, functions, ctors, None);
}

fn parse_function_or_ctor(
    node: Node,
    source: &str,
    current_class: Option<&str>,
) -> Option<(Option<CppFunction>, Option<CppCtor>)> {
    let mut return_type: Option<String> = None;
    let mut params = Vec::new();
    let mut is_static = false;
    let mut ctor_name: Option<String> = None;
    let mut func_name: Option<String> = None;

    for i in 0..node.child_count() {
        if let Some(child) = node.child(i as u32) {
            match child.kind() {
                "primitive_type"
                | "type_identifier"
                | "qualified_identifier"
                | "type_qualifier" => {
                    let text = child.utf8_text(source.as_bytes()).unwrap_or("");
                    return_type = Some(match return_type {
                        Some(prev) => format!("{} {}", prev, text),
                        None => text.to_string(),
                    });
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
                                    ctor_name = Some(
                                        grand
                                            .utf8_text(source.as_bytes())
                                            .unwrap_or("")
                                            .to_string(),
                                    );
                                }
                                "field_identifier" => {
                                    func_name = Some(
                                        grand
                                            .utf8_text(source.as_bytes())
                                            .unwrap_or("")
                                            .to_string(),
                                    );
                                }
                                "parameter_list" => {
                                    for k in 0..grand.child_count() {
                                        if let Some(param) = grand.child(k as u32) {
                                            if param.kind() == "parameter_declaration" {
                                                let full = param
                                                    .utf8_text(source.as_bytes())
                                                    .unwrap_or("")
                                                    .to_string();
                                                let mut ty = full.clone();
                                                if let Some(declarator) =
                                                    param.child_by_field_name("declarator")
                                                {
                                                    if let Some(id) =
                                                        find_param_identifier(declarator)
                                                    {
                                                        let start =
                                                            id.start_byte() - param.start_byte();
                                                        let end =
                                                            id.end_byte() - param.start_byte();

                                                        ty.replace_range(start..end, "");
                                                    }
                                                }
                                                params.push(ty.trim().to_string());
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
        Some((None, Some(CppCtor { params })))
    }
    // 判断普通函数：有 field_identifier 名称
    else if func_name.is_some() {
        Some((
            Some(CppFunction {
                name: func_name.unwrap(),
                full_qual_name: String::new(),
                return_type: return_type.unwrap_or_default(),
                params,
                use_namespaces: Vec::new(),
                use_items: Vec::new(),
                is_static,
                start_byte: node.start_byte(),
                end_byte: node.end_byte()
            }),
            None,
        ))
    } else {
        None
    }
}

pub fn parse_enumeration(node: Node, source: &str) -> Option<CppEnumeration> {
    let mut enum_items: Vec<CppEnumerationItem> = Vec::new();
    let mut is_enum_class: bool = false;
    let mut enum_name: String = String::new();
    for i in 0..node.child_count() {
        if let Some(child) = node.child(i as u32) {
            match child.kind() {
                "type_identifier" => {
                    enum_name = extract_type_name(node, source)?;
                }
                "class" => {
                    is_enum_class = true;
                }
                "enumerator_list" => {
                    let mut cursor = child.walk();
                    for child in child.children(&mut cursor) {
                        if child.kind() == "enumerator" {
                            if let Some(enumerator) = child.child_by_field_name("name") {
                                let item = enumerator
                                    .utf8_text(source.as_bytes())
                                    .unwrap_or("")
                                    .trim_matches('"');
                                enum_items.push(CppEnumerationItem {
                                    name: item.to_string(),
                                });
                            }
                        }
                    }
                }
                _ => {}
            }
        }
    }
    Some(CppEnumeration {
        items: enum_items,
        is_enum_class,
        namespace_location: "".to_string(),
        full_qual_name: "".to_string(),
        name: enum_name,
        use_namespaces: vec![],
        use_items: vec![],
    })
}

fn is_function_or_ctor(node: Node) -> bool {
    for i in 0..node.child_count() {
        if let Some(child) = node.child(i as u32) {
            if child.kind() == "function_declarator" {
                return true;
            }
        }
    }
    false
}

pub fn extract_global_function(node: Node, source: &str) -> Option<CppFunction> {
    let mut return_type: Option<String> = None;
    let mut params = Vec::new();
    let mut is_static = false;
    let mut func_name: Option<String> = None;

    for i in 0..node.child_count() {
        let child = node.child(i as u32)?;

        match child.kind() {
            "primitive_type" | "type_identifier" => {
                return_type = Some(child.utf8_text(source.as_bytes()).unwrap_or("").to_string());
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
                                        if let Some(typ) = param.child_by_field_name("type") {
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
        full_qual_name: String::new(),
        return_type: return_type.unwrap_or_default(),
        params,
        use_namespaces: Vec::new(),
        use_items: Vec::new(),
        is_static,
        start_byte: node.start_byte(),
        end_byte: node.end_byte()
    })
}

pub fn extract_properties(
    class_node: Node,
    source: &str,
    properties: &mut Vec<CppProperty>,
    access_level: AccessLevel,
) {
    let access_spec = match access_level {
        AccessLevel::Private => "private",
        AccessLevel::Protected => "protected",
        AccessLevel::Public => "public",
    };

    for i in 0..class_node.child_count() {
        let child = match class_node.child(i as u32) {
            Some(c) => c,
            None => continue,
        };

        // 找到 field_declaration_list，在里面查找字段
        if child.kind() == "field_declaration_list" {
            extract_from_declaration_list(child, source, properties, access_spec);
        }
    }
}

fn extract_from_declaration_list(
    list_node: Node,
    source: &str,
    properties: &mut Vec<CppProperty>,
    access_spec: &str,
) {
    let mut is_hit = false;

    for i in 0..list_node.child_count() {
        let child = match list_node.child(i as u32) {
            Some(c) => c,
            None => continue,
        };

        match child.kind() {
            "access_specifier" => {
                let text = child.utf8_text(source.as_bytes()).unwrap_or("");
                // access_specifier 可能包含冒号，比如 "public:"
                let text = text.trim_end_matches(':');
                is_hit = text == access_spec;
            }
            "field_declaration" if is_hit => {
                let mut typ: Option<String> = None;
                let mut name: Option<String> = None;
                let mut is_static = false;

                for j in 0..child.child_count() {
                    if let Some(grand) = child.child(j as u32) {
                        match grand.kind() {
                            "storage_class_specifier" => {
                                if grand.utf8_text(source.as_bytes()).unwrap_or("") == "static" {
                                    is_static = true;
                                }
                            }
                            "primitive_type" | "type_identifier" | "qualified_identifier" => {
                                typ = extract_decl_type(grand, source);
                            }
                            "field_identifier" => {
                                name = Some(
                                    grand.utf8_text(source.as_bytes()).unwrap_or("").to_string(),
                                );
                            }
                            _ => {}
                        }
                    }
                }
                if let (Some(typ), Some(name)) = (typ, name) {
                    properties.push(CppProperty {
                        name,
                        property_type: typ,
                        is_static,
                    });
                }
            }
            _ => {}
        }
    }
}

pub fn find_matching_paren(text: &str) -> Option<usize> {
    let mut depth = 0;
    for (i, ch) in text.char_indices() {
        match ch {
            '(' => depth += 1,
            ')' => {
                depth -= 1;
                if depth == 0 {
                    return Some(i);
                }
            }
            _ => {}
        }
    }
    None
}

pub fn extract_string_argument(text: &str) -> Option<String> {
    // 查找第一个左括号
    let start = text.find('(')?;
    // 查找匹配的右括号
    let end = find_matching_paren(&text[start..])?;
    let args_text = &text[start + 1..start + end];

    // 提取字符串字面量
    let trimmed = args_text.trim();
    if trimmed.starts_with('"') && trimmed.ends_with('"') {
        Some(trimmed[1..trimmed.len() - 1].to_string())
    } else {
        None
    }
}
