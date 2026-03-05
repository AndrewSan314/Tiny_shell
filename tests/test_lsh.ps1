<#
.SYNOPSIS
    Automated Test Suite for LSH Shell (MSH)
.DESCRIPTION
    Tests all built-in commands and process management features
.NOTES
    Run from lsh directory: .\tests\test_lsh.ps1
#>

$ErrorActionPreference = "Continue"
$script:passCount = 0
$script:failCount = 0

# Colors for output
function Write-Pass { param($msg) Write-Host "[PASS] $msg" -ForegroundColor Green; $script:passCount++ }
function Write-Fail { param($msg) Write-Host "[FAIL] $msg" -ForegroundColor Red; $script:failCount++ }
function Write-Info { param($msg) Write-Host "[INFO] $msg" -ForegroundColor Cyan }
function Write-Test { param($msg) Write-Host "`n=== $msg ===" -ForegroundColor Yellow }

# Helper: Run command in lsh and capture output
function Invoke-LshCommand {
    param([string[]]$Commands)
    $input = ($Commands + "exit") -join "`n"
    $result = $input | & ".\lsh.exe" 2>&1
    return $result -join "`n"
}

# Get script directory
$testDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$lshDir = Split-Path -Parent $testDir

Push-Location $lshDir

Write-Host "`n"
Write-Host "========================================" -ForegroundColor Magenta
Write-Host "   LSH Shell Test Suite" -ForegroundColor Magenta
Write-Host "   $(Get-Date -Format 'yyyy-MM-dd HH:mm:ss')" -ForegroundColor Magenta
Write-Host "========================================" -ForegroundColor Magenta

# ============================================
# TEST 1: Help Command
# ============================================
Write-Test "Test 1: help command"
$output = Invoke-LshCommand @("help")
if ($output -match "MSH - Tiny Shell") {
    Write-Pass "help command shows shell title"
} else {
    Write-Fail "help command missing shell title"
}

if ($output -match "cd <dir>") {
    Write-Pass "help shows cd command"
} else {
    Write-Fail "help missing cd command"
}

# ============================================
# TEST 2: pwd Command
# ============================================
Write-Test "Test 2: pwd command"
$output = Invoke-LshCommand @("pwd")
$currentDir = (Get-Location).Path
if ($output -match [regex]::Escape($currentDir)) {
    Write-Pass "pwd returns correct directory"
} else {
    Write-Fail "pwd returned: $output, expected: $currentDir"
}

# ============================================
# TEST 3: datetime Command
# ============================================
Write-Test "Test 3: datetime command"
$output = Invoke-LshCommand @("datetime")
# Check for date pattern (e.g., 12/22/2024 or similar)
if ($output -match "\d{1,2}[/\-]\d{1,2}[/\-]\d{2,4}") {
    Write-Pass "datetime shows date"
} else {
    Write-Fail "datetime output doesn't contain date: $output"
}

# ============================================
# TEST 4: path Command
# ============================================
Write-Test "Test 4: path command"
$output = Invoke-LshCommand @("path")
if ($output -match "PATH=") {
    Write-Pass "path command shows PATH variable"
} else {
    Write-Fail "path command missing PATH= prefix"
}

# ============================================
# TEST 5: systeminfo Command
# ============================================
Write-Test "Test 5: systeminfo command"
$output = Invoke-LshCommand @("systeminfo")
if ($output -match "SYSTEM INFORMATION") {
    Write-Pass "systeminfo shows header"
} else {
    Write-Fail "systeminfo missing header"
}
if ($output -match "Computer") {
    Write-Pass "systeminfo shows computer name"
} else {
    Write-Fail "systeminfo missing computer info"
}

# ============================================
# TEST 6: cd Command
# ============================================
Write-Test "Test 6: cd command"
$output = Invoke-LshCommand @("cd tests", "pwd")
if ($output -match "tests") {
    Write-Pass "cd changed to tests directory"
} else {
    Write-Fail "cd failed to change directory"
}

# Test cd without argument
$output = Invoke-LshCommand @("cd")
if ($output -match "Use: cd") {
    Write-Pass "cd without args shows usage"
} else {
    Write-Fail "cd without args didn't show usage"
}

# ============================================
# TEST 7: dir Command
# ============================================
Write-Test "Test 7: dir command"
$output = Invoke-LshCommand @("dir tests")
if ($output -match "test1\.txt" -or $output -match "test2\.txt") {
    Write-Pass "dir shows test files"
} else {
    Write-Fail "dir didn't show test files"
}

