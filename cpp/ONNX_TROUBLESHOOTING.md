# ONNX Runtime 异常排查指南

## 问题：ONNX Runtime 初始化时抛出异常

```cpp
env_ = std::make_unique<Ort::Env>(ORT_LOGGING_LEVEL_WARNING, "FootballAnalytics");
// Exception thrown here
```

---

## 🔍 常见原因与解决方案

### 1. onnxruntime.dll 找不到或版本不匹配

#### 症状
- 程序启动时立即崩溃
- 错误信息: "找不到 onnxruntime.dll" 或 "无法加载 DLL"

#### 检查方法
```powershell
# 检查 DLL 是否存在
Test-Path C:\onnxruntime-win-x64-1.23.2\lib\onnxruntime.dll

# 检查 DLL 是否在可执行文件目录
Test-Path build\Release\onnxruntime.dll

# 检查系统 PATH
$env:PATH -split ';' | Where-Object { $_ -like "*onnxruntime*" }
```

#### 解决方案
```powershell
# 方案 1: 复制 DLL 到可执行文件目录
copy C:\onnxruntime-win-x64-1.23.2\lib\onnxruntime.dll build\Release\

# 方案 2: 添加到 PATH
$env:PATH += ";C:\onnxruntime-win-x64-1.23.2\lib"

# 方案 3: 永久添加到系统 PATH（需要管理员权限）
[Environment]::SetEnvironmentVariable("Path", $env:Path + ";C:\onnxruntime-win-x64-1.23.2\lib", [System.EnvironmentVariableTarget]::Machine)
```

---

### 2. 头文件和 DLL 版本不匹配

#### 症状
- 编译成功但运行时崩溃
- 异常发生在 ONNX Runtime 初始化

#### 检查方法
```powershell
# 检查头文件版本
Select-String -Path "C:\onnxruntime-win-x64-1.23.2\include\onnxruntime_c_api.h" -Pattern "ORT_API_VERSION"

# 检查 DLL 版本
Get-Item C:\onnxruntime-win-x64-1.23.2\lib\onnxruntime.dll | Select-Object VersionInfo
```

#### 解决方案
1. **重新下载匹配版本的 ONNX Runtime**
   - 访问: https://github.com/microsoft/onnxruntime/releases
   - 下载与头文件版本匹配的包

2. **更新 CMakeLists.txt 中的路径**
   ```cmake
   set(ONNXRUNTIME_DIR "C:/onnxruntime-win-x64-1.23.2" CACHE PATH "ONNX Runtime installation directory")
   ```

---

### 3. 链接错误（.lib 文件问题）

#### 症状
- 编译时出现链接错误
- 无法解析的外部符号

#### 检查方法
```powershell
# 检查 .lib 文件是否存在
Test-Path C:\onnxruntime-win-x64-1.23.2\lib\onnxruntime.lib

# 检查文件大小（应该 > 1MB）
Get-Item C:\onnxruntime-win-x64-1.23.2\lib\onnxruntime.lib | Select-Object Length
```

#### 解决方案
重新配置 CMake：
```powershell
cd cpp
rmdir /s /q build
mkdir build
cd build
cmake .. -DONNXRUNTIME_DIR=C:/onnxruntime-win-x64-1.23.2
```

---

### 4. CPU/GPU 版本不匹配

#### 症状
- 运行时崩溃
- 错误信息涉及 CUDA 或 DirectML

#### 解决方案
确保使用正确的 ONNX Runtime 版本：

**如果没有 NVIDIA GPU，使用 CPU 版本：**
- 文件名: `onnxruntime-win-x64-*.zip` (不带 gpu 后缀)

**如果有 NVIDIA GPU，使用 GPU 版本：**
- 文件名: `onnxruntime-win-x64-gpu-*.zip`
- 需要安装 CUDA Toolkit

---

### 5. 模型文件问题

#### 症状
- 环境初始化成功，但加载模型时崩溃
- 在第 32 行 `Session` 创建时异常

