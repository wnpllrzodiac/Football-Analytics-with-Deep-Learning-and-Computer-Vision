#!/bin/bash
set -e

echo "=========================================="
echo "Football Analytics - Test Run"
echo "=========================================="

# 检查可执行文件
if [ ! -f "build/football_analytics" ]; then
    echo "[ERROR] Executable not found: build/football_analytics"
    echo "Please run: ./build_linux.sh"
    exit 1
fi

# 检查模型
echo "[1/4] Checking models..."
if [ ! -f "models/players.onnx" ] || [ ! -f "models/keypoints.onnx" ]; then
    echo "[WARNING] ONNX models not found."
    echo "Converting models..."
    python3 convert_models_en.py
else
    echo "✓ Models found"
fi

# 检查测试视频
echo ""
echo "[2/4] Checking test video..."
TEST_VIDEO="../Streamlit web app/demo_vid_1.mp4"

if [ ! -f "$TEST_VIDEO" ]; then
    echo "[WARNING] Default test video not found: $TEST_VIDEO"
    
    # 尝试其他视频
    if [ -f "../test vid.mp4" ]; then
        TEST_VIDEO="../test vid.mp4"
        echo "Using: $TEST_VIDEO"
    else
        echo ""
        echo "Available videos:"
        find .. -name "*.mp4" -type f 2>/dev/null | head -5 || echo "  (none found)"
        echo ""
        read -p "Enter video path (or press Enter to skip): " USER_VIDEO
        
        if [ -n "$USER_VIDEO" ]; then
            TEST_VIDEO="$USER_VIDEO"
        else
            echo "[SKIP] No video provided. Test skipped."
            exit 0
        fi
    fi
else
    echo "✓ Test video found: $TEST_VIDEO"
fi

# 启动 API 服务器
echo ""
echo "[3/4] Starting API server..."
python3 simple_api_server.py &
SERVER_PID=$!

# 等待服务器启动
sleep 2

# 检查服务器是否运行
if ! kill -0 $SERVER_PID 2>/dev/null; then
    echo "[ERROR] API server failed to start"
    exit 1
fi

echo "✓ API server running (PID: $SERVER_PID)"

# 运行 C++ 程序
echo ""
echo "[4/4] Running C++ program..."
echo "=========================================="
echo ""

cd build
./football_analytics --video "$TEST_VIDEO" || TEST_RESULT=$?

# 清理
echo ""
echo "=========================================="
echo "Cleaning up..."
cd ..
kill $SERVER_PID 2>/dev/null || true
wait $SERVER_PID 2>/dev/null || true

if [ -n "$TEST_RESULT" ] && [ $TEST_RESULT -ne 0 ]; then
    echo ""
    echo "[ERROR] Test failed with exit code: $TEST_RESULT"
    exit $TEST_RESULT
fi

echo ""
echo "=========================================="
echo "✓ Test completed successfully!"
echo "=========================================="
