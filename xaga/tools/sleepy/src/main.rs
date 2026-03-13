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
mod data;
mod i18n;
mod parser;
mod utils;
mod generator;
mod toolchain;
mod gen_compile_db;

use std::collections::{HashMap, HashSet};
use std::path::{PathBuf};
use anyhow::Context;
use clang::{Clang, Index};

fn main() -> anyhow::Result<()> {
    let clang = Clang::new().unwrap();
    let index = Index::new(&clang, false, false);

    // 查找配置文件
    let config_path = data::config::find_config()
        .ok_or_else(|| anyhow::anyhow!(
            "no config file found, expected sleepy.json / sleepy.yaml in current directory"
        ))?;

    println!("[sleepy] using config: {}", config_path.display());
    let config = data::config::load(&config_path)?;

    std::fs::create_dir_all(&config.output_dir)
        .with_context(|| format!("cannot create output dir: {}", config.output_dir.display()))?;

    let gen = generator::markdown::MarkdownGenerator::new(&config.lang);

    for source in &config.sources {
        process_source(source, &config, &index, &gen)?;
    }

    Ok(())
}

fn process_source(
    source: &data::config::SourceConfig,
    config: &data::config::SleepyConfig,
    index: &Index,
    gen: &generator::markdown::MarkdownGenerator,
) -> anyhow::Result<()> {
    println!("[sleepy] source: {}", source.name);
    let toolchain = match &source.compile_flags.compiler {
        Some(path) => {
            println!("[toolchain] using compiler from config: {}", path.display());
            toolchain::Toolchain::from_path(path)?
        }
        None => toolchain::Toolchain::detect()?,
    };
    let args = source.compile_flags.to_args(&source.include_dirs, &toolchain)?;
    println!("[sleepy]   compiler args: {}", args.join(" "));

    let headers: Vec<PathBuf> = if !source.files.is_empty() {
        source.files.iter()
            .map(|f| f.canonicalize().unwrap_or_else(|_| f.clone()))
            .collect()
    } else {
        let mut h = Vec::new();
        for dir in &source.include_dirs {
            let canonical = dir.canonicalize()
                .with_context(|| format!("cannot resolve: {}", dir.display()))?;
            h.extend(gen_compile_db::collect_headers_pub(&canonical, &source.extensions));
        }
        h
    };
    println!("[sleepy]   found {} files", headers.len());
    let mut merge_map: HashMap<PathBuf, PathBuf> = HashMap::new();
    for header in &headers {
        if let Some(target_name) = parser::read_mergeto_tag(header) {
            let include_root = match source.include_dirs[0].canonicalize() {
                Ok(p) => p,
                Err(_) => continue,
            };
            let target_path = include_root.join(&target_name);
            if let (Ok(src), Ok(dst)) = (
                std::fs::canonicalize(header),
                std::fs::canonicalize(&target_path),
            ) {
                merge_map.insert(src, dst);
            } else {
                eprintln!(
                    "[sleepy warn] @mergeto 目标无法解析: {} → {}",
                    header.display(),
                    target_name
                );
            }
        }
    }
    let mut owned_map: HashMap<PathBuf, HashSet<PathBuf>> = HashMap::new();
    for header in &headers {
        let canon = match std::fs::canonicalize(header) {
            Ok(p) => p,
            Err(_) => continue,
        };
        if merge_map.contains_key(&canon) {
            continue;
        }
        let mut owned = HashSet::new();
        owned.insert(canon.clone());
        for (src, dst) in &merge_map {
            if dst == &canon {
                owned.insert(src.clone());
            }
        }
        owned_map.insert(canon, owned);
    }
    let source_output_dir = config.output_dir.join(&source.name);
    std::fs::create_dir_all(&source_output_dir)?;
    for (main_file, owned_files) in &owned_map {
        let file_str = main_file.to_str()
            .ok_or_else(|| anyhow::anyhow!("invalid path: {}", main_file.display()))?;

        let tu = index.parser(file_str)
            .arguments(&args)
            .incomplete(true)
            .skip_function_bodies(true)
            .detailed_preprocessing_record(true)
            .parse()
            .with_context(|| format!("failed to parse: {}", main_file.display()))?;

        for diag in tu.get_diagnostics() {
            if diag.get_severity() >= clang::diagnostic::Severity::Error {
                eprintln!("[parse warn] {}: {}", file_str, diag.get_text());
            }
        }

        let source_text = std::fs::read_to_string(main_file).unwrap_or_default();
        let nodoc_ranges = parser::collect_nodoc_ranges(&source_text);
        let include_root = source.include_dirs[0].canonicalize()?;

        let context = parser::ParseContext {
            include_root: include_root.to_str().unwrap_or(""),
            nodoc_ranges: &nodoc_ranges,
            owned_files,
        };

        let doc = parser::build_file_document(&tu, file_str, &context);
        let md = gen.generate_file(&doc);

        let out_name = main_file.file_stem()
            .and_then(|s| s.to_str())
            .unwrap_or("unknown");
        let out_path = source_output_dir.join(format!("{}.md", out_name));
        std::fs::write(&out_path, &md)
            .with_context(|| format!("failed to write: {}", out_path.display()))?;
        println!("[sleepy]   → {}", out_path.display());
    }

    Ok(())
}