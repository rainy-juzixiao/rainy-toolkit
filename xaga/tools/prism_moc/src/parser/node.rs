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

use tree_sitter::Node;

pub fn find_node_by_range(node: Node, start: usize, end: usize) -> Option<Node> {
    if node.start_byte() == start && node.end_byte() == end {
        return Some(node);
    }

    for i in 0..node.child_count() {
        if let Some(child) = node.child(i as u32) {
            if let Some(found) = find_node_by_range(child, start, end) {
                return Some(found);
            }
        }
    }
    None
}

pub fn find_param_identifier(node: Node) -> Option<Node> {
    if node.kind() == "identifier" {
        return Some(node);
    }
    for i in 0..node.child_count() {
        if let Some(child) = node.child(i as u32) {
            if let Some(id) = find_param_identifier(child) {
                return Some(id);
            }
        }
    }
    None
}
