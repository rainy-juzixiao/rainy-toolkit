use anyhow::{anyhow, Result};
use toml::Value;

pub fn convert_toml_to_fluent(toml_str: &str) -> Result<String> {
    let value: Value = toml::from_str(toml_str).map_err(|e| anyhow!("Invalid TOML: {e}"))?;
    let table = value
        .as_table()
        .ok_or_else(|| anyhow!("TOML root is not a table"))?;

    let mut out = String::new();
    walk_table(table, None, &mut out);
    Ok(out)
}

fn walk_table(table: &toml::value::Table, prefix: Option<&str>, out: &mut String) {
    for (key, val) in table {
        let full_key = match prefix {
            Some(p) => format!("{p}-{key}"),
            None => key.clone(),
        };
        match val {
            Value::String(s) => {
                let escaped = escape_fluent_string(s);
                out.push_str(&format!("{full_key} = {escaped}\n"));
            }
            Value::Table(sub) => {
                walk_table(sub, Some(&full_key), out);
            }
            Value::Array(arr) => {
                for item in arr {
                    if let Value::Table(sub) = item {
                        walk_table(sub, Some(&full_key), out);
                    }
                }
            }
            _ => {}
        }
    }
}

fn escape_fluent_string(s: &str) -> String {
    s.replace('\\', "\\\\")
        .replace('"', "\\\"")
        .replace('\n', "\\n")
}
