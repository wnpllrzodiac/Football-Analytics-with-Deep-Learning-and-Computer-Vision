# GPU 加速使用总结

## ✅ 已完成的配置

### 1. Python 环境（venv）
- ✅ **ONNX Runtime GPU 1.23.2** 已安装
- ✅ 支持 **CUDAExecutionProvider** 和 **TensorrtExecutionProvider**
- ✅ **立即可用**，无需额外配置

**验证：**
```bash
cd /home/zodiac/work/git/Football-Analytics-with-Deep-Learning-and-Computer-Vision
source venv/bin/activate
python -c "import onnxruntime as ort; print(ort.get_available_providers())"
# 输出: ['TensorrtExecutionProvider', 'CUDAExecutionProvider', 'CPUExecutionProvider']
```

### 2. C++ 环境
- ✅ **ONNX Runtime GPU 1.16.3** 已下载
  - 位置: `/home/zodiac/work/tools/onnxruntime-gpu`
  - 版本: 适配 CUDA 11.x
- ✅ **cuDNN 8.9.7** 已安装
  - 位置: `/home/zodiac/work/tools/cudnn8-libs`
- ✅ **代码已修改**: `YOLODetector.cpp` 启用 CUDA
- ✅ **CMakeLists.txt** 已更新指向 GPU 路径
- ✅ **运行脚本** 已创建: `run_with_gpu.sh`

---

## 🚀 重新编译并运行（3个步骤）

### 步骤 1: 重新编译（使用 GPU 版本）

使用自动化脚本：

```bash
cd /home/zodiac/work/git/Football-Analytics-with-Deep-Learning-and-Computer-Vision/cpp
chmod +x rebuild_gpu.sh
./rebuild_gpu.sh
```

或手动执行：

```bash
cd /home/zodiac/work/git/Football-Analytics-with-Deep-Learning-and-Computer-Vision/cpp
rm -rf build
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release \
      -DONNXRUNTIME_DIR=/home/zodiac/work/tools/onnxruntime-gpu \
      ..
make -j$(nproc)
```

**预期输出：**
```
-- ONNX Runtime GPU found at: /home/zodiac/work/tools/onnxruntime-gpu
...
[100%] Built target football_analytics
```

### 步骤 2: 验证链接

```bash
cd build
ldd ./football_analytics | grep onnxruntime
```

**应该显示：**
```
libonnxruntime.so.1 => /home/zodiac/work/tools/onnxruntime-gpu/lib/libonnxruntime.so.1
```

### 步骤 3: 使用 GPU 运行

```bash
cd /home/zodiac/work/git/Football-Analytics-with-Deep-Learning-and-Computer-Vision/cpp
chmod +x run_with_gpu.sh
./run_with_gpu.sh --video ../test\ vid.mp4 --player-model ../models/players.onnx
```

**成功的输出应该包含：**
```
✓ GPU 检测成功
NVIDIA RTX A4000, 16376 MiB

Initializing ONNX Runtime...
  Attempting to use CUDA (GPU)...
  ✓ CUDA provider enabled (GPU acceleration)  ← 关键！
  Model loaded successfully
```

---

## 📁 关键文件和工具

### 新创建的文件

```
cpp/
├── rebuild_gpu.sh              # GPU 重新编译脚本
├── run_with_gpu.sh            # GPU 运行脚本
├── GPU_SETUP_GUIDE.md         # 完整配置指南
├── GPU_USAGE_SUMMARY.md       # 本文档
└── install_deps_linux.sh      # 已更新支持 GPU 安装

/home/zodiac/work/tools/
├── onnxruntime-gpu/           # ONNX Runtime 1.16.3 (CUDA 11)
│   ├── lib/
│   │   ├── libonnxruntime.so.1.16.3
│   │   └── libonnxruntime_providers_cuda.so (361MB)
│   └── include/
└── cudnn8-libs/               # cuDNN 8.9.7
    ├── libcudnn.so.8.9.7
    ├── libcudnn_cnn_infer.so.8 (538MB)
    └── ... (其他 cuDNN 库)
```

### 修改的文件

- ✅ `cpp/src/YOLODetector.cpp` - 启用 CUDA GPU 加速
- ✅ `cpp/src/TeamPredictor.cpp` - 修复 kmeans 崩溃问题
- ✅ `cpp/CMakeLists.txt` - 指向 GPU 版本 ONNX Runtime
- ✅ `cpp/install_deps_linux.sh` - 添加 GPU 安装选项
- ✅ `cpp/INSTALL_LINUX.md` - 添加 GPU 版本说明

---

## 🔍 故障排查

### 问题 1: 编译时找不到 ONNX Runtime

**症状：**
```
CMake Error: ONNX Runtime not found
```

