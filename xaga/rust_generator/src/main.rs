mod cli;
mod dev_debug_tools;
mod gen;
mod lang;
mod marco;
mod model;
mod parser;
mod utility;

use crate::cli::{code_rain, find_node_by_range, print_usage, tea_ceremony, CommandArguments};
use crate::dev_debug_tools::ast_node_tree::print_node_tree;
use crate::gen::generate_registration;
use crate::lang::language::set_this_session_lang;
use crate::model::cpp_class::ParseResult;
use crate::model::cpp_code_registration::RegistrationClass;
use crate::utility::{modify_filename_in_front, write_cpp_file};
use clap::Parser;
use parser::parse_cpp;

#[tokio::main]
async fn main() -> anyhow::Result<()> {
    let mut exit = false;
    let cli = CommandArguments::parse();
    match &cli.lang {
        Some(lang) => set_this_session_lang(&cli, &lang),
        None => {}
    }
    if cli.help {
        print_usage();
        return Ok(());
    }
    if cli.tea {
        tea_ceremony().await;
        exit = true;
    } else if cli.rain {
        code_rain(cli.rain_duration).await;
        exit = true;
    }
    cli.input.is_none().then(|| {
        if !exit {
            print_usage();
            exit = true;
        }
    });
    if exit {
        return Ok(());
    }
    let input_file = cli.input.as_ref().unwrap();
    let source = std::fs::read_to_string(input_file)?;
    let parse_result: ParseResult = parse_cpp(&source, &cli)?;
    let mut total_generate: Vec<RegistrationClass> = Vec::new();
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
                print_node_tree(node, &source, true);
            }
            total_generate.push(RegistrationClass::new(class, &node, &source));
        }
    }
    if cli.verbose {
        if total_generate.is_empty() {
            println!(
                "We didn't seen any moc class to generate when we read {:?}",
                cli.input
            );
        }
        println!("Find {} moc class need to generate", total_generate.len());
    }
    let registration_code = generate_registration(&cli, &total_generate, input_file);
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
    Ok(())
}
