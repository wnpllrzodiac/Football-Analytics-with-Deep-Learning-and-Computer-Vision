@echo off
echo ================================================================================
echo   Quick Rebuild (只编译，不配置)
echo ================================================================================
echo.

cd build

echo [1/2] Building project...
cmake --build . --config Release

if %errorlevel% neq 0 (
    echo.
    echo [ERROR] Build failed!
    echo.
    pause
    exit /b 1
)

echo.
echo [2/2] Checking executable...
if exist Release\football_analytics.exe (
    echo [SUCCESS] Build complete!
    echo.
    echo Executable: build\Release\football_analytics.exe
    echo.
) else (
    echo [ERROR] Executable not found!
    exit /b 1
)

echo ================================================================================
echo   Build successful! Ready to run.
echo ================================================================================
echo.
pause
