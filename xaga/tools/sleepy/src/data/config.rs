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
use std::collections::HashSet;
use std::path::{Path, PathBuf};
use serde::Deserialize;
use anyhow::{Context, Result};

#[derive(Debug, Deserialize)]
pub struct SleepyConfig {
    pub output_dir: PathBuf,
    #[serde(default = "default_lang")]
    pub lang: String,
    #[serde(default)]
    pub sources: Vec<SourceConfig>,
    /// 要忽略的命名空间（默认：implements, detail, impl）
    #[serde(default = "default_ignored_namespaces")]
    pub ignored_namespaces: Vec<String>,
    /// 引用其它目录的 sleepy 配置（相对当前配置文件所在目录）
    #[serde(default)]
    pub includes: Vec<PathBuf>,
    /// 依赖图与循环依赖检查配置
    #[serde(default)]
    pub dependency_graph: DependencyGraphConfig,
    /// true = 平铺输出（同名加编号）；false = 保留目录结构（如 rainy/core/）
    #[serde(default = "default_false")]
    pub flat: bool,
}

/// 依赖图生成与循环依赖检查选项。
///
/// 根配置中的该节决定全局行为；被引用配置中的同名节点会被忽略。
#[derive(Debug, Deserialize, Clone)]
pub struct DependencyGraphConfig {
    /// 是否生成依赖图与索引页（默认 true）
    #[serde(default = "default_true")]
    pub enabled: bool,
    /// 依赖图输出文件名（不含扩展名，默认 "dependency_graph"）
    #[serde(default = "default_graph_out_name")]
    pub out_name: String,
    /// 发现循环依赖时是否视为致命错误（默认 true；可用 --allow-cycles 临时放行）
    #[serde(default = "default_true")]
    pub fail_on_cycle: bool,
    /// 豁免的包含边，格式 "a.hpp -> b.hpp"（相对 include 根的显示名）
    #[serde(default)]
    pub ignore: Vec<String>,
}

impl Default for DependencyGraphConfig {
    fn default() -> Self {
        Self {
            enabled: true,
            out_name: default_graph_out_name(),
            fail_on_cycle: true,
            ignore: Vec::new(),
        }
    }
}

impl SleepyConfig {
    /// 空配置：用于菱形引用（已被合并过的配置静默跳过时）占位。
    fn empty() -> Self {
        Self {
            output_dir: PathBuf::new(),
            lang: default_lang(),
            sources: Vec::new(),
            ignored_namespaces: Vec::new(),
            includes: Vec::new(),
            dependency_graph: DependencyGraphConfig::default(),
            flat: false,
        }
    }
}

#[derive(Debug, Deserialize)]
pub struct SourceConfig {
    pub name: String,
    /// 所有要递归扫描的头文件目录
    pub include_dirs: Vec<PathBuf>,
    /// 要处理的文件后缀
    #[serde(default = "default_extensions")]
    pub extensions: Vec<String>,
    /// 编译参数，与 compile_commands 完全解耦
    #[serde(default)]
    pub compile_flags: CompileFlags,
    #[serde(default)]
    pub files: Vec<PathBuf>,
    /// 排除的目录（相对当前配置文件；其下所有头文件不参与生成）
    #[serde(default)]
    pub exclude_dirs: Vec<PathBuf>,
    /// 排除的单个文件（相对当前配置文件）
    #[serde(default)]
    pub exclude_files: Vec<PathBuf>,
}

#[derive(Debug, Deserialize, Default)]
pub struct CompileFlags {
    pub std: Option<String>,
    #[serde(default)]
    pub defines: Vec<String>,
    #[serde(default)]
    pub includes: Vec<PathBuf>,
    #[serde(default)]
    pub extra: Vec<String>,
    pub inherit_from: Option<PathBuf>,
    /// 手动指定的编译器路径，覆盖自动探测
    /// e.g. "/usr/bin/clang++-17" 或 "C:/LLVM/bin/clang++.exe"
    pub compiler: Option<PathBuf>,
}

