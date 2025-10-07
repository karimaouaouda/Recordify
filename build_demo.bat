@echo off
echo =========================
echo Recordify Build Demo
echo =========================
echo.

echo Available make commands:
echo   make all             - Build main executable
echo   make test            - Build unit tests  
echo   make build-module-core - Build core module only
echo   make info-module-core  - Show core module info
echo   make check           - Show project structure
echo   make help            - Show all commands
echo.

echo Project structure optimized for modular screen recorder!
echo.

echo Current modules:
for %%m in (core screen_handler audio_handler video_handler file_manager ui config utils) do (
    echo   - %%m
)
echo.

echo To use: mingw32-make [target] or make [target] (if make is available)
pause
