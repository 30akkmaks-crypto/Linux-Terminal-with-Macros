// TerminalMacros.java
import java.io.*;
import java.nio.file.*;
import java.util.*;
import com.google.gson.Gson;
import com.google.gson.GsonBuilder;
import com.google.gson.reflect.TypeToken;

public class TerminalMacros {
    private Map<String, List<String>> macros;
    private String recording;
    private List<String> recordedCmds;
    private String macrosFile;
    private Gson gson;

    public TerminalMacros() {
        macros = new HashMap<>();
        recording = null;
        recordedCmds = new ArrayList<>();
        String home = System.getProperty("user.home");
        macrosFile = home + File.separator + ".terminal_macros.json";
        gson = new GsonBuilder().setPrettyPrinting().create();
        loadMacros();
    }

    private void loadMacros() {
        Path path = Paths.get(macrosFile);
        if (Files.exists(path)) {
            try {
                String json = new String(Files.readAllBytes(path));
                java.lang.reflect.Type type = new TypeToken<Map<String, List<String>>>(){}.getType();
                macros = gson.fromJson(json, type);
                if (macros == null) macros = new HashMap<>();
            } catch (Exception e) {
                macros = new HashMap<>();
            }
        }
    }

    private void saveMacros() {
        try {
            String json = gson.toJson(macros);
            Files.write(Paths.get(macrosFile), json.getBytes());
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    private int runShell(String cmd) {
        try {
            String shell = "/bin/sh";
            String flag = "-c";
            if (System.getProperty("os.name").toLowerCase().contains("win")) {
                shell = "cmd";
                flag = "/c";
            }
            ProcessBuilder pb = new ProcessBuilder(shell, flag, cmd);
            pb.inheritIO();
            Process p = pb.start();
            return p.waitFor();
        } catch (Exception e) {
            e.printStackTrace();
            return 1;
        }
    }

    public boolean handleCommand(String line) {
        line = line.trim();
        if (line.isEmpty()) return true;
        String[] parts = line.split("\\s+");
        String cmd = parts[0];
        String[] args = parts.length > 1 ? Arrays.copyOfRange(parts, 1, parts.length) : new String[0];

        switch (cmd) {
            case "record":
                if (args.length == 0) {
                    System.out.println("Usage: record <name>");
                    return true;
                }
                String name = args[0];
                if (recording != null) {
                    System.out.println("Already recording macro '" + recording + "'. Use 'stop' to finish.");
                    return true;
                }
                recording = name;
                recordedCmds.clear();
                System.out.println("Recording macro '" + name + "'. Type 'stop' to finish.");
                return true;

            case "stop":
                if (recording == null) {
                    System.out.println("No macro is currently being recorded.");
                    return true;
                }
                if (!recordedCmds.isEmpty()) {
                    macros.put(recording, new ArrayList<>(recordedCmds));
                    saveMacros();
                    System.out.println("Macro '" + recording + "' recorded (" + recordedCmds.size() + " commands).");
                } else {
                    System.out.println("Macro '" + recording + "' is empty, not saved.");
                }
                recording = null;
                recordedCmds.clear();
                return true;

            case "play":
                if (args.length == 0) {
                    System.out.println("Usage: play <name>");
                    return true;
                }
                String macroName = args[0];
                List<String> cmds = macros.get(macroName);
                if (cmds == null) {
                    System.out.println("Macro '" + macroName + "' not found.");
                    return true;
                }
                System.out.println("Executing macro '" + macroName + "'...");
                for (String c : cmds) {
                    System.out.println("> " + c);
                    runShell(c);
                }
                return true;

            case "list":
                if (macros.isEmpty()) {
                    System.out.println("No macros defined.");
                } else {
                    for (Map.Entry<String, List<String>> entry : macros.entrySet()) {
                        System.out.println(entry.getKey() + " (" + entry.getValue().size() + " commands)");
                    }
                }
                return true;

            case "delete":
                if (args.length == 0) {
                    System.out.println("Usage: delete <name>");
                    return true;
                }
                String delName = args[0];
                if (macros.remove(delName) != null) {
                    saveMacros();
                    System.out.println("Macro '" + delName + "' deleted.");
                } else {
                    System.out.println("Macro '" + delName + "' not found.");
                }
                return true;

            case "help":
                System.out.println("Commands:");
                System.out.println("  record <name>   Start recording a macro");
                System.out.println("  stop            Stop recording");
                System.out.println("  play <name>     Execute a macro");
                System.out.println("  list            Show all macros");
                System.out.println("  delete <name>   Delete a macro");
                System.out.println("  help            Show this help");
                System.out.println("  exit            Exit the program");
                System.out.println("Any other command is executed in the shell.");
                return true;

            case "exit":
                return false;

            default:
                if (recording != null) {
                    recordedCmds.add(line);
                    System.out.println("[recording] " + line);
                }
                runShell(line);
                return true;
        }
    }

    public void run() {
        System.out.println("Terminal Macros v1.0. Type 'help' for commands.");
        BufferedReader reader = new BufferedReader(new InputStreamReader(System.in));
        while (true) {
            System.out.print("> ");
            try {
                String line = reader.readLine();
                if (line == null) break;
                if (!handleCommand(line)) break;
            } catch (IOException e) {
                break;
            }
        }
        saveMacros();
    }

    public static void main(String[] args) {
        TerminalMacros app = new TerminalMacros();
        app.run();
    }
}
