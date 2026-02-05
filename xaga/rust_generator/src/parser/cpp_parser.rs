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
    visit(root, source, &mut classes, &mut global_functions);
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
) {
    if node.kind() == "class_specifier" || node.kind() == "struct_specifier" {
        if let Some(name) = extract_class_name(node, source) {
            let has_moc_macro = contains_macro(node, source, "RAINY_ENABLE_MOC");
            if has_moc_macro {
                classes.push(CppClass {
                    name,
                    cpp_ctors: Vec::new(),
                    cpp_functions: Vec::new(),
                    start_byte: node.start_byte(),
                    end_byte: node.end_byte(),
                });
            }
        }
    }
    if node.kind() == "function_definition" && is_global_function(node) {
        if has_prev_macro(node, source, "RAINY_ENABLE_MOC") {
            if let Some(func) = parse_global_function(node, source) {
                global_functions.push(func);
            }
        }
    }
    for i in 0..node.child_count() {
        if let Some(child) = node.child(i as u32) {
            visit(child, source, classes, global_functions);
        }
    }
}

fn is_global_function(node: Node) -> bool {
    node.parent()
        .map(|p| p.kind() == "translation_unit")
        .unwrap_or(false)
}

fn contains_macro(node: Node, source: &str, macro_name: &str) -> bool {
    // 如果是 class body 内的 declaration
    if node.kind() == "declaration" {
        if let Ok(text) = node.utf8_text(source.as_bytes()) {
            if text.contains(&format!("{}()", macro_name)) {
                return true;
            }
        }
    }
    for i in 0..node.child_count() {
        if let Some(child) = node.child(i as u32) {
            if contains_macro(child, source, macro_name) {
                return true;
            }
        }
    }
    false
}

fn has_prev_macro(node: Node, source: &str, macro_name: &str) -> bool {
    let mut prev = node.prev_named_sibling();

    while let Some(p) = prev {
        match p.kind() {
            "comment" => {
                // 跳过注释
                prev = p.prev_named_sibling();
            }
            "declaration" | "expression_statement" => {
                // 可能是宏调用，检查文本内容
                if let Ok(text) = p.utf8_text(source.as_bytes()) {
                    let trimmed = text.trim();
                    // 检查是否包含宏名称
                    if trimmed.contains(macro_name) {
                        return true;
                    }
                }
                return false;
            }
            _ => {
                // 其他类型节点，停止搜索
                return false;
            }
        }
    }
    false
}