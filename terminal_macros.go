// terminal_macros.go
package main

import (
	"bufio"
	"encoding/json"
	"fmt"
	"io"
	"os"
	"os/exec"
	"strings"
)

const macrosFile = ".terminal_macros.json"

type TerminalMacros struct {
	macros          map[string][]string
	recording       string
	recordedCmds    []string
}

func NewTerminalMacros() *TerminalMacros {
	tm := &TerminalMacros{
		macros: make(map[string][]string),
	}
	tm.loadMacros()
	return tm
}

func (tm *TerminalMacros) loadMacros() {
	file, err := os.OpenFile(macrosFile, os.O_RDONLY|os.O_CREATE, 0644)
	if err != nil {
		return
	}
	defer file.Close()
	data, _ := io.ReadAll(file)
	if len(data) > 0 {
		json.Unmarshal(data, &tm.macros)
	}
}

func (tm *TerminalMacros) saveMacros() {
	data, _ := json.MarshalIndent(tm.macros, "", "  ")
	os.WriteFile(macrosFile, data, 0644)
}

func (tm *TerminalMacros) runShell(cmd string) {
	// Выполняем через оболочку
	var shell, flag string
	if _, err := os.Stat("/bin/bash"); err == nil {
		shell = "/bin/bash"
		flag = "-c"
	} else if _, err := os.Stat("/bin/sh"); err == nil {
		shell = "/bin/sh"
		flag = "-c"
	} else {
		// Windows fallback
		shell = "cmd"
		flag = "/c"
	}
	c := exec.Command(shell, flag, cmd)
	c.Stdout = os.Stdout
	c.Stderr = os.Stderr
	c.Run()
}

func (tm *TerminalMacros) handleCommand(line string) bool {
	line = strings.TrimSpace(line)
	if line == "" {
		return true
	}
	parts := strings.Fields(line)
	if len(parts) == 0 {
		return true
	}
	cmd := parts[0]
	args := parts[1:]

	switch cmd {
	case "record":
		if len(args) == 0 {
			fmt.Println("Usage: record <name>")
			return true
		}
		name := args[0]
		if tm.recording != "" {
			fmt.Printf("Already recording macro '%s'. Use 'stop' to finish.\n", tm.recording)
			return true
		}
		tm.recording = name
		tm.recordedCmds = []string{}
		fmt.Printf("Recording macro '%s'. Type 'stop' to finish.\n", name)
		return true

	case "stop":
		if tm.recording == "" {
			fmt.Println("No macro is currently being recorded.")
			return true
		}
		if len(tm.recordedCmds) > 0 {
			tm.macros[tm.recording] = tm.recordedCmds
			tm.saveMacros()
			fmt.Printf("Macro '%s' recorded (%d commands).\n", tm.recording, len(tm.recordedCmds))
		} else {
			fmt.Printf("Macro '%s' is empty, not saved.\n", tm.recording)
		}
		tm.recording = ""
		tm.recordedCmds = nil
		return true

	case "play":
		if len(args) == 0 {
			fmt.Println("Usage: play <name>")
			return true
		}
		name := args[0]
		cmds, ok := tm.macros[name]
		if !ok {
			fmt.Printf("Macro '%s' not found.\n", name)
			return true
		}
		fmt.Printf("Executing macro '%s'...\n", name)
		for _, c := range cmds {
			fmt.Printf("> %s\n", c)
			tm.runShell(c)
		}
		return true

	case "list":
		if len(tm.macros) == 0 {
			fmt.Println("No macros defined.")
		} else {
			for name, cmds := range tm.macros {
				fmt.Printf("%s (%d commands)\n", name, len(cmds))
			}
		}
		return true

	case "delete":
		if len(args) == 0 {
			fmt.Println("Usage: delete <name>")
			return true
		}
		name := args[0]
		if _, ok := tm.macros[name]; ok {
			delete(tm.macros, name)
			tm.saveMacros()
			fmt.Printf("Macro '%s' deleted.\n", name)
		} else {
			fmt.Printf("Macro '%s' not found.\n", name)
		}
		return true

	case "help":
		fmt.Println("Commands:")
		fmt.Println("  record <name>   Start recording a macro")
		fmt.Println("  stop            Stop recording")
		fmt.Println("  play <name>     Execute a macro")
		fmt.Println("  list            Show all macros")
		fmt.Println("  delete <name>   Delete a macro")
		fmt.Println("  help            Show this help")
		fmt.Println("  exit            Exit the program")
		fmt.Println("Any other command is executed in the shell.")
		return true

	case "exit":
		return false

	default:
		// Если идёт запись, добавляем в макрос
		if tm.recording != "" {
			tm.recordedCmds = append(tm.recordedCmds, line)
			fmt.Printf("[recording] %s\n", line)
		}
		// Выполняем команду
		tm.runShell(line)
		return true
	}
}

func main() {
	tm := NewTerminalMacros()
	fmt.Println("Terminal Macros v1.0. Type 'help' for commands.")
	scanner := bufio.NewScanner(os.Stdin)
	for scanner.Scan() {
		line := scanner.Text()
		if !tm.handleCommand(line) {
			break
		}
	}
	tm.saveMacros()
}
