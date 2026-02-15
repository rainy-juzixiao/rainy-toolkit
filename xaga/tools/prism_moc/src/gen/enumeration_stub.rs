use std::fmt::format;
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
use crate::model::cpp_enumeration::CppEnumeration;

pub fn generate_enumeration_stub(output: &mut String, enumerations: &Vec<CppEnumeration>) -> bool {
    if enumerations.is_empty() {
        return false;
    }
    for (index, item) in enumerations.iter().enumerate() {
        let mut new_line = true;
        if index == enumerations.len() - 1 {
            new_line = false;
        }
        let type_name = &item.full_qual_name;
        output.push_str(&format!(
            "    registration::enumeration<{}>(\"{}\")",
            type_name, type_name
        ));
        output.push('\n');
        output.push_str("    (\n");
        for (index, enum_item) in item.items.iter().enumerate() {

            output.push_str(&format!(
                "        enum_value(\"{}\", {})",
                enum_item.name,
                format!("{}::{}", type_name, enum_item.name)
            ));
            if index == item.items.len() - 1 {
                output.push('\n');
            } else {
                output.push_str(",\n");
            }
        }
        output.push_str("    );");
        if new_line {
            output.push('\n');
        }
    }
    true
}
