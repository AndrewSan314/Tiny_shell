@echo off
echo Building LSH Shell for Windows...

REM Check if gcc is available
gcc --version >nul 2>&1
if %errorlevel% neq 0 (
    echo Error: GCC not found. Please install MinGW-w64.
    echo You can download it from: https://www.mingw-w64.org/
    pause
    exit /b 1
)

REM Clean previous build
if exist src\*.o del /Q src\*.o
if exist lsh.exe del /Q lsh.exe

REM Compile
echo Compiling source files...
gcc -Wall -Wextra -std=c99 -D_CRT_SECURE_NO_WARNINGS -DWIN32_LEAN_AND_MEAN -c src/main.c -o src/main.o
if %errorlevel% neq 0 goto :error

REM Link
echo Linking...
gcc src/main.o -o lsh.exe
if %errorlevel% neq 0 goto :error

echo.
echo Build successful! You can now run lsh.exe
echo.
echo Type "lsh.exe" to start the shell
pause
exit /b 0

:error
echo.
echo Build failed!
pause
exit /b 1