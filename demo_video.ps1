<#
.SYNOPSIS
    MSH Shell Demo Video Automation Script
.DESCRIPTION
    Launches msh.exe, types the curated `demo` showcase, and queues `exit`
    so the full recording runs hands-free.
.NOTES
    1. Start your screen recorder (OBS, Xbox Game Bar, etc.)
    2. Run: powershell -ExecutionPolicy Bypass -File demo_video.ps1
    3. The script opens MSH, types `demo`, then exits when the showcase ends.
#>

$ErrorActionPreference = "Continue"

# Configuration
$MSH_PATH = Join-Path $PSScriptRoot "msh.exe"
$CHAR_DELAY_MS = 35        # Delay between each character (typing effect)
$BOOT_WAIT_MS = 6000       # Wait for the startup sequence to finish
$POST_DEMO_QUEUE_MS = 1200 # Small pause before queueing `exit`

# Verify msh.exe exists
if (-not (Test-Path $MSH_PATH)) {
    Write-Host "ERROR: msh.exe not found at $MSH_PATH" -ForegroundColor Red
    Write-Host "Run build.bat first!" -ForegroundColor Yellow
    exit 1
}

Write-Host ""
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "   MSH Showcase Recorder" -ForegroundColor Cyan
Write-Host "   Start your screen recorder NOW!" -ForegroundColor Yellow
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""
Write-Host "Press ENTER when ready to begin..." -ForegroundColor Green
Read-Host

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
Start-Sleep -Milliseconds $BOOT_WAIT_MS

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

Type-Command "demo"
Start-Sleep -Milliseconds $POST_DEMO_QUEUE_MS
Type-Command "exit"

# Wait for process to exit
$process.WaitForExit(300000)

Write-Host ""
Write-Host "========================================" -ForegroundColor Green
Write-Host "   Demo complete! Stop recording." -ForegroundColor Green
Write-Host "========================================" -ForegroundColor Green
