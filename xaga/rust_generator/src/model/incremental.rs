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

use crate::utility::{hash_build_config, hash_file};
use serde::{Deserialize, Serialize};
use std::collections::HashMap;
use std::fs::File;
use std::path::{Path, PathBuf};
use std::time::{SystemTime, UNIX_EPOCH};
use std::{fs, io};

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct FileState {
    /// 源文件内容 hash
    pub hash: String,
    /// 最后修改时间(纳秒时间戳)
    pub timestamp: u128,
    /// 该文件生成代码的 hash
    pub generated_code_hash: String,
}

#[derive(Debug, Serialize, Deserialize, Clone)]
pub struct IncrementalCache {
    /// 源文件路径 -> 文件状态
    pub file_states: HashMap<PathBuf, FileState>,
    /// target 名称 -> 配置 hash
    pub target_config_hashes: HashMap<String, String>,
    /// 输出文件路径 -> 源文件路径(用于缓存临时文件)
    pub output_mapping: HashMap<PathBuf, PathBuf>,
    /// 缓存版本号
    pub version: u32,
    /// 上一次成功构建的时间
    pub last_build_time: SystemTime,
}

#[derive(Debug)]
pub struct BuildChanges {
    pub needs_full_rebuild: bool,
    pub files_to_regenerate: Vec<PathBuf>,
    pub stale_outputs: Vec<PathBuf>,
}

#[derive(Debug)]
pub enum CacheError {
    VersionMismatch,
    InvalidOpenCache,
    ErrorInSerialize,
    ErrorInDeserialize,
    IoError
}

impl IncrementalCache {
    pub fn save_to<P: AsRef<Path>>(&self, path: P) -> Result<(), CacheError> {
        let file = match File::create(path) {
            Ok(f) => f,
            Err(_) => {
                return Err(CacheError::InvalidOpenCache);
            }
        };
        match bincode::serialize_into(file, self) {
            Ok(_) => {}
            Err(err) => {
                println!("Cannot serialize value ,error: {}", err);
                return Err(CacheError::ErrorInSerialize);
            }
        }
        Ok(())
    }

    pub fn load_from<P: AsRef<Path>>(path: P, expected_version: u32) -> Result<Self, CacheError> {
        let file = match File::open(path) {
            Ok(f) => f,
            Err(_) => {
                return Err(CacheError::InvalidOpenCache);
            }
        };
        let cache: IncrementalCache = match bincode::deserialize_from(file) {
            Ok(cache) => cache,
            Err(_) => return Err(CacheError::ErrorInDeserialize)
        };
        if cache.version != expected_version {
            return Err(CacheError::VersionMismatch);
        }
        Ok(cache)
    }
}

#[derive(Serialize)]
struct BuildConfig {
    optimize: bool,
    target: String,
}

pub const CACHE_VERSION: u32 = 1;

#[derive(Debug)]
pub struct IncrementalState {
    pub input_path: PathBuf,
    pub input_hash: String,
    pub input_timestamp: u128,
    pub config_hash: String,
    pub cache: IncrementalCache,
    pub cache_path: PathBuf,
    pub current_file_state: FileState,
}

impl IncrementalState {
    /// 创建新的增量构建状态
    pub fn new(input_path: PathBuf) -> io::Result<Self> {
        let cache_path = PathBuf::from("./build/incremental_cache.bin");

        // 加载或创建缓存
        let cache = IncrementalCache::load_from(&cache_path, CACHE_VERSION).unwrap_or_else(|_| {
            IncrementalCache {
                file_states: HashMap::new(),
                target_config_hashes: HashMap::new(),
                output_mapping: HashMap::new(),
                version: CACHE_VERSION,
                last_build_time: SystemTime::UNIX_EPOCH,
            }
        });

        // 计算配置 hash
        let build_config = BuildConfig {
            optimize: true,
            target: "x86_64".into(),
        };
        let config_hash = hash_build_config(&build_config);

        // 计算输入文件 hash 和时间戳
        let input_hash = hash_file(&input_path)?;
        let metadata = fs::metadata(&input_path)?;
        let input_timestamp = metadata
            .modified()?
            .duration_since(UNIX_EPOCH)
            .unwrap()
            .as_nanos();

        let current_file_state = FileState {
            hash: input_hash.clone(),
            timestamp: input_timestamp,
            generated_code_hash: String::new(),
        };

        Ok(IncrementalState {
            input_path,
            input_hash,
            input_timestamp,
            config_hash,
            cache,
            cache_path,
            current_file_state,
        })
    }

