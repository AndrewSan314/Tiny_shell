# LSH - Modern Shell Demo Edition

A simple shell implementation converted to work with Windows Win32 API.

## Features

- **Cinematic startup**: hacker-style boot sequence and animated prompt
- **Built-in toolkit**: process control, file utilities, history, aliases, env
- **`demo` command**: one-shot scripted showcase for a clean 2-minute video
- **External command execution**: run Windows programs directly

## Building

### Windows (Using MinGW/GCC)

**Option 1: Using the build script (Recommended)**
```cmd
build.bat
```

**Option 2: Using Make**
```cmd
make
```

**Option 3: Manual compilation**
```cmd
gcc -Wall -Wextra -std=c99 -D_CRT_SECURE_NO_WARNINGS -DWIN32_LEAN_AND_MEAN src/main.c -o lsh.exe
```

## Usage

```cmd
msh.exe
```

### Available Commands

#### Built-in Commands
- `cd <directory>` - Change directory
- `pwd` - Print working directory  
- `help` - Show available commands
- `demo` - Run the cinematic showcase flow
- `exit` - Exit the shell

#### Windows System Commands
- `dir` - List files and directories
- `type <file>` - Display file contents
- `echo <text>` - Print text
- `cls` - Clear screen
- `notepad <file>` - Open file in Notepad
- And any other Windows executable

### 2-Minute Demo Flow

The curated `demo` flow now opens with the GoogleMap app launcher, then moves through source workflow, visual identity, and ends with the built-in AI copilot. The GoogleMap batch segment opens the local web app briefly before returning to the shell.

```cmd
msh.exe
demo
```

For a higher-intensity recording, use:

```cmd
demo epic
```

For automated capture, run:

```powershell
powershell -ExecutionPolicy Bypass -File demo_video.ps1
```

If you run the showcase manually, finish with:

```cmd
exit
```

### Example Usage

```cmd
> pwd
C:\Users\YourName

> cd Documents
> pwd  
C:\Users\YourName\Documents

> dir
# Lists files in current directory

> echo Hello World
Hello World

> notepad test.txt
# Opens Notepad with test.txt

> help
LSH - Simple Shell for Windows
Type program names and arguments, and hit enter.
The following are built in:
  cd
  help
  exit
  pwd
Use Windows commands like: dir, type, echo, cls, notepad, etc.

> exit
```

## Architecture

The shell uses Win32 API instead of Unix system calls:

- `CreateProcess()` instead of `fork()/execvp()`
- `_chdir()` instead of `chdir()`
- `_getcwd()` instead of `getcwd()`
- `WaitForSingleObject()` instead of `waitpid()`

## Building Requirements

- MinGW-w64 or similar GCC compiler for Windows
- Windows SDK (usually included with MinGW-w64)

## License

This project is released under the public domain (UNLICENSE).
