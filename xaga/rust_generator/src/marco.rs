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