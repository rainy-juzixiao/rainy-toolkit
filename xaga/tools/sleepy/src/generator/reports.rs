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

//! Shared include-graph structures used by BOTH the dependency-graph
//! rendering (this file) and the cycle detection (`super::cycles`).
//!
//! Nodes are keyed by a *display name* — the file path relative to its
//! source's include root, prefixed with the source name, e.g.
//! `"lib-alpha/core.hpp"`. External includes (system headers or headers
//! that do not belong to any scanned source) are kept separately in
//! `external_nodes` / `external_edges` so the cycle detector only ever
//! walks internal edges.

use crate::data::document::FileDocument;
use anyhow::Context;
use std::collections::{BTreeMap, BTreeSet, HashMap};
use std::fmt::Write as _;
use std::path::Path;

#[derive(Debug, Clone)]
pub struct NodeInfo {
    /// Which source (library) this file belongs to.
    pub source_name: String,
}

/// Internal + external include graph over the scanned files.
#[derive(Debug, Default)]
pub struct IncludeGraph {
    /// display name -> node info (files that are part of the scan)
    pub nodes: BTreeMap<String, NodeInfo>,
    /// internal edges: (from, to), both display names present in `nodes`
    pub edges: BTreeSet<(String, String)>,
    /// display names of includes that could not be resolved internally
    pub external_nodes: BTreeSet<String>,
    /// external edges: (from, external include name)
    pub external_edges: BTreeSet<(String, String)>,
}

impl IncludeGraph {
    pub fn internal_edge_count(&self) -> usize {
        self.edges.len()
    }
}

/// Parse ignore entries of the form `"a.hpp -> b.hpp"` into edge pairs.
pub fn parse_ignored_edges(raw: &[String]) -> BTreeSet<(String, String)> {
    let mut out = BTreeSet::new();
    for entry in raw {
        if let Some((from, to)) = entry.split_once("->") {
            let from = normalize_include(from);
            let to = normalize_include(to);
            if !from.is_empty() && !to.is_empty() {
                out.insert((from, to));
            }
        }
    }
    out
}

fn normalize_include(s: &str) -> String {
    s.trim()
        .trim_matches(|c| c == '"' || c == '<' || c == '>')
        .trim()
        .to_string()
}

/// Options for building the graph.
pub struct GraphBuildOptions<'a> {
    /// canonical file path -> display name (e.g. `/abs/inc/core.hpp` -> `lib-alpha/core.hpp`)
    pub file_infos: &'a HashMap<String, String>,
    /// ignore entries from config (edges to exclude from the graph entirely)
    pub ignored_edges: BTreeSet<(String, String)>,
}

fn strip_source_prefix(name: &str) -> &str {
    name.split_once('/').map(|(_, r)| r).unwrap_or(name)
}

/// An edge is ignored if it matches either the full display names
/// ("lib-a/engine.hpp -> lib-b/vehicle.hpp") or the bare relative names
/// ("engine.hpp -> vehicle.hpp").
fn is_ignored(from: &str, to: &str, ignored: &BTreeSet<(String, String)>) -> bool {
    ignored.contains(&(from.to_string(), to.to_string()))
        || ignored.contains(&(
            strip_source_prefix(from).to_string(),
            strip_source_prefix(to).to_string(),
        ))
}

