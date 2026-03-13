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

pub fn is_noexcept(entity: &Entity) -> bool {
    entity.get_exception_specification()
        .map(|s| matches!(s,
            clang::ExceptionSpecification::BasicNoexcept |
            clang::ExceptionSpecification::NoThrow
        ))
        .unwrap_or(false)
}

pub fn is_override(entity: &Entity) -> bool {
    // libclang 没有直接 API，但 CXCursor_CXXOverrideAttr 子节点能判断
    entity.get_children().iter().any(|c| {
        format!("{:?}", c.get_kind()).contains("Override")
    })
}

pub fn is_deleted(entity: &Entity) -> bool {
    entity.get_availability() == clang::Availability::Unavailable
}

pub fn is_defaulted(entity: &Entity) -> bool {
    entity.is_defaulted()
}

pub fn is_constexpr(entity: &Entity) -> bool {
    // clang crate 暂无直接 API，通过 children 里找 ConstexprAttr
    entity.get_children().iter().any(|c| {
        format!("{:?}", c.get_kind()).contains("Constexpr")
    })
}

pub fn is_inline(entity: &Entity) -> bool {
    entity.is_inline_function()
}

// is_ref_lvalue / is_ref_rvalue 用 display_name 里的签名解析
pub fn is_ref_lvalue(entity: &Entity) -> bool {
    entity.get_display_name()
        .map(|s| s.ends_with(" &") || s.contains(") & ") || s.contains(") &\n"))
        .unwrap_or(false)
}

pub fn is_ref_rvalue(entity: &Entity) -> bool {
    entity.get_display_name()
        .map(|s| s.ends_with(" &&") || s.contains(") && ") || s.contains(") &&\n"))
        .unwrap_or(false)
}

pub fn is_explicit(entity: &Entity) -> bool {
    entity.get_children().iter().any(|c| {
        format!("{:?}", c.get_kind()).contains("Explicit")
    })
}

pub fn is_final(entity: &Entity) -> bool {
    entity.get_children().iter().any(|c| {
        format!("{:?}", c.get_kind()).contains("Final")
    })
}

pub fn is_consteval(entity: &Entity) -> bool {
    entity.get_children().iter().any(|c| {
        format!("{:?}", c.get_kind()).contains("Consteval")
    })
}