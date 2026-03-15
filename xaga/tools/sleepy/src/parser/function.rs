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
use crate::data::document::{
    AccessLevel, FreeFunctionDocument, MemberFunctionDocument, TemplateParam,
};
use crate::utils::traits::{
    is_consteval, is_constexpr, is_defaulted, is_deleted, is_explicit, is_final, is_inline,
    is_override, is_ref_lvalue, is_ref_rvalue,
};
use clang::{Entity, EntityKind};

pub fn build_free_function(
    entity: &Entity,
    namespace_stack: &[String],
) -> Option<FreeFunctionDocument> {
    if !matches!(
        entity.get_kind(),
        EntityKind::FunctionDecl | EntityKind::FunctionTemplate
    ) {
        return None;
    }
    let name = entity.get_name()?;
    let raw = extract_raw_comment(entity).unwrap_or_default();
    let mut parsed = parse_comment(&raw, &name, namespace_stack.to_vec());
    if let Some(t) = entity.get_type() {
        let arg_types = t.get_argument_types().unwrap_or_default();
        for (i, param) in parsed.params.iter_mut().enumerate() {
            if param.type_name.is_empty() {
                if let Some(arg_ty) = arg_types.get(i) {
                    param.type_name = arg_ty.get_display_name();
                }
            }
        }
    }
    let return_type = entity
        .get_result_type()
        .map(|t| t.get_display_name())
        .unwrap_or_default();
    parsed.return_doc.type_name = return_type;
    // template params
    let (is_template, template_params) = build_template_params(entity, &parsed.tparams_desc);
    parsed.basic.is_template = is_template;
    parsed.basic.template_params = template_params;
    let doc = FreeFunctionDocument {
        is_static: entity
            .get_storage_class()
            .map(|s| s == clang::StorageClass::Static)
            .unwrap_or(false),
        is_inline: entity.is_inline_function(),
        is_constexpr: entity.is_const_method(),
        is_consteval: false,
        is_noexcept: is_noexcept(entity),
        params: parsed.params,
        return_doc: parsed.return_doc,
        exceptions: parsed.exceptions,
        base: parsed.basic,
    };
    if doc.base.is_not_public {
        return None;
    }
    Some(doc)
}

pub fn build_member_function(
    entity: &Entity,
    namespace_stack: &[String],
) -> Option<MemberFunctionDocument> {
    if !matches!(
        entity.get_kind(),
        EntityKind::Method
            | EntityKind::Constructor
            | EntityKind::Destructor
            | EntityKind::ConversionFunction
    ) {
        return None;
    }

    let name = entity.get_name()?;
    let raw = extract_raw_comment(entity).unwrap_or_default();
    let mut parsed = parse_comment(&raw, &name, namespace_stack.to_vec());
    if let Some(t) = entity.get_type() {
        let arg_types = t.get_argument_types().unwrap_or_default();
        for (i, param) in parsed.params.iter_mut().enumerate() {
            if param.type_name.is_empty() {
                if let Some(arg_ty) = arg_types.get(i) {
                    param.type_name = arg_ty.get_display_name();
                }
            }
        }
    }

    let return_type = entity
        .get_result_type()
        .map(|t| t.get_display_name())
        .unwrap_or_default();
    parsed.return_doc.type_name = return_type;

    let (is_template, template_params) = build_template_params(entity, &parsed.tparams_desc);
    parsed.basic.is_template = is_template;
    parsed.basic.template_params = template_params;

    let doc = MemberFunctionDocument {
        is_const: entity.is_const_method(),
        is_noexcept: is_noexcept(entity),
        is_for_lvalue: is_ref_lvalue(entity),
        is_for_rvalue: is_ref_rvalue(entity),
        is_virtual: entity.is_virtual_method(),
        is_pure_virtual: entity.is_pure_virtual_method(),
        is_override: is_override(entity),
        is_final: is_final(entity),
        is_static: entity.is_static_method(),
        is_explicit: is_explicit(entity),
        is_inline: is_inline(entity),
        is_constexpr: is_constexpr(entity),
        is_consteval: is_consteval(entity),
        is_defaulted: is_defaulted(entity),
        is_deleted: is_deleted(entity),
        access: build_access(entity),
        params: parsed.params,
        return_doc: parsed.return_doc,
        exceptions: parsed.exceptions,
        base: parsed.basic,
    };

    if doc.base.is_not_public {
        return None;
    }

    Some(doc)
}
pub fn build_overload_doc(
    entity: &Entity,
    namespace_stack: &[String],
) -> Option<crate::data::document::OverloadDoc> {
    let doc = build_member_function(entity, namespace_stack)?;

    // @overload_decl 手动原型优先，否则从 AST 自动生成
    let decl_prototype = doc
        .base
        .overload_decl
        .clone()
        .or_else(|| build_prototype(entity, &doc));

    Some(crate::data::document::OverloadDoc {
        decl_prototype,
        doc,
    })
}

