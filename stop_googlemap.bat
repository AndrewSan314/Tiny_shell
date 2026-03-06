@echo off
setlocal

set "PROJECT_ROOT=%~dp0IT3160-Project-GoogleMap"
set "PID_DIR=%PROJECT_ROOT%\.run"

if exist "%PID_DIR%\backend.pid" (
    for /f "usebackq delims=" %%i in ("%PID_DIR%\backend.pid") do (
        powershell -NoProfile -Command "try { Stop-Process -Id %%i -Force -ErrorAction Stop; Write-Output '[OK] Stopped backend PID %%i' } catch { Write-Output '[WARN] Backend PID %%i not running' }"
    )
    del /f /q "%PID_DIR%\backend.pid" >nul 2>&1
)

if exist "%PID_DIR%\frontend.pid" (
    for /f "usebackq delims=" %%i in ("%PID_DIR%\frontend.pid") do (
        powershell -NoProfile -Command "try { Stop-Process -Id %%i -Force -ErrorAction Stop; Write-Output '[OK] Stopped frontend PID %%i' } catch { Write-Output '[WARN] Frontend PID %%i not running' }"
    )
    del /f /q "%PID_DIR%\frontend.pid" >nul 2>&1
)

echo [INFO] Stopping processes bound to ports 8000 and 8080...

powershell -NoProfile -Command ^
  "$ports = @(8000,8080); " ^
  "$killed = @(); " ^
  "foreach($p in $ports){ " ^
  "  $conns = Get-NetTCPConnection -LocalPort $p -ErrorAction SilentlyContinue; " ^
  "  foreach($c in $conns){ " ^
  "    $procId = $c.OwningProcess; " ^
  "    if($procId -and ($killed -notcontains $procId)){ " ^
  "      try { Stop-Process -Id $procId -Force -ErrorAction Stop; $killed += $procId; Write-Output ('[OK] Killed PID ' + $procId + ' on port ' + $p) } " ^
  "      catch { Write-Output ('[WARN] Could not kill PID ' + $procId + ' on port ' + $p + ': ' + $_.Exception.Message) } " ^
  "    } " ^
  "  } " ^
  "} " ^
  "if($killed.Count -eq 0){ Write-Output '[INFO] No listening process found on ports 8000/8080.' }"

echo [DONE]
endlocal
