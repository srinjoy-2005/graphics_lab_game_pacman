@echo off
cd /d "%~dp0web"
echo Starting Pacman Maze Web Server at http://localhost:8080 ...
start "" "http://localhost:8080"
python -m http.server 8080
if %errorlevel% neq 0 (
    echo Python not found, trying PowerShell simple server or opening directly...
    start "" "index.html"
)
