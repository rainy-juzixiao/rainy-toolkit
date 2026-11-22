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
mod progress;
mod toolchain;
mod undocumented;
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

    /// 仅生成 reference 页面，不生成站点骨架 (reference pages only)
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

    let clang = Clang::new()
        .map_err(|e| anyhow::anyhow!("failed to initialize libclang: {e}"))?;
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
    validate_output_names(&config)?;

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
    // canonical file path -> display name (e.g. "<source>/<rel path>")
    let mut file_infos: HashMap<String, String> = HashMap::new();
    let mut diag_seen: std::collections::HashSet<String> = std::collections::HashSet::new();

    let source_headers: Vec<Vec<PathBuf>> = config
        .sources
        .iter()
        .map(|s| collect_source_headers(s))
        .collect::<anyhow::Result<_>>()?;
    let total: usize = source_headers.iter().map(|h| h.len()).sum();
    let mut progress = progress::Progress::new(total as u64);

    for (source, headers) in config.sources.iter().zip(source_headers) {
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
            headers,
            &mut progress,
        )?;
    }
    progress.finish();

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
    let mut written = 0usize;
    for (path, content) in &pending_writes {
        if let Some(parent) = path.parent() {
            std::fs::create_dir_all(parent)
                .with_context(|| format!("cannot create dir: {}", parent.display()))?;
        }
        std::fs::write(path, content)
            .with_context(|| format!("failed to write: {}", path.display()))?;
        written += 1;
    }

    // --- HTML (always run if requested, independent of docs count) ---
    if want_html {
        let html_gen =
            generator::html::HtmlGenerator::new("assets/html", config.lang.clone());
        if cli.partial {
            let ref_dir = config.output_dir.join("html").join("reference");
            written += html_gen.generate_reference_only(&all_docs, &ref_dir, config.flat)?;
            println!("[sleepy] HTML reference page at {}", ref_dir.display());
        } else {
            written +=
                html_gen.generate_site(&all_docs, &config.output_dir.join("html"), config.flat)?;
            println!("[sleepy] HTML site at {}", config.output_dir.join("html").display());
        }
    }

    if all_docs.is_empty() {
        println!("[sleepy] {} 个文件已写入 (files written)", written);
        return Ok(());
    }

    // --- dependency graph & index page ---
    if want_markdown && !cli.no_graph && config.dependency_graph.enabled {
        let graph_path = config
            .output_dir
            .join(format!("{}.md", config.dependency_graph.out_name));
        generator::reports::generate_dependency_graph(&graph, &graph_path, &config.lang)?;
        written += 1;
        println!("[sleepy] dependency graph → {}", graph_path.display());

        let index_path = config.output_dir.join("index.md");
        generator::reports::generate_index_page(
            &all_docs,
            &file_infos,
            &index_path,
            &config.lang,
        )?;
        written += 1;
        println!("[sleepy] index page → {}", index_path.display());
    }

    if want_markdown {
    // --- VitePress / markdown output ---
    let vp_gen = generator::rt_vitepress_markdown::VitePressMarkdownGenerator::new(&config.lang);
    if cli.partial {
        let ref_dir = config.output_dir.join("reference");
        written += vp_gen.generate_reference_only(&all_docs, &ref_dir)?;
        println!(
            "[sleepy] VitePress reference pages generated at {}",
            ref_dir.display()
        );
    } else {
        written += vp_gen.generate_site(&all_docs, &config.output_dir)?;
        println!(
            "[sleepy] VitePress site generated at {}",
            config.output_dir.join("docs").display()
        );
    }
    }

    println!("[sleepy] {} 个文件已写入 (files written)", written);

    Ok(())
}

