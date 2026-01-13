# Quick Fix Script for ONNX Runtime Issues

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "ONNX Runtime Quick Fix Tool" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

$ONNXRUNTIME_DIR = "C:\onnxruntime-win-x64-1.23.2"
$BUILD_DIR = ".\build\Release"

# Step 1: Check if ONNX Runtime DLL exists
Write-Host "[1/3] Checking ONNX Runtime installation..." -ForegroundColor Yellow
$dllSource = "$ONNXRUNTIME_DIR\lib\onnxruntime.dll"

if (-not (Test-Path $dllSource)) {
    Write-Host "  ERROR: ONNX Runtime not found at: $ONNXRUNTIME_DIR" -ForegroundColor Red
    Write-Host ""
    Write-Host "Please download ONNX Runtime:" -ForegroundColor Yellow
    Write-Host "  1. Visit: https://github.com/microsoft/onnxruntime/releases" -ForegroundColor White
    Write-Host "  2. Download: onnxruntime-win-x64-*.zip (CPU version)" -ForegroundColor White
    Write-Host "  3. Extract to: $ONNXRUNTIME_DIR" -ForegroundColor White
    Write-Host ""
    exit 1
}

Write-Host "  Found: $dllSource" -ForegroundColor Green
$dll = Get-Item $dllSource
Write-Host "  Size: $([math]::Round($dll.Length/1MB, 2)) MB" -ForegroundColor Green
Write-Host ""

# Step 2: Copy DLL to build directory
Write-Host "[2/3] Copying onnxruntime.dll to build directory..." -ForegroundColor Yellow

if (-not (Test-Path $BUILD_DIR)) {
    Write-Host "  Build directory not found: $BUILD_DIR" -ForegroundColor Yellow
    Write-Host "  Creating directory..." -ForegroundColor Yellow
    New-Item -ItemType Directory -Path $BUILD_DIR -Force | Out-Null
}

try {
    Copy-Item $dllSource $BUILD_DIR -Force
    Write-Host "  Copied to: $BUILD_DIR\onnxruntime.dll" -ForegroundColor Green
    
    # Verify
    $copied = Get-Item "$BUILD_DIR\onnxruntime.dll"
    if ($copied.Length -eq $dll.Length) {
        Write-Host "  Verification: OK" -ForegroundColor Green
    } else {
        Write-Host "  Verification: File size mismatch!" -ForegroundColor Red
    }
} catch {
    Write-Host "  ERROR: Failed to copy DLL" -ForegroundColor Red
    Write-Host "  $_" -ForegroundColor Red
    exit 1
}
Write-Host ""

# Step 3: Copy other dependencies
Write-Host "[3/3] Checking other dependencies..." -ForegroundColor Yellow

$deps = @{
    "OpenCV" = @{
        "source" = "C:\OpenCV4.10.0\opencv\build\x64\vc16\bin\opencv_world4100.dll"
        "name" = "opencv_world4100.dll"
    }
    "FFmpeg avformat" = @{
        "source" = "D:\sdk\ffmpeg-8.0.1-full_build-shared\bin\avformat-61.dll"
        "name" = "avformat-61.dll"
    }
    "FFmpeg avcodec" = @{
        "source" = "D:\sdk\ffmpeg-8.0.1-full_build-shared\bin\avcodec-61.dll"
        "name" = "avcodec-61.dll"
    }
    "FFmpeg avutil" = @{
        "source" = "D:\sdk\ffmpeg-8.0.1-full_build-shared\bin\avutil-59.dll"
        "name" = "avutil-59.dll"
    }
    "FFmpeg swscale" = @{
        "source" = "D:\sdk\ffmpeg-8.0.1-full_build-shared\bin\swscale-8.dll"
        "name" = "swscale-8.dll"
    }
}

$copiedCount = 0
foreach ($name in $deps.Keys) {
    $info = $deps[$name]
    $source = $info.source
    $dest = Join-Path $BUILD_DIR $info.name
    
    if (Test-Path $source) {
        if (-not (Test-Path $dest)) {
            try {
                Copy-Item $source $BUILD_DIR -Force
                Write-Host "  Copied: $($info.name)" -ForegroundColor Green
                $copiedCount++
            } catch {
                Write-Host "  Failed to copy: $($info.name)" -ForegroundColor Yellow
            }
        } else {
            Write-Host "  Already exists: $($info.name)" -ForegroundColor Gray
        }
    } else {
        Write-Host "  Not found: $($info.name)" -ForegroundColor Yellow
        Write-Host "    Source: $source" -ForegroundColor Gray
    }
}

if ($copiedCount -gt 0) {
    Write-Host ""
    Write-Host "  Copied $copiedCount additional DLL(s)" -ForegroundColor Green
}

Write-Host ""

# Summary
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "Fix Complete!" -ForegroundColor Green
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""
Write-Host "Next steps:" -ForegroundColor Yellow
Write-Host "  1. Ensure models are converted:" -ForegroundColor White
Write-Host "     cd cpp && python convert_models_en.py" -ForegroundColor Gray
Write-Host ""
Write-Host "  2. Run the program:" -ForegroundColor White
Write-Host "     cd build\Release" -ForegroundColor Gray
Write-Host "     .\football_analytics.exe --video test.mp4" -ForegroundColor Gray
Write-Host ""
Write-Host "If issues persist, run: .\diagnose_onnx.ps1" -ForegroundColor Yellow
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""
