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

use std::path::PathBuf;

static CPP_HEADER_EXTENSIONS: &'static [&'static str] =
    &["h", "hh", "hpp", "hxx", "h++", "inl"];

pub fn is_cpp_header(filepath: &PathBuf) -> bool {
    let extension = match filepath.extension() {
        Some(ext) => ext.to_str().unwrap(),
        None => "",
    };
    CPP_HEADER_EXTENSIONS
        .iter()
        .any(|ext| extension.eq(ext.to_string().as_str()))
}
