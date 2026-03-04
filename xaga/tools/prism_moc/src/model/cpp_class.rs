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

use tree_sitter::Tree;

use crate::model::cpp_property::CppProperty;
use crate::model::{cpp_ctor::CppCtor, cpp_function::CppFunction};
use crate::model::cpp_enumeration::{CppEnumeration};
use crate::utility::BitSet;

#[derive(Debug, Clone)]
pub struct CppClass {
    pub name: String,
    pub namespace_location: String,
    pub full_qual_name: String,
    pub cpp_ctors: Vec<CppCtor>,
    pub cpp_functions: Vec<CppFunction>,
    pub cpp_public_properties: Vec<CppProperty>,
    pub use_namespaces: Vec<String>,
    pub use_items: Vec<String>,
    pub start_byte: usize,
    pub end_byte: usize,
}

#[repr(u8)]
pub enum ClazzItemCategory {
    Constructors = 0,
    MemberFunctions = 1,
    PublicProperties = 2,
}

impl ClazzItemCategory {
    fn index(self) -> usize {
        self as usize
    }
}

impl CppClass {
    pub fn type_name(&self) -> &String {
        &self.name
    }

    pub fn functions(&self) -> &Vec<CppFunction> {
        &self.cpp_functions
    }

    pub fn constructors(&self) -> &Vec<CppCtor> {
        &self.cpp_ctors
    }

    pub fn public_properties(&self) -> &Vec<CppProperty> {
        &self.cpp_public_properties
    }

    pub fn has_any_after(&self, item_category: ClazzItemCategory) -> bool {
        let current_index = item_category.index();
        let status_map = self.get_status_map();
        for i in (current_index + 1)..3 {
            if status_map.get(i) {
                return true;
            }
        }
        false
    }

    pub fn all_is_empty(&self) -> bool {
        self.get_status_map().all(true)
    }

    fn get_status_map(&self) -> BitSet {
        let mut bitset: BitSet = BitSet::new(3);
        bitset.set(
            ClazzItemCategory::index(ClazzItemCategory::Constructors),
            !self.constructors().is_empty(),
        );
        bitset.set(
            ClazzItemCategory::index(ClazzItemCategory::MemberFunctions),
            !self.functions().is_empty(),
        );
        bitset.set(
            ClazzItemCategory::index(ClazzItemCategory::PublicProperties),
            !self.public_properties().is_empty(),
        );
        bitset
    }
}

pub enum AccessLevel {
    Public,
    Protected,
    Private,
}

pub struct ParseResult {
    pub tree: Tree,
    pub classes: Vec<CppClass>,
    pub global_functions: Vec<CppFunction>,
    pub global_enumerations: Vec<CppEnumeration>,
}
