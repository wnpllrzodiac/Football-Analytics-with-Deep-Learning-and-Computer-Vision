#!/bin/bash
# Football Analytics - GPU 加速运行脚本
# 自动设置 CUDA/cuDNN 库路径

# 颜色定义
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m' # No Color

echo -e "${GREEN}========================================${NC}"
echo -e "${GREEN}  Football Analytics - GPU Mode${NC}"
echo -e "${GREEN}========================================${NC}"
echo

# ONNX Runtime GPU 库路径
export ONNXRUNTIME_LIB="/home/zodiac/work/tools/onnxruntime-gpu/lib"

# cuDNN 8 库路径（兼容 ONNX Runtime 1.16.3）
export CUDNN8_LIB="/home/zodiac/work/tools/cudnn8-libs"

# 将所有必要的库路径添加到 LD_LIBRARY_PATH（优先级从左到右）
# 1. cuDNN 8 (最高优先级，覆盖系统的 cuDNN 9)
# 2. ONNX Runtime GPU
# 3. 系统 CUDA 库
export LD_LIBRARY_PATH="${CUDNN8_LIB}:${ONNXRUNTIME_LIB}:/usr/lib/x86_64-linux-gnu:${LD_LIBRARY_PATH}"

# 显示配置信息
echo -e "${YELLOW}配置信息:${NC}"
echo "  ONNX Runtime: ${ONNXRUNTIME_LIB}"
echo "  cuDNN 8: ${CUDNN8_LIB}"
echo "  LD_LIBRARY_PATH: ${LD_LIBRARY_PATH}"
echo

# 检查 GPU
if command -v nvidia-smi &> /dev/null; then
    echo -e "${GREEN}✓ GPU 检测成功${NC}"
    nvidia-smi --query-gpu=name,memory.total --format=csv,noheader | head -1
    echo
else
    echo -e "${RED}⚠ 未检测到 NVIDIA GPU${NC}"
    echo
fi

# 检查可执行文件
if [ ! -f "./build/football_analytics" ]; then
    echo -e "${RED}错误: 未找到可执行文件${NC}"
    echo "请先编译项目："
    echo "  cd cpp"
    echo "  bash build_linux.sh"
    exit 1
fi

# 运行程序
echo -e "${GREEN}启动程序...${NC}"
echo "========================================"
echo

cd "$(dirname "$0")"
./build/football_analytics "$@"

exit_code=$?
echo
echo "========================================"
if [ $exit_code -eq 0 ]; then
    echo -e "${GREEN}✓ 程序执行完成${NC}"
else
    echo -e "${RED}✗ 程序退出，退出码: $exit_code${NC}"
fi

exit $exit_code