/// Build the include graph from parsed file documents.
///
/// Resolution of an include written as `name` inside a file:
/// 1. exact match against a known display name;
/// 2. unique suffix match (`display.ends_with("/name")`); among multiple
///    suffix matches prefer the same source, then the shortest name;
/// 3. otherwise the include is external.
pub fn build_include_graph(
    docs: &[FileDocument],
    opts: &GraphBuildOptions<'_>,
) -> IncludeGraph {
    let mut graph = IncludeGraph::default();

    for (_canon, display) in opts.file_infos {
        let source_name = display
            .split_once('/')
            .map(|(s, _)| s.to_string())
            .unwrap_or_default();
        graph
            .nodes
            .entry(display.clone())
            .or_insert_with(|| NodeInfo { source_name });
    }

    // Pre-compute basename -> display names for suffix resolution.
    let mut by_name: HashMap<String, Vec<String>> = HashMap::new();
    for display in graph.nodes.keys() {
        if let Some(base) = display.rsplit('/').next() {
            by_name.entry(base.to_string()).or_default().push(display.clone());
        }
    }

    for doc in docs {
        let from_canon = doc.file_path.as_str();
        let from = match opts.file_infos.get(from_canon) {
            Some(d) => d.clone(),
            None => continue,
        };
        for inc in &doc.includes {
            let name = normalize_include(inc);
            if name.is_empty() {
                continue;
            }
            if let Some(to) = resolve_include(&name, &graph.nodes, &by_name) {
                if is_ignored(&from, &to, &opts.ignored_edges) {
                    continue;
                }
                graph.edges.insert((from.clone(), to));
            } else {
                if is_ignored(&from, &name, &opts.ignored_edges) {
                    continue;
                }
                graph.external_nodes.insert(name.clone());
                graph.external_edges.insert((from.clone(), name));
            }
        }
    }

    graph
}

fn resolve_include(
    name: &str,
    nodes: &BTreeMap<String, NodeInfo>,
    by_name: &HashMap<String, Vec<String>>,
) -> Option<String> {
    // 1. exact display-name match
    if nodes.contains_key(name) {
        return Some(name.to_string());
    }
    // 2. path-suffix match: display ends with "/{name}" (handles includes
    //    written relative to another library's include root)
    let suffix: Vec<String> = nodes
        .keys()
        .filter(|d| d.ends_with(&format!("/{}", name)))
        .cloned()
        .collect();
    let candidates = if !suffix.is_empty() {
        suffix
    } else {
        // 3. basename fallback (include written as bare "vehicle.hpp")
        by_name.get(name.rsplit('/').next().unwrap_or(name))?.clone()
    };
    match candidates.len() {
        0 => None,
        1 => Some(candidates[0].clone()),
        _ => {
            // multiple files share the basename: shortest display name wins
            let mut best: Option<&String> = None;
            for c in &candidates {
                if best.map(|b| c.len() < b.len()).unwrap_or(true) {
                    best = Some(c);
                }
            }
            best.cloned()
        }
    }
}

// ---------------------------------------------------------------------------
// Rendering: dependency graph page + index page
// ---------------------------------------------------------------------------

fn is_chinese(lang: &str) -> bool {
    lang.starts_with("zh")
        || lang.contains("chinese")
        || lang.contains("Chinese")
        || lang.contains("zh-Hans")
        || lang.contains("zh_hans")
}

