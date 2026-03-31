@echo off
setlocal EnableExtensions

set "SCRIPT_DIR=%~dp0"
for %%I in ("%SCRIPT_DIR%..") do set "PROJECT_ROOT=%%~fI"

set "UPROJECT=%PROJECT_ROOT%\DdProject.uproject"
set "ENGINE_ROOT=C:\Program Files\Epic Games\UE_5.7"
set "RUN_UAT=%ENGINE_ROOT%\Engine\Build\BatchFiles\RunUAT.bat"
set "ARCHIVE_DIR=C:\DdGame"
set "LOG_DIR=%SCRIPT_DIR%Logs"
set "BUILD_CONFIG=DebugGame"
set "TARGET_PLATFORM=Win64"
set "BUILD_EXIT_CODE=1"
set "PACKAGED_SAVED_DIR=%ARCHIVE_DIR%\Windows\DdProject\Saved"
set "PACKAGED_USER_SETTINGS=%PACKAGED_SAVED_DIR%\Config\Windows\GameUserSettings.ini"

if not exist "%LOG_DIR%" (
	mkdir "%LOG_DIR%"
)

for /f "usebackq delims=" %%I in (`powershell -NoProfile -ExecutionPolicy Bypass -Command "(Get-Date).ToString('yyyyMMdd_HHmmss')"`) do set "BUILD_STAMP=%%I"

if not defined BUILD_STAMP (
	set "BUILD_STAMP=unknown_timestamp"
)

set "LOG_FILE=%LOG_DIR%\BuildWindowClient_%BUILD_STAMP%.log"

if not exist "%UPROJECT%" (
	echo [Error] Could not find project file: %UPROJECT%
	(
		echo [Error] Could not find project file: %UPROJECT%
	) > "%LOG_FILE%"
	echo [Info] Log file: %LOG_FILE%
	pause
	exit /b 1
)

if not exist "%RUN_UAT%" (
	echo [Error] Could not find RunUAT.bat: %RUN_UAT%
	(
		echo [Error] Could not find RunUAT.bat: %RUN_UAT%
	) > "%LOG_FILE%"
	echo [Info] Log file: %LOG_FILE%
	pause
	exit /b 1
)

(
	echo [Info] BuildWindowClient started: %DATE% %TIME%
	echo [Info] Project: %UPROJECT%
	echo [Info] Archive directory: %ARCHIVE_DIR%
	echo [Info] Configuration: %BUILD_CONFIG%
	echo [Info] Platform: %TARGET_PLATFORM%
	echo.
) > "%LOG_FILE%"

if exist "%PACKAGED_SAVED_DIR%" (
	echo [Info] Removing stale packaged saved data: %PACKAGED_SAVED_DIR%
	echo [Info] Removing stale packaged saved data: %PACKAGED_SAVED_DIR%>> "%LOG_FILE%"
	rmdir /s /q "%PACKAGED_SAVED_DIR%"
)

echo [Info] Building and packaging DdProject to %ARCHIVE_DIR%
echo [Info] Build log: %LOG_FILE%
echo [Info] The command window will stay open after the build completes.
echo.

(
	echo [Info] Building and packaging DdProject to %ARCHIVE_DIR%
	echo [Info] Build log: %LOG_FILE%
	echo [Info] The command window will stay open after the build completes.
	echo.
) >> "%LOG_FILE%"

powershell -NoProfile -ExecutionPolicy Bypass -Command ^
	"$logFile = '%LOG_FILE%';" ^
	"$uatPath = '%RUN_UAT%';" ^
	"$uatArgs = @(" ^
	"  'BuildCookRun'," ^
	"  '-project=%UPROJECT%'," ^
	"  '-target=DdProject'," ^
	"  '-platform=%TARGET_PLATFORM%'," ^
	"  '-clientconfig=%BUILD_CONFIG%'," ^
	"  '-build'," ^
	"  '-cook'," ^
	"  '-stage'," ^
	"  '-package'," ^
	"  '-archive'," ^
	"  '-archivedirectory=%ARCHIVE_DIR%'," ^
	"  '-pak'," ^
	"  '-prereqs'," ^
	"  '-utf8output'," ^
	"  '-noP4'" ^
	");" ^
	"& $uatPath @uatArgs 2>&1 | Tee-Object -FilePath $logFile -Append;" ^
	"exit $LASTEXITCODE"

set "BUILD_EXIT_CODE=%ERRORLEVEL%"

if exist "%PACKAGED_USER_SETTINGS%" (
	echo [Info] Removing packaged GameUserSettings override: %PACKAGED_USER_SETTINGS%
	echo [Info] Removing packaged GameUserSettings override: %PACKAGED_USER_SETTINGS%>> "%LOG_FILE%"
	del /f /q "%PACKAGED_USER_SETTINGS%"
)

echo.>> "%LOG_FILE%"
if "%BUILD_EXIT_CODE%"=="0" (
	echo [Success] BuildWindowClient completed successfully.>> "%LOG_FILE%"
	echo [Success] BuildWindowClient completed successfully.
) else (
	echo [Error] BuildWindowClient failed with exit code %BUILD_EXIT_CODE%.>> "%LOG_FILE%"
	echo [Error] BuildWindowClient failed with exit code %BUILD_EXIT_CODE%.
)

echo [Info] Log file: %LOG_FILE%
pause
exit /b %BUILD_EXIT_CODE%
