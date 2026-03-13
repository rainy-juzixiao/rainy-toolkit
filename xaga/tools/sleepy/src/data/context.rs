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
use clang::Entity;

pub struct ParseContext<'a> {
    pub include_root: &'a str,
    pub nodoc_ranges: &'a [std::ops::Range<u32>],
    pub owned_files: &'a std::collections::HashSet<std::path::PathBuf>,
}

pub fn is_from_owned_file(entity: &Entity, ctx: &ParseContext) -> bool {
    entity
        .get_location()
        .and_then(|loc| {
            let fl = loc.get_file_location();
            fl.file.map(|f| {
                let p = std::fs::canonicalize(f.get_path()).unwrap_or(f.get_path());
                ctx.owned_files.contains(&p)
            })
        })
        .unwrap_or(false)
}
