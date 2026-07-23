use clap::Parser;
use std::collections::HashSet;
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
    /// Show full command for each PID
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
    fn new(lsof_line: &str, resolve_full_command: bool) -> Option<Self> {
        let splits: Vec<&str> = lsof_line.split_ascii_whitespace().collect();
        //println!("{:?}", splits);

        let pid = splits.get(1)?.parse::<u32>().ok()?;

        let (inaddr, port) = splits.get(8)?.split_once(':')?;

        Some(Self {
            command: splits.get(0)?.to_string().replace("\\x20", " "),
            port: port.parse::<u32>().ok()?,
            pid,
            fd: splits.get(3)?.to_string(),
            user: splits.get(2)?.to_string(),
            node: splits.get(7)?.to_string(),
            inaddr: inaddr.to_string(),
            action: "LISTEN".to_string(),
            full_command: if resolve_full_command {
                Listener::get_full_command(pid)
            } else {
                String::new()
            },
        })
    }

    fn get_full_command(pid: u32) -> String {
        let pid = pid.to_string();
        let ports = Command::new("ps")
            .args(["-p", &pid, "-o", "command=", "-w", "-w"])
            .output()
            .unwrap();

        let stdout = String::from_utf8(ports.stdout).expect("bad stdout from echo");
        stdout.trim().to_string()
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
        self.command == other.command
            && self.pid == other.pid
            && self.port == other.port
            && self.inaddr == other.inaddr
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
struct ListenerHash {
    listeners: HashSet<Listener>,
}

impl ListenerHash {
    fn new(resolve_full_command: bool) -> Self {
        // create a temporary HashSet to dedup results
        let mut listeners_hash: HashSet<Listener> = HashSet::new();

        let lsof_lines = Command::new("/usr/sbin/lsof")
            .arg("-nP")
            .arg("+c")
            .arg("0")
            .arg("-i4")
            .output()
            .unwrap();
        let stdout = String::from_utf8(lsof_lines.stdout).expect("bad stdout from lsof command");

        //'lsof -nP +c 0 -i4' command returns lines like the following:
        //node                        10166 pete   31u  IPv4 0xe4ad34249b227fc5      0t0  TCP 127.0.0.1:45623 (LISTEN)
        for line in stdout.lines().collect::<HashSet<_>>() {
            if line.ends_with("(LISTEN)") {
                if let Some(listener) = Listener::new(line, resolve_full_command) {
                    listeners_hash.insert(listener);
                }
            }
        }
        Self {
            listeners: listeners_hash,
        }
    }
}

fn print_table(list: &ListenerHash) {
    let mut listener_vec: Vec<Listener> = list.listeners.clone().into_iter().collect();
    listener_vec.sort_by_key(|l| l.port);
    let mut table = Table::new(listener_vec);
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
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn parses_basic_fields() {
        let line = "node                        10166 pete   31u  IPv4 0xe4ad34249b227fc5      0t0  TCP 127.0.0.1:45623 (LISTEN)";
        let l = Listener::new(line, false).unwrap();
        assert_eq!(l.command, "node");
        assert_eq!(l.pid, 10166);
        assert_eq!(l.user, "pete");
        assert_eq!(l.fd, "31u");
        assert_eq!(l.node, "TCP");
        assert_eq!(l.inaddr, "127.0.0.1");
        assert_eq!(l.port, 45623);
        assert_eq!(l.action, "LISTEN");
    }

    #[test]
    fn unescapes_spaces_in_command() {
        let line = "Adobe\\x20Desktop\\x20Service  1102 pete   10u  IPv4 0x6c6607cf201365e5      0t0  TCP 127.0.0.1:15292 (LISTEN)";
        let l = Listener::new(line, false).unwrap();
        assert_eq!(l.command, "Adobe Desktop Service");
        assert_eq!(l.port, 15292);
    }
}

fn main() {
    let config = Config::parse();

    let listeners = ListenerHash::new(config.commands);
    print_table(&listeners);

    if config.commands {
        // sort listeners by PID, dedup on PID, and print out full_commands
        let mut listener_vec: Vec<Listener> = listeners.listeners.into_iter().collect();
        listener_vec.sort_by_key(|l| l.pid);
        listener_vec.dedup_by(|a, b| a.pid == b.pid);
        for listener in listener_vec {
            println!("{}: {}", listener.pid, listener.full_command);
        }
    };
}
