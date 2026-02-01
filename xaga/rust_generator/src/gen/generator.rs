use crate::model::cpp_class::CppFunction;
use crate::model::CppClass;

pub fn generate_registration(class: &CppClass, functions: &[CppFunction], header_file: &str) -> String {
    let mut output = String::new();
    output.push_str("#include <rainy/meta/reflection/registration.hpp>\n");
    output.push_str(&format!("#include \"{}\"\n\n", header_file));
    output.push_str("using namespace rainy::meta::reflection;\n\n");
    output.push_str("RAINY_REFLECTION_REGISTRATION {\n");
    output.push_str(&format!("    registration::class_<{}>(\"{}\")\n", class.name, class.name));

    for func in functions {
        let param_list = func.params.join(", ");
        if func.is_static {
            output.push_str(&format!(
                "        .method(utility::get_overloaded_func<{}()>(&{}::{}))\n",
                func.return_type, class.name, func.name
            ));
        } else {
            output.push_str(&format!(
                "        .method(utility::get_overloaded_func<{}, {}{}>(&{}::{})\n",
                class.name,
                func.return_type,
                if param_list.is_empty() { "".to_string() } else { format!(", {}", param_list) },
                class.name,
                func.name
            ));
        }
    }

    output.push_str("    ;\n}\n");
    output
}
