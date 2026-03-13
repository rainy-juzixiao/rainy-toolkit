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
use crate::data::document::*;
use crate::i18n::{I18n, LangTag};
use std::fmt::Write;

pub struct MarkdownGenerator {
    pub lang: LangTag,
}

impl MarkdownGenerator {
    pub fn new(lang: impl Into<String>) -> Self {
        Self {
            lang: LangTag(lang.into()),
        }
    }

    fn get<'a>(&self, i18n: &'a I18n<String>) -> Option<&'a String> {
        i18n.get(Some(&self.lang))
            .or_else(|| i18n.translations.values().next())
            .or_else(|| i18n.default.as_ref())
    }

    fn get_vec<'a>(&self, i18n: &'a I18n<Vec<String>>) -> Option<&'a Vec<String>> {
        i18n.get(Some(&self.lang))
            .or_else(|| i18n.translations.values().next())
            .or_else(|| i18n.default.as_ref())
    }

    pub fn generate_file(&self, doc: &FileDocument) -> String {
        let mut out = String::new();

        let file_name = std::path::Path::new(&doc.file_path)
            .file_name()
            .and_then(|n| n.to_str())
            .unwrap_or(&doc.file_path);

        writeln!(out, "# {}", file_name).unwrap();
        writeln!(out).unwrap();

        if let Some(brief) = self.get(&doc.brief) {
            writeln!(out, "{}", brief).unwrap();
            writeln!(out).unwrap();
        }
        if let Some(desc) = self.get(&doc.description) {
            writeln!(out, "{}", desc).unwrap();
            writeln!(out).unwrap();
        }
        let visible_classes: Vec<_> = doc
            .classes
            .iter()
            .chain(doc.namespaces.iter().flat_map(|ns| collect_all_classes(ns)))
            .filter(|c| !c.base.is_hidden && !c.base.is_not_public)
            .collect();
        let visible_enums: Vec<_> = doc
            .enums
            .iter()
            .chain(doc.namespaces.iter().flat_map(|ns| ns.enums.iter()))
            .filter(|e| !e.base.is_not_public)
            .collect();
        let visible_vars: Vec<_> = doc
            .variables
            .iter()
            .chain(doc.namespaces.iter().flat_map(|ns| ns.variables.iter()))
            .filter(|v| !v.base.is_not_public)
            .collect();
        let visible_fns: Vec<_> = doc
            .free_functions
            .iter()
            .chain(
                doc.namespaces
                    .iter()
                    .flat_map(|ns| collect_all_functions(ns)),
            )
            .filter(|c| !c.base.is_hidden && !c.base.is_not_public)
            .collect();
        let marcos: Vec<_> = doc
            .macros
            .iter()
            .filter(|c| !c.base.is_hidden && !c.base.is_not_public)
            .collect();
        // 概览表格
        if !visible_classes.is_empty() {
            writeln!(out, "## Classes").unwrap();
            writeln!(out).unwrap();
            writeln!(out, "| Name | Description |").unwrap();
            writeln!(out, "|------|-------------|").unwrap();
            for c in &visible_classes {
                let brief = self.get(&c.base.brief).map(|s| s.as_str()).unwrap_or("");
                let anchor = class_anchor(&c.base.name);
                writeln!(out, "| [`{}`](#{}) | {} |", c.base.name, anchor, brief).unwrap();
            }
            writeln!(out).unwrap();
        }

        if !visible_enums.is_empty() {
            writeln!(out, "## Enums").unwrap();
            writeln!(out).unwrap();
            writeln!(out, "| Name | Description |").unwrap();
            writeln!(out, "|------|-------------|").unwrap();
            for e in &visible_enums {
                let brief = self.get(&e.base.brief).map(|s| s.as_str()).unwrap_or("");
                let anchor = class_anchor(&e.base.name);
                writeln!(out, "| [`{}`](#{}) | {} |", e.base.name, anchor, brief).unwrap();
            }
            writeln!(out).unwrap();
        }

        if !visible_vars.is_empty() {
            writeln!(out, "## Variables").unwrap();
            writeln!(out).unwrap();
            writeln!(out, "| Name | Type | Description |").unwrap();
            writeln!(out, "|------|------|-------------|").unwrap();
            for v in &visible_vars {
                let brief = self.get(&v.base.brief).map(|s| s.as_str()).unwrap_or("");
                let anchor = class_anchor(&v.base.name);
                writeln!(
                    out,
                    "| [`{}`](#{}) | `{}` | {} |",
                    v.base.name, anchor, v.type_name, brief
                )
                .unwrap();
            }
            writeln!(out).unwrap();
        }

        if !visible_fns.is_empty() {
            writeln!(out, "## Functions").unwrap();
            writeln!(out).unwrap();
            writeln!(out, "| Name | Description |").unwrap();
            writeln!(out, "|------|-------------|").unwrap();
            for f in &visible_fns {
                let brief = self.get(&f.base.brief).map(|s| s.as_str()).unwrap_or("");
                let anchor = class_anchor(&f.base.name);
                writeln!(out, "| [`{}`](#{}) | {} |", f.base.name, anchor, brief).unwrap();
            }
            writeln!(out).unwrap();
        }

        if !marcos.is_empty() {
            writeln!(out, "## Marcos").unwrap();
            writeln!(out).unwrap();
            writeln!(out, "| Name | Description |").unwrap();
            writeln!(out, "|------|-------------|").unwrap();
            for m in &marcos {
                let brief = self.get(&m.base.brief).map(|s| s.as_str()).unwrap_or("");
                writeln!(out, "| [`{}`](#{}) | {} |", m.base.name, m.base.name, brief).unwrap();
            }
            writeln!(out).unwrap();
        }

        writeln!(out, "---").unwrap();
        writeln!(out).unwrap();
        // 详细文档
        for c in &visible_classes {
            out.push_str(&self.generate_class(c, 2));
        }
        for e in &visible_enums {
            out.push_str(&self.generate_enum(e, 2));
        }
        for v in &visible_vars {
            out.push_str(&self.generate_variable(v, 2));
        }
        for f in &visible_fns {
            out.push_str(&self.generate_free_function(f, 2));
        }
        out
    }

    pub fn generate_class(&self, c: &ClassDocument, heading: usize) -> String {
        let mut out = String::new();
        let h = "#".repeat(heading);
        let kind = match c.kind {
            ClassKind::Class => "class",
            ClassKind::Struct => "struct",
            ClassKind::Union => "union",
        };
        // 标题
        writeln!(out, "{} {} `{}`", h, kind, c.base.name).unwrap();
        writeln!(out).unwrap();

        // 模板标记
        if c.base.is_main_template {
            writeln!(out, "> **Primary template**").unwrap();
            writeln!(out).unwrap();
        }
        if !c.base.spec_template_args.is_empty() {
            writeln!(
                out,
                "> **Specialization** `<{}>`",
                c.base.spec_template_args.join(", ")
            )
            .unwrap();
            writeln!(out).unwrap();
        }
        // 模板参数
        if c.base.is_template && !c.base.template_params.is_empty() {
            writeln!(out, "**Template parameters**").unwrap();
            writeln!(out).unwrap();
            writeln!(out, "| Parameter | Description |").unwrap();
            writeln!(out, "|-----------|-------------|").unwrap();
            for tp in &c.base.template_params {
                let desc = self.get(&tp.description).map(|s| s.as_str()).unwrap_or("");
                writeln!(out, "| `{}` | {} |", tp.name, desc).unwrap();
            }
            writeln!(out).unwrap();
        }
        // brief / details
        if let Some(brief) = self.get(&c.base.brief) {
            writeln!(out, "{}", brief).unwrap();
            writeln!(out).unwrap();
        }
        if let Some(desc) = self.get(&c.base.description) {
            writeln!(out, "{}", desc).unwrap();
            writeln!(out).unwrap();
        }
        // 继承
        if !c.base_classes.is_empty() {
            let bases: Vec<String> = c
                .base_classes
                .iter()
                .map(|b| {
                    let access = match b.access {
                        AccessLevel::Public => "public",
                        AccessLevel::Protected => "protected",
                        AccessLevel::Private => "private",
                    };
                    format!("`{}` {}", access, b.name)
                })
                .collect();
            writeln!(out, "**Inherits:** {}", bases.join(", ")).unwrap();
            writeln!(out).unwrap();
        }
        // 通用注释块
        out.push_str(&self.generate_common_tags(&c.base));
        // API Ref 表格
        let visible_methods: Vec<_> = c
            .overload_groups
            .iter()
            .filter(|g| !g.overloads.iter().all(|o| o.doc.base.is_not_public))
            .collect();
        let visible_fields: Vec<_> = c
            .member_fields
            .iter()
            .filter(|f| matches!(f.access, AccessLevel::Public) && !f.base.is_not_public)
            .collect();
        let visible_nested: Vec<_> = c
            .nested_classes
            .iter()
            .filter(|n| !n.base.is_hidden && !n.base.is_not_public)
            .collect();
        let visible_nested_enums: Vec<_> = c
            .nested_enums
            .iter()
            .filter(|e| !e.base.is_not_public)
            .collect();
        if !visible_methods.is_empty() {
            writeln!(out, "### Member Functions").unwrap();
            writeln!(out).unwrap();
            writeln!(out, "| Function | Description |").unwrap();
            writeln!(out, "|----------|-------------|").unwrap();
            for group in &visible_methods {
                let brief = group
                    .overloads
                    .first()
                    .and_then(|o| self.get(&o.doc.base.brief))
                    .map(|s| s.as_str())
                    .unwrap_or("");
                let anchor = method_anchor(&c.base.name, &group.name);
                let overload_note = if group.overloads.len() > 1 {
                    format!(" *({} overloads)*", group.overloads.len())
                } else {
                    String::new()
                };
                writeln!(
                    out,
                    "| [{}](#{}){}  | {} |",
                    group.name, anchor, overload_note, brief
                )
                .unwrap();
            }
            writeln!(out).unwrap();
        }
        if !visible_fields.is_empty() {
            writeln!(out, "### Member Variables").unwrap();
            writeln!(out).unwrap();
            writeln!(out, "| Name | Type | Description |").unwrap();
            writeln!(out, "|------|------|-------------|").unwrap();
            for f in &visible_fields {
                let brief = self.get(&f.base.brief).map(|s| s.as_str()).unwrap_or("");
                let anchor = method_anchor(&c.base.name, &f.base.name);
                writeln!(
                    out,
                    "| [{}](#{}) | `{}` | {} |",
                    f.base.name, anchor, f.type_name, brief
                )
                .unwrap();
            }
            writeln!(out).unwrap();
        }
        if !visible_nested.is_empty() {
            writeln!(out, "### Nested Classes").unwrap();
            writeln!(out).unwrap();
            writeln!(out, "| Name | Description |").unwrap();
            writeln!(out, "|------|-------------|").unwrap();
            for n in &visible_nested {
                let brief = self.get(&n.base.brief).map(|s| s.as_str()).unwrap_or("");
                let anchor = class_anchor(&n.base.name);
                writeln!(out, "| [{}](#{}) | {} |", n.base.name, anchor, brief).unwrap();
            }
            writeln!(out).unwrap();
        }
        if !visible_nested_enums.is_empty() {
            writeln!(out, "### Nested Enums").unwrap();
            writeln!(out).unwrap();
            writeln!(out, "| Name | Description |").unwrap();
            writeln!(out, "|------|-------------|").unwrap();
            for e in &visible_nested_enums {
                let brief = self.get(&e.base.brief).map(|s| s.as_str()).unwrap_or("");
                let anchor = class_anchor(&e.base.name);
                writeln!(out, "| [{}](#{}) | {} |", e.base.name, anchor, brief).unwrap();
            }
            writeln!(out).unwrap();
        }
        writeln!(out, "---").unwrap();
        writeln!(out).unwrap();
        // 成员函数详细文档
        for group in &visible_methods {
            out.push_str(&self.generate_overload_group(group, &c.base.name, heading + 1));
        }

        // 成员变量详细文档
        for f in &visible_fields {
            out.push_str(&self.generate_member_field(f, &c.base.name, heading + 1));
        }
        // 嵌套类详细文档（递归，heading +1）
        for n in &visible_nested {
            out.push_str(&self.generate_class(n, heading + 1));
        }
        // 嵌套枚举详细文档
        for e in &visible_nested_enums {
            out.push_str(&self.generate_enum(e, heading + 1));
        }
        out
    }

    fn generate_overload_group(
        &self,
        group: &OverloadGroup,
        class_name: &str,
        heading: usize,
    ) -> String {
        let mut out = String::new();
        let h = "#".repeat(heading);
        let anchor = method_anchor(class_name, &group.name);

        writeln!(out, "{} `{}` {{#{}}}", h, group.name, anchor).unwrap();
        writeln!(out).unwrap();

        for (i, o) in group.overloads.iter().enumerate() {
            if o.doc.base.is_not_public {
                continue;
            }
            if group.overloads.len() > 1 {
                writeln!(out, "#### Overload {}", i + 1).unwrap();
                writeln!(out).unwrap();
            }
            // 声明原型
            let proto = o
                .decl_prototype
                .as_deref()
                .or_else(|| Some(group.name.as_str()));
            if let Some(p) = proto {
                writeln!(out, "```cpp").unwrap();
                writeln!(out, "{}", p).unwrap();
                writeln!(out, "```").unwrap();
                writeln!(out).unwrap();
            }
            // 修饰符 badges
            let mut badges = Vec::new();
            if o.doc.is_const {
                badges.push("`const`");
            }
            if o.doc.is_static {
                badges.push("`static`");
            }
            if o.doc.is_virtual {
                badges.push("`virtual`");
            }
            if o.doc.is_pure_virtual {
                badges.push("`pure virtual`");
            }
            if o.doc.is_override {
                badges.push("`override`");
            }
            if o.doc.is_noexcept {
                badges.push("`noexcept`");
            }
            if o.doc.is_constexpr {
                badges.push("`constexpr`");
            }
            if o.doc.is_deleted {
                badges.push("`= delete`");
            }
            if o.doc.is_defaulted {
                badges.push("`= default`");
            }
            if !badges.is_empty() {
                writeln!(out, "{}", badges.join(" ")).unwrap();
                writeln!(out).unwrap();
            }
            // brief / details
            if let Some(brief) = self.get(&o.doc.base.brief) {
                writeln!(out, "{}", brief).unwrap();
                writeln!(out).unwrap();
            }
            if let Some(desc) = self.get(&o.doc.base.description) {
                writeln!(out, "{}", desc).unwrap();
                writeln!(out).unwrap();
            }
            // 通用注释块
            out.push_str(&self.generate_common_tags(&o.doc.base));
            // 参数表
            let visible_params: Vec<_> =
                o.doc.params.iter().filter(|p| !p.name.is_empty()).collect();
            if !visible_params.is_empty() {
                writeln!(out, "**Parameters**").unwrap();
                writeln!(out).unwrap();
                writeln!(out, "| Name | Type | Direction | Description |").unwrap();
                writeln!(out, "|------|------|-----------|-------------|").unwrap();
                for p in &visible_params {
                    let dir = match p.direction {
                        ParamDirection::In => "in",
                        ParamDirection::Out => "out",
                        ParamDirection::InOut => "in,out",
                    };
                    let desc = self.get(&p.description).map(|s| s.as_str()).unwrap_or("");
                    writeln!(
                        out,
                        "| `{}` | `{}` | {} | {} |",
                        p.name, p.type_name, dir, desc
                    )
                    .unwrap();
                }
                writeln!(out).unwrap();
            }
            // 返回值
            let has_return = self.get(&o.doc.return_doc.description).is_some()
                || !o.doc.return_doc.return_values.is_empty();
            if has_return {
                writeln!(out, "**Returns**").unwrap();
                writeln!(out).unwrap();
                if let Some(ret) = self.get(&o.doc.return_doc.description) {
                    writeln!(out, "{}", ret).unwrap();
                    writeln!(out).unwrap();
                }
                if !o.doc.return_doc.return_values.is_empty() {
                    writeln!(out, "| Value | Description |").unwrap();
                    writeln!(out, "|-------|-------------|").unwrap();
                    for rv in &o.doc.return_doc.return_values {
                        let desc = self.get(&rv.description).map(|s| s.as_str()).unwrap_or("");
                        writeln!(out, "| `{}` | {} |", rv.value, desc).unwrap();
                    }
                    writeln!(out).unwrap();
                }
            }
            // 异常
            if !o.doc.exceptions.is_empty() {
                writeln!(out, "**Throws**").unwrap();
                writeln!(out).unwrap();
                writeln!(out, "| Exception | Description |").unwrap();
                writeln!(out, "|-----------|-------------|").unwrap();
                for ex in &o.doc.exceptions {
                    let desc = self.get(&ex.description).map(|s| s.as_str()).unwrap_or("");
                    writeln!(out, "| `{}` | {} |", ex.exception_type, desc).unwrap();
                }
                writeln!(out).unwrap();
            }
        }
        out
    }

    fn generate_member_field(
        &self,
        f: &MemberFieldDocument,
        class_name: &str,
        heading: usize,
    ) -> String {
        let mut out = String::new();
        let h = "#".repeat(heading);
        let anchor = method_anchor(class_name, &f.base.name);

        writeln!(out, "{} `{}` {{#{}}}", h, f.base.name, anchor).unwrap();
        writeln!(out).unwrap();

        writeln!(out, "```cpp").unwrap();
        let mut decl = String::new();
        if f.is_static {
            decl.push_str("static ");
        }
        if f.is_constexpr {
            decl.push_str("constexpr ");
        }
        if f.is_const {
            decl.push_str("const ");
        }
        if f.is_mutable {
            decl.push_str("mutable ");
        }
        decl.push_str(&f.type_name);
        decl.push(' ');
        decl.push_str(&f.base.name);
        if let Some(val) = &f.default_value {
            decl.push_str(&format!(" = {}", val));
        }
        decl.push(';');
        writeln!(out, "{}", decl).unwrap();
        writeln!(out, "```").unwrap();
        writeln!(out).unwrap();
        if let Some(brief) = self.get(&f.base.brief) {
            writeln!(out, "{}", brief).unwrap();
            writeln!(out).unwrap();
        }
        if let Some(desc) = self.get(&f.base.description) {
            writeln!(out, "{}", desc).unwrap();
            writeln!(out).unwrap();
        }
        out.push_str(&self.generate_common_tags(&f.base));
        out
    }

    pub fn generate_enum(&self, e: &EnumDocument, heading: usize) -> String {
        let mut out = String::new();
        let h = "#".repeat(heading);
        let anchor = class_anchor(&e.base.name);
        writeln!(
            out,
            "{} enum{} `{}` {{#{}}}",
            h,
            if e.is_scoped { " class" } else { "" },
            e.base.name,
            anchor
        )
        .unwrap();
        writeln!(out).unwrap();
        if let Some(ty) = &e.underlying_type {
            writeln!(out, "**Underlying type:** `{}`", ty).unwrap();
            writeln!(out).unwrap();
        }
        if let Some(brief) = self.get(&e.base.brief) {
            writeln!(out, "{}", brief).unwrap();
            writeln!(out).unwrap();
        }
        if let Some(desc) = self.get(&e.base.description) {
            writeln!(out, "{}", desc).unwrap();
            writeln!(out).unwrap();
        }
        out.push_str(&self.generate_common_tags(&e.base));
        // 成员表格
        if !e.variants.is_empty() {
            writeln!(out, "**Enumerators**").unwrap();
            writeln!(out).unwrap();
            writeln!(out, "| Name | Value | Description |").unwrap();
            writeln!(out, "|------|-------|-------------|").unwrap();
            for v in &e.variants {
                let val = v.value.as_deref().unwrap_or("");
                let brief = self.get(&v.brief).map(|s| s.as_str()).unwrap_or("");
                let desc = self.get(&v.description).map(|s| s.as_str()).unwrap_or("");
                let combined = if !brief.is_empty() && !desc.is_empty() {
                    format!("{} {}", brief, desc)
                } else {
                    format!("{}{}", brief, desc)
                };
                writeln!(out, "| `{}` | `{}` | {} |", v.name, val, combined).unwrap();
            }
            writeln!(out).unwrap();
        }

        out
    }

    pub fn generate_variable(&self, v: &VariableDocument, heading: usize) -> String {
        let mut out = String::new();
        let h = "#".repeat(heading);
        let anchor = class_anchor(&v.base.name);

        writeln!(out, "{} `{}` {{#{}}}", h, v.base.name, anchor).unwrap();
        writeln!(out).unwrap();

        // 声明
        writeln!(out, "```cpp").unwrap();
        let mut decl = String::new();
        if v.is_static {
            decl.push_str("static ");
        }
        if v.is_inline {
            decl.push_str("inline ");
        }
        if v.is_constexpr {
            decl.push_str("constexpr ");
        }
        if v.is_const {
            decl.push_str("const ");
        }
        decl.push_str(&v.type_name);
        decl.push(' ');
        decl.push_str(&v.base.name);
        if let Some(val) = &v.default_value {
            decl.push_str(&format!(" = {}", val));
        }
        decl.push(';');
        writeln!(out, "{}", decl).unwrap();
        writeln!(out, "```").unwrap();
        writeln!(out).unwrap();

        // 模板标记
        if v.base.is_main_template {
            writeln!(out, "> **Primary template**").unwrap();
            writeln!(out).unwrap();
        }
        if !v.base.spec_template_args.is_empty() {
            writeln!(
                out,
                "> **Specialization** `<{}>`",
                v.base.spec_template_args.join(", ")
            )
            .unwrap();
            writeln!(out).unwrap();
        }
        if v.base.is_template && !v.base.template_params.is_empty() {
            writeln!(out, "**Template parameters**").unwrap();
            writeln!(out).unwrap();
            writeln!(out, "| Parameter | Description |").unwrap();
            writeln!(out, "|-----------|-------------|").unwrap();
            for tp in &v.base.template_params {
                let desc = self.get(&tp.description).map(|s| s.as_str()).unwrap_or("");
                writeln!(out, "| `{}` | {} |", tp.name, desc).unwrap();
            }
            writeln!(out).unwrap();
        }

        if let Some(brief) = self.get(&v.base.brief) {
            writeln!(out, "{}", brief).unwrap();
            writeln!(out).unwrap();
        }
        if let Some(desc) = self.get(&v.base.description) {
            writeln!(out, "{}", desc).unwrap();
            writeln!(out).unwrap();
        }
        out.push_str(&self.generate_common_tags(&v.base));

        out
    }

    pub fn generate_free_function(&self, f: &FreeFunctionDocument, heading: usize) -> String {
        let mut out = String::new();
        let h = "#".repeat(heading);
        let anchor = class_anchor(&f.base.name);

        writeln!(out, "{} `{}` {{#{}}}", h, f.base.name, anchor).unwrap();
        writeln!(out).unwrap();

        // 修饰符
        let mut badges = Vec::new();
        if f.is_static {
            badges.push("`static`");
        }
        if f.is_inline {
            badges.push("`inline`");
        }
        if f.is_constexpr {
            badges.push("`constexpr`");
        }
        if f.is_noexcept {
            badges.push("`noexcept`");
        }
        if !badges.is_empty() {
            writeln!(out, "{}", badges.join(" ")).unwrap();
            writeln!(out).unwrap();
        }

        if let Some(brief) = self.get(&f.base.brief) {
            writeln!(out, "{}", brief).unwrap();
            writeln!(out).unwrap();
        }
        if let Some(desc) = self.get(&f.base.description) {
            writeln!(out, "{}", desc).unwrap();
            writeln!(out).unwrap();
        }
        out.push_str(&self.generate_common_tags(&f.base));
        // 参数
        let visible_params: Vec<_> = f.params.iter().filter(|p| !p.name.is_empty()).collect();
        if !visible_params.is_empty() {
            writeln!(out, "**Parameters**").unwrap();
            writeln!(out).unwrap();
            writeln!(out, "| Name | Type | Direction | Description |").unwrap();
            writeln!(out, "|------|------|-----------|-------------|").unwrap();
            for p in &visible_params {
                let dir = match p.direction {
                    ParamDirection::In => "in",
                    ParamDirection::Out => "out",
                    ParamDirection::InOut => "in,out",
                };
                let desc = self.get(&p.description).map(|s| s.as_str()).unwrap_or("");
                writeln!(
                    out,
                    "| `{}` | `{}` | {} | {} |",
                    p.name, p.type_name, dir, desc
                )
                .unwrap();
            }
            writeln!(out).unwrap();
        }

        // 返回
        if let Some(ret) = self.get(&f.return_doc.description) {
            writeln!(out, "**Returns:** {}", ret).unwrap();
            writeln!(out).unwrap();
        }

        // 异常
        if !f.exceptions.is_empty() {
            writeln!(out, "**Throws**").unwrap();
            writeln!(out).unwrap();
            writeln!(out, "| Exception | Description |").unwrap();
            writeln!(out, "|-----------|-------------|").unwrap();
            for ex in &f.exceptions {
                let desc = self.get(&ex.description).map(|s| s.as_str()).unwrap_or("");
                writeln!(out, "| `{}` | {} |", ex.exception_type, desc).unwrap();
            }
            writeln!(out).unwrap();
        }
        out
    }

    fn generate_common_tags(&self, base: &BasicDocument) -> String {
        let mut out = String::new();

        if let Some(note) = self.get(&base.note) {
            writeln!(out, "> **Note:** {}", note).unwrap();
            writeln!(out).unwrap();
        }
        if let Some(warn) = self.get(&base.warning) {
            writeln!(out, "> **Warning:** {}", warn).unwrap();
            writeln!(out).unwrap();
        }
        if let Some(attn) = self.get(&base.attention) {
            writeln!(out, "> **Attention:** {}", attn).unwrap();
            writeln!(out).unwrap();
        }
        if let Some(rem) = self.get(&base.remark) {
            writeln!(out, "> **Remark:** {}", rem).unwrap();
            writeln!(out).unwrap();
        }
        if let Some(dep) = self.get(&base.deprecated) {
            writeln!(out, "> **Deprecated:** {}", dep).unwrap();
            writeln!(out).unwrap();
        }
        if let Some(since) = &base.since {
            writeln!(out, "> **Since:** {}", since).unwrap();
            writeln!(out).unwrap();
        }
        if let Some(ver) = &base.version {
            writeln!(out, "> **Version:** {}", ver).unwrap();
            writeln!(out).unwrap();
        }
        if let Some(todos) = self.get_vec(&base.todo) {
            for t in todos {
                writeln!(out, "> **TODO:** {}", t).unwrap();
            }
            if !todos.is_empty() {
                writeln!(out).unwrap();
            }
        }
        if let Some(bugs) = self.get_vec(&base.bug) {
            for b in bugs {
                writeln!(out, "> **Bug:** {}", b).unwrap();
            }
            if !bugs.is_empty() {
                writeln!(out).unwrap();
            }
        }
        if let Some(pres) = self.get_vec(&base.pre) {
            for p in pres {
                writeln!(out, "> **Pre:** {}", p).unwrap();
            }
            if !pres.is_empty() {
                writeln!(out).unwrap();
            }
        }
        if let Some(posts) = self.get_vec(&base.post) {
            for p in posts {
                writeln!(out, "> **Post:** {}", p).unwrap();
            }
            if !posts.is_empty() {
                writeln!(out).unwrap();
            }
        }
        if !base.see_also.is_empty() {
            writeln!(out, "**See also:** {}", base.see_also.join(", ")).unwrap();
            writeln!(out).unwrap();
        }
        if !base.code_examples.is_empty() {
            for ex in &base.code_examples {
                let lang = ex.language.as_deref().unwrap_or("cpp");
                writeln!(out, "```{}", lang).unwrap();
                writeln!(out, "{}", ex.content.trim()).unwrap();
                writeln!(out, "```").unwrap();
                writeln!(out).unwrap();
            }
        }
        for par in &base.par {
            if let Some(title) = self.get(&par.title) {
                writeln!(out, "**{}**", title).unwrap();
                writeln!(out).unwrap();
            }
            if let Some(content) = self.get(&par.content) {
                writeln!(out, "{}", content).unwrap();
                writeln!(out).unwrap();
            }
        }

        out
    }
}

fn class_anchor(name: &str) -> String {
    name.to_lowercase()
        .replace("::", "-")
        .replace('<', "-")
        .replace('>', "")
}

fn method_anchor(class_name: &str, method_name: &str) -> String {
    format!(
        "{}-{}",
        class_anchor(class_name),
        method_name.to_lowercase()
    )
}

fn collect_all_classes(ns: &NamespaceDocument) -> Vec<&ClassDocument> {
    let mut result: Vec<&ClassDocument> = ns.classes.iter().collect();
    for sub in &ns.sub_namespaces {
        result.extend(collect_all_classes(sub));
    }
    result
}

fn collect_all_functions(ns: &NamespaceDocument) -> Vec<&FreeFunctionDocument> {
    let mut result: Vec<&FreeFunctionDocument> = ns.free_functions.iter().collect();
    for sub in &ns.sub_namespaces {
        result.extend(collect_all_functions(sub));
    }
    result
}
