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
#![allow(dead_code)]

use crate::i18n::{I18n, LangTag};
use std::path::PathBuf;

#[derive(Clone)]
pub enum AccessLevel {
    Public,
    Protected,
    Private,
}

#[derive(Clone, Debug)]
pub enum ParamDirection {
    In,
    Out,
    InOut,
}

#[derive(Clone)]
pub struct TemplateParam {
    pub name: String,
    pub constraint: Option<String>,
    pub default_value: Option<String>,
    pub description: I18n<String>, // @tparam
}

#[derive(Clone)]
pub struct Param {
    pub name: String,
    pub type_name: String,
    pub default_value: Option<String>,
    pub description: I18n<String>, // @param
    pub direction: ParamDirection,
}

#[derive(Clone)]
pub struct ExceptionDoc {
    pub exception_type: String,
    pub description: I18n<String>, // @throws
}

#[derive(Clone)]
pub struct BaseClass {
    pub name: String,
    pub access: AccessLevel,
    pub is_virtual: bool,
}

#[derive(Clone)]
pub struct ReturnValue {
    pub value: String,
    pub description: I18n<String>, // @retval
}

#[derive(Clone)]
pub struct ReturnDoc {
    pub type_name: String,
    pub description: I18n<String>, // @return
    pub return_values: Vec<ReturnValue>,
}

#[derive(Clone)]
pub struct CustomParagraph {
    pub title: I18n<String>,
    pub content: I18n<String>, // @par
}

#[derive(Clone)]
pub struct CodeExample {
    pub language: Option<String>,
    pub content: String,
    pub is_snippet: bool,
    pub snippet_file: Option<String>,
    pub snippet_tag: Option<String>,
}

#[derive(Clone)]
pub struct BasicDocument {
    pub name: String,
    pub namespace_stack: Vec<String>,
    pub is_template: bool,
    pub template_params: Vec<TemplateParam>,
    pub since: Option<String>,
    pub version: Option<String>,
    pub date: Option<String>,
    pub authors: Vec<String>,
    pub copyright: Option<String>,
    pub license: Option<String>,
    pub see_also: Vec<String>,
    pub references: Vec<String>,
    pub xref_internal: Vec<String>,
    pub xref_external: Vec<String>,
    pub code_examples: Vec<CodeExample>,

    pub brief: I18n<String>,
    pub description: I18n<String>,
    pub note: I18n<String>,
    pub warning: I18n<String>,
    pub attention: I18n<String>,
    pub remark: I18n<String>,
    pub todo: I18n<Vec<String>>,
    pub bug: I18n<Vec<String>>,
    pub test: I18n<Vec<String>>,
    pub deprecated: I18n<String>,
    pub pre: I18n<Vec<String>>,
    pub post: I18n<Vec<String>>,
    pub invariant: I18n<Vec<String>>,
    pub par: Vec<CustomParagraph>,
    pub is_not_public: bool,
    pub is_hidden: bool,
    /// @show：无论访问级别如何都生成该实体
    pub is_shown: bool,
    pub overload_decl: Option<String>,
    pub is_main_template: bool,           // @main_template
    pub spec_template_args: Vec<String>,  // @spec_template 参数...
    pub module: Option<String>,           // @module

    /// 该实体是否带有文档注释（@notpublic/@hide 的实体不计入未文档化统计）
    pub is_documented: bool,
    /// 实体声明位置，形如 `path:line`
    pub location: Option<String>,
}

pub enum LangContext {
    Default,
    Scoped(LangTag),
}

#[derive(Clone)]
pub struct FreeFunctionDocument {
    pub decl_prototype: Option<String>,
    pub is_static: bool,
    pub is_inline: bool,
    pub is_constexpr: bool,
    pub is_consteval: bool,
    pub is_noexcept: bool,
    pub params: Vec<Param>,
    pub return_doc: ReturnDoc,
    pub exceptions: Vec<ExceptionDoc>,
    pub base: BasicDocument,
}

#[derive(Clone)]
pub struct MemberFunctionDocument {
    pub is_const: bool,
    pub is_noexcept: bool,
    pub is_for_lvalue: bool,
    pub is_for_rvalue: bool,
    pub is_virtual: bool,
    pub is_pure_virtual: bool,
    pub is_override: bool,
    pub is_final: bool,
    pub is_static: bool,
    pub is_explicit: bool,
    pub is_inline: bool,
    pub is_constexpr: bool,
    pub is_consteval: bool,
    pub is_defaulted: bool,
    pub is_deleted: bool,
    pub access: AccessLevel,
    pub params: Vec<Param>,
    pub return_doc: ReturnDoc,
    pub exceptions: Vec<ExceptionDoc>,
    pub base: BasicDocument,
}

