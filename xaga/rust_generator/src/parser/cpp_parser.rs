use tree_sitter::{Parser, Node};
use tree_sitter_cpp::LANGUAGE;
use crate::model::cpp_class::{CppFunction, ParseResult};
use crate::model::CppClass;
use crate::parser::extract_class_name;

pub fn parse_cpp(source: &str) -> anyhow::Result<ParseResult> {
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
    if node.kind() == "class_specifier" {
        if let Some(name) = extract_class_name(node, source) {
            let has_moc_macro = contains_macro(node, source, "RAINY_ENABLE_MOC");

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