@echo off
REM Quick start script for API server

echo ========================================
echo Football Analytics - API Server
echo ========================================
echo.

REM Check if Flask is installed
python -c "import flask" 2>nul
if %ERRORLEVEL% neq 0 (
    echo Flask not installed. Installing now...
    pip install flask
    if %ERRORLEVEL% neq 0 (
        echo ERROR: Failed to install Flask
        echo Please run manually: pip install flask
        pause
        exit /b 1
    )
)

echo Flask is installed.
echo.
echo Starting API server...
echo Server will run on: http://localhost:8080
echo.
echo Press Ctrl+C to stop the server
echo ========================================
echo.

REM Start the server
python simple_api_server.py

pause
