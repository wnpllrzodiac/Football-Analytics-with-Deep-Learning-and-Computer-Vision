# Linux 编译和测试指南

本指南适用于 Ubuntu/Debian 和 CentOS/RHEL 系统。

---

## 📋 系统要求

- **操作系统：** Ubuntu 20.04+ / Debian 11+ / CentOS 8+ / RHEL 8+
- **编译器：** GCC 9+ 或 Clang 10+
- **CMake：** 3.15+
- **Python：** 3.8+ (用于 API 服务器和模型转换)

---

## 🚀 快速开始（3 步）

```bash
# 1. 安装依赖
./install_deps_linux.sh

# 2. 编译项目
./build_linux.sh

# 3. 运行测试
./run_test_linux.sh
```

---

## 📦 步骤 1: 安装依赖

### Ubuntu/Debian

```bash
# 更新包列表
sudo apt update

# 安装基础工具
sudo apt install -y \
    build-essential \
    cmake \
    git \
    wget \
    curl \
    pkg-config \
    python3 \
    python3-pip

# 安装 FFmpeg 开发库
sudo apt install -y \
    libavcodec-dev \
    libavformat-dev \
    libavutil-dev \
    libswscale-dev \
    libavdevice-dev

# 安装 OpenCV
sudo apt install -y \
    libopencv-dev \
    python3-opencv

# 检查 OpenCV 版本（应该是 4.x）
pkg-config --modversion opencv4
```

### CentOS/RHEL

```bash
# 启用 EPEL 和 PowerTools
sudo dnf install -y epel-release
sudo dnf config-manager --set-enabled powertools

# 安装基础工具
sudo dnf install -y \
    gcc \
    gcc-c++ \
    cmake \
    git \
    wget \
    curl \
    pkgconfig \
    python3 \
    python3-pip

# 安装 FFmpeg 开发库
sudo dnf install -y \
    ffmpeg-devel

# 安装 OpenCV
sudo dnf install -y \
    opencv-devel \
    python3-opencv
```

### 安装 Python 依赖

```bash
# 模型转换需要
pip3 install --user \
    ultralytics \
    torch \
    onnx \
    onnxruntime

# API 服务器需要
pip3 install --user \
    flask \
    flask-cors
```

---

## 📥 步骤 2: 下载 ONNX Runtime

### 选项 A：使用预编译版本（推荐）

```bash
cd ~/Downloads

# 下载 ONNX Runtime 1.23.2 (Linux x64)
wget https://github.com/microsoft/onnxruntime/releases/download/v1.23.2/onnxruntime-linux-x64-1.23.2.tgz

# 解压到 /opt
sudo mkdir -p /opt/onnxruntime
sudo tar -xzf onnxruntime-linux-x64-1.23.2.tgz -C /opt/onnxruntime --strip-components=1

# 设置环境变量（添加到 ~/.bashrc）
echo 'export ONNXRUNTIME_DIR=/opt/onnxruntime' >> ~/.bashrc
echo 'export LD_LIBRARY_PATH=/opt/onnxruntime/lib:$LD_LIBRARY_PATH' >> ~/.bashrc
source ~/.bashrc
```

### 选项 B：从源码编译（高级用户）

```bash
git clone --recursive https://github.com/microsoft/onnxruntime.git
cd onnxruntime
./build.sh --config Release --build_shared_lib --parallel
```

---

## 🔧 步骤 3: 配置 CMake

### 修改 CMakeLists.txt

Linux 路径和库名称与 Windows 不同，需要条件编译：

```bash
cd cpp
nano CMakeLists.txt
```

确保有以下 Linux 特定配置：

