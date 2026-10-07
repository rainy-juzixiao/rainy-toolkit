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

//! Circular include detection over the internal include graph.
//!
//! Two levels are checked:
//! - **file level**: SCCs of size > 1 plus self-loops over file nodes;
//! - **module level**: the file graph is projected onto source (library)
//!   names; SCCs of size > 1 plus self-loops over modules.
//!
//! Detection uses Tarjan's algorithm; for each SCC a deterministic
//! representative cycle is extracted (starting from the lexicographically
//! smallest member, following sorted edges) for readable diagnostics.

use super::reports::IncludeGraph;
use std::collections::BTreeMap;
use std::fmt::Write as _;

#[derive(Debug, Clone)]
pub struct Cycle {
    /// the walk, e.g. ["a.hpp", "b.hpp", "a.hpp"] (starts and ends on the same node)
    pub path: Vec<String>,
    /// SCC members involved in this cycle
    #[allow(dead_code)]
    pub members: Vec<String>,
}

#[derive(Debug, Default)]
pub struct CycleReport {
    /// file-level cycles (sorted, deterministic)
    pub cycles: Vec<Cycle>,
    /// module-level cycles, each a list of module names forming the loop
    pub module_cycles: Vec<Vec<String>>,
    /// number of distinct files involved in any cycle
    pub involved_files: usize,
}

impl CycleReport {
    pub fn has_cycles(&self) -> bool {
        !self.cycles.is_empty() || !self.module_cycles.is_empty()
    }
}

/// Find all file-level and module-level cycles in the include graph.
pub fn find_cycles(graph: &IncludeGraph) -> CycleReport {
    let adj = build_adjacency(graph);

    let sccs = tarjan_scc(&adj);

    // ---- file-level cycles ----
    let mut cycles: Vec<Cycle> = Vec::new();
    let mut involved: std::collections::BTreeSet<String> = std::collections::BTreeSet::new();

    for scc in &sccs {
        if scc.len() == 1 {
            let node = &scc[0];
            // self-loop?
            if adj
                .get(node)
                .map(|outs| outs.contains(node))
                .unwrap_or(false)
            {
                cycles.push(Cycle {
                    path: vec![node.clone(), node.clone()],
                    members: vec![node.clone()],
                });
                involved.insert(node.clone());
            }
            continue;
        }
        if let Some(path) = representative_cycle(&adj, scc) {
            involved.extend(scc.iter().cloned());
            cycles.push(Cycle {
                path,
                members: scc.iter().cloned().collect(),
            });
        }
    }
    cycles.sort_by(|a, b| a.path.cmp(&b.path));

    // ---- module-level projection ----
    let mut module_adj: BTreeMap<String, std::collections::BTreeSet<String>> =
        BTreeMap::new();
    let source_of = |display: &String| -> String {
        display
            .split_once('/')
            .map(|(s, _)| s.to_string())
            .unwrap_or_else(|| display.clone())
    };
    for (from, to) in &graph.edges {
        let (mf, mt) = (source_of(from), source_of(to));
        if mf != mt {
            module_adj.entry(mf.clone()).or_default().insert(mt.clone());
        }
        module_adj.entry(mf).or_default();
        module_adj.entry(mt).or_default();
    }
    for name in graph.nodes.values() {
        module_adj
            .entry(source_of(&name.source_name))
            .or_default();
    }

    let module_sccs = tarjan_scc(&module_adj);
    let mut module_cycles: Vec<Vec<String>> = Vec::new();
    for scc in &module_sccs {
        if scc.len() == 1 {
            let m = &scc[0];
            if module_adj
                .get(m)
                .map(|outs| outs.contains(m))
                .unwrap_or(false)
            {
                module_cycles.push(vec![m.clone(), m.clone()]);
            }
            continue;
        }
        if let Some(path) = representative_cycle(&module_adj, scc) {
            let mut members = path;
            members.pop(); // drop repeated start node
            members.sort();
            module_cycles.push(members);
        }
    }
    module_cycles.sort();
    module_cycles.dedup();

    CycleReport {
        cycles,
        module_cycles,
        involved_files: involved.len(),
    }
}

fn build_adjacency(
    graph: &IncludeGraph,
) -> BTreeMap<String, std::collections::BTreeSet<String>> {
    let mut adj: BTreeMap<String, std::collections::BTreeSet<String>> = BTreeMap::new();
    for name in graph.nodes.keys() {
        adj.entry(name.clone()).or_default();
    }
    for (from, to) in &graph.edges {
        adj.entry(from.clone()).or_default().insert(to.clone());
        adj.entry(to.clone()).or_default();
    }
    adj
}

