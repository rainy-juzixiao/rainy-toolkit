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
use std::cell::RefCell;
use std::collections::HashMap;

#[derive(Debug, Clone)]
struct Template {
    raw: String,
}

#[derive(Debug, Clone)]
pub enum Override {
    Substituted(String),
}

#[derive(Debug, Default)]
pub struct ScopeTable {
    entries: Vec<(std::ops::RangeInclusive<u32>, Override)>,
    diagnostics: Vec<Diagnostic>,
}

#[derive(Debug, Clone)]
pub struct Diagnostic {
    pub line: u32,
    pub kind: DiagnosticKind,
    pub detail: String,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum DiagnosticKind {
    MissingValue,
    UnknownGroup,
    MissingGroupName,
    DuplicateGroup,
}

impl ScopeTable {
    pub fn get(&self, line: u32) -> Option<&Override> {
        self.entries
            .iter()
            .find(|(range, _)| range.contains(&line))
            .map(|(_, over)| over)
    }

    fn insert(&mut self, range: std::ops::RangeInclusive<u32>, over: Override) {
        self.entries.push((range, over));
    }
}

thread_local! {
    static CURRENT: RefCell<ScopeTable> = RefCell::new(ScopeTable::default());
}

pub fn install(text: &str) {
    let table = collect(text);
    CURRENT.with(|slot| *slot.borrow_mut() = table);
}

pub fn clear() {
    CURRENT.with(|slot| *slot.borrow_mut() = ScopeTable::default());
}

pub fn override_for(line: u32) -> Option<Override> {
    CURRENT.with(|slot| slot.borrow().get(line).cloned())
}

pub fn take_diagnostics() -> Vec<Diagnostic> {
    CURRENT.with(|slot| std::mem::take(&mut slot.borrow_mut().diagnostics))
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
enum Kind {
    Scope,
    ForScope,
    Var,
}

#[derive(Debug)]
struct Directive {
    kind: Kind,
    value: String,
}

pub fn collect(text: &str) -> ScopeTable {
    let lines: Vec<&str> = text.lines().collect();
    let mut table = ScopeTable::default();
    let mut active: HashMap<String, Template> = HashMap::new();
    let mut last_group: Option<String> = None;
    let mut i = 0;

    while i < lines.len() {
        let Some(block) = read_comment_block(&lines, &mut i) else {
            i += 1;
            continue;
        };
        let directives = parse_directives(&block);
        if directives.is_empty() {
            continue;
        }
        let Some(decl_range) = next_decl_range(&lines, i) else {
            continue;
        };
        let decl_line = *decl_range.start();
        let raw = strip_directives(&block);

        if let Some(group) = directives
            .iter()
            .find(|d| d.kind == Kind::Scope && !d.value.is_empty())
            .map(|d| d.value.clone())
        {
            let mut values = HashMap::new();
            for d in directives.iter().filter(|d| d.kind == Kind::Var) {
                match d.value.split_once('=') {
                    Some((var, value)) => {
                        values.insert(var.trim().to_string(), value.trim().to_string());
                    }
                    None => table.diagnostics.push(Diagnostic {
                        line: decl_line,
                        kind: DiagnosticKind::MissingValue,
                        detail: d.value.clone(),
                    }),
                }
            }
            if values.is_empty() {
                continue;
            }
            if active.contains_key(&group) {
                table.diagnostics.push(Diagnostic {
                    line: decl_line,
                    kind: DiagnosticKind::DuplicateGroup,
                    detail: group.clone(),
                });
            }
            table.insert(decl_range, Override::Substituted(substitute(&raw, &values)));
            active.insert(group.clone(), Template { raw });
            last_group = Some(group);
            continue;
        }

        let Some(found) = directives.iter().find(|d| d.kind == Kind::ForScope) else {
            continue;
        };
        let group = if found.value.is_empty() {
            match &last_group {
                Some(group) => group.clone(),
                None => {
                    table.diagnostics.push(Diagnostic {
                        line: decl_line,
                        kind: DiagnosticKind::MissingGroupName,
                        detail: String::new(),
                    });
                    continue;
                }
            }
        } else {
            found.value.clone()
        };
        let Some(template) = active.get(&group) else {
            table.diagnostics.push(Diagnostic {
                line: decl_line,
                kind: DiagnosticKind::UnknownGroup,
                detail: group,
            });
            continue;
        };
        let mut values = HashMap::new();
        for d in directives.iter().filter(|d| d.kind == Kind::Var) {
            if let Some((var, value)) = d.value.split_once('=') {
                values.insert(var.trim().to_string(), value.trim().to_string());
            }
        }
        table.insert(decl_range, Override::Substituted(substitute(&template.raw, &values)));
    }

    table
}

fn parse_directives(block: &[&str]) -> Vec<Directive> {
    let mut out = Vec::new();
    for line in block {
        let line = strip_comment_marker(line.trim());
        if let Some(rest) = strip_tag(line, "scope") {
            if let Some((kind, value)) = parse_arg(rest) {
                out.push(Directive { kind, value });
            }
            continue;
        }
        if let Some(rest) = strip_tag(line, "forscope") {
            if let Some((mut kind, value)) = parse_arg(rest) {
                if kind == Kind::Scope {
                    kind = Kind::ForScope;
                }
                out.push(Directive { kind, value });
            }
        }
    }
    out
}

fn parse_arg(rest: &str) -> Option<(Kind, String)> {
    let rest = rest.trim();
    if rest.is_empty() {
        return None;
    }
    let (head, value) = match rest.split_once('[') {
        Some((head, tail)) => (head.trim(), tail.trim_end_matches(']').trim().to_string()),
        None => match rest.split_once(char::is_whitespace) {
            Some((head, tail)) => (head.trim(), tail.trim().to_string()),
            None => (rest, String::new()),
        },
    };
    let is_name = (head.is_empty() && !value.is_empty()) || head.eq_ignore_ascii_case("name");
    let kind = if is_name {
        Kind::Scope
    } else if head.eq_ignore_ascii_case("var") {
        Kind::Var
    } else {
        return None;
    };
    Some((kind, value))
}

fn strip_tag<'a>(line: &'a str, tag: &str) -> Option<&'a str> {
    let at = format!("@{tag}");
    let bsl = format!("\\{tag}");
    if let Some(r) = line.strip_prefix(at.as_str()) {
        return Some(r.trim_start_matches(char::is_whitespace));
    }
    if let Some(r) = line.strip_prefix(bsl.as_str()) {
        return Some(r.trim_start_matches(char::is_whitespace));
    }
    None
}