```cmake
# 检测操作系统
if(UNIX AND NOT APPLE)
    set(LINUX TRUE)
endif()

# Linux 特定配置
if(LINUX)
    # OpenCV
    find_package(OpenCV 4 REQUIRED)
    include_directories(${OpenCV_INCLUDE_DIRS})
    
    # FFmpeg
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(FFMPEG REQUIRED
        libavcodec
        libavformat
        libavutil
        libswscale
    )
    include_directories(${FFMPEG_INCLUDE_DIRS})
    link_directories(${FFMPEG_LIBRARY_DIRS})
    
    # ONNX Runtime
    set(ONNXRUNTIME_DIR "/opt/onnxruntime" CACHE PATH "ONNX Runtime directory")
    include_directories(${ONNXRUNTIME_DIR}/include)
    link_directories(${ONNXRUNTIME_DIR}/lib)
    
    # 链接库
    target_link_libraries(football_analytics
        ${OpenCV_LIBS}
        ${FFMPEG_LIBRARIES}
        onnxruntime
        pthread
        dl
    )
endif()

# Windows 特定配置（保持原有）
if(WIN32)
    # ... (保持原有 Windows 配置)
endif()
```

---

## 🛠️ 步骤 4: 编译项目

### 创建编译脚本

```bash
cd cpp
nano build_linux.sh
```

内容：

```bash
#!/bin/bash
set -e

echo "==================================="
echo "Football Analytics - Linux Build"
echo "==================================="

# 创建构建目录
mkdir -p build
cd build

# 配置 CMake
echo "[1/3] Configuring CMake..."
cmake .. \
    -DCMAKE_BUILD_TYPE=Release \
    -DONNXRUNTIME_DIR=/opt/onnxruntime

# 编译
echo "[2/3] Building project..."
cmake --build . --config Release -j$(nproc)

# 检查可执行文件
echo "[3/3] Checking executable..."
if [ -f "./football_analytics" ]; then
    echo "[SUCCESS] Build complete!"
    echo "Executable: ./build/football_analytics"
else
    echo "[ERROR] Build failed - executable not found"
    exit 1
fi

echo "==================================="
```

保存并添加执行权限：

```bash
chmod +x build_linux.sh
```

### 运行编译

```bash
./build_linux.sh
```

**预期输出：**
```
===================================
Football Analytics - Linux Build
===================================
[1/3] Configuring CMake...
-- The C compiler identification is GNU 11.4.0
-- The CXX compiler identification is GNU 11.4.0
-- Found OpenCV: /usr (found version "4.5.4")
-- Found FFMPEG
-- Configuring done
-- Generating done
[2/3] Building project...
[ 16%] Building CXX object CMakeFiles/football_analytics.dir/src/main.cpp.o
[ 33%] Building CXX object CMakeFiles/football_analytics.dir/src/YOLODetector.cpp.o
[ 50%] Building CXX object CMakeFiles/football_analytics.dir/src/ApiClient.cpp.o
[ 66%] Building CXX object CMakeFiles/football_analytics.dir/src/VideoReader.cpp.o
[ 83%] Building CXX object CMakeFiles/football_analytics.dir/src/CoordinateTransform.cpp.o
[100%] Linking CXX executable football_analytics
[SUCCESS] Build complete!
Executable: ./build/football_analytics
===================================
```

---

## 🔄 步骤 5: 转换模型

```bash
cd cpp

# 使用英文版脚本（避免编码问题）
python3 convert_models_en.py
```

**预期输出：**
```
====================================
YOLO Model Conversion to ONNX
====================================

[1/2] Converting Players Detection Model
----------------------------------------
Input:  ../models/Yolo8L Players/weights/best.pt
Output: models/players.onnx

Ultralytics YOLOv8.0.0 🚀
Export success ✅ 2.5s
✓ Players model converted

[2/2] Converting Field Keypoints Model
----------------------------------------
Input:  ../models/Yolo8M Field Keypoints/weights/best.pt
Output: models/keypoints.onnx

Export success ✅ 1.8s
✓ Keypoints model converted

====================================
✓ All models converted successfully!
====================================
```

---

## 🧪 步骤 6: 测试运行

### 6.1 启动 API 服务器

