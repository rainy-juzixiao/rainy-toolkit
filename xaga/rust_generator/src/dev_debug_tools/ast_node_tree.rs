use tree_sitter::Node;

pub fn print_node_tree(node: Node, source: &str, show_source: bool) {
    let mut cursor = node.walk();
    print_node_tree_recursive(&node, source, "", show_source, &mut cursor);
}

fn print_node_tree_recursive(
    node: &Node,
    source: &str,
    prefix: &str,
    show_source: bool,
    cursor: &mut tree_sitter::TreeCursor,
) {
    let child_count = node.child_count();
    for i in 0..child_count {
        if let Some(child) = node.child(i as u32) {
            let is_last = i == child_count - 1;

            let branch = if is_last { "└── " } else { "├── " };
            let next_prefix = if is_last {
                format!("{}    ", prefix)
            } else {
                format!("{}│   ", prefix)
            };

            let style = get_node_style(child.kind());
            if show_source {
                let text = child
                    .utf8_text(source.as_bytes())
                    .unwrap_or("")
                    .trim()
                    .replace(['\n', '\r'], " ")
                    .chars()
                    .take(40)
                    .collect::<String>();
                let source_text = if text.is_empty() {
                    String::new()
                } else {
                    format!(" \"{}\"{}", text, if text.len() >= 40 { "..." } else { "" })
                };
                println!(
                    "{}{}\x1b[{}m{:<28}\x1b[0m{}",
                    prefix,
                    branch,
                    style.color_code,
                    child.kind(),
                    source_text
                );
            } else {
                println!(
                    "{}{}\x1b[{}m{:<28}\x1b[0m",
                    prefix,
                    branch,
                    style.color_code,
                    child.kind()
                );
            }
            print_node_tree_recursive(&child, source, &next_prefix, show_source, cursor);
        }
    }
}

struct NodeStyle {
    color_code: u8,
}

fn get_node_style(node_kind: &str) -> NodeStyle {
    // ===== 顶层结构 =====
    if node_kind == "class"
        || node_kind.contains("function_definition")
        || node_kind.contains("compound_statement")
        || node_kind.contains("field_declaration_list")
    {
        NodeStyle { color_code: 94 } // 亮蓝 - 结构骨架
    }

    // ===== 声明 / 声明器 =====
    else if node_kind.contains("declaration")
        || node_kind.contains("definition")
        || node_kind.contains("declarator")
    {
        NodeStyle { color_code: 34 } // 蓝 - 声明相关
    }

    // ===== 类型系统 =====
    else if node_kind.contains("primitive_type")
        || node_kind.contains("type")
    {
        NodeStyle { color_code: 36 } // 青 - 类型
    }

    // ===== 参数 =====
    else if node_kind.contains("parameter")
    {
        NodeStyle { color_code: 96 } // 亮青 - 参数
    }

    // ===== 访问 / 存储修饰 =====
    else if node_kind.contains("access_specifier")
        || node_kind.contains("storage_class_specifier")
        || node_kind == "public"
        || node_kind == "private"
        || node_kind == "protected"
        || node_kind == "static"
    {
        NodeStyle { color_code: 91 } // 亮红 - 修饰符
    }

    // ===== 标识符 =====
    else if node_kind.contains("identifier")
        || node_kind.contains("name")
    {
        NodeStyle { color_code: 32 } // 绿 - 名字
    }

    // ===== 标点 / 符号 =====
    else if matches!(
        node_kind,
        "{" | "}" | "(" | ")" | ";" | ":" | ","
    ) {
        NodeStyle { color_code: 90 } // 深灰 - 结构符号
    }

    // ===== 注释 =====
    else if node_kind.contains("comment") {
        NodeStyle { color_code: 90 } // 灰
    }

    // ===== 兜底 =====
    else {
        NodeStyle { color_code: 37 } // 默认
    }
}
