use crate::lang::translate_string;

fn print_sub_item(command: &str, desc: &str, max_width: usize) {
    println!("{:<width$} | {}", command, desc, width = max_width);
}

pub fn print_usage() {
    println!("{}", translate_string("help-print_usage_header"));
    let command_items = vec![
        (
            "-i --input <input_file_path>",
            translate_string("help-options_desc-config"),
        ),
        (
            "-v --verbose",
            translate_string("help-options_desc-verbose"),
        ),
        (
            "-o --out <output_file_path>",
            translate_string("help-options_desc-out"),
        ),
        ("-h --help", translate_string("help-options_desc-help")),
        ("-s --suffix", translate_string("help-options_desc-suffix")),
        ("-c --config", translate_string("help-options_desc-config")),
        ("-l --lang", translate_string("help-options_desc-lang")),
    ];
    let max_width = command_items
        .iter()
        .map(|(cmd, _)| cmd.len())
        .max()
        .unwrap_or(0);
    for (command, desc) in &command_items {
        print_sub_item(command, desc, max_width);
    }
}
