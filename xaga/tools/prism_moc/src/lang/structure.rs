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

use serde::Deserialize;

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