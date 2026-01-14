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
ONNX_VERSION="1.23.2"
ONNX_URL="https://github.com/microsoft/onnxruntime/releases/download/v${ONNX_VERSION}/onnxruntime-linux-x64-${ONNX_VERSION}.tgz"
ONNX_INSTALL_DIR="/home/zodiac/work/tools/onnxruntime"

if [ -d "$ONNX_INSTALL_DIR" ]; then
    echo "ONNX Runtime already installed at $ONNX_INSTALL_DIR"
else
    echo "Downloading ONNX Runtime ${ONNX_VERSION}..."
    cd /tmp
    wget -q "$ONNX_URL"
    
    echo "Installing to $ONNX_INSTALL_DIR..."
    sudo mkdir -p "$ONNX_INSTALL_DIR"
    sudo tar -xzf "onnxruntime-linux-x64-${ONNX_VERSION}.tgz" -C "$ONNX_INSTALL_DIR" --strip-components=1
    rm "onnxruntime-linux-x64-${ONNX_VERSION}.tgz"
    
    echo "✓ ONNX Runtime installed"
fi

echo ""
echo "[4/4] Setting up environment variables..."

# 检查是否已经设置
if ! grep -q "ONNXRUNTIME_DIR" ~/.bashrc; then
    echo "" >> ~/.bashrc
    echo "# Football Analytics - ONNX Runtime" >> ~/.bashrc
    echo "export ONNXRUNTIME_DIR=$ONNX_INSTALL_DIR" >> ~/.bashrc
    echo "export LD_LIBRARY_PATH=$ONNX_INSTALL_DIR/lib:\$LD_LIBRARY_PATH" >> ~/.bashrc
    echo "✓ Environment variables added to ~/.bashrc"
else
    echo "✓ Environment variables already set"
fi

# 当前 shell 也设置
export ONNXRUNTIME_DIR=$ONNX_INSTALL_DIR
export LD_LIBRARY_PATH=$ONNX_INSTALL_DIR/lib:$LD_LIBRARY_PATH

echo ""
echo "=========================================="
echo "✓ All dependencies installed successfully!"
echo "=========================================="
echo ""
echo "Next steps:"
echo "  1. Reload environment: source ~/.bashrc"
echo "  2. Build project:      ./build_linux.sh"
echo "  3. Convert models:     python3 convert_models_en.py"
echo "  4. Run test:           ./run_test_linux.sh"
echo ""