/// Tarjan strongly-connected components (iterative, sorted output).
fn tarjan_scc(
    adj: &BTreeMap<String, std::collections::BTreeSet<String>>,
) -> Vec<Vec<String>> {
    #[derive(Clone)]
    struct State {
        index: usize,
        lowlink: usize,
        on_stack: bool,
    }

    let mut index_counter: usize = 0;
    let mut states: BTreeMap<String, State> = BTreeMap::new();
    let mut stack: Vec<String> = Vec::new();
    let mut sccs: Vec<Vec<String>> = Vec::new();

    for root in adj.keys() {
        if states.contains_key(root) {
            continue;
        }
        // explicit DFS stack: (node, iterator position over successors)
        let mut work: Vec<(String, std::collections::btree_set::IntoIter<String>)> = Vec::new();
        let successors = adj
            .get(root)
            .cloned()
            .unwrap_or_default()
            .into_iter();
        states.insert(
            root.clone(),
            State {
                index: index_counter,
                lowlink: index_counter,
                on_stack: true,
            },
        );
        index_counter += 1;
        stack.push(root.clone());
        work.push((root.clone(), successors));

        while let Some((v, mut it)) = work.pop() {
            let mut descended = false;
            while let Some(w) = it.next() {
                match states.get(&w) {
                    None => {
                        // recurse into w
                        work.push((v.clone(), it));
                        states.insert(
                            w.clone(),
                            State {
                                index: index_counter,
                                lowlink: index_counter,
                                on_stack: true,
                            },
                        );
                        index_counter += 1;
                        stack.push(w.clone());
                        let wsucc = adj.get(&w).cloned().unwrap_or_default().into_iter();
                        work.push((w, wsucc));
                        descended = true;
                        break;
                    }
                    Some(s) if s.on_stack => {
                        let wl = s.index;
                        let st = states.get_mut(&v).unwrap();
                        if wl < st.lowlink {
                            st.lowlink = wl;
                        }
                    }
                    _ => {}
                }
            }
            if descended {
                continue;
            }

            // v finished: fold into parent's lowlink, then maybe emit SCC
            let vl = states.get(&v).unwrap().lowlink;
            let is_root_of_scc = vl == states.get(&v).unwrap().index;
            if let Some((parent, _)) = work.last() {
                let ps = states.get_mut(parent).unwrap();
                if vl < ps.lowlink {
                    ps.lowlink = vl;
                }
            }

            if is_root_of_scc {
                let mut scc: Vec<String> = Vec::new();
                while let Some(w) = stack.pop() {
                    let st = states.get_mut(&w).unwrap();
                    st.on_stack = false;
                    let is_v = w == v;
                    scc.push(w);
                    if is_v {
                        break;
                    }
                }
                scc.sort();
                sccs.push(scc);
            }
        }
    }

    sccs
}

/// Extract a deterministic representative cycle for an SCC.
///
/// 1. For small SCCs (≤ 12 members) first look for a simple cycle that
///    visits *every* member exactly once (the common "pure loop" case);
/// 2. Otherwise / as fallback, return the shortest cycle through the
///    lexicographically smallest member via BFS (always terminates,
///    deterministic because adjacency is sorted).
fn representative_cycle(
    adj: &BTreeMap<String, std::collections::BTreeSet<String>>,
    scc: &[String],
) -> Option<Vec<String>> {
    let members: std::collections::HashSet<&String> = scc.iter().collect();
    let start = scc.iter().min()?.clone();

    if scc.len() > 1 && scc.len() <= 12 {
        if let Some(cycle) = dfs_full_cycle(adj, &members, &start, scc.len() + 1) {
            return Some(cycle);
        }
    }
    bfs_shortest_cycle(adj, &members, &start)
}

/// DFS for a simple cycle visiting all SCC members then closing on `start`.
fn dfs_full_cycle(
    adj: &BTreeMap<String, std::collections::BTreeSet<String>>,
    members: &std::collections::HashSet<&String>,
    start: &str,
    target_len: usize,
) -> Option<Vec<String>> {
    fn go(
        adj: &BTreeMap<String, std::collections::BTreeSet<String>>,
        members: &std::collections::HashSet<&String>,
        start: &str,
        target_len: usize,
        path: &mut Vec<String>,
        on_path: &mut std::collections::HashSet<String>,
    ) -> Option<Vec<String>> {
        let current = path.last().unwrap().clone();
        for next in adj.get(&current).cloned().unwrap_or_default() {
            if !members.contains(&next) {
                continue;
            }
            let closing = path.len() + 1 == target_len;
            if closing {
                if next == start {
                    let mut cycle = path.clone();
                    cycle.push(start.to_string());
                    return Some(cycle);
                }
                continue; // must close exactly now
            }
            if next == start || on_path.contains(&next) {
                continue;
            }
            path.push(next.clone());
            on_path.insert(next.clone());
            if let Some(c) = go(adj, members, start, target_len, path, on_path) {
                return Some(c);
            }
            on_path.remove(&next);
            path.pop();
        }
        None
    }

    let mut path = vec![start.to_string()];
    let mut on_path: std::collections::HashSet<String> =
        std::iter::once(start.to_string()).collect();
    go(adj, members, start, target_len, &mut path, &mut on_path)
}

