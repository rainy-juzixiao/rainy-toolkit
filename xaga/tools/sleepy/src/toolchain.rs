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
use anyhow::{Context, Result};
use std::path::PathBuf;
use std::process::Command;

#[derive(Debug, Clone)]
pub struct Toolchain {
    pub compiler: PathBuf,
    pub kind: CompilerKind,
    pub system_includes: Vec<PathBuf>,
    pub target: Option<String>,
}

#[derive(Debug, Clone, PartialEq)]
pub enum CompilerKind {
    Clang,
    GCC,
    MSVC,
}

impl Toolchain {
    pub fn detect() -> Result<Self> {
        // 优先 clang++，因为 libclang 和 clang 最兼容
        let candidates: &[(&str, CompilerKind)] = &[
            ("clang++", CompilerKind::Clang),
            ("clang++-17", CompilerKind::Clang),
            ("clang++-16", CompilerKind::Clang),
            ("clang++-15", CompilerKind::Clang),
            ("g++", CompilerKind::GCC),
            ("g++-13", CompilerKind::GCC),
            ("g++-12", CompilerKind::GCC),
            ("cl", CompilerKind::MSVC),
        ];
        for (name, kind) in candidates {
            if let Ok(path) = which::which(name) {
                println!("[toolchain] detected: {} at {}", name, path.display());
                match Self::probe(&path, kind.clone()) {
                    Ok(tc) => return Ok(tc),
                    Err(e) => {
                        eprintln!("[toolchain] probe failed for {}: {}", name, e);
                        continue;
                    }
                }
            }
        }

        anyhow::bail!("no C++ compiler found in PATH, please set 'compiler' in config")
    }

    pub fn from_path(compiler: &PathBuf) -> Result<Self> {
        let kind = detect_kind(compiler);
        Self::probe(compiler, kind)
    }

    fn probe(compiler: &PathBuf, kind: CompilerKind) -> Result<Self> {
        let system_includes = match kind {
            CompilerKind::Clang | CompilerKind::GCC => probe_gcc_clang_includes(compiler)?,
            CompilerKind::MSVC => probe_msvc_includes(compiler)?,
        };

        let target = probe_target(compiler, &kind);

        println!("[toolchain] system includes ({}):", system_includes.len());
        for p in &system_includes {
            println!("[toolchain]   {}", p.display());
        }

        Ok(Self {
            compiler: compiler.clone(),
            kind,
            system_includes,
            target,
        })
    }
    pub fn to_args(&self) -> Vec<String> {
        let mut args = Vec::new();
        for inc in &self.system_includes {
            args.push("-isystem".into());
            args.push(inc.to_string_lossy().to_string()); // 每个路径单独一行
        }
        args
    }
}

fn probe_gcc_clang_includes(compiler: &PathBuf) -> Result<Vec<PathBuf>> {
    let null_file = if cfg!(windows) { "NUL" } else { "/dev/null" };
    let output = Command::new(compiler)
        .args(["-v", "-x", "c++", "-E", null_file])
        .output()
        .with_context(|| format!("failed to run {}", compiler.display()))?;
    let stderr = String::from_utf8_lossy(&output.stderr);
    parse_gcc_include_paths(&stderr)
}

fn parse_gcc_include_paths(stderr: &str) -> Result<Vec<PathBuf>> {
    let mut paths = Vec::new();
    let mut in_section = false;
    for line in stderr.lines() {
        let trimmed = line.trim();
        if trimmed == "#include <...> search starts here:"
            || trimmed == "#include \"...\" search starts here:"
        {
            in_section = true;
            continue;
        }
        if trimmed == "End of search list." {
            break;
        }

        if in_section && !trimmed.is_empty() {
            let path_str = trimmed
                .trim_end_matches(" (framework directory)")
                .trim_end_matches(" (headermap)")
                .trim();
            let p = PathBuf::from(path_str);
            if p.exists() {
                paths.push(p);
            }
        }
    }

    Ok(paths)
}

fn probe_msvc_includes(compiler: &PathBuf) -> Result<Vec<PathBuf>> {
    let mut paths = Vec::new();
    if let Ok(include_env) = std::env::var("INCLUDE") {
        for p in include_env.split(';') {
            let path = PathBuf::from(p.trim());
            if path.exists() {
                paths.push(path);
            }
        }
        if !paths.is_empty() {
            return Ok(paths);
        }
    }
    if let Ok(vswhere) = which::which("vswhere") {
        let output = Command::new(vswhere)
            .args(["-latest", "-property", "installationPath"])
            .output();

        if let Ok(out) = output {
            let vs_path = String::from_utf8_lossy(&out.stdout).trim().to_string();
            if !vs_path.is_empty() {
                // 遍历 VC/Tools/MSVC/<version>/include
                let msvc_base = PathBuf::from(&vs_path)
                    .join("VC")
                    .join("Tools")
                    .join("MSVC");
                if let Ok(entries) = std::fs::read_dir(&msvc_base) {
                    // 取最新版本（按字母序最大）
                    let mut versions: Vec<_> = entries
                        .filter_map(|e| e.ok())
                        .filter(|e| e.path().is_dir())
                        .collect();
                    versions.sort_by_key(|e| e.file_name());
                    if let Some(latest) = versions.last() {
                        let inc = latest.path().join("include");
                        if inc.exists() {
                            paths.push(inc);
                        }
                        let atl = latest.path().join("atlmfc").join("include");
                        if atl.exists() {
                            paths.push(atl);
                        }
                    }
                }

                // Windows SDK include
                let sdk_base = PathBuf::from("C:/Program Files (x86)/Windows Kits/10/Include");
                if sdk_base.exists() {
                    if let Ok(entries) = std::fs::read_dir(&sdk_base) {
                        let mut versions: Vec<_> = entries
                            .filter_map(|e| e.ok())
                            .filter(|e| e.path().is_dir())
                            .collect();
                        versions.sort_by_key(|e| e.file_name());
                        if let Some(latest) = versions.last() {
                            for sub in &["ucrt", "um", "shared", "winrt", "cppwinrt"] {
                                let p = latest.path().join(sub);
                                if p.exists() {
                                    paths.push(p);
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    if paths.is_empty() {
        anyhow::bail!(
            "cannot find MSVC include paths. \
             Run sleepy from a Visual Studio Developer Command Prompt, \
             or set 'compiler' and ensure vcvarsall.bat has been called"
        );
    }

    Ok(paths)
}

fn detect_kind(compiler: &PathBuf) -> CompilerKind {
    let name = compiler
        .file_name()
        .and_then(|n| n.to_str())
        .unwrap_or("")
        .to_lowercase();

    if name.contains("clang") {
        CompilerKind::Clang
    } else if name.contains("cl") && !name.contains("clang") {
        CompilerKind::MSVC
    } else {
        CompilerKind::GCC
    }
}

fn probe_target(compiler: &PathBuf, kind: &CompilerKind) -> Option<String> {
    if matches!(kind, CompilerKind::MSVC) {
        return None; // MSVC 不用 --target
    }
    let out = Command::new(compiler).arg("-dumpmachine").output().ok()?;
    let target = String::from_utf8_lossy(&out.stdout).trim().to_string();
    if target.is_empty() {
        None
    } else {
        Some(target)
    }
}
