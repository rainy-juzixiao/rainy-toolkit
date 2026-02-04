use std::env;
use std::fs;
use std::path::{Path, PathBuf};
use std::process::Command;

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

fn git_branch() -> Option<String> {
    let output = Command::new("git")
        .args(["rev-parse", "--abbrev-ref", "HEAD"])
        .output()
        .ok()?;

    if !output.status.success() {
        return None;
    }
    let branch = String::from_utf8(output.stdout).ok()?;
    Some(branch.trim().to_string())
}

fn main() {
    println!("cargo:rerun-if-changed=*");

    let out_dir = PathBuf::from(env::var("OUT_DIR").unwrap());

    // OUT_DIR = target/{debug|release}/build/xxx/out
    let exe_dir = out_dir
        .parent()
        .unwrap() // xxx
        .parent()
        .unwrap() // build
        .parent()
        .unwrap(); // debug / release

    let manifest_dir = PathBuf::from(env::var("CARGO_MANIFEST_DIR").unwrap());

    let from = manifest_dir.join("statics");
    let to = exe_dir.join("statics");

    copy_dir_recursive(&from, &to);

    println!("cargo:warning=Copied {:?} to {:?}", from, to);

    {
        let git_hash = Command::new("git")
            .args(["rev-parse", "--short", "HEAD"])
            .output()
            .ok()
            .and_then(|o| String::from_utf8(o.stdout).ok())
            .unwrap_or_else(|| "unknown".into());
        println!("cargo:rustc-env=GIT_HASH={}", git_hash.trim());
    }

    {
        let build_time = chrono::Utc::now().to_rfc3339();
        println!("cargo:rustc-env=BUILD_TIME={}", build_time);
    }

    {
        let branch = git_branch();
        println!("cargo:rustc-env=GIT_BRANCH={}", branch.unwrap_or_else(|| "unknown".into()));
    }

}
