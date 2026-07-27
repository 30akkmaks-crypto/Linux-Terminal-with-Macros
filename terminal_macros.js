// terminal_macros.js
#!/usr/bin/env node
const fs = require('fs');
const path = require('path');
const os = require('os');
const readline = require('readline');
const { spawn, execSync } = require('child_process');

const MACROS_FILE = path.join(os.homedir(), '.terminal_macros.json');

class TerminalMacros {
    constructor() {
        this.macros = {};
        this.recording = null;
        this.recordedCmds = [];
        this.rl = readline.createInterface({
            input: process.stdin,
            output: process.stdout,
            prompt: '> '
        });
        this.loadMacros();
    }

    loadMacros() {
        try {
            const data = fs.readFileSync(MACROS_FILE, 'utf8');
            this.macros = JSON.parse(data);
        } catch (err) {
            this.macros = {};
        }
    }

    saveMacros() {
        fs.writeFileSync(MACROS_FILE, JSON.stringify(this.macros, null, 2));
    }

    runShell(cmd) {
        // Используем оболочку
        const shell = process.env.SHELL || (process.platform === 'win32' ? 'cmd' : '/bin/sh');
        const args = process.platform === 'win32' ? ['/c', cmd] : ['-c', cmd];
        const child = spawn(shell, args, { stdio: 'inherit' });
        return new Promise((resolve) => {
            child.on('close', resolve);
        });
    }

    async handleCommand(line) {
        line = line.trim();
        if (!line) return true;

        const parts = line.split(/\s+/);
        const cmd = parts[0];
        const args = parts.slice(1);

        switch (cmd) {
            case 'record':
                if (args.length === 0) {
                    console.log('Usage: record <name>');
                    return true;
                }
                const name = args[0];
                if (this.recording) {
                    console.log(`Already recording macro '${this.recording}'. Use 'stop' to finish.`);
                    return true;
                }
                this.recording = name;
                this.recordedCmds = [];
                console.log(`Recording macro '${name}'. Type 'stop' to finish.`);
                return true;

            case 'stop':
                if (!this.recording) {
                    console.log('No macro is currently being recorded.');
                    return true;
                }
                if (this.recordedCmds.length > 0) {
                    this.macros[this.recording] = this.recordedCmds;
                    this.saveMacros();
                    console.log(`Macro '${this.recording}' recorded (${this.recordedCmds.length} commands).`);
                } else {
                    console.log(`Macro '${this.recording}' is empty, not saved.`);
                }
                this.recording = null;
                this.recordedCmds = [];
                return true;

            case 'play':
                if (args.length === 0) {
                    console.log('Usage: play <name>');
                    return true;
                }
                const macroName = args[0];
                const cmds = this.macros[macroName];
                if (!cmds) {
                    console.log(`Macro '${macroName}' not found.`);
                    return true;
                }
                console.log(`Executing macro '${macroName}'...`);
                for (const c of cmds) {
                    console.log(`> ${c}`);
                    await this.runShell(c);
                }
                return true;

            case 'list':
                if (Object.keys(this.macros).length === 0) {
                    console.log('No macros defined.');
                } else {
                    for (const [name, cmds] of Object.entries(this.macros)) {
                        console.log(`${name} (${cmds.length} commands)`);
                    }
                }
                return true;

            case 'delete':
                if (args.length === 0) {
                    console.log('Usage: delete <name>');
                    return true;
                }
                const delName = args[0];
                if (this.macros[delName]) {
                    delete this.macros[delName];
                    this.saveMacros();
                    console.log(`Macro '${delName}' deleted.`);
                } else {
                    console.log(`Macro '${delName}' not found.`);
                }
                return true;

            case 'help':
                console.log('Commands:');
                console.log('  record <name>   Start recording a macro');
                console.log('  stop            Stop recording');
                console.log('  play <name>     Execute a macro');
                console.log('  list            Show all macros');
                console.log('  delete <name>   Delete a macro');
                console.log('  help            Show this help');
                console.log('  exit            Exit the program');
                console.log('Any other command is executed in the shell.');
                return true;

            case 'exit':
                return false;

            default:
                if (this.recording) {
                    this.recordedCmds.push(line);
                    console.log(`[recording] ${line}`);
                }
                await this.runShell(line);
                return true;
        }
    }

    async run() {
        console.log('Terminal Macros v1.0. Type "help" for commands.');
        this.rl.prompt();
        for await (const line of this.rl) {
            const cont = await this.handleCommand(line);
            if (!cont) break;
            this.rl.prompt();
        }
        this.saveMacros();
        process.exit(0);
    }
}

const app = new TerminalMacros();
app.run().catch(console.error);
