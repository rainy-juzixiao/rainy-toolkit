use serde::Deserialize;
use std::fs;
use std::collections::HashMap;
#[derive(Debug, Deserialize)]
pub struct HelpSection {
    pub print_usage_header: String,
}

#[derive(Debug, Deserialize)]
pub struct OptionsDesc {
    pub input: String,
    pub verbose: String,
    pub out: String,
    pub help: String,
    pub suffix: String,
    pub config: String,
    pub lang: String,
}

#[derive(Debug, Deserialize)]
pub struct CodeRain {
    pub init_text: String,
    pub press_ctrl: String,
}

#[derive(Debug, Deserialize)]
pub struct EasterEggs {
    #[serde(rename = "code-rain")]
    pub code_rain: CodeRain,
}

#[derive(Debug, Deserialize)]
pub struct Messages {
    pub help: Vec<HelpSection>,

    #[serde(rename = "help.options_desc")]
    pub help_options_desc: Vec<OptionsDesc>,

    #[serde(rename = "easter-eggs")]
    pub easter_eggs: EasterEggs,
}

#[derive(Debug)]
pub struct AppConfig {
    pub help_header: String,
    pub options: OptionsDesc,
    pub easter_eggs: CodeRain,
}

impl AppConfig {
    pub fn new() -> Self {
        AppConfig {
            help_header: String::new(),
            options: OptionsDesc {
                input: String::new(),
                verbose: String::new(),
                out: String::new(),
                help: String::new(),
                suffix: String::new(),
                config: String::new(),
                lang: String::new(),
            },
            easter_eggs: CodeRain {
                init_text: String::new(),
                press_ctrl: String::new(),
            },
        }
    }
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
    }
    if let Some(options) = messages.help_options_desc.first() {
        options_desc = OptionsDesc {
            input: options.input.clone(),
            verbose: options.verbose.clone(),
            out: options.out.clone(),
            help: options.help.clone(),
            suffix: options.suffix.clone(),
            config: options.config.clone(),
            lang: options.lang.clone(),
        };
    }
    Ok((help_header, options_desc))
}

pub fn load_easter_eggs(path: &str) -> Result<CodeRain, Box<dyn std::error::Error>> {
    let content = fs::read_to_string(path)?;
    let messages: Messages = toml::from_str(&content)?;
    Ok(messages.easter_eggs.code_rain)
}