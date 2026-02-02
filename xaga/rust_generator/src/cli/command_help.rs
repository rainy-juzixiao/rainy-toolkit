use crate::lang::language::*;

fn print_sub_item(command: &str, desc: &str, max_width: usize) {
    println!("{:<width$} | {}", command, desc, width = max_width);
}

pub fn print_usage() {
    let sections = match load_help_sections("misc/en-US/generator_langpack.toml") {
        Ok(sections) => sections,
        Err(err) => {
            panic!("Error loading help sections: {}", err);
        }
    };
    let header = sections.0;
    let descs = sections.1;
    let command_items = vec![
        ("-i --input <input_file_path>", descs.input),
        ("-v --verbose", descs.verbose),
        ("-o --out <output_file_path>", descs.out),
        ("-h --help", descs.help),
        ("-s --suffix", descs.suffix),
        ("-c --config", descs.config),
        ("-l --lang", descs.lang),
    ];
    let max_width = command_items
        .iter()
        .map(|(cmd, _)| cmd.len())
        .max()
        .unwrap_or(0);
    println!("{}", header);
    for (command, desc) in &command_items {
        print_sub_item(command, desc, max_width);
    }
}