fn strip_comment_marker(line: &str) -> &str {
    let line = line.strip_prefix("/**").unwrap_or(line);
    let line = line.strip_prefix("/*!").unwrap_or(line);
    let line = line.strip_prefix("//!").unwrap_or(line);
    let line = line.strip_prefix("///").unwrap_or(line);
    let line = if let Some(s) = line.strip_prefix("* ") {
        s
    } else {
        line.strip_prefix('*').unwrap_or(line)
    };
    line.trim()
}

fn read_comment_block<'a>(lines: &[&'a str], i: &mut usize) -> Option<Vec<&'a str>> {
    let trimmed = lines.get(*i)?.trim();
    if !trimmed.starts_with("/*") && !trimmed.starts_with("//") {
        return None;
    }
    let mut block = Vec::new();
    if trimmed.starts_with("/*") {
        while *i < lines.len() {
            let line = lines[*i];
            block.push(line);
            *i += 1;
            if line.contains("*/") {
                break;
            }
        }
    } else {
        while *i < lines.len() {
            let line = lines[*i];
            if !line.trim().starts_with("//") {
                break;
            }
            block.push(line);
            *i += 1;
        }
    }
    Some(block)
}

fn next_decl_range(lines: &[&str], mut i: usize) -> Option<std::ops::RangeInclusive<u32>> {
    while i < lines.len() {
        let trimmed = lines[i].trim();
        if trimmed.is_empty() || trimmed.starts_with("//") || trimmed.starts_with("/*") {
            i += 1;
            continue;
        }
        break;
    }
    if i >= lines.len() {
        return None;
    }
    let start = i;
    while i < lines.len() {
        let line = lines[i];
        i += 1;
        if line.contains('{') || line.trim_end().ends_with(';') {
            break;
        }
        if i - start > 40 {
            break;
        }
    }
    Some((start as u32 + 1)..=(i as u32))
}

fn strip_directives(block: &[&str]) -> String {
    block
        .iter()
        .filter(|line| {
            let line = strip_comment_marker(line.trim());
            strip_tag(line, "scope").is_none() && strip_tag(line, "forscope").is_none()
        })
        .copied()
        .collect::<Vec<_>>()
        .join("\n")
}

fn substitute(raw: &str, values: &HashMap<String, String>) -> String {
    let mut out = raw.to_string();
    for (var, value) in values {
        out = out.replace(&format!("{{{var}}}"), value);
    }
    out
}
