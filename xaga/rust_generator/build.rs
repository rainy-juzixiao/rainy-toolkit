use std::env;
use std::fs;
use std::path::{Path, PathBuf};

fn copy_dir_recursive(from: &Path, to: &Path) {
    if !to.exists() {
        fs::create_dir_all(to).unwrap();
    }

    for entry in fs::read_dir(from).unwrap() {
        let entry = entry.unwrap();
        let file_type = entry.file_type().unwrap();
        let from_path = entry.path();
        let to_path = to.join(entry.file_name());

        if file_type.is_dir() {
            copy_dir_recursive(&from_path, &to_path);
        } else if file_type.is_file() {
            fs::copy(&from_path, &to_path).unwrap();
        }
    }
}

fn main() {
    let out_dir = PathBuf::from(env::var("OUT_DIR").unwrap());

    // OUT_DIR = target/{debug|release}/build/xxx/out
    let exe_dir = out_dir
        .parent().unwrap() // xxx
        .parent().unwrap() // build
        .parent().unwrap(); // debug / release

    let manifest_dir = PathBuf::from(env::var("CARGO_MANIFEST_DIR").unwrap());

    let from = manifest_dir.join("statics");
    let to = exe_dir.join("statics");

    copy_dir_recursive(&from, &to);

    println!("cargo:warning=Copied {:?} to {:?}", from, to);
}
