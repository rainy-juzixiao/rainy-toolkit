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

#[derive(Debug, Deserialize, Clone)]

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

#[derive(Deserialize, Clone)]
pub struct EasterEggs {
    pub code_rain: Vec<CodeRain>,
}

#[derive(Deserialize, Clone)]
pub struct DescriptionSection {
    pub content: String,
}

#[derive(Deserialize, Clone)]
pub struct Messages {
    pub help: Vec<Help>,

    pub easter_eggs: EasterEggs,

    pub description: Vec<DescriptionSection>,
}

#[derive(Clone)]
pub struct LanguageEntry {
    pub locale: String,
    pub messages: Option<Messages>,
}

lazy_static! {
    static ref LANGUAGE_MAP: RwLock<HashMap<String, LanguageEntry>> = RwLock::new({
        let mut map = HashMap::new();
        map.insert(
            "english-us".to_string(),
            LanguageEntry {
                locale: "en-US".to_string(),
                messages: None,
            },
        );
        map.insert(
            "simpchinese".to_string(),
            LanguageEntry {
                locale: "zh-Hans".to_string(),
                messages: None,
            },
        );
        map.insert(
            "tradchinese".to_string(),
            LanguageEntry {
                locale: "zh-Hant".to_string(),
                messages: None,
            },
        );
        map
    });

    static ref LANGUAGE: RwLock<String> =
        RwLock::new("english-us".to_string());
}


fn load_messages_for_lang(lang_key: &str) -> Result<Messages, Box<dyn std::error::Error>> {
    let locale = {
        let map = LANGUAGE_MAP.read().unwrap();
        let entry = map
            .get(lang_key)
            .ok_or("language not registered")?;
        entry.locale.clone()
    };
    if let Ok(map) = LANGUAGE_MAP.read() {
        if let Some(entry) = map.get(lang_key) {
            if let Some(messages) = &entry.messages {
                return Ok(messages.clone());
            }
        }
    }
    let content = fs::read_to_string(locate_runtime_resources!(
        "{}/generator_langpack.toml",
        locale
    ))
    .or_else(|_| {
        fs::read_to_string(locate_res_path!(
            "{}/generator_langpack.toml",
            locale
        ))
    })
    .unwrap_or_else(|_| {
        include_statics!("en-US/generator_langpack.toml").to_string()
    });
    let messages: Messages = toml::from_str(&content)?;
    if let Ok(mut map) = LANGUAGE_MAP.write() {
        if let Some(entry) = map.get_mut(lang_key) {
            entry.messages = Some(messages.clone());
        }
    }
    Ok(messages)
}

fn has_lang_res(language: &String) -> bool {
    let locale = match LANGUAGE_MAP.read() {
        Ok(map) => match map.get(language) {
            Some(entry) => entry.locale.clone(),
            None => return false,
        },
        Err(_) => return false,
    };
    fs::exists(locate_runtime_resources!(
        "{}/generator_langpack.toml",
        locale
    ))
    .unwrap_or(false)
        || language == "english-us"
}

pub fn set_this_session_lang(cli: &CommandArguments, language: &String) {
    if *language == *LANGUAGE.read().unwrap() {
        return;
    }
    if !has_lang_res(language) {
        if cli.verbose {
            println!("Cannot get lang res, because the res does not exist!");
        }
        return;
    }
    let mut cur_lang = match LANGUAGE.write() {
        Ok(v) => v,
        Err(_) => {
            if cli.verbose {
                println!("Cannot get write lock from language settings");
            }
            return;
        }
    };
    let mut map = LANGUAGE_MAP.write().unwrap();
    match map.get_mut(language.as_str()) {
        Some(entry) => {
            *cur_lang = language.clone();
            entry.messages = None;
        }
        None => {
            if cli.verbose {
                println!("Language exists but not registered in LANGUAGE_MAP");
            }
        }
    }
}

pub fn load_help_sections() -> Result<(String, OptionsDesc), Box<dyn std::error::Error>> {
    let lang = LANGUAGE.read().unwrap().clone();
    let messages = load_messages_for_lang(&lang)?;
    let help = messages.help.first().ok_or("help section empty")?;
    let options = help
        .options_desc
        .first()
        .cloned()
        .ok_or("options desc empty")?;
    Ok((help.print_usage_header.clone(), options))
}

pub fn load_version_description() -> Result<DescriptionSection, Box<dyn std::error::Error>> {
    let lang = LANGUAGE.read().unwrap().clone();
    let messages = load_messages_for_lang(&lang)?;

    messages
        .description
        .into_iter()
        .next()
        .ok_or_else(|| "description section is empty".into())
}

pub fn load_easter_eggs() -> Result<EasterEggs, Box<dyn std::error::Error>> {
    let lang = LANGUAGE.read().unwrap().clone();
    let messages = load_messages_for_lang(&lang)?;
    Ok(messages.easter_eggs)
}
