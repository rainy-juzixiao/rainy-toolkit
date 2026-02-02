mod cli;
mod dev_debug_tools;
mod gen;
mod lang;
mod model;
mod parser;
mod utility;

use crate::cli::{code_rain, find_node_by_range, print_usage, tea_ceremony, CommandArguments};
use crate::dev_debug_tools::ast_node_tree::print_node_tree;
use crate::gen::generate_registration;
use crate::model::cpp_class::{CppClass, ParseResult};
use crate::parser::extract_functions_and_ctors;
use crate::utility::{modify_filename_in_front, write_cpp_file};
use clap::Parser;
use parser::parse_cpp;

#[tokio::main]
async fn main() -> anyhow::Result<()> {
    let mut exit = false;
    let cli = CommandArguments::parse();
    if cli.tea {
        tea_ceremony().await;
        exit = true;
    } else if cli.rain {
        code_rain(cli.rain_duration).await;
        exit = true;
    }
    cli.input.is_none().then(|| {
        if !exit {
            //print_usage();
            exit = true;
        }
    });
    if exit {
        return Ok(());
    }
    let input_file = cli.input.as_ref().unwrap();
    let source = std::fs::read_to_string(input_file)?;
    let parse_result: ParseResult = parse_cpp(&source, &cli)?;
    for class in &parse_result.classes {
        if cli.verbose {
            println!("Found cpp moc class: {}", class.name);
        }
        if let Some(node) = find_node_by_range(
            parse_result.tree.root_node(),
            class.start_byte,
            class.end_byte,
        ) {
            if cli.dev {
                print_node_tree(node, &source, 0, true);
            }
            let mut functions = Vec::new();
            let mut ctors = Vec::new();
            extract_functions_and_ctors(node, &source, &mut functions, &mut ctors);
            let registration_code = generate_registration(class, &functions, &ctors, input_file);
            if cli.verbose {
                println!(
                    "Generated code for class {} , generated code: \n{}",
                    class.name, registration_code
                );
            }
            let out_file: String;
            if cli.out.is_none() {
                if cli.verbose {
                    println!("No output file specified, use the original filename to generate.");
                }
                out_file = input_file.clone();
            } else {
                out_file = cli.out.clone().unwrap();
            }
            let file_name = modify_filename_in_front(out_file, "moc_").unwrap();
            write_cpp_file(&registration_code, file_name.as_str())?;
        }
    }
    Ok(())
}
