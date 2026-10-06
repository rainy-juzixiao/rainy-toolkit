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
mod gen_compile_db;
mod generator;
mod i18n;
mod parser;
mod toolchain;
mod utils;

use anyhow::Context;
use clap::Parser;
use clang::{Clang, Index};
use std::collections::{HashMap, HashSet};
use std::path::PathBuf;
use std::process::ExitCode;

#[derive(Debug, Parser)]
#[command(
    name = "sleepy",
    version,
    about = "MuZiYan / sleepy — C++ 头文件文档生成器 (C++ header documentation generator)",
    after_help = "退出码 / exit codes: 0 = 成功 (success), 1 = 失败或检测到循环依赖 (failure or cycles detected)"
)]
struct Cli {
    /// 在查找/加载配置前切换工作目录 (change working directory first)
    #[arg(long, value_name = "DIR")]
    root: Option<PathBuf>,

    /// 显式指定配置文件路径，默认在当前目录查找 sleepy.json/yaml/yml
    /// (explicit config path; searched in CWD by default)
    #[arg(short = 'c', long, value_name = "PATH")]
    config: Option<PathBuf>,

    /// 仅生成 VitePress 的 reference 页面，不生成站点骨架 (reference pages only)
    #[arg(long)]
    partial: bool,

    /// 跳过依赖图与索引页生成；循环依赖检查仍会执行
    /// (skip dependency graph & index generation; cycle check still runs)
    #[arg(long)]
    no_graph: bool,

    /// 试运行：解析与检查照常，但不写任何输出文件
    /// (dry run: parse & check as usual, but write nothing)
    #[arg(long)]
    dry_run: bool,

    /// 发现循环依赖时仅警告，不作为错误退出 (warn instead of fail on cycles)
    #[arg(long)]
    allow_cycles: bool,

    /// 输出格式：markdown、html 自由组合（分隔符 + , ; 空格任选），也可用 all；默认 markdown
    /// (output formats, freely combinable; default markdown)
    #[arg(long, value_name = "FMT", default_value = "markdown")]
    format: String,
}

fn main() -> ExitCode {
    let cli = Cli::parse();
    match run(&cli) {
        Ok(()) => ExitCode::SUCCESS,
        Err(e) => {
            eprintln!("[sleepy] error: {e:#}");
            ExitCode::FAILURE
        }
    }
}

