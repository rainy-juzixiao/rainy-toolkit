use tree_sitter::Node;

pub fn print_node_tree(node: Node, source: &str, indent: usize, show_source: bool) {
    let padding = " ".repeat(indent);

    if show_source {
        // 获取源码片段并去掉换行和多余空格
        let text = node
            .utf8_text(source.as_bytes())
            .unwrap_or("")
            .trim()
            .replace("\n", " ");
        println!(
            "{}{} [{}..{}]: {}",
            padding,
            node.kind(),
            node.start_byte(),
            node.end_byte(),
            text
        );
    } else {
        println!(
            "{}{} [{}..{}]",
            padding,
            node.kind(),
            node.start_byte(),
            node.end_byte()
        );
    }

    // 遍历子节点
    for i in 0..node.child_count() {
        if let Some(child) = node.child(i as u32) {
            print_node_tree(child, source, indent + 2, show_source);
        }
    }
}
