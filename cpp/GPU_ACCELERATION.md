# GPU 加速配置指南

## 问题：CUDA Provider 错误

### 错误信息
```
Error loading "onnxruntime_providers_cuda.dll" which is missing.
(Error 126: "The specified module could not be found.")
```

### 原因
这个错误说明 ONNX Runtime 试图使用 CUDA（GPU 加速），但是：
- ❌ 没有找到 CUDA provider DLL
- ❌ 或者没有安装 CUDA/cuDNN

---

## ✅ 解决方案 1：使用 CPU（已应用）

**最简单的方法，兼容性最好。**

### 当前状态
✅ 代码已修改为**默认使用 CPU**，不会再尝试加载 CUDA。

### 重新编译
```powershell
cd D:\git\Football-Analytics-with-Deep-Learning-and-Computer-Vision\cpp
.\rebuild.bat
```

### 预期结果
```
Initializing ONNX Runtime...
  Model path: models/players.onnx
  Creating ONNX Runtime environment...
  Environment created successfully
  Creating session options...
  Using CPU execution provider          ← 使用 CPU
  Session options configured
  Loading model file...
  Model loaded successfully
```

### 性能
- ✅ **兼容性**：任何 Windows 电脑都能运行
- ✅ **稳定性**：不需要额外依赖
- ⚠️ **速度**：比 GPU 慢 3-10 倍

对于实时分析可能较慢，但对于离线处理视频完全够用。

---

## 🚀 解决方案 2：启用 GPU 加速（可选）

如果你有 NVIDIA GPU 并想要更快的推理速度：

### 步骤 1: 检查系统要求

**硬件要求：**
- ✅ NVIDIA GPU（支持 CUDA）
- ✅ 至少 4GB 显存

**检查 GPU：**
```powershell
nvidia-smi
```

如果显示 GPU 信息，说明有 NVIDIA GPU。

### 步骤 2: 安装 CUDA 和 cuDNN

1. **下载 CUDA 11.8**（推荐）
   - 官网：https://developer.nvidia.com/cuda-11-8-0-download-archive
   - 选择 Windows x86_64
   - 下载并安装（约 3GB）

2. **下载 cuDNN 8.9**
   - 官网：https://developer.nvidia.com/cudnn
   - 需要注册 NVIDIA 账号
   - 下载 cuDNN for CUDA 11.x
   - 解压并复制文件到 CUDA 目录：
     ```
     cudnn\bin\*.dll    → C:\Program Files\NVIDIA GPU Computing Toolkit\CUDA\v11.8\bin
     cudnn\include\*.h  → C:\Program Files\NVIDIA GPU Computing Toolkit\CUDA\v11.8\include
     cudnn\lib\*.lib    → C:\Program Files\NVIDIA GPU Computing Toolkit\CUDA\v11.8\lib\x64
     ```

### 步骤 3: 下载 ONNX Runtime GPU 版本

**当前使用：** ONNX Runtime 1.23.2 (CPU only)

**需要下载：** ONNX Runtime GPU 版本

1. 访问：https://github.com/microsoft/onnxruntime/releases/tag/v1.23.2
2. 下载 `onnxruntime-win-x64-gpu-1.23.2.zip`
3. 解压到：`C:\onnxruntime-win-x64-gpu-1.23.2`

### 步骤 4: 更新 CMakeLists.txt

修改 ONNX Runtime 路径：

```cmake
# 旧的（CPU 版本）
set(ONNXRUNTIME_DIR "C:/onnxruntime-win-x64-1.23.2" CACHE PATH "ONNX Runtime directory")

# 新的（GPU 版本）
set(ONNXRUNTIME_DIR "C:/onnxruntime-win-x64-gpu-1.23.2" CACHE PATH "ONNX Runtime directory")
```

### 步骤 5: 启用 CUDA 代码

编辑 `src/YOLODetector.cpp`，取消这部分的注释：

```cpp
/* 要启用 CUDA (GPU)，取消下面的注释：
try {
    std::cout << "  Attempting to use CUDA (GPU)..." << std::endl;
    OrtCUDAProviderOptions cuda_options;
    sessionOptions_->AppendExecutionProvider_CUDA(cuda_options);
    std::cout << "  ✓ CUDA provider enabled (GPU acceleration)" << std::endl;
} catch (const std::exception& e) {
    std::cout << "  ⚠ CUDA not available: " << e.what() << std::endl;
    std::cout << "  ✓ Falling back to CPU" << std::endl;
}
*/
```

改为：

```cpp
// CUDA (GPU) 加速已启用
try {
    std::cout << "  Attempting to use CUDA (GPU)..." << std::endl;
    OrtCUDAProviderOptions cuda_options;
    sessionOptions_->AppendExecutionProvider_CUDA(cuda_options);
    std::cout << "  ✓ CUDA provider enabled (GPU acceleration)" << std::endl;
} catch (const std::exception& e) {
    std::cout << "  ⚠ CUDA not available: " << e.what() << std::endl;
    std::cout << "  ✓ Falling back to CPU" << std::endl;
}
```

### 步骤 6: 重新编译

```powershell
cd cpp
.\build.bat
```

### 步骤 7: 复制 CUDA DLLs

