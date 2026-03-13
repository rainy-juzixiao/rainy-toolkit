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
use crate::data::document::{MemberFieldDocument, VariableDocument};
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
