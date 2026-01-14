#!/bin/bash
# 使用 ONNX Runtime GPU 重新编译项目

set -e  # 遇到错误立即退出

GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m'

echo -e "${GREEN}========================================${NC}"
echo -e "${GREEN}  Football Analytics - GPU 构建脚本${NC}"
echo -e "${GREEN}========================================${NC}"
echo

# 切换到脚本所在目录
cd "$(dirname "$0")"

# GPU 版本的 ONNX Runtime 路径
ONNXRUNTIME_GPU_DIR="/home/zodiac/work/tools/onnxruntime-gpu"

# 检查 ONNX Runtime GPU 是否存在
if [ ! -d "$ONNXRUNTIME_GPU_DIR" ]; then
    echo -e "${RED}错误: ONNX Runtime GPU 未找到${NC}"
    echo "路径: $ONNXRUNTIME_GPU_DIR"
    echo
    echo "请先安装 ONNX Runtime GPU："
    echo "  bash install_deps_linux.sh"
    echo "  选择选项 2) GPU version"
    exit 1
fi

echo -e "${YELLOW}[1/3] 清理旧的构建目录...${NC}"
rm -rf build
echo "✓ 清理完成"
echo

echo -e "${YELLOW}[2/3] 配置 CMake (使用 GPU 版本)...${NC}"
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release \
      -DONNXRUNTIME_DIR="$ONNXRUNTIME_GPU_DIR" \
      ..

if [ $? -ne 0 ]; then
    echo -e "${RED}✗ CMake 配置失败${NC}"
    exit 1
fi
echo "✓ CMake 配置完成"
echo

echo -e "${YELLOW}[3/3] 编译项目...${NC}"
make -j$(nproc)

if [ $? -ne 0 ]; then
    echo -e "${RED}✗ 编译失败${NC}"
    exit 1
fi
echo "✓ 编译完成"
echo

echo -e "${GREEN}========================================${NC}"
echo -e "${GREEN}  ✓ 构建成功！${NC}"
echo -e "${GREEN}========================================${NC}"
echo
echo "可执行文件: ./build/football_analytics"
echo
echo "运行方式："
echo "  1) 使用 GPU 运行脚本（推荐）:"
echo "     ./run_with_gpu.sh --video ../test\\ vid.mp4"
echo
echo "  2) 直接运行:"
echo "     cd build && ./football_analytics --video ../../test\\ vid.mp4"
echo
echo -e "${YELLOW}注意: 直接运行需要先设置库路径：${NC}"
echo "  export LD_LIBRARY_PATH=/home/zodiac/work/tools/cudnn8-libs:/home/zodiac/work/tools/onnxruntime-gpu/lib:\${LD_LIBRARY_PATH}"
echo

# 验证链接的库
echo -e "${YELLOW}验证库链接...${NC}"
ldd ./football_analytics | grep onnxruntime | head -1
echo
