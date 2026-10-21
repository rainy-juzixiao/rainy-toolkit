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

//! 未文档化实体统计。
//!
//! 遍历解析后的文档树，找出既没有被 `@hide` / `@notpublic` 排除、也没有写
//! 文档注释的实体，按文件汇总后输出警告。可见性口径与 markdown 生成器一致：
//! 私有/受保护的成员不算有效实体。

use crate::data::document::*;

/// 每个文件最多逐条列出的实体数量，其余只计入统计。
const MAX_LISTED: usize = 20;

#[derive(Clone, Copy, PartialEq, Eq)]
pub enum Kind {
    Class,
    Enum,
    EnumVariant,
    Alias,
    Concept,
    Function,
    Variable,
    Constant,
    VariableTemplate,
    Macro,
    Method,
    Field,
}

impl Kind {
    const ALL: [Kind; 12] = [
        Kind::Class,
        Kind::Enum,
        Kind::EnumVariant,
        Kind::Alias,
        Kind::Concept,
        Kind::Function,
        Kind::Variable,
        Kind::Constant,
        Kind::VariableTemplate,
        Kind::Macro,
        Kind::Method,
        Kind::Field,
    ];

    fn index(self) -> usize {
        Kind::ALL.iter().position(|k| *k == self).unwrap_or(0)
    }

    fn zh(self) -> &'static str {
        match self {
            Kind::Class => "类",
            Kind::Enum => "枚举",
            Kind::EnumVariant => "枚举项",
            Kind::Alias => "别名",
            Kind::Concept => "概念",
            Kind::Function => "函数",
            Kind::Variable => "变量",
            Kind::Constant => "常量",
            Kind::VariableTemplate => "变量模板",
            Kind::Macro => "宏",
            Kind::Method => "成员函数",
            Kind::Field => "成员变量",
        }
    }

    fn en(self) -> &'static str {
        match self {
            Kind::Class => "class",
            Kind::Enum => "enum",
            Kind::EnumVariant => "enumerator",
            Kind::Alias => "alias",
            Kind::Concept => "concept",
            Kind::Function => "function",
            Kind::Variable => "variable",
            Kind::Constant => "constant",
            Kind::VariableTemplate => "variable template",
            Kind::Macro => "macro",
            Kind::Method => "member function",
            Kind::Field => "member variable",
        }
    }

    fn en_plural(self, count: usize) -> &'static str {
        if count == 1 {
            return self.en();
        }
        match self {
            Kind::Class => "classes",
            Kind::Enum => "enums",
            Kind::EnumVariant => "enumerators",
            Kind::Alias => "aliases",
            Kind::Concept => "concepts",
            Kind::Function => "functions",
            Kind::Variable => "variables",
            Kind::Constant => "constants",
            Kind::VariableTemplate => "variable templates",
            Kind::Macro => "macros",
            Kind::Method => "member functions",
            Kind::Field => "member variables",
        }
    }
}

pub struct Undocumented {
    pub kind: Kind,
    pub name: String,
    pub line: Option<u32>,
}

pub struct Report {
    pub entities: Vec<Undocumented>,
}

impl Report {
    pub fn is_empty(&self) -> bool {
        self.entities.is_empty()
    }

    fn counts(&self) -> [usize; 12] {
        let mut counts = [0usize; 12];
        for entity in &self.entities {
            counts[entity.kind.index()] += 1;
        }
        counts
    }
}

pub fn collect(doc: &FileDocument) -> Report {
    let mut entities = Vec::new();
    if !doc.is_umbrella {
        collect_file(doc, &mut entities);
    }
    Report { entities }
}

fn collect_file(doc: &FileDocument, out: &mut Vec<Undocumented>) {
    for class in &doc.classes {
        collect_class(class, out);
    }
    for e in &doc.enums {
        collect_enum(e, out);
    }
    for a in &doc.aliases {
        push(out, Kind::Alias, &a.base);
    }
    for c in &doc.concepts {
        push(out, Kind::Concept, &c.base);
    }
    for v in &doc.variables {
        push(out, Kind::Variable, &v.base);
    }
    for c in &doc.constants {
        push(out, Kind::Constant, &c.base);
    }
    for vt in &doc.variable_templates {
        push(out, Kind::VariableTemplate, &vt.base);
    }
    for group in &doc.free_function_overloads {
        collect_free_functions(group, out);
    }
    for m in &doc.macros {
        push(out, Kind::Macro, &m.base);
    }
    for ns in &doc.namespaces {
        collect_namespace(ns, out);
    }
}

fn collect_namespace(ns: &NamespaceDocument, out: &mut Vec<Undocumented>) {
    for class in &ns.classes {
        collect_class(class, out);
    }
    for e in &ns.enums {
        collect_enum(e, out);
    }
    for a in &ns.aliases {
        push(out, Kind::Alias, &a.base);
    }
    for c in &ns.concepts {
        push(out, Kind::Concept, &c.base);
    }
    for v in &ns.variables {
        push(out, Kind::Variable, &v.base);
    }
    for c in &ns.constants {
        push(out, Kind::Constant, &c.base);
    }
    for vt in &ns.variable_templates {
        push(out, Kind::VariableTemplate, &vt.base);
    }
    for group in &ns.free_function_overloads {
        collect_free_functions(group, out);
    }
    for m in &ns.macros {
        push(out, Kind::Macro, &m.base);
    }
    for sub in &ns.sub_namespaces {
        collect_namespace(sub, out);
    }
}