#### 检查方法
```powershell
# 检查模型文件是否存在
Test-Path cpp\models\players.onnx
Test-Path cpp\models\keypoints.onnx

# 检查模型文件大小（应该 > 10MB）
Get-Item cpp\models\*.onnx | Select-Object Name, Length
```

#### 解决方案
如果模型文件不存在或损坏：
```powershell
cd cpp
python convert_models_en.py
```

---

## 🛠️ 调试步骤

### 步骤 1: 添加详细错误信息

修改 `YOLODetector.cpp`：

```cpp
YOLODetector::YOLODetector(const std::string& modelPath,
                           float confThreshold,
                           float iouThreshold)
    : inputSize_(640, 640)
    , confThreshold_(confThreshold)
    , iouThreshold_(iouThreshold)
{
    try {
        std::cout << "Initializing ONNX Runtime environment..." << std::endl;
        
        // 创建ONNX Runtime环境
        env_ = std::make_unique<Ort::Env>(ORT_LOGGING_LEVEL_WARNING, "FootballAnalytics");
        std::cout << "Environment created successfully" << std::endl;
        
        // 创建会话选项
        std::cout << "Creating session options..." << std::endl;
        sessionOptions_ = std::make_unique<Ort::SessionOptions>();
        sessionOptions_->SetIntraOpNumThreads(4);
        sessionOptions_->SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_ALL);
        std::cout << "Session options created" << std::endl;
        
        // 创建会话
        std::cout << "Loading model: " << modelPath << std::endl;
#ifdef _WIN32
        std::wstring wideModelPath(modelPath.begin(), modelPath.end());
        session_ = std::make_unique<Ort::Session>(*env_, wideModelPath.c_str(), *sessionOptions_);
#else
        session_ = std::make_unique<Ort::Session>(*env_, modelPath.c_str(), *sessionOptions_);
#endif
        std::cout << "Model loaded successfully" << std::endl;
        
        // ... rest of code
        
    } catch (const Ort::Exception& e) {
        std::cerr << "ONNX Runtime error: " << e.what() << std::endl;
        std::cerr << "Error code: " << e.GetOrtErrorCode() << std::endl;
        throw std::runtime_error("Failed to initialize YOLODetector: " + std::string(e.what()));
    } catch (const std::exception& e) {
        std::cerr << "Standard exception: " << e.what() << std::endl;
        throw;
    } catch (...) {
        std::cerr << "Unknown exception occurred" << std::endl;
        throw;
    }
}
```

### 步骤 2: 运行诊断脚本

创建 `diagnose_onnx.ps1`：

```powershell
Write-Host "ONNX Runtime Diagnostic Tool" -ForegroundColor Cyan
Write-Host "=" * 60

$ONNXRUNTIME_DIR = "C:\onnxruntime-win-x64-1.23.2"

# 1. Check DLL
Write-Host "`n[1] Checking onnxruntime.dll..." -ForegroundColor Yellow
$dllPath = "$ONNXRUNTIME_DIR\lib\onnxruntime.dll"
if (Test-Path $dllPath) {
    $dll = Get-Item $dllPath
    Write-Host "  Found: $dllPath" -ForegroundColor Green
    Write-Host "  Size: $([math]::Round($dll.Length/1MB, 2)) MB" -ForegroundColor Green
    Write-Host "  Version: $($dll.VersionInfo.FileVersion)" -ForegroundColor Green
} else {
    Write-Host "  NOT FOUND: $dllPath" -ForegroundColor Red
}

# 2. Check .lib
Write-Host "`n[2] Checking onnxruntime.lib..." -ForegroundColor Yellow
$libPath = "$ONNXRUNTIME_DIR\lib\onnxruntime.lib"
if (Test-Path $libPath) {
    $lib = Get-Item $libPath
    Write-Host "  Found: $libPath" -ForegroundColor Green
    Write-Host "  Size: $([math]::Round($lib.Length/1KB, 2)) KB" -ForegroundColor Green
} else {
    Write-Host "  NOT FOUND: $libPath" -ForegroundColor Red
}

