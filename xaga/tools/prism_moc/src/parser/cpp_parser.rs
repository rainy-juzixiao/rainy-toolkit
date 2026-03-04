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

use crate::cli::CommandArguments;
use crate::model::cpp_class::{CppClass, ParseResult};
use crate::model::cpp_enumeration::CppEnumeration;
use crate::model::cpp_function::CppFunction;
use crate::parser::extract_type_name;
use crate::parser::extractor::{extract_global_function, extract_string_argument, find_matching_paren, parse_enumeration};
use tree_sitter::{Node, Parser};
use tree_sitter_cpp::LANGUAGE;

/// 解析给定的 C++ 源代码字符串，并返回解析结果。
///
/// # 参数
/// - `source`: 待解析的 C++ 源代码字符串
/// - `cli`: 命令行参数结构体 `CommandArguments`，用于控制解析行为（如 verbose 输出）
///
/// # 返回
/// 返回 `anyhow::Result<ParseResult>`：
/// - `Ok(ParseResult)` 包含：
///   - `tree`: 解析生成的语法树
///   - `classes`: 在源代码中找到的所有类信息
///   - `global_functions`: 所有全局函数信息
///   - `global_enumerations`: 所有全局枚举信息
/// - `Err`: 如果设置语言或解析过程中出现错误
///
pub fn parse_cpp(source: &str, cli: &CommandArguments) -> anyhow::Result<ParseResult> {
    let mut parser = Parser::new();
    let language: tree_sitter::Language = LANGUAGE.into();
    if cli.verbose {
        println!("Setting C++ language to parse");
    }
    parser.set_language(&language)?;
    let tree = parser.parse(source, None).unwrap();
    let root = tree.root_node();
    let mut classes = Vec::new();
    let mut global_functions = Vec::new();
    let mut global_enumerations = Vec::new();
    visit(
        root,
        source,
        &mut classes,
        &mut global_functions,
        &mut global_enumerations,
        Vec::new(),
        Vec::new(),
    );
    Ok(ParseResult {
        tree,
        classes,
        global_functions,
        global_enumerations,
    })
}

