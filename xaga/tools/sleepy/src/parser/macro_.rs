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
use crate::data::document::MacroDocument;
use clang::{Entity, EntityKind};

pub fn build_macro(entity: &Entity, namespace_stack: &[String]) -> Option<MacroDocument> {
    if !matches!(entity.get_kind(), EntityKind::MacroDefinition) {
        return None;
    }

    let name = entity.get_name()?;
    let raw = extract_raw_comment(entity).unwrap_or_default();
    if raw.is_empty() {
        // 为了避免宏的污染，仅限在构建宏的时候，检查是否定义注释，如果未提供注释文档，则不进行生成
        return None;
    }
    let parsed = parse_comment(&raw, &name, namespace_stack.to_vec());

    let is_function_like = entity.is_function_like_macro();
    let params = if is_function_like {
        extract_macro_params(entity)
    } else {
        vec![]
    };
    Some(MacroDocument {
        is_function_like,
        params,
        expansion: None,
        base: parsed.basic,
    })
}

fn extract_macro_params(entity: &Entity) -> Vec<String> {
    let Some(range) = entity.get_range() else {
        return vec![];
    };
    let tokens = range.tokenize();
    let mut params = Vec::new();
    let mut in_paren = false;

    for token in &tokens {
        let spelling = token.get_spelling();
        match spelling.as_str() {
            "(" => in_paren = true,
            ")" => break,
            "," => {}
            s if in_paren => params.push(s.to_string()),
            _ => {}
        }
    }
    params
}
