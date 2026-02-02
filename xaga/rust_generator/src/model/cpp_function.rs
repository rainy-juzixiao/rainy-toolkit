#[derive(Debug, Clone)]
pub struct CppFunction {
    pub name: String,
    pub return_type: String,
    pub params: Vec<String>,
    pub is_static: bool,
}