@echo off
setlocal

net session >nul 2>&1
if errorlevel 1 (
    echo [!] Administrator privileges required.
    exit /b 1
)

set "SERVICE_NAME=DCRCVDRV_U"

sc query %SERVICE_NAME% >nul 2>&1
if errorlevel 1 (
    echo [*] Service %SERVICE_NAME% is not installed.
    exit /b 0
)

echo [*] Stopping %SERVICE_NAME%...
sc stop %SERVICE_NAME%
timeout /t 2 /nobreak >nul

echo [*] Deleting %SERVICE_NAME%...
sc delete %SERVICE_NAME%

echo [+] Service removed.
exit /b 0
