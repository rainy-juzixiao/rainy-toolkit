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
use crate::data::document::{
    BasicDocument, CodeExample, CustomParagraph, ExceptionDoc, Param, ParamDirection, ReturnDoc,
    ReturnValue,
};
use crate::i18n::{I18n, LangTag};
use std::collections::HashMap;

pub fn extract_raw_comment(entity: &clang::Entity) -> Option<String> {
    let result = entity.get_comment();
    // 优先 libclang
    if let Some(comment) = entity.get_comment() {
        if !comment.trim().is_empty() {
            return Some(comment);
        }
    }
    if let Some(loc) = entity.get_location() {
        let file_loc = loc.get_file_location();
        if let Some(file_obj) = file_loc.file {
            let path = file_obj.get_path();
            if let Some(path_str) = path.to_str() {
                return crate::utils::comment::find_comment(path_str, file_loc.line as usize);
            }
        }
    }
    None
}

#[derive(Debug)]
enum CommentToken {
    LangSwitch(String),                                       // \lang <tag>
    Tag(String, String),                                      // @brief foo / @param[in] x desc
    TagWithArg(String, String, String), // @param[in] name desc / @tparam T desc
    TagWithDirection(String, ParamDirection, String, String), // @param[in/out] name desc
    RetVal(String, String),             // @retval value desc
    Throws(String, String),             // @throws Type desc
    See(String),                        // @see / @sa
    Ref(String),                        // @ref
    Link(String),                       // @link
    Cite(String),                       // @cite
    Author(String),                     // @author
    Since(String),                      // @since
    Version(String),                    // @version
    Date(String),                       // @date
    Copyright(String),                  // @copyright
    CodeBlock(Option<String>, String, bool, Option<String>, Option<String>),
    Par(String, String),  // @par title content
    Plain(String),        // 纯文本（属于 @details）
    NotPublic,            // @notpublic
    Hide,                 // @hide
    OverloadDecl(String), // @overload_decl
    MainTemplate,
    SpecTemplate(Vec<String>),
}

