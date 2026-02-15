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
use std::string::String;
use crate::lang::storage::{load_messages_for_lang, LANGUAGE};
use crate::lang::structure::{DescriptionSection, EasterEggs, OptionsDesc};

pub fn load_help_sections() -> Result<(String, OptionsDesc), Box<dyn std::error::Error>> {
    let lang = LANGUAGE.read()?.clone();
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
    let lang = LANGUAGE.read()?.clone();
    let messages = load_messages_for_lang(&lang)?;

    messages
        .description
        .into_iter()
        .next()
        .ok_or_else(|| "description section is empty".into())
}

pub fn load_easter_eggs() -> Result<EasterEggs, Box<dyn std::error::Error>> {
    let lang = LANGUAGE.read()?.clone();
    let messages = load_messages_for_lang(&lang)?;
    Ok(messages.easter_eggs)
}
