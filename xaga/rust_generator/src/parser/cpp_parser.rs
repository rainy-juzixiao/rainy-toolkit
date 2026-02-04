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

use tree_sitter::{Parser, Node};
use tree_sitter_cpp::LANGUAGE;
use crate::cli::CommandArguments;
use crate::model::cpp_class::{CppClass, ParseResult};
use crate::parser::extract_class_name;

pub fn parse_cpp(source: &str, cli: &CommandArguments) -> anyhow::Result<ParseResult> {
    let mut parser = Parser::new();
    let language: tree_sitter::Language = LANGUAGE.into();
    parser.set_language(&language)?;
    let tree = parser.parse(source, None).unwrap();
    let root = tree.root_node();
    let mut classes = Vec::new();
    visit(root, source, &mut classes);
    Ok(ParseResult { tree, classes })
}

fn visit(node: Node, source: &str, classes: &mut Vec<CppClass>) {
    if node.kind() == "class_specifier" || node.kind() == "struct_specifier" { // class struct
        if let Some(name) = extract_class_name(node, source) {
            let has_moc_macro = contains_macro(node, source, "RAINY_ENABLE_MOC"); // 启用MOC的宏
            if has_moc_macro {
                classes.push(CppClass {
                    name,
                    has_moc_macro,
                    start_byte: node.start_byte(),
                    end_byte: node.end_byte()
                });
            }
        }
    }
    for i in 0..node.child_count() {
        if let Some(child) = node.child(i as u32) {
            visit(child, source, classes);
        }
    }
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