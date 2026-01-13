# Football Analytics - 依赖检查脚本
# 检查所有必需的依赖库是否正确安装

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "Football Analytics - 依赖检查工具" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

$allOk = $true

# 配置路径
$FFMPEG_DIR = "D:\sdk\ffmpeg-8.0.1-full_build-shared"
$ONNXRUNTIME_DIR = "C:\onnxruntime-win-x64-1.23.2"
$OPENCV_DIR = "C:\OpenCV4.10.0\opencv\build"
$VCPKG_ROOT = "D:\git\vcpkg"
$PROJECT_ROOT = $PSScriptRoot

Write-Host "检查依赖库..." -ForegroundColor Yellow
Write-Host ""

# 1. 检查 FFmpeg
Write-Host "[1/5] 检查 FFmpeg..." -ForegroundColor White
if (Test-Path "$FFMPEG_DIR\include\libavformat\avformat.h") {
    Write-Host "  ✅ FFmpeg 头文件: OK" -ForegroundColor Green
} else {
    Write-Host "  ❌ FFmpeg 头文件: 未找到" -ForegroundColor Red
    $allOk = $false
}

if (Test-Path "$FFMPEG_DIR\lib\avformat.lib") {
    Write-Host "  ✅ FFmpeg 库文件: OK" -ForegroundColor Green
} else {
    Write-Host "  ❌ FFmpeg 库文件: 未找到" -ForegroundColor Red
    $allOk = $false
}

$ffmpegDlls = @("avformat-61.dll", "avcodec-61.dll", "avutil-59.dll", "swscale-8.dll")
$missingDlls = @()
foreach ($dll in $ffmpegDlls) {
    if (-not (Test-Path "$FFMPEG_DIR\bin\$dll")) {
        $missingDlls += $dll
    }
}

if ($missingDlls.Count -eq 0) {
    Write-Host "  ✅ FFmpeg DLL文件: OK ($($ffmpegDlls.Count) files)" -ForegroundColor Green
} else {
    Write-Host "  ❌ FFmpeg DLL文件: 缺失 $($missingDlls -join ', ')" -ForegroundColor Red
    $allOk = $false
}
Write-Host ""

# 2. 检查 ONNX Runtime
Write-Host "[2/5] 检查 ONNX Runtime..." -ForegroundColor White
if (Test-Path "$ONNXRUNTIME_DIR\include\onnxruntime_cxx_api.h") {
    Write-Host "  ✅ ONNX Runtime 头文件: OK" -ForegroundColor Green
} else {
    Write-Host "  ❌ ONNX Runtime 头文件: 未找到" -ForegroundColor Red
    $allOk = $false
}

if (Test-Path "$ONNXRUNTIME_DIR\lib\onnxruntime.lib") {
    Write-Host "  ✅ ONNX Runtime 库文件: OK" -ForegroundColor Green
} else {
    Write-Host "  ❌ ONNX Runtime 库文件: 未找到" -ForegroundColor Red
    $allOk = $false
}

if (Test-Path "$ONNXRUNTIME_DIR\lib\onnxruntime.dll") {
    Write-Host "  ✅ ONNX Runtime DLL: OK" -ForegroundColor Green
} else {
    Write-Host "  ❌ ONNX Runtime DLL: 未找到" -ForegroundColor Red
    $allOk = $false
}
Write-Host ""

# 3. 检查 OpenCV
Write-Host "[3/5] 检查 OpenCV..." -ForegroundColor White
if (Test-Path "$OPENCV_DIR\include\opencv2\opencv.hpp") {
    Write-Host "  ✅ OpenCV 头文件: OK" -ForegroundColor Green
} else {
    Write-Host "  ❌ OpenCV 头文件: 未找到" -ForegroundColor Red
    $allOk = $false
}

$opencvLibPath = "$OPENCV_DIR\x64\vc16\lib\opencv_world4100.lib"
if (Test-Path $opencvLibPath) {
    Write-Host "  ✅ OpenCV 库文件: OK (vc16)" -ForegroundColor Green
} else {
    # 尝试其他可能的路径
    $foundLib = $false
    foreach ($vc in @("vc17", "vc16", "vc15", "vc14")) {
        $testPath = "$OPENCV_DIR\x64\$vc\lib"
        if (Test-Path $testPath) {
            $libs = Get-ChildItem "$testPath\opencv_world*.lib" -ErrorAction SilentlyContinue
            if ($libs) {
                Write-Host "  ✅ OpenCV 库文件: OK ($vc)" -ForegroundColor Green
                $foundLib = $true
                break
            }
        }
    }
    if (-not $foundLib) {
        Write-Host "  ❌ OpenCV 库文件: 未找到" -ForegroundColor Red
        Write-Host "     请检查: $OPENCV_DIR\x64\vc*\lib\" -ForegroundColor Yellow
        $allOk = $false
    }
}

$opencvDllPath = "$OPENCV_DIR\x64\vc16\bin\opencv_world4100.dll"
if (Test-Path $opencvDllPath) {
    Write-Host "  ✅ OpenCV DLL文件: OK" -ForegroundColor Green
} else {
    # 尝试其他可能的路径
    $foundDll = $false
    foreach ($vc in @("vc17", "vc16", "vc15", "vc14")) {
        $testPath = "$OPENCV_DIR\x64\$vc\bin"
        if (Test-Path $testPath) {
            $dlls = Get-ChildItem "$testPath\opencv_world*.dll" -ErrorAction SilentlyContinue
            if ($dlls) {
                Write-Host "  ✅ OpenCV DLL文件: OK ($vc)" -ForegroundColor Green
                $foundDll = $true
                break
            }
        }
    }
    if (-not $foundDll) {
        Write-Host "  ⚠️  OpenCV DLL文件: 未找到" -ForegroundColor Yellow
        Write-Host "     请检查: $OPENCV_DIR\x64\vc*\bin\" -ForegroundColor Yellow
    }
}
Write-Host ""

