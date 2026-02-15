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
mod test;

use crate::cli::easter_egg::handle_easter_egg;
use crate::cli::{print_moc_compiler_description, print_usage, CommandArguments};
use crate::gen::generate_code;
use crate::lang::set_this_session_lang;
use crate::model::incremental::{clear_cache, IncrementalState};
use crate::utility::{hash_generated_code, modify_filename_in_front, write_cpp_file, write_file};
use clap::Parser;
use std::path::{Path, PathBuf};
use std::{fs, io};

fn start_generate(
    cli: &CommandArguments,
    input_path: PathBuf,
    input_file: &str,
) -> anyhow::Result<()> {
    let mut state = IncrementalState::new(input_path.clone())?;
    let changes = state.analyze_changes();
    // Fix: 使用绝对路径或者当前工作目录
    let temp_dir = std::env::current_dir()?
        .join("build")
        .join("temp");
    // Fix: 确保目录创建成功，添加更详细的错误信息
    fs::create_dir_all(&temp_dir).map_err(|e| {
        anyhow::anyhow!("Failed to create temp directory {:?}: {}", temp_dir, e)
    })?;
    let input_filename = input_path
        .file_name()
        .unwrap()
        .to_string_lossy()
        .to_string();
    let temp_file_path = temp_dir.join(modify_filename_in_front(input_filename, "tmp_").unwrap());
    if !cli.no_cache {
        if cli.dev {
            state.print_debug_info(cli.verbose);
        }
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
                    let output_file_name: String;
                    if cli.out.is_none() {
                        if cli.verbose {
                            println!(
                                "No output file specified, use the original filename to generate."
                            );
                        }
                        output_file_name = modify_filename_in_front(out_file, "moc_").unwrap();
                    } else {
                        output_file_name = out_file.clone();
                    }
                    // Fix: 添加文件复制的错误处理
                    fs::copy(&cached_temp, &output_file_name).map_err(|e| {
                        anyhow::anyhow!("Failed to copy from {:?} to {}: {}", cached_temp, output_file_name, e)
                    })?;

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
    } else {
        if cli.verbose && cli.no_cache {
            println!("Detected cache is disabled");
        }
    }
    let registration_code = generate_code(&cli, &input_path, input_file)?;
    // Fix: 添加文件写入的错误处理
    write_cpp_file(&registration_code, temp_file_path.to_str().unwrap()).map_err(|e| {
        anyhow::anyhow!("Failed to write temp file {:?}: {}", temp_file_path, e)
    })?;
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
    let cli = CommandArguments::parse();
    if handle_easter_egg(&cli).await {
        return Ok(());
    }
    if let Some(lang) = &cli.lang {
        set_this_session_lang(&cli, &lang);
    }
    if cli.clear_cache {
        clear_cache();
        return Ok(());
    }
    if cli.help {
        print_usage();
        return Ok(());
    }
    if cli.version {
        print_moc_compiler_description(&cli);
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
        return Err(anyhow::format_err!("Cannot start generate for {}, file not exists", input_path.display()));
    };
    start_generate(&cli, input_path, input_file)?;
    Ok(())
}
