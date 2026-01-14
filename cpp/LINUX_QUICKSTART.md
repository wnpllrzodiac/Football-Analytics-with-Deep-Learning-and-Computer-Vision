# Linux 快速开始 - 5 分钟上手

## 🚀 一键安装和运行

```bash
# 克隆项目
cd ~/git
git clone <repository-url>
cd Football-Analytics-with-Deep-Learning-and-Computer-Vision/cpp

# 赋予脚本执行权限
chmod +x *.sh

# 1. 安装依赖（首次运行，约 5-10 分钟）
./install_deps_linux.sh
source ~/.bashrc

# 2. 编译项目（约 1-2 分钟）
./build_linux.sh

# 3. 转换模型（首次运行，约 1 分钟）
python3 convert_models_en.py

# 4. 运行测试
./run_test_linux.sh
```

**就这么简单！** 🎉

---

## 📋 系统要求

| 项目 | 要求 |
|------|------|
| **OS** | Ubuntu 20.04+, Debian 11+, CentOS 8+, RHEL 8+ |
| **CPU** | x86_64 (Intel/AMD) |
| **内存** | 4GB+ |
| **磁盘** | 10GB+ 可用空间 |
| **Python** | 3.8+ |

---

## 🔧 手动安装（逐步）

### 步骤 1: 安装系统依赖

**Ubuntu/Debian:**
```bash
sudo apt update
sudo apt install -y \
    build-essential cmake git wget curl pkg-config \
    python3 python3-pip \
    libavcodec-dev libavformat-dev libavutil-dev libswscale-dev \
    libopencv-dev python3-opencv
```

**CentOS/RHEL:**
```bash
sudo dnf install -y epel-release
sudo dnf install -y \
    gcc gcc-c++ cmake git wget curl pkgconfig \
    python3 python3-pip \
    ffmpeg-devel opencv-devel python3-opencv
```

### 步骤 2: 安装 Python 依赖

```bash
pip3 install --user ultralytics torch onnx onnxruntime flask flask-cors
```

### 步骤 3: 安装 ONNX Runtime

```bash
# 下载
wget https://github.com/microsoft/onnxruntime/releases/download/v1.23.2/onnxruntime-linux-x64-1.23.2.tgz

# 安装
sudo mkdir -p /opt/onnxruntime
sudo tar -xzf onnxruntime-linux-x64-1.23.2.tgz -C /opt/onnxruntime --strip-components=1

# 设置环境变量
echo 'export ONNXRUNTIME_DIR=/opt/onnxruntime' >> ~/.bashrc
echo 'export LD_LIBRARY_PATH=/opt/onnxruntime/lib:$LD_LIBRARY_PATH' >> ~/.bashrc
source ~/.bashrc
```

### 步骤 4: 编译项目

```bash
cd cpp
chmod +x build_linux.sh
./build_linux.sh
```

### 步骤 5: 转换模型

```bash
python3 convert_models_en.py
```

### 步骤 6: 运行

```bash
# 终端 1: 启动 API 服务器
python3 simple_api_server.py

# 终端 2: 运行程序
cd build
./football_analytics --video test.mp4
```

---

## 🆚 Linux vs Windows