fn tokenize_comment(raw: &str) -> Vec<CommentToken> {
    let lines = strip_comment_markers(raw);
    let mut tokens = Vec::new();
    let mut i = 0;
    let mut has_brief = false;

    'outer: while i < lines.len() {
        let line = lines[i].trim();

        if strip_tag(line, "notpublic").is_some() {
            tokens.push(CommentToken::NotPublic);
            i += 1;
            continue;
        }
        if strip_tag(line, "hide").is_some() {
            tokens.push(CommentToken::Hide);
            i += 1;
            continue;
        }
        if strip_tag(line, "main_template").is_some() {
            tokens.push(CommentToken::MainTemplate);
            i += 1;
            continue;
        }
        if let Some(rest) = strip_tag(line, "spec_template") {
            let args = rest
                .split(',')
                .map(|s| s.trim().to_string())
                .filter(|s| !s.is_empty())
                .collect();
            tokens.push(CommentToken::SpecTemplate(args));
            i += 1;
            continue;
        }
        // tokenize_comment 里替换 @overload_decl 的处理
        if let Some(rest) = strip_tag(line, "overload_decl") {
            // 单行形式：@overload_decl void impl1()
            if !rest.trim().is_empty() {
                tokens.push(CommentToken::OverloadDecl(rest.trim().to_string()));
                i += 1;
                continue;
            }

            // 多行形式：下一行是 ```
            i += 1;
            let mut proto = String::new();
            if i < lines.len() && lines[i].trim() == "```" {
                i += 1;
                while i < lines.len() && lines[i].trim() != "```" {
                    proto.push_str(lines[i].trim());
                    proto.push('\n');
                    i += 1;
                }
                i += 1; // skip 结尾 ```
            }
            tokens.push(CommentToken::OverloadDecl(proto.trim().to_string()));
            continue;
        }
        // \lang <tag>
        if let Some(rest) = line.strip_prefix("\\lang") {
            tokens.push(CommentToken::LangSwitch(rest.trim().to_string()));
            // \lang 切换语言后，brief 状态重置，允许新语言块有自己的隐式 brief
            has_brief = false;
            i += 1;
            continue;
        }

        // @code / @code{.cpp}
        if line.starts_with("@code") || line.starts_with("\\code") {
            let lang = extract_code_lang(line);
            let mut content = String::new();
            i += 1;
            while i < lines.len()
                && !lines[i].trim().starts_with("@endcode")
                && !lines[i].trim().starts_with("\\endcode")
            {
                content.push_str(&lines[i]);
                content.push('\n');
                i += 1;
            }
            i += 1; // skip @endcode
            tokens.push(CommentToken::CodeBlock(lang, content, false, None, None));
            continue;
        }

        // @snippet file tag
        if let Some(rest) = strip_tag(line, "snippet") {
            let mut parts = rest.splitn(2, char::is_whitespace);
            let file = parts.next().unwrap_or("").to_string();
            let tag = parts.next().unwrap_or("").trim().to_string();
            tokens.push(CommentToken::CodeBlock(
                None,
                String::new(),
                true,
                Some(file),
                Some(tag),
            ));
            i += 1;
            continue;
        }

        // @param[direction] name desc
        if let Some(rest) = strip_tag(line, "param") {
            let (dir, rest2) = parse_param_direction(rest);
            let (name, desc) = split_first_word(rest2);
            tokens.push(CommentToken::TagWithDirection(
                "param".into(),
                dir,
                name,
                desc,
            ));
            i += 1;
            continue;
        }

        // @tparam name desc
        if let Some(rest) = strip_tag(line, "tparam") {
            let (name, desc) = split_first_word(rest);
            tokens.push(CommentToken::TagWithArg("tparam".into(), name, desc));
            i += 1;
            continue;
        }

        // @retval value desc
        if let Some(rest) = strip_tag(line, "retval") {
            let (val, desc) = split_first_word(rest);
            tokens.push(CommentToken::RetVal(val, desc));
            i += 1;
            continue;
        }

        // @throws / @throw / @exception
        if let Some(rest) = strip_tag(line, "throws")
            .or_else(|| strip_tag(line, "throw"))
            .or_else(|| strip_tag(line, "exception"))
        {
            let (ty, desc) = split_first_word(rest);
            tokens.push(CommentToken::Throws(ty, desc));
            i += 1;
            continue;
        }

        // @par title (multiline content until next tag)
        if let Some(rest) = strip_tag(line, "par") {
            let title = rest.trim().to_string();
            let mut content = String::new();
            i += 1;
            while i < lines.len() && !is_tag_line(lines[i].trim()) {
                content.push_str(lines[i].trim());
                content.push('\n');
                i += 1;
            }
            tokens.push(CommentToken::Par(title, content.trim().to_string()));
            continue;
        }

        // 单参数 tags
        macro_rules! single_tag {
            ($tag:literal, $variant:ident) => {
                if let Some(rest) = strip_tag(line, $tag) {
                    tokens.push(CommentToken::$variant(rest.trim().to_string()));
                    i += 1;
                    continue;
                }
            };
        }

        single_tag!("see", See);
        single_tag!("sa", See);
        single_tag!("ref", Ref);
        single_tag!("link", Link);
        single_tag!("cite", Cite);
        single_tag!("author", Author);
        single_tag!("since", Since);
        single_tag!("version", Version);
        single_tag!("date", Date);
        single_tag!("copyright", Copyright);

        // 多行 tags
        for tag_name in &[
            "brief",
            "details",
            "note",
            "warning",
            "attention",
            "remark",
            "deprecated",
            "todo",
            "bug",
            "test",
            "pre",
            "post",
            "invariant",
            "return",
            "returns",
        ] {
            if let Some(rest) = strip_tag(line, tag_name) {
                let mut content = rest.trim().to_string();
                i += 1;
                while i < lines.len() && !is_tag_line(lines[i].trim()) {
                    content.push(' ');
                    content.push_str(lines[i].trim());
                    i += 1;
                }
                if *tag_name == "brief" {
                    has_brief = true;
                }
                tokens.push(CommentToken::Tag(tag_name.to_string(), content));
                continue 'outer;
            }
        }

        // 纯文本：无 tag 的首行作为隐式 brief
        if !line.is_empty() {
            if !has_brief && !is_tag_line(line) {
                tokens.push(CommentToken::Tag("brief".to_string(), line.to_string()));
                has_brief = true;
            } else {
                tokens.push(CommentToken::Plain(line.to_string()));
            }
        }
        i += 1;
    }

    tokens
}

