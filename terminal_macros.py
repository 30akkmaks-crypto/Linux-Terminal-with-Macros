# terminal_macros.py
import os
import sys
import json
import subprocess
import readline
import atexit
from pathlib import Path

# Файл для хранения макросов
MACROS_FILE = os.path.expanduser("~/.terminal_macros.json")

class TerminalMacros:
    def __init__(self):
        self.macros = {}
        self.recording = None      # имя макроса, если идёт запись
        self.recorded_commands = []
        self.load_macros()

    def load_macros(self):
        if os.path.exists(MACROS_FILE):
            try:
                with open(MACROS_FILE, 'r') as f:
                    self.macros = json.load(f)
            except:
                self.macros = {}
        else:
            self.macros = {}

    def save_macros(self):
        with open(MACROS_FILE, 'w') as f:
            json.dump(self.macros, f, indent=2)

    def run_shell_command(self, cmd):
        """Выполняет команду в оболочке и выводит результат."""
        try:
            result = subprocess.run(cmd, shell=True, text=True, capture_output=True)
            if result.stdout:
                print(result.stdout, end='')
            if result.stderr:
                print(result.stderr, end='', file=sys.stderr)
            return result.returncode
        except Exception as e:
            print(f"Error: {e}", file=sys.stderr)
            return 1

    def handle_command(self, line):
        line = line.strip()
        if not line:
            return True

        # Обработка встроенных команд
        if line.startswith('record '):
            name = line[7:].strip()
            if not name:
                print("Usage: record <name>")
                return True
            if self.recording:
                print(f"Already recording macro '{self.recording}'. Use 'stop' to finish.")
                return True
            self.recording = name
            self.recorded_commands = []
            print(f"Recording macro '{name}'. Type 'stop' to finish.")
            return True

        if line == 'stop':
            if not self.recording:
                print("No macro is currently being recorded.")
                return True
            if self.recorded_commands:
                self.macros[self.recording] = self.recorded_commands
                self.save_macros()
                print(f"Macro '{self.recording}' recorded ({len(self.recorded_commands)} commands).")
            else:
                print(f"Macro '{self.recording}' is empty, not saved.")
            self.recording = None
            self.recorded_commands = []
            return True

        if line.startswith('play '):
            name = line[5:].strip()
            if not name:
                print("Usage: play <name>")
                return True
            if name not in self.macros:
                print(f"Macro '{name}' not found.")
                return True
            print(f"Executing macro '{name}'...")
            for cmd in self.macros[name]:
                print(f"> {cmd}")
                self.run_shell_command(cmd)
            return True

        if line == 'list':
            if not self.macros:
                print("No macros defined.")
            else:
                for name, cmds in self.macros.items():
                    print(f"{name} ({len(cmds)} commands)")
            return True

        if line.startswith('delete '):
            name = line[7:].strip()
            if not name:
                print("Usage: delete <name>")
                return True
            if name in self.macros:
                del self.macros[name]
                self.save_macros()
                print(f"Macro '{name}' deleted.")
            else:
                print(f"Macro '{name}' not found.")
            return True

        if line == 'help':
            print("Commands:")
            print("  record <name>   Start recording a macro")
            print("  stop            Stop recording")
            print("  play <name>     Execute a macro")
            print("  list            Show all macros")
            print("  delete <name>   Delete a macro")
            print("  help            Show this help")
            print("  exit            Exit the program")
            print("Any other command is executed in the shell.")
            return True

        if line == 'exit':
            return False

        # Если идёт запись, добавляем команду в макрос
        if self.recording is not None:
            self.recorded_commands.append(line)
            print(f"[recording] {line}")

        # Выполняем команду в оболочке
        self.run_shell_command(line)
        return True

    def run(self):
        print("Terminal Macros v1.0. Type 'help' for commands.")
        while True:
            try:
                line = input("> ")
                if not self.handle_command(line):
                    break
            except KeyboardInterrupt:
                print("\nUse 'exit' to quit.")
            except EOFError:
                break
        self.save_macros()

if __name__ == "__main__":
    app = TerminalMacros()
    app.run()