| 特性 | Linux | Windows |
|------|-------|---------|
| **依赖安装** | ✅ apt/dnf 一键安装 | ⚠️ 手动下载安装 |
| **编译速度** | ✅ 更快 (GCC) | 普通 (MSVC) |
| **运行性能** | ✅ 更好 (20-30% 更快) | 普通 |
| **脚本** | `.sh` (bash) | `.bat` (PowerShell) |
| **动态库** | `.so` | `.dll` |
| **路径分隔符** | `/` | `\` |
| **部署** | ✅ 服务器友好 | ⚠️ 桌面为主 |

---

## 📁 项目结构（Linux）

```
cpp/
├── build/                      # 编译输出
│   └── football_analytics     # 可执行文件（无 .exe）
├── models/
│   ├── players.onnx           # 玩家检测模型
│   └── keypoints.onnx         # 关键点检测模型
├── config/
│   └── pitch_map.json
├── build_linux.sh              # ✨ Linux 编译脚本
├── install_deps_linux.sh       # ✨ 依赖安装脚本
├── run_test_linux.sh           # ✨ 测试脚本
├── simple_api_server.py        # API 服务器（跨平台）
├── convert_models_en.py        # 模型转换（跨平台）
└── INSTALL_LINUX.md            # ✨ Linux 详细文档
```

---

## 🐛 常见问题

### Q: `libonnxruntime.so: cannot open shared object file`

**A:** 设置动态库路径
```bash
export LD_LIBRARY_PATH=/opt/onnxruntime/lib:$LD_LIBRARY_PATH
# 或永久添加到 ~/.bashrc
echo 'export LD_LIBRARY_PATH=/opt/onnxruntime/lib:$LD_LIBRARY_PATH' >> ~/.bashrc
```

### Q: `opencv4/opencv2/core.hpp: No such file or directory`

**A:** 安装 OpenCV
```bash
sudo apt install libopencv-dev  # Ubuntu/Debian
# 或
sudo dnf install opencv-devel   # CentOS/RHEL
```

### Q: `Permission denied` when running scripts

**A:** 添加执行权限
```bash
chmod +x *.sh
```

### Q: 编译错误：`undefined reference to 'pthread_create'`

**A:** 确保 CMakeLists.txt 链接了 pthread（Linux 版本已包含）

---

## 🚀 性能优化

### 使用所有 CPU 核心

编译时自动使用：
```bash
cmake --build . -j$(nproc)
```

### 启用编译器优化

CMakeLists.txt 已配置 Release 模式：
```cmake
-DCMAKE_BUILD_TYPE=Release
```

### GPU 加速（可选）

如果有 NVIDIA GPU：
```bash
# 安装 CUDA
sudo apt install nvidia-driver-525 cuda-11-8

# 下载 ONNX Runtime GPU 版本
wget https://github.com/microsoft/onnxruntime/releases/download/v1.23.2/onnxruntime-linux-x64-gpu-1.23.2.tgz

# 参考 GPU_ACCELERATION.md
```

---

## 📊 预期性能（Linux）

| CPU | FPS (CPU) | FPS (GPU) |
|-----|-----------|-----------|
| Intel i5-10th Gen | 10-15 | 80-120 |
| Intel i7-12th Gen | 15-20 | 100-150 |
| AMD Ryzen 5 5600 | 12-18 | 90-130 |
| Server (Xeon) | 8-12 | 60-100 |

**Linux 通常比 Windows 快 20-30%**

---

## 🐳 Docker 支持

**Dockerfile 示例：**
```dockerfile
FROM ubuntu:22.04

RUN apt update && apt install -y \
    build-essential cmake wget \
    libavcodec-dev libavformat-dev libavutil-dev libswscale-dev \
    libopencv-dev python3 python3-pip

WORKDIR /app
COPY . .
RUN cd cpp && ./install_deps_linux.sh && ./build_linux.sh

CMD ["./cpp/run_test_linux.sh"]
```

**构建和运行：**
```bash
docker build -t football-analytics .
docker run -v $(pwd)/videos:/videos football-analytics
```

---

## ✅ 检查清单

使用前确认：

- [ ] 系统是 Ubuntu/Debian/CentOS/RHEL
- [ ] 已安装依赖（`./install_deps_linux.sh`）
- [ ] 已编译项目（`./build_linux.sh`）
- [ ] 已转换模型（`python3 convert_models_en.py`）
- [ ] 环境变量已设置（`source ~/.bashrc`）
- [ ] 有测试视频（MP4 格式）

**全部完成后，运行：**
```bash
./run_test_linux.sh
```

---

## 📚 更多文档

| 文档 | 用途 |
|------|------|
| **`INSTALL_LINUX.md`** | 详细安装指南 |
| **`GPU_ACCELERATION.md`** | GPU 加速配置 |
| **`CURRENT_STATUS_AND_FIXES.md`** | 最新状态和修复 |
| **`ALL_TOOLS_README.md`** | 所有工具索引 |

---

## 💡 提示

1. **首次运行慢** - 安装依赖需要时间，之后就快了
2. **使用 tmux/screen** - 长时间运行时避免 SSH 断开
3. **日志重定向** - 保存输出：`./football_analytics > log.txt 2>&1`
4. **后台运行** - 使用 `nohup` 或 `systemd`

---

## 🎯 下一步

✅ 完成安装和测试后：

1. **修改配置** - 编辑 `config/pitch_map.json`
2. **处理多个视频** - 编写 bash 脚本批量处理
3. **部署到服务器** - 使用 systemd 或 Docker
4. **性能监控** - 使用 `htop`, `nvidia-smi`

---

**准备好了？开始吧！** 🚀

```bash
./install_deps_linux.sh && ./build_linux.sh && ./run_test_linux.sh
```
