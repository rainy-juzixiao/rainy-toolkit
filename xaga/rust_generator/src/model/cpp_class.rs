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

#[derive(Debug, Clone)]
pub struct CppFunction {
    pub name: String,
    pub return_type: String,
    pub params: Vec<String>,
    pub is_static: bool,
}