pub enum LangContext {
    Default,
    Scoped(LangTag),
}

pub struct CommentBlockParser {
    current_lang: LangContext,
    brief_buf: Option<String>,
    description_buf: Option<String>,
    note_buf: Option<String>,
    warning_buf: Option<String>,
    attention_buf: Option<String>,
    remark_buf: Option<String>,
    deprecated_buf: Option<String>,
    todo_buf: Vec<String>,
    bug_buf: Vec<String>,
    test_buf: Vec<String>,
    pre_buf: Vec<String>,
    post_buf: Vec<String>,
    invariant_buf: Vec<String>,
}

impl CommentBlockParser {
    pub fn new() -> Self {
        Self {
            current_lang: LangContext::Default,
            brief_buf: None,
            description_buf: None,
            note_buf: None,
            warning_buf: None,
            attention_buf: None,
            remark_buf: None,
            deprecated_buf: None,
            todo_buf: vec![],
            bug_buf: vec![],
            test_buf: vec![],
            pre_buf: vec![],
            post_buf: vec![],
            invariant_buf: vec![],
        }
    }

    fn flush(&mut self, target: &mut BasicDocument) {
        let lang = match &self.current_lang {
            LangContext::Default => None,
            LangContext::Scoped(tag) => Some(tag.clone()),
        };

        macro_rules! flush_opt {
            ($buf:expr, $field:expr) => {
                if let Some(val) = $buf.take() {
                    append_i18n(&mut $field, lang.clone(), val);
                }
            };
        }
        macro_rules! flush_vec {
            ($buf:expr, $field:expr) => {
                if !$buf.is_empty() {
                    let v = std::mem::take(&mut $buf);
                    match &lang {
                        None => {
                            $field.default.get_or_insert_with(Vec::new).extend(v);
                        }
                        Some(t) => {
                            $field
                                .translations
                                .entry(t.clone())
                                .or_insert_with(Vec::new)
                                .extend(v);
                        }
                    }
                }
            };
        }

        flush_opt!(self.brief_buf, target.brief);
        flush_opt!(self.description_buf, target.description);
        flush_opt!(self.note_buf, target.note);
        flush_opt!(self.warning_buf, target.warning);
        flush_opt!(self.attention_buf, target.attention);
        flush_opt!(self.remark_buf, target.remark);
        flush_opt!(self.deprecated_buf, target.deprecated);
        flush_vec!(self.todo_buf, target.todo);
        flush_vec!(self.bug_buf, target.bug);
        flush_vec!(self.test_buf, target.test);
        flush_vec!(self.pre_buf, target.pre);
        flush_vec!(self.post_buf, target.post);
        flush_vec!(self.invariant_buf, target.invariant);
    }
}

pub struct ParsedComment {
    pub basic: BasicDocument,
    pub params: Vec<Param>,
    pub tparams_desc: HashMap<String, I18n<String>>, // name → desc，合并到 template_params
    pub return_doc: ReturnDoc,
    pub exceptions: Vec<ExceptionDoc>,
    pub overload_decl: Option<String>,
}

fn get_current_lang(parser: &CommentBlockParser) -> Option<LangTag> {
    match &parser.current_lang {
        LangContext::Default => None,
        LangContext::Scoped(t) => Some(t.clone()),
    }
}

