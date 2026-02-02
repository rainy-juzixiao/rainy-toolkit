use std::fs;
use std::path::{Path, PathBuf};

pub fn write_cpp_file(output: &str, path: &str) -> anyhow::Result<()> {
    fs::write(path, output)?;
    Ok(())
}

pub fn modify_filename_in_front<P: AsRef<Path>>(file_path: P, modifier: &str) -> Option<String> {
    let path = file_path.as_ref();
    let stem = path.file_stem()?.to_string_lossy();
    let extension = path.extension().map(|ext| ext.to_string_lossy());
    let new_stem = format!("{}{}", modifier, stem);
    let new_filename = match extension {
        Some(ext) => format!("{}.{}", new_stem, ext),
        None => new_stem,
    };
    Some(new_filename)
}