fn run(cli: &Cli) -> anyhow::Result<()> {
    let formats = parse_formats(&cli.format)?;
    let want_html = formats.contains(&OutputFormat::Html);
    let want_markdown = formats.contains(&OutputFormat::Markdown);
    if let Some(root) = &cli.root {
        std::env::set_current_dir(root)
            .with_context(|| format!("cannot change directory to --root: {}", root.display()))?;
        println!(
            "[sleepy] working directory set to: {}",
            std::env::current_dir()
                .map(|p| p.display().to_string())
                .unwrap_or_else(|_| root.display().to_string())
        );
    }

    let clang = Clang::new().unwrap();
    let index = Index::new(&clang, false, false);

    // --- locate & load config (with recursive includes) ---
    let config_path = match &cli.config {
        Some(p) => p.clone(),
        None => data::config::find_config().ok_or_else(|| {
            anyhow::anyhow!(
                "no config file found, expected sleepy.json / sleepy.yaml in current directory"
            )
        })?,
    };
    println!("[sleepy] using config: {}", config_path.display());
    let config = data::config::load_with_includes(&config_path)
        .with_context(|| format!("failed to load config: {}", config_path.display()))?;

    // NOTE: output dirs are created lazily in the write phase, so that a
    // failed cycle check truly leaves the filesystem untouched.

    // All include roots of all sources, so headers can include across
    // libraries referenced via `includes:` (convenience for multi-lib docs).
    let mut global_include_dirs: Vec<PathBuf> = Vec::new();
    let mut seen_dirs: HashSet<PathBuf> = HashSet::new();
    for source in &config.sources {
        for dir in &source.include_dirs {
            if let Ok(canon) = dir.canonicalize() {
                if seen_dirs.insert(canon.clone()) {
                    global_include_dirs.push(canon);
                }
            }
        }
    }

    let gen = generator::markdown::MarkdownGenerator::new(&config.lang);
    let mut all_docs: Vec<data::document::FileDocument> = Vec::new();
    // two-phase pipeline: (output path, content) registered first, written later
    let mut pending_writes: Vec<(PathBuf, String)> = Vec::new();
    let mut flat_seen: std::collections::HashMap<String, usize> = std::collections::HashMap::new();
    // canonical file path -> display name (e.g. "<source>/<rel path>")
    let mut file_infos: HashMap<String, String> = HashMap::new();
    let mut diag_seen: std::collections::HashSet<String> = std::collections::HashSet::new();

    for source in &config.sources {
        process_source(
            source,
            &config,
            &index,
            &gen,
            &global_include_dirs,
            &mut all_docs,
            &mut pending_writes,
            &mut file_infos,
            &mut diag_seen,
            want_markdown,
        )?;
    }

    // --- cycle detection (independent of --no-graph) ---
    let graph_opts = generator::reports::GraphBuildOptions {
        file_infos: &file_infos,
        ignored_edges: generator::reports::parse_ignored_edges(&config.dependency_graph.ignore),
    };
    let graph = generator::reports::build_include_graph(&all_docs, &graph_opts);
    let report = generator::cycles::find_cycles(&graph);
    if report.has_cycles() {
        let text = generator::cycles::format_cycle_report(&report, &config.lang);
        if config.dependency_graph.fail_on_cycle && !cli.allow_cycles {
            // two-phase guarantee: nothing has been written so far
            eprintln!("{text}");
            anyhow::bail!("循环依赖检查未通过 (cycle check failed); no output was written");
        }
        eprintln!("{text}");
        eprintln!(
            "[sleepy warn] --allow-cycles 生效，继续生成 (continuing despite cycles)"
        );
    }

    // --- phase 2: write everything ---
    if cli.dry_run {
        println!(
            "[sleepy] dry-run: {} 个文件待写入已跳过 (files would be written, skipped)",
            pending_writes.len()
        );
        return Ok(());
    }
    for (path, content) in &pending_writes {
        if let Some(parent) = path.parent() {
            std::fs::create_dir_all(parent)
                .with_context(|| format!("cannot create dir: {}", parent.display()))?;
        }
        std::fs::write(path, content)
            .with_context(|| format!("failed to write: {}", path.display()))?;
    }
    println!(
        "[sleepy] {} 个文件已写入 (files written)",
        pending_writes.len()
    );

    // --- HTML (always run if requested, independent of docs count) ---
    if want_html {
        let html_gen =
            generator::html::HtmlGenerator::new("assets/html", config.lang.clone());
        html_gen.generate_site(&all_docs, &config.output_dir.join("html"), config.flat)?;
        println!("[sleepy] HTML site at {}", config.output_dir.join("html").display());
    }

    if all_docs.is_empty() {
        return Ok(());
    }

    // --- dependency graph & index page ---
    if want_markdown && !cli.no_graph && config.dependency_graph.enabled {
        let graph_path = config
            .output_dir
            .join(format!("{}.md", config.dependency_graph.out_name));
        generator::reports::generate_dependency_graph(&graph, &graph_path, &config.lang)?;
        println!("[sleepy] dependency graph → {}", graph_path.display());

        let index_path = config.output_dir.join("index.md");
        generator::reports::generate_index_page(
            &all_docs,
            &file_infos,
            &index_path,
            &config.lang,
        )?;
        println!("[sleepy] index page → {}", index_path.display());
    }

    if want_markdown {
    // --- VitePress / markdown output ---
    let vp_gen = generator::rt_vitepress_markdown::VitePressMarkdownGenerator::new(&config.lang);
    if cli.partial {
        let ref_dir = config.output_dir.join("reference");
        vp_gen.generate_reference_only(&all_docs, &ref_dir)?;
        println!(
            "[sleepy] VitePress reference pages generated at {}",
            ref_dir.display()
        );
    } else {
        vp_gen.generate_site(&all_docs, &config.output_dir)?;
        println!(
            "[sleepy] VitePress site generated at {}",
            config.output_dir.join("docs").display()
        );
    }
    }

    Ok(())
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
enum OutputFormat {
    Markdown,
    Html,
}

fn parse_formats(raw: &str) -> anyhow::Result<Vec<OutputFormat>> {
    fn push_unique(out: &mut Vec<OutputFormat>, fmt: OutputFormat) {
        if !out.contains(&fmt) {
            out.push(fmt);
        }
    }
    let mut out: Vec<OutputFormat> = Vec::new();
    for part in raw.split(|c: char| c == '+' || c == ',' || c == ';' || c.is_whitespace()) {
        match part.trim().to_ascii_lowercase().as_str() {
            "" => {}
            "markdown" | "md" | "vitepress" => push_unique(&mut out, OutputFormat::Markdown),
            "html" => push_unique(&mut out, OutputFormat::Html),
            "all" => {
                push_unique(&mut out, OutputFormat::Markdown);
                push_unique(&mut out, OutputFormat::Html);
            }
            other => anyhow::bail!(
                "unknown --format `{other}` (expected a combination of markdown/md/vitepress, html joined by `+`, `,`, `;` or whitespace, or `all`)"
            ),
        }
    }
    if out.is_empty() {
        anyhow::bail!("--format must request at least one of: markdown, html");
    }
    Ok(out)
}

fn diagnostic_label(severity: clang::diagnostic::Severity) -> &'static str {
    match severity {
        clang::diagnostic::Severity::Ignored => "ignored",
        clang::diagnostic::Severity::Note => "note",
        clang::diagnostic::Severity::Warning => "warning",
        clang::diagnostic::Severity::Error => "error",
        clang::diagnostic::Severity::Fatal => "fatal",
    }
}