#[derive(Clone)]
pub struct MemberFieldDocument {
    pub is_constexpr: bool,
    pub is_static: bool,
    pub is_mutable: bool,
    pub is_const: bool,
    pub access: AccessLevel,
    pub type_name: String,
    pub default_value: Option<String>,
    pub base: BasicDocument,
}

#[derive(Clone)]
pub enum ClassKind {
    Class,
    Struct,
    Union,
}

#[derive(Clone)]
pub struct ClassDocument {
    pub kind: ClassKind,
    pub is_abstract: bool,
    pub is_final: bool,
    pub access: AccessLevel,
    pub base_classes: Vec<BaseClass>,
    pub overload_groups: Vec<OverloadGroup>,
    pub member_fields: Vec<MemberFieldDocument>,
    pub constant_members: Vec<ConstantDocument>,
    pub variable_template_members: Vec<VariableTemplateDocument>,
    pub nested_classes: Vec<ClassDocument>,
    pub nested_enums: Vec<EnumDocument>,
    pub nested_aliases: Vec<TypeAliasDocument>,
    pub friend_classes: Vec<String>,
    pub related_functions: Vec<String>,
    pub base: BasicDocument,
    pub using_members: Vec<String>,
    pub using_doc_overrides: Vec<OverloadDoc>,
}
#[derive(Clone)]
pub struct EnumDocument {
    pub is_scoped: bool,
    pub underlying_type: Option<String>,
    pub access: AccessLevel,
    pub variants: Vec<EnumVariantDocument>,
    pub base: BasicDocument,
}

#[derive(Clone)]
pub struct EnumVariantDocument {
    pub name: String,
    pub value: Option<String>,
    pub brief: I18n<String>,
    pub description: I18n<String>,
    pub is_documented: bool,
    pub location: Option<String>,
}

#[derive(Clone)]
pub enum TypeAliasKind {
    Using,
    Typedef,
}

#[derive(Clone)]
pub struct TypeAliasDocument {
    pub kind: TypeAliasKind,
    pub target_type: String,
    pub access: AccessLevel,
    pub base: BasicDocument,
}

#[derive(Clone)]
pub struct OverloadDoc {
    pub decl_prototype: Option<String>,
    pub doc: MemberFunctionDocument,
}

#[derive(Clone)]
pub struct OverloadGroup {
    pub name: String,
    pub overloads: Vec<OverloadDoc>,
}

#[derive(Clone)]
pub struct FreeFunctionOverloadGroup {
    pub name: String,
    pub overloads: Vec<FreeFunctionDocument>,
}

/// A constant variable (const, constexpr, or constinit at file/namespace/class scope)
#[derive(Clone)]
pub struct ConstantDocument {
    pub is_constexpr: bool,
    pub is_constinit: bool,
    pub is_static: bool,
    pub is_inline: bool,
    pub type_name: String,
    pub default_value: Option<String>,
    pub access: AccessLevel,
    pub base: BasicDocument,
}

/// A variable template (e.g., `template<typename T> constexpr T pi = T(3.14);`)
#[derive(Clone)]
pub struct VariableTemplateDocument {
    pub is_constexpr: bool,
    pub is_const: bool,
    pub is_static: bool,
    pub is_inline: bool,
    pub type_name: String,
    pub default_value: Option<String>,
    pub access: AccessLevel,
    pub base: BasicDocument,
}

pub struct VariableDocument {
    pub is_constexpr: bool,
    pub is_constinit: bool,
    pub is_const: bool,
    pub is_static: bool,
    pub is_inline: bool,
    pub type_name: String,
    pub default_value: Option<String>,
    pub base: BasicDocument,
}

pub struct MacroDocument {
    pub is_function_like: bool,
    pub params: Vec<String>,
    pub expansion: Option<String>,
    pub base: BasicDocument,
}

pub struct ConceptDocument {
    pub params: Vec<TemplateParam>,
    pub constraint_expression: String,
    pub base: BasicDocument,
}

pub struct NamespaceDocument {
    pub is_inline: bool,
    pub free_function_overloads: Vec<FreeFunctionOverloadGroup>,
    pub variables: Vec<VariableDocument>,
    pub constants: Vec<ConstantDocument>,
    pub variable_templates: Vec<VariableTemplateDocument>,
    pub classes: Vec<ClassDocument>,
    pub enums: Vec<EnumDocument>,
    pub aliases: Vec<TypeAliasDocument>,
    pub concepts: Vec<ConceptDocument>,
    pub macros: Vec<MacroDocument>,
    pub sub_namespaces: Vec<NamespaceDocument>,
    pub base: BasicDocument,
}

