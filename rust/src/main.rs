use clap::Parser;
use std::error::Error;
use tabled::{
    settings::{object::Columns, themes::Colorization, Alignment, Color, Style},
    Table, Tabled,
};
#[derive(Parser, Debug, Clone)]
#[command(version, about)]
struct Config {
    #[clap(long, short, action)]
    commands: bool,
}

#[derive(Tabled, Default, Debug, Clone)]
#[tabled(rename_all = "PascalCase")]
struct Listener {
    command: String,
    port: u32,
    #[tabled(rename = "PID")]
    pid: u32,
    #[tabled(rename = "FD")]
    fd: String,
    user: String,
    node: String,
    inaddr: String,
    #[tabled(rename = "INADDR")]
    action: String,
    #[tabled(skip)]
    full_command: String,
}

impl Listener {
    fn new(lsof_line: String) -> Self {
        Self {
            command: lsof_line.clone(),
            port: 5000,
            pid: 716,
            fd: String::from("10u"),
            user: String::from("pete"),
            node: String::from("TCP"),
            inaddr: String::from("127.0.0.1"),
            action: String::from("LISTEN"),
            full_command: lsof_line.clone(),
        }
    }
}

#[derive(Clone)]
struct ListenerList {
    listeners: Vec<Listener>,
}

impl ListenerList {
    fn new() -> Self {
        Self {
            listeners: vec![
                Listener::new("Stream Deck".to_string()),
                Listener::new("node".to_string()),
            ],
        }
    }
}

fn print_table(list: &ListenerList) -> Result<(), Box<dyn Error>> {
    let mut table = Table::new(list.listeners.clone());
    table
        .with(Style::rounded())
        .with(Colorization::columns([
            Color::FG_WHITE,
            Color::FG_YELLOW,
            Color::FG_GREEN,
            Color::FG_BRIGHT_BLUE,
            Color::FG_BRIGHT_MAGENTA,
        ]))
        .modify(Columns::new(3..4), Alignment::right());

    println!("{}", table);
    Ok(())
}

fn main() -> Result<(), Box<dyn std::error::Error>> {
    let config = Config::parse();

    let listeners = ListenerList::new();
    print_table(&listeners).expect("Failed to output listeners table");

    if config.commands {
        for listener in listeners.listeners {
            println!("{}", listener.full_command);
        }
    };
    Ok(())
}
