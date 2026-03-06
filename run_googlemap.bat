@echo off
setlocal

set "PROJECT_ROOT=%~dp0IT3160-Project-GoogleMap"
set "BACKEND_DIR=%PROJECT_ROOT%\backend"
set "FRONTEND_DIR=%PROJECT_ROOT%\frontend"
set "PID_DIR=%PROJECT_ROOT%\.run"

if not exist "%PROJECT_ROOT%" (
    echo [ERROR] Folder not found: %PROJECT_ROOT%
    exit /b 1
)

where python >nul 2>&1
if errorlevel 1 (
    echo [ERROR] Python is not available in PATH.
    exit /b 1
)

if not exist "%BACKEND_DIR%\.env" (
    if exist "%BACKEND_DIR%\.env.example" (
        copy "%BACKEND_DIR%\.env.example" "%BACKEND_DIR%\.env" >nul
        echo [INFO] Created backend\.env from .env.example
    )
)

if not exist "%PID_DIR%" mkdir "%PID_DIR%" >nul 2>&1

echo [INFO] Releasing ports 8000/8080 if already in use...
powershell -NoProfile -Command ^
  "$ports = @(8000,8080); " ^
  "$killed = @(); " ^
  "foreach($p in $ports){ " ^
  "  $conns = Get-NetTCPConnection -LocalPort $p -ErrorAction SilentlyContinue; " ^
  "  foreach($c in $conns){ " ^
  "    $procId = $c.OwningProcess; " ^
  "    if($procId -and ($killed -notcontains $procId)){ " ^
  "      try { Stop-Process -Id $procId -Force -ErrorAction Stop; $killed += $procId; Write-Output ('[OK] Released PID ' + $procId + ' on port ' + $p) } " ^
  "      catch { Write-Output ('[WARN] Could not release PID ' + $procId + ' on port ' + $p) } " ^
  "    } " ^
  "  } " ^
  "} " ^
  "if($killed.Count -eq 0){ Write-Output '[INFO] Ports already free.' }"

echo [INFO] Starting GoogleMap backend on http://localhost:8000 ...
powershell -NoProfile -Command ^
  "$out = '%PID_DIR%\\backend.out.log'; " ^
  "$err = '%PID_DIR%\\backend.err.log'; " ^
  "if(Test-Path $out){ Remove-Item $out -Force }; " ^
  "if(Test-Path $err){ Remove-Item $err -Force }; " ^
  "$prev = $env:DEBUG; " ^
  "$env:DEBUG = 'True'; " ^
  "$p = Start-Process -FilePath 'python' -ArgumentList @('-m','uvicorn','app.main:app','--host','0.0.0.0','--port','8000') -WorkingDirectory '%BACKEND_DIR%' -RedirectStandardOutput $out -RedirectStandardError $err -WindowStyle Hidden -PassThru; " ^
  "if($null -eq $prev){ Remove-Item Env:DEBUG -ErrorAction SilentlyContinue } else { $env:DEBUG = $prev }; " ^
  "Set-Content -Path '%PID_DIR%\\backend.pid' -Value $p.Id; " ^
  "Write-Output ('[OK] Backend PID: ' + $p.Id)"

echo [INFO] Starting GoogleMap frontend on http://localhost:8080 ...
powershell -NoProfile -Command ^
  "$out = '%PID_DIR%\\frontend.out.log'; " ^
  "$err = '%PID_DIR%\\frontend.err.log'; " ^
  "if(Test-Path $out){ Remove-Item $out -Force }; " ^
  "if(Test-Path $err){ Remove-Item $err -Force }; " ^
  "$p = Start-Process -FilePath 'python' -ArgumentList @('-m','http.server','8080') -WorkingDirectory '%FRONTEND_DIR%' -RedirectStandardOutput $out -RedirectStandardError $err -WindowStyle Hidden -PassThru; " ^
  "Set-Content -Path '%PID_DIR%\\frontend.pid' -Value $p.Id; " ^
  "Write-Output ('[OK] Frontend PID: ' + $p.Id)"

echo.
echo [DONE] Services started.
echo        User:  http://localhost:8080
echo        Admin: http://localhost:8080/admin.html
echo        API:   http://localhost:8000/docs

endlocal
