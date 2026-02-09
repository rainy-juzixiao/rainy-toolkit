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

use crate::model::{cpp_class::CppClass, cpp_function::CppFunction};

fn make_function_stub(output: &mut String, func: &CppFunction, type_name: &String, new_line: bool) {
    let param_list = func.params.join(", ");
    if func.is_static {
        output.push_str(&format!(
            "        .method(\"{}\",rainy::utility::get_overloaded_func<{}()>(&{}::{}))",
            func.name, func.return_type, type_name, func.name
        ));
    } else {
        output.push_str(&format!(
            "        .method(\"{}\",rainy::utility::get_overloaded_func<{}, {}({})>(&{}::{}))",
            func.name,
            type_name,
            func.return_type,
            if param_list.is_empty() {
                "".to_string()
            } else {
                format!("{}", param_list)
            },
            type_name,
            func.name
        ));
    }
    if new_line {
        output.push('\n');
    }
}

pub fn generate_memfun_stub(output: &mut String, class: &CppClass) -> bool {
    if class.functions().is_empty() {
        return false;
    }
    let type_name = class.type_name();
    for (index, func) in class.functions().iter().enumerate() {
        if index == class.functions().len() - 1 {
            make_function_stub(output, func, type_name, false);
            break;
        }
        make_function_stub(output, func, type_name, true);
    }
    true
}

pub fn generate_global_fun_stub(output: &mut String, global_functions: &Vec<CppFunction>) -> bool {
    if !global_functions.is_empty() {
        let first_func = global_functions.first().unwrap();
        let param_list = first_func.params.join(", ");
        output.push_str(&format!(
            "    registration::method(\"{}\",rainy::utility::get_overloaded_func<{}({})>(&{}))",
            first_func.name,
            first_func.return_type,
            if param_list.is_empty() {
                "".to_string()
            } else {
                format!("{}", param_list)
            },
            first_func.name
        ));
        for (index, func) in global_functions.iter().skip(1).enumerate() {
            if index != global_functions.len() - 1 {
                output.push('\n');
            }
            // 从第2个函数开始
            let param_list = func.params.join(", ");
            output.push_str(&format!(
                "        .method(\"{}\",rainy::utility::get_overloaded_func<{}({})>(&{}))",
                func.name,
                func.return_type,
                if param_list.is_empty() {
                    "".to_string()
                } else {
                    format!("{}", param_list)
                },
                func.name
            ));
        }
        return true;
    }
    false
}
