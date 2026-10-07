use crate::data::document::FileDocument;
use crate::generator::markdown::MarkdownGenerator;
use anyhow::Context;
use std::collections::HashMap;
use std::path::{Path, PathBuf};

const LAYOUT_TPL: &str = include_str!("../../assets/html/layout.html");
const INDEX_TPL: &str = include_str!("../../assets/html/index.html");
const PAGE_TPL: &str = include_str!("../../assets/html/page.html");
const REFERENCE_TPL: &str = include_str!("../../assets/html/reference.html");

pub struct HtmlGenerator {
    tpl_dir: PathBuf,
    lang: String,
}

impl HtmlGenerator {
    pub fn new(tpl_dir: impl AsRef<Path>, lang: impl Into<String>) -> Self {
        Self {
            tpl_dir: tpl_dir.as_ref().to_path_buf(),
            lang: lang.into(),
        }
    }

    fn load_tera(&self) -> anyhow::Result<tera::Tera> {
        if !self.tpl_dir.as_os_str().is_empty() {
            let pattern = self.tpl_dir.join("*.html");
            if let Some(pattern_str) = pattern.to_str() {
                match tera::Tera::new(pattern_str) {
                    Ok(fs_tera) if fs_tera.get_template_names().count() > 0 => {
                        return Ok(fs_tera);
                    }
                    Ok(_) => {}
                    Err(e) => {
                        eprintln!(
                            "[sleepy warn] HTML template dir {} unusable ({}), falling back to embedded templates",
                            self.tpl_dir.display(),
                            e
                        );
                    }
                }
            }
        }
        let mut tera = tera::Tera::default();
        tera.add_raw_template("layout.html", LAYOUT_TPL)
            .context("failed to load embedded layout.html")?;
        tera.add_raw_template("index.html", INDEX_TPL)
            .context("failed to load embedded index.html")?;
        tera.add_raw_template("page.html", PAGE_TPL)
            .context("failed to load embedded page.html")?;
        tera.add_raw_template("reference.html", REFERENCE_TPL)
            .context("failed to load embedded reference.html")?;
        if tera.get_template_names().count() == 0 {
            anyhow::bail!("no HTML templates available");
        }
        Ok(tera)
    }

    fn page_rel(&self, d: &FileDocument, flat: bool) -> PathBuf {
        if flat {
            let stem = d
                .path
                .file_stem()
                .map(|s| s.to_string_lossy().to_string())
                .unwrap_or_else(|| d.title.clone());
            PathBuf::from(format!("{stem}.html"))
        } else {
            let rel = if d.rel_path.is_absolute() {
                d.path
                    .file_name()
                    .map(PathBuf::from)
                    .unwrap_or_else(|| PathBuf::from(format!("{}.html", d.title)))
            } else {
                d.rel_path.with_extension("html")
            };
            if d.source_name.is_empty() {
                rel
            } else {
                Path::new(&d.source_name).join(rel)
            }
        }
    }

