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
use std::path::{Path, PathBuf};
use serde::Deserialize;
use anyhow::{Context, Result};

#[derive(Debug, Deserialize)]
pub struct SleepyConfig {
    pub output_dir: PathBuf,
    #[serde(default = "default_lang")]
    pub lang: String,
    pub sources: Vec<SourceConfig>,
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

fn default_extensions() -> Vec<String> {
    vec!["h".into(), "hpp".into(), "hxx".into()]
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