use crate::data::document::FileDocument;
use std::path::{Path, PathBuf};

pub struct HtmlGenerator {
    tpl_dir: PathBuf,
}

impl HtmlGenerator {
    pub fn new(tpl_dir: impl AsRef<Path>) -> Self {
        Self { tpl_dir: tpl_dir.as_ref().to_path_buf() }
    }

    fn docs_index(docs: &[FileDocument]) -> Vec<serde_json::Value> {
        docs.iter()
            .map(|d| {
                serde_json::json!({
                    "filename": d.path.file_name().unwrap_or_default().to_string_lossy(),
                    "title": d.title.clone()
                })
            })
            .collect()
    }

    pub fn generate_site(&self, docs: &[FileDocument], out_dir: &Path) -> anyhow::Result<()> {
        std::fs::create_dir_all(out_dir)?;
        let tera = tera::Tera::new(self.tpl_dir.join("*.html").to_str().unwrap())?;
        let docs_index = Self::docs_index(docs);
        let mut ctx = tera::Context::new();
        ctx.insert("title", "sleepy html");
        ctx.insert("docs", &docs_index);
        let out = out_dir.join("index.html");
        std::fs::write(&out, tera.render("index.html", &ctx)?)?;
        let out_ref = out_dir.join("reference.html");
        std::fs::write(&out_ref, tera.render("reference.html", &ctx)?)?;
        for d in docs {
            let mut ctx = tera::Context::new();
            ctx.insert("title", &d.title);
            ctx.insert("path", &d.path.display().to_string());
            ctx.insert("body", &d.body);
            ctx.insert("docs", &docs_index);
            let name = d.path.file_stem().unwrap_or_default().to_string_lossy();
            std::fs::write(out_dir.join(format!("{}.html", name)), tera.render("page.html", &ctx)?)?;
        }
        Ok(())
    }

    pub fn generate_reference_only(&self, docs: &[FileDocument], out_dir: &Path) -> anyhow::Result<()> {
        std::fs::create_dir_all(out_dir)?;
        let tera = tera::Tera::new(self.tpl_dir.join("*.html").to_str().unwrap())?;
        let mut ctx = tera::Context::new();
        ctx.insert("docs", &Self::docs_index(docs));
        std::fs::write(out_dir.join("reference.html"), tera.render("reference.html", &ctx)?)?;
        Ok(())
    }
}