fn visit(
    node: Node,
    source: &str,
    classes: &mut Vec<CppClass>,
    global_functions: &mut Vec<CppFunction>,
    global_enumerations: &mut Vec<CppEnumeration>,
    namespace_stack: Vec<String>,
    parent_class_stack: Vec<String>,
) {
    // 处理namespace定义
    if node.kind() == "namespace_definition" {
        let mut new_namespace_stack = namespace_stack.clone();
        // 提取namespace名称
        if let Some(name_node) = node.child_by_field_name("name") {
            if let Ok(ns_name) = name_node.utf8_text(source.as_bytes()) {
                new_namespace_stack.push(ns_name.to_string());
            }
        }
        // 递归处理namespace内部
        for i in 0..node.child_count() {
            if let Some(child) = node.child(i as u32) {
                visit(
                    child,
                    source,
                    classes,
                    global_functions,
                    global_enumerations,
                    new_namespace_stack.clone(),
                    parent_class_stack.clone(),
                );
            }
        }
        return;
    }
    // 处理类/结构体（包括嵌套类）
    if node.kind() == "class_specifier" || node.kind() == "struct_specifier" {
        if let Some(name) = extract_type_name(node, source) {
            if let Some((use_namespaces, use_items)) =
                contains_macro(node, source, "RAINY_ENABLE_MOC")
            {
                // 构建完整的类名
                let mut qual_name_parts = Vec::new();
                // 添加namespace
                if !namespace_stack.is_empty() {
                    qual_name_parts.push(namespace_stack.join("::"));
                }

                // 添加父类
                if !parent_class_stack.is_empty() {
                    qual_name_parts.push(parent_class_stack.join("::"));
                }
                // 添加当前类名
                qual_name_parts.push(name.clone());

                let full_qual_name = qual_name_parts.join("::");

                classes.push(CppClass {
                    name: name.clone(),
                    full_qual_name,
                    use_namespaces,
                    namespace_location: if namespace_stack.is_empty() {
                        String::new()
                    } else {
                        namespace_stack.join("::")
                    },
                    use_items,
                    cpp_ctors: Vec::new(),
                    cpp_functions: Vec::new(),
                    cpp_public_properties: Vec::new(),
                    start_byte: node.start_byte(),
                    end_byte: node.end_byte(),
                });
            }

            // 无论是否有宏，都要递归处理嵌套内容以维护 parent_class_stack
            let mut new_parent_class_stack = parent_class_stack.clone();
            new_parent_class_stack.push(name);

            for i in 0..node.child_count() {
                if let Some(child) = node.child(i as u32) {
                    visit(
                        child,
                        source,
                        classes,
                        global_functions,
                        global_enumerations,
                        namespace_stack.clone(),
                        new_parent_class_stack.clone(),
                    );
                }
            }
            return;
        }
    }

    // 处理 field_declaration 中的枚举（类内枚举）
    if node.kind() == "field_declaration" {
        // 检查是否包含 enum_specifier
        let mut has_enum = false;
        for i in 0..node.child_count() {
            if let Some(child) = node.child(i as u32) {
                if child.kind() == "enum_specifier" {
                    has_enum = true;
                    // 检查 field_declaration 的前一个兄弟节点是否有宏
                    if let Some((use_namespaces, use_items)) =
                        has_prev_macro(node, source, "RAINY_ENABLE_MOC")
                    {
                        if let Some(name) = extract_type_name(child, source) {
                            if let Some(mut enumeration) = parse_enumeration(child, source) {
                                let mut qual_name_parts = Vec::new();

                                // 添加namespace
                                if !namespace_stack.is_empty() {
                                    qual_name_parts.push(namespace_stack.join("::"));
                                }

                                // 添加父类
                                if !parent_class_stack.is_empty() {
                                    qual_name_parts.push(parent_class_stack.join("::"));
                                }
                                // 添加当前枚举名
                                qual_name_parts.push(name.clone());

                                let full_qual_name = qual_name_parts.join("::");
                                enumeration.full_qual_name = full_qual_name;
                                enumeration.use_namespaces = use_namespaces;
                                enumeration.use_items = use_items;
                                global_enumerations.push(enumeration);
                            }
                        }
                    }
                    break;
                }
            }
        }

        // 如果包含枚举，处理完后返回；否则继续递归处理可能的嵌套类
        if has_enum {
            return;
        }
        // 继续往下，让通用递归处理 field_declaration 中的其他内容（如嵌套类）
    }

    // 处理全局/namespace级别的枚举（不在 field_declaration 内的）
    if node.kind() == "enum_specifier" {
        // 检查父节点，如果是 field_declaration 则跳过（已在上面处理）
        if let Some(parent) = node.parent() {
            if parent.kind() == "field_declaration" {
                return;
            }
        }

        // 枚举用 has_prev_macro
        if let Some((use_namespaces, use_items)) = has_prev_macro(node, source, "RAINY_ENABLE_MOC")
        {
            if let Some(name) = extract_type_name(node, source) {
                if let Some(mut enumeration) = parse_enumeration(node, source) {
                    let mut qual_name_parts = Vec::new();

                    // 添加namespace
                    if !namespace_stack.is_empty() {
                        qual_name_parts.push(namespace_stack.join("::"));
                    }

                    // 添加父类
                    if !parent_class_stack.is_empty() {
                        qual_name_parts.push(parent_class_stack.join("::"));
                    }
                    // 添加当前枚举名
                    qual_name_parts.push(name.clone());

                    let full_qual_name = qual_name_parts.join("::");
                    enumeration.full_qual_name = full_qual_name;
                    enumeration.use_namespaces = use_namespaces;
                    enumeration.use_items = use_items;
                    global_enumerations.push(enumeration);
                }
            }
        }
        return;
    }
    // 处理全局函数
    // function_definition -> 定义
    // declaration -> 函数声明
    if (node.kind() == "function_definition" || node.kind() == "declaration")
        && is_global_function(node)
    {
        // 函数用 has_prev_macro
        if let Some((use_namespaces, use_items)) = has_prev_macro(node, source, "RAINY_ENABLE_MOC")
        {
            if let Some(mut func) = extract_global_function(node, source) {
                func.use_namespaces = use_namespaces;
                func.use_items = use_items;
                func.full_qual_name = if namespace_stack.is_empty() {
                    func.name.clone()
                } else {
                    format!("{}::{}", namespace_stack.join("::"), func.name)
                };
                global_functions.push(func);
            }
        }
    }
    // 递归处理其他节点
    for i in 0..node.child_count() {
        if let Some(child) = node.child(i as u32) {
            visit(
                child,
                source,
                classes,
                global_functions,
                global_enumerations,
                namespace_stack.clone(),
                parent_class_stack.clone(),
            );
        }
    }
}

fn is_global_function(node: Node) -> bool {
    let parent = node.parent();
    if parent.is_none() {
        return false;
    }
    let p = parent.unwrap();
    if p.kind() == "translation_unit" {
        return true;
    }
    if p.kind() == "declaration_list" {
        if let Some(pp) = p.parent() {
            if pp.kind() == "namespace_definition" {
                return true;
            }
        }
    }
    if p.kind().starts_with("preproc_") {
        if let Some(pp) = p.parent() {
            if pp.kind() == "translation_unit" {
                return true;
            }
            if pp.kind() == "declaration_list" {
                if let Some(ppp) = pp.parent() {
                    if ppp.kind() == "namespace_definition" {
                        return true;
                    }
                }
            }
        }
    }
    false
}

