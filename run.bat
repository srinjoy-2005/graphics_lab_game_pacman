@echo off
cd /d "%~dp0"
if exist "release\PacmanMaze.exe" (
    cd release
    start "" "PacmanMaze.exe"
    exit /b 0
)
if exist "debug\PacmanMaze.exe" (
    cd debug
    start "" "PacmanMaze.exe"
    exit /b 0
)
echo Executable not found.
pause
