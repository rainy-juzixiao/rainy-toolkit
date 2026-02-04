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

mod cli;
mod dev_debug_tools;
mod gen;
mod lang;
mod marco;
mod model;
mod parser;
mod utility;

use crate::cli::{
    code_rain, find_node_by_range, print_moc_compiler_description, print_usage, tea_ceremony,
    CommandArguments,
};
use crate::dev_debug_tools::ast_node_tree::print_node_tree;
use crate::gen::generate_registration;
use crate::lang::language::set_this_session_lang;
use crate::model::cpp_class::ParseResult;
use crate::model::cpp_code_registration::RegistrationClass;
use crate::model::incremental::IncrementalState;
use crate::utility::{hash_generated_code, modify_filename_in_front, write_cpp_file};
use clap::Parser;
use parser::parse_cpp;
use std::path::{Path, PathBuf};
use std::{fs, io};

fn write_file(cli: &CommandArguments, input_file: &String, registration_code: &String) {
    let out_file: String;
    if cli.out.is_none() {
        if cli.verbose {
            println!("No output file specified, use the original filename to generate.");
        }
        out_file = input_file.clone();
    } else {
        out_file = cli.out.clone().unwrap();
    }

    let file_name = modify_filename_in_front(out_file, "moc_").unwrap();
    write_cpp_file(registration_code, file_name.as_str()).unwrap();
}

fn generate_code(
    cli: &CommandArguments,
    input_path: &PathBuf,
    input_file: &str,
) -> anyhow::Result<String> {
    if cli.verbose {
        println!("Generating new output...");
    }
    let source = fs::read_to_string(&input_path)?;
    let parse_result: ParseResult = parse_cpp(&source, &cli)?;
    let mut total_generate: Vec<RegistrationClass> = Vec::new();
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
            total_generate.push(RegistrationClass::new(class, &node, &source));
        }
    }
    if cli.verbose {
        if total_generate.is_empty() {
            println!(
                "We didn't see any moc class to generate when we read {:?}",
                cli.input
            );
        }
        println!("Found {} moc class(es) to generate", total_generate.len());
    }
    let registration_code = generate_registration(&cli, &total_generate, input_file);
    Ok(registration_code)
}

fn start_generate(
    cli: &CommandArguments,
    input_path: PathBuf,
    input_file: &str,
) -> anyhow::Result<()> {
    // 创建增量状态
    let mut state = IncrementalState::new(input_path.clone())?;
    state.print_debug_info(cli.verbose);
    let changes = state.analyze_changes();
    let temp_dir = PathBuf::from("./build/temp");
    fs::create_dir_all(&temp_dir)?;
    let input_filename = input_path
        .file_name()
        .unwrap()
        .to_string_lossy()
        .to_string();
    let temp_file_path = temp_dir.join(modify_filename_in_front(input_filename, "tmp_").unwrap());
    if !changes.needs_full_rebuild && changes.files_to_regenerate.is_empty() {
        if let Some(cached_temp) = state.get_cached_temp_file() {
            if cli.verbose {
                println!("Input unchanged, found cached temp file: {:?}", cached_temp);
            }
            if cached_temp.exists() {
                let out_file = cli
                    .out
                    .as_ref()
                    .map(|s| s.clone())
                    .unwrap_or_else(|| input_file.to_string());

                let output_file_name = modify_filename_in_front(out_file, "moc_").unwrap();
                fs::copy(cached_temp, &output_file_name)?;

                if cli.verbose {
                    println!("  Cache hit! Skipped compilation.");
                    println!("  Copied from: {:?}", cached_temp);
                    println!("  Output to: {}", output_file_name);
                }
                return Ok(());
            } else if cli.verbose {
                println!("Cached temp file not found, regenerating...");
            }
        }
    }
    let registration_code = generate_code(&cli, &input_path, input_file)?;
    write_cpp_file(&registration_code, temp_file_path.to_str().unwrap())?;
    let generated_code_hash = hash_generated_code(&registration_code);
    write_file(&cli, &input_file.to_string(), &registration_code);
    state.update_file_state(generated_code_hash, temp_file_path.clone());
    state.save_cache().map_err(|e| {
        io::Error::new(
            io::ErrorKind::Other,
            format!("Failed to save cache: {:?}", e),
        )
    })?;
    if cli.verbose {
        println!("Build completed and cache updated.");
    }
    Ok(())
}

#[tokio::main]
async fn main() -> anyhow::Result<()> {
    let mut exit = false;
    let cli = CommandArguments::parse();

    if let Some(lang) = &cli.lang {
        set_this_session_lang(&cli, &lang);
    }

    if cli.help {
        print_usage();
        return Ok(());
    }

    if cli.version {
        print_moc_compiler_description(&cli);
    }

    if cli.tea {
        tea_ceremony().await;
        exit = true;
    } else if cli.rain {
        code_rain(cli.rain_duration).await;
        exit = true;
    }

    if exit {
        return Ok(());
    }

    let input_file = match &cli.input {
        Some(input) => input,
        None => {
            print_usage();
            return Ok(());
        }
    };
    let input_path = if Path::new(input_file).is_absolute() {
        PathBuf::from(input_file)
    } else {
        std::env::current_dir()?.join(input_file)
    };
    let input_path = if input_path.exists() {
        input_path.canonicalize()?
    } else {
        input_path
    };
    start_generate(&cli, input_path, input_file)?;
    Ok(())
}
