use std::fs;
use crate::model::{cpp_class::CppClass, cpp_ctor::CppCtor, cpp_function::CppFunction};

pub fn generate_registration(
    class: &CppClass,
    functions: &[CppFunction],
    ctors: &[CppCtor],
    header_file: &str,
) -> String {
    let mut output = String::new();
    output.push_str("#include <rainy/meta/reflection/registration.hpp>\n");
    let abs_header_file: String = match fs::canonicalize(header_file) {
        Ok(abs_path) => {
            let path_str = abs_path.to_string_lossy().to_string();
            path_str
        }
        Err(e) => panic!("Error: {}", e),
    };
    output.push_str(&format!("#include \"{}\"\n\n", abs_header_file));
    output.push_str("using namespace rainy::meta::reflection;\n\n");
    output.push_str("RAINY_REFLECTION_REGISTRATION {\n");
    output.push_str(&format!(
        "    registration::class_<{}>(\"{}\")\n",
        class.name, class.name
    ));
    for ctor in ctors {
        let param_list = ctor.params.join(", ");
        output.push_str(&format!("        .constructor<{}>()\n", param_list));
    }
    for func in functions {
        let param_list = func.params.join(", ");
        if func.is_static {
            output.push_str(&format!(
                "        .method(\"{}\",rainy::utility::get_overloaded_func<{}()>(&{}::{}))\n",
                func.name, func.return_type, class.name, func.name
            ));
        } else {
            output.push_str(&format!(
                "        .method(\"{}\",rainy::utility::get_overloaded_func<{}, {}({})>(&{}::{}))\n",
                func.name,
                class.name,
                func.return_type,
                if param_list.is_empty() {
                    "".to_string()
                } else {
                    format!("{}", param_list)
                },
                class.name,
                func.name
            ));
        }
    }
    output.push_str("    ;\n}\n");
    output
}
