# 下载 cpp-httplib 库
# 这是一个header-only库，只需要下载单个头文件

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "下载 cpp-httplib 库" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

$HTTPLIB_URL = "https://raw.githubusercontent.com/yhirose/cpp-httplib/master/httplib.h"
$OUTPUT_FILE = "$PSScriptRoot\third_party\httplib.h"
$BACKUP_FILE = "$PSScriptRoot\third_party\httplib.h.backup"

# 备份现有文件
if (Test-Path $OUTPUT_FILE) {
    Write-Host "备份现有文件..." -ForegroundColor Yellow
    Copy-Item $OUTPUT_FILE $BACKUP_FILE -Force
}

Write-Host "正在下载 cpp-httplib..." -ForegroundColor Yellow
Write-Host "URL: $HTTPLIB_URL" -ForegroundColor Gray
Write-Host ""

try {
    # 使用 Invoke-WebRequest 下载
    $ProgressPreference = 'SilentlyContinue'  # 禁用进度条以提高速度
    Invoke-WebRequest -Uri $HTTPLIB_URL -OutFile $OUTPUT_FILE -UseBasicParsing
    
    # 检查文件大小
    $fileSize = (Get-Item $OUTPUT_FILE).Length
    
    if ($fileSize -gt 100000) {  # 应该至少有100KB
        Write-Host "✅ 下载成功！" -ForegroundColor Green
        Write-Host ""
        Write-Host "文件信息:" -ForegroundColor Cyan
        Write-Host "  路径: $OUTPUT_FILE" -ForegroundColor White
        Write-Host "  大小: $([math]::Round($fileSize / 1KB, 2)) KB" -ForegroundColor White
        Write-Host ""
        Write-Host "现在可以重新编译项目了！" -ForegroundColor Green
        Write-Host "运行: .\build.bat" -ForegroundColor Yellow
    } else {
        throw "下载的文件大小异常"
    }
    
} catch {
    Write-Host "❌ 下载失败: $_" -ForegroundColor Red
    Write-Host ""
    Write-Host "请手动下载:" -ForegroundColor Yellow
    Write-Host "  1. 访问: https://github.com/yhirose/cpp-httplib" -ForegroundColor White
    Write-Host "  2. 下载 httplib.h" -ForegroundColor White
    Write-Host "  3. 保存到: $OUTPUT_FILE" -ForegroundColor White
    Write-Host ""
    Write-Host "或使用 vcpkg 安装:" -ForegroundColor Yellow
    Write-Host "  vcpkg install cpp-httplib:x64-windows" -ForegroundColor White
    
    # 恢复备份
    if (Test-Path $BACKUP_FILE) {
        Write-Host ""
        Write-Host "恢复备份文件..." -ForegroundColor Yellow
        Copy-Item $BACKUP_FILE $OUTPUT_FILE -Force
    }
    
    exit 1
}

Write-Host ""
Write-Host "========================================" -ForegroundColor Cyan
