use tree_sitter::Node;
use crate::model::cpp_class::CppFunction;

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

pub fn extract_functions(node: Node, source: &str, functions: &mut Vec<CppFunction>) {
    if node.kind() == "function_definition" {
        let mut name = String::new();
        let mut return_type = String::new();
        let mut params = Vec::new();
        let mut is_static = false;

        for i in 0..node.child_count() {
            if let Some(child) = node.child(i as u32) {
                match child.kind() {
                    "primitive_type" | "type_identifier" => {
                        return_type = child.utf8_text(source.as_bytes()).unwrap_or("").to_string();
                    }
                    "function_declarator" => {
                        for j in 0..child.child_count() {
                            if let Some(grand) = child.child(j as u32) {
                                match grand.kind() {
                                    "field_identifier" => {
                                        name = grand.utf8_text(source.as_bytes()).unwrap_or("").to_string();
                                    }
                                    "parameter_list" => {
                                        for k in 0..grand.child_count() {
                                            if let Some(param) = grand.child(k as u32) {
                                                if param.kind() == "parameter_declaration" {
                                                    if let Some(typ) = param.child_by_field_name("type") {
                                                        params.push(typ.utf8_text(source.as_bytes()).unwrap_or("").to_string());
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
                    "storage_class_specifier" => {
                        if child.utf8_text(source.as_bytes()).unwrap_or("") == "static" {
                            is_static = true;
                        }
                    }
                    _ => {}
                }
            }
        }

        functions.push(CppFunction { name, return_type, params, is_static });
    }

    for i in 0..node.child_count() {
        if let Some(child) = node.child(i as u32) {
            extract_functions(child, source, functions);
        }
    }
}