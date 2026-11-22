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
    comment::{entity_location, extract_raw_comment, parse_comment},
    enum_::build_enum,
    function::{build_access, build_template_params},
    using_decl::{self, ClassIndex},
    variable::{
        build_constant as build_constant_field, build_member_field, build_variable_template,
        is_constant_variable, is_variable_template_entity,
    },
};
use crate::data::document::{
    AccessLevel, BaseClass, BasicDocument, ClassDocument, ClassKind, ConstantDocument,
    EnumDocument, MemberFieldDocument, MemberFunctionDocument, OverloadDoc, OverloadGroup,
    TypeAliasDocument, VariableTemplateDocument,
};
use crate::parser::function::build_overload_doc;
use crate::utils::traits::is_final;
use clang::{Entity, EntityKind};
use indexmap::IndexMap;

/// `using` 声明解析出的最终实体，按种类分别归档到类的各个成员列表。
enum UsingTarget {
    Method(OverloadDoc),
    Field(MemberFieldDocument),
    Constant(ConstantDocument),
    VariableTemplate(VariableTemplateDocument),
    Class(ClassDocument),
    Enum(EnumDocument),
    Alias(TypeAliasDocument),
}

pub fn build_class<'tu>(
    entity: &Entity<'tu>,
    namespace_stack: &[String],
    index: &ClassIndex<'tu>,
) -> Option<ClassDocument> {
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
    let mut parsed = parse_comment(&raw, &name, namespace_stack.to_vec(), entity_location(entity).as_deref());
    let (is_template, template_params) = build_template_params(entity, &parsed.tparams_desc);
    parsed.basic.is_template = is_template;
    parsed.basic.template_params = template_params;
    let mut member_fields = Vec::new();
    let mut constant_members = Vec::new();
    let mut variable_template_members = Vec::new();
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
            | EntityKind::ConversionFunction
            | EntityKind::FunctionTemplate => {
                if let Some(o) = build_overload_doc(&child, &child_ns) {
                    raw_methods.push(o);
                }
            }
            EntityKind::UsingDeclaration => {
                if let Some(member_name) = child.get_name() {
                    nested_usings.push(member_name.clone());

                    let mut resolved = false;
                    for target in using_decl::resolve_targets(&child, &member_name, index) {
                        match convert_using_target(
                            &target,
                            &child,
                            &member_name,
                            &child_ns,
                            index,
                        ) {
                            Some(UsingTarget::Method(o)) => {
                                raw_methods.push(o);
                                resolved = true;
                            }
                            Some(UsingTarget::Field(f)) => {
                                member_fields.push(f);
                                resolved = true;
                            }
                            Some(UsingTarget::Constant(c)) => {
                                constant_members.push(c);
                                resolved = true;
                            }
                            Some(UsingTarget::VariableTemplate(vt)) => {
                                variable_template_members.push(vt);
                                resolved = true;
                            }
                            Some(UsingTarget::Class(c)) => {
                                nested_classes.push(c);
                                resolved = true;
                            }
                            Some(UsingTarget::Enum(e)) => {
                                nested_enums.push(e);
                                resolved = true;
                            }
                            Some(UsingTarget::Alias(a)) => {
                                nested_aliases.push(a);
                                resolved = true;
                            }
                            None => {}
                        }
                    }
                    if resolved {
                        continue;
                    }

                    let raw = extract_raw_comment(&child).unwrap_or_default();
                    if !raw.is_empty() {
                        let parsed = parse_comment(&raw, &member_name, child_ns.clone(), entity_location(&child).as_deref());
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
            EntityKind::VarDecl => {
                // Static data members, constexpr constants, variable templates in class
                if is_variable_template_entity(&child) {
                    if let Some(vt) = build_variable_template(&child, &child_ns) {
                        variable_template_members.push(vt);
                    }
                } else if is_constant_variable(&child) {
                    if let Some(c) = build_constant_field(&child, &child_ns) {
                        constant_members.push(c);
                    }
                } else {
                    // Non-const static member — treat as regular field
                    if let Some(f) = build_member_field(&child, &child_ns) {
                        member_fields.push(f);
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
                if !child.is_definition() || super::is_anonymous_record(&child) {
                    continue;
                }
                if let Some(c) = build_class(&child, &child_ns, index) {
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
        constant_members,
        variable_template_members,
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

/// 把 `using` 指向的最终实体转成可归档的成员文档。
///
/// 访问级别取自 `using` 声明本身：基类私有继承、成员原本是 protected/private
/// 都不影响，只要这条 `using` 是 public 的就该出现在文档里。
fn convert_using_target<'tu>(
    target: &Entity<'tu>,
    using_decl: &Entity<'tu>,
    using_name: &str,
    child_ns: &[String],
    index: &ClassIndex<'tu>,
) -> Option<UsingTarget> {
    let access = build_access(using_decl);
    let using_raw = extract_raw_comment(using_decl).unwrap_or_default();
    match target.get_kind() {
        EntityKind::Method
        | EntityKind::Constructor
        | EntityKind::Destructor
        | EntityKind::ConversionFunction
        | EntityKind::FunctionTemplate => {
            let mut overload = build_overload_doc(target, child_ns)?;
            overload.doc.access = access;
            if overload.doc.base.name != using_name {
                if let Some(proto) = overload.decl_prototype.as_deref() {
                    overload.decl_prototype =
                        Some(rename_in_prototype(proto, &overload.doc.base.name, using_name));
                }
                overload.doc.base.name = using_name.to_string();
            }
            if !using_raw.is_empty() {
                apply_using_comment(&mut overload, &using_raw, using_name, child_ns, using_decl);
                if overload.doc.base.is_not_public {
                    return None;
                }
            }
            Some(UsingTarget::Method(overload))
        }
        EntityKind::FieldDecl => {
            let mut field = build_member_field(target, child_ns)?;
            field.access = access;
            field.base.name = using_name.to_string();
            apply_basic_comment(&mut field.base, &using_raw, using_name, child_ns, using_decl);
            if field.base.is_not_public {
                return None;
            }
            Some(UsingTarget::Field(field))
        }
        EntityKind::VarDecl => {
            if is_variable_template_entity(target) {
                let mut vt = build_variable_template(target, child_ns)?;
                vt.access = access;
                vt.base.name = using_name.to_string();
                apply_basic_comment(&mut vt.base, &using_raw, using_name, child_ns, using_decl);
                if vt.base.is_not_public {
                    return None;
                }
                Some(UsingTarget::VariableTemplate(vt))
            } else {
                let mut constant = build_constant_field(target, child_ns)?;
                constant.access = access;
                constant.base.name = using_name.to_string();
                apply_basic_comment(&mut constant.base, &using_raw, using_name, child_ns, using_decl);
                if constant.base.is_not_public {
                    return None;
                }
                Some(UsingTarget::Constant(constant))
            }
        }
        EntityKind::ClassDecl
        | EntityKind::StructDecl
        | EntityKind::UnionDecl
        | EntityKind::ClassTemplate
        | EntityKind::ClassTemplatePartialSpecialization => {
            let mut class = build_class(target, child_ns, index)?;
            class.access = access;
            class.base.name = using_name.to_string();
            apply_basic_comment(&mut class.base, &using_raw, using_name, child_ns, using_decl);
            if class.base.is_not_public {
                return None;
            }
            Some(UsingTarget::Class(class))
        }
        EntityKind::EnumDecl => {
            let mut e = build_enum(target, child_ns)?;
            e.access = access;
            e.base.name = using_name.to_string();
            apply_basic_comment(&mut e.base, &using_raw, using_name, child_ns, using_decl);
            if e.base.is_not_public {
                return None;
            }
            Some(UsingTarget::Enum(e))
        }
        EntityKind::TypeAliasDecl | EntityKind::TypedefDecl => {
            let mut alias = build_type_alias(target, child_ns)?;
            alias.access = access;
            alias.base.name = using_name.to_string();
            apply_basic_comment(&mut alias.base, &using_raw, using_name, child_ns, using_decl);
            if alias.base.is_not_public {
                return None;
            }
            Some(UsingTarget::Alias(alias))
        }
        _ => None,
    }
}

/// 用 `using` 声明自带的注释覆盖签名实体上的文档。
fn apply_using_comment(
    overload: &mut OverloadDoc,
    raw: &str,
    using_name: &str,
    child_ns: &[String],
    using_decl: &Entity,
) {
    let parsed = parse_comment(
        raw,
        using_name,
        child_ns.to_vec(),
        entity_location(using_decl).as_deref(),
    );
    for param in overload.doc.params.iter_mut() {
        if let Some(doc_param) = parsed.params.iter().find(|p| p.name == param.name) {
            param.description = doc_param.description.clone();
        }
    }
    overload.doc.return_doc.description = parsed.return_doc.description.clone();
    overload.doc.exceptions = parsed.exceptions.clone();
    let name = overload.doc.base.name.clone();
    overload.doc.base = parsed.basic;
    overload.doc.base.name = name;
}

fn apply_basic_comment(
    basic: &mut BasicDocument,
    raw: &str,
    using_name: &str,
    child_ns: &[String],
    using_decl: &Entity,
) {
    if raw.is_empty() {
        return;
    }
    let parsed = parse_comment(
        raw,
        using_name,
        child_ns.to_vec(),
        entity_location(using_decl).as_deref(),
    );
    *basic = parsed.basic;
}

fn rename_in_prototype(proto: &str, old_name: &str, new_name: &str) -> String {
    if old_name.is_empty() || old_name == new_name {
        return proto.to_string();
    }
    let needle = format!("{old_name}(");
    match proto.find(&needle) {
        Some(pos) => {
            let mut out = String::with_capacity(proto.len() + new_name.len());
            out.push_str(&proto[..pos]);
            out.push_str(new_name);
            out.push_str(&proto[pos + old_name.len()..]);
            out
        }
        None => proto.to_string(),
    }
}

fn group_by_name(methods: Vec<OverloadDoc>) -> Vec<OverloadGroup> {
    let mut map: IndexMap<String, Vec<OverloadDoc>> = IndexMap::new();
    for m in methods {
        let entry = map.entry(m.doc.base.name.clone()).or_default();
        // 继承构造函数会与类自身的隐式构造函数撞原型，同原型只保留先到的那份
        let duplicate = m.decl_prototype.as_deref().is_some_and(|proto| {
            let proto = normalize_prototype(proto);
            entry.iter().any(|existing| {
                existing
                    .decl_prototype
                    .as_deref()
                    .is_some_and(|other| normalize_prototype(other) == proto)
            })
        });
        if !duplicate {
            entry.push(m);
        }
    }
    map.into_iter()
        .map(|(name, overloads)| OverloadGroup { name, overloads })
        .collect()
}

fn normalize_prototype(proto: &str) -> String {
    proto.split_whitespace().collect::<Vec<_>>().join(" ")
}
