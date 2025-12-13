@echo off
echo ==========================================
echo   Building MSH Shell (Modular Version)
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
if exist lsh.exe del /Q lsh.exe

REM Compile each source file
echo.
echo Compiling source files...

echo   [1/5] main.c
gcc -Wall -Wextra -std=c99 -I./include -c src/main.c -o src/main.o
if %errorlevel% neq 0 goto :error

echo   [2/5] core.c
gcc -Wall -Wextra -std=c99 -I./include -c src/core.c -o src/core.o
if %errorlevel% neq 0 goto :error

echo   [3/5] process_manager.c
gcc -Wall -Wextra -std=c99 -I./include -c src/process_manager.c -o src/process_manager.o
if %errorlevel% neq 0 goto :error

echo   [4/5] builtins.c
gcc -Wall -Wextra -std=c99 -I./include -c src/builtins.c -o src/builtins.o
if %errorlevel% neq 0 goto :error

echo   [5/5] launcher.c
gcc -Wall -Wextra -std=c99 -I./include -c src/launcher.c -o src/launcher.o
if %errorlevel% neq 0 goto :error

REM Link
echo.
echo Linking...
gcc src/main.o src/core.o src/process_manager.o src/builtins.o src/launcher.o -o lsh.exe
if %errorlevel% neq 0 goto :error

echo.
echo ==========================================
echo   Build successful!
echo   Run: lsh.exe
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