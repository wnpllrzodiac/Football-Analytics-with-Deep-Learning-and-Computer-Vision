@echo off
REM Football Analytics C++ Build Script for Windows

echo ========================================
echo Football Analytics C++ Build Script
echo ========================================
echo.

REM 检查 cpp-httplib 是否已下载
echo Checking cpp-httplib...
for %%F in (third_party\httplib.h) do set HTTPLIB_SIZE=%%~zF
if %HTTPLIB_SIZE% LSS 100000 (
    echo.
    echo WARNING: cpp-httplib appears to be a placeholder!
    echo Current size: %HTTPLIB_SIZE% bytes
    echo Expected size: 700KB+
    echo.
    echo Please download the real cpp-httplib library:
    echo   Option 1: Run download_httplib.ps1
    echo   Option 2: See HTTPLIB_SETUP.md for manual steps
    echo.
    choice /C YN /M "Continue anyway (compilation may fail)"
    if errorlevel 2 (
        echo Build cancelled.
        pause
        exit /b 1
    )
)
echo cpp-httplib check: OK
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
set VCPKG_ROOT=D:\git\vcpkg
set ONNXRUNTIME_DIR=C:\onnxruntime-win-x64-1.23.2
set FFMPEG_DIR=D:\sdk\ffmpeg-8.0.1-full_build-shared
set OPENCV_DIR=C:\OpenCV4.10.0\opencv\build

echo.
echo Configuration:
echo   VCPKG_ROOT=%VCPKG_ROOT%
echo   ONNXRUNTIME_DIR=%ONNXRUNTIME_DIR%
echo   FFMPEG_DIR=%FFMPEG_DIR%
echo   OPENCV_DIR=%OPENCV_DIR%
echo.

REM 配置CMake
echo Configuring CMake...
cmake .. ^
  -DCMAKE_TOOLCHAIN_FILE=%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake ^
  -DONNXRUNTIME_DIR=%ONNXRUNTIME_DIR% ^
  -DFFMPEG_DIR=%FFMPEG_DIR% ^
  -DOPENCV_DIR=%OPENCV_DIR% ^
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
