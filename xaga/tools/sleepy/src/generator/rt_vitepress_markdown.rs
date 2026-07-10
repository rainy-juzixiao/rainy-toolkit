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

//! VitePress reference site generator.
//!
//! Reads a collection of [`FileDocument`]s, groups them by the `@module` tag,
//! and produces a complete VitePress site with:
//!
//! - `docs/.vitepress/config.mts` — theme config with sidebar navigation
//! - `docs/index.md` — landing page listing all modules
//! - `docs/reference/*.md` — per-file reference pages
//!
//! ## Modes
//!
//! | Mode | Output | Use case |
//! |------|--------|----------|
//! | [`generate_site`] | Full site (config + index + reference) | Standalone doc site |
//! | [`generate_reference_only`] | Just `reference/*.md` pages | Embed into existing site |
//!
//! Usage (after collecting all parsed documents):
//!
//! ```ignore
//! let gen = VitePressMarkdownGenerator::new("english");
//! gen.generate_site(&all_docs, &output_dir)?;
//! // or, partial mode:
//! gen.generate_reference_only(&all_docs, &ref_output_dir)?;
//! // then run: npx vitepress build docs
//! ```

use crate::data::document::*;
use crate::generator::markdown::MarkdownGenerator;
use crate::i18n::{I18n, LangTag};
use std::fmt::Write;
use std::path::Path;
use indexmap::IndexMap;

pub struct VitePressMarkdownGenerator {
    pub lang: LangTag,
}

impl VitePressMarkdownGenerator {
    pub fn new(lang: impl Into<String>) -> Self {
        Self {
            lang: LangTag(lang.into()),
        }
    }