fn validate_output_names(config: &data::config::SleepyConfig) -> anyhow::Result<()> {
    fn is_single_component(name: &str) -> bool {
        !name.is_empty()
            && name != "."
            && name != ".."
            && !name.contains('/')
            && !name.contains('\\')
            && !std::path::Path::new(name).is_absolute()
    }
    for source in &config.sources {
        if !is_single_component(&source.name) {
            anyhow::bail!(
                "invalid source name '{}': must be a single path component",
                source.name
            );
        }
    }
    let out_name = &config.dependency_graph.out_name;
    if !is_single_component(out_name) {
        anyhow::bail!(
            "invalid dependency_graph.out_name '{}': must be a single path component",
            out_name
        );
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

/// 被扫描到、却无法产出任何条目的头文件。
enum EmptyReason {
    /// 解析成功，但文件里没有 sleepy 会归档的实体（可能只是聚合头文件或前置声明）
    NoEntities,
    /// 解析到实体，但全部被 @notpublic / @hide 过滤掉
    AllFiltered,
    /// 文件不存在或无法读取
    Unreadable,
}

fn is_chinese(lang: &str) -> bool {
    lang.starts_with("zh")
        || lang.contains("chinese")
        || lang.contains("Chinese")
        || lang.contains("zh-Hans")
        || lang.contains("zh_hans")
}

/// 处理到该文件时立刻报告，不攒到最后汇总。
fn warn_empty_doc(display: &str, reason: EmptyReason, lang: &str) {
    let zh = is_chinese(lang);
    let hint = match (zh, reason) {
        (true, EmptyReason::NoEntities) => {
            "未解析到可归档的实体，确认它是否只是聚合头文件或前置声明，或整体位于 ignored_namespaces 中"
        }
        (true, EmptyReason::AllFiltered) => {
            "实体全部被 @notpublic/@hide 过滤，确认是否应从扫描范围排除"
        }
        (true, EmptyReason::Unreadable) => "文件不存在或无法读取，确认路径与权限",
        (false, EmptyReason::NoEntities) => {
            "no archivable entity was parsed; it may be only an umbrella/forward-declaration header or live entirely in ignored_namespaces"
        }
        (false, EmptyReason::AllFiltered) => {
            "every entity was filtered by @notpublic/@hide; consider excluding the file from the scan"
        }
        (false, EmptyReason::Unreadable) => {
            "the file is missing or unreadable; check its path and permissions"
        }
    };
    if zh {
        eprintln!("[sleepy warn] 未生成任何文档: {display}: {hint}");
    } else {
        eprintln!("[sleepy warn] produced no documentation: {display}: {hint}");
    }
}


fn warn_scope_diagnostics(
    display: &str,
    diagnostics: &[parser::scope::Diagnostic],
    lang: &str,
) {
    let zh = is_chinese(lang);
    for d in diagnostics {
        let hint = match (zh, d.kind) {
            (true, parser::scope::DiagnosticKind::MissingValue) => {
                format!("\\scope Var [{}] 缺少初值，应写成 \\scope Var [{}=<值>]", d.detail, d.detail)
            }
            (true, parser::scope::DiagnosticKind::UnknownGroup) => {
                format!("\\forscope 引用了未声明的组 [{}]", d.detail)
            }
            (true, parser::scope::DiagnosticKind::MissingGroupName) => {
                "\\forscope 没有组名，且之前没有声明任何 \\scope 组".to_string()
            }
            (true, parser::scope::DiagnosticKind::DuplicateGroup) => {
                format!("组 [{}] 被重复声明，后一次会覆盖前一次的模板注释", d.detail)
            }
            (false, parser::scope::DiagnosticKind::MissingValue) => {
                format!("\\scope Var [{}] has no initial value; write \\scope Var [{}=<value>]", d.detail, d.detail)
            }
            (false, parser::scope::DiagnosticKind::UnknownGroup) => {
                format!("\\forscope references undeclared group [{}]", d.detail)
            }
            (false, parser::scope::DiagnosticKind::MissingGroupName) => {
                "\\forscope has no group name and no \\scope group was declared".to_string()
            }
            (false, parser::scope::DiagnosticKind::DuplicateGroup) => {
                format!("group [{}] is declared more than once; the later one overrides the earlier template", d.detail)
            }
        };
        if zh {
            eprintln!("[sleepy warn] 批量定义组: {display}:{}: {hint}", d.line);
        } else {
            eprintln!("[sleepy warn] batch definition group: {display}:{}: {hint}", d.line);
        }
    }
}

fn collect_source_headers(source: &data::config::SourceConfig) -> anyhow::Result<Vec<PathBuf>> {
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
    Ok(headers
        .into_iter()
        .filter(|h| {
            let canon = h.canonicalize().unwrap_or_else(|_| h.clone());
            if exclude_files.contains(&canon) {
                return false;
            }
            !exclude_dirs.iter().any(|d| canon.starts_with(d))
        })
        .map(|h| h.canonicalize().unwrap_or(h))
        .collect())
}

/// 诊断里使用的文件路径：优先相对于当前工作目录，避免整条绝对路径挤满一行。
fn actual_path_display(path: &std::path::Path) -> String {
    let base = std::env::current_dir().ok();
    let shown = base
        .as_deref()
        .and_then(|base| path.strip_prefix(base).ok())
        .filter(|p| !p.as_os_str().is_empty())
        .unwrap_or(path);
    shown.display().to_string()
}

/// 单页 markdown 的输出路径；平铺模式下同名文件加编号。
fn markdown_out_path(
    config: &data::config::SleepyConfig,
    source_output_dir: &std::path::Path,
    main_file: &std::path::Path,
    rel_path: &std::path::Path,
    flat_seen: &mut HashMap<String, usize>,
) -> PathBuf {
    if !config.flat {
        return source_output_dir.join(rel_path.with_extension("md"));
    }
    let base = main_file
        .file_stem()
        .and_then(|s| s.to_str())
        .unwrap_or("unknown")
        .to_string();
    let count = flat_seen.entry(base.clone()).or_insert(0);
    *count += 1;
    let final_name = if *count > 1 {
        format!("{}_{}", base, count)
    } else {
        base
    };
    source_output_dir.join(format!("{}.md", final_name))
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
    headers: Vec<PathBuf>,
    progress: &mut progress::Progress,
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

    println!("[sleepy]   found {} files", headers.len());

    let source_output_dir = config.output_dir.join(&source.name);
    let include_root = source
        .include_dirs
        .first()
        .ok_or_else(|| anyhow::anyhow!("source '{}' has no include_dirs", source.name))?
        .canonicalize()
        .with_context(|| format!("cannot resolve include dir for source '{}'", source.name))?;
    let mut flat_seen: std::collections::HashMap<String, usize> = std::collections::HashMap::new();
    for header in &headers {
        let main_file = match std::fs::canonicalize(header) {
            Ok(p) => p,
            Err(_) => {
                progress.break_line();
                warn_empty_doc(
                    &actual_path_display(header),
                    EmptyReason::Unreadable,
                    &config.lang,
                );
                progress.tick(&header.display().to_string());
                continue;
            }
        };
        let rel_path = main_file
            .strip_prefix(&include_root)
            .map(|r| r.to_path_buf())
            .unwrap_or_else(|_| {
                main_file
                    .file_name()
                    .map(std::path::PathBuf::from)
                    .unwrap_or_else(|| std::path::PathBuf::from("unknown.hpp"))
            });
        let display = format!("{}/{}", source.name, rel_path.display());

        let source_text = match std::fs::read_to_string(&main_file) {
            Ok(text) => text,
            Err(_) => {
                progress.break_line();
                warn_empty_doc(
                    &actual_path_display(&main_file),
                    EmptyReason::Unreadable,
                    &config.lang,
                );
                progress.tick(&rel_path.display().to_string());
                continue;
            }
        };
        // 文件级 @hide / @notpublic：伞头与实现文件整体排除，不解析也不产出页面
        let markers = parser::read_file_markers(&source_text);
        if markers.hide || markers.not_public {
            progress.tick(&rel_path.display().to_string());
            continue;
        }
        // @umbrella：不解析实体，只读取 #include 列表生成一页
        if markers.umbrella {
            let file_str = main_file.to_string_lossy().to_string();
            let doc = parser::build_umbrella_document(
                &file_str,
                &source.name,
                rel_path.clone(),
                &source_text,
            );
            file_infos.insert(file_str, display.clone());
            if want_markdown {
                let md = gen.generate_file(&doc);
                let out_path = markdown_out_path(
                    config,
                    &source_output_dir,
                    &main_file,
                    &rel_path,
                    &mut flat_seen,
                );
                pending_writes.push((out_path, md));
            }
            all_docs.push(doc);
            progress.tick(&rel_path.display().to_string());
            continue;
        }

        let owned_files: HashSet<PathBuf> = [main_file.clone()].into_iter().collect();
        let main_file = &main_file;
        let owned_files = &owned_files;
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

        parser::scope::install(&source_text);
        let nodoc_ranges = parser::collect_nodoc_ranges(&source_text);

        let context = parser::ParseContext {
            include_root: include_root.to_str().unwrap_or(""),
            nodoc_ranges: &nodoc_ranges,
            owned_files,
            ignored_namespaces: &config.ignored_namespaces,
        };

        let mut doc = parser::build_file_document(&tu, file_str, &context);
        let scope_diagnostics = parser::scope::take_diagnostics();
        parser::scope::clear();
        if !scope_diagnostics.is_empty() {
            progress.break_line();
            warn_scope_diagnostics(
                &actual_path_display(main_file),
                &scope_diagnostics,
                &config.lang,
            );
        }
        doc.source_name = source.name.clone();
        doc.rel_path = rel_path.clone();
        file_infos.insert(main_file.to_string_lossy().to_string(), display.clone());

        if !doc.has_visible_entities() {
            let reason = if doc.entity_count() == 0 {
                EmptyReason::NoEntities
            } else {
                EmptyReason::AllFiltered
            };
            progress.break_line();
            warn_empty_doc(&actual_path_display(main_file), reason, &config.lang);
        }

        let undocumented = undocumented::collect(&doc);
        if !undocumented.is_empty() {
            progress.break_line();
            undocumented::warn(
                &actual_path_display(main_file),
                &undocumented,
                &config.lang,
            );
        }

        if want_markdown {
            let md = gen.generate_file(&doc);
            let out_path = markdown_out_path(
                config,
                &source_output_dir,
                main_file,
                &doc.rel_path,
                &mut flat_seen,
            );
            pending_writes.push((out_path, md));
        }

        progress.tick(&doc.rel_path.display().to_string());
        all_docs.push(doc);
    }

    Ok(())
}