    /// 分析需要重新构建的内容
    pub fn analyze_changes(&self) -> BuildChanges {
        let mut needs_full_rebuild = false;
        let mut files_to_regenerate = Vec::new();
        let stale_outputs = Vec::new();

        // 检查配置是否变化
        if let Some(old_config_hash) = self.cache.target_config_hashes.get("default") {
            if old_config_hash != &self.config_hash {
                needs_full_rebuild = true;
            }
        } else {
            needs_full_rebuild = true;
        }

        // 检查输入文件是否变化
        if let Some(old_state) = self.cache.file_states.get(&self.input_path) {
            if old_state.hash != self.input_hash || old_state.timestamp != self.input_timestamp {
                files_to_regenerate.push(self.input_path.clone());
            }
        } else {
            files_to_regenerate.push(self.input_path.clone());
        }

        BuildChanges {
            needs_full_rebuild,
            files_to_regenerate,
            stale_outputs,
        }
    }

    pub fn get_cached_temp_file(&self) -> Option<&PathBuf> {
        self.cache.output_mapping.get(&self.input_path)
    }

    pub fn update_file_state(&mut self, generated_code_hash: String, temp_file_path: PathBuf) {
        self.current_file_state.generated_code_hash = generated_code_hash;
        self.cache
            .file_states
            .insert(self.input_path.clone(), self.current_file_state.clone());
        self.cache
            .target_config_hashes
            .insert("default".to_string(), self.config_hash.clone());
        self.cache
            .output_mapping
            .insert(self.input_path.clone(), temp_file_path);
        self.cache.last_build_time = SystemTime::now();
    }

    pub fn save_cache(&self) -> Result<(), CacheError> {
        if let Some(parent) = self.cache_path.parent() {
            match fs::create_dir_all(parent) {
                Ok(_) => {},
                Err(_) => {
                    return Err(CacheError::IoError);
                }
            };
        }
        self.cache.save_to(&self.cache_path)
    }

    pub fn print_debug_info(&self, verbose: bool) {
        if !verbose {
            return;
        }
        println!("Cache Info:");
        println!("  Input path: {:?}", self.input_path);
        println!("  Input hash: {}", self.input_hash);
        println!("  Input timestamp: {}", self.input_timestamp);
        println!("  Config hash: {}", self.config_hash);

        let changes = self.analyze_changes();
        println!("  Needs full rebuild: {}", changes.needs_full_rebuild);
        println!(
            "  Files to regenerate: {}",
            changes.files_to_regenerate.len()
        );

        if let Some(old_state) = self.cache.file_states.get(&self.input_path) {
            println!("Found in cache:");
            println!("  Old hash: {}", old_state.hash);
            println!("  Old timestamp: {}", old_state.timestamp);
            println!("  Hash match: {}", old_state.hash == self.input_hash);
            println!(
                "  Timestamp match: {}",
                old_state.timestamp == self.input_timestamp
            );
        } else {
            println!("  NOT found in cache");
            println!("  Available cache keys:");
            for key in self.cache.file_states.keys() {
                println!("    - {:?}", key);
            }
        }

        if let Some(old_config) = self.cache.target_config_hashes.get("default") {
            println!("  Config hash match: {}", old_config == &self.config_hash);
        } else {
            println!("  No previous config hash found");
        }
    }
}
