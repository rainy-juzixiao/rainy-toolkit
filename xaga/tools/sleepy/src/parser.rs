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
pub mod alias;
pub mod class;
pub mod comment;
pub mod concept;
pub mod enum_;
pub mod function;
pub mod macro_;
pub mod namespace;
pub mod variable;

use crate::data::document::{
    AccessLevel, ClassDocument, FileDocument, NamespaceDocument, OverloadDoc, OverloadGroup,
};

pub(crate) use crate::data::context::{is_from_owned_file, ParseContext};
use crate::parser::concept::is_concept_decl;
use alias::build_type_alias;
use clang::{Entity, EntityKind, TranslationUnit};
use class::build_class;
use comment::{extract_raw_comment, parse_comment};
use concept::build_concept;
use enum_::build_enum;
use function::build_free_function;
use indexmap::IndexMap;
use macro_::build_macro;
use namespace::build_namespace;
use std::collections::HashMap;
use std::path::PathBuf;
use which::Path;
use variable::build_variable;

pub fn collect_nodoc_ranges(source_text: &str) -> Vec<std::ops::Range<u32>> {
    let mut ranges = Vec::new();
    let mut begin: Option<u32> = None;
    for (i, line) in source_text.lines().enumerate() {
        let line_no = (i + 1) as u32;
        if line.contains("// @NODOCBEGIN") {
            begin = Some(line_no);
        } else if line.contains("// @NODOCEND") {
            if let Some(start) = begin.take() {
                ranges.push(start..line_no);
            }
        }
    }
    // 未闭合的 NODOCBEGIN 延伸到文件末尾
    if let Some(start) = begin {
        ranges.push(start..u32::MAX);
    }
    ranges
}

pub fn is_in_nodoc_range(entity: &Entity, ranges: &[std::ops::Range<u32>]) -> bool {
    if ranges.is_empty() {
        return false;
    }
    let line = entity
        .get_location()
        .map(|l| l.get_file_location().line)
        .unwrap_or(0);
    ranges.iter().any(|r| r.contains(&line))
}

pub fn read_mergeto_tag(file: &PathBuf) -> Option<String> {
    let text = std::fs::read_to_string(file).ok()?;
    for line in text.lines() {
        let line = line.trim().trim_start_matches('*').trim();
        if let Some(rest) = line.strip_prefix("@mergeto") {
            let target = rest.trim().to_string();
            if !target.is_empty() {
                return Some(target);
            }
        }
    }
    None
}

pub fn build_file_document(
    tu: &TranslationUnit,
    file_path: &str,
    context: &ParseContext,
) -> FileDocument {
    let root = tu.get_entity();
    let namespace_stack = vec![];

    let mut doc = FileDocument {
        file_path: file_path.to_string(),
        includes: vec![],
        brief: crate::i18n::I18n::new(),
        description: crate::i18n::I18n::new(),
        authors: vec![],
        date: None,
        version: None,
        copyright: None,
        license: None,
        namespaces: vec![],
        free_functions: vec![],
        variables: vec![],
        classes: vec![],
        enums: vec![],
        aliases: vec![],
        concepts: vec![],
        macros: vec![],
        merge_into: None,
    };

    if let Some(raw) = extract_raw_comment(&root) {
        let parsed = parse_comment(&raw, file_path, vec![]);
        doc.brief       = parsed.basic.brief;
        doc.description = parsed.basic.description;
        doc.authors     = parsed.basic.authors;
        doc.date        = parsed.basic.date;
        doc.version     = parsed.basic.version;
        doc.copyright   = parsed.basic.copyright;
        doc.license     = parsed.basic.license;
    }

    let mut ns_map: IndexMap<String, NamespaceDocument> = IndexMap::new();

    for child in root.get_children() {
        if !is_from_owned_file(&child, context) {
            continue;
        }
        if is_in_nodoc_range(&child, context.nodoc_ranges) {
            continue;
        }
        match child.get_kind() {
            EntityKind::Namespace => {
                if let Some(ns) = build_namespace(&child, &namespace_stack, context) {
                    let name = ns.base.name.clone();
                    if let Some(existing) = ns_map.get_mut(&name) {
                        merge_namespace(existing, ns);
                    } else {
                        ns_map.insert(name, ns);
                    }
                }
            }
            _ => match child.get_kind() {
                EntityKind::InclusionDirective => {
                    if let Some(name) = child.get_name() {
                        doc.includes.push(name);
                    }
                }
                EntityKind::FunctionDecl | EntityKind::FunctionTemplate => {
                    if let Some(f) = build_free_function(&child, &namespace_stack) {
                        doc.free_functions.push(f);
                    }
                }
                EntityKind::VarDecl => {
                    if let Some(v) = build_variable(&child, &namespace_stack) {
                        doc.variables.push(v);
                    }
                }
                EntityKind::ClassDecl
                | EntityKind::StructDecl
                | EntityKind::UnionDecl
                | EntityKind::ClassTemplate
                | EntityKind::ClassTemplatePartialSpecialization => {
                    if !child.is_definition() {
                        continue;
                    }
                    if let Some(c) = build_class(&child, &namespace_stack) {
                        doc.classes.push(c);
                    }
                }
                EntityKind::EnumDecl => {
                    if let Some(e) = build_enum(&child, &namespace_stack) {
                        doc.enums.push(e);
                    }
                }
                EntityKind::TypeAliasDecl | EntityKind::TypedefDecl => {
                    if let Some(a) = build_type_alias(&child, &namespace_stack) {
                        doc.aliases.push(a);
                    }
                }
                EntityKind::MacroDefinition => {
                    if let Some(m) = build_macro(&child, &namespace_stack) {
                        doc.macros.push(m);
                    }
                }
                _ if is_concept_decl(&child) => {
                    if let Some(c) = build_concept(&child, &namespace_stack) {
                        doc.concepts.push(c);
                    }
                }
                _ => {}
            },
        }
    }

    doc.namespaces = ns_map.into_values().collect();
    resolve_inheritance(&mut doc);
    doc
}

