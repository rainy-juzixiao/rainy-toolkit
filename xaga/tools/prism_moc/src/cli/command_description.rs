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

use std::collections::HashMap;

use crate::{cli::CommandArguments, lang::language::load_version_description};

const GIT_HASH: &str = env!("GIT_HASH");
const BUILD_TIME: &str = env!("BUILD_TIME");
const GIT_BRANCH: &str = env!("GIT_BRANCH");
const AUTHORS: &str = env!("CARGO_PKG_AUTHORS");

pub fn print_moc_compiler_description(cli: &CommandArguments) {
    let description = load_version_description().unwrap();
    let template = description.content;
    let mut vars: HashMap<String, String> = HashMap::new();
    vars.insert("git_hash".to_string(), GIT_HASH.to_string());
    vars.insert("build_time".to_string(), BUILD_TIME.to_string());
    vars.insert("git_branch".to_string(), GIT_BRANCH.to_string());
    vars.insert("authors".to_string(), AUTHORS.to_string());

    let content = match strfmt::strfmt(&template, &vars) {
        Ok(content) => content,
        Err(err) => {
            if cli.verbose {
                println!(
                    "Err: {}\nCannot format version stub, use default stub to generate",
                    err
                );
            }
            template.to_string()
        }
    };

    println!("{}", content);
}
