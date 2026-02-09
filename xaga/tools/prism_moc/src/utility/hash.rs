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

use blake3::Hasher;

pub fn blake3_hash_bytes(data: &[u8]) -> String {
    let mut hasher = Hasher::new();
    hasher.update(data);
    hasher.finalize().to_hex().to_string()
}

pub fn hash_generated_code(code: &str) -> String {
    blake3_hash_bytes(code.as_bytes())
}