**终端 1：**

```bash
cd cpp
python3 simple_api_server.py
```

**预期输出：**
```
 * Serving Flask app 'simple_api_server'
 * Debug mode: off
WARNING: This is a development server.
 * Running on http://127.0.0.1:5000
Press CTRL+C to quit
```

### 6.2 运行 C++ 程序

**终端 2：**

```bash
cd cpp/build
./football_analytics --video "../Streamlit web app/demo_vid_1.mp4"
```

**预期输出：**
```
Initializing ONNX Runtime...
  Creating ONNX Runtime environment...
  Environment created successfully
  Creating session options...
  Using CPU execution provider
  Session options configured
  Loading model file...
  Model loaded successfully

Loading detection models...
  Players model loaded: models/players.onnx
  Keypoints model loaded: models/keypoints.onnx

Opening video: ../Streamlit web app/demo_vid_1.mp4
Video info:
  Resolution: 1920x1080
  FPS: 30.0
  Total frames: 3000

Processing video...
  Frame 1/3000 (0.0%)
  Frame 31/3000 (1.0%)
```

---

## 🐛 常见问题

### 1. `libonnxruntime.so.1.23.2: cannot open shared object file`

**原因：** 动态链接库路径未设置

**解决：**
```bash
# 临时设置
export LD_LIBRARY_PATH=/opt/onnxruntime/lib:$LD_LIBRARY_PATH

# 永久设置（添加到 ~/.bashrc）
echo 'export LD_LIBRARY_PATH=/opt/onnxruntime/lib:$LD_LIBRARY_PATH' >> ~/.bashrc
source ~/.bashrc

# 或者使用 ldconfig（需要 root）
echo '/opt/onnxruntime/lib' | sudo tee /etc/ld.so.conf.d/onnxruntime.conf
sudo ldconfig
```

### 2. `opencv4/opencv2/core.hpp: No such file or directory`

**原因：** OpenCV 未安装或路径不对

**解决：**
```bash
# 检查 OpenCV
pkg-config --modversion opencv4

# 如果没有 opencv4，尝试 opencv
pkg-config --modversion opencv

# 重新安装
sudo apt install libopencv-dev
```

### 3. `libavcodec.so: undefined reference`

**原因：** FFmpeg 库链接顺序问题

**解决：** 修改 CMakeLists.txt 中的链接顺序：
```cmake
target_link_libraries(football_analytics
    ${OpenCV_LIBS}
    avformat
    avcodec
    avutil
    swscale
    onnxruntime
    pthread
    dl
)
```

### 4. `Permission denied` when running executable

**解决：**
```bash
chmod +x build/football_analytics
```

---

## 🚀 GPU 加速（CUDA on Linux）

### 安装 CUDA（Ubuntu）

```bash
# 安装 NVIDIA 驱动
sudo apt install nvidia-driver-525

# 添加 CUDA 仓库
wget https://developer.download.nvidia.com/compute/cuda/repos/ubuntu2204/x86_64/cuda-keyring_1.0-1_all.deb
sudo dpkg -i cuda-keyring_1.0-1_all.deb
sudo apt update

# 安装 CUDA 11.8
sudo apt install cuda-11-8

# 设置环境变量
echo 'export PATH=/usr/local/cuda-11.8/bin:$PATH' >> ~/.bashrc
echo 'export LD_LIBRARY_PATH=/usr/local/cuda-11.8/lib64:$LD_LIBRARY_PATH' >> ~/.bashrc
source ~/.bashrc
```

### 下载 ONNX Runtime GPU 版本

```bash
wget https://github.com/microsoft/onnxruntime/releases/download/v1.23.2/onnxruntime-linux-x64-gpu-1.23.2.tgz
sudo tar -xzf onnxruntime-linux-x64-gpu-1.23.2.tgz -C /opt/onnxruntime-gpu --strip-components=1

# 更新环境变量
export ONNXRUNTIME_DIR=/opt/onnxruntime-gpu
export LD_LIBRARY_PATH=/opt/onnxruntime-gpu/lib:$LD_LIBRARY_PATH
```

