<#
.SYNOPSIS
    MSH Shell Demo Video Automation Script
.DESCRIPTION
    Launches msh.exe and types commands one by one with realistic delays,
    so you just need to record your screen.
.NOTES
    1. Start your screen recorder (OBS, Xbox Game Bar, etc.)
    2. Run: powershell -ExecutionPolicy Bypass -File demo_video.ps1
    3. Watch the magic happen, then stop recording
#>

$ErrorActionPreference = "Continue"

# Configuration
$MSH_PATH = Join-Path $PSScriptRoot "msh.exe"
$CHAR_DELAY_MS = 35        # Delay between each character (typing effect)
$CMD_PAUSE_MS = 1200       # Pause after each command output
$SECTION_PAUSE_MS = 2000   # Pause between sections

# Verify msh.exe exists
if (-not (Test-Path $MSH_PATH)) {
    Write-Host "ERROR: msh.exe not found at $MSH_PATH" -ForegroundColor Red
    Write-Host "Run build.bat first!" -ForegroundColor Yellow
    exit 1
}

Write-Host ""
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "   MSH Demo Video Script" -ForegroundColor Cyan
Write-Host "   Start your screen recorder NOW!" -ForegroundColor Yellow
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""
Write-Host "Press ENTER when ready to begin..." -ForegroundColor Green
Read-Host

# Define demo commands - each section is a group
$sections = @(
    @{
        Title = "Basic Navigation"
        Commands = @("pwd", "datetime", "dir")
    },
    @{
        Title = "File Operations"
        Commands = @("tree src -d 1", "search *.c src", "grep msh src\main.c", "wc src\main.c", "head src\main.c 5")
    },
    @{
        Title = "Built-in Calculator"
        Commands = @("calc 2+3*4", "calc (10-3)/2", "calc 3.14*5*5")
    },
    @{
        Title = "System Information"
        Commands = @("systeminfo", "whoami", "uptime")
    },
    @{
        Title = "Aliases & Environment"
        Commands = @("alias ll=dir", "alias", "export GREETING=Hello_from_MSH", "echo `$GREETING", "unalias ll", "unset GREETING")
    },
    @{
        Title = "Pipe & Redirect"
        Commands = @("echo Hello World > test_demo.txt", "cat test_demo.txt", "echo MSH Shell is awesome >> test_demo.txt", "cat test_demo.txt", "diff test.txt test2.txt", "rm test_demo.txt")
    },
    @{
        Title = "Color Themes"
        Commands = @("color", "color ocean", "tree src -d 1", "color sunset", "calc 42*42", "color matrix")
    },
    @{
        Title = "History & Help"
        Commands = @("history", "help")
    }
)

# Launch msh.exe as a subprocess
$psi = New-Object System.Diagnostics.ProcessStartInfo
$psi.FileName = $MSH_PATH
$psi.UseShellExecute = $false
$psi.RedirectStandardInput = $true
$psi.RedirectStandardOutput = $false
$psi.RedirectStandardError = $false
$psi.WorkingDirectory = $PSScriptRoot

$process = [System.Diagnostics.Process]::Start($psi)
$stdin = $process.StandardInput

# Wait for boot sequence to complete
Start-Sleep -Seconds 6

# Function to type a command character by character
function Type-Command {
    param([string]$cmd)
    foreach ($char in $cmd.ToCharArray()) {
        $stdin.Write($char)
        $stdin.Flush()
        Start-Sleep -Milliseconds $CHAR_DELAY_MS
    }
    Start-Sleep -Milliseconds 200
    $stdin.WriteLine("")
    $stdin.Flush()
}

# Run through each section
foreach ($section in $sections) {
    Start-Sleep -Milliseconds $SECTION_PAUSE_MS
    
    foreach ($cmd in $section.Commands) {
        Type-Command $cmd
        Start-Sleep -Milliseconds $CMD_PAUSE_MS
    }
}

# Final pause then exit
Start-Sleep -Milliseconds $SECTION_PAUSE_MS
Type-Command "exit"

# Wait for process to exit
$process.WaitForExit(5000)

Write-Host ""
Write-Host "========================================" -ForegroundColor Green
Write-Host "   Demo complete! Stop recording." -ForegroundColor Green
Write-Host "========================================" -ForegroundColor Green
