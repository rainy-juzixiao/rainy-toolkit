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

mod file;
mod config;
mod hash;

pub use file::write_cpp_file;
pub use file::modify_filename_in_front;
pub use file::hash_file;
pub use hash::blake3_hash_bytes;
pub use hash::hash_generated_code;
pub use config::hash_build_config;