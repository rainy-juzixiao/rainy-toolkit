use crate::utility::{blake3_hash_bytes};

pub fn hash_build_config<T: serde::Serialize>(config: &T) -> String {
    let bytes = bincode::serialize(config).expect("build config must be serializable");
    blake3_hash_bytes(&bytes)
}