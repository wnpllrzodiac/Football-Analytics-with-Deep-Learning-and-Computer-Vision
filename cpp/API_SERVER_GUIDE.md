# API Server 使用指南

## 简介

提供了两个Python API服务器示例：

1. **`simple_api_server.py`** - 简单版本（推荐）
   - 接收请求并打印日志
   - 不写入数据库
   - 内存中临时存储会话信息
   - 适合开发和测试

2. **`example_api_server.py`** - 完整版本
   - 接收请求并保存到JSON文件
   - 提供统计信息
   - 支持会话查询
   - 适合需要保存数据的场景

---

## 快速开始

### 步骤 1: 安装依赖

```powershell
pip install flask
```

### 步骤 2: 启动服务器

```powershell
cd D:\git\Football-Analytics-with-Deep-Learning-and-Computer-Vision\cpp
python simple_api_server.py
```

### 步骤 3: 运行C++程序

在另一个终端：

```powershell
cd D:\git\Football-Analytics-with-Deep-Learning-and-Computer-Vision\cpp\build\Release
.\football_analytics.exe --video "..\..\Streamlit web app\demo_vid_1.mp4"
```

---

## API端点

### GET /api/health
**健康检查**

```bash
curl http://localhost:8080/api/health
```

响应：
```json
{
  "status": "healthy",
  "timestamp": "2026-01-13T10:30:00.000000",
  "service": "Football Analytics API"
}
```

---

### POST /api/video/start
**开始视频处理**

请求体：
```json
{
  "videoSource": "demo_vid_1.mp4",
  "totalFrames": 3000,
  "timestamp": 1234567890
}
```

响应：
```json
{
  "status": "success",
  "message": "Video processing started for demo_vid_1.mp4"
}
```

服务器日志输出：
```
================================================================================
VIDEO PROCESSING STARTED
================================================================================
Time:         2026-01-13 10:30:00
Video:        demo_vid_1.mp4
Total frames: 3000
================================================================================
```

---

### POST /api/frames
**发送帧数据**

请求体：
```json
{
  "frameNumber": 1,
  "timestamp": 1234567890,
  "videoSource": "demo_vid_1.mp4",
  "players": [
    {
      "bbox": {"x": 100, "y": 200, "width": 50, "height": 80},
      "classId": 0,
      "confidence": 0.95,
      "label": "player",
      "teamId": 0,
      "tacMapPosition": {"x": 450.5, "y": 320.2}
    }
  ],
  "keypoints": [
    {
      "label": "Center circle",
      "x": 640.0,
      "y": 360.0,
      "confidence": 0.98
    }
  ],
  "balls": [
    {
      "x": 500.5,
      "y": 300.2,
      "confidence": 0.87
    }
  ]
}
```

服务器日志输出（每30帧）：
```
[10:30:15] Frame    1 | Progress:   0.0% | Players: 18 | Keypoints: 12 | Balls: 1
[10:30:16] Frame   30 | Progress:   1.0% | Players: 20 | Keypoints: 14 | Balls: 1
[10:30:17] Frame   60 | Progress:   2.0% | Players: 19 | Keypoints: 13 | Balls: 0
```

详细信息（每100帧）：
```
--- Frame 100 Details ---
  Players by team: {0: 10, 1: 9}
  Example player: bbox=(100, 200, 50, 80), conf=0.95
  Keypoints detected: 12
  Example keypoint: Center circle at (640.0, 360.0)
---
```

---

### POST /api/video/complete
**视频处理完成**

请求体：
```json
{
  "videoSource": "demo_vid_1.mp4",
  "timestamp": 1234567890
}
```

服务器日志输出：
```
================================================================================
VIDEO PROCESSING COMPLETED
================================================================================
Video:           demo_vid_1.mp4
Frames received: 3000/3000
Duration:        180.5 seconds
Avg FPS:         16.62
Completed at:    2026-01-13 10:33:00
================================================================================
```

---

### POST /api/test
**测试端点（打印完整JSON）**

发送任意JSON数据，服务器会打印完整内容。

```bash
curl -X POST http://localhost:8080/api/test \
  -H "Content-Type: application/json" \
  -d '{"test": "data", "value": 123}'
```

服务器输出：
```
--------------------------------------------------------------------------------
[10:30:00] TEST endpoint called
--------------------------------------------------------------------------------
{
  "test": "data",
  "value": 123
}
--------------------------------------------------------------------------------
```

---

## 测试服务器

### 方法 1: 使用 curl

```bash
# 健康检查
curl http://localhost:8080/api/health

# 开始视频处理
curl -X POST http://localhost:8080/api/video/start \
  -H "Content-Type: application/json" \
  -d '{"videoSource":"test.mp4","totalFrames":100}'

# 发送帧数据
curl -X POST http://localhost:8080/api/frames \
  -H "Content-Type: application/json" \
  -d '{"frameNumber":1,"players":[],"keypoints":[],"balls":[]}'

# 完成处理
curl -X POST http://localhost:8080/api/video/complete \
  -H "Content-Type: application/json" \
  -d '{"videoSource":"test.mp4"}'
```

