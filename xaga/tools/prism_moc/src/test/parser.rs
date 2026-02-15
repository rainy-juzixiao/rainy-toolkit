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

#[cfg(test)]
mod tests {
    use crate::include_test_set;
    use crate::parser::parse_cpp;
    use crate::test::client_arguments::default_cli;

    const TEST_SOURCE: &str = include_test_set!("parser/example.cc");

    #[test]
    fn test_parse_cpp_classes_with_moc() {
        let cli = default_cli();
        let result = parse_cpp(TEST_SOURCE, &cli).unwrap();

        let class_names: Vec<_> = result
            .classes
            .iter()
            .map(|c| c.full_qual_name.as_str())
            .collect();

        // Foo / Bar 没有宏，不应出现
        assert!(!class_names.contains(&"Foo"));
        assert!(!class_names.contains(&"Bar"));

        // a / b 有宏
        assert!(class_names.contains(&"a"));
        assert!(class_names.contains(&"b"));

        // namespace 中的类
        assert!(class_names.contains(&"test_namespace::nest::c"));
    }

    #[test]
    fn test_class_macro_arguments() {
        let cli = default_cli();
        let result = parse_cpp(TEST_SOURCE, &cli).unwrap();

        let class_b = result.classes.iter().find(|c| c.name == "b").unwrap();

        assert_eq!(
            class_b.use_namespaces,
            vec!["xxx".to_string(), "x1xx".to_string()]
        );

        assert_eq!(class_b.use_items, vec!["xxx".to_string()]);
    }

    #[test]
    fn test_global_functions_with_moc() {
        let cli = default_cli();
        let result = parse_cpp(TEST_SOURCE, &cli).unwrap();

        let func_names: Vec<_> = result
            .global_functions
            .iter()
            .map(|f| f.full_qual_name.as_str())
            .collect();

        assert!(func_names.contains(&"function"));
        assert!(func_names.contains(&"function1"));
        assert!(func_names.contains(&"function2"));
    }

    #[test]
    fn test_global_function_namespace_qual_name() {
        let cli = default_cli();
        let result = parse_cpp(TEST_SOURCE, &cli).unwrap();

        for f in &result.global_functions {
            // 确保 full_qual_name 构造逻辑不 panic
            assert!(!f.full_qual_name.is_empty());
        }
    }

    #[test]
    fn test_empty_moc_macro_arguments() {
        let cli = default_cli();
        let result = parse_cpp(TEST_SOURCE, &cli).unwrap();

        let class_a = result.classes.iter().find(|c| c.name == "a").unwrap();

        assert!(class_a.use_namespaces.is_empty());
        assert!(class_a.use_items.is_empty());
    }
}
