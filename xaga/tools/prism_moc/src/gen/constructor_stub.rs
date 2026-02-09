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

use crate::model::{cpp_class::CppClass, cpp_ctor::CppCtor};

fn make_constructor_stub(output: &mut String, ctor: &CppCtor, new_line: bool) {
    let param_list = ctor.params.join(", ");
    output.push_str(&format!("        .constructor<{}>()", param_list));
    if new_line {
        output.push('\n');
    }
}

pub fn generate_ctor_stub(output: &mut String, class: &CppClass) -> bool {
    if class.constructors().is_empty() {
        return false;
    }
    for (index, ctor) in class.constructors().iter().enumerate() {
        if index == class.constructors().len() - 1 {
            make_constructor_stub(output, ctor, false);
            break;
        }
        make_constructor_stub(output, ctor, true);
    }
    true
}