pub fn resolve_inheritance(doc: &mut FileDocument) {
    let mut class_map: HashMap<String, ClassDocument> = HashMap::new();
    collect_classes(doc, &mut class_map);

    // 对每个类做继承解析
    apply_inheritance_all(doc, &class_map);
}

fn collect_classes(doc: &FileDocument, map: &mut HashMap<String, ClassDocument>) {
    for ns in &doc.namespaces {
        collect_classes_from_ns(ns, map);
    }
    for c in &doc.classes {
        collect_class_recursive(c, map);
    }
}

fn collect_classes_from_ns(ns: &NamespaceDocument, map: &mut HashMap<String, ClassDocument>) {
    for c in &ns.classes {
        collect_class_recursive(c, map);
    }
    for sub in &ns.sub_namespaces {
        collect_classes_from_ns(sub, map);
    }
}

fn collect_class_recursive(c: &ClassDocument, map: &mut HashMap<String, ClassDocument>) {
    map.insert(c.base.name.clone(), c.clone());
    for nested in &c.nested_classes {
        collect_class_recursive(nested, map);
    }
}

/// 对整个文档树里的每个类做继承文档填充
fn apply_inheritance_all(doc: &mut FileDocument, map: &HashMap<String, ClassDocument>) {
    for ns in &mut doc.namespaces {
        apply_inheritance_ns(ns, map);
    }
    for c in &mut doc.classes {
        apply_inheritance_class(c, map);
    }
}

fn apply_inheritance_ns(ns: &mut NamespaceDocument, map: &HashMap<String, ClassDocument>) {
    for c in &mut ns.classes {
        apply_inheritance_class(c, map);
    }
    for sub in &mut ns.sub_namespaces {
        apply_inheritance_ns(sub, map);
    }
}

