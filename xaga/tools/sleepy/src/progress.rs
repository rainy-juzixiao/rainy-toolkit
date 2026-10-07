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
use std::io::{IsTerminal, Write};

const BAR_WIDTH: usize = 24;

pub struct Progress {
    enabled: bool,
    tty: bool,
    color: bool,
    total: u64,
    done: u64,
    width: usize,
    active: bool,
}

impl Progress {
    pub fn new(total: u64) -> Self {
        let tty = std::io::stderr().is_terminal();
        Self {
            enabled: std::env::var("SLEEPY_QUIET").as_deref() != Ok("1"),
            tty,
            color: std::env::var_os("NO_COLOR").is_none(),
            total,
            done: 0,
            width: terminal_size::terminal_size()
                .map(|(w, _)| w.0 as usize)
                .unwrap_or(80),
            active: false,
        }
    }

    fn paint(&self, code: &str, text: &str) -> String {
        if self.color {
            format!("{code}{text}\x1b[0m")
        } else {
            text.to_string()
        }
    }

    pub fn tick(&mut self, label: &str) {
        if !self.enabled {
            return;
        }
        self.done += 1;
        let frac = if self.total == 0 {
            1.0
        } else {
            (self.done as f64 / self.total as f64).clamp(0.0, 1.0)
        };
        let filled = (frac * BAR_WIDTH as f64).round() as usize;
        let bar = format!(
            "{}{}",
            self.paint("\x1b[32m", &"#".repeat(filled)),
            self.paint("\x1b[2m", &"-".repeat(BAR_WIDTH - filled)),
        );
        let counter = self.paint("\x1b[36m", &format!("{}/{}", self.done, self.total));
        let head = self.paint("\x1b[1m", "[sleepy]");
        let prefix = format!("{head} {counter} [{bar}] ");

        let plain_len = format!("[sleepy] {}/{} [{}] ", self.done, self.total, "-".repeat(BAR_WIDTH))
            .chars()
            .count();
        let budget = self.width.saturating_sub(plain_len + 1);
        let shown = self.paint("\x1b[1m", &truncate_left(label, budget));

        let mut err = std::io::stderr();
        if self.tty {
            let _ = write!(err, "\r\x1b[K{prefix}{shown}");
        } else {
            let _ = write!(err, "{prefix}{shown}\n");
        }
        let _ = err.flush();
        self.active = true;
    }

    pub fn finish(&mut self) {
        if self.enabled && self.active && self.tty {
            let _ = writeln!(std::io::stderr());
        }
    }
}

impl Drop for Progress {
    fn drop(&mut self) {
        self.finish();
    }
}

fn truncate_left(s: &str, max: usize) -> String {
    if max == 0 {
        return String::new();
    }
    let count = s.chars().count();
    if count <= max {
        return s.to_string();
    }
    s.chars().skip(count - max).collect()
}
