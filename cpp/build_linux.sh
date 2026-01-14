#!/bin/bash
set -e

echo "=========================================="
echo "Football Analytics - Linux Build"
echo "=========================================="

# 检查 ONNX Runtime
if [ -z "$ONNXRUNTIME_DIR" ]; then
    echo "[WARNING] ONNXRUNTIME_DIR not set"
    echo "Using default: /opt/onnxruntime"
    export ONNXRUNTIME_DIR=/opt/onnxruntime
fi

if [ ! -d "$ONNXRUNTIME_DIR" ]; then
    echo "[ERROR] ONNX Runtime not found at: $ONNXRUNTIME_DIR"
    echo "Please run: ./install_deps_linux.sh"
    exit 1
fi

echo "Configuration:"
echo "  ONNX Runtime: $ONNXRUNTIME_DIR"
echo "  Build Type:   Release"
echo "  Threads:      $(nproc)"
echo ""

# 创建构建目录
mkdir -p build
cd build

# 配置 CMake
echo "[1/3] Configuring CMake..."
cmake .. \
    -DCMAKE_BUILD_TYPE=Release \
    -DONNXRUNTIME_DIR="$ONNXRUNTIME_DIR" \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

# 编译
echo ""
echo "[2/3] Building project..."
cmake --build . --config Release -j$(nproc)

# 检查可执行文件
echo ""
echo "[3/3] Checking executable..."
if [ -f "./football_analytics" ]; then
    echo "✓ Build complete!"
    echo ""
    echo "Executable: $(pwd)/football_analytics"
    
    # 检查依赖
    echo ""
    echo "Checking dependencies:"
    ldd ./football_analytics | grep -E "onnxruntime|opencv|avcodec" || true
    
    echo ""
    echo "=========================================="
    echo "✓ Build successful!"
    echo "=========================================="
    echo ""
    echo "Next steps:"
    echo "  1. Convert models:  cd .. && python3 convert_models_en.py"
    echo "  2. Run program:     ./football_analytics --help"
    echo "  3. Test:            cd .. && ./run_test_linux.sh"
    echo ""
else
    echo "[ERROR] Build failed - executable not found"
    exit 1
fi
