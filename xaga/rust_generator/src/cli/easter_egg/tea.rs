use colored::*;
use std::io::{self, Write};
use std::time::Duration;
use rand::Rng;
use tokio::time::sleep;
use indicatif::{ProgressBar, ProgressStyle};

const TEA_WISDOM: &[&str] = &[
    "A good programmer is like a good tea - strong, clear, and refreshing.",
    "Bugs are like tea leaves - sometimes you need to wait for them to settle.",
    "Compiling... meanwhile, let's enjoy the moment.",
    "The code will compile when it's ready. Just like tea.",
    "Patience is bitter, but its fruit is sweet. - Aristotle",
    "静坐听雨，闲坐品茶。",
    "Sometimes the best solution is to step away and have a cup of tea.",
    "The water boils, the code compiles. All in good time.",
];

pub async fn tea_ceremony() {
    println!("\n{}", "☕️  Tea Break Initiated ☕️".bold().green());
    println!("{}", "━".repeat(40).blue());
    let steps = vec![
        ("Boiling water...", 4),
        ("Selecting tea leaves...", 3),
        ("Warming the teapot...", 2),
        ("Steeping tea...", 8),
        ("Pouring tea...", 2),
        ("Letting it cool...", 3),
    ];
    for (desc, secs) in steps {
        let pb = ProgressBar::new(secs);
        pb.set_style(
            ProgressStyle::default_bar()
                .template("{spinner:.green} {msg} [{elapsed_precise}] {bar:40.cyan/blue}")
                .unwrap()
                .progress_chars("█▓▒░"),
        );
        pb.set_message(desc.to_string());

        for _ in 0..secs {
            pb.inc(1);
            sleep(Duration::from_secs(1)).await;
        }
        pb.finish_with_message(format!("✓ {}", desc.replace("...", "")));
    }
    println!("\n{}", "✨ Tea is ready! ✨".bold().yellow());
    let wisdom = TEA_WISDOM[rand::thread_rng().gen_range(0..TEA_WISDOM.len())];
    println!("\n💭 {}", wisdom.italic().cyan());
    println!("\n{}", "Take a deep breath and enjoy your tea.".green());
    println!("{}", "Resuming normal operations in 3 seconds...".dimmed());

    for i in (1..=3).rev() {
        print!("\r⏳ {}... ", i);
        io::stdout().flush().unwrap();
        sleep(Duration::from_secs(1)).await;
    }
    println!("\n✅ Time to back home!\n");
}