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

use clap::{Parser};

#[derive(Parser, Debug)]
#[command(author, version, about, long_about = None, disable_help_flag = true, disable_version_flag = true
)]
pub struct CommandArguments {
    /// 输入文件路径
    #[arg(short, long)]
    pub input: Option<String>,

    #[arg(long)]
    pub lang: Option<String>,

    #[arg(long, action = clap::ArgAction::SetTrue)]
    pub verbose: bool,

    #[arg(long)]
    pub tea: bool,

    #[arg(long)]
    pub rain: bool,

    #[arg(short, long, default_value = "30")]
    pub rain_duration: u64,

    #[arg(long)]
    pub dev: bool,

    #[arg(short, long)]
    pub out: Option<String>,

    #[arg(long, action = clap::ArgAction::SetTrue)]
    pub help: bool,

    #[arg(short, long, action = clap::ArgAction::SetTrue)]
    pub version: bool,

    #[arg(long = "no-cache", action = clap::ArgAction::SetTrue)]
    pub no_cache: bool,
}
