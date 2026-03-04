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

/// 创建并返回一个带有默认值的命令行参数结构体 `CommandArguments`。
///
/// 所有字段都被初始化为默认状态：
/// - `input`: `None`，表示没有指定输入文件或路径
/// - `lang`: `None`，表示没有指定语言
/// - `verbose`: `false`，表示默认不输出详细日志
/// - `tea`: `false`，自定义布尔选项，默认关闭
/// - `rain`: `false`，自定义布尔选项，默认关闭
/// - `rain_duration`: `0`，雨的持续时间，默认 0
/// - `dev`: `false`，开发模式标志，默认关闭
/// - `out`: `None`，表示没有指定输出路径
/// - `help`: `false`，是否显示帮助信息，默认不显示
/// - `version`: `false`，是否显示版本信息，默认不显示
/// - `no_cache`: `false`，是否禁用缓存，默认启用缓存
/// - `clear_cache`: `false`，是否清理缓存，默认不清理
///
#[allow(unused)]
pub fn default_cli() -> CommandArguments {
    CommandArguments {
        input: None,
        lang: None,
        verbose: false,
        tea: false,
        rain: false,
        rain_duration: 0,
        dev: false,
        out: None,
        help: false,
        version: false,
        no_cache: false,
        clear_cache: false,
    }
}

