# LSH - Simple Shell

A simple shell implementation converted to work with Windows Win32 API.

## Features

- **Built-in commands**: `cd`, `pwd`, `help`, `exit`
- **External command execution**: Run any Windows program
- **Cross-platform compatibility**: Works on Windows with Win32 API
- **Memory management**: Proper allocation and cleanup

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
lsh.exe
```

### Available Commands

#### Built-in Commands
- `cd <directory>` - Change directory
- `pwd` - Print working directory  
- `help` - Show available commands
- `exit` - Exit the shell

#### Windows System Commands
- `dir` - List files and directories
- `type <file>` - Display file contents
- `echo <text>` - Print text
- `cls` - Clear screen
- `notepad <file>` - Open file in Notepad
- And any other Windows executable

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