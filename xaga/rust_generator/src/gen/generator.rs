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

use crate::cli::{find_node_by_range, CommandArguments};
use crate::dev_debug_tools::ast_node_tree::print_node_tree;
use crate::model::cpp_class::ParseResult;
use crate::model::cpp_code_registration::RegistrationCode;
use crate::model::cpp_function::CppFunction;
use crate::model::cpp_header_names::is_cpp_header;
use crate::model::cpp_sources_names::is_cpp_source;
use crate::parser::parse_cpp;
use crate::{include_statics, locate_runtime_resources};
use std::collections::HashMap;
use std::fs;
use std::path::PathBuf;
use strfmt::strfmt;

fn new_line(output: &mut String) {
    output.push_str("\n"); // 插入新行
}

fn generated_header_stub(cli: &CommandArguments, output: &mut String, path: &PathBuf) {
    {
        output.push_str(include_statics!("embed_licences")); // 插入许可证说明
    }
    new_line(output);
    {
        let version = env!("CARGO_PKG_VERSION").to_string();
        let absoulte_source = path.display().to_string();
        let input_source = path.file_name().unwrap();

        let template = match fs::read_to_string(locate_runtime_resources!("embed_warnings")) {
            Ok(template) => template,
            Err(_) => {
                if cli.verbose {
                    println!("Cannot get embed_warnings template in statics folder, use internal embed_warnings template");
                }
                let template = include_statics!("embed_warnings");
                template.to_string()
            }
        };
        let mut vars: HashMap<String, String> = HashMap::new();
        vars.insert(
            "input_source".to_string(),
            input_source.to_str().unwrap().to_string(),
        );
        vars.insert("absoulte_source".to_string(), absoulte_source.to_string());
        vars.insert("generator_version".to_string(), version.to_string());
        let result = match strfmt(&template, &vars) {
            Ok(content) => content,
            Err(err) => {
                if cli.verbose {
                    println!(
                        "Err: {}\nCannot format liceneces stub, use default stub to generate",
                        err
                    );
                }
                let res = include_statics!("embed_warnings");
                res.to_string()
            }
        };
        output.push_str(&*result);
    }
    new_line(output);
    output.push_str(&format!("#include \"{}\"\n", path.to_str().unwrap()));
    output.push_str("#include <rainy/meta/reflection/registration.hpp>\n");
    new_line(output);
    output.push_str("using namespace rainy::meta::reflection;\n");
    {
        new_line(output);
        output.push_str(&format!("#line 1 \"{}\"\n", path.to_str().unwrap()));
        new_line(output);
    }
}

pub fn generate_registration(
    cli: &CommandArguments,
    registration_code: &RegistrationCode,
    header_file: &str,
) -> String {
    let mut output = String::new();
    let header_path = fs::canonicalize(header_file).unwrap();
    generated_header_stub(cli, &mut output, &header_path); // 生成头stub节
    output.push_str("RAINY_REFLECTION_REGISTRATION {\n");
    for item in registration_code.classes() {
        let type_name = item.type_name();
        output.push_str(&format!(
            "    registration::class_<{}>(\"{}\")\n",
            type_name, type_name
        ));
        for ctor in item.constructors() {
            let param_list = ctor.params.join(", ");
            output.push_str(&format!("        .constructor<{}>()\n", param_list));
        }
        for func in item.functions() {
            let param_list = func.params.join(", ");
            if func.is_static {
                output.push_str(&format!(
                    "        .method(\"{}\",rainy::utility::get_overloaded_func<{}()>(&{}::{}))\n",
                    func.name, func.return_type, type_name, func.name
                ));
            } else {
                output.push_str(&format!(
                    "        .method(\"{}\",rainy::utility::get_overloaded_func<{}, {}({})>(&{}::{}))\n",
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
        }
    }
    output.push_str("    ;");
    let global_functions: &Vec<CppFunction> = registration_code.global_functions();
    if !global_functions.is_empty() {
        new_line(&mut output);
        let first_func = global_functions.first().unwrap();
        let param_list = first_func.params.join(", ");
        output.push_str(&format!(
            "    registration::method(\"{}\",rainy::utility::get_overloaded_func<{}({})>(&{}))\n",
            first_func.name,
            first_func.return_type,
            if param_list.is_empty() {
                "".to_string()
            } else {
                format!("{}", param_list)
            },
            first_func.name
        ));
        for func in global_functions.iter().skip(1) { // 从第2个函数开始
            let param_list = func.params.join(", ");
            output.push_str(&format!(
                "        .method(\"{}\",rainy::utility::get_overloaded_func<{}({})>(&{}))\n",
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
    }

    output.push_str("    ;\n}\n");
    if cli.verbose {
        println!("Finish generate for file: {}", header_file);
        println!("Generated code for these classes: ");
        // for item in registration_class {
        //     println!("{},", item.type_name());
        // }
        println!("Generated code: \n{}", output);
    }
    output
}

pub fn generate_code(
    cli: &CommandArguments,
    input_path: &PathBuf,
    input_file: &str,
) -> anyhow::Result<String> {
    if cli.verbose {
        println!("Generating new output...");
    }
    if is_cpp_header(input_path) | is_cpp_source(input_path) {
        if cli.verbose {
            println!(
                "Scan header / source files : {:?}",
                input_path.file_name().unwrap()
            );
        }
    } else {
        if cli.verbose {
            println!("Detected a unknown extension file, make sure is a valid cpp source file");
        }
    }
    let source = fs::read_to_string(&input_path)?;
    let parse_result: ParseResult = parse_cpp(&source, &cli)?;
    let mut total_generate: RegistrationCode = RegistrationCode::make();
    for class in &parse_result.classes {
        if cli.verbose {
            println!("Found cpp moc class: {}", class.name);
        }
        if let Some(node) = find_node_by_range(
            parse_result.tree.root_node(),
            class.start_byte,
            class.end_byte,
        ) {
            if cli.dev {
                print_node_tree(node, &source, true);
            }
            total_generate.add_class(class.clone(), &node, &source);
        }
    }
    for function in &parse_result.global_functions {
        if cli.verbose {
            println!("Found moc cpp global function: {}", function.name);
        }
        total_generate.add_global_function(function.clone());
    }
    if cli.verbose {
        if total_generate.classes_is_empty() {
            println!(
                "We didn't see any moc class to generate when we read {:?}",
                cli.input
            );
        }
        println!(
            "Found {} moc class(es) to generate",
            total_generate.classes_count()
        );
    }
    let registration_code = generate_registration(&cli, &total_generate, input_file);
    Ok(registration_code)
}
