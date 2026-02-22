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
use std::process::Command;

fn git_branch() -> Option<String> {
    let output = Command::new("git")
        .args(["rev-parse", "--abbrev-ref", "HEAD"])
        .output()
        .ok()?;

    if !output.status.success() {
        return None;
    }
    let branch = String::from_utf8(output.stdout).ok()?;
    Some(branch.trim().to_string())
}

fn main() {
    {
        let git_hash = Command::new("git")
            .args(["rev-parse", "--short", "HEAD"])
            .output()
            .ok()
            .and_then(|o| String::from_utf8(o.stdout).ok())
            .unwrap_or_else(|| "unknown".into());
        println!("cargo:rustc-env=GIT_HASH={}", git_hash.trim());
    }

    {
        let build_time = chrono::Utc::now().to_rfc3339();
        println!("cargo:rustc-env=BUILD_TIME={}", build_time);
    }

    {
        let branch = git_branch();
        println!("cargo:rustc-env=GIT_BRANCH={}", branch.unwrap_or_else(|| "unknown".into()));
    }
}
