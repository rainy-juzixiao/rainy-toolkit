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

use crate::model::incremental::{BuildChanges, FileState, IncrementalCache};

#[derive(Debug)]
pub struct IncrementalBuilder<'a> {
    pub cache: &'a IncrementalCache,
    pub input_path: &'a std::path::Path,
    pub current_file_state: &'a FileState,
    pub target_name: &'a str,
    pub current_target_config_hash: &'a str,
}

impl<'a> IncrementalBuilder<'a> {
    pub fn analyze(&self) -> BuildChanges {
        // 检查 target 配置是否变化
        let needs_full_rebuild = match self.cache.target_config_hashes.get(self.target_name) {
            Some(old_hash) => old_hash != self.current_target_config_hash,
            None => true,
        };

        if needs_full_rebuild {
            return BuildChanges {
                needs_full_rebuild: true,
                files_to_regenerate: vec![self.input_path.to_path_buf()],
                stale_outputs: self.cache.output_mapping.keys().cloned().collect(),
            };
        }

        // 检查单输入是否变化
        let mut files_to_regenerate = Vec::new();
        let mut stale_outputs = Vec::new();

        match self.cache.file_states.get(self.input_path) {
            Some(old_state) => {
                if old_state.hash != self.current_file_state.hash
                    || old_state.timestamp != self.current_file_state.timestamp
                {
                    files_to_regenerate.push(self.input_path.to_path_buf());
                }
            }
            None => {
                files_to_regenerate.push(self.input_path.to_path_buf());
            }
        }

        // 删除失效输出
        for (output, source) in &self.cache.output_mapping {
            if source != self.input_path {
                continue;
            }
            if self.current_file_state.hash != self.cache.file_states[source].hash {
                stale_outputs.push(output.clone());
            }
        }

        BuildChanges {
            needs_full_rebuild: false,
            files_to_regenerate,
            stale_outputs,
        }
    }
}
