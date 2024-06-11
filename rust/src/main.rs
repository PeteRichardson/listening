use clap::Parser;
use std::collections::HashSet;
use std::error::Error;
use std::fmt;
use std::hash::{Hash, Hasher};
use std::process::Command;
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
    #[tabled(rename = "INADDR")]
    inaddr: String,
    action: String,
    #[tabled(skip)]
    full_command: String,
}

impl Listener {
    fn new(lsof_line: &str) -> Self {
        let splits: Vec<&str> = lsof_line.split_ascii_whitespace().collect();
        //println!("{:?}", splits);

        let (inaddr, port) = splits[8].split_once(':').expect("couldn't split");

        Self {
            command: splits[0].to_string().replace("\\x20", " "),
            port: port.parse::<u32>().unwrap(),
            pid: splits[1].parse::<u32>().unwrap(),
            fd: splits[3].to_string(),
            user: splits[2].to_string(),
            node: splits[7].to_string(),
            inaddr: inaddr.to_string(),
            action: "LISTEN".to_string(), // todo: trim parens off this value
            full_command: lsof_line.to_string(),
        }
    }
}

impl fmt::Display for Listener {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        write!(
            f,
            "{}:{}:{}:{}:{}:{}:{}:{}",
            self.command,
            self.port,
            self.pid,
            self.fd,
            self.user,
            self.node,
            self.inaddr,
            self.action
        )
    }
}
impl PartialEq for Listener {
    fn eq(&self, other: &Self) -> bool {
        self.port == other.port
    }
}
impl Eq for Listener {}
impl Hash for Listener {
    fn hash<H: Hasher>(&self, state: &mut H) {
        self.command.hash(state);
        self.pid.hash(state);
        self.port.hash(state);
        self.inaddr.hash(state);
    }
}

#[derive(Clone)]
struct ListenerList {
    listeners: Vec<Listener>,
}

impl ListenerList {
    fn new() -> Self {
        // create a temporary HashSet to dedup results
        let mut listeners_hash: HashSet<Listener> = HashSet::new();

        let lsof_lines = Command::new("lsof")
            .arg("-nP")
            .arg("+c")
            .arg("0")
            .arg("-i4")
            .output()
            .unwrap();
        let stdout = String::from_utf8(lsof_lines.stdout).expect("bad stdout from lsof command");

        //'lsof -nP +c 0 -i4' command returns lines like the following:
        //node                        10166 pete   31u  IPv4 0xe4ad34249b227fc5      0t0  TCP 127.0.0.1:45623 (LISTEN)
        for line in stdout.lines().skip(1).collect::<HashSet<_>>() {
            if line.ends_with("(LISTEN)") {
                listeners_hash.insert(Listener::new(line));
            }
        }
        let mut listener_vec: Vec<Listener> = listeners_hash.into_iter().collect();
        listener_vec.sort_by(|a, b| a.port.cmp(&b.port));
        Self {
            listeners: listener_vec,
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
