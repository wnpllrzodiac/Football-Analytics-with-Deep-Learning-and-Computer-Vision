# GPU 加速完整配置指南

## 📊 当前状态

### ✅ 已完成
1. **硬件**: NVIDIA RTX A4000 GPU ✓
2. **驱动**: NVIDIA Driver 580.95.05 ✓
3. **CUDA**: 11.5 已安装 ✓
4. **ONNX Runtime (Python)**: GPU 版本已安装 ✓
   - 版本: 1.16.3 (支持 CUDA 11.x)
   - 位置: `/home/zodiac/work/tools/onnxruntime-gpu`
   - 提供程序: CUDAExecutionProvider, TensorrtExecutionProvider
5. **C++ 代码**: 已修改启用 CUDA ✓

### ⚠️ 待解决
1. **cuDNN 版本不匹配**: 
   - 系统安装: cuDNN 9.13.1
   - ONNX Runtime 需要: cuDNN 8
   - **解决方案**: 见下文

---

## 🔧 解决 cuDNN 版本问题

### 方案 1：创建兼容性符号链接（推荐）

cuDNN 9 通常向后兼容 cuDNN 8 的 API，可以创建符号链接：

```bash
sudo ln -sf /usr/lib/x86_64-linux-gnu/libcudnn.so.9 /usr/lib/x86_64-linux-gnu/libcudnn.so.8
```

**验证:**
```bash
ls -lh /usr/lib/x86_64-linux-gnu/libcudnn.so.8
```

### 方案 2：使用 LD_LIBRARY_PATH（无需 sudo）

如果没有 sudo 权限，使用运行脚本设置库路径：

```bash
cd /home/zodiac/work/git/Football-Analytics-with-Deep-Learning-and-Computer-Vision/cpp
chmod +x run_with_gpu.sh
./run_with_gpu.sh --video ../test\ vid.mp4
```

脚本会自动配置：
- ONNX Runtime GPU 库路径
- CUDA 库路径
- cuDNN 兼容性处理

### 方案 3：禁用 cuDNN 使用纯 CUDA（性能稍差）

修改 `src/YOLODetector.cpp` 的 CUDA 配置：

```cpp
OrtCUDAProviderOptions cuda_options;
cuda_options.device_id = 0;
cuda_options.cudnn_conv_algo_search = OrtCudnnConvAlgoSearchDefault;  // 使用默认算法
// cuda_options.do_copy_in_default_stream = 1;  // 注释掉此行
```

---

## 🚀 重新编译并测试

### 步骤 1: 重新编译

```bash
cd /home/zodiac/work/git/Football-Analytics-with-Deep-Learning-and-Computer-Vision/cpp

# 清理旧的构建
rm -rf build

# 重新构建
bash build_linux.sh
```

### 步骤 2: 使用 GPU 运行脚本测试

```bash
# 方式 1: 使用运行脚本（推荐）
./run_with_gpu.sh --video ../test\ vid.mp4 --player-model ../models/players.onnx

# 方式 2: 手动设置环境变量
export LD_LIBRARY_PATH="/home/zodiac/work/tools/onnxruntime-gpu/lib:/usr/lib/x86_64-linux-gnu:${LD_LIBRARY_PATH}"
./build/football_analytics --video ../test\ vid.mp4
```

### 预期输出

✅ **成功的输出**:
```
Initializing ONNX Runtime...
  Model path: models/players.onnx
  Creating ONNX Runtime environment...
  Environment created successfully
  Creating session options...
  Attempting to use CUDA (GPU)...
  ✓ CUDA provider enabled (GPU acceleration)  ← 关键！
  Session options configured
  Loading model file...
  Model loaded successfully
```

❌ **如果仍然失败**:
```
  ⚠ CUDA not available: ...
  ✓ Falling back to CPU
```
→ 检查库路径和依赖

---

## 🐍 Python 环境中使用 GPU

Python 环境已经配置好 GPU 支持，可以直接使用：

```python
# 激活虚拟环境
cd /home/zodiac/work/git/Football-Analytics-with-Deep-Learning-and-Computer-Vision
source venv/bin/activate

# 验证 GPU 支持
python -c "import onnxruntime as ort; print('Available providers:', ort.get_available_providers())"

# 输出应该包含:
# ['TensorrtExecutionProvider', 'CUDAExecutionProvider', 'CPUExecutionProvider']
```

### 在 Python 代码中使用 GPU

