@echo off
echo ==========================================
echo   MSH Shell - Build and Run
echo ==========================================
echo.

REM Build if msh.exe doesn't exist or source is newer
if not exist msh.exe (
    echo msh.exe not found. Building...
    echo.
    call build.bat
    if %errorlevel% neq 0 (
        echo.
        echo Build failed. Cannot start the shell.
        pause
        exit /b 1
    )
    echo.
)

REM Run the shell
echo Starting MSH Shell...
echo ==========================================
echo.
msh.exe
