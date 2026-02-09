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

use std::fs;
use std::path::{Path, PathBuf};
use std::process::Command;

fn copy_dir_recursive(from: &Path, to: &Path) -> std::io::Result<()> {
    fs::create_dir_all(to)?;
    for entry in fs::read_dir(from)? {
        let entry = entry?;
        let ty = entry.file_type()?;
        if ty.is_dir() {
            copy_dir_recursive(&entry.path(), &to.join(entry.file_name()))?;
        } else {
            fs::copy(entry.path(), to.join(entry.file_name()))?;
        }
    }
    Ok(())
}

fn main() {
    println!("Installing...");    
    // 1. 编译并安装二进制
    let status = Command::new("cargo")
        .args(["install", "--path", "."])
        .status()
        .expect("Failed to run cargo install");
    if !status.success() {
        eprintln!("Installation failed");
        std::process::exit(1);
    }
    // 2. 找到安装位置
    let cargo_bin = home::cargo_home()
        .expect("Cannot find cargo home")
        .join("bin");
    // 3. 复制配置文件
    let statics_src = PathBuf::from("statics");
    let statics_dst = cargo_bin.join("statics");
    copy_dir_recursive(&statics_src, &statics_dst)
        .expect("Failed to copy statics");
    println!("Installation complete!");
    println!("Config files: {:?}", statics_dst);
    println!("Now you are able to run with: rt_moc");
}