fn apply_inheritance_class(c: &mut ClassDocument, map: &HashMap<String, ClassDocument>) {
    for nested in &mut c.nested_classes {
        apply_inheritance_class(nested, map);
    }

    for base_class in &c.base_classes {
        let is_public = matches!(base_class.access, AccessLevel::Public);

        let base_doc = map.get(&base_class.name).or_else(|| {
            let bare = base_class.name.split('<').next().unwrap_or("");
            map.get(bare)
        });

        let Some(base_doc) = base_doc else { continue };

        if base_doc.overload_groups.is_empty() {
            for using_name in &c.using_members {
                let overrides: Vec<OverloadDoc> = c
                    .using_doc_overrides
                    .iter()
                    .filter(|o| &o.doc.base.name == using_name)
                    .cloned()
                    .collect();
                if overrides.is_empty() {
                    continue;
                }
                if let Some(existing) = c.overload_groups.iter_mut().find(|g| &g.name == using_name)
                {
                    for o in overrides {
                        let already = existing
                            .overloads
                            .iter()
                            .any(|e| e.decl_prototype == o.decl_prototype);
                        if !already {
                            existing.overloads.push(o);
                        }
                    }
                } else {
                    c.overload_groups.push(OverloadGroup {
                        name: using_name.clone(),
                        overloads: overrides,
                    });
                }
            }
            continue;
        }
        for base_group in &base_doc.overload_groups {
            let visible = is_public || c.using_members.contains(&base_group.name);
            if !visible {
                continue;
            }

            let manual_overloads: Vec<OverloadDoc> = c
                .using_doc_overrides
                .iter()
                .filter(|o| o.doc.base.name == base_group.name)
                .cloned()
                .collect();

            if let Some(existing_group) = c
                .overload_groups
                .iter_mut()
                .find(|g| g.name == base_group.name)
            {
                for base_overload in &base_group.overloads {
                    if base_overload.doc.base.is_not_public {
                        continue;
                    }
                    let has_manual = manual_overloads.iter().any(|m| {
                        prototype_matches(
                            m.doc.base.overload_decl.as_deref().unwrap_or(""),
                            base_overload,
                        )
                    });
                    if !has_manual {
                        existing_group.overloads.push(base_overload.clone());
                    }
                }
            } else {
                let mut new_group = base_group.clone();
                new_group.overloads.retain(|o| !o.doc.base.is_not_public);

                if manual_overloads.is_empty() {
                    c.overload_groups.push(new_group);
                } else {
                    for manual in &manual_overloads {
                        let proto = manual.doc.base.overload_decl.as_deref().unwrap_or("");
                        if proto.is_empty() {
                            new_group.overloads = vec![manual.clone()];
                        } else if let Some(slot) = new_group
                            .overloads
                            .iter_mut()
                            .find(|o| prototype_matches(proto, o))
                        {
                            *slot = manual.clone();
                        } else {
                            new_group.overloads.push(manual.clone());
                        }
                    }
                    c.overload_groups.push(new_group);
                }
            }
        }

        // 字段继承
        for base_field in &base_doc.member_fields {
            if base_field.base.is_not_public {
                continue;
            }
            let visible = is_public || c.using_members.contains(&base_field.base.name);
            if !visible {
                continue;
            }
            if !is_public && !matches!(base_field.access, AccessLevel::Public) {
                continue;
            }
            let already = c
                .member_fields
                .iter()
                .any(|f| f.base.name == base_field.base.name);
            if !already {
                c.member_fields.push(base_field.clone());
            }
        }
    }
}

fn prototype_matches(proto: &str, overload: &OverloadDoc) -> bool {
    let proto_name = extract_proto_name(proto);
    if proto_name != overload.doc.base.name {
        return false;
    }
    if let Some(base_proto) = &overload.doc.base.overload_decl {
        return normalize_proto(proto) == normalize_proto(base_proto);
    }
    let proto_param_count = count_proto_params(proto);
    let base_param_count = overload.doc.params.len();
    proto_param_count == base_param_count
}

fn extract_proto_name(proto: &str) -> &str {
    proto
        .split('(')
        .next()
        .unwrap_or("")
        .split_whitespace()
        .last()
        .unwrap_or("")
}

fn normalize_proto(proto: &str) -> String {
    proto.split_whitespace().collect::<Vec<_>>().join(" ")
}

fn count_proto_params(proto: &str) -> usize {
    let inner = proto
        .split('(')
        .nth(1)
        .unwrap_or("")
        .split(')')
        .next()
        .unwrap_or("");
    if inner.trim().is_empty() {
        return 0;
    }
    inner.split(',').count()
}

fn merge_namespace(target: &mut NamespaceDocument, source: NamespaceDocument) {
    target.classes.extend(source.classes);
    target.enums.extend(source.enums);
    target.free_functions.extend(source.free_functions);
    target.variables.extend(source.variables);
    target.aliases.extend(source.aliases);
    target.concepts.extend(source.concepts);
    target.macros.extend(source.macros);
    for sub in source.sub_namespaces {
        let sub_name = sub.base.name.clone();
        if let Some(existing_sub) = target
            .sub_namespaces
            .iter_mut()
            .find(|s| s.base.name == sub_name)
        {
            merge_namespace(existing_sub, sub);
        } else {
            target.sub_namespaces.push(sub);
        }
    }
}