pub struct FileDocument {
    pub path: PathBuf,
    pub title: String,
    pub file_path: String,
    pub source_name: String,
    pub rel_path: PathBuf,
    pub includes: Vec<String>,
    pub brief: I18n<String>,
    pub description: I18n<String>,
    pub authors: Vec<String>,
    pub date: Option<String>,
    pub version: Option<String>,
    pub copyright: Option<String>,
    pub license: Option<String>,
    pub namespaces: Vec<NamespaceDocument>,
    pub free_function_overloads: Vec<FreeFunctionOverloadGroup>,
    pub variables: Vec<VariableDocument>,
    pub constants: Vec<ConstantDocument>,
    pub variable_templates: Vec<VariableTemplateDocument>,
    pub classes: Vec<ClassDocument>,
    pub enums: Vec<EnumDocument>,
    pub aliases: Vec<TypeAliasDocument>,
    pub concepts: Vec<ConceptDocument>,
    pub macros: Vec<MacroDocument>,
    pub module: Option<String>,
    pub body: String,
    /// 伞头：不解析实体，页面上只列出 `#include` 的头文件名
    pub is_umbrella: bool,
}

impl FileDocument {
    /// 该文件能否产出至少一个文档条目。
    pub fn has_visible_entities(&self) -> bool {
        self.visible_entity_count() > 0
    }

    /// 会出现在文档中的实体数量（与 markdown 生成器的过滤口径一致）。
    pub fn visible_entity_count(&self) -> usize {
        fn count_ns(ns: &NamespaceDocument) -> usize {
            ns.classes
                .iter()
                .filter(|c| !c.base.is_hidden && !c.base.is_not_public)
                .count()
                + ns.enums.iter().filter(|e| !e.base.is_not_public).count()
                + ns
                    .variables
                    .iter()
                    .filter(|v| !v.base.is_not_public)
                    .count()
                + ns
                    .constants
                    .iter()
                    .filter(|c| !c.base.is_not_public)
                    .count()
                + ns
                    .variable_templates
                    .iter()
                    .filter(|v| !v.base.is_not_public)
                    .count()
                + ns
                    .free_function_overloads
                    .iter()
                    .filter(|g| {
                        !g.overloads.is_empty()
                            && !g
                                .overloads
                                .iter()
                                .all(|f| f.base.is_hidden || f.base.is_not_public)
                    })
                    .count()
                + ns
                    .macros
                    .iter()
                    .filter(|m| !m.base.is_hidden && !m.base.is_not_public)
                    .count()
                + ns
                    .aliases
                    .iter()
                    .filter(|a| !a.base.is_not_public)
                    .count()
                + ns
                    .concepts
                    .iter()
                    .filter(|c| !c.base.is_not_public)
                    .count()
                + ns.sub_namespaces.iter().map(count_ns).sum::<usize>()
        }
        self.classes
            .iter()
            .filter(|c| !c.base.is_hidden && !c.base.is_not_public)
            .count()
            + self.enums.iter().filter(|e| !e.base.is_not_public).count()
            + self
                .variables
                .iter()
                .filter(|v| !v.base.is_not_public)
                .count()
            + self
                .constants
                .iter()
                .filter(|c| !c.base.is_not_public)
                .count()
            + self
                .variable_templates
                .iter()
                .filter(|v| !v.base.is_not_public)
                .count()
            + self
                .free_function_overloads
                .iter()
                .filter(|g| {
                    !g.overloads.is_empty()
                        && !g
                            .overloads
                            .iter()
                            .all(|f| f.base.is_hidden || f.base.is_not_public)
                })
                .count()
            + self
                .macros
                .iter()
                .filter(|m| !m.base.is_hidden && !m.base.is_not_public)
                .count()
            + self
                .aliases
                .iter()
                .filter(|a| !a.base.is_not_public)
                .count()
            + self
                .concepts
                .iter()
                .filter(|c| !c.base.is_not_public)
                .count()
            + self.namespaces.iter().map(count_ns).sum::<usize>()
    }

    /// 解析到的实体数量，无论是否可见。
    pub fn entity_count(&self) -> usize {
        fn count_ns(ns: &NamespaceDocument) -> usize {
            ns.classes.len()
                + ns.enums.len()
                + ns.variables.len()
                + ns.constants.len()
                + ns.variable_templates.len()
                + ns.free_function_overloads.len()
                + ns.macros.len()
                + ns.aliases.len()
                + ns.concepts.len()
                + ns.sub_namespaces.iter().map(count_ns).sum::<usize>()
        }
        self.classes.len()
            + self.enums.len()
            + self.variables.len()
            + self.constants.len()
            + self.variable_templates.len()
            + self.free_function_overloads.len()
            + self.macros.len()
            + self.aliases.len()
            + self.concepts.len()
            + self.namespaces.iter().map(count_ns).sum::<usize>()
    }
}