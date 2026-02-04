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

use crate::utility::{blake3_hash_bytes};
use std::fs;
use std::path::Path;

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

pub fn hash_file(path: &Path) -> Result<String, std::io::Error> {
    let bytes = fs::read(path)?;
    Ok(blake3_hash_bytes(&bytes))
}