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

#[macro_export]
macro_rules! include_statics {
    ($path:literal) => {
        include_str!(concat!(
            env!("CARGO_MANIFEST_DIR"),
            "/statics/",
            $path
        ))
    };
}

#[macro_export]
macro_rules! locate_runtime_resources {
    ($path:literal) => {{
        let exe_dir = std::env::current_exe()
            .ok()
            .and_then(|p| p.parent().map(|d| d.to_path_buf()));
        match exe_dir {
            Some(dir) => dir.join("statics").join($path).to_string_lossy().into_owned(),
            None => String::new(),
        }
    }};

    ($($arg:tt)*) => {{
        let formatted_path = format!($($arg)*);
        let exe_dir = std::env::current_exe()
            .ok()
            .and_then(|p| p.parent().map(|d| d.to_path_buf()));
        match exe_dir {
            Some(dir) => dir.join("statics").join(formatted_path).to_string_lossy().into_owned(),
            None => String::new(),
        }
    }};
}

#[macro_export]
macro_rules! locate_res_path {
    ($path:literal) => {{
        let resource_path = option_env!("RESOURCE_PATH");
        match resource_path {
            Some(dir) => std::path::Path::new(dir)
                .join("statics")
                .join($path)
                .to_string_lossy()
                .into_owned(),
            None => String::new(),
        }
    }};

    ($($arg:tt)*) => {{
        let formatted_path = format!($($arg)*);
        let resource_path = option_env!("RESOURCE_PATH");
        match resource_path {
            Some(dir) => std::path::Path::new(dir)
                .join("statics")
                .join(formatted_path)
                .to_string_lossy()
                .into_owned(),
            None => String::new(),
        }
    }};
}
