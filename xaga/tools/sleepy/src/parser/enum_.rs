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
use crate::data::document::{EnumDocument, EnumVariantDocument};
use crate::i18n::I18n;
use super::comment::{entity_location, extract_raw_comment, parse_comment};
use super::function::build_access;

pub fn build_enum(
    entity: &Entity,
    namespace_stack: &[String],
) -> Option<EnumDocument> {
    if !matches!(entity.get_kind(), EntityKind::EnumDecl) {
        return None;
    }

    let name = entity.get_name()?;
    let raw  = extract_raw_comment(entity).unwrap_or_default();
    let parsed = parse_comment(&raw, &name, namespace_stack.to_vec(), entity_location(entity).as_deref());

    let underlying_type = entity.get_enum_underlying_type()
        .map(|t| t.get_display_name());

    let variants = entity.get_children().into_iter()
        .filter(|c| c.get_kind() == EntityKind::EnumConstantDecl)
        .map(|c| build_enum_variant(&c))
        .collect();

    Some(EnumDocument {
        is_scoped:       entity.is_scoped(),
        underlying_type,
        access:          build_access(entity),
        variants,
        base:            parsed.basic,
    })
}

fn build_enum_variant(entity: &Entity) -> EnumVariantDocument {
    let name  = entity.get_name().unwrap_or_default();
    let value = entity.get_enum_constant_value()
        .map(|(v, _)| v.to_string());

    let raw = extract_raw_comment(entity).unwrap_or_default();
    // variant 的注释通常只有 brief/details
    let (brief, description, documented) = if raw.is_empty() {
        (I18n::new(), I18n::new(), false)
    } else {
        let p = parse_comment(&raw, &name, vec![], entity_location(entity).as_deref());
        (p.basic.brief, p.basic.description, p.basic.is_documented)
    };
    // libclang 会把某个枚举项的注释也挂到它后面的枚举项上，
    // 只有注释结束行紧挨着声明行时才算该枚举项自己的文档
    let documented = documented && comment_is_attached(entity);

    EnumVariantDocument {
        name,
        value,
        brief,
        description,
        is_documented: documented,
        location: entity_location(entity),
    }
}

fn comment_is_attached(entity: &Entity) -> bool {
    let Some(end_line) = entity
        .get_comment_range()
        .map(|r| r.get_end().get_file_location().line)
    else {
        return false;
    };
    let Some(entity_line) = entity.get_location().map(|l| l.get_file_location().line) else {
        return false;
    };
    end_line + 1 == entity_line
}