    fn pages_of<'a>(
        &self,
        docs: &'a [FileDocument],
        flat: bool,
    ) -> anyhow::Result<Vec<(&'a FileDocument, PathBuf)>> {
        let mut used: HashMap<String, usize> = HashMap::new();
        let mut pages = Vec::with_capacity(docs.len());
        for d in docs {
            let mut rel = self.page_rel(d, flat);
            if flat {
                let key = rel.to_string_lossy().to_string();
                let count = used.entry(key.clone()).or_insert(0);
                *count += 1;
                if *count > 1 {
                    let stem = rel
                        .file_stem()
                        .map(|s| s.to_string_lossy().to_string())
                        .unwrap_or_default();
                    rel = PathBuf::from(format!("{stem}_{count}.html"));
                }
            }
            if rel.is_absolute() {
                anyhow::bail!(
                    "refusing to write outside HTML dir (absolute page path): {}",
                    rel.display()
                );
            }
            pages.push((d, rel));
        }
        Ok(pages)
    }

    fn docs_index(&self, pages: &[(&FileDocument, PathBuf)], root_prefix: &str) -> Vec<serde_json::Value> {
        let mut entries: Vec<serde_json::Value> = pages
            .iter()
            .map(|(d, rel)| {
                let filename = rel
                    .file_stem()
                    .map(|s| s.to_string_lossy().to_string())
                    .unwrap_or_else(|| d.title.clone());
                serde_json::json!({
                    "filename": filename,
                    "title": d.title.clone(),
                    "href": format!("{root_prefix}{}", to_posix(rel)),
                    "path": d.file_path.clone(),
                })
            })
            .collect();
        entries.sort_by(|a, b| {
            a.get("href")
                .and_then(|v| v.as_str())
                .cmp(&b.get("href").and_then(|v| v.as_str()))
        });
        entries
    }

    fn markdown_to_html(&self, md: &str) -> String {
        let parser = pulldown_cmark::Parser::new_ext(
            md,
            pulldown_cmark::Options::all(),
        );
        let mut html = String::new();
        pulldown_cmark::html::push_html(&mut html, parser);
        html
    }

    fn render_body(&self, d: &FileDocument) -> String {
        let md_gen = MarkdownGenerator::new(self.lang.clone());
        let md = md_gen.generate_file(d);
        self.markdown_to_html(&md)
    }

    pub fn generate_site(
        &self,
        docs: &[FileDocument],
        out_dir: &Path,
        flat: bool,
    ) -> anyhow::Result<usize> {
        if out_dir.as_os_str().is_empty() {
            anyhow::bail!("HTML output dir is empty");
        }
        std::fs::create_dir_all(out_dir)
            .with_context(|| format!("cannot create HTML dir: {}", out_dir.display()))?;
        let tera = self.load_tera()?;
        let pages = self.pages_of(docs, flat)?;
        let root_docs = self.docs_index(&pages, "");

        let mut ctx = tera::Context::new();
        ctx.insert("title", "sleepy html");
        ctx.insert("docs", &root_docs);
        ctx.insert("root_prefix", "");
        std::fs::write(
            out_dir.join("index.html"),
            tera.render("index.html", &ctx)
                .context("failed to render index.html")?,
        )
        .context("failed to write index.html")?;
        std::fs::write(
            out_dir.join("reference.html"),
            tera.render("reference.html", &ctx)
                .context("failed to render reference.html")?,
        )
        .context("failed to write reference.html")?;

        let mut written = 2;
        for (d, rel) in &pages {
            let depth = rel.components().count().saturating_sub(1);
            let root_prefix = "../".repeat(depth);
            let mut ctx = tera::Context::new();
            ctx.insert("title", &d.title);
            ctx.insert("path", &d.file_path);
            ctx.insert("body", &self.render_body(d));
            ctx.insert("root_prefix", &root_prefix);
            ctx.insert("docs", &self.docs_index(&pages, &root_prefix));
            let page_path = out_dir.join(rel);
            if let Some(parent) = page_path.parent() {
                std::fs::create_dir_all(parent).with_context(|| {
                    format!("cannot create HTML page dir: {}", parent.display())
                })?;
            }
            std::fs::write(
                &page_path,
                tera.render("page.html", &ctx)
                    .with_context(|| format!("failed to render page for {}", d.file_path))?,
            )
            .with_context(|| format!("failed to write {}", page_path.display()))?;
            written += 1;
        }
        Ok(written)
    }

    pub fn generate_reference_only(
        &self,
        docs: &[FileDocument],
        out_dir: &Path,
        flat: bool,
    ) -> anyhow::Result<usize> {
        std::fs::create_dir_all(out_dir)
            .with_context(|| format!("cannot create HTML dir: {}", out_dir.display()))?;
        let tera = self.load_tera()?;
        let pages = self.pages_of(docs, flat)?;
        let mut ctx = tera::Context::new();
        ctx.insert("title", "sleepy html");
        ctx.insert("docs", &self.docs_index(&pages, ""));
        ctx.insert("root_prefix", "");
        std::fs::write(
            out_dir.join("reference.html"),
            tera.render("reference.html", &ctx)
                .context("failed to render reference.html")?,
        )
        .context("failed to write reference.html")?;
        Ok(1)
    }
}

fn to_posix(p: &Path) -> String {
    p.components()
        .map(|c| c.as_os_str().to_string_lossy().to_string())
        .collect::<Vec<_>>()
        .join("/")
}
