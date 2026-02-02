use clap::Parser;

#[derive(Parser, Debug)]
#[command(author, version, about, long_about = None)]
pub struct CommandArguments {
    /// 输入文件路径
    #[arg(short, long)]
    pub input: Option<String>,

    #[arg(short, long, action = clap::ArgAction::SetTrue)]
    pub verbose: bool,

    #[arg(long)]
    pub tea: bool,

    #[arg(long)]
    pub rain: bool,

    #[arg(short, long, default_value = "30")]
    pub rain_duration: u64,

    #[arg(long)]
    pub dev: bool,

    #[arg(short, long)]
    pub out: Option<String>,
}