```python
import onnxruntime as ort

# 创建推理会话，指定使用 CUDA
session = ort.InferenceSession(
    "models/players.onnx",
    providers=['CUDAExecutionProvider', 'CPUExecutionProvider']  # CUDA 优先，CPU 作为后备
)

# 检查实际使用的提供程序
print("Using providers:", session.get_providers())
```

---

## 📈 性能对比

| 模式 | FPS (预估) | 显存占用 | 优点 | 缺点 |
|------|-----------|---------|------|------|
| **CPU** | 5-15 | 0 GB | 兼容性好 | 速度慢 |
| **CUDA** | 50-150 | 1-2 GB | 速度快 | 需要配置 |
| **TensorRT** | 100-200 | 1-3 GB | 最快 | 需要模型转换 |

---

## 🔍 故障排查

### 问题 1: libcublasLt.so.11 not found

```bash
# 检查库是否存在
ls -lh /usr/lib/x86_64-linux-gnu/libcublasLt.so.11

# 如果不存在，添加到 LD_LIBRARY_PATH
export LD_LIBRARY_PATH="/usr/lib/x86_64-linux-gnu:${LD_LIBRARY_PATH}"
```

### 问题 2: libcudnn.so.8 not found

```bash
# 方案 A: 创建符号链接 (需要 sudo)
sudo ln -sf /usr/lib/x86_64-linux-gnu/libcudnn.so.9 /usr/lib/x86_64-linux-gnu/libcudnn.so.8

# 方案 B: 使用运行脚本 (无需 sudo)
./run_with_gpu.sh --video test.mp4
```

### 问题 3: CUDA provider 初始化失败

```bash
# 检查 GPU 状态
nvidia-smi

# 检查 CUDA 版本
nvcc --version

# 检查 ONNX Runtime 库
ldd /home/zodiac/work/tools/onnxruntime-gpu/lib/libonnxruntime_providers_cuda.so
```

### 问题 4: 显存不足

修改 `src/YOLODetector.cpp` 中的显存限制：

```cpp
cuda_options.gpu_mem_limit = 1ULL * 1024 * 1024 * 1024;  // 降低到 1GB
```

---

## 📝 关键文件清单

```
cpp/
├── CMakeLists.txt                 # ✅ 已更新指向 GPU 版本
├── src/YOLODetector.cpp          # ✅ 已启用 CUDA
├── run_with_gpu.sh               # ✅ GPU 运行脚本（新建）
├── build_linux.sh                # 构建脚本
└── build/
    └── football_analytics        # 编译后的可执行文件

/home/zodiac/work/tools/
└── onnxruntime-gpu/              # ✅ ONNX Runtime 1.16.3 (CUDA 11)
    ├── lib/
    │   ├── libonnxruntime.so.1.16.3
    │   └── libonnxruntime_providers_cuda.so  # 361MB
    └── include/
```

---

## ✅ 快速测试清单

执行以下命令验证配置：

```bash
# 1. 检查 GPU
nvidia-smi

# 2. 检查 CUDA
nvcc --version

# 3. 检查 Python ONNX Runtime
cd /home/zodiac/work/git/Football-Analytics-with-Deep-Learning-and-Computer-Vision
source venv/bin/activate
python -c "import onnxruntime as ort; print(ort.get_available_providers())"

# 4. 创建 cuDNN 符号链接（需要一次性执行）
sudo ln -sf /usr/lib/x86_64-linux-gnu/libcudnn.so.9 /usr/lib/x86_64-linux-gnu/libcudnn.so.8

# 5. 重新编译 C++ 项目
cd cpp
bash build_linux.sh

# 6. 运行测试
./run_with_gpu.sh --video ../test\ vid.mp4
```

---

## 💡 提示

1. **首次运行可能较慢**: CUDA 需要预热和 JIT 编译
2. **使用 TensorRT 获得最佳性能**: 需要转换模型为 `.engine` 文件
3. **监控 GPU 使用**: 运行时使用 `watch -n 1 nvidia-smi` 监控
4. **批处理优化**: 处理多个视频时保持模型加载在内存中

---

## 📚 相关文档

- [ONNX Runtime GPU 官方文档](https://onnxruntime.ai/docs/execution-providers/CUDA-ExecutionProvider.html)
- [CUDA Toolkit 文档](https://docs.nvidia.com/cuda/)
- [cuDNN 文档](https://docs.nvidia.com/deeplearning/cudnn/)

---

**如有问题，请参考本文档的"故障排查"部分。**
