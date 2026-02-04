use blake3::Hasher;

pub fn blake3_hash_bytes(data: &[u8]) -> String {
    let mut hasher = Hasher::new();
    hasher.update(data);
    hasher.finalize().to_hex().to_string()
}

pub fn hash_generated_code(code: &str) -> String {
    blake3_hash_bytes(code.as_bytes())
}