@echo off
setlocal

:: Run this script as Administrator.
net session >nul 2>&1
if errorlevel 1 (
    echo [!] Administrator privileges required.
    echo     Right-click this file and choose "Run as administrator".
    exit /b 1
)

set "DRIVER_DIR=%~dp0"
set "DRIVER_PATH=%DRIVER_DIR%DCRCVDrv.sys"
set "SERVICE_NAME=DCRCVDRV_U"

if not exist "%DRIVER_PATH%" (
    if exist "%DRIVER_DIR%567c158ee0858f8e941d4ab7a6c18dbc.bin" (
        copy /Y "%DRIVER_DIR%567c158ee0858f8e941d4ab7a6c18dbc.bin" "%DRIVER_PATH%" >nul
        echo [*] Copied sample to %DRIVER_PATH%
    ) else (
        echo [-] Driver not found: %DRIVER_PATH%
        exit /b 1
    )
)

sc query %SERVICE_NAME% >nul 2>&1
if not errorlevel 1 (
    echo [*] Service %SERVICE_NAME% already exists. Stopping and removing...
    sc stop %SERVICE_NAME% >nul 2>&1
    timeout /t 2 /nobreak >nul
    sc delete %SERVICE_NAME% >nul 2>&1
    timeout /t 1 /nobreak >nul
)

echo [*] Creating kernel driver service %SERVICE_NAME%...
sc create %SERVICE_NAME% type= kernel binPath= "%DRIVER_PATH%" DisplayName= "DCRCV_U Driver"
if errorlevel 1 exit /b 1

sc description %SERVICE_NAME% "DCRCVDrv.sys lab driver (LolDriver project)"

echo [*] Starting service...
sc start %SERVICE_NAME%
if errorlevel 1 (
    echo [-] sc start failed. Common causes:
    echo     - Driver signature enforcement / blocked vulnerable driver list
    echo     - Test signing required for unsigned drivers
    echo     - Driver file path not accessible to SCM
    exit /b 1
)

echo [+] Service %SERVICE_NAME% is running.
echo [+] Device should be available at \\.\DCRCVDRV_U
echo.
echo Example:
echo     ringkiller.exe ^<pid^>
exit /b 0
