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

use colored::Colorize;
use crate::lang::language::*;
use owo_colors::OwoColorize;

fn print_sub_item(command: &str, desc: &str, max_width: usize) {
    let cmd_colored = command.cyan().bold();
    let desc_colored = desc.white();
    println!("  {:<width$} → {}", cmd_colored, desc_colored, width = max_width);
}

pub fn print_usage() {
    let sections = load_help_sections().unwrap_or_else(|err| {
        panic!("Error loading help sections: {}", err);
    });
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

    let max_width = command_items.iter().map(|(cmd, _)| cmd.len()).max().unwrap_or(0);
    let separator = "━".repeat(max_width + 3 + 50);

    println!("\n{}\n{}\n", sections.0.green().bold(), separator.yellow());
    for (command, desc) in &command_items {
        print_sub_item(command, desc, max_width);
    }
    println!("\n{}", separator.yellow());
}
