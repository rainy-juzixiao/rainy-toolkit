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
use crate::gen::constructor_stub::generate_ctor_stub;
use crate::gen::function_stub::{generate_global_fun_stub, generate_memfun_stub};
use crate::gen::header_stub::generate_header_stub;
use crate::gen::property_stub::generate_public_properties_stub;
use crate::model::cpp_class::ClazzItemCategory;
use crate::model::cpp_class::{CppClass, ParseResult};
use crate::model::cpp_code_registration::RegistrationCode;
use crate::model::cpp_function::CppFunction;
use crate::model::cpp_header_names::is_cpp_header;
use crate::model::cpp_sources_names::is_cpp_source;
use crate::parser::parse_cpp;
use crate::utility::new_line;
use std::fs;
use std::path::PathBuf;

fn generate_class_registration_begin_stub(output: &mut String, item: &CppClass) {
    let type_name = item.type_name();
    if item.namespace_location.is_empty() {
        output.push_str(&format!(
            "    registration::class_<{}>(\"{}\")\n",
            type_name, type_name
        ));
    } else {
        output.push_str(&format!(
            "    registration::class_<{}>(\"{}\")\n",
            item.full_qual_name, item.full_qual_name,
        ));
    }
}

fn generate_class_registration_end_stub(output: &mut String) {
    output.push_str(";\n");
}

fn begin_registration_stub(output: &mut String) {
    output.push_str("RAINY_REFLECTION_REGISTRATION {");
    new_line(output);
}

fn end_registration_stub(output: &mut String) {
    output.push_str(";\n}\n");
}

pub fn generate_registration(
    cli: &CommandArguments,
    registration_code: &RegistrationCode,
    header_file: &str,
) -> String {
    let mut output = String::new();
    let header_path = fs::canonicalize(header_file).unwrap();
    generate_header_stub(cli, &mut output, &header_path); // 生成头stub节
    begin_registration_stub(&mut output);
    for item in registration_code.classes() {
        generate_class_registration_begin_stub(&mut output, item);
        if !item.all_is_empty() {
            if generate_ctor_stub(&mut output, &item)
                && item.has_any_after(ClazzItemCategory::Constructors)
            {
                new_line(&mut output);
            }
            if generate_memfun_stub(&mut output, &item)
                && item.has_any_after(ClazzItemCategory::MemberFunctions)
            {
                new_line(&mut output);
            }
            generate_public_properties_stub(&mut output, &item);
        }
        generate_class_registration_end_stub(&mut output);
    }
    let global_functions: &Vec<CppFunction> = registration_code.global_functions();
    if generate_global_fun_stub(&mut output, &global_functions) {}
    end_registration_stub(&mut output);
    if cli.verbose {
        println!("Finish generate for file: {}", header_file);
        println!("Generated code for these classes: ");
        for item in registration_code.classes() {
            println!("    class: {} ", item.type_name());
        }
        println!("Generated code for these functions: ");
        for item in registration_code.global_functions() {
            println!("    functions: {}", item.name);
        }
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
    if is_cpp_header(input_path) || is_cpp_source(input_path) {
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