fn collect_free_functions(group: &FreeFunctionOverloadGroup, out: &mut Vec<Undocumented>) {
    for f in &group.overloads {
        push(out, Kind::Function, &f.base);
    }
}

fn collect_class(class: &ClassDocument, out: &mut Vec<Undocumented>) {
    // 类本身被排除时，成员同样不会出现在文档里
    if is_excluded(&class.base) {
        return;
    }
    push(out, Kind::Class, &class.base);

    for group in &class.overload_groups {
        for o in &group.overloads {
            if is_public_member(&o.doc.base, o.doc.access.clone()) {
                push(out, Kind::Method, &o.doc.base);
            }
        }
    }
    for f in &class.member_fields {
        if is_public_member(&f.base, f.access.clone()) {
            push(out, Kind::Field, &f.base);
        }
    }
    for c in &class.constant_members {
        if is_public_member(&c.base, c.access.clone()) {
            push(out, Kind::Constant, &c.base);
        }
    }
    for vt in &class.variable_template_members {
        if is_public_member(&vt.base, vt.access.clone()) {
            push(out, Kind::VariableTemplate, &vt.base);
        }
    }
    for nested in &class.nested_classes {
        if is_public_member(&nested.base, nested.access.clone()) {
            collect_class(nested, out);
        }
    }
    for e in &class.nested_enums {
        if is_public_member(&e.base, e.access.clone()) {
            collect_enum(e, out);
        }
    }
    for a in &class.nested_aliases {
        if is_public_member(&a.base, a.access.clone()) {
            push(out, Kind::Alias, &a.base);
        }
    }
}

fn collect_enum(e: &EnumDocument, out: &mut Vec<Undocumented>) {
    if is_excluded(&e.base) {
        return;
    }
    push(out, Kind::Enum, &e.base);
    for v in &e.variants {
        if !v.is_documented {
            out.push(Undocumented {
                kind: Kind::EnumVariant,
                name: format!("{}::{}", qualified(&e.base), v.name),
                line: line_of(&v.location),
            });
        }
    }
}

fn is_excluded(base: &BasicDocument) -> bool {
    base.is_hidden || base.is_not_public
}

fn is_public_member(base: &BasicDocument, access: AccessLevel) -> bool {
    !is_excluded(base) && (base.is_shown || matches!(access, AccessLevel::Public))
}

fn push(out: &mut Vec<Undocumented>, kind: Kind, base: &BasicDocument) {
    if is_excluded(base) || base.is_documented {
        return;
    }
    out.push(Undocumented {
        kind,
        name: qualified(base),
        line: line_of(&base.location),
    });
}

fn qualified(base: &BasicDocument) -> String {
    if base.namespace_stack.is_empty() {
        base.name.clone()
    } else {
        format!("{}::{}", base.namespace_stack.join("::"), base.name)
    }
}

fn line_of(location: &Option<String>) -> Option<u32> {
    location
        .as_deref()?
        .rsplit_once(':')
        .and_then(|(_, line)| line.parse().ok())
}

/// 打印单个文件的未文档化实体统计。
pub fn warn(display: &str, report: &Report, lang: &str) {
    let zh = crate::is_chinese(lang);
    let counts = report.counts();
    let mut breakdown = Vec::new();
    for kind in Kind::ALL {
        let count = counts[kind.index()];
        if count > 0 {
            let label = if zh {
                kind.zh()
            } else {
                kind.en_plural(count)
            };
            breakdown.push(format!("{count} {label}"));
        }
    }
    let sep = if zh { "、" } else { ", " };

    if zh {
        eprintln!(
            "[sleepy warn] 未文档化的有效实体: {display}: 共 {} 个（{}）",
            report.entities.len(),
            breakdown.join(sep)
        );
    } else {
        eprintln!(
            "[sleepy warn] undocumented entities: {display}: {} total ({})",
            report.entities.len(),
            breakdown.join(sep)
        );
    }

    for entity in report.entities.iter().take(MAX_LISTED) {
        let kind = if zh { entity.kind.zh() } else { entity.kind.en() };
        let location = entity
            .line
            .map(|line| format!("L{line}"))
            .unwrap_or_default();
        if location.is_empty() {
            eprintln!("  |- {kind} `{}`", entity.name);
        } else {
            eprintln!("  |- {kind} `{}` ({location})", entity.name);
        }
    }
    let rest = report.entities.len().saturating_sub(MAX_LISTED);
    if rest > 0 {
        if zh {
            eprintln!("  |- ... 另有 {rest} 个未列出");
        } else {
            eprintln!("  |- ... {rest} more not listed");
        }
    }
}
