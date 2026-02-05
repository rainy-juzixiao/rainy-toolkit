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

use crate::cli::CommandArguments;
use crate::{include_statics, locate_res_path, locate_runtime_resources};
use lazy_static::lazy_static;
use serde::Deserialize;
use std::collections::HashMap;
use std::fs;
use std::string::String;
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
    pub complete: String,
    pub welcome_back: String,
    pub interrupted_text: String,
}

#[derive(Deserialize)]
pub struct EasterEggs {
    pub code_rain: Vec<CodeRain>,
}

#[derive(Deserialize)]
pub struct DescriptionSection {
    pub content: String,
}

#[derive(Deserialize)]
pub struct Messages {
    pub help: Vec<Help>,

    pub easter_eggs: EasterEggs,

    pub description: Vec<DescriptionSection>,
}

lazy_static! {
    static ref LANGUAGE_MAP: RwLock<HashMap<String, (String, String)>> = RwLock::new({
        let mut map: HashMap<String, (String, String)> = HashMap::new();
        map.insert(
            "english-us".to_string(),
            ("en-US".to_string(), "".to_string()),
        );
        map.insert(
            "simpchinese".to_string(),
            ("zh-Hans".to_string(), "".to_string()),
        );
        map.insert(
            "tradchinese".to_string(),
            ("zh-Hant".to_string(), "".to_string()),
        );
        map
    });
    static ref LANGUAGE: RwLock<String> = RwLock::new({
        if let Some(v) = LANGUAGE_MAP.try_read().unwrap().get("english-us") {
            return RwLock::new(v.0.clone());
        }
        "english-us".to_string()
    });
}

fn get_current_lang_res() -> std::io::Result<String> {
    match LANGUAGE_MAP.try_read() {
        Ok(map) => match map.get(LANGUAGE.try_read().unwrap().to_string().as_str()) {
            Some(get) => {
                if !get.1.is_empty() {
                    return Ok(get.1.to_string());
                }
            }
            None => {}
        },
        Err(_) => {}
    }
    let res = fs::read_to_string(locate_runtime_resources!(
        "{}/generator_langpack.toml",
        LANGUAGE.try_read().unwrap()
    ))
    .unwrap_or_else(|_| {
        let res = match fs::read_to_string(locate_res_path!(
            "{}/generator_langpack.toml",
            LANGUAGE.try_read().unwrap()
        )) {
            Ok(res) => res,
            Err(_) => {
                return include_statics!("en-US/generator_langpack.toml").to_string();
            }
        };
        res
    });
    Ok(res)
}

fn has_lang_res(language: &String) -> bool {
    match LANGUAGE_MAP.try_read() {
        Ok(map) => {
            if let Some(content) = map.get(language) {
                if !&content.1.is_empty() {
                    return true;
                }
            }
        }
        Err(_) => {}
    }
    match fs::exists(locate_runtime_resources!(
        "{}/generator_langpack.toml",
        language
    )) {
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
            let mut map = LANGUAGE_MAP.try_write().unwrap();
            let res = map.get(language.as_str());
            let default_res = (
                "english-us".to_string(),
                include_statics!("en-US/generator_langpack.toml").to_string(),
            );
            let wait_for_use_lang = res.unwrap_or_else(|| &default_res).to_owned();
            if has_lang_res(language) {
                let (k, v) = wait_for_use_lang;
                *cur_lang = k.to_string();
                match map.get_mut(language) {
                    Some(content) => {
                        content.1 = v;
                    }
                    None => {
                        panic!("Cannot set the language");
                    }
                }
            } else {
                if cli.verbose {
                    println!("Cannot get lang res, because the res dose not exists!");
                    return;
                }
            }
        }
        Err(_) => {
            if cli.verbose {
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

pub fn load_version_description() -> Result<DescriptionSection, Box<dyn std::error::Error>> {
    let load_res = get_current_lang_res();
    let content = load_res.unwrap_or_else(|_| {
        include_statics!("en-US/generator_langpack.toml")
            .parse()
            .unwrap()
    });
    let messages: Messages = toml::from_str(&content).unwrap_or_else(|_| {
        let res = include_statics!("en-US/generator_langpack.toml");
        toml::from_str(res).unwrap()
    });
    messages
        .description
        .into_iter()
        .next()
        .ok_or_else(|| "description section is empty".into())
}

pub fn load_easter_eggs() -> Result<EasterEggs, Box<dyn std::error::Error>> {
    let load_res = get_current_lang_res();
    let content = match load_res {
        Ok(content) => content,
        Err(_) => include_statics!("en-US/generator_langpack.toml").parse()?,
    };
    let messages: Messages = toml::from_str(&content)?;
    Ok(messages.easter_eggs)
}
