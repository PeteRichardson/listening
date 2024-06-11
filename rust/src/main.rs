use clap::Parser;
#[derive(Parser, Debug, Clone)]
#[command(version, about)]
struct Config {
    #[clap(long, short, action)]
    commands: bool,
}
fn main() -> Result<(), Box<dyn std::error::Error>> {
    let config = Config::parse();
    println!("welcome to listening");
    if config.commands {
        println!("\tprinting commands");
    };
    Ok(())
}