impl CompileFlags {
    pub fn to_args(
        &self,
        include_dirs: &[PathBuf],
        toolchain: &crate::toolchain::Toolchain,
    ) -> Result<Vec<String>> {
        let mut paired: Vec<(String, String)> = Vec::new(); // (-isystem, path)
        let mut single: Vec<String> = Vec::new();
        let tc_args = toolchain.to_args();
        let mut tc_iter = tc_args.into_iter().peekable();
        while let Some(arg) = tc_iter.next() {
            if arg == "-isystem" || arg == "-I" || arg == "-include" {
                if let Some(val) = tc_iter.next() {
                    paired.push((arg, val));
                }
            } else {
                single.push(arg);
            }
        }
        // 继承参数
        if let Some(inherit_path) = &self.inherit_from {
            let inherited = extract_reusable_args(inherit_path)?;
            let mut iter = inherited.into_iter().peekable();
            while let Some(arg) = iter.next() {
                if arg == "-isystem" || arg == "-include" {
                    if let Some(val) = iter.next() {
                        paired.push((arg, val));
                    }
                } else {
                    single.push(arg);
                }
            }
        }

        // -std
        if let Some(std) = &self.std {
            single.retain(|a| !a.starts_with("-std="));
            single.push(format!("-std={}", std));
        }
        // include_dirs → -I
        for dir in include_dirs {
            let canonical = dir.canonicalize()
                .with_context(|| format!("cannot resolve: {}", dir.display()))?;
            paired.push(("-I".into(), canonical.to_string_lossy().to_string()));
        }
        for inc in &self.includes {
            paired.push(("-I".into(), inc.to_string_lossy().to_string()));
        }
        // 始终注入 __MUZIYAN_IS_HERE__，让头文件能检测到正在被 sleepy 解析
        single.push("-D__MUZIYAN_IS_HERE__".into());

        // -D
        for def in &self.defines {
            if def.starts_with('-') {
                single.push(def.clone());
            } else {
                single.push(format!("-D{}", def));
            }
        }
        // extra
        single.extend(self.extra.clone());
        let mut seen_pairs: std::collections::HashSet<String> = std::collections::HashSet::new();
        let mut seen_singles: std::collections::HashSet<String> = std::collections::HashSet::new();
        let mut result: Vec<String> = Vec::new();
        result.push("-x".into());
        result.push("c++-header".into());
        for a in &single {
            if a == "-target" { continue; } // 从 single 里跳过，下面单独加
        }
        if let Some(target) = &toolchain.target {
            result.push("-target".into());
            result.push(target.clone());
        }
        for (flag, val) in paired {
            let key = format!("{}={}", flag, val);
            if seen_pairs.insert(key) {
                result.push(flag);
                result.push(val);
            }
        }
        for a in single {
            if a == "-target" { continue; }
            if seen_singles.insert(a.clone()) {
                result.push(a);
            }
        }

        Ok(result)
    }
}

fn extract_reusable_args(path: &Path) -> Result<Vec<String>> {
    let content = std::fs::read_to_string(path)
        .with_context(|| format!("cannot read {}", path.display()))?;
    let entries: Vec<serde_json::Value> = serde_json::from_str(&content)
        .context("failed to parse compile_commands.json")?;
    for entry in &entries {
        let raw_args: Vec<String> = if let Some(args) = entry.get("arguments").and_then(|a| a.as_array()) {
            args.iter().filter_map(|v| v.as_str().map(|s| s.to_string())).collect()
        } else if let Some(cmd) = entry.get("command").and_then(|c| c.as_str()) {
            shell_words::split(cmd).context("failed to parse command string")?
        } else {
            continue;
        };
        let reusable: Vec<String> = raw_args.into_iter()
            .filter(|a| is_reusable_arg(a))
            .collect();
        if !reusable.is_empty() {
            return Ok(reusable);
        }
    }
    Ok(vec![])
}

fn is_reusable_arg(arg: &str) -> bool {
    arg.starts_with("-std=")
        || arg.starts_with("-I")
        || arg.starts_with("-D")
        || arg.starts_with("-isystem")
        || arg.starts_with("-include")
        || arg.starts_with("-f")
        || arg.starts_with("-W")
        || arg == "-nostdinc"
        || arg == "-nostdinc++"
        || arg == "-m32"
        || arg == "-m64"
}

fn default_lang() -> String { "english".into() }

fn default_false() -> bool { false }

fn default_true() -> bool { true }

fn default_graph_out_name() -> String { "dependency_graph".into() }

fn default_ignored_namespaces() -> Vec<String> {
    vec!["implements".into(), "detail".into(), "impl".into()]
}

fn default_extensions() -> Vec<String> {
    vec!["h".into(), "hpp".into(), "hxx".into()]
}

