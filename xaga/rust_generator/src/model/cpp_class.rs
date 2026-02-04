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

