static CPP_HEADER_EXTENSIONS: &'static [&'static str] = &[
    ".h",
    ".hh",
    ".hpp",
    ".hxx",
    ".h++",
    ".inl",
];

pub fn is_cpp_header(file_name: &str) -> bool {
    CPP_HEADER_EXTENSIONS.iter().any(|ext| file_name.ends_with(ext))
}