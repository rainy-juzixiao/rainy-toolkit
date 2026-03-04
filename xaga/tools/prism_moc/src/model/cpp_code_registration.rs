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

use crate::model::cpp_class::{AccessLevel, CppClass};
use crate::model::cpp_enumeration::CppEnumeration;
use crate::model::cpp_function::CppFunction;
use crate::parser::{extract_functions_and_ctors, extract_properties};
use tree_sitter::Node;

pub struct RegistrationCode {
    classes: Vec<CppClass>,
    global_function: Vec<CppFunction>,
    enumerations: Vec<CppEnumeration>,
}

impl RegistrationCode {
    pub fn make() -> RegistrationCode {
        RegistrationCode {
            classes: Vec::new(),
            global_function: Vec::new(),
            enumerations: Vec::new(),
        }
    }

    pub fn classes(&self) -> &Vec<CppClass> {
        &self.classes
    }

    pub fn add_class(&mut self, class: CppClass, node: &Node, source: &String) {
        let mut clazz = class.clone();
        extract_functions_and_ctors(
            *node,
            &source,
            &mut clazz.cpp_functions,
            &mut clazz.cpp_ctors,
        );
        extract_properties(
            *node,
            &source,
            &mut clazz.cpp_public_properties,
            AccessLevel::Public,
        );
        self.classes.push(clazz);
    }

    pub fn enumerations(&self) -> &Vec<CppEnumeration> {
        &self.enumerations
    }

    pub fn add_global_function(&mut self, function: CppFunction) {
        self.global_function.push(function);
    }

    pub fn add_enumeration(&mut self, enumeration: CppEnumeration) {
        self.enumerations.push(enumeration);
    }

    pub fn global_functions(&self) -> &Vec<CppFunction> {
        &self.global_function
    }

    pub fn classes_count(&self) -> usize {
        self.classes.len()
    }

    pub fn classes_is_empty(&self) -> bool {
        self.classes.is_empty()
    }
}