### 启用 CUDA 代码

编辑 `src/YOLODetector.cpp`，取消 CUDA 代码注释（参考 `GPU_ACCELERATION.md`）。

### 重新编译

```bash
./build_linux.sh
```

---

## 📁 项目结构（Linux）

```
cpp/
├── build/                      # 编译输出（Linux）
│   ├── football_analytics     # 可执行文件（无 .exe 扩展名）
│   └── CMakeFiles/
├── models/
│   ├── players.onnx
│   └── keypoints.onnx
├── build_linux.sh              # ✅ Linux 编译脚本
├── run_test_linux.sh           # ✅ Linux 测试脚本
├── install_deps_linux.sh       # ✅ 依赖安装脚本
└── simple_api_server.py        # API 服务器（跨平台）
```

---

## 🔧 完整的辅助脚本

### install_deps_linux.sh

```bash
#!/bin/bash
set -e

echo "Installing dependencies for Football Analytics..."

# 检测发行版
if [ -f /etc/os-release ]; then
    . /etc/os-release
    OS=$ID
else
    echo "Cannot detect OS"
    exit 1
fi

case $OS in
    ubuntu|debian)
        sudo apt update
        sudo apt install -y \
            build-essential cmake git wget curl pkg-config \
            python3 python3-pip \
            libavcodec-dev libavformat-dev libavutil-dev libswscale-dev \
            libopencv-dev python3-opencv
        ;;
    centos|rhel|fedora)
        sudo dnf install -y \
            gcc gcc-c++ cmake git wget curl pkgconfig \
            python3 python3-pip \
            ffmpeg-devel opencv-devel python3-opencv
        ;;
    *)
        echo "Unsupported OS: $OS"
        exit 1
        ;;
esac

# Python 依赖
pip3 install --user ultralytics torch onnx onnxruntime flask flask-cors

# ONNX Runtime
if [ ! -d "/opt/onnxruntime" ]; then
    echo "Downloading ONNX Runtime..."
    wget https://github.com/microsoft/onnxruntime/releases/download/v1.23.2/onnxruntime-linux-x64-1.23.2.tgz
    sudo mkdir -p /opt/onnxruntime
    sudo tar -xzf onnxruntime-linux-x64-1.23.2.tgz -C /opt/onnxruntime --strip-components=1
    rm onnxruntime-linux-x64-1.23.2.tgz
fi

# 设置环境变量
if ! grep -q "ONNXRUNTIME_DIR" ~/.bashrc; then
    echo 'export ONNXRUNTIME_DIR=/opt/onnxruntime' >> ~/.bashrc
    echo 'export LD_LIBRARY_PATH=/opt/onnxruntime/lib:$LD_LIBRARY_PATH' >> ~/.bashrc
fi

echo "✓ Dependencies installed successfully!"
echo "Run: source ~/.bashrc"
```

### run_test_linux.sh

```bash
#!/bin/bash
set -e

echo "==================================="
echo "Football Analytics - Test Run"
echo "==================================="

# 检查可执行文件
if [ ! -f "build/football_analytics" ]; then
    echo "[ERROR] Executable not found. Please run ./build_linux.sh first."
    exit 1
fi

# 检查模型
if [ ! -f "models/players.onnx" ] || [ ! -f "models/keypoints.onnx" ]; then
    echo "[WARNING] ONNX models not found. Converting..."
    python3 convert_models_en.py
fi

# 检查测试视频
TEST_VIDEO="../Streamlit web app/demo_vid_1.mp4"
if [ ! -f "$TEST_VIDEO" ]; then
    echo "[WARNING] Test video not found: $TEST_VIDEO"
    echo "Please provide a video file."
    read -p "Enter video path: " TEST_VIDEO
fi

# 运行
echo "[INFO] Starting API server in background..."
python3 simple_api_server.py &
SERVER_PID=$!
sleep 2

echo "[INFO] Running C++ program..."
cd build
./football_analytics --video "$TEST_VIDEO"

# 清理
echo "[INFO] Stopping API server..."
kill $SERVER_PID

echo "==================================="
echo "[SUCCESS] Test completed!"
echo "==================================="
```

