use crate::cli::CommandArguments;
use crate::{include_statics, locate_runtime_resources};
use lazy_static::lazy_static;
use serde::Deserialize;
use std::collections::HashMap;
use std::fs;
use std::sync::RwLock;

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

lazy_static! {
    static ref LANGUAGE_MAP: HashMap<&'static str, &'static str> = {
        let mut map = HashMap::new();
        map.insert("english-us", "en-US");
        map.insert("simpchinese", "zh-Hans");
        map.insert("tradchinese", "zh-Hant");
        map
    };
    static ref LANGUAGE: RwLock<&'static str> =
        RwLock::new(LANGUAGE_MAP.get("english-us").unwrap());
}

fn get_current_lang_res() -> std::io::Result<String> {
    let res = fs::read_to_string(locate_runtime_resources!(
        "{}/generator_langpack.toml",
        LANGUAGE.try_read().unwrap()
    ));
    res
}

fn has_lang_res(language: &String) -> bool {
    match fs::exists(format!("statics/{}/generator_langpack.toml", language)) {
        Ok(_) => true,
        Err(_) => false,
    }
}

pub fn set_this_session_lang(cli: &CommandArguments, language: &String) {
    if *language == *LANGUAGE.try_read().unwrap() {
        return;
    }
    let res = LANGUAGE.try_write();
    match res {
        Ok(mut cur_lang) => {
            let res = LANGUAGE_MAP.get(language.as_str());
            let wait_for_use_lang = match res {
                Some(lang) => lang,
                None => "english-us",
            };
            if has_lang_res(language) {
                *cur_lang = wait_for_use_lang;
            } else {
                if cli.verbose {
                    println!("Cannot get lang res, because the res dose not exists!");
                    return;
                }
            }
        }
        Err(_) => {
            if (cli.verbose) {
                println!("Cannot get write lock from language settings");
            }
            return;
        }
    }
}

pub fn load_help_sections() -> Result<(String, OptionsDesc), Box<dyn std::error::Error>> {
    let load_res = get_current_lang_res();
    let content =
        load_res.unwrap_or_else(|_| include_statics!("en-US/generator_langpack.toml").to_string());
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
    Ok((help_header, options_desc))
}

pub fn load_easter_eggs() -> Result<CodeRain, Box<dyn std::error::Error>> {
    let load_res = get_current_lang_res();
    let content = match load_res {
        Ok(content) => content,
        Err(_) => include_str!("../../statics/en-US/generator_langpack.toml").parse()?,
    };
    let messages: Messages = toml::from_str(&content)?;
    messages
        .easter_eggs
        .code_rain
        .first()
        .cloned()
        .ok_or("No code_rain found".into())
}
