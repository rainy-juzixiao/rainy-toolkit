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