/// BFS for the shortest cycle that starts and ends at `start`.
fn bfs_shortest_cycle(
    adj: &BTreeMap<String, std::collections::BTreeSet<String>>,
    members: &std::collections::HashSet<&String>,
    start: &str,
) -> Option<Vec<String>> {
    use std::collections::{HashMap, VecDeque};

    let mut pred: HashMap<String, String> = HashMap::new();
    let mut queue: VecDeque<String> = VecDeque::new();
    let start_owned = start.to_string();
    queue.push_back(start_owned.clone());

    while let Some(node) = queue.pop_front() {
        for next in adj.get(&node).cloned().unwrap_or_default() {
            if !members.contains(&next) {
                continue;
            }
            if next == start {
                // reconstruct: start -> ... -> node -> start
                let mut rev: Vec<String> = vec![node.clone()];
                let mut cur = node.clone();
                while let Some(p) = pred.get(&cur) {
                    rev.push(p.clone());
                    if p == start {
                        break;
                    }
                    cur = p.clone();
                }
                rev.reverse();
                rev.push(start_owned.clone());
                return Some(rev);
            }
            if !pred.contains_key(&next) {
                pred.insert(next.clone(), node.clone());
                queue.push_back(next);
            }
        }
    }
    None
}

// ---------------------------------------------------------------------------
// Bilingual report
// ---------------------------------------------------------------------------

/// Format the cycle report as a human-readable diagnostic.
/// `lang` follows the config `lang` field (e.g. "english" / "chinese").
pub fn format_cycle_report(report: &CycleReport, lang: &str) -> String {
    let zh = lang.starts_with("zh")
        || lang.contains("chinese")
        || lang.contains("Chinese")
        || lang.contains("zh-Hans")
        || lang.contains("zh_hans");

    let mut out = String::new();

    if zh {
        let _ = writeln!(out, "✘ 检测到循环依赖 (circular includes detected)");
    } else {
        let _ = writeln!(out, "✘ Circular include dependencies detected");
    }

    if !report.cycles.is_empty() {
        if zh {
            let _ = writeln!(
                out,
                "  文件级循环: {} 处，涉及 {} 个文件:",
                report.cycles.len(),
                report.involved_files
            );
        } else {
            let _ = writeln!(
                out,
                "  File-level cycles: {}, involving {} files:",
                report.cycles.len(),
                report.involved_files
            );
        }
        for (i, cyc) in report.cycles.iter().enumerate() {
            let chain = cyc.path.join(" -> ");
            if zh {
                let _ = writeln!(out, "    [{}] {}  ← 闭环", i + 1, chain);
            } else {
                let _ = writeln!(out, "    [{}] {}  ← cycle closes here", i + 1, chain);
            }
        }
    }

    if !report.module_cycles.is_empty() {
        if zh {
            let _ = writeln!(
                out,
                "  模块级循环: {} 处:",
                report.module_cycles.len()
            );
        } else {
            let _ = writeln!(
                out,
                "  Module-level cycles: {}:",
                report.module_cycles.len()
            );
        }
        for (i, mods) in report.module_cycles.iter().enumerate() {
            let chain = mods.join(" <-> ");
            if zh {
                let _ = writeln!(out, "    [{}] {}  ← 闭环", i + 1, chain);
            } else {
                let _ = writeln!(out, "    [{}] {}  ← cycle closes here", i + 1, chain);
            }
        }
    }

    let _ = writeln!(out);
    if zh {
        let _ = writeln!(out, "  修复建议:");
        let _ = writeln!(out, "    1. 优先使用前置声明 (forward declaration) 打断头文件之间的相互包含;");
        let _ = writeln!(out, "    2. 若该包含是预期行为，可在 sleepy.yaml 的 dependency_graph.ignore 中豁免，");
        let _ = writeln!(out, "       例如: ignore: [\"a.hpp -> b.hpp\"];");
        let _ = writeln!(out, "    3. 或使用 --allow-cycles 跳过本次检查（不推荐）.");
    } else {
        let _ = writeln!(out, "  Hints:");
        let _ = writeln!(out, "    1. Prefer forward declarations to break mutual header includes;");
        let _ = writeln!(out, "    2. If the include is intentional, whitelist it in sleepy.yaml under");
        let _ = writeln!(out, "       dependency_graph.ignore, e.g. ignore: [\"a.hpp -> b.hpp\"];");
        let _ = writeln!(out, "    3. Or pass --allow-cycles to skip this check (not recommended).");
    }

    out
}
