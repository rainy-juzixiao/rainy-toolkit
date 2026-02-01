mod cli;
mod lang;
mod model;
mod parser;
mod utility;
mod gen;

use crate::cli::{code_rain, find_node_by_range, print_usage, tea_ceremony};
use crate::lang::{init_translate, translate_string};
use crate::model::cpp_class::ParseResult;
use crate::utility::convert_toml_to_fluent;
use clap::Parser;
use parser::parse_cpp;
use std::fs;
use tree_sitter::Node;
use crate::gen::generate_registration;
use crate::parser::extract_functions;

pub fn print_node_tree(node: Node, source: &str, indent: usize, show_source: bool) {
    let padding = " ".repeat(indent);

    if show_source {
        // 获取源码片段并去掉换行和多余空格
        let text = node
            .utf8_text(source.as_bytes())
            .unwrap_or("")
            .trim()
            .replace("\n", " ");
        println!(
            "{}{} [{}..{}]: {}",
            padding,
            node.kind(),
            node.start_byte(),
            node.end_byte(),
            text
        );
    } else {
        println!(
            "{}{} [{}..{}]",
            padding,
            node.kind(),
            node.start_byte(),
            node.end_byte()
        );
    }

    // 遍历子节点
    for i in 0..node.child_count() {
        if let Some(child) = node.child(i as u32) {
            print_node_tree(child, source, indent + 2, show_source);
        }
    }
}

#[derive(Parser, Debug)]
#[command(author, version, about, long_about = None)]
struct Cli {
    /// 输入文件路径
    #[arg(short, long)]
    input: Option<String>,

    /// 是否启用详细模式
    #[arg(short, long, action = clap::ArgAction::SetTrue)]
    verbose: bool,

    #[arg(long)]
    tea: bool,

    #[arg(long)]
    rain: bool,

    #[arg(short, long, default_value = "30")]
    rain_duration: u64,
}

#[tokio::main]
async fn main() -> anyhow::Result<()> {
    let lang = "en-US";
    let toml = fs::read_to_string(format!("misc/{lang}/generator_langpack.toml"))?;
    let ftl = convert_toml_to_fluent(&toml)?;
    init_translate(ftl, lang).is_err().then(|| {
        panic!("Fail to init translate component");
    });
    let mut exit = false;
    let cli = Cli::parse();
    if cli.tea {
        tea_ceremony().await;
        exit = true;
    } else if cli.rain {
        code_rain(cli.rain_duration).await;
        exit = true;
    }
    // cli.input.is_none().then(|| {
    //     if !exit {
    //         print_usage();
    //         exit = true;
    //     }
    // });
    if exit {
        return Ok(());
    }
    let source = std::fs::read_to_string("misc/example.cc")?;
    let parse_result: ParseResult = parse_cpp(&source)?;
    for class in &parse_result.classes {
        println!("Class: {}", class.name);
        if let Some(node) = find_node_by_range(
            parse_result.tree.root_node(),
            class.start_byte,
            class.end_byte,
        ) {
            let mut functions = Vec::new();
            extract_functions(node, &source, &mut functions);

            let registration_code = generate_registration(class, &functions, "example.cc");
            println!("{}", registration_code);
        }
    }
    Ok(())
}
