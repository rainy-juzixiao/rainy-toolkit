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
use super::{
    alias::build_type_alias,
    comment::{extract_raw_comment, parse_comment},
    enum_::build_enum,
    function::{build_access, build_template_params},
    variable::build_member_field,
};
use crate::data::document::{
    AccessLevel, BaseClass, ClassDocument, ClassKind, MemberFunctionDocument, OverloadDoc,
    OverloadGroup,
};
use crate::parser::function::build_overload_doc;
use crate::utils::traits::is_final;
use clang::{Entity, EntityKind};
use indexmap::IndexMap;

pub fn build_class(entity: &Entity, namespace_stack: &[String]) -> Option<ClassDocument> {
    let kind = match entity.get_kind() {
        EntityKind::ClassDecl => ClassKind::Class,
        EntityKind::StructDecl => ClassKind::Struct,
        EntityKind::UnionDecl => ClassKind::Union,
        EntityKind::ClassTemplate | EntityKind::ClassTemplatePartialSpecialization => {
            ClassKind::Class
        }
        _ => return None,
    };
    let name = entity.get_name()?;
    let raw = extract_raw_comment(entity).unwrap_or_default();
    let mut parsed = parse_comment(&raw, &name, namespace_stack.to_vec());
    let (is_template, template_params) = build_template_params(entity, &parsed.tparams_desc);
    parsed.basic.is_template = is_template;
    parsed.basic.template_params = template_params;
    let mut member_fields = Vec::new();
    let mut nested_classes = Vec::new();
    let mut nested_enums = Vec::new();
    let mut nested_aliases = Vec::new();
    let mut nested_usings = Vec::new();
    let mut base_classes = Vec::new();
    let mut friend_classes = Vec::new();
    let mut raw_methods: Vec<OverloadDoc> = Vec::new();
    let mut using_doc_overrides: Vec<OverloadDoc> = Vec::new();
    let mut child_ns = namespace_stack.to_vec();
    child_ns.push(name.clone());
    for child in entity.get_children() {
        match child.get_kind() {
            EntityKind::Method
            | EntityKind::Constructor
            | EntityKind::Destructor
            | EntityKind::ConversionFunction => {
                if let Some(o) = build_overload_doc(&child, &child_ns) {
                    raw_methods.push(o);
                }
            }
            EntityKind::UsingDeclaration => {
                if let Some(member_name) = child.get_name() {
                    nested_usings.push(member_name.clone());

                    let raw = extract_raw_comment(&child).unwrap_or_default();
                    if !raw.is_empty() {
                        let parsed = parse_comment(&raw, &member_name, child_ns.clone());
                        let decl_prototype = parsed.basic.overload_decl.clone(); // 只用手动提供的，using 没有 AST 签名
                        let func_doc = MemberFunctionDocument {
                            is_const: false,
                            is_noexcept: false,
                            is_for_lvalue: false,
                            is_for_rvalue: false,
                            is_virtual: false,
                            is_pure_virtual: false,
                            is_override: false,
                            is_final: false,
                            is_static: false,
                            is_explicit: false,
                            is_inline: false,
                            is_constexpr: false,
                            is_consteval: false,
                            is_defaulted: false,
                            is_deleted: false,
                            access: AccessLevel::Public,
                            params: parsed.params,
                            return_doc: parsed.return_doc,
                            exceptions: parsed.exceptions,
                            base: parsed.basic,
                        };
                        using_doc_overrides.push(OverloadDoc {
                            decl_prototype,
                            doc: func_doc,
                        });
                    }
                }
            }
            EntityKind::FieldDecl => {
                if let Some(f) = build_member_field(&child, &child_ns) {
                    member_fields.push(f);
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
                    nested_classes.push(c);
                }
            }
            EntityKind::EnumDecl => {
                if let Some(e) = build_enum(&child, &child_ns) {
                    nested_enums.push(e);
                }
            }
            EntityKind::TypeAliasDecl | EntityKind::TypedefDecl => {
                if let Some(a) = build_type_alias(&child, &child_ns) {
                    nested_aliases.push(a);
                }
            }
            EntityKind::BaseSpecifier => {
                let base_name = child.get_name().unwrap_or_default();
                base_classes.push(BaseClass {
                    name: base_name,
                    access: build_access(&child),
                    is_virtual: child.is_virtual_base(),
                });
            }
            EntityKind::FriendDecl => {
                if let Some(n) = child.get_name() {
                    friend_classes.push(n);
                }
            }
            _ => {}
        }
    }
    // @relates / @relatesalso 从 see_also 中提取
    let related_functions = parsed
        .basic
        .see_also
        .iter()
        .filter(|s| s.starts_with("relates:"))
        .map(|s| s.trim_start_matches("relates:").to_string())
        .collect();
    let overload_groups = group_by_name(raw_methods);
    Some(ClassDocument {
        kind,
        is_abstract: entity.is_abstract_record(),
        is_final: is_final(entity),
        access: build_access(entity),
        base_classes,
        overload_groups,
        member_fields,
        nested_classes,
        nested_enums,
        nested_aliases,
        friend_classes,
        related_functions,
        base: parsed.basic,
        using_members: nested_usings,
        using_doc_overrides,
    })
}

fn group_by_name(methods: Vec<OverloadDoc>) -> Vec<OverloadGroup> {
    let mut map: IndexMap<String, Vec<OverloadDoc>> = IndexMap::new();
    for m in methods {
        map.entry(m.doc.base.name.clone()).or_default().push(m);
    }
    map.into_iter()
        .map(|(name, overloads)| OverloadGroup { name, overloads })
        .collect()
}