pub fn parse_comment(raw: &str, name: &str, namespace_stack: Vec<String>) -> ParsedComment {
    let tokens = tokenize_comment(raw);
    let mut basic = BasicDocument {
        name: name.to_string(),
        namespace_stack,
        is_template: false,
        template_params: vec![],
        since: None,
        version: None,
        date: None,
        authors: vec![],
        copyright: None,
        license: None,
        see_also: vec![],
        references: vec![],
        xref_internal: vec![],
        xref_external: vec![],
        code_examples: vec![],
        brief: I18n::new(),
        description: I18n::new(),
        note: I18n::new(),
        warning: I18n::new(),
        attention: I18n::new(),
        remark: I18n::new(),
        todo: I18n::new(),
        bug: I18n::new(),
        test: I18n::new(),
        deprecated: I18n::new(),
        pre: I18n::new(),
        post: I18n::new(),
        invariant: I18n::new(),
        par: vec![],
        is_not_public: false,
        is_hidden: false,
        overload_decl: None,
        is_main_template: false,
        spec_template_args: vec![],
    };
    let mut parser = CommentBlockParser::new();
    let mut params: HashMap<String, Param> = HashMap::new();
    let mut tparams_desc: HashMap<String, I18n<String>> = HashMap::new();
    let mut return_doc = ReturnDoc {
        type_name: String::new(),
        description: I18n::new(),
        return_values: vec![],
    };
    let mut overload_decl: Option<String> = None;
    let mut exceptions: Vec<ExceptionDoc> = vec![];
    let mut seen_langs: HashMap<String, usize> = HashMap::new(); // lang → 出现次数
    for token in tokens {
        match token {
            CommentToken::LangSwitch(tag) => {
                parser.flush(&mut basic);
                // 计数并警告
                let count = seen_langs.entry(tag.clone()).or_insert(0);
                *count += 1;
                if *count >= 2 {
                    eprintln!(
                        "[sleepy warn] {}: 检测到 {} 个或以上的 \\lang {} 语言块文档，可能会被覆盖",
                        name, *count, tag
                    );
                }
                parser.current_lang = LangContext::Scoped(LangTag(tag));
            }

            CommentToken::Tag(tag, content) => {
                match tag.as_str() {
                    "brief" => {
                        // 多个 @brief 合并，用换行连接
                        match &parser.current_lang {
                            LangContext::Default | LangContext::Scoped(_) => {
                                if let Some(existing) = &mut parser.brief_buf {
                                    existing.push('\n');
                                    existing.push_str(&content);
                                } else {
                                    parser.brief_buf = Some(content);
                                }
                            }
                        }
                    }
                    "details" => {
                        if let Some(existing) = &mut parser.description_buf {
                            existing.push('\n');
                            existing.push_str(&content);
                        } else {
                            parser.description_buf = Some(content);
                        }
                    }
                    // note / warning / attention / remark / deprecated 同理
                    "note" => append_opt(&mut parser.note_buf, content),
                    "warning" => append_opt(&mut parser.warning_buf, content),
                    "attention" => append_opt(&mut parser.attention_buf, content),
                    "remark" => append_opt(&mut parser.remark_buf, content),
                    "deprecated" => append_opt(&mut parser.deprecated_buf, content),
                    // Vec 类型直接 push
                    "todo" => parser.todo_buf.push(content),
                    "bug" => parser.bug_buf.push(content),
                    "test" => parser.test_buf.push(content),
                    "pre" => parser.pre_buf.push(content),
                    "post" => parser.post_buf.push(content),
                    "invariant" => parser.invariant_buf.push(content),
                    "return" | "returns" => {
                        let lang = get_current_lang(&parser);
                        append_i18n(&mut return_doc.description, lang, content);
                    }
                    _ => {}
                }
            }

            CommentToken::Plain(text) => {
                // 纯文本追加到 description
                let buf = parser.description_buf.get_or_insert_with(String::new);
                if !buf.is_empty() {
                    buf.push(' ');
                }
                buf.push_str(&text);
            }

            CommentToken::TagWithDirection(_, dir, name, desc) => {
                // @param
                let lang = get_current_lang(&parser);
                let entry = params.entry(name.clone()).or_insert_with(|| Param {
                    name: name.clone(),
                    type_name: String::new(),
                    default_value: None,
                    description: I18n::new(),
                    direction: dir.clone(),
                });
                entry.direction = dir;
                match lang {
                    None => entry.description.default = Some(desc),
                    Some(t) => {
                        entry.description.translations.insert(t, desc);
                    }
                }
            }

            CommentToken::TagWithArg(tag, arg, desc) => {
                // @tparam
                if tag == "tparam" {
                    let lang = get_current_lang(&parser);

                    let entry = tparams_desc.entry(arg).or_insert_with(I18n::new);
                    match lang {
                        None => entry.default = Some(desc),
                        Some(t) => {
                            entry.translations.insert(t, desc);
                        }
                    }
                }
            }

            CommentToken::RetVal(value, desc) => {
                let lang = get_current_lang(&parser);

                // 找已有的 ReturnValue 或新建
                if let Some(rv) = return_doc
                    .return_values
                    .iter_mut()
                    .find(|r| r.value == value)
                {
                    match lang {
                        None => rv.description.default = Some(desc),
                        Some(t) => {
                            rv.description.translations.insert(t, desc);
                        }
                    }
                } else {
                    let mut rv = ReturnValue {
                        value,
                        description: I18n::new(),
                    };
                    match lang {
                        None => rv.description.default = Some(desc),
                        Some(t) => {
                            rv.description.translations.insert(t, desc);
                        }
                    }
                    return_doc.return_values.push(rv);
                }
            }

            CommentToken::Throws(ty, desc) => {
                let lang = get_current_lang(&parser);

                if let Some(ex) = exceptions.iter_mut().find(|e| e.exception_type == ty) {
                    match lang {
                        None => ex.description.default = Some(desc),
                        Some(t) => {
                            ex.description.translations.insert(t, desc);
                        }
                    }
                } else {
                    let mut ex = ExceptionDoc {
                        exception_type: ty,
                        description: I18n::new(),
                    };
                    match lang {
                        None => ex.description.default = Some(desc),
                        Some(t) => {
                            ex.description.translations.insert(t, desc);
                        }
                    }
                    exceptions.push(ex);
                }
            }

            CommentToken::See(s) => basic.see_also.push(s),
            CommentToken::Ref(s) => basic.xref_internal.push(s),
            CommentToken::Link(s) => basic.xref_external.push(s),
            CommentToken::Cite(s) => basic.references.push(s),
            CommentToken::Author(s) => basic.authors.push(s),
            CommentToken::Since(s) => basic.since = Some(s),
            CommentToken::Version(s) => basic.version = Some(s),
            CommentToken::Date(s) => basic.date = Some(s),
            CommentToken::Copyright(s) => basic.copyright = Some(s),
            CommentToken::CodeBlock(lang, content, is_snippet, sf, st) => {
                basic.code_examples.push(CodeExample {
                    language: lang,
                    content,
                    is_snippet,
                    snippet_file: sf,
                    snippet_tag: st,
                });
            }
            CommentToken::Par(title, content) => {
                let lang = get_current_lang(&parser);
                // 找同名 par 或新建
                if let Some(p) = basic
                    .par
                    .iter_mut()
                    .find(|p| p.title.default.as_deref() == Some(&title))
                {
                    match lang {
                        None => p.content.default = Some(content),
                        Some(t) => {
                            p.content.translations.insert(t, content);
                        }
                    }
                } else {
                    let mut p = CustomParagraph {
                        title: I18n::new(),
                        content: I18n::new(),
                    };
                    match lang {
                        None => {
                            p.title.default = Some(title);
                            p.content.default = Some(content);
                        }
                        Some(t) => {
                            p.title.translations.insert(t.clone(), title);
                            p.content.translations.insert(t, content);
                        }
                    }
                    basic.par.push(p);
                }
            }
            CommentToken::NotPublic => {
                basic.is_not_public = true;
            }
            CommentToken::Hide => {
                basic.is_hidden = true;
            }
            CommentToken::OverloadDecl(proto) => {
                overload_decl = Some(proto);
            }
            CommentToken::MainTemplate => {
                basic.is_main_template = true;
            }
            CommentToken::SpecTemplate(args) => {
                basic.spec_template_args = args;
            }
        }
    }
    parser.flush(&mut basic);
    basic.overload_decl = overload_decl.clone();
    ParsedComment {
        basic,
        params: params.into_values().collect(),
        tparams_desc,
        return_doc,
        exceptions,
        overload_decl,
    }
}