/// Generate the dependency graph markdown page (Mermaid + edge list).
pub fn generate_dependency_graph(
    graph: &IncludeGraph,
    out_path: &Path,
    lang: &str,
) -> anyhow::Result<()> {
    let zh = is_chinese(lang);
    let title = if zh { "依赖图" } else { "Dependency Graph" };
    let legend_internal = if zh { "内部包含（实线）" } else { "internal include (solid)" };
    let legend_external = if zh { "外部依赖（虚线）" } else { "external include (dashed)" };
    let edge_list_title = if zh { "边列表" } else { "Edge list" };
    let module_title = if zh { "模块概览" } else { "Module overview" };

    let mut out = String::new();
    let _ = writeln!(out, "# {title}");
    let _ = writeln!(out);

    // ---- module overview table ----
    let mut module_files: BTreeMap<&str, Vec<&str>> = BTreeMap::new();
    for name in graph.nodes.keys() {
        let (src, rest) = name.split_once('/').unwrap_or(("", name.as_str()));
        module_files.entry(src).or_default().push(rest);
    }
    let _ = writeln!(out, "## {module_title}");
    let _ = writeln!(out);
    let files_hdr = if zh { "文件数" } else { "Files" };
    let module_hdr = if zh { "模块" } else { "Module" };
    let _ = writeln!(out, "| {module_hdr} | {files_hdr} |");
    let _ = writeln!(out, "|---|---|");
    for (src, files) in &module_files {
        let _ = writeln!(out, "| **{src}** | {} |", files.len());
    }
    let _ = writeln!(out);

    // ---- mermaid diagram ----
    let _ = writeln!(out, "```mermaid");
    let _ = writeln!(out, "graph LR");
    for (i, name) in graph.nodes.keys().enumerate() {
        let _ = writeln!(out, "  n{i}[\"{name}\"]");
    }
    for (i, name) in graph.external_nodes.iter().enumerate() {
        let _ = writeln!(out, "  e{i}[\"{name}\"]:::external");
    }

    let mut node_ids: HashMap<&String, String> = HashMap::new();
    for (i, name) in graph.nodes.keys().enumerate() {
        node_ids.insert(name, format!("n{i}"));
    }
    for (i, name) in graph.external_nodes.iter().enumerate() {
        node_ids.insert(name, format!("e{i}"));
    }

    for (from, to) in &graph.edges {
        let f = node_ids.get(from).cloned().unwrap_or_default();
        let t = node_ids.get(to).cloned().unwrap_or_default();
        let _ = writeln!(out, "  {f} --> {t}");
    }
    for (from, to) in &graph.external_edges {
        let f = node_ids.get(from).cloned().unwrap_or_default();
        let t = node_ids.get(to).cloned().unwrap_or_default();
        let _ = writeln!(out, "  {f} -.-> {t}");
    }
    let _ = writeln!(out, "  classDef external fill:#f9f,stroke:#333,stroke-dasharray: 5 5;");
    let _ = writeln!(out, "```");
    let _ = writeln!(out);
    let _ = writeln!(out, "> {legend_internal} / {legend_external}");
    let _ = writeln!(out);

    // ---- plain edge list ----
    let _ = writeln!(out, "## {edge_list_title}");
    let _ = writeln!(out);
    let _ = writeln!(out, "```text");
    for (from, to) in &graph.edges {
        let _ = writeln!(out, "{from} -> {to}");
    }
    for (from, to) in &graph.external_edges {
        let _ = writeln!(out, "{from} -> {to} (external)");
    }
    let _ = writeln!(out, "```");
    let _ = writeln!(out);

    if let Some(parent) = out_path.parent() {
        std::fs::create_dir_all(parent)?;
    }
    std::fs::write(out_path, out)
        .with_context(|| format!("failed to write: {}", out_path.display()))?;
    Ok(())
}

/// Generate a simple index page grouping generated files per source.
pub fn generate_index_page(
    docs: &[FileDocument],
    file_infos: &HashMap<String, String>,
    out_path: &Path,
    lang: &str,
) -> anyhow::Result<()> {
    let zh = is_chinese(lang);
    let title = if zh { "文档索引" } else { "Documentation Index" };
    let file_hdr = if zh { "文件" } else { "File" };
    let desc_hdr = if zh { "简介" } else { "Description" };

    let mut out = String::new();
    let _ = writeln!(out, "# {title}");
    let _ = writeln!(out);

    // group docs by source name (display prefix before the first '/')
    let mut per_source: BTreeMap<String, Vec<(&String, &FileDocument)>> = BTreeMap::new();
    for doc in docs {
        if let Some(display) = file_infos.get(&doc.file_path) {
            let (src, _) = display.split_once('/').unwrap_or(("", display.as_str()));
            per_source.entry(src.to_string()).or_default().push((display, doc));
        }
    }

    for (src, mut entries) in per_source {
        entries.sort_by(|a, b| a.0.cmp(b.0));
        let _ = writeln!(out, "## {src}");
        let _ = writeln!(out);
        let _ = writeln!(out, "| {file_hdr} | {desc_hdr} |");
        let _ = writeln!(out, "|---|---|");
        for (display, doc) in entries {
            let rel = display.split_once('/').map(|(_, r)| r).unwrap_or(display);
            let stem = rel.trim_end_matches(".hpp").trim_end_matches(".h");
            let brief = doc
                .brief
                .default
                .as_deref()
                .or_else(|| doc.brief.translations.values().next().map(|s| s.as_str()))
                .unwrap_or("");
            let _ = writeln!(out, "| [{rel}]({src}/{stem}.md) | {brief} |");
        }
        let _ = writeln!(out);
    }

    if let Some(parent) = out_path.parent() {
        std::fs::create_dir_all(parent)?;
    }
    std::fs::write(out_path, out)
        .with_context(|| format!("failed to write: {}", out_path.display()))?;
    Ok(())
}
