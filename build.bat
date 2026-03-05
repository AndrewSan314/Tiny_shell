@echo off
echo ==========================================
echo   Building MSH Shell v2.0 (Modular)
echo ==========================================
echo.

REM Check if gcc is available
gcc --version >nul 2>&1
if %errorlevel% neq 0 (
    echo Error: GCC not found. Please install MinGW-w64.
    echo Download from: https://www.mingw-w64.org/
    pause
    exit /b 1
)

REM Clean previous build
echo Cleaning previous build...
if exist src\*.o del /Q src\*.o
if exist msh.exe del /Q msh.exe

REM Compile each source file
echo.
echo Compiling source files...

echo   [1/14] main.c
gcc -Wall -Wextra -std=c99 -I./include -c src/main.c -o src/main.o
if %errorlevel% neq 0 goto :error

echo   [2/14] core.c
gcc -Wall -Wextra -std=c99 -I./include -c src/core.c -o src/core.o
if %errorlevel% neq 0 goto :error

echo   [3/14] colors.c
gcc -Wall -Wextra -std=c99 -I./include -c src/colors.c -o src/colors.o
if %errorlevel% neq 0 goto :error

echo   [4/14] history.c
gcc -Wall -Wextra -std=c99 -I./include -c src/history.c -o src/history.o
if %errorlevel% neq 0 goto :error

echo   [5/14] readline.c
gcc -Wall -Wextra -std=c99 -I./include -c src/readline.c -o src/readline.o
if %errorlevel% neq 0 goto :error

echo   [6/14] alias.c
gcc -Wall -Wextra -std=c99 -I./include -c src/alias.c -o src/alias.o
if %errorlevel% neq 0 goto :error

echo   [7/14] pipe_redirect.c
gcc -Wall -Wextra -std=c99 -I./include -c src/pipe_redirect.c -o src/pipe_redirect.o
if %errorlevel% neq 0 goto :error

echo   [8/14] config.c
gcc -Wall -Wextra -std=c99 -I./include -c src/config.c -o src/config.o
if %errorlevel% neq 0 goto :error

echo   [9/14] hacker.c
gcc -Wall -Wextra -std=c99 -I./include -c src/hacker.c -o src/hacker.o
if %errorlevel% neq 0 goto :error

echo   [10/14] ai.c
gcc -Wall -Wextra -std=c99 -I./include -c src/ai.c -o src/ai.o
if %errorlevel% neq 0 goto :error

echo   [11/14] process_manager.c
gcc -Wall -Wextra -std=c99 -I./include -c src/process_manager.c -o src/process_manager.o
if %errorlevel% neq 0 goto :error

echo   [12/14] builtins.c
gcc -Wall -Wextra -std=c99 -I./include -c src/builtins.c -o src/builtins.o
if %errorlevel% neq 0 goto :error

echo   [13/14] utils.c
gcc -Wall -Wextra -std=c99 -I./include -c src/utils.c -o src/utils.o
if %errorlevel% neq 0 goto :error

echo   [14/14] launcher.c
gcc -Wall -Wextra -std=c99 -I./include -c src/launcher.c -o src/launcher.o
if %errorlevel% neq 0 goto :error

REM Link
echo.
echo Linking...
gcc src/main.o src/core.o src/colors.o src/history.o src/readline.o src/alias.o src/pipe_redirect.o src/config.o src/hacker.o src/ai.o src/process_manager.o src/builtins.o src/launcher.o src/utils.o -o msh.exe
if %errorlevel% neq 0 goto :error

echo.
echo ==========================================
echo   Build successful!
echo   Run: msh.exe
echo ==========================================
pause
exit /b 0

:error
echo.
echo ==========================================
echo   BUILD FAILED!
echo ==========================================
pause
exit /b 1