保存并添加执行权限：

```bash
chmod +x install_deps_linux.sh build_linux.sh run_test_linux.sh
```

---

## ✅ 完整测试流程

```bash
# 1. 克隆项目（如果还没有）
cd ~/git
git clone <repository-url>
cd Football-Analytics-with-Deep-Learning-and-Computer-Vision/cpp

# 2. 安装依赖
./install_deps_linux.sh
source ~/.bashrc

# 3. 编译项目
./build_linux.sh

# 4. 转换模型
python3 convert_models_en.py

# 5. 测试运行
./run_test_linux.sh
```

---

## 🐳 Docker 支持（可选）

创建 `Dockerfile`：

```dockerfile
FROM ubuntu:22.04

# 安装依赖
RUN apt update && apt install -y \
    build-essential cmake git wget \
    python3 python3-pip \
    libavcodec-dev libavformat-dev libavutil-dev libswscale-dev \
    libopencv-dev

# 下载 ONNX Runtime
RUN wget https://github.com/microsoft/onnxruntime/releases/download/v1.23.2/onnxruntime-linux-x64-1.23.2.tgz && \
    mkdir -p /opt/onnxruntime && \
    tar -xzf onnxruntime-linux-x64-1.23.2.tgz -C /opt/onnxruntime --strip-components=1 && \
    rm onnxruntime-linux-x64-1.23.2.tgz

# 设置环境变量
ENV ONNXRUNTIME_DIR=/opt/onnxruntime
ENV LD_LIBRARY_PATH=/opt/onnxruntime/lib:$LD_LIBRARY_PATH

# 复制项目
WORKDIR /app
COPY . .

# 编译
RUN cd cpp && ./build_linux.sh

# 运行
CMD ["./cpp/run_test_linux.sh"]
```

构建和运行：

```bash
docker build -t football-analytics .
docker run -it --rm -v $(pwd)/videos:/videos football-analytics
```

---

## 📊 性能对比（Linux vs Windows）

| 平台 | 编译器 | FPS (CPU) | 备注 |
|------|-------|-----------|------|
| **Linux** | GCC 11 | 10-20 | 通常比 Windows 快 20-30% |
| **Windows** | MSVC 2022 | 8-15 | |

Linux 性能通常更好，因为：
- ✅ 更高效的系统调用
- ✅ 更好的内存管理
- ✅ 更优化的库

---

## 📝 总结

**Linux 编译步骤：**
1. ✅ 安装依赖（FFmpeg, OpenCV, ONNX Runtime）
2. ✅ 配置 CMake（Linux 特定路径）
3. ✅ 编译项目（`build_linux.sh`）
4. ✅ 转换模型（`convert_models_en.py`）
5. ✅ 运行测试（`run_test_linux.sh`）

**与 Windows 的主要区别：**
- 📦 包管理器安装依赖（apt/dnf）
- 🔗 动态库扩展名（`.so` vs `.dll`）
- 🛠️ Bash 脚本（`.sh` vs `.bat`）
- 📁 路径分隔符（`/` vs `\`）
- ⚙️ 环境变量（`LD_LIBRARY_PATH` vs `PATH`）

**优势：**
- ✅ 更简单的依赖管理
- ✅ 更好的性能
- ✅ 更稳定的环境
- ✅ 更容易部署到服务器

---

**相关文档：**
- `INSTALL_WINDOWS.md` - Windows 安装指南
- `GPU_ACCELERATION.md` - GPU 加速配置
- `QUICKSTART.md` - 快速开始指南
