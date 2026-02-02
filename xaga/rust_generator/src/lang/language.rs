use serde::Deserialize;
use std::collections::HashMap;
use std::fs;
#[derive(Debug, Deserialize)]

pub struct Help {
    pub print_usage_header: String,
    pub options_desc: Vec<OptionsDesc>,
}

#[derive(Debug, Deserialize, Clone)]
pub struct OptionsDesc {
    pub input: String,
    pub verbose: String,
    pub out: String,
    pub help: String,
    pub suffix: String,
    pub config: String,
    pub lang: String,
}

#[derive(Debug, Deserialize, Clone)]
pub struct CodeRain {
    pub init_text: String,
    pub press_ctrl: String,
}

#[derive(Deserialize)]
pub struct EasterEggs {
    pub code_rain: Vec<CodeRain>,
}

#[derive(Deserialize)]
pub struct Messages {
    pub help: Vec<Help>,
    
    pub easter_eggs: EasterEggs,
}


pub fn load_help_sections(path: &str) -> Result<(String, OptionsDesc), Box<dyn std::error::Error>> {
    let content = fs::read_to_string(path)?;
    let messages: Messages = toml::from_str(&content)?;
    let mut help_header = String::new();
    let mut options_desc = OptionsDesc {
        input: String::new(),
        verbose: String::new(),
        out: String::new(),
        help: String::new(),
        suffix: String::new(),
        config: String::new(),
        lang: String::new(),
    };
    if let Some(help_section) = messages.help.first() {
        help_header = help_section.print_usage_header.clone();
        if let Some(options) = help_section.options_desc.first() {
            options_desc = options.clone();
        }
    }
    println!("Loaded help header: {}", help_header);
    println!("Loaded options description: {:?}", options_desc.help);
    Ok((help_header, options_desc))
}

pub fn load_easter_eggs(path: &str) -> Result<CodeRain, Box<dyn std::error::Error>> {
    let content = std::fs::read_to_string(path)?;
    let messages: Messages = toml::from_str(&content)?;
    messages
        .easter_eggs
        .code_rain
        .first()
        .cloned()
        .ok_or("No code_rain found".into())
}