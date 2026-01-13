# ONNX Runtime Diagnostic Tool

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "ONNX Runtime Diagnostic Tool" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

$ONNXRUNTIME_DIR = "C:\onnxruntime-win-x64-1.23.2"
$allOk = $true

# 1. Check DLL
Write-Host "[1] Checking onnxruntime.dll..." -ForegroundColor Yellow
$dllPath = "$ONNXRUNTIME_DIR\lib\onnxruntime.dll"
if (Test-Path $dllPath) {
    $dll = Get-Item $dllPath
    Write-Host "  Status: FOUND" -ForegroundColor Green
    Write-Host "  Path: $dllPath" -ForegroundColor White
    Write-Host "  Size: $([math]::Round($dll.Length/1MB, 2)) MB" -ForegroundColor White
    if ($dll.VersionInfo.FileVersion) {
        Write-Host "  Version: $($dll.VersionInfo.FileVersion)" -ForegroundColor White
    }
} else {
    Write-Host "  Status: NOT FOUND" -ForegroundColor Red
    Write-Host "  Expected: $dllPath" -ForegroundColor White
    $allOk = $false
}
Write-Host ""

# 2. Check .lib
Write-Host "[2] Checking onnxruntime.lib..." -ForegroundColor Yellow
$libPath = "$ONNXRUNTIME_DIR\lib\onnxruntime.lib"
if (Test-Path $libPath) {
    $lib = Get-Item $libPath
    Write-Host "  Status: FOUND" -ForegroundColor Green
    Write-Host "  Path: $libPath" -ForegroundColor White
    Write-Host "  Size: $([math]::Round($lib.Length/1KB, 2)) KB" -ForegroundColor White
} else {
    Write-Host "  Status: NOT FOUND" -ForegroundColor Red
    Write-Host "  Expected: $libPath" -ForegroundColor White
    $allOk = $false
}
Write-Host ""

# 3. Check headers
Write-Host "[3] Checking header files..." -ForegroundColor Yellow
$headerPath = "$ONNXRUNTIME_DIR\include\onnxruntime_cxx_api.h"
if (Test-Path $headerPath) {
    Write-Host "  Status: FOUND" -ForegroundColor Green
    Write-Host "  Path: $headerPath" -ForegroundColor White
    
    # Try to find version
    $versionFile = "$ONNXRUNTIME_DIR\include\onnxruntime_c_api.h"
    if (Test-Path $versionFile) {
        $versionLine = Select-String -Path $versionFile -Pattern "ORT_API_VERSION" | Select-Object -First 1
        if ($versionLine) {
            Write-Host "  API Version: $($versionLine.Line.Trim())" -ForegroundColor White
        }
    }
} else {
    Write-Host "  Status: NOT FOUND" -ForegroundColor Red
    Write-Host "  Expected: $headerPath" -ForegroundColor White
    $allOk = $false
}
Write-Host ""

# 4. Check if DLL is accessible
Write-Host "[4] Checking DLL accessibility..." -ForegroundColor Yellow
$exePath = ".\build\Release\football_analytics.exe"

if (Test-Path $exePath) {
    $exeDir = Split-Path $exePath -Resolve
    $localDll = Join-Path $exeDir "onnxruntime.dll"
    
    if (Test-Path $localDll) {
        Write-Host "  DLL in executable directory: YES" -ForegroundColor Green
        Write-Host "  Path: $localDll" -ForegroundColor White
    } else {
        Write-Host "  DLL in executable directory: NO" -ForegroundColor Yellow
        Write-Host ""
        Write-Host "  Checking system PATH..." -ForegroundColor Yellow
        
        $pathDirs = $env:PATH -split ';'
        $found = $false
        foreach ($dir in $pathDirs) {
            $testPath = Join-Path $dir "onnxruntime.dll"
            if (Test-Path $testPath -ErrorAction SilentlyContinue) {
                Write-Host "  Found in PATH: $dir" -ForegroundColor Green
                $found = $true
                break
            }
        }
        
        if (-not $found) {
            Write-Host "  DLL NOT in PATH" -ForegroundColor Red
            Write-Host ""
            Write-Host "  SOLUTION:" -ForegroundColor Yellow
            Write-Host "    copy $dllPath build\Release\" -ForegroundColor White
            $allOk = $false
        }
    }
} else {
    Write-Host "  Executable not found (not built yet)" -ForegroundColor Yellow
    Write-Host "  Run build.bat first" -ForegroundColor White
}
Write-Host ""