/// 加载配置并递归合并 `includes` 引用。
///
/// 合并语义：
/// - `sources`：被引用配置的 sources 会被追加，路径相对**该配置文件自身所在目录**重写；
/// - `ignored_namespaces`：取并集；
/// - `output_dir` / `lang` / `dependency_graph`：以根配置为准；
/// - 显式引用链成环 → 报错并打印完整引用链；
/// - 菱形引用（同一配置被引用多次）→ 静默跳过。
pub fn load_with_includes(path: &Path) -> Result<SleepyConfig> {
    let mut stack: Vec<(PathBuf, String)> = Vec::new();
    let mut visited: HashSet<PathBuf> = HashSet::new();
    let mut config = merge_config_tree(path, &mut stack, &mut visited)?;

    // output_dir 相对根配置文件所在目录解析（-c 指向别处时输出也落在配置旁）
    if config.output_dir.is_relative() {
        if let Some(dir) = path.canonicalize().ok().and_then(|p| p.parent().map(|d| d.to_path_buf())) {
            config.output_dir = dir.join(&config.output_dir);
        }
    }
    Ok(config)
}

fn merge_config_tree(
    path: &Path,
    stack: &mut Vec<(PathBuf, String)>,
    visited: &mut HashSet<PathBuf>,
) -> Result<SleepyConfig> {
    let canonical = path
        .canonicalize()
        .with_context(|| format!("cannot resolve config: {}", path.display()))?;

    // 显式引用栈：真环检测
    if let Some(pos) = stack.iter().position(|(p, _)| *p == canonical) {
        let mut chain: Vec<String> = stack[pos..]
            .iter()
            .map(|(_, name)| name.clone())
            .collect();
        chain.push(path.display().to_string());
        anyhow::bail!(
            "检测到配置文件循环引用 (circular config includes):\n  {}\n  cycle closes here",
            chain.join(" -> ")
        );
    }
    // 菱形引用：已合并过则静默跳过
    if !visited.insert(canonical.clone()) {
        return Ok(SleepyConfig::empty());
    }

    let base = load(path)?;
    let mut merged = base;
    let config_dir = canonical
        .parent()
        .map(|p| p.to_path_buf())
        .unwrap_or_else(|| PathBuf::from("."));

    // 递归合并 includes（路径相对当前配置文件所在目录）
    let includes = merged.includes.clone();
    for inc in &includes {
        let inc_path = config_dir.join(inc);
        stack.push((canonical.clone(), path.display().to_string()));
        let child = merge_config_tree(&inc_path, stack, visited);
        stack.pop();
        let child = child?;
        for src in child.sources {
            merged.sources.push(src);
        }
        for ns in child.ignored_namespaces {
            if !merged.ignored_namespaces.contains(&ns) {
                merged.ignored_namespaces.push(ns);
            }
        }
    }

    // 本层 sources 路径相对当前配置文件所在目录重写
    for src in &mut merged.sources {
        rewrite_source_paths(src, &config_dir);
    }
    Ok(merged)
}

fn rewrite_relative(config_dir: &Path, p: &Path) -> PathBuf {
    if p.is_absolute() {
        p.to_path_buf()
    } else {
        config_dir.join(p)
    }
}

fn rewrite_source_paths(src: &mut SourceConfig, config_dir: &Path) {
    for d in &mut src.include_dirs {
        *d = rewrite_relative(config_dir, d);
    }
    for f in &mut src.files {
        *f = rewrite_relative(config_dir, f);
    }
    for d in &mut src.exclude_dirs {
        *d = rewrite_relative(config_dir, d);
    }
    for f in &mut src.exclude_files {
        *f = rewrite_relative(config_dir, f);
    }
    for i in &mut src.compile_flags.includes {
        *i = rewrite_relative(config_dir, i);
    }
    if let Some(inherit) = &mut src.compile_flags.inherit_from {
        *inherit = rewrite_relative(config_dir, inherit);
    }
    if let Some(compiler) = &mut src.compile_flags.compiler {
        *compiler = rewrite_relative(config_dir, compiler);
    }
}

pub fn load(path: &Path) -> Result<SleepyConfig> {
    let content = std::fs::read_to_string(path)
        .with_context(|| format!("cannot read config: {}", path.display()))?;

    let ext = path.extension().and_then(|e| e.to_str()).unwrap_or("");
    match ext {
        "json" => serde_json::from_str(&content).context("failed to parse JSON config"),
        "yaml" | "yml" => serde_yaml::from_str(&content).context("failed to parse YAML config"),
        other => anyhow::bail!("unsupported config format: .{}", other),
    }
}

pub fn find_config() -> Option<PathBuf> {
    ["sleepy.json", "sleepy.yaml", "sleepy.yml"]
        .iter()
        .map(PathBuf::from)
        .find(|p| p.exists())
}
