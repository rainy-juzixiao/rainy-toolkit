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
use clang::Entity;
use crate::data::document::ConceptDocument;
use super::comment::{entity_location, extract_raw_comment, parse_comment};
use super::function::build_template_params;

/// clang crate 没有 `ConceptDecl` 变体，概念声明会被映射成
/// `NotImplemented`（更老的 libclang 则可能给出别的不透明 kind），
/// 因此按源码 token 判断：声明里出现 `concept` 关键字即为概念。
pub fn is_concept_decl(entity: &Entity) -> bool {
    let opaque = matches!(
        entity.get_kind(),
        clang::EntityKind::NotImplemented | clang::EntityKind::UnexposedDecl
    );
    if !opaque && !format!("{:?}", entity.get_kind()).contains("Concept") {
        return false;
    }
    concept_tokens(entity).iter().any(|t| t == "concept")
}

fn concept_tokens(entity: &Entity) -> Vec<String> {
    entity
        .get_range()
        .map(|r| {
            r.tokenize()
                .iter()
                .map(|t| t.get_spelling())
                .collect::<Vec<_>>()
        })
        .unwrap_or_default()
}

/// `template <...> concept Name = <constraint>;` —— 取 `=` 之后的约束表达式。
fn constraint_expression(entity: &Entity) -> String {
    let tokens = concept_tokens(entity);
    let Some(eq) = tokens.iter().position(|t| t == "=") else {
        return String::new();
    };
    tokens[eq + 1..]
        .iter()
        .filter(|t| t.as_str() != ";")
        .cloned()
        .collect::<Vec<_>>()
        .join(" ")
}

pub fn build_concept(
    entity: &Entity,
    namespace_stack: &[String],
) -> Option<ConceptDocument> {
    if !is_concept_decl(entity) {
        return None;
    }
    let name = entity.get_name()?;
    let raw  = extract_raw_comment(entity).unwrap_or_default();
    let mut parsed = parse_comment(&raw, &name, namespace_stack.to_vec(), entity_location(entity).as_deref());
    let (is_template, template_params) =
        build_template_params(entity, &parsed.tparams_desc);
    parsed.basic.is_template     = is_template;
    parsed.basic.template_params = template_params;
    let params = parsed.basic.template_params.clone();
    Some(ConceptDocument {
        params,
        constraint_expression: constraint_expression(entity),
        base: parsed.basic,
    })
}