# 5. Check model files
Write-Host "[5] Checking model files..." -ForegroundColor Yellow
$models = @{
    "cpp\models\players.onnx" = "Player Detection Model"
    "cpp\models\keypoints.onnx" = "Keypoint Detection Model"
}

foreach ($model in $models.Keys) {
    $name = $models[$model]
    if (Test-Path $model) {
        $modelFile = Get-Item $model
        $sizeMB = [math]::Round($modelFile.Length/1MB, 2)
        
        if ($sizeMB -gt 10) {
            Write-Host "  $name : OK" -ForegroundColor Green
            Write-Host "    Path: $model" -ForegroundColor White
            Write-Host "    Size: $sizeMB MB" -ForegroundColor White
        } else {
            Write-Host "  $name : SUSPICIOUS" -ForegroundColor Yellow
            Write-Host "    Path: $model" -ForegroundColor White
            Write-Host "    Size: $sizeMB MB (too small, may be corrupted)" -ForegroundColor Yellow
            $allOk = $false
        }
    } else {
        Write-Host "  $name : NOT FOUND" -ForegroundColor Red
        Write-Host "    Expected: $model" -ForegroundColor White
        Write-Host "    Run: python convert_models_en.py" -ForegroundColor Yellow
        $allOk = $false
    }
}
Write-Host ""

# 6. Check dependencies summary
Write-Host "[6] Dependencies Summary..." -ForegroundColor Yellow
$deps = @{
    "OpenCV" = "C:\OpenCV4.10.0\opencv\build\x64\vc16\bin\opencv_world4100.dll"
    "FFmpeg avformat" = "D:\sdk\ffmpeg-8.0.1-full_build-shared\bin\avformat-61.dll"
    "FFmpeg avcodec" = "D:\sdk\ffmpeg-8.0.1-full_build-shared\bin\avcodec-61.dll"
}

foreach ($dep in $deps.Keys) {
    $path = $deps[$dep]
    if (Test-Path $path) {
        Write-Host "  $dep : OK" -ForegroundColor Green
    } else {
        Write-Host "  $dep : NOT FOUND" -ForegroundColor Yellow
        Write-Host "    Expected: $path" -ForegroundColor White
    }
}
Write-Host ""

# Summary
Write-Host "========================================" -ForegroundColor Cyan
if ($allOk) {
    Write-Host "All checks passed!" -ForegroundColor Green
    Write-Host ""
    Write-Host "If you still get errors, try:" -ForegroundColor Yellow
    Write-Host "  1. Clean rebuild: rmdir /s /q build && build.bat" -ForegroundColor White
    Write-Host "  2. Check main.cpp line numbers in error message" -ForegroundColor White
    Write-Host "  3. Run with --debug flag for more info" -ForegroundColor White
} else {
    Write-Host "Some issues found!" -ForegroundColor Red
    Write-Host ""
    Write-Host "Quick fixes:" -ForegroundColor Yellow
    Write-Host "  1. Copy ONNX Runtime DLL:" -ForegroundColor White
    Write-Host "     copy $dllPath build\Release\" -ForegroundColor Gray
    Write-Host ""
    Write-Host "  2. Convert models if missing:" -ForegroundColor White
    Write-Host "     cd cpp && python convert_models_en.py" -ForegroundColor Gray
    Write-Host ""
    Write-Host "For detailed help, see: ONNX_TROUBLESHOOTING.md" -ForegroundColor White
}
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

# Return exit code
if ($allOk) { exit 0 } else { exit 1 }