    fn get<'a>(&self, i18n: &'a I18n<String>) -> Option<&'a String> {
        i18n
            .get(Some(&self.lang))
            .or_else(|| i18n.translations.values().next())
            .or_else(|| i18n.default.as_ref())
    }

        /// Group documents by their `@module` tag.
    ///
    /// Files without `@module` (or with an empty one) go into "Uncategorized".
    /// Within each group files are sorted by their display name for stable output.
    fn group_by_module<'a>(
        &self,
        docs: &'a [FileDocument],
    ) -> IndexMap<String, Vec<&'a FileDocument>> {
        let mut modules: IndexMap<String, Vec<&FileDocument>> = IndexMap::new();

        for doc in docs {
            let module_name = doc
                .module
                .as_deref()
                .filter(|m| !m.is_empty())
                .unwrap_or("Uncategorized");
            modules
                .entry(module_name.to_string())
                .or_default()
                .push(doc);
        }

        for (_name, file_docs) in modules.iter_mut() {
            file_docs.sort_by(|a, b| {
                let a_stem = file_stem(&a.file_path).unwrap_or("");
                let b_stem = file_stem(&b.file_path).unwrap_or("");
                a_stem.cmp(b_stem)
            });
        }

        modules
    }

    /// Write the per-file reference markdown pages into `ref_dir`.
    ///
    /// Reused by both [`generate_site`] and [`generate_reference_only`].
    fn write_reference_pages(&self, docs: &[FileDocument], ref_dir: &Path) -> anyhow::Result<()> {
        std::fs::create_dir_all(ref_dir)?;
        let md_gen = MarkdownGenerator::new(self.lang.0.clone());

        for doc in docs {
            let stem = file_stem(&doc.file_path).unwrap_or("unknown");
            let page = self.wrap_with_frontmatter(&md_gen, doc, stem);
            std::fs::write(ref_dir.join(format!("{}.md", stem)), &page)?;
        }

        Ok(())
    }

    /// Generate the entire VitePress site under `output_dir`.
    ///
    /// The resulting directory structure:
    ///
    /// ```text
    /// output_dir/
    /// └── docs/
    ///     ├── .vitepress/
    ///     │   └── config.mts    — theme config with sidebar
    ///     ├── index.md           — landing page
    ///     └── reference/
    ///         ├── header1.md
    ///         └── header2.md
    /// ```
    ///
    /// Build the site with: `npx vitepress build docs` (run from `output_dir`).
    pub fn generate_site(
        &self,
        docs: &[FileDocument],
        output_dir: &Path,
    ) -> anyhow::Result<()> {
        let modules = self.group_by_module(docs);

        let docs_dir = output_dir.join("docs");
        let vitepress_dir = docs_dir.join(".vitepress");
        let ref_dir = docs_dir.join("reference");

        std::fs::create_dir_all(&vitepress_dir)?;

        let config_mts = self.generate_config_mts(&modules);
        std::fs::write(vitepress_dir.join("config.mts"), &config_mts)?;

        let index_md = self.generate_index_md(&modules);
        std::fs::write(docs_dir.join("index.md"), &index_md)?;

        self.write_reference_pages(docs, &ref_dir)?;

        Ok(())
    }

    /// Generate only the reference markdown pages, **without** `config.mts` or `index.md`.
    ///
    /// This is for **partial** usage — when the user already has an existing VitePress
    /// documentation site and only wants to drop the C++ reference pages into it.
    ///
    /// ```text
    /// output_ref_dir/
    /// ├── header1.md
    /// └── header2.md
    /// ```
    ///
    /// These pages include frontmatter (`title`, `description`, `category`) so the
    /// host site can reference them via its own sidebar config.
    pub fn generate_reference_only(
        &self,
        docs: &[FileDocument],
        output_ref_dir: &Path,
    ) -> anyhow::Result<()> {
        self.write_reference_pages(docs, output_ref_dir)
    }

    fn generate_config_mts(
        &self,
        modules: &IndexMap<String, Vec<&FileDocument>>,
    ) -> String {
        let mut out = String::new();

        writeln!(out, "import {{ defineConfig }} from 'vitepress'").unwrap();
        writeln!(out).unwrap();
        writeln!(out, "export default defineConfig({{").unwrap();
        writeln!(out, "  lang: 'en-US',").unwrap();
        writeln!(out, "  title: 'C++ Reference',").unwrap();
        writeln!(out, "  description: 'C++ API Reference Documentation',").unwrap();
        writeln!(out).unwrap();
        writeln!(out, "  lastUpdated: true,").unwrap();
        writeln!(out).unwrap();
        writeln!(out, "  themeConfig: {{").unwrap();
        writeln!(out).unwrap();
        writeln!(out, "    // Navigation bar").unwrap();
        writeln!(out, "    nav: [").unwrap();
        writeln!(out, "      {{ text: 'Home', link: '/' }},").unwrap();
        writeln!(
            out,
            "      {{ text: 'Reference', link: '/reference/' }},"
        )
        .unwrap();
        writeln!(out, "    ],").unwrap();
        writeln!(out).unwrap();
        writeln!(out, "    // Sidebar — organised by @module").unwrap();
        writeln!(out, "    sidebar: {{").unwrap();
        writeln!(out, "      '/reference/': [").unwrap();

        for (module_name, file_docs) in modules {
            writeln!(out, "        {{").unwrap();
            writeln!(out, "          text: '{}',", module_name).unwrap();
            writeln!(out, "          collapsed: false,").unwrap();
            writeln!(out, "          items: [").unwrap();

            for doc in file_docs {
                let stem = file_stem(&doc.file_path).unwrap_or("unknown");
                let display = file_display_name(stem);
                writeln!(
                    out,
                    "            {{ text: '{}', link: '/reference/{}' }},",
                    display, stem
                )
                .unwrap();
            }

            writeln!(out, "          ],").unwrap();
            writeln!(out, "        }},").unwrap();
        }

        writeln!(out, "      ],").unwrap();
        writeln!(out, "    }},").unwrap();
        writeln!(out).unwrap();
        writeln!(out, "    socialLinks: [").unwrap();
        writeln!(out, "      {{ icon: 'github', link: '#' }},").unwrap();
        writeln!(out, "    ],").unwrap();
        writeln!(out, "  }},").unwrap();
        writeln!(out).unwrap();
        writeln!(out, "  markdown: {{").unwrap();
        writeln!(out, "    lineNumbers: true,").unwrap();
        writeln!(out, "  }},").unwrap();
        writeln!(out, "}});").unwrap();

        out
    }

    fn generate_index_md(
        &self,
        modules: &IndexMap<String, Vec<&FileDocument>>,
    ) -> String {
        let mut out = String::new();

        writeln!(out, "---").unwrap();
        writeln!(out, "title: C++ Reference").unwrap();
        writeln!(out, "---").unwrap();
        writeln!(out).unwrap();
        writeln!(out, "# C++ API Reference").unwrap();
        writeln!(out).unwrap();
        writeln!(out).unwrap();
        writeln!(
            out,
            "This site contains the auto-generated C++ API reference documentation."
        )
        .unwrap();
        writeln!(out).unwrap();

        if modules.is_empty() {
            writeln!(out, "_No documentation has been generated yet._").unwrap();
            writeln!(out).unwrap();
            return out;
        }

        writeln!(out, "## Modules").unwrap();
        writeln!(out).unwrap();
        writeln!(out, "| Module | Files |").unwrap();
        writeln!(out, "|--------|-------|").unwrap();

        for (module_name, file_docs) in modules {
            let file_count = file_docs.len();
            let links: Vec<String> = file_docs
                .iter()
                .map(|doc| {
                    let stem = file_stem(&doc.file_path).unwrap_or("unknown");
                    format!("[{}](reference/{})", stem, stem)
                })
                .collect();
            let display = if links.len() <= 3 {
                links.join(", ")
            } else {
                format!("{} files", file_count)
            };
            writeln!(out, "| **{}** | {} |", module_name, display).unwrap();
        }

        writeln!(out).unwrap();

        // Detailed listing per module
        for (module_name, file_docs) in modules {
            writeln!(out, "### {}", module_name).unwrap();
            writeln!(out).unwrap();

            if file_docs.is_empty() {
                continue;
            }

            writeln!(out, "| File | Description |").unwrap();
            writeln!(out, "|------|-------------|").unwrap();

            for doc in file_docs {
                let stem = file_stem(&doc.file_path).unwrap_or("unknown");
                let brief = self.get(&doc.brief).map(|s| s.as_str()).unwrap_or("");
                writeln!(out, "| [{}](reference/{}.md) | {} |", stem, stem, brief).unwrap();
            }

            writeln!(out).unwrap();
        }

        out
    }

    fn wrap_with_frontmatter(
        &self,
        md_gen: &MarkdownGenerator,
        doc: &FileDocument,
        file_stem: &str,
    ) -> String {
        let mut out = String::new();

        // --- YAML frontmatter ---
        writeln!(out, "---").unwrap();
        writeln!(out, "title: {}", file_stem).unwrap();
        if let Some(brief) = self.get(&doc.brief) {
            writeln!(out, "description: {}", escape_yaml(brief)).unwrap();
        }
        if let Some(ref module) = doc.module {
            writeln!(out, "category: {}", module).unwrap();
        }
        writeln!(out, "---").unwrap();
        writeln!(out).unwrap();

        if let Some(ref module) = doc.module {
            writeln!(out, "[← Back to {}](/)", module).unwrap();
            writeln!(out).unwrap();
        }

        let content = md_gen.generate_file(doc);

        // Skip the first line (the `# filename` heading) since the frontmatter
        // already provides the title.  Everything after the first blank line
        // is the real content.
        let body = if content.trim_start().starts_with("# ") {
            let after_heading = content
                .lines()
                .skip(1)
                .collect::<Vec<_>>()
                .join("\n");
            after_heading
        } else {
            content
        };

        out.push_str(&body);

        // Ensure trailing newline
        if !out.ends_with('\n') {
            writeln!(out).unwrap();
        }

        out
    }
}

fn file_stem(file_path: &str) -> Option<&str> {
    Path::new(file_path)
        .file_stem()
        .and_then(|s| s.to_str())
}

fn file_display_name(stem: &str) -> String {
    let mut out = String::with_capacity(stem.len());
    let mut cap_next = true;
    for ch in stem.chars() {
        if ch == '_' || ch == '-' || ch == '.' {
            out.push(' ');
            cap_next = true;
        } else if cap_next {
            out.extend(ch.to_uppercase());
            cap_next = false;
        } else {
            out.push(ch);
        }
    }
    out
}

fn escape_yaml(s: &str) -> String {
    s.replace('\\', "\\\\").replace('"', "\\\"")
}