# 4. 检查 vcpkg (仅用于cpp-httplib)
Write-Host "[4/5] 检查 vcpkg..." -ForegroundColor White
if (Test-Path "$VCPKG_ROOT\vcpkg.exe") {
    Write-Host "  ✅ vcpkg 可执行文件: OK" -ForegroundColor Green
} else {
    Write-Host "  ⚠️  vcpkg 可执行文件: 未找到" -ForegroundColor Yellow
    Write-Host "     (可选) 用于安装cpp-httplib" -ForegroundColor Yellow
}

if (Test-Path "$VCPKG_ROOT\scripts\buildsystems\vcpkg.cmake") {
    Write-Host "  ✅ vcpkg CMake 工具链: OK" -ForegroundColor Green
} else {
    Write-Host "  ⚠️  vcpkg CMake 工具链: 未找到" -ForegroundColor Yellow
}

# 检查cpp-httplib
if (Test-Path "$VCPKG_ROOT\vcpkg.exe") {
    $installedPackages = & "$VCPKG_ROOT\vcpkg.exe" list 2>$null
    
    if ($installedPackages -match "cpp-httplib:x64-windows") {
        Write-Host "  ✅ cpp-httplib (vcpkg): 已安装" -ForegroundColor Green
    } else {
        Write-Host "  ⚠️  cpp-httplib (vcpkg): 未安装" -ForegroundColor Yellow
        Write-Host "     (可选) 用于HTTP通信" -ForegroundColor Yellow
    }
}
Write-Host ""

# 5. 检查项目文件
Write-Host "[5/6] 检查项目文件..." -ForegroundColor White
if (Test-Path "$PROJECT_ROOT\models\players.onnx") {
    $size = (Get-Item "$PROJECT_ROOT\models\players.onnx").Length / 1MB
    Write-Host "  ✅ 球员检测模型: OK ($('{0:N2}' -f $size) MB)" -ForegroundColor Green
} else {
    Write-Host "  ❌ 球员检测模型: 未找到" -ForegroundColor Red
    Write-Host "     请运行: python convert_models.py" -ForegroundColor Yellow
    $allOk = $false
}

if (Test-Path "$PROJECT_ROOT\models\keypoints.onnx") {
    $size = (Get-Item "$PROJECT_ROOT\models\keypoints.onnx").Length / 1MB
    Write-Host "  ✅ 关键点检测模型: OK ($('{0:N2}' -f $size) MB)" -ForegroundColor Green
} else {
    Write-Host "  ❌ 关键点检测模型: 未找到" -ForegroundColor Red
    Write-Host "     请运行: python convert_models.py" -ForegroundColor Yellow
    $allOk = $false
}

if (Test-Path "$PROJECT_ROOT\resources\tactical_map.jpg") {
    Write-Host "  ✅ 战术地图: OK" -ForegroundColor Green
} else {
    Write-Host "  ❌ 战术地图: 未找到" -ForegroundColor Red
    Write-Host "     请复制: copy '..\tactical map.jpg' resources\tactical_map.jpg" -ForegroundColor Yellow
    $allOk = $false
}
Write-Host ""

# 6. 检查构建工具
Write-Host "[6/6] 检查构建工具..." -ForegroundColor White
$cmake = Get-Command cmake -ErrorAction SilentlyContinue
if ($cmake) {
    $cmakeVersion = & cmake --version 2>$null | Select-Object -First 1
    Write-Host "  ✅ CMake: $cmakeVersion" -ForegroundColor Green
} else {
    Write-Host "  ❌ CMake: 未找到" -ForegroundColor Red
    Write-Host "     请安装 CMake: https://cmake.org/download/" -ForegroundColor Yellow
    $allOk = $false
}

$python = Get-Command python -ErrorAction SilentlyContinue
if ($python) {
    $pythonVersion = & python --version 2>$null
    Write-Host "  ✅ Python: $pythonVersion" -ForegroundColor Green
    
    # 检查 Python 包
    $ultralytics = & python -c "import ultralytics; print('OK')" 2>$null
    if ($ultralytics -eq "OK") {
        Write-Host "  ✅ ultralytics (Python): 已安装" -ForegroundColor Green
    } else {
        Write-Host "  ⚠️  ultralytics (Python): 未安装" -ForegroundColor Yellow
        Write-Host "     用于模型转换: pip install ultralytics" -ForegroundColor Yellow
    }
} else {
    Write-Host "  ❌ Python: 未找到" -ForegroundColor Red
    $allOk = $false
}
Write-Host ""

# 总结
Write-Host "========================================" -ForegroundColor Cyan
if ($allOk) {
    Write-Host "✅ 所有依赖检查通过！" -ForegroundColor Green
    Write-Host ""
    Write-Host "下一步:" -ForegroundColor Yellow
    Write-Host "  1. 运行 .\build.bat 编译项目" -ForegroundColor White
    Write-Host "  2. 复制DLL文件到可执行文件目录" -ForegroundColor White
    Write-Host "  3. 运行程序测试" -ForegroundColor White
} else {
    Write-Host "❌ 存在缺失的依赖" -ForegroundColor Red
    Write-Host ""
    Write-Host "请根据上述提示安装缺失的依赖" -ForegroundColor Yellow
    Write-Host ""
    Write-Host "详细说明请参考:" -ForegroundColor Yellow
    Write-Host "  - INSTALL_WINDOWS.md" -ForegroundColor White
    Write-Host "  - CONFIG_REFERENCE.md" -ForegroundColor White
}
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

# 返回状态
if ($allOk) {
    exit 0
} else {
    exit 1
}
