use crate::cli::CommandArguments;
use crate::{include_statics, locate_runtime_resources};
use crate::model::cpp_code_registration::RegistrationClass;
use std::collections::HashMap;
use std::fs;
use std::path::PathBuf;
use strfmt::strfmt;

fn new_line(output: &mut String) {
    output.push_str("\n"); // 插入新行
}

fn generated_header_stub(cli: &CommandArguments, output: &mut String, path: &PathBuf) {
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

pub fn generate_registration(
    cli: &CommandArguments,
    registration_classes: &Vec<RegistrationClass>,
    header_file: &str,
) -> String {
    let mut output = String::new();
    let header_path = fs::canonicalize(header_file).unwrap();
    generated_header_stub(cli, &mut output, &header_path); // 生成头stub节
    output.push_str("RAINY_REFLECTION_REGISTRATION {\n");
    for item in registration_classes {
        let type_name = item.type_name();
        output.push_str(&format!(
            "    registration::class_<{}>(\"{}\")\n",
            type_name, type_name
        ));
        for ctor in item.constructors() {
            let param_list = ctor.params.join(", ");
            output.push_str(&format!("        .constructor<{}>()\n", param_list));
        }
        for func in item.functions() {
            let param_list = func.params.join(", ");
            if func.is_static {
                output.push_str(&format!(
                    "        .method(\"{}\",rainy::utility::get_overloaded_func<{}()>(&{}::{}))\n",
                    func.name, func.return_type, type_name, func.name
                ));
            } else {
                output.push_str(&format!(
                    "        .method(\"{}\",rainy::utility::get_overloaded_func<{}, {}({})>(&{}::{}))\n",
                    func.name,
                    type_name,
                    func.return_type,
                    if param_list.is_empty() {
                        "".to_string()
                    } else {
                        format!("{}", param_list)
                    },
                    type_name,
                    func.name
                ));
            }
        }
    }
    output.push_str("    ;\n}\n");
    if cli.verbose {
        println!("Finish generate for file: {}", header_file);
        println!("Generated code for these classes: ");
        // for item in registration_class {
        //     println!("{},", item.type_name());
        // }
        println!("Generated code: \n{}", output);
    }
    output
}