fn strip_comment_markers(raw: &str) -> Vec<String> {
    let mut lines = Vec::new();
    for line in raw.lines() {
        let l = line.trim();
        let l = l.strip_prefix("/**").unwrap_or(l);
        let l = l.strip_prefix("/*!").unwrap_or(l);
        let l = l.strip_prefix("*/").unwrap_or(l);
        let l = l.strip_prefix("//!").unwrap_or(l);
        let l = l.strip_prefix("///").unwrap_or(l);
        let l = if let Some(s) = l.strip_prefix("* ") {
            s
        } else {
            l.strip_prefix('*').unwrap_or(l)
        };
        lines.push(l.to_string());
    }
    lines
}

fn strip_tag<'a>(line: &'a str, tag: &str) -> Option<&'a str> {
    let at = format!("@{}", tag);
    let bsl = format!("\\{}", tag);
    if let Some(r) = line.strip_prefix(at.as_str()) {
        return Some(r.trim_start_matches(char::is_whitespace));
    }
    if let Some(r) = line.strip_prefix(bsl.as_str()) {
        return Some(r.trim_start_matches(char::is_whitespace));
    }
    None
}

fn is_tag_line(line: &str) -> bool {
    line.starts_with('@') || line.starts_with('\\')
}

fn extract_code_lang(line: &str) -> Option<String> {
    // @code{.cpp} → Some("cpp")
    if let Some(inner) = line.split('{').nth(1) {
        let inner = inner.trim_end_matches('}').trim_start_matches('.');
        if !inner.is_empty() {
            return Some(inner.to_string());
        }
    }
    None
}

