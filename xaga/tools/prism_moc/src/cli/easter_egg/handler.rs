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

use crate::cli::{CommandArguments, code_rain, tea_ceremony};

pub async fn handle_easter_egg(cli: &CommandArguments) -> bool {
    if cli.tea {
        tea_ceremony().await;
        return true;
    } else if cli.rain {
        code_rain(cli.rain_duration).await;
        return true;
    }
    false
}
