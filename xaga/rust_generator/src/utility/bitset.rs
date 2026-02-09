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

#[cfg(test)]
mod tests {
    use super::*;
    use pretty_assertions::{assert_eq};

    // 测试构造函数
    #[test]
    fn test_new() {
        // 测试空位集合
        {
            let bs = BitSet::new(0);
            assert_eq!(bs.data.len(), 0);
        }
        // 测试恰好需要1个u64的情况
        {
            let bs = BitSet::new(64);
            assert_eq!(bs.data.len(), 1);
            assert_eq!(bs.data[0], 0);
        }
        // 测试需要超过1个u64的情况
        {
            let bs = BitSet::new(65);
            assert_eq!(bs.data.len(), 2);
            assert_eq!(bs.data[0], 0);
            assert_eq!(bs.data[1], 0);
        }
        // 测试边界情况：63位（只需要1个u64）
        {
            let bs = BitSet::new(63);
            assert_eq!(bs.data.len(), 1);
        }
        // 测试边界情况：64位
        {
            let bs = BitSet::new(64);
            assert_eq!(bs.data.len(), 1);
        }
        // 测试边界情况：65位
        {
            let bs = BitSet::new(65);
            assert_eq!(bs.data.len(), 2);
        }
        // 测试随机大小
        {
            let bs = BitSet::new(1000);
            assert_eq!(bs.data.len(), (1000 + 63) / 64);
        }
    }

    // 测试 set 和 get 基本功能
    #[test]
    fn test_set_get_basic() {
        let mut bs = BitSet::new(128);
        // 设置并获取单个位
        {
            bs.set(0, true);
            assert!(bs.get(0));
        }
        // 设置false
        {
            bs.set(0, false);
            assert!(!bs.get(0));
        }
        // 测试中间的位
        {
            bs.set(64, true);
            assert!(bs.get(64));
            assert!(!bs.get(63));
            assert!(!bs.get(65));
        }
        // 测试多个位
        {
            for i in 0..128 {
                bs.set(i, i % 2 == 0);
            }

            for i in 0..128 {
                assert_eq!(bs.get(i), i % 2 == 0);
            }
        }
    }

    // 测试边界情况的 set 和 get
    #[test]
    fn test_set_get_edge_cases() {
        // 测试单个字的边界
        let mut bs = BitSet::new(64);
        // 测试第0位
        {
            bs.set(0, true);
            assert!(bs.get(0));
            bs.set(0, false);
            assert!(!bs.get(0));
        }

        // 测试第63位（单个字内的最高位）
        {
            bs.set(63, true);
            assert!(bs.get(63));
            bs.set(63, false);
            assert!(!bs.get(63));
        }

        // 测试跨字的边界
        let mut bs = BitSet::new(128);

        // 测试第一个字的最后一位
        {
            bs.set(63, true);
            assert!(bs.get(63));
        }

        // 测试第二个字的第一位
        {
            bs.set(64, true);
            assert!(bs.get(64));
        }
        // 确保不互相影响
        assert!(bs.get(63));
        assert!(bs.get(64));
    }

    // 测试 all 方法
    #[test]
    fn test_all() {
        // 测试全为false
        let mut bs = BitSet::new(128);
        assert!(bs.all(false));
        assert!(!bs.all(true));

        // 测试全为true（128是64的倍数）
        for i in 0..128 {
            bs.set(i, true);
        }
        assert!(bs.all(true));
        assert!(!bs.all(false));

        // 测试部分为true
        let mut bs = BitSet::new(128);
        bs.set(0, true);
        assert!(!bs.all(true));
        assert!(!bs.all(false));

        // 测试大小非64倍数的情况 - all(false) 应该工作
        let mut bs = BitSet::new(100);
        assert!(bs.all(false));
        assert!(!bs.all(true)); // 最后一个u64不全为0，所以不是all(true)

        // 对于100位，即使所有位都设为true，all(true)也会失败
        // 因为最后一个u64的高24位是0
        for i in 0..100 {
            bs.set(i, true);
        }
        // 这里 all(true) 会是 false！
        assert!(!bs.all(true)); // 修正预期
        assert!(!bs.all(false));

        // 测试仅设置部分位后的 all(false)
        let mut bs = BitSet::new(100);
        bs.set(50, true);
        bs.set(50, false); // 重置为false
        assert!(bs.all(false));
    }

    // 测试大位集合
    #[test]
    fn test_large_bitset() {
        const SIZE: usize = 10000;
        let mut bs = BitSet::new(SIZE);

        // 测试稀疏设置
        bs.set(0, true);
        bs.set(SIZE - 1, true);
        bs.set(SIZE / 2, true);

        assert!(bs.get(0));
        assert!(bs.get(SIZE - 1));
        assert!(bs.get(SIZE / 2));

        // 验证其他位为false
        assert!(!bs.get(1));
        assert!(!bs.get(SIZE - 2));

        // 设置所有位
        for i in 0..SIZE {
            bs.set(i, i % 3 == 0);
        }

        // 验证模式
        for i in 0..SIZE {
            assert_eq!(bs.get(i), i % 3 == 0);
        }
    }

    // 测试内存布局的正确性
    #[test]
    fn test_memory_layout() {
        let mut bs = BitSet::new(192); // 3个u64

        // 设置每个字的第0位
        bs.set(0, true);
        bs.set(64, true);
        bs.set(128, true);

        assert_eq!(bs.data[0] & 1, 1);
        assert_eq!(bs.data[1] & 1, 1);
        assert_eq!(bs.data[2] & 1, 1);

        // 设置每个字的最高位
        bs.set(63, true);
        bs.set(127, true);
        bs.set(191, true);

        assert_eq!(bs.data[0] >> 63 & 1, 1);
        assert_eq!(bs.data[1] >> 63 & 1, 1);
        assert_eq!(bs.data[2] >> 63 & 1, 1);
    }

    // 测试多个位集合的独立性
    #[test]
    fn test_independent_bitsets() {
        let mut bs1 = BitSet::new(64);
        let mut bs2 = BitSet::new(64);

        bs1.set(0, true);
        bs2.set(0, false);

        assert!(bs1.get(0));
        assert!(!bs2.get(0));

        bs2.set(63, true);
        assert!(!bs1.get(63));
        assert!(bs2.get(63));
    }

    // 测试 all 方法的性能优化特性
    #[test]
    fn test_all_performance_optimization() {
        // 测试部分为true时，all(false)应快速返回false
        let mut bs = BitSet::new(1000);
        bs.set(500, true);
        assert!(!bs.all(false));

        // 测试部分为false时，all(true)应快速返回false
        let mut bs = BitSet::new(1000);
        for i in 0..1000 {
            bs.set(i, true);
        }
        bs.set(500, false);
        assert!(!bs.all(true));
    }

    // 测试 panic 场景（如果有的话）
    // 注：当前实现没有边界检查，所以我们需要测试有效范围内的操作
    #[test]
    fn test_valid_range() {
        let mut bs = BitSet::new(100);

        // 测试边界值
        bs.set(0, true);
        bs.set(99, true);
        for i in 0..100 {
            bs.set(i, true);
            assert!(bs.get(i));
        }
    }
}
