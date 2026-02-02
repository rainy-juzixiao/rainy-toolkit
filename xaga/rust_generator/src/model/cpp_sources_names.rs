static CPP_SOURCE_EXTENSIONS: &'static [&'static str] = &[
    ".cc",
    ".cpp",
    ".cxx",
    ".c++",
];

pub fn is_cpp_source(file_name: &str) -> bool {
    CPP_SOURCE_EXTENSIONS.iter().any(|ext| file_name.ends_with(ext))
}