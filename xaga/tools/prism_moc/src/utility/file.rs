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

use crate::cli::CommandArguments;
use crate::utility::{blake3_hash_bytes};
use std::fs;
use std::path::{Path, PathBuf};
use std::str::FromStr;
use crate::model::cpp_header_names::is_cpp_header;

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
    if !path.exists() {
        panic!("file doesn't exists!");
    }
    let bytes = match fs::read(path) {
        Ok(bytes) => bytes,
        Err(e) => {
            panic!("Cannot read input_path: {}, err: {}", path.to_str().unwrap(), e);
        }
    };
    Ok(blake3_hash_bytes(&bytes))
}

pub fn write_file(cli: &CommandArguments, input_file: &String, registration_code: &String) {
    let mut out_file: String;
    if cli.out.is_none() {
        if cli.verbose {
            println!("No output file specified, use the original filename to generate.");
        }
        out_file = input_file.clone();
        out_file = modify_filename_in_front(out_file, "moc_").unwrap();
    } else {
        out_file = cli.out.clone().unwrap();
        let mut path = PathBuf::from_str(out_file.as_str()).unwrap();
        if is_cpp_header(&path) || path.extension().is_none() {
            path.set_extension("cpp"); // 调整为cpp文件
        }
    }
    write_cpp_file(registration_code, &out_file.as_str()).unwrap();
}