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
use crate::model::cpp_function::CppFunction;
use crate::parser::extract_class_name;
use crate::parser::extractor::parse_global_function;
use tree_sitter::{Node, Parser};
use tree_sitter_cpp::LANGUAGE;

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
    visit(
        root,
        source,
        &mut classes,
        &mut global_functions,
        Vec::new(),
    );
    Ok(ParseResult {
        tree,
        classes,
        global_functions,
    })
}

fn visit(
    node: Node,
    source: &str,
    classes: &mut Vec<CppClass>,
    global_functions: &mut Vec<CppFunction>,
    namespace_stack: Vec<String>,
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
                    new_namespace_stack.clone(),
                );
            }
        }
        return;
    }

    if node.kind() == "class_specifier" || node.kind() == "struct_specifier" {
        if let Some(name) = extract_class_name(node, source) {
            if let Some((use_namespaces, use_items)) =
                contains_macro(node, source, "RAINY_ENABLE_MOC")
            {
                let full_qual_name = if namespace_stack.is_empty() {
                    name.clone()
                } else {
                    format!("{}::{}", namespace_stack.join("::"), name)
                };

                classes.push(CppClass {
                    name,
                    full_qual_name,
                    use_namespaces,
                    namespace_dest: if namespace_stack.is_empty() {
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
        }
    }

    if node.kind() == "function_definition" && is_global_function(node) {
        if let Some((use_namespaces, use_items)) = has_prev_macro(node, source, "RAINY_ENABLE_MOC")
        {
            if let Some(mut func) = parse_global_function(node, source) {
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

    for i in 0..node.child_count() {
        if let Some(child) = node.child(i as u32) {
            visit(
                child,
                source,
                classes,
                global_functions,
                namespace_stack.clone(),
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
    // 全局函数的父节点应该是 translation_unit
    // 或者在 namespace 内的 declaration_list
    p.kind() == "translation_unit"
        || (p.kind() == "declaration_list"
            && p.parent()
                .map(|pp| pp.kind() == "namespace_definition")
                .unwrap_or(false))
}

// 如果找到宏，返回Some((use_namespaces, use_items))，否则返回None
fn contains_macro(
    node: Node,
    source: &str,
    macro_name: &str,
) -> Option<(Vec<String>, Vec<String>)> {
    // 检查当前节点及其所有子节点
    if let Ok(text) = node.utf8_text(source.as_bytes()) {
        if text.contains(macro_name) {
            // 找到了宏，解析整个节点的文本
            let (use_namespaces, use_items) = parse_macro_arguments(text);
            return Some((use_namespaces, use_items));
        }
    }

    // 递归检查子节点
    for i in 0..node.child_count() {
        if let Some(child) = node.child(i as u32) {
            if let Some(result) = contains_macro(child, source, macro_name) {
                return Some(result);
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

fn find_matching_paren(text: &str) -> Option<usize> {
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

fn extract_string_argument(text: &str) -> Option<String> {
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

#[cfg(test)]
mod tests {
    use super::*;
    use crate::cli::CommandArguments;
    use crate::include_test_set;

    fn default_cli() -> CommandArguments {
        CommandArguments {
            input: None,
            lang: None,
            verbose: false,
            tea: false,
            rain: false,
            rain_duration: 0,
            dev: false,
            out: None,
            help: false,
            version: false,
            no_cache: false,
        }
    }

    const TEST_SOURCE: &str = include_test_set!("parser/example.cc");

    #[test]
    fn test_parse_cpp_classes_with_moc() {
        let cli = default_cli();
        let result = parse_cpp(TEST_SOURCE, &cli).unwrap();

        let class_names: Vec<_> = result
            .classes
            .iter()
            .map(|c| c.full_qual_name.as_str())
            .collect();

        // Foo / Bar 没有宏，不应出现
        assert!(!class_names.contains(&"Foo"));
        assert!(!class_names.contains(&"Bar"));

        // a / b 有宏
        assert!(class_names.contains(&"a"));
        assert!(class_names.contains(&"b"));

        // namespace 中的类
        assert!(class_names.contains(&"test_namespace::nest::c"));
    }

    #[test]
    fn test_class_macro_arguments() {
        let cli = default_cli();
        let result = parse_cpp(TEST_SOURCE, &cli).unwrap();

        let class_b = result.classes.iter().find(|c| c.name == "b").unwrap();

        assert_eq!(
            class_b.use_namespaces,
            vec!["xxx".to_string(), "x1xx".to_string()]
        );

        assert_eq!(class_b.use_items, vec!["xxx".to_string()]);
    }

    #[test]
    fn test_global_functions_with_moc() {
        let cli = default_cli();
        let result = parse_cpp(TEST_SOURCE, &cli).unwrap();

        let func_names: Vec<_> = result
            .global_functions
            .iter()
            .map(|f| f.full_qual_name.as_str())
            .collect();

        assert!(func_names.contains(&"function"));
        assert!(func_names.contains(&"function1"));
        assert!(func_names.contains(&"function2"));
    }

    #[test]
    fn test_global_function_namespace_qual_name() {
        let cli = default_cli();
        let result = parse_cpp(TEST_SOURCE, &cli).unwrap();

        for f in &result.global_functions {
            // 确保 full_qual_name 构造逻辑不 panic
            assert!(!f.full_qual_name.is_empty());
        }
    }

    #[test]
    fn test_empty_moc_macro_arguments() {
        let cli = default_cli();
        let result = parse_cpp(TEST_SOURCE, &cli).unwrap();

        let class_a = result.classes.iter().find(|c| c.name == "a").unwrap();

        assert!(class_a.use_namespaces.is_empty());
        assert!(class_a.use_items.is_empty());
    }
}
