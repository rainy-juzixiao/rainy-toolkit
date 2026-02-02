use tree_sitter::{Node, Tree};

#[derive(Debug, Clone)]
pub struct CppClass {
    pub name: String,
    pub has_moc_macro: bool,
    pub start_byte: usize,
    pub end_byte: usize,
}

pub struct ParseResult {
    pub tree: Tree,
    pub classes: Vec<CppClass>,
}