```powershell
# 复制 ONNX Runtime CUDA DLLs
copy C:\onnxruntime-win-x64-gpu-1.23.2\lib\*.dll build\Release\

# 复制 CUDA Runtime DLLs（如果需要）
copy "C:\Program Files\NVIDIA GPU Computing Toolkit\CUDA\v11.8\bin\cudart64_*.dll" build\Release\
copy "C:\Program Files\NVIDIA GPU Computing Toolkit\CUDA\v11.8\bin\cublas64_*.dll" build\Release\
copy "C:\Program Files\NVIDIA GPU Computing Toolkit\CUDA\v11.8\bin\cublasLt64_*.dll" build\Release\
copy "C:\Program Files\NVIDIA GPU Computing Toolkit\CUDA\v11.8\bin\cudnn64_*.dll" build\Release\
```

### 步骤 8: 测试

```powershell
cd build\Release
.\football_analytics.exe --video test.mp4
```

预期输出：
```
Initializing ONNX Runtime...
  Attempting to use CUDA (GPU)...
  ✓ CUDA provider enabled (GPU acceleration)
  Model loaded successfully
```

---

## 🆚 性能对比

| 执行方式 | 速度（FPS） | 依赖 | 兼容性 |
|---------|-----------|------|--------|
| **CPU** | 5-15 | 最少 | ✅ 所有电脑 |
| **CUDA** | 50-150 | CUDA+cuDNN | ⚠️ 仅 NVIDIA GPU |
| **DirectML** | 20-60 | Windows 10+ | ✅ 大多数 GPU |

---

## 🔍 模型是通用的吗？

### ✅ 是的！ONNX 模型是通用的

同一个 `.onnx` 模型文件可以在不同硬件上运行：

```
models/players.onnx
  ↓
  ├─ CPU (任何电脑)
  ├─ CUDA (NVIDIA GPU)
  ├─ DirectML (Windows GPU)
  ├─ CoreML (Apple 芯片)
  └─ TensorRT (NVIDIA GPU，优化版)
```

### ⚠️ 但是 ONNX Runtime 有不同版本

| 版本 | 文件名 | 支持的硬件 |
|------|--------|-----------|
| CPU | `onnxruntime.dll` | 所有 |
| GPU | `onnxruntime.dll` + `onnxruntime_providers_cuda.dll` | NVIDIA GPU |
| DirectML | `onnxruntime.dll` + `onnxruntime_providers_dml.dll` | Windows GPU |

**你当前使用的是 CPU 版本**，所以不包含 CUDA provider。

---

## 📊 当前推荐配置

### 选项 A：CPU 模式（当前默认）

**优点：**
- ✅ 零额外依赖
- ✅ 最高兼容性
- ✅ 稳定可靠

**适合：**
- 离线处理视频
- 没有 NVIDIA GPU
- 快速部署

### 选项 B：CUDA 模式（高性能）

**优点：**
- 🚀 5-10 倍速度提升
- 🚀 支持实时处理

**缺点：**
- ❌ 需要 NVIDIA GPU
- ❌ 需要安装 CUDA/cuDNN（~5GB）
- ❌ 配置复杂

**适合：**
- 实时分析
- 大量视频处理
- 有 NVIDIA GPU

---

## ✅ 快速检查清单

**当前状态（CPU 模式）：**
- [x] 代码已修改为使用 CPU
- [ ] 重新编译（运行 `rebuild.bat`）
- [ ] 测试运行

**如果要启用 GPU：**
- [ ] 检查有 NVIDIA GPU
- [ ] 安装 CUDA 11.8
- [ ] 安装 cuDNN 8.9
- [ ] 下载 ONNX Runtime GPU 版本
- [ ] 更新 CMakeLists.txt
- [ ] 取消 CUDA 代码注释
- [ ] 重新编译
- [ ] 复制 DLLs
- [ ] 测试

---

## 🆘 常见问题

### Q: 我必须使用 GPU 吗？
**A:** 不需要。CPU 模式完全够用，只是慢一些。

### Q: 不同 CUDA 版本可以吗？
**A:** 可以，但需要匹配：
- CUDA 11.x → cuDNN 8.x → ONNX Runtime built for CUDA 11
- CUDA 12.x → cuDNN 9.x → ONNX Runtime built for CUDA 12

### Q: 可以使用 AMD GPU 吗？
**A:** 可以，使用 DirectML provider（Windows）或 ROCm（Linux）。

### Q: TensorRT 呢？
**A:** TensorRT 更快，但需要：
1. 先将 ONNX 转为 TensorRT 引擎（`.engine` 文件）
2. 使用 TensorRT 执行（而非 ONNX Runtime）
3. 引擎文件只能在特定 GPU 上运行（不可移植）

---

## 📝 总结

**现在的修复：**
✅ 代码已改为**默认使用 CPU**
✅ 不会再尝试加载 CUDA DLLs
✅ 兼容所有 Windows 电脑

**只需重新编译即可：**
```powershell
.\rebuild.bat
```

**如果将来需要 GPU 加速，按照本文档的"解决方案 2"操作。**

---

**相关文档：**
- `ONNX_TROUBLESHOOTING.md` - ONNX Runtime 问题排查
- `INSTALL_WINDOWS.md` - 依赖安装指南
- `config/inference.json` - 推理配置（预留）
