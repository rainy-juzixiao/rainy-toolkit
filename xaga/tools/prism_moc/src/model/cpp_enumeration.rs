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

#[derive(Debug, Clone)]
pub struct CppEnumerationItem {
    pub name: String
}

#[derive(Debug, Clone)]
pub struct CppEnumeration {
    pub items: Vec<CppEnumerationItem>,
    pub is_enum_class: bool,
    pub namespace_location: String,
    pub full_qual_name: String,
    pub name: String,
    pub use_namespaces: Vec<String>,
    pub use_items: Vec<String>,
}