# 3. Check headers
Write-Host "`n[3] Checking header files..." -ForegroundColor Yellow
$headerPath = "$ONNXRUNTIME_DIR\include\onnxruntime_cxx_api.h"
if (Test-Path $headerPath) {
    Write-Host "  Found: $headerPath" -ForegroundColor Green
    $version = Select-String -Path "$ONNXRUNTIME_DIR\include\onnxruntime_c_api.h" -Pattern "ORT_API_VERSION" | Select-Object -First 1
    Write-Host "  $version" -ForegroundColor Green
} else {
    Write-Host "  NOT FOUND: $headerPath" -ForegroundColor Red
}

# 4. Check if DLL is accessible
Write-Host "`n[4] Checking DLL accessibility..." -ForegroundColor Yellow
$exePath = ".\build\Release\football_analytics.exe"
if (Test-Path $exePath) {
    $exeDir = Split-Path $exePath
    $localDll = Join-Path $exeDir "onnxruntime.dll"
    
    if (Test-Path $localDll) {
        Write-Host "  DLL found in executable directory" -ForegroundColor Green
    } else {
        Write-Host "  DLL NOT in executable directory" -ForegroundColor Yellow
        Write-Host "  Checking PATH..." -ForegroundColor Yellow
        
        $pathDirs = $env:PATH -split ';'
        $found = $false
        foreach ($dir in $pathDirs) {
            if (Test-Path "$dir\onnxruntime.dll" -ErrorAction SilentlyContinue) {
                Write-Host "  Found in PATH: $dir" -ForegroundColor Green
                $found = $true
                break
            }
        }
        
        if (-not $found) {
            Write-Host "  NOT in PATH" -ForegroundColor Red
            Write-Host "  SOLUTION: Copy DLL or add to PATH" -ForegroundColor Yellow
        }
    }
}

# 5. Check model files
Write-Host "`n[5] Checking model files..." -ForegroundColor Yellow
$models = @("cpp\models\players.onnx", "cpp\models\keypoints.onnx")
foreach ($model in $models) {
    if (Test-Path $model) {
        $modelFile = Get-Item $model
        Write-Host "  Found: $model" -ForegroundColor Green
        Write-Host "    Size: $([math]::Round($modelFile.Length/1MB, 2)) MB" -ForegroundColor Green
    } else {
        Write-Host "  NOT FOUND: $model" -ForegroundColor Red
        Write-Host "    Run: python convert_models_en.py" -ForegroundColor Yellow
    }
}

Write-Host "`n" + ("=" * 60)
Write-Host "Diagnostic complete!" -ForegroundColor Cyan
```

运行诊断：
```powershell
.\diagnose_onnx.ps1
```

---

## 📝 快速修复清单

按顺序检查：

- [ ] **DLL 文件存在**: `C:\onnxruntime-win-x64-1.23.2\lib\onnxruntime.dll`
- [ ] **DLL 可访问**: 在 `build\Release\` 或 PATH 中
- [ ] **版本匹配**: 头文件和 DLL 版本一致
- [ ] **模型文件存在**: `cpp\models\players.onnx` 和 `keypoints.onnx`
- [ ] **模型文件有效**: 文件大小 > 10MB
- [ ] **正确的 ONNX Runtime 类型**: CPU 或 GPU 版本

---

## 🚀 推荐解决流程

```powershell
# 1. 复制 DLL 到可执行文件目录
copy C:\onnxruntime-win-x64-1.23.2\lib\onnxruntime.dll build\Release\

# 2. 验证模型文件
dir cpp\models\*.onnx

# 3. 如果模型不存在，转换模型
cd cpp
python convert_models_en.py

# 4. 重新运行程序
cd ..\build\Release
.\football_analytics.exe --video test.mp4
```

---

## 📞 获取更多帮助

如果问题仍然存在：

1. **运行诊断脚本**: `.\diagnose_onnx.ps1`
2. **查看详细错误**: 在代码中添加 try-catch 打印详细信息
3. **检查事件查看器**: Windows 事件日志可能有更多信息
4. **提供错误信息**: 完整的异常消息和堆栈跟踪

---

**常用链接**:
- ONNX Runtime 发布页: https://github.com/microsoft/onnxruntime/releases
- ONNX Runtime 文档: https://onnxruntime.ai/docs/