fn diagnostic_location(diag: &clang::diagnostic::Diagnostic, tu_path: &str) -> (String, u32, u32) {
    let spelling = diag.get_location().get_spelling_location();
    let path = spelling
        .file
        .map(|f| f.get_path().to_string_lossy().to_string())
        .unwrap_or_else(|| tu_path.to_string());
    (path, spelling.line, spelling.column)
}

fn format_fixit(fixit: &clang::diagnostic::FixIt) -> String {
    match fixit {
        clang::diagnostic::FixIt::Deletion(range) => {
            let s = range.get_start().get_spelling_location();
            let e = range.get_end().get_spelling_location();
            format!(
                "delete {}:{}:{}-{}:{}",
                s.file
                    .map(|f| f.get_path().to_string_lossy().to_string())
                    .unwrap_or_else(|| "<unknown>".into()),
                s.line,
                s.column,
                e.line,
                e.column
            )
        }
        clang::diagnostic::FixIt::Insertion(loc, text) => {
            let s = loc.get_spelling_location();
            format!(
                "insert {:?} at {}:{}:{}",
                text,
                s.file
                    .map(|f| f.get_path().to_string_lossy().to_string())
                    .unwrap_or_else(|| "<unknown>".into()),
                s.line,
                s.column
            )
        }
        clang::diagnostic::FixIt::Replacement(range, text) => {
            let s = range.get_start().get_spelling_location();
            let e = range.get_end().get_spelling_location();
            format!(
                "replace {}:{}:{}-{}:{} with {:?}",
                s.file
                    .map(|f| f.get_path().to_string_lossy().to_string())
                    .unwrap_or_else(|| "<unknown>".into()),
                s.line,
                s.column,
                e.line,
                e.column,
                text
            )
        }
    }
}

