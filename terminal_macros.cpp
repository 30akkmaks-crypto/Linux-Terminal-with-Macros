// terminal_macros.cpp
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <unistd.h>
#include <sys/wait.h>
#include <readline/readline.h>
#include <readline/history.h>

using namespace std;

const string MACROS_FILE = string(getenv("HOME")) + "/.terminal_macros.json";

class TerminalMacros {
public:
    TerminalMacros() {
        loadMacros();
    }

    void run() {
        cout << "Terminal Macros v1.0. Type 'help' for commands." << endl;
        char* line;
        while ((line = readline("> ")) != nullptr) {
            if (line && *line) {
                add_history(line);
                string cmd(line);
                if (!handleCommand(cmd)) {
                    free(line);
                    break;
                }
            }
            free(line);
        }
        saveMacros();
    }

private:
    map<string, vector<string>> macros;
    string recording;
    vector<string> recordedCmds;

    void loadMacros() {
        ifstream file(MACROS_FILE);
        if (!file.is_open()) return;
        // Простой парсинг JSON не реализуем, для демонстрации оставим пустым.
        // В реальном проекте стоит использовать библиотеку типа nlohmann/json.
        // Здесь оставим заглушку: загружаем сохранённые макросы из простого текстового формата.
        // Для простоты оставим макросы в памяти.
    }

    void saveMacros() {
        // В реальном проекте использовать json-библиотеку.
        // Для демонстрации сохраняем в простом формате.
        ofstream file(MACROS_FILE);
        if (!file) return;
        file << "{";
        for (auto it = macros.begin(); it != macros.end(); ++it) {
            if (it != macros.begin()) file << ",";
            file << "\"" << it->first << "\": [";
            for (size_t i = 0; i < it->second.size(); ++i) {
                if (i) file << ",";
                file << "\"" << it->second[i] << "\"";
            }
            file << "]";
        }
        file << "}";
    }

    int runShell(const string& cmd) {
        pid_t pid = fork();
        if (pid == 0) {
            // child
            execl("/bin/sh", "sh", "-c", cmd.c_str(), nullptr);
            exit(1);
        } else if (pid > 0) {
            int status;
            waitpid(pid, &status, 0);
            return WEXITSTATUS(status);
        }
        return -1;
    }

    bool handleCommand(const string& line) {
        string trimmed = line;
        // trim
        size_t start = trimmed.find_first_not_of(" \t");
        if (start == string::npos) return true;
        trimmed = trimmed.substr(start);
        start = trimmed.find_last_not_of(" \t");
        if (start != string::npos) trimmed = trimmed.substr(0, start+1);
        if (trimmed.empty()) return true;

        vector<string> parts;
        stringstream ss(trimmed);
        string part;
        while (ss >> part) parts.push_back(part);
        string cmd = parts[0];
        vector<string> args(parts.begin()+1, parts.end());

        if (cmd == "record") {
            if (args.empty()) {
                cout << "Usage: record <name>" << endl;
                return true;
            }
            string name = args[0];
            if (!recording.empty()) {
                cout << "Already recording macro '" << recording << "'. Use 'stop' to finish." << endl;
                return true;
            }
            recording = name;
            recordedCmds.clear();
            cout << "Recording macro '" << name << "'. Type 'stop' to finish." << endl;
            return true;
        }

        if (cmd == "stop") {
            if (recording.empty()) {
                cout << "No macro is currently being recorded." << endl;
                return true;
            }
            if (!recordedCmds.empty()) {
                macros[recording] = recordedCmds;
                saveMacros();
                cout << "Macro '" << recording << "' recorded (" << recordedCmds.size() << " commands)." << endl;
            } else {
                cout << "Macro '" << recording << "' is empty, not saved." << endl;
            }
            recording.clear();
            recordedCmds.clear();
            return true;
        }

        if (cmd == "play") {
            if (args.empty()) {
                cout << "Usage: play <name>" << endl;
                return true;
            }
            string name = args[0];
            if (macros.find(name) == macros.end()) {
                cout << "Macro '" << name << "' not found." << endl;
                return true;
            }
            cout << "Executing macro '" << name << "'..." << endl;
            for (const string& c : macros[name]) {
                cout << "> " << c << endl;
                runShell(c);
            }
            return true;
        }

        if (cmd == "list") {
            if (macros.empty()) {
                cout << "No macros defined." << endl;
            } else {
                for (const auto& p : macros) {
                    cout << p.first << " (" << p.second.size() << " commands)" << endl;
                }
            }
            return true;
        }

        if (cmd == "delete") {
            if (args.empty()) {
                cout << "Usage: delete <name>" << endl;
                return true;
            }
            string name = args[0];
            if (macros.erase(name) > 0) {
                saveMacros();
                cout << "Macro '" << name << "' deleted." << endl;
            } else {
                cout << "Macro '" << name << "' not found." << endl;
            }
            return true;
        }

        if (cmd == "help") {
            cout << "Commands:" << endl;
            cout << "  record <name>   Start recording a macro" << endl;
            cout << "  stop            Stop recording" << endl;
            cout << "  play <name>     Execute a macro" << endl;
            cout << "  list            Show all macros" << endl;
            cout << "  delete <name>   Delete a macro" << endl;
            cout << "  help            Show this help" << endl;
            cout << "  exit            Exit the program" << endl;
            cout << "Any other command is executed in the shell." << endl;
            return true;
        }

        if (cmd == "exit") {
            return false;
        }

        // Обычная команда
        if (!recording.empty()) {
            recordedCmds.push_back(trimmed);
            cout << "[recording] " << trimmed << endl;
        }
        runShell(trimmed);
        return true;
    }
};

int main() {
    TerminalMacros tm;
    tm.run();
    return 0;
}