fn contains_macro(
    node: Node,
    source: &str,
    macro_name: &str,
) -> Option<(Vec<String>, Vec<String>)> {
    for i in 0..node.child_count() {
        if let Some(child) = node.child(i as u32) {
            if child.kind() == "field_declaration_list" {
                for j in 0..child.child_count() {
                    if let Some(grandchild) = child.child(j as u32) {
                        if grandchild.kind() == "field_declaration" {
                            let mut has_nested_class = false;
                            for k in 0..grandchild.child_count() {
                                if let Some(ggchild) = grandchild.child(k as u32) {
                                    if ggchild.kind() == "class_specifier"
                                        || ggchild.kind() == "struct_specifier"
                                    {
                                        has_nested_class = true;
                                        break;
                                    }
                                }
                            }
                            if has_nested_class {
                                continue;
                            }
                        }

                        if let Ok(text) = grandchild.utf8_text(source.as_bytes()) {
                            // 判断宏是否被注释掉
                            let trimmed = text.trim_start();
                            let is_commented =
                                trimmed.starts_with("//") || trimmed.starts_with("/*");
                            if is_commented {
                                // 宏被注释掉，跳过这个节点
                                continue;
                            }

                            if text.contains(macro_name) {
                                let (use_namespaces, use_items) = parse_macro_arguments(text);
                                return Some((use_namespaces, use_items));
                            }
                        }
                    }
                }
                return None;
            }
        }
    }
    None
}

// 如果找到宏，返回Some((use_namespaces, use_items))，否则返回None
fn has_prev_macro(
    node: Node,
    source: &str,
    macro_name: &str,
) -> Option<(Vec<String>, Vec<String>)> {
    let mut prev = node.prev_named_sibling();

    while let Some(p) = prev {
        match p.kind() {
            "comment" => {
                prev = p.prev_named_sibling();
            }
            "declaration" | "expression_statement" => {
                if let Ok(text) = p.utf8_text(source.as_bytes()) {
                    let trimmed = text.trim();
                    if trimmed.contains(macro_name) {
                        let (use_namespaces, use_items) = parse_macro_arguments(trimmed);
                        return Some((use_namespaces, use_items));
                    }
                }
                return None;
            }
            _ => {
                return None;
            }
        }
    }

    None
}

fn parse_macro_arguments(text: &str) -> (Vec<String>, Vec<String>) {
    let mut use_namespaces = Vec::new();
    let mut use_items = Vec::new();
    // 首先检查是否存在 RAINY_ENABLE_MOC
    if !text.contains("RAINY_ENABLE_MOC") {
        return (use_namespaces, use_items);
    }
    // 查找 RAINY_ENABLE_MOC 后的括号内容
    if let Some(moc_pos) = text.find("RAINY_ENABLE_MOC") {
        let after_moc = &text[moc_pos + "RAINY_ENABLE_MOC".len()..];

        // 查找括号范围
        if let Some(open_paren) = after_moc.find('(') {
            if let Some(close_paren) = find_matching_paren(&after_moc[open_paren..]) {
                let macro_content = &after_moc[open_paren + 1..open_paren + close_paren];

                // 在宏内容中查找 RAINY_GRANT_ACCESS_NAMESPACE
                let mut search_start = 0;
                while let Some(pos) =
                    macro_content[search_start..].find("RAINY_GRANT_ACCESS_NAMESPACE")
                {
                    let abs_pos = search_start + pos;
                    if let Some(arg) = extract_string_argument(&macro_content[abs_pos..]) {
                        use_namespaces.push(arg);
                    } else {
                        panic!("RAINY_GRANT_ACCESS_NAMESPACE must have a string literal argument");
                    }
                    search_start = abs_pos + "RAINY_GRANT_ACCESS_NAMESPACE".len();
                }

                // 在宏内容中查找 RAINY_GRANT_ACCESS_USING
                search_start = 0;
                while let Some(pos) = macro_content[search_start..].find("RAINY_GRANT_ACCESS_USING")
                {
                    let abs_pos = search_start + pos;
                    if let Some(arg) = extract_string_argument(&macro_content[abs_pos..]) {
                        use_items.push(arg);
                    } else {
                        panic!("RAINY_GRANT_ACCESS_USING must have a string literal argument");
                    }
                    search_start = abs_pos + "RAINY_GRANT_ACCESS_USING".len();
                }
            }
        }
    }
    (use_namespaces, use_items)
}
