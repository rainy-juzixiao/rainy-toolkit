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

use std::{collections::HashMap, fs, path::PathBuf};

use strfmt::strfmt;

use crate::{cli::CommandArguments, include_statics, locate_runtime_resources, utility::new_line};

pub fn generate_header_stub(cli: &CommandArguments, output: &mut String, path: &PathBuf) {
    {
        output.push_str(include_statics!("embed_licences")); // 插入许可证说明
    }
    new_line(output);
    {
        let version = env!("CARGO_PKG_VERSION").to_string();
        let absoulte_source = path.display().to_string();
        let input_source = path.file_name().unwrap();

        let template = match fs::read_to_string(locate_runtime_resources!("embed_warnings")) {
            Ok(template) => template,
            Err(_) => {
                if cli.verbose {
                    println!("Cannot get embed_warnings template in statics folder, use internal embed_warnings template");
                }
                let template = include_statics!("embed_warnings");
                template.to_string()
            }
        };
        let mut vars: HashMap<String, String> = HashMap::new();
        vars.insert(
            "input_source".to_string(),
            input_source.to_str().unwrap().to_string(),
        );
        vars.insert("absoulte_source".to_string(), absoulte_source.to_string());
        vars.insert("generator_version".to_string(), version.to_string());
        let result = match strfmt(&template, &vars) {
            Ok(content) => content,
            Err(err) => {
                if cli.verbose {
                    println!(
                        "Err: {}\nCannot format liceneces stub, use default stub to generate",
                        err
                    );
                }
                let res = include_statics!("embed_warnings");
                res.to_string()
            }
        };
        output.push_str(&*result);
    }
    new_line(output);
    output.push_str(&format!("#include \"{}\"\n", path.to_str().unwrap()));
    output.push_str("#include <rainy/meta/reflection/registration.hpp>\n");
    new_line(output);
    output.push_str("using namespace rainy::meta::reflection;\n");
    {
        new_line(output);
        output.push_str(&format!("#line 1 \"{}\"\n", path.to_str().unwrap()));
        new_line(output);
    }
}
