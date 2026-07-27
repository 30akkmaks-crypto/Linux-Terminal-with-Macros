💻 Linux Terminal with Macros – Автоматизация командной строки
Интерактивный терминал с поддержкой макросов – записывайте, сохраняйте и воспроизводите последовательности команд для автоматизации рутинных задач.
Реализован на 7 языках программирования – выберите свой!

✨ Возможности
🖥️ Полноценная эмуляция терминала – выполнение произвольных команд в подпроцессе (bash/cmd/pwsh).

📝 Запись макросов – команда record <имя> начинает запись всех последующих введённых команд до stop.

▶️ Воспроизведение макросов – команда play <имя> выполняет сохранённую последовательность команд.

📋 Управление макросами – list для просмотра сохранённых макросов, delete <имя> для удаления.

💾 Постоянное хранение – макросы сохраняются в файл (например, ~/.terminal_macros.json) и загружаются при старте.

⚡ Псевдонимы (алиасы) – возможность создавать короткие имена для часто используемых команд (опционально).

🔄 Автодополнение – при вводе поддерживается автодополнение имён макросов и команд (где реализовано).

🛠 Кроссплатформенность – работает в Linux, macOS и Windows (использует системную оболочку).

📦 Поддерживаемые языки
Язык	Версия	Файл	Основная библиотека
Python	3.8+	terminal_macros.py	subprocess, readline
Go	1.18+	terminal_macros.go	os/exec, bufio
Rust	1.60+	terminal_macros.rs	std::process, rustyline
JavaScript	Node.js 14+	terminal_macros.js	child_process, readline
C#	.NET 6+	terminal_macros.cs	System.Diagnostics, Console
Java	11+	TerminalMacros.java	ProcessBuilder, java.io
C++	C++17	terminal_macros.cpp	fork/exec, readline (или linenoise)
🚀 Быстрый старт
1. Склонируйте репозиторий
bash
git clone https://github.com/yourname/terminal-macros.git
cd terminal-macros
2. Запустите на любом языке
Python

bash
python terminal_macros.py
Go

bash
go mod init terminal_macros
go run terminal_macros.go
Rust (сборка)

bash
cargo new terminal_macros
# добавьте зависимости в Cargo.toml
cargo run
JavaScript (Node.js)

bash
node terminal_macros.js
C#

bash
dotnet run
Java (сборка)

bash
javac TerminalMacros.java
java TerminalMacros
C++ (сборка с readline)

bash
g++ -std=c++17 terminal_macros.cpp -lreadline -o terminal_macros
./terminal_macros
📋 Пример сессии
text
$ python terminal_macros.py
Terminal Macros v1.0. Type 'help' for commands.
> record deploy
Recording macro 'deploy'. Type 'stop' to finish.
> git pull origin main
> npm install
> npm run build
> pm2 restart app
> stop
Macro 'deploy' recorded (4 commands).
> list
deploy (4 commands)
> play deploy
Executing macro 'deploy'...
> git pull origin main
Already up to date.
> npm install
...
> pm2 restart app
[PM2] App restarted.
Macro 'deploy' executed.
> delete deploy
Macro 'deploy' deleted.
> exit
⚙️ Команды (внутри терминала)
Команда	Описание
record <имя>	Начать запись макроса с указанным именем
stop	Остановить запись текущего макроса
play <имя>	Воспроизвести сохранённый макрос
list	Показать все сохранённые макросы
delete <имя>	Удалить макрос
help	Показать справку
exit	Выйти из программы
Все остальные команды выполняются в системной оболочке (вывод направляется на консоль).

📄 Лицензия
MIT – свободно используйте, модифицируйте и распространяйте.

🤝 Вклад
Приветствуются pull request'ы! Если хотите добавить новый язык или улучшить существующий – создавайте issue.

🧠 Авторы
Проект создан в образовательных целях для демонстрации интерактивных оболочек и автоматизации на разных языках.
