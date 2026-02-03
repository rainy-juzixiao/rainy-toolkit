use crate::model::cpp_class::CppClass;
use crate::model::cpp_ctor::CppCtor;
use crate::model::cpp_function::CppFunction;
use crate::parser::extract_functions_and_ctors;
use tree_sitter::Node;

pub struct RegistrationClass {
    class: CppClass,
    cpp_ctors: Vec<CppCtor>,
    cpp_functions: Vec<CppFunction>,
}

impl RegistrationClass {
    pub fn new(class: &CppClass, node: &Node, source: &String) -> RegistrationClass {
        let mut cpp_functions = Vec::new();
        let mut cpp_ctors = Vec::new();
        extract_functions_and_ctors(*node, &source, &mut cpp_functions, &mut cpp_ctors);
        RegistrationClass {
            class: class.clone(),
            cpp_ctors,
            cpp_functions,
        }
    }

    pub fn type_name(&self) -> &String {
        &self.class.name
    }

    pub fn functions(&self) -> &Vec<CppFunction> {
        &self.cpp_functions
    }

    pub fn constructors(&self) -> &Vec<CppCtor> {
        &self.cpp_ctors
    }
}
