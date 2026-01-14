#!/bin/bash
set -e

echo "=========================================="
echo "Football Analytics - Linux Dependencies"
echo "=========================================="

# 检测发行版
if [ -f /etc/os-release ]; then
    . /etc/os-release
    OS=$ID
    VERSION=$VERSION_ID
    echo "Detected: $OS $VERSION"
else
    echo "[ERROR] Cannot detect OS"
    exit 1
fi

echo ""
echo "[1/4] Installing system packages..."
case $OS in
    ubuntu|debian)
        sudo apt update
        sudo apt install -y \
            build-essential \
            cmake \
            git \
            wget \
            curl \
            pkg-config \
            python3 \
            python3-pip \
            libavcodec-dev \
            libavformat-dev \
            libavutil-dev \
            libswscale-dev \
            libavdevice-dev \
            libopencv-dev \
            python3-opencv
        ;;
    centos|rhel|fedora)
        sudo dnf install -y epel-release
        sudo dnf config-manager --set-enabled powertools 2>/dev/null || true
        sudo dnf install -y \
            gcc \
            gcc-c++ \
            cmake \
            git \
            wget \
            curl \
            pkgconfig \
            python3 \
            python3-pip \
            ffmpeg-devel \
            opencv-devel \
            python3-opencv
        ;;
    *)
        echo "[ERROR] Unsupported OS: $OS"
        echo "Supported: Ubuntu, Debian, CentOS, RHEL, Fedora"
        exit 1
        ;;
esac

echo ""
echo "[2/4] Installing Python dependencies..."
#pip3 install --user \
#    ultralytics \
#    torch \
#    onnx \
#    onnxruntime \
#    flask \
#    flask-cors

echo ""
echo "[3/4] Installing ONNX Runtime..."
echo "Choose installation type:"
echo "  1) CPU version (default, compatible with all systems)"
echo "  2) GPU version (requires NVIDIA GPU, CUDA 11.x, cuDNN 8)"
read -p "Enter choice [1-2] (default: 1): " onnx_choice
onnx_choice=${onnx_choice:-1}

if [ "$onnx_choice" == "2" ]; then
    # GPU version (CUDA 11.x compatible)
    ONNX_VERSION="1.16.3"
    ONNX_URL="https://github.com/microsoft/onnxruntime/releases/download/v${ONNX_VERSION}/onnxruntime-linux-x64-gpu-${ONNX_VERSION}.tgz"
    ONNX_INSTALL_DIR="/home/zodiac/work/tools/onnxruntime-gpu"
    echo "Installing ONNX Runtime GPU ${ONNX_VERSION} (CUDA 11.x)..."
    
    # Install cuDNN 8 (required for ONNX Runtime GPU)
    CUDNN8_DIR="/home/zodiac/work/tools/cudnn8-libs"
    CUDNN_VERSION="8.9.7.29"
    CUDNN_FILE="cudnn-linux-x86_64-${CUDNN_VERSION}_cuda11-archive.tar.xz"
    CUDNN_URL="https://developer.download.nvidia.com/compute/cudnn/redist/cudnn/linux-x86_64/${CUDNN_FILE}"
    
    if [ ! -d "$CUDNN8_DIR" ]; then
        echo ""
        echo "Downloading cuDNN 8.9.7 for CUDA 11..."
        echo "Note: This is a large download (~1.1 GB)"
        echo "Source: ${CUDNN_URL}"
        
        cd /tmp
        
        # Download with progress bar
        if wget --show-progress "$CUDNN_URL" 2>&1 | grep -q "failed"; then
            echo "Error: Failed to download cuDNN 8"
            echo "Please download manually from:"
            echo "  https://developer.nvidia.com/cudnn"
            echo "And extract to: $CUDNN8_DIR"
            exit 1
        fi
        
        echo "Extracting cuDNN 8..."
        tar -xf "$CUDNN_FILE"
        
        if [ $? -ne 0 ]; then
            echo "Error: Failed to extract cuDNN archive"
            exit 1
        fi
        
        echo "Installing cuDNN 8 libraries..."
        mkdir -p "$CUDNN8_DIR"
        cp -r cudnn-linux-x86_64-${CUDNN_VERSION}_cuda11-archive/lib/* "$CUDNN8_DIR/"
        
        # Verify installation
        if [ -f "$CUDNN8_DIR/libcudnn.so.8" ]; then
            echo "✓ cuDNN 8.9.7 installed successfully"
            echo "  Location: $CUDNN8_DIR"
            echo "  Size: $(du -sh $CUDNN8_DIR | cut -f1)"
            ls -lh "$CUDNN8_DIR/libcudnn.so.8"* | head -3
        else
            echo "Error: cuDNN installation verification failed"
            exit 1
        fi
        
        # Cleanup
        rm -rf cudnn-linux-x86_64-${CUDNN_VERSION}_cuda11-archive*
        echo "✓ Cleaned up temporary files"
    else
        echo "cuDNN 8 already installed at $CUDNN8_DIR"
        # Verify existing installation
        if [ -f "$CUDNN8_DIR/libcudnn.so.8" ]; then
            echo "✓ Verified: libcudnn.so.8 found"
        else
            echo "Warning: cuDNN installation may be incomplete"
            echo "Consider removing $CUDNN8_DIR and reinstalling"
        fi
    fi
    echo ""
else
    # CPU version
    ONNX_VERSION="1.23.2"
    ONNX_URL="https://github.com/microsoft/onnxruntime/releases/download/v${ONNX_VERSION}/onnxruntime-linux-x64-${ONNX_VERSION}.tgz"
    ONNX_INSTALL_DIR="/home/zodiac/work/tools/onnxruntime"
    echo "Installing ONNX Runtime CPU ${ONNX_VERSION}..."
fi

if [ -d "$ONNX_INSTALL_DIR" ]; then
    echo "ONNX Runtime already installed at $ONNX_INSTALL_DIR"
else
    echo "Downloading ONNX Runtime ${ONNX_VERSION}..."
    cd /tmp
    wget -q "$ONNX_URL"
    
    echo "Installing to $ONNX_INSTALL_DIR..."
    mkdir -p "$ONNX_INSTALL_DIR"
    tar -xzf "onnxruntime-linux-x64-*.tgz"
    mv onnxruntime-linux-x64-*/* "$ONNX_INSTALL_DIR/"
    rm -rf onnxruntime-linux-x64-*
    
    echo "✓ ONNX Runtime installed to $ONNX_INSTALL_DIR"
