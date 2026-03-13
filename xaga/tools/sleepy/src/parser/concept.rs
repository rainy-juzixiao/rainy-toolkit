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
use crate::data::document::ConceptDocument;
use super::comment::{extract_raw_comment, parse_comment};
use super::function::build_template_params;

pub fn is_concept_decl(entity: &Entity) -> bool {
    format!("{:?}", entity.get_kind()).contains("Concept")
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
    let mut parsed = parse_comment(&raw, &name, namespace_stack.to_vec());
    let (is_template, template_params) =
        build_template_params(entity, &parsed.tparams_desc);
    parsed.basic.is_template     = is_template;
    parsed.basic.template_params = template_params;
    let constraint_expression = entity.get_children().into_iter()
        .find(|c| format!("{:?}", c.get_kind()).contains("Requires"))
        .and_then(|c| c.get_display_name())
        .unwrap_or_default();
    let params = parsed.basic.template_params.clone();
    Some(ConceptDocument {
        params,
        constraint_expression,
        base: parsed.basic,
    })
}