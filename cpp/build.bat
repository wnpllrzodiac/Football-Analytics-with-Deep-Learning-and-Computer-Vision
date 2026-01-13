@echo off
REM Football Analytics C++ Build Script for Windows

echo ========================================
echo Football Analytics C++ Build Script
echo ========================================
echo.

REM 检查是否存在build目录
if exist build (
    echo Cleaning previous build...
    rmdir /s /q build
)

REM 创建build目录
echo Creating build directory...
mkdir build
cd build

REM 设置变量（根据实际情况修改）
set VCPKG_ROOT=C:\vcpkg
set ONNXRUNTIME_DIR=C:\onnxruntime

echo.
echo Configuration:
echo   VCPKG_ROOT=%VCPKG_ROOT%
echo   ONNXRUNTIME_DIR=%ONNXRUNTIME_DIR%
echo.

REM 配置CMake
echo Configuring CMake...
cmake .. ^
  -DCMAKE_TOOLCHAIN_FILE=%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake ^
  -DONNXRUNTIME_DIR=%ONNXRUNTIME_DIR% ^
  -DCMAKE_BUILD_TYPE=Release

if %ERRORLEVEL% neq 0 (
    echo CMake configuration failed!
    cd ..
    pause
    exit /b 1
)

echo.
echo Building project...
cmake --build . --config Release -j 8

if %ERRORLEVEL% neq 0 (
    echo Build failed!
    cd ..
    pause
    exit /b 1
)

echo.
echo ========================================
echo Build completed successfully!
echo ========================================
echo.
echo Executable location: build\Release\football_analytics.exe
echo.
echo To run the application:
echo   cd build\Release
echo   football_analytics.exe --video path\to\video.mp4
echo.

cd ..
pause
