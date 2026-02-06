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

use crate::model::{cpp_class::CppClass, cpp_property::CppProperty};

fn make_property_stub(output: &mut String, prop: &CppProperty, type_name: &String, new_line: bool) {
    output.push_str(&format!(
        "        .property(\"{}\", &{}::{})",
        prop.name, type_name, prop.name
    ));
    if new_line {
        output.push('\n');
    }
}

pub fn generate_public_properties_stub(output: &mut String, class: &CppClass) -> bool {
    if class.public_properties().is_empty() {
        return false;
    }
    let type_name = class.type_name();
    for (index, prop) in class.public_properties().iter().enumerate() {
        if index == class.public_properties().len() - 1 {
            make_property_stub(output, prop, type_name, false);
            break;
        }
        make_property_stub(output, prop, type_name, true);
    }
    true
}
