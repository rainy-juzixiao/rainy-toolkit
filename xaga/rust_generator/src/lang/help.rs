use serde::Deserialize;

#[derive(Debug, Deserialize)]
struct Help {
    print_usage_header: String,
    output_options_desc: String
}