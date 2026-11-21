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
use clang::{Entity, EntityKind};
use std::collections::HashMap;

/// 翻译单元内的类索引，用于解析依赖型 `using` 声明的限定符。
///
/// `using base::reset;` 里的 `base` 可能是依赖别名，libclang 不会给出
/// 被引用的成员；此时只能把限定符脱糖成完整类型名，再回到 AST 里找类。
pub struct ClassIndex<'tu> {
    by_qualified: HashMap<String, Entity<'tu>>,
    by_name: HashMap<String, Vec<Entity<'tu>>>,
}

impl<'tu> ClassIndex<'tu> {
    pub fn build(root: &Entity<'tu>) -> Self {
        let mut index = Self {
            by_qualified: HashMap::new(),
            by_name: HashMap::new(),
        };
        index.collect(root);
        index
    }

    fn collect(&mut self, entity: &Entity<'tu>) {
        for child in entity.get_children() {
            if is_record(child.get_kind()) && child.is_definition() {
                if let Some(name) = child.get_name() {
                    if !name.is_empty() {
                        let qualified = qualified_name(&child).unwrap_or_else(|| name.clone());
                        self.by_qualified.entry(qualified).or_insert(child);
                        self.by_name.entry(name).or_default().push(child);
                    }
                }
            }
            self.collect(&child);
        }
    }

    fn find(&self, qualified: &str) -> Option<Entity<'tu>> {
        if let Some(entity) = self.by_qualified.get(qualified) {
            return Some(*entity);
        }
        let short = qualified.rsplit("::").next().unwrap_or(qualified);
        let short = short.split('<').next().unwrap_or(short).trim();
        self.by_name.get(short).and_then(|v| v.first().copied())
    }
}

fn is_record(kind: EntityKind) -> bool {
    matches!(
        kind,
        EntityKind::ClassDecl
            | EntityKind::StructDecl
            | EntityKind::UnionDecl
            | EntityKind::ClassTemplate
            | EntityKind::ClassTemplatePartialSpecialization
    )
}

/// 成员级别的实体种类：`using` 可能指向其中任意一种。
pub fn is_member_kind(kind: EntityKind) -> bool {
    matches!(
        kind,
        EntityKind::Method
            | EntityKind::Constructor
            | EntityKind::Destructor
            | EntityKind::ConversionFunction
            | EntityKind::FunctionTemplate
            | EntityKind::FieldDecl
            | EntityKind::VarDecl
            | EntityKind::TypeAliasDecl
            | EntityKind::TypedefDecl
            | EntityKind::EnumDecl
            | EntityKind::ClassDecl
            | EntityKind::StructDecl
            | EntityKind::UnionDecl
            | EntityKind::ClassTemplate
            | EntityKind::ClassTemplatePartialSpecialization
    )
}

fn qualified_name(entity: &Entity) -> Option<String> {
    let mut parts: Vec<String> = Vec::new();
    let mut current = entity.get_semantic_parent();
    let mut guard = 0;
    while let Some(parent) = current {
        guard += 1;
        if guard > 32 {
            break;
        }
        if matches!(
            parent.get_kind(),
            EntityKind::Namespace
                | EntityKind::ClassDecl
                | EntityKind::StructDecl
                | EntityKind::UnionDecl
                | EntityKind::ClassTemplate
                | EntityKind::ClassTemplatePartialSpecialization
        ) {
            if let Some(name) = parent.get_name() {
                if !name.is_empty() {
                    parts.push(name);
                }
            }
        }
        current = parent.get_semantic_parent();
    }
    parts.reverse();
    let name = entity.get_name()?;
    if parts.is_empty() {
        Some(name)
    } else {
        Some(format!("{}::{}", parts.join("::"), name))
    }
}

/// 解析 `using` 声明最终指向的成员实体。
///
/// 两条路径：
/// 1. 非依赖型 `using std::nullopt;` / `using Base::reset;`：子节点里的
///    `OverloadedDeclRef` 直接带着被引用声明；
/// 2. 依赖型 `using base::reset;`（`base` 是别名模板或依赖类型）：把限定符
///    脱糖成完整类型名，再按名字回到类索引里找，找不到就沿基类链继续找。
pub fn resolve_targets<'tu>(
    using_decl: &Entity<'tu>,
    name: &str,
    index: &ClassIndex<'tu>,
) -> Vec<Entity<'tu>> {
    let children = using_decl.get_children();
    for child in &children {
        if child.get_kind() != EntityKind::OverloadedDeclRef {
            continue;
        }
        if let Some(declarations) = child.get_overloaded_declarations() {
            let found: Vec<Entity<'tu>> = declarations
                .into_iter()
                .filter(|d| is_member_kind(d.get_kind()))
                .collect();
            if !found.is_empty() {
                return found;
            }
        }
    }

    let Some(type_ref) = children.iter().find(|c| c.get_kind() == EntityKind::TypeRef) else {
        return vec![];
    };
    let Some(ty) = type_ref.get_type() else {
        return vec![];
    };
    let canonical = ty.get_canonical_type();
    if let Some(decl) = canonical.get_declaration() {
        let found = find_members(&decl, name, 0);
        if !found.is_empty() {
            return found;
        }
    }
    for candidate in class_candidates(&canonical.get_display_name()) {
        if let Some(class) = index.find(&candidate) {
            let found = find_members(&class, name, 0);
            if !found.is_empty() {
                return found;
            }
        }
    }
    vec![]
}

fn find_members<'tu>(class: &Entity<'tu>, name: &str, depth: usize) -> Vec<Entity<'tu>> {
    if depth > 8 {
        return vec![];
    }
    let children = class.get_children();
    let direct: Vec<Entity<'tu>> = children
        .iter()
        .copied()
        .filter(|c| is_member_kind(c.get_kind()) && c.get_name().as_deref() == Some(name))
        .collect();
    if !direct.is_empty() {
        return direct;
    }
    for base in &children {
        if base.get_kind() != EntityKind::BaseSpecifier {
            continue;
        }
        let Some(base_entity) = base.get_reference() else {
            continue;
        };
        let found = find_members(&base_entity, name, depth + 1);
        if !found.is_empty() {
            return found;
        }
    }
    vec![]
}

/// 从脱糖后的类型文本里提取候选类名，限定程度高的排在前面。
fn class_candidates(text: &str) -> Vec<String> {
    let mut out: Vec<String> = Vec::new();
    let mut token = String::new();
    let flush = |token: &mut String, out: &mut Vec<String>| {
        let trimmed = token.trim_matches(':').to_string();
        token.clear();
        if trimmed.is_empty() {
            return;
        }
        let first = trimmed.chars().next().unwrap_or(' ');
        if !(first.is_alphabetic() || first == '_') {
            return;
        }
        if !out.contains(&trimmed) {
            out.push(trimmed);
        }
    };
    for ch in text.chars() {
        if ch.is_alphanumeric() || ch == '_' || ch == ':' {
            token.push(ch);
        } else {
            flush(&mut token, &mut out);
        }
    }
    flush(&mut token, &mut out);
    out.sort_by(|a, b| {
        let a_depth = a.matches("::").count();
        let b_depth = b.matches("::").count();
        b_depth.cmp(&a_depth).then(b.len().cmp(&a.len()))
    });
    out
}