### 方法 2: 使用 PowerShell

```powershell
# 健康检查
Invoke-RestMethod -Uri http://localhost:8080/api/health

# 开始视频处理
$body = @{
    videoSource = "test.mp4"
    totalFrames = 100
} | ConvertTo-Json

Invoke-RestMethod -Uri http://localhost:8080/api/video/start -Method POST -Body $body -ContentType "application/json"
```

### 方法 3: 使用测试脚本

创建 `test_api.py`：

```python
import requests
import json

BASE_URL = "http://localhost:8080"

# 1. Health check
print("Testing health endpoint...")
response = requests.get(f"{BASE_URL}/api/health")
print(f"Status: {response.status_code}")
print(f"Response: {response.json()}\n")

# 2. Start video
print("Testing video start...")
data = {
    "videoSource": "test.mp4",
    "totalFrames": 100
}
response = requests.post(f"{BASE_URL}/api/video/start", json=data)
print(f"Status: {response.status_code}")
print(f"Response: {response.json()}\n")

# 3. Send frames
print("Testing frame data...")
for i in range(1, 6):
    frame_data = {
        "frameNumber": i,
        "timestamp": 1234567890 + i,
        "players": [
            {
                "bbox": {"x": 100, "y": 200, "width": 50, "height": 80},
                "confidence": 0.95,
                "teamId": 0
            }
        ],
        "keypoints": [],
        "balls": []
    }
    response = requests.post(f"{BASE_URL}/api/frames", json=frame_data)
    print(f"Frame {i}: {response.status_code}")

print()

# 4. Complete video
print("Testing video complete...")
data = {"videoSource": "test.mp4"}
response = requests.post(f"{BASE_URL}/api/video/complete", json=data)
print(f"Status: {response.status_code}")
print(f"Response: {response.json()}\n")

print("All tests completed!")
```

运行测试：
```powershell
python test_api.py
```

---

## 配置

### 修改端口

编辑 `simple_api_server.py`，修改最后一行：

```python
app.run(host='0.0.0.0', port=8080, debug=False)  # 改为其他端口
```

然后在C++程序中使用：
```bash
.\football_analytics.exe --api-url http://localhost:新端口 --video test.mp4
```

### 启用调试模式

```python
app.run(host='0.0.0.0', port=8080, debug=True)  # 启用Flask调试模式
```

### 修改日志详细程度

在 `receive_frame()` 函数中修改：

```python
# 每帧都打印（详细）
if True:  # 替换 if frame_number % 30 == 0
    print(...)

# 只打印摘要（简洁）
if frame_number % 100 == 0:  # 替换 % 30
    print(...)
```

---

## 两个服务器版本对比

| 特性 | simple_api_server.py | example_api_server.py |
|------|---------------------|----------------------|
| **安装依赖** | 仅需 Flask | Flask |
| **数据存储** | 内存（不保存） | JSON文件 |
| **日志输出** | 实时控制台 | 控制台 + 文件 |
| **统计信息** | 基本统计 | 详细统计 |
| **会话查询** | 不支持 | 支持 |
| **适用场景** | 开发测试 | 生产环境 |
| **文件大小** | ~150行 | ~300行 |

---

## 故障排除

### 问题 1: 端口被占用

**错误信息**:
```
OSError: [WinError 10048] 通常每个套接字地址只允许使用一次
```

**解决方案**:
```powershell
# 查找占用端口的进程
netstat -ano | findstr :8080

# 结束进程（替换 PID）
taskkill /PID <PID> /F

# 或者使用不同端口
python simple_api_server.py  # 修改代码中的端口
```

### 问题 2: Flask 未安装

**错误信息**:
```
ModuleNotFoundError: No module named 'flask'
```

**解决方案**:
```powershell
pip install flask
```

### 问题 3: C++程序无法连接

**检查**:
1. 服务器是否运行？
2. 端口是否正确（8080）？
3. 防火墙是否阻止？

**测试连接**:
```powershell
curl http://localhost:8080/api/health
```

---

## 生产部署建议

对于生产环境，考虑使用：

1. **Gunicorn** (Linux/Mac):
   ```bash
   pip install gunicorn
   gunicorn -w 4 -b 0.0.0.0:8080 simple_api_server:app
   ```

2. **Waitress** (Windows):
   ```powershell
   pip install waitress
   waitress-serve --port=8080 simple_api_server:app
   ```

3. **Docker**:
   ```dockerfile
   FROM python:3.11-slim
   WORKDIR /app
   COPY simple_api_server.py .
   RUN pip install flask
   EXPOSE 8080
   CMD ["python", "simple_api_server.py"]
   ```

---

## 支持

- 查看服务器日志获取详细信息
- 使用 `/api/test` 端点测试数据格式
- 检查 C++ 程序的 `--debug` 输出

**相关文档**:
- `README.md` - 项目总体文档
- `QUICK_FIX.md` - 快速修复指南