fi

# Update CMakeLists.txt with the correct path
if [ "$onnx_choice" == "2" ]; then
    echo ""
    echo "To use GPU acceleration, update CMakeLists.txt:"
    echo "  set(ONNXRUNTIME_DIR \"$ONNX_INSTALL_DIR\" CACHE PATH \"ONNX Runtime installation directory\")"
    echo ""
    echo "Or rebuild with:"
    echo "  cd build && cmake -DONNXRUNTIME_DIR=$ONNX_INSTALL_DIR .."
fi

echo ""
echo "[4/4] Setting up environment variables..."

# 检查是否已经设置
if ! grep -q "ONNXRUNTIME_DIR" ~/.bashrc; then
    echo "" >> ~/.bashrc
    echo "# Football Analytics - ONNX Runtime" >> ~/.bashrc
    echo "export ONNXRUNTIME_DIR=$ONNX_INSTALL_DIR" >> ~/.bashrc
    
    # GPU 版本需要额外的 cuDNN 路径
    if [ "$onnx_choice" == "2" ]; then
        echo "export CUDNN8_LIB=$CUDNN8_DIR" >> ~/.bashrc
        echo "export LD_LIBRARY_PATH=$CUDNN8_DIR:$ONNX_INSTALL_DIR/lib:\$LD_LIBRARY_PATH" >> ~/.bashrc
        echo "✓ Environment variables added to ~/.bashrc (GPU mode with cuDNN 8)"
    else
        echo "export LD_LIBRARY_PATH=$ONNX_INSTALL_DIR/lib:\$LD_LIBRARY_PATH" >> ~/.bashrc
        echo "✓ Environment variables added to ~/.bashrc (CPU mode)"
    fi
else
    echo "✓ Environment variables already set in ~/.bashrc"
    
    # 如果是 GPU 模式但 cuDNN 路径未设置，添加它
    if [ "$onnx_choice" == "2" ] && ! grep -q "CUDNN8_LIB" ~/.bashrc; then
        echo "export CUDNN8_LIB=$CUDNN8_DIR" >> ~/.bashrc
        echo "✓ Added cuDNN 8 path to ~/.bashrc"
    fi
fi

# 当前 shell 也设置
export ONNXRUNTIME_DIR=$ONNX_INSTALL_DIR
if [ "$onnx_choice" == "2" ]; then
    export CUDNN8_LIB=$CUDNN8_DIR
    export LD_LIBRARY_PATH=$CUDNN8_DIR:$ONNX_INSTALL_DIR/lib:$LD_LIBRARY_PATH
else
    export LD_LIBRARY_PATH=$ONNX_INSTALL_DIR/lib:$LD_LIBRARY_PATH
fi

echo ""
echo "=========================================="
echo "✓ All dependencies installed successfully!"
echo "=========================================="
echo ""

if [ "$onnx_choice" == "2" ]; then
    # GPU mode instructions
    echo "GPU Mode Configuration:"
    echo ""
    echo "Installed components:"
    echo "  - ONNX Runtime GPU ${ONNX_VERSION} → ${ONNX_INSTALL_DIR}"
    echo "  - cuDNN 8.9.7 → ${CUDNN8_DIR}"
    echo ""
    echo "Next steps:"
    echo "  1. Reload environment:"
    echo "     source ~/.bashrc"
    echo ""
    echo "  2. Verify GPU support:"
    echo "     nvidia-smi"
    echo ""
    echo "  3. Build project with GPU support:"
    echo "     cd build && rm -rf * && cmake -DONNXRUNTIME_DIR=${ONNX_INSTALL_DIR} .."
    echo "     make -j\$(nproc)"
    echo "     Or use: ./rebuild_gpu.sh"
    echo ""
    echo "  4. Run with GPU acceleration:"
    echo "     ./run_with_gpu.sh --video ../test\\ vid.mp4"
    echo ""
    echo "Performance: GPU mode is 5-10x faster than CPU mode"
else
    # CPU mode instructions
    echo "CPU Mode Configuration:"
    echo ""
    echo "Installed:"
    echo "  - ONNX Runtime CPU ${ONNX_VERSION} → ${ONNX_INSTALL_DIR}"
    echo ""
    echo "Next steps:"
    echo "  1. Reload environment: source ~/.bashrc"
    echo "  2. Build project:      ./build_linux.sh"
    echo "  3. Convert models:     python3 convert_models_en.py"
    echo "  4. Run test:           ./run_test_linux.sh"
fi
echo ""
