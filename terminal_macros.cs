// terminal_macros.cs
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Text;
using System.Text.Json;

class TerminalMacros
{
    private Dictionary<string, List<string>> macros = new Dictionary<string, List<string>>();
    private string recording = null;
    private List<string> recordedCmds = new List<string>();
    private string macrosFile = Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.UserProfile), ".terminal_macros.json");

    public TerminalMacros()
    {
        LoadMacros();
    }

    private void LoadMacros()
    {
        if (File.Exists(macrosFile))
        {
            try
            {
                string json = File.ReadAllText(macrosFile);
                macros = JsonSerializer.Deserialize<Dictionary<string, List<string>>>(json) ?? new Dictionary<string, List<string>>();
            }
            catch { macros = new Dictionary<string, List<string>>(); }
        }
    }

    private void SaveMacros()
    {
        string json = JsonSerializer.Serialize(macros, new JsonSerializerOptions { WriteIndented = true });
        File.WriteAllText(macrosFile, json);
    }

    private int RunShell(string cmd)
    {
        ProcessStartInfo psi = new ProcessStartInfo();
        psi.FileName = "/bin/sh";
        psi.Arguments = "-c \"" + cmd.Replace("\"", "\\\"") + "\"";
        psi.RedirectStandardOutput = false;
        psi.RedirectStandardError = false;
        psi.UseShellExecute = false;
        psi.CreateNoWindow = true;
        try
        {
            Process p = Process.Start(psi);
            p.WaitForExit();
            return p.ExitCode;
        }
        catch
        {
            // fallback для Windows
            psi.FileName = "cmd";
            psi.Arguments = "/c " + cmd;
            Process p = Process.Start(psi);
            p.WaitForExit();
            return p.ExitCode;
        }
    }

    public bool HandleCommand(string line)
    {
        line = line.Trim();
        if (string.IsNullOrEmpty(line)) return true;

        string[] parts = line.Split(new char[] { ' ' }, StringSplitOptions.RemoveEmptyEntries);
        string cmd = parts[0];
        string[] args = parts.Length > 1 ? parts[1..] : new string[0];

        switch (cmd)
        {
            case "record":
                if (args.Length == 0)
                {
                    Console.WriteLine("Usage: record <name>");
                    return true;
                }
                string name = args[0];
                if (recording != null)
                {
                    Console.WriteLine($"Already recording macro '{recording}'. Use 'stop' to finish.");
                    return true;
                }
                recording = name;
                recordedCmds.Clear();
                Console.WriteLine($"Recording macro '{name}'. Type 'stop' to finish.");
                return true;

            case "stop":
                if (recording == null)
                {
                    Console.WriteLine("No macro is currently being recorded.");
                    return true;
                }
                if (recordedCmds.Count > 0)
                {
                    macros[recording] = new List<string>(recordedCmds);
                    SaveMacros();
                    Console.WriteLine($"Macro '{recording}' recorded ({recordedCmds.Count} commands).");
                }
                else
                {
                    Console.WriteLine($"Macro '{recording}' is empty, not saved.");
                }
                recording = null;
                recordedCmds.Clear();
                return true;

            case "play":
                if (args.Length == 0)
                {
                    Console.WriteLine("Usage: play <name>");
                    return true;
                }
                string macroName = args[0];
                if (!macros.ContainsKey(macroName))
                {
                    Console.WriteLine($"Macro '{macroName}' not found.");
                    return true;
                }
                Console.WriteLine($"Executing macro '{macroName}'...");
                foreach (string c in macros[macroName])
                {
                    Console.WriteLine($"> {c}");
                    RunShell(c);
                }
                return true;

            case "list":
                if (macros.Count == 0)
                {
                    Console.WriteLine("No macros defined.");
                }
                else
                {
                    foreach (var kv in macros)
                    {
                        Console.WriteLine($"{kv.Key} ({kv.Value.Count} commands)");
                    }
                }
                return true;

            case "delete":
                if (args.Length == 0)
                {
                    Console.WriteLine("Usage: delete <name>");
                    return true;
                }
                string delName = args[0];
                if (macros.Remove(delName))
                {
                    SaveMacros();
                    Console.WriteLine($"Macro '{delName}' deleted.");
                }
                else
                {
                    Console.WriteLine($"Macro '{delName}' not found.");
                }
                return true;

            case "help":
                Console.WriteLine("Commands:");
                Console.WriteLine("  record <name>   Start recording a macro");
                Console.WriteLine("  stop            Stop recording");
                Console.WriteLine("  play <name>     Execute a macro");
                Console.WriteLine("  list            Show all macros");
                Console.WriteLine("  delete <name>   Delete a macro");
                Console.WriteLine("  help            Show this help");
                Console.WriteLine("  exit            Exit the program");
                Console.WriteLine("Any other command is executed in the shell.");
                return true;

            case "exit":
                return false;

            default:
                if (recording != null)
                {
                    recordedCmds.Add(line);
                    Console.WriteLine($"[recording] {line}");
                }
                RunShell(line);
                return true;
        }
    }

    public void Run()
    {
        Console.WriteLine("Terminal Macros v1.0. Type 'help' for commands.");
        while (true)
        {
            Console.Write("> ");
            string line = Console.ReadLine();
            if (line == null) break;
            if (!HandleCommand(line)) break;
        }
        SaveMacros();
    }

    static void Main()
    {
        TerminalMacros app = new TerminalMacros();
        app.Run();
    }
}
