use tree_sitter::Node;

pub fn find_node_by_range<'a>(node: Node<'a>, start: usize, end: usize) -> Option<Node<'a>> {
    if node.start_byte() == start && node.end_byte() == end {
        return Some(node);
    }

    for i in 0..node.child_count() {
        if let Some(child) = node.child(i as u32) {
            if let Some(found) = find_node_by_range(child, start, end) {
                return Some(found);
            }
        }
    }
    None
}