pub fn build_template_params(
    entity: &Entity,
    descs: &std::collections::HashMap<String, crate::i18n::I18n<String>>,
) -> (bool, Vec<TemplateParam>) {
    let raw_tparams: Vec<_> = entity
        .get_children()
        .into_iter()
        .filter(|c| {
            matches!(
                c.get_kind(),
                EntityKind::TemplateTypeParameter | EntityKind::NonTypeTemplateParameter
            )
        })
        .collect();

    let is_template = !raw_tparams.is_empty();
    let params = raw_tparams
        .into_iter()
        .map(|tp| {
            let name = tp.get_name().unwrap_or_default();
            let desc = descs
                .get(&name)
                .cloned()
                .unwrap_or_else(crate::i18n::I18n::new);
            TemplateParam {
                name: name.clone(),
                constraint: None, // concept 约束需要更深的 AST 分析，暂留
                default_value: tp.get_children().first().and_then(|c| c.get_display_name()),
                description: desc,
            }
        })
        .collect();

    (is_template, params)
}

pub fn build_access(entity: &Entity) -> AccessLevel {
    match entity.get_accessibility() {
        Some(clang::Accessibility::Protected) => AccessLevel::Protected,
        Some(clang::Accessibility::Private) => AccessLevel::Private,
        _ => AccessLevel::Public,
    }
}

pub fn is_noexcept(entity: &Entity) -> bool {
    // libclang 没有直接的 is_noexcept，检查 exception spec
    entity
        .get_exception_specification()
        .map(|s| {
            s == clang::ExceptionSpecification::BasicNoexcept
                || s == clang::ExceptionSpecification::NoThrow
        })
        .unwrap_or(false)
}

fn build_prototype(entity: &Entity, doc: &MemberFunctionDocument) -> Option<String> {
    let name = entity.get_name()?;
    let return_type = entity
        .get_result_type()
        .map(|t| t.get_display_name())
        .unwrap_or_default();
    let params: Vec<String> = entity
        .get_children()
        .into_iter()
        .filter(|c| c.get_kind() == EntityKind::ParmDecl)
        .map(|c| {
            let ty = c
                .get_type()
                .map(|t| t.get_display_name())
                .unwrap_or_default();
            let pname = c.get_name().unwrap_or_default();
            if pname.is_empty() {
                ty
            } else {
                format!("{} {}", ty, pname)
            }
        })
        .collect();
    let mut proto = if return_type.is_empty() {
        // 构造函数 / 析构函数
        format!("{}({})", name, params.join(", "))
    } else {
        format!("{} {}({})", return_type, name, params.join(", "))
    };
    // 修饰符后缀
    if doc.is_const {
        proto.push_str(" const");
    }
    if doc.is_noexcept {
        proto.push_str(" noexcept");
    }
    if doc.is_override {
        proto.push_str(" override");
    }
    if doc.is_final {
        proto.push_str(" final");
    }
    if doc.is_for_lvalue {
        proto.push_str(" &");
    }
    if doc.is_for_rvalue {
        proto.push_str(" &&");
    }
    if doc.is_deleted {
        proto.push_str(" = delete");
    }
    if doc.is_defaulted {
        proto.push_str(" = default");
    }
    proto.push(';');

    Some(proto)
}