fn parse_param_direction(s: &str) -> (ParamDirection, &str) {
    if let Some(rest) = s
        .strip_prefix("[in,out]")
        .or_else(|| s.strip_prefix("[inout]"))
    {
        return (ParamDirection::InOut, rest.trim());
    }
    if let Some(rest) = s.strip_prefix("[out]") {
        return (ParamDirection::Out, rest.trim());
    }
    if let Some(rest) = s.strip_prefix("[in]") {
        return (ParamDirection::In, rest.trim());
    }
    (ParamDirection::In, s.trim()) // 默认 In
}

fn split_first_word(s: &str) -> (String, String) {
    let s = s.trim();
    if let Some(pos) = s.find(char::is_whitespace) {
        (s[..pos].to_string(), s[pos..].trim().to_string())
    } else {
        (s.to_string(), String::new())
    }
}

fn append_opt(buf: &mut Option<String>, content: String) {
    if let Some(existing) = buf {
        existing.push('\n');
        existing.push_str(&content);
    } else {
        *buf = Some(content);
    }
}

fn append_i18n(i18n: &mut I18n<String>, lang: Option<LangTag>, content: String) {
    match lang {
        None => append_opt(&mut i18n.default, content),
        Some(t) => {
            i18n.translations
                .entry(t)
                .and_modify(|v| {
                    v.push('\n');
                    v.push_str(&content);
                })
                .or_insert(content);
        }
    }
}