**解决：**
```bash
# 检查路径
ls /home/zodiac/work/tools/onnxruntime-gpu/

# 如果不存在，重新安装
cd /home/zodiac/work/git/Football-Analytics-with-Deep-Learning-and-Computer-Vision/cpp
bash install_deps_linux.sh
# 选择选项 2) GPU version
```

### 问题 2: 运行时找不到 libcudnn.so.8

**症状：**
```
libcudnn.so.8: cannot open shared object file
```

**解决：**
```bash
# 检查 cuDNN 8 是否存在
ls /home/zodiac/work/tools/cudnn8-libs/libcudnn.so.8

# 使用 run_with_gpu.sh 脚本（自动设置库路径）
./run_with_gpu.sh --video test.mp4
```

### 问题 3: CUDA provider 初始化失败

**症状：**
```
⚠ CUDA not available: ...
✓ Falling back to CPU
```

**解决：**
```bash
# 1. 检查 GPU
nvidia-smi

# 2. 检查 CUDA
nvcc --version

# 3. 检查库依赖
cd /home/zodiac/work/tools/onnxruntime-gpu/lib
ldd libonnxruntime_providers_cuda.so | grep "not found"

# 4. 设置环境变量后运行
export LD_LIBRARY_PATH=/home/zodiac/work/tools/cudnn8-libs:/home/zodiac/work/tools/onnxruntime-gpu/lib:${LD_LIBRARY_PATH}
./build/football_analytics --video test.mp4
```

### 问题 4: 链接到旧的 ONNX Runtime

**症状：**
```
libonnxruntime.so.1 => /home/zodiac/work/tools/onnxruntime/lib/...
```

**解决：**
```bash
# 必须重新编译！
cd cpp
rm -rf build
./rebuild_gpu.sh
```

---

## 📊 性能监控

### 实时监控 GPU 使用

在另一个终端运行：

```bash
watch -n 1 nvidia-smi
```

**正常输出应该显示：**
- GPU 利用率: 80-100%
- 显存使用: 1-2 GB
- 温度: 60-80°C

### 性能基准测试

```bash
# CPU 模式（对比）
time ./build/football_analytics --video test.mp4 > /dev/null

# GPU 模式
time ./run_with_gpu.sh --video test.mp4 > /dev/null
```

**预期加速比：** 5-10倍

---

## 💡 使用技巧

### 1. 批量处理视频

创建批处理脚本：

```bash
#!/bin/bash
for video in videos/*.mp4; do
    echo "Processing: $video"
    ./run_with_gpu.sh --video "$video"
done
```

### 2. 调整显存限制

编辑 `src/YOLODetector.cpp` 第 37 行：

```cpp
cuda_options.gpu_mem_limit = 1ULL * 1024 * 1024 * 1024;  // 1GB
```

### 3. Python 脚本中使用 GPU

```python
import onnxruntime as ort

# 创建 GPU 推理会话
session = ort.InferenceSession(
    "models/players.onnx",
    providers=['CUDAExecutionProvider', 'CPUExecutionProvider']
)

# 检查使用的提供程序
print("Using:", session.get_providers())
# 输出: ['CUDAExecutionProvider', 'CPUExecutionProvider']
```

---

## ✅ 快速检查清单

在运行之前，确认以下项目：

- [ ] NVIDIA 驱动已安装（`nvidia-smi` 有输出）
- [ ] CUDA 11.x 已安装（`nvcc --version` 显示 11.x）
- [ ] ONNX Runtime GPU 已安装（`ls /home/zodiac/work/tools/onnxruntime-gpu`）
- [ ] cuDNN 8 已安装（`ls /home/zodiac/work/tools/cudnn8-libs`）
- [ ] 项目已使用 GPU 版本重新编译（`./rebuild_gpu.sh`）
- [ ] 链接到正确的库（`ldd build/football_analytics | grep onnxruntime-gpu`）
- [ ] 使用 GPU 运行脚本（`./run_with_gpu.sh`）

---

## 📚 相关文档

- **GPU_SETUP_GUIDE.md** - 详细的 GPU 配置指南
- **INSTALL_LINUX.md** - Linux 安装完整指南
- **README.md** - 项目总体说明

---

## 🆘 需要帮助？

如果遇到问题：

1. 查看 **GPU_SETUP_GUIDE.md** 的"故障排查"部分
2. 确认所有环境变量正确设置
3. 验证所有依赖库都已安装
4. 确保项目已重新编译

**关键命令：**
```bash
# 完整重新安装和编译流程
cd /home/zodiac/work/git/Football-Analytics-with-Deep-Learning-and-Computer-Vision/cpp
bash install_deps_linux.sh  # 选择 GPU 版本
./rebuild_gpu.sh
./run_with_gpu.sh --video ../test\ vid.mp4
```

---

**最后更新：** 2026-01-14  
**GPU 支持版本：** ONNX Runtime 1.16.3 + CUDA 11.5 + cuDNN 8.9.7
