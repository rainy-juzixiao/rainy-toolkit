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

use std::io::{self, Write};
use std::time::{Duration, Instant};
use rand::Rng;
use terminal_size::{terminal_size, Width, Height};
use tokio::time::sleep;

// 颜色转义码
const GREEN: &str = "\x1b[32m";
const BRIGHT_GREEN: &str = "\x1b[92m";
const CYAN: &str = "\x1b[36m";
const WHITE: &str = "\x1b[37m";
const DIM: &str = "\x1b[2m";
const RESET: &str = "\x1b[0m";
const CLEAR_LINE: &str = "\x1b[2K";
const CHARS: &[u8] = b"01abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ@#$%&*+-=<>?/\\";

pub async fn code_rain(duration_secs: u64) {
    print!("\n");
    println!("🌧️  Initiating Smooth Code Rain 🌧️");
    println!("Press Ctrl+C to stop early");
    sleep(Duration::from_millis(500)).await;
    let (cols, rows) = match terminal_size() {
        Some((Width(w), Height(h))) => (w as usize, h as usize),
        None => (80, 24),
    };
    let width = cols.min(120);
    let height = rows.saturating_sub(3);
    let mut current_buffer = vec![vec![' '; width]; height];
    let mut prev_buffer = vec![vec![' '; width]; height];
    let mut color_buffer = vec![vec![""; width]; height];
    // 初始化雨滴
    let mut drops = vec![0; width];
    let mut speeds = vec![1; width];
    for i in 0..width {
        speeds[i] = 1 + (i % 3);
    }
    print!("\x1b[2J\x1b[H\x1b[?25l");
    io::stdout().flush().unwrap();
    let start_time = Instant::now();
    let duration = Duration::from_secs(duration_secs);
    while start_time.elapsed() < duration {
        for x in 0..width {
            if drops[x] == 0 && rand::thread_rng().gen_range(0..100) < 1 {
                drops[x] = 1;
            }
            if drops[x] > 0 {
                let drop_len = 8;
                for y in 0..height {
                    let drop_pos = drops[x] as i32 - y as i32;
                    if drop_pos >= 0 && drop_pos < drop_len as i32 {
                        let char_idx = rand::thread_rng().gen_range(0..CHARS.len());
                        let ch = CHARS[char_idx] as char;
                        current_buffer[y][x] = ch;
                        color_buffer[y][x] = match drop_pos {
                            0 => BRIGHT_GREEN,  // 头部 - 最亮
                            1 => GREEN,         // 次亮
                            2 => CYAN,          // 中等
                            3 => WHITE,         // 较暗
                            _ => DIM,           // 尾部 - 最暗
                        };
                    } else if y < height && x < width {
                        current_buffer[y][x] = ' ';
                    }
                }
                drops[x] += speeds[x];
                if drops[x] > height + 20 {
                    drops[x] = if rand::thread_rng().gen_range(0..100) < 10 { 1 } else { 0 };
                }
            } else {
                for y in 0..height {
                    current_buffer[y][x] = ' ';
                }
            }
        }
        print!("\x1b[H");
        for y in 0..height {
            for x in 0..width {
                let current_char = current_buffer[y][x];
                let prev_char = prev_buffer[y][x];
                // 只有当字符变化时才更新
                if current_char != prev_char {
                    // 移动到正确位置
                    print!("\x1b[{};{}H", y + 1, x + 1);
                    // 如果有颜色则应用
                    if !color_buffer[y][x].is_empty() {
                        print!("{}{}{}", color_buffer[y][x], current_char, RESET);
                    } else {
                        print!("{}", current_char);
                    }
                }
            }
        }
        let elapsed = start_time.elapsed().as_secs();
        let remaining = duration_secs.saturating_sub(elapsed);
        let progress = (elapsed as f32 / duration_secs as f32).min(1.0);
        let bar_width = 30;
        let filled = (progress * bar_width as f32) as usize;
        let bar = format!("[{}{}]", "█".repeat(filled), "░".repeat(bar_width - filled));
        print!("{}", CLEAR_LINE);
        print!("\x1b[{};1H", height + 1);
        print!("Rain | {:02}:{:02} / {:02}:{:02} | {} ",
               elapsed / 60, elapsed % 60,
               remaining / 60, remaining % 60,
               bar
        );
        print!("\x1b[{};1H", height + 2);
        print!("{}", CLEAR_LINE);
        // 4. 交换缓冲区
        std::mem::swap(&mut current_buffer, &mut prev_buffer);

        io::stdout().flush().unwrap();
        sleep(Duration::from_millis(60)).await;
    }
    print!("\x1b[2J\x1b[H\x1b[?25h");
    print!("\n\x1b[32m");
    print!("Code Rain Complete");
    print!("\x1b[0m\n");
    println!("\x1b[90m welcome back to reality.\x1b[0m\n");
}