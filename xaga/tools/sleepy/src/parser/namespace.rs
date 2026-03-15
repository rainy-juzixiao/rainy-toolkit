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
use super::{alias::build_type_alias, class::build_class, comment::{extract_raw_comment, parse_comment}, concept::build_concept, enum_::build_enum, function::build_free_function, is_in_nodoc_range, macro_::build_macro, merge_namespace, variable::build_variable};
use crate::data::document::NamespaceDocument;
use crate::parser::concept::is_concept_decl;
use clang::{Entity, EntityKind};
use crate::data::context::ParseContext;

pub fn build_namespace(
    entity: &Entity,
    namespace_stack: &[String],
    context: &ParseContext,
) -> Option<NamespaceDocument> {
    if !matches!(entity.get_kind(), EntityKind::Namespace) {
        return None;
    }
    let name = entity.get_name().unwrap_or_default();
    let raw = extract_raw_comment(entity).unwrap_or_default();
    let parsed = parse_comment(&raw, &name, namespace_stack.to_vec());

    let mut child_ns = namespace_stack.to_vec();
    if !name.is_empty() {
        child_ns.push(name);
    }

    let mut doc = NamespaceDocument {
        is_inline: entity.is_inline_namespace(),
        free_functions: vec![],
        variables: vec![],
        classes: vec![],
        enums: vec![],
        aliases: vec![],
        concepts: vec![],
        macros: vec![],
        sub_namespaces: vec![],
        base: parsed.basic,
    };
    for child in entity.get_children() {
        if is_in_nodoc_range(&child, context.nodoc_ranges) {
            continue;
        }
        match child.get_kind() {
            EntityKind::FunctionDecl | EntityKind::FunctionTemplate => {
                if let Some(f) = build_free_function(&child, &child_ns) {
                    doc.free_functions.push(f);
                }
            }
            EntityKind::VarDecl => {
                if let Some(v) = build_variable(&child, &child_ns) {
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
                if let Some(c) = build_class(&child, &child_ns) {
                    doc.classes.push(c);
                }
            }
            EntityKind::EnumDecl => {
                if let Some(e) = build_enum(&child, &child_ns) {
                    doc.enums.push(e);
                }
            }
            EntityKind::TypeAliasDecl | EntityKind::TypedefDecl => {
                if let Some(a) = build_type_alias(&child, &child_ns) {
                    doc.aliases.push(a);
                }
            }
            EntityKind::MacroDefinition => {
                if let Some(m) = build_macro(&child, &child_ns) {
                    doc.macros.push(m);
                }
            }
            EntityKind::Namespace => {
                if let Some(ns) = build_namespace(&child, &child_ns, context) {
                    let sub_name = ns.base.name.clone();
                    if let Some(existing) = doc
                        .sub_namespaces
                        .iter_mut()
                        .find(|s| s.base.name == sub_name)
                    {
                        merge_namespace(existing, ns);
                    } else {
                        doc.sub_namespaces.push(ns);
                    }
                }
            }
            _ if is_concept_decl(&child) => {
                if let Some(c) = build_concept(&child, &child_ns) {
                    doc.concepts.push(c);
                }
            }
            _ => {}
        }
    }

    Some(doc)
}
