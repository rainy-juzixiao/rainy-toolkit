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
use crate::data::document::{TypeAliasDocument, TypeAliasKind};
use super::comment::{extract_raw_comment, parse_comment};
use super::function::build_access;

pub fn build_type_alias(
    entity: &Entity,
    namespace_stack: &[String],
) -> Option<TypeAliasDocument> {
    let kind = match entity.get_kind() {
        EntityKind::TypeAliasDecl => TypeAliasKind::Using,
        EntityKind::TypedefDecl   => TypeAliasKind::Typedef,
        _                         => return None,
    };

    let name = entity.get_name()?;
    let raw  = extract_raw_comment(entity).unwrap_or_default();
    let parsed = parse_comment(&raw, &name, namespace_stack.to_vec());

    let target_type = entity.get_typedef_underlying_type()
        .map(|t| t.get_display_name())
        .unwrap_or_default();

    Some(TypeAliasDocument {
        kind,
        target_type,
        access: build_access(entity),
        base: parsed.basic,
    })
}