fn report_diagnostics(
    tu: &clang::TranslationUnit,
    tu_path: &str,
    seen: &mut std::collections::HashSet<String>,
) {
    const MAX_PER_TU: usize = 20;
    let mut shown = 0usize;
    let mut skipped_dup = 0usize;
    for diag in tu.get_diagnostics() {
        let severity = diag.get_severity();
        if severity < clang::diagnostic::Severity::Warning {
            continue;
        }
        let (path, line, column) = diagnostic_location(&diag, tu_path);
        let text = diag.get_text();
        let key = format!("{}:{line}:{column}:{}", path, text);
        if !seen.insert(key) {
            skipped_dup += 1;
            continue;
        }
        if shown >= MAX_PER_TU {
            continue;
        }
        shown += 1;
        let label = diagnostic_label(severity);
        if path == tu_path {
            eprintln!("[sleepy][parse {label}] {path}:{line}:{column}: {text}");
        } else {
            eprintln!("[sleepy][parse {label}] {path}:{line}:{column}: {text} (in TU {tu_path})");
        }
        for child in diag.get_children() {
            let (cpath, cline, ccolumn) = diagnostic_location(&child, tu_path);
            eprintln!(
                "  |- {} {}:{cline}:{ccolumn}: {}",
                diagnostic_label(child.get_severity()),
                cpath,
                child.get_text()
            );
        }
        for fixit in diag.get_fix_its().iter().take(3) {
            eprintln!("  |- fix-it: {}", format_fixit(fixit));
        }
    }
    if shown >= MAX_PER_TU {
        eprintln!("[sleepy][parse note] {tu_path}: further diagnostics suppressed (cap {MAX_PER_TU}/TU)");
    }
    if skipped_dup > 0 {
        eprintln!(
            "[sleepy][parse note] {tu_path}: {skipped_dup} duplicate diagnostic(s) already reported from other TUs"
        );
    }
}

