@echo off
REM Football Analytics - Model Setup Script

echo ========================================
echo Football Analytics - Model Setup
echo ========================================
echo.

REM Step 1: Install Python dependencies
echo [1/4] Installing Python dependencies...
pip install ultralytics
if %ERRORLEVEL% neq 0 (
    echo ERROR: Failed to install ultralytics
    echo Please run manually: pip install ultralytics
    pause
    exit /b 1
)
echo OK
echo.

REM Step 2: Convert models
echo [2/4] Converting PyTorch models to ONNX...
python convert_models_en.py
if %ERRORLEVEL% neq 0 (
    echo ERROR: Model conversion failed
    echo Please check error messages above
    pause
    exit /b 1
)
echo OK
echo.

REM Step 3: Copy tactical map
echo [3/4] Copying tactical map...
if not exist resources mkdir resources
if exist "..\tactical map.jpg" (
    copy "..\tactical map.jpg" resources\tactical_map.jpg
    echo OK
) else (
    echo WARNING: tactical map.jpg not found in parent directory
    echo Please copy manually to: resources\tactical_map.jpg
)
echo.

REM Step 4: Verify setup
echo [4/4] Verifying setup...
set ALL_OK=1

if exist models\players.onnx (
    echo   [OK] players.onnx
) else (
    echo   [FAILED] players.onnx not found
    set ALL_OK=0
)

if exist models\keypoints.onnx (
    echo   [OK] keypoints.onnx
) else (
    echo   [FAILED] keypoints.onnx not found
    set ALL_OK=0
)

if exist resources\tactical_map.jpg (
    echo   [OK] tactical_map.jpg
) else (
    echo   [WARNING] tactical_map.jpg not found
)

echo.
echo ========================================
if %ALL_OK%==1 (
    echo Setup Complete!
    echo ========================================
    echo.
    echo Next steps:
    echo   1. Run build.bat to compile the project
    echo   2. Copy DLL files to build\Release\
    echo   3. Run the program
    echo.
) else (
    echo Setup Failed!
    echo ========================================
    echo.
    echo Please check error messages above and try again
    echo.
)

pause
