use crate::cli::CommandArguments;
use crate::lang::structure::{LanguageEntry, Messages};
use crate::{include_statics, locate_runtime_resources};
use lazy_static::lazy_static;
use std::collections::HashMap;
use std::fs;
use std::sync::RwLock;

lazy_static! {
    pub static ref LANGUAGE_MAP: RwLock<HashMap<String, LanguageEntry>> = RwLock::new({
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
    pub static ref LANGUAGE: RwLock<String> = RwLock::new("english-us".to_string());
}

pub fn load_messages_for_lang(lang_key: &str) -> Result<Messages, Box<dyn std::error::Error>> {
    let locale = {
        let map = LANGUAGE_MAP.read()?;
        let entry = map.get(lang_key).ok_or("language not registered")?;
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
    .unwrap_or_else(|_| include_statics!("en-US/generator_langpack.toml").to_string());
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
