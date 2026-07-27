// terminal_macros.rs
use std::collections::HashMap;
use std::fs;
use std::io::{self, Write};
use std::process::{Command, Stdio};
use std::path::PathBuf;
use serde::{Serialize, Deserialize};
use rustyline::Editor;

const MACROS_FILE: &str = ".terminal_macros.json";

#[derive(Serialize, Deserialize, Default)]
struct MacrosStore {
    macros: HashMap<String, Vec<String>>,
}

struct TerminalMacros {
    store: MacrosStore,
    recording: Option<String>,
    recorded_cmds: Vec<String>,
}

impl TerminalMacros {
    fn new() -> Self {
        let mut tm = TerminalMacros {
            store: MacrosStore::default(),
            recording: None,
            recorded_cmds: Vec::new(),
        };
        tm.load_macros();
        tm
    }

    fn load_macros(&mut self) {
        let path = PathBuf::from(env!("HOME")).join(MACROS_FILE);
        if path.exists() {
            if let Ok(data) = fs::read_to_string(&path) {
                if let Ok(store) = serde_json::from_str(&data) {
                    self.store = store;
                }
            }
        }
    }

    fn save_macros(&self) {
        let path = PathBuf::from(env!("HOME")).join(MACROS_FILE);
        if let Ok(data) = serde_json::to_string_pretty(&self.store) {
            let _ = fs::write(path, data);
        }
    }

    fn run_shell(&self, cmd: &str) {
        let shell = if cfg!(windows) { "cmd" } else { "/bin/sh" };
        let flag = if cfg!(windows) { "/c" } else { "-c" };
        let status = Command::new(shell)
            .arg(flag)
            .arg(cmd)
            .stdout(Stdio::inherit())
            .stderr(Stdio::inherit())
            .status();
        if let Err(e) = status {
            eprintln!("Error executing command: {}", e);
        }
    }

    fn handle_command(&mut self, line: &str) -> bool {
        let line = line.trim();
        if line.is_empty() {
            return true;
        }
        let parts: Vec<&str> = line.split_whitespace().collect();
        if parts.is_empty() {
            return true;
        }
        let cmd = parts[0];
        let args = &parts[1..];

        match cmd {
            "record" => {
                if args.is_empty() {
                    println!("Usage: record <name>");
                    return true;
                }
                let name = args[0].to_string();
                if self.recording.is_some() {
                    println!("Already recording macro '{:?}'. Use 'stop' to finish.", self.recording);
                    return true;
                }
                self.recording = Some(name.clone());
                self.recorded_cmds.clear();
                println!("Recording macro '{}'. Type 'stop' to finish.", name);
                return true;
            }
            "stop" => {
                if let Some(name) = self.recording.take() {
                    if !self.recorded_cmds.is_empty() {
                        self.store.macros.insert(name.clone(), self.recorded_cmds.clone());
                        self.save_macros();
                        println!("Macro '{}' recorded ({} commands).", name, self.recorded_cmds.len());
                    } else {
                        println!("Macro '{}' is empty, not saved.", name);
                    }
                    self.recorded_cmds.clear();
                } else {
                    println!("No macro is currently being recorded.");
                }
                return true;
            }
            "play" => {
                if args.is_empty() {
                    println!("Usage: play <name>");
                    return true;
                }
                let name = args[0];
                if let Some(cmds) = self.store.macros.get(name) {
                    println!("Executing macro '{}'...", name);
                    for c in cmds {
                        println!("> {}", c);
                        self.run_shell(c);
                    }
                } else {
                    println!("Macro '{}' not found.", name);
                }
                return true;
            }
            "list" => {
                if self.store.macros.is_empty() {
                    println!("No macros defined.");
                } else {
                    for (name, cmds) in &self.store.macros {
                        println!("{} ({} commands)", name, cmds.len());
                    }
                }
                return true;
            }
            "delete" => {
                if args.is_empty() {
                    println!("Usage: delete <name>");
                    return true;
                }
                let name = args[0];
                if self.store.macros.remove(name).is_some() {
                    self.save_macros();
                    println!("Macro '{}' deleted.", name);
                } else {
                    println!("Macro '{}' not found.", name);
                }
                return true;
            }
            "help" => {
                println!("Commands:");
                println!("  record <name>   Start recording a macro");
                println!("  stop            Stop recording");
                println!("  play <name>     Execute a macro");
                println!("  list            Show all macros");
                println!("  delete <name>   Delete a macro");
                println!("  help            Show this help");
                println!("  exit            Exit the program");
                println!("Any other command is executed in the shell.");
                return true;
            }
            "exit" => {
                return false;
            }
            _ => {
                // Если идёт запись, добавляем в макрос
                if let Some(_) = &self.recording {
                    self.recorded_cmds.push(line.to_string());
                    println!("[recording] {}", line);
                }
                self.run_shell(line);
                return true;
            }
        }
    }

    fn run(&mut self) {
        println!("Terminal Macros v1.0. Type 'help' for commands.");
        let mut rl = Editor::<()>::new();
        loop {
            let readline = rl.readline("> ");
            match readline {
                Ok(line) => {
                    rl.add_history_entry(&line);
                    if !self.handle_command(&line) {
                        break;
                    }
                }
                Err(_) => break,
            }
        }
        self.save_macros();
    }
}

fn main() {
    let mut tm = TerminalMacros::new();
    tm.run();
}
