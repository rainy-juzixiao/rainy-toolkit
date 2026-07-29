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
use super::comment::{extract_raw_comment, parse_comment};
use super::function::build_access;
use super::function::build_template_params;
use crate::data::document::{
    ConstantDocument, MemberFieldDocument, VariableDocument, VariableTemplateDocument,
};
use clang::{Entity, EntityKind};

pub fn build_variable(entity: &Entity, namespace_stack: &[String]) -> Option<VariableDocument> {
    if !matches!(entity.get_kind(), EntityKind::VarDecl) {
        return None;
    }

    let name = entity.get_name()?;
    let raw = extract_raw_comment(entity).unwrap_or_default();
    let parsed = parse_comment(&raw, &name, namespace_stack.to_vec());

    let type_name = entity
        .get_type()
        .map(|t| t.get_display_name())
        .unwrap_or_default();

    Some(VariableDocument {
        is_constexpr: entity.is_const_method(),
        is_constinit: false,
        is_const: entity
            .get_type()
            .map(|t| t.is_const_qualified())
            .unwrap_or(false),
        is_static: entity
            .get_storage_class()
            .map(|s| s == clang::StorageClass::Static)
            .unwrap_or(false),
        is_inline: false,
        type_name,
        default_value: None,
        base: parsed.basic,
    })
}

pub fn build_member_field(
    entity: &Entity,
    namespace_stack: &[String],
) -> Option<MemberFieldDocument> {
    if !matches!(entity.get_kind(), EntityKind::FieldDecl) {
        return None;
    }
    let name = entity.get_name()?;
    let raw = extract_raw_comment(entity).unwrap_or_default();
    let parsed = parse_comment(&raw, &name, namespace_stack.to_vec());
    let type_name = entity
        .get_type()
        .map(|t| t.get_display_name())
        .unwrap_or_default();
    Some(MemberFieldDocument {
        is_constexpr: false,
        is_static: false,
        is_mutable: entity.is_mutable(),
        is_const: entity
            .get_type()
            .map(|t| t.is_const_qualified())
            .unwrap_or(false),
        access: build_access(entity),
        type_name,
        default_value: None,
        base: parsed.basic,
    })
}

/// Build a ConstantDocument from a VarDecl with const-qualified type.
pub fn build_constant(
    entity: &Entity,
    namespace_stack: &[String],
) -> Option<ConstantDocument> {
    if !matches!(entity.get_kind(), EntityKind::VarDecl) {
        return None;
    }
    let name = entity.get_name()?;
    let raw = extract_raw_comment(entity).unwrap_or_default();
    let parsed = parse_comment(&raw, &name, namespace_stack.to_vec());
    let type_name = entity
        .get_type()
        .map(|t| t.get_display_name())
        .unwrap_or_default();
    Some(ConstantDocument {
        is_constexpr: false,
        is_constinit: false,
        is_static: entity
            .get_storage_class()
            .map(|s| s == clang::StorageClass::Static)
            .unwrap_or(false),
        is_inline: false,
        type_name,
        default_value: None,
        access: build_access(entity),
        base: parsed.basic,
    })
}

/// Build a VariableTemplateDocument from a VarDecl that has template parameter children.
pub fn build_variable_template(
    entity: &Entity,
    namespace_stack: &[String],
) -> Option<VariableTemplateDocument> {
    if !matches!(entity.get_kind(), EntityKind::VarDecl) {
        return None;
    }
    let name = entity.get_name()?;
    let raw = extract_raw_comment(entity).unwrap_or_default();
    let mut parsed = parse_comment(&raw, &name, namespace_stack.to_vec());
    let type_name = entity
        .get_type()
        .map(|t| t.get_display_name())
        .unwrap_or_default();
    // Populate template parameters
    let (is_template, template_params) = build_template_params(entity, &parsed.tparams_desc);
    parsed.basic.is_template = is_template;
    parsed.basic.template_params = template_params;

    Some(VariableTemplateDocument {
        is_constexpr: false,
        is_const: entity
            .get_type()
            .map(|t| t.is_const_qualified())
            .unwrap_or(false),
        is_static: entity
            .get_storage_class()
            .map(|s| s == clang::StorageClass::Static)
            .unwrap_or(false),
        is_inline: false,
        type_name,
        default_value: None,
        access: build_access(entity),
        base: parsed.basic,
    })
}

/// Check whether a VarDecl entity is a variable template (has template parameter children).
pub fn is_variable_template_entity(entity: &Entity) -> bool {
    entity
        .get_children()
        .iter()
        .any(|c| matches!(c.get_kind(), EntityKind::TemplateTypeParameter
                               | EntityKind::NonTypeTemplateParameter
                               | EntityKind::TemplateTemplateParameter))
}

/// Check whether a VarDecl is a constant (type is const-qualified).
pub fn is_constant_variable(entity: &Entity) -> bool {
    entity
        .get_type()
        .map(|t| t.is_const_qualified())
        .unwrap_or(false)
}

/// Try to extract a VarDecl from an UnexposedDecl that wraps a variable template.
/// Variable templates are often represented as UnexposedDecl with template params + VarDecl.
pub fn unwrap_variable_template_candidate<'a>(entity: &'a Entity<'a>) -> Option<Entity<'a>> {
    if entity.get_kind() != EntityKind::UnexposedDecl {
        return None;
    }
    let children = entity.get_children();
    let has_template_params = children.iter().any(|c| matches!(
        c.get_kind(),
        EntityKind::TemplateTypeParameter
            | EntityKind::NonTypeTemplateParameter
            | EntityKind::TemplateTemplateParameter
    ));
    if !has_template_params {
        return None;
    }
    children.into_iter().find(|c| c.get_kind() == EntityKind::VarDecl)
}