#[allow(clippy::too_many_arguments)]
fn process_source(
    source: &data::config::SourceConfig,
    config: &data::config::SleepyConfig,
    index: &Index,
    gen: &generator::markdown::MarkdownGenerator,
    global_include_dirs: &[PathBuf],
    all_docs: &mut Vec<data::document::FileDocument>,
    pending_writes: &mut Vec<(PathBuf, String)>,
    file_infos: &mut HashMap<String, String>,
    diag_seen: &mut std::collections::HashSet<String>,
    want_markdown: bool,
) -> anyhow::Result<()> {
    println!("[sleepy] source: {}", source.name);
    let toolchain = match &source.compile_flags.compiler {
        Some(path) => {
            println!("[toolchain] using compiler from config: {}", path.display());
            toolchain::Toolchain::from_path(path)?
        }
        None => toolchain::Toolchain::detect()?,
    };
    let mut args = source
        .compile_flags
        .to_args(&source.include_dirs, &toolchain)?;
    // cross-library convenience: expose every scanned include root to -I
    for dir in global_include_dirs {
        args.push("-I".into());
        args.push(dir.to_string_lossy().to_string());
    }
    println!("[sleepy]   compiler args: {}", args.join(" "));

    let headers: Vec<PathBuf> = if !source.files.is_empty() {
        source
            .files
            .iter()
            .map(|f| f.canonicalize().unwrap_or_else(|_| f.clone()))
            .collect()
    } else {
        let mut h = Vec::new();
        for dir in &source.include_dirs {
            let canonical = dir
                .canonicalize()
                .with_context(|| format!("cannot resolve: {}", dir.display()))?;
            h.extend(gen_compile_db::collect_headers_pub(
                &canonical,
                &source.extensions,
            ));
        }
        h
    };

    // apply exclude_dirs / exclude_files filters
    let exclude_dirs: Vec<PathBuf> = source
        .exclude_dirs
        .iter()
        .filter_map(|d| d.canonicalize().ok())
        .collect();
    let exclude_files: HashSet<PathBuf> = source
        .exclude_files
        .iter()
        .filter_map(|f| f.canonicalize().ok())
        .collect();
    let headers: Vec<PathBuf> = headers
        .into_iter()
        .filter(|h| {
            let canon = h.canonicalize().unwrap_or_else(|_| h.clone());
            if exclude_files.contains(&canon) {
                return false;
            }
            !exclude_dirs.iter().any(|d| canon.starts_with(d))
        })
        .collect();

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
    let mut flat_seen: std::collections::HashMap<String, usize> = std::collections::HashMap::new();
    for (main_file, owned_files) in &owned_map {
        let file_str = main_file
            .to_str()
            .ok_or_else(|| anyhow::anyhow!("invalid path: {}", main_file.display()))?;

        let tu = index
            .parser(file_str)
            .arguments(&args)
            .incomplete(true)
            .skip_function_bodies(true)
            .detailed_preprocessing_record(true)
            .parse()
            .with_context(|| format!("failed to parse: {}", main_file.display()))?;

        report_diagnostics(&tu, file_str, diag_seen);

        let source_text = std::fs::read_to_string(main_file).unwrap_or_default();
        let nodoc_ranges = parser::collect_nodoc_ranges(&source_text);
        let include_root = source.include_dirs[0].canonicalize()?;

        let context = parser::ParseContext {
            include_root: include_root.to_str().unwrap_or(""),
            nodoc_ranges: &nodoc_ranges,
            owned_files,
            ignored_namespaces: &config.ignored_namespaces,
        };

        let mut doc = parser::build_file_document(&tu, file_str, &context);
        doc.source_name = source.name.clone();
        doc.rel_path = main_file
            .strip_prefix(&include_root)
            .map(|r| r.to_path_buf())
            .unwrap_or_else(|_| {
                main_file
                    .file_name()
                    .map(std::path::PathBuf::from)
                    .unwrap_or_else(|| std::path::PathBuf::from("unknown.hpp"))
            });
        if want_markdown {
            let md = gen.generate_file(&doc);

            // display name: "<source>/<relative path under include root>"
            let display = match main_file.strip_prefix(&include_root) {
                Ok(rel) => format!("{}/{}", source.name, rel.display()),
                Err(_) => format!(
                    "{}/{}",
                    source.name,
                    main_file
                        .file_name()
                        .map(|f| f.to_string_lossy().to_string())
                        .unwrap_or_else(|| "unknown".to_string())
                ),
            };
            file_infos.insert(main_file.to_string_lossy().to_string(), display);

            let out_name = main_file
                .file_stem()
                .and_then(|s| s.to_str())
                .unwrap_or("unknown");
            let out_path = if config.flat {
                let base = out_name.to_string();
                let count = flat_seen.entry(base.clone()).or_insert(0);
                *count += 1;
                let final_name = if *count > 1 {
                    format!("{}_{}", base, count)
                } else {
                    base
                };
                source_output_dir.join(format!("{}.md", final_name))
            } else {
                let include_root = source.include_dirs.get(0)
                    .and_then(|d| d.canonicalize().ok())
                    .unwrap_or_else(|| source.include_dirs.get(0).cloned().unwrap_or_default());
                let rel = main_file.strip_prefix(&include_root).unwrap_or(main_file.as_path());
                source_output_dir.join(rel.with_extension("md"))
            };
            if let Some(parent) = out_path.parent() {
                std::fs::create_dir_all(parent)?;
            }
            pending_writes.push((out_path.clone(), md));
            if std::env::var("SLEEPY_QUIET").as_deref() != Ok("1") {
                println!("[sleepy]   → {}", out_path.display());
            }
        }

        all_docs.push(doc);
    }

    Ok(())
}