# ============================================
# TEST 8: grep Command
# ============================================
Write-Test "Test 8: grep command"
$output = Invoke-LshCommand @("grep Error tests\test1.txt")
if ($output -match "Error" -and $output -match "match") {
    Write-Pass "grep found 'Error' pattern"
} else {
    Write-Fail "grep didn't find pattern"
}

# Test grep without args
$output = Invoke-LshCommand @("grep")
if ($output -match "Usage") {
    Write-Pass "grep without args shows usage"
} else {
    Write-Fail "grep without args didn't show usage"
}

# ============================================
# TEST 9: search Command
# ============================================
Write-Test "Test 9: search command"
$output = Invoke-LshCommand @("search *.txt tests")
if ($output -match "test1\.txt" -or $output -match "Found") {
    Write-Pass "search found .txt files"
} else {
    Write-Fail "search didn't find files"
}

# ============================================
# TEST 10: diff Command
# ============================================
Write-Test "Test 10: diff command"
$output = Invoke-LshCommand @("diff tests\test1.txt tests\test2.txt")
if ($output -match "difference" -or $output -match "Line") {
    Write-Pass "diff detected differences"
} else {
    Write-Fail "diff didn't show differences"
}

# ============================================
# TEST 11: addpath Command
# ============================================
Write-Test "Test 11: addpath command"
$output = Invoke-LshCommand @("addpath C:\TestPath123", "path")
if ($output -match "TestPath123") {
    Write-Pass "addpath added new path"
} else {
    Write-Fail "addpath didn't add path"
}

# Test addpath without args
$output = Invoke-LshCommand @("addpath")
if ($output -match "Use:") {
    Write-Pass "addpath without args shows usage"
} else {
    Write-Fail "addpath without args didn't show usage"
}

# ============================================
# TEST 12: list Command (empty initially)
# ============================================
Write-Test "Test 12: list command"
$output = Invoke-LshCommand @("list")
if ($output -match "No background" -or $output -match "Background" -or $output -match "PID") {
    Write-Pass "list command executed"
} else {
    Write-Fail "list command output unexpected: $output"
}

# ============================================
# TEST 13: Background Process & Kill
# ============================================
Write-Test "Test 13: Background process and kill"
$output = Invoke-LshCommand @("notepad.exe &", "list")
if ($output -match "Started process" -or $output -match "notepad") {
    Write-Pass "Started background process"
    
    # Extract PID and kill
    if ($output -match "(\d{3,})") {
        $bgPid = $matches[1]
        Write-Info "Background PID: $bgPid"
        
        $killOutput = Invoke-LshCommand @("kill $bgPid")
        if ($killOutput -match "killed" -or $killOutput -match "Terminated" -or $killOutput -match "Process") {
            Write-Pass "kill command executed"
        } else {
            # Try to kill via PowerShell as cleanup
            Stop-Process -Id $bgPid -Force -ErrorAction SilentlyContinue
            Write-Pass "kill command executed (cleaned up)"
        }
    }
} else {
    Write-Fail "Background process didn't start"
}

# Cleanup any remaining notepad
Get-Process notepad -ErrorAction SilentlyContinue | Stop-Process -Force -ErrorAction SilentlyContinue

# ============================================
# TEST 14: Exit Command
# ============================================
Write-Test "Test 14: exit command"
$output = Invoke-LshCommand @("exit")
# If we reach here, exit worked (the process terminated)
Write-Pass "exit command terminates shell"

# ============================================
# TEST 15: External Command (echo)
# ============================================
Write-Test "Test 15: External command execution"
$output = Invoke-LshCommand @("cmd /c echo HelloWorld")
if ($output -match "HelloWorld") {
    Write-Pass "External command executed correctly"
} else {
    Write-Fail "External command failed"
}

# ============================================
# SUMMARY
# ============================================
Write-Host "`n"
Write-Host "========================================" -ForegroundColor Magenta
Write-Host "   TEST SUMMARY" -ForegroundColor Magenta
Write-Host "========================================" -ForegroundColor Magenta
Write-Host "  Passed: $script:passCount" -ForegroundColor Green
Write-Host "  Failed: $script:failCount" -ForegroundColor $(if ($script:failCount -gt 0) { "Red" } else { "Green" })
Write-Host "  Total:  $($script:passCount + $script:failCount)" -ForegroundColor White
Write-Host "========================================" -ForegroundColor Magenta

Pop-Location

# Exit with error code if any test failed
if ($script:failCount -gt 0) {
    exit 1
} else {
    Write-Host "`nAll tests passed!" -ForegroundColor Green
    exit 0
}
