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

pub struct BitSet {
    data: Vec<u64>,
}

impl BitSet {
    pub fn new(bits: usize) -> Self {
        Self {
            data: vec![0; (bits + 63) / 64],
        }
    }

    pub fn set(&mut self, i: usize, value: bool) {
        let word = i / 64;
        let bit = i % 64;
        if value {
            self.data[word] |= 1 << bit;
        } else {
            self.data[word] &= !(1 << bit);
        }
    }

    pub fn get(&self, i: usize) -> bool {
        (self.data[i / 64] >> (i % 64)) & 1 == 1
    }

    pub fn all(&self, value: bool) -> bool {
        let target = if value { !0 } else { 0 };
        self.data.iter().all(|&word| word == target)
    }
}
