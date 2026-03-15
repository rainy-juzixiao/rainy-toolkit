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

pub fn find_comment(file_path: &str, start_line: usize) -> Option<String> {
    let source = fs::read_to_string(file_path).ok()?;
    let lines: Vec<&str> = source.lines().collect();
    if start_line < 2 {
        return None;
    }
    let mut comment = Vec::new();
    let mut i = start_line - 2;

    loop {
        let line = lines[i].trim();
        if line.starts_with("//") {
            comment.push(line);
        } else if line.ends_with("*/") {
            comment.push(line);
            loop {
                if i == 0 { break; }
                i -= 1;
                let l = lines[i].trim();
                comment.push(l);
                if l.starts_with("/*") {
                    break;
                }
            }
            break;
        } else {
            // 空行或其他内容，终止
            break;
        }
        if i == 0 { break; }
        i -= 1;
    }

    if comment.is_empty() {
        None
    } else {
        comment.reverse();
        Some(comment.join("\n"))
    }
}