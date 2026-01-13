# Football Analytics C++ Implementation

基于深度学习和计算机视觉的足球分析系统 - C++版本

## 功能特性

- ⚽ **球员检测** - 使用YOLOv8检测球员、裁判和足球
- 👥 **球队识别** - 基于颜色分析的球队归属预测
- 🗺️ **战术图映射** - 将球员位置映射到2D战术图
- 🎯 **球场关键点检测** - 检测球场线条和关键点
- 📡 **API集成** - 通过HTTP API将结果发送到数据库
- 🚀 **高性能** - 使用ONNX Runtime或TensorRT加速

## 系统架构

```
┌─────────────────────────────────────────────────────────┐
│                  Football Analytics                     │
├─────────────────────────────────────────────────────────┤
│  VideoReader (FFmpeg) → 视频帧抽取                      │
│  YOLODetector (ONNX Runtime) → 目标检测                │
│  CoordinateTransform (OpenCV) → 坐标转换               │
│  TeamPredictor (OpenCV) → 球队预测                     │
│  ApiClient (cpp-httplib) → 数据上传                    │
└─────────────────────────────────────────────────────────┘
```

## 依赖项

### 必需依赖

- **CMake** >= 3.15
- **C++17** 编译器 (MSVC, GCC, Clang)
- **OpenCV** >= 4.5.0
- **FFmpeg** >= 4.0
- **ONNX Runtime** >= 1.10.0

### 可选依赖

- **TensorRT** - GPU加速（替代ONNX Runtime）
- **nlohmann/json** - JSON解析
- **cpp-httplib** - HTTP客户端（header-only）

## Windows 安装指南

### 使用 vcpkg 安装依赖

```powershell
# 安装 vcpkg
git clone https://github.com/microsoft/vcpkg.git
cd vcpkg
.\bootstrap-vcpkg.bat

# 安装依赖
.\vcpkg install opencv4:x64-windows
.\vcpkg install ffmpeg:x64-windows
.\vcpkg install nlohmann-json:x64-windows
.\vcpkg install cpp-httplib:x64-windows

# 集成到系统
.\vcpkg integrate install
```

### 安装 ONNX Runtime

1. 下载 ONNX Runtime: https://github.com/microsoft/onnxruntime/releases
2. 解压到 `C:\onnxruntime`
3. 在CMake配置时设置路径：
   ```bash
   -DONNXRUNTIME_DIR=C:/onnxruntime
   ```

## 构建项目

```powershell
cd cpp
mkdir build
cd build

# 配置CMake
cmake .. -DCMAKE_TOOLCHAIN_FILE=[vcpkg root]/scripts/buildsystems/vcpkg.cmake -DONNXRUNTIME_DIR=C:/onnxruntime

# 编译
cmake --build . --config Release

# 或使用Visual Studio打开解决方案
```

## 模型准备

### 1. 转换PyTorch模型为ONNX

在Python环境中运行：

```python
from ultralytics import YOLO

# 转换球员检测模型
model_players = YOLO('../models/Yolo8L Players/weights/best.pt')
model_players.export(format='onnx', imgsz=640, simplify=True)

# 转换关键点检测模型
model_keypoints = YOLO('../models/Yolo8M Field Keypoints/weights/best.pt')
model_keypoints.export(format='onnx', imgsz=640, simplify=True)
```

### 2. 放置模型文件

```
cpp/models/
├── players.onnx         # 重命名导出的模型
└── keypoints.onnx       # 重命名导出的模型
```

## 使用方法

### 基本用法

```bash
./football_analytics --video /path/to/video.mp4
```

### 完整参数

```bash
./football_analytics \
  --video demo_vid_1.mp4 \
  --api-url http://localhost:8080 \
  --api-key your_api_key \
  --player-model ./models/players.onnx \
  --keypoint-model ./models/keypoints.onnx \
  --player-conf 0.6 \
  --keypoint-conf 0.7 \
  --team1-name "France" \
  --team2-name "Switzerland" \
  --debug
```

### 参数说明

| 参数 | 描述 | 默认值 |
|------|------|--------|
| `--video` | 输入视频文件路径 | **必需** |
| `--api-url` | API服务器地址 | `http://localhost:8080` |
| `--api-key` | API认证密钥 | 空 |
| `--player-model` | 球员检测模型 | `./models/players.onnx` |
| `--keypoint-model` | 关键点检测模型 | `./models/keypoints.onnx` |
| `--player-conf` | 球员检测置信度阈值 | `0.6` |
| `--keypoint-conf` | 关键点检测置信度阈值 | `0.7` |
| `--team1-name` | 第一支球队名称 | `Team1` |
| `--team2-name` | 第二支球队名称 | `Team2` |
| `--debug` | 启用调试模式 | 关闭 |

## API接口规范

### 端点

#### 1. 开始视频处理
```
POST /api/video/start
Content-Type: application/json

{
  "videoSource": "video.mp4",
  "totalFrames": 3000,
  "timestamp": 1234567890
}
```

#### 2. 发送帧数据
```
POST /api/frames
Content-Type: application/json

{
  "frameNumber": 1,
  "timestamp": 1234567890,
  "videoSource": "video.mp4",
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

#### 3. 完成视频处理
```
POST /api/video/complete
Content-Type: application/json

{
  "videoSource": "video.mp4",
  "timestamp": 1234567890
}
```

#### 4. 健康检查
```
GET /api/health
```

## 性能优化

### 使用TensorRT加速

1. 转换ONNX为TensorRT引擎：
   ```bash
   trtexec --onnx=players.onnx \
           --saveEngine=players_fp16.engine \
           --fp16 \
           --workspace=4096
   ```

2. 修改代码使用TensorRT引擎

### 多线程处理

可以修改main.cpp实现：
- 一个线程读取视频帧
- 一个线程进行推理
- 一个线程发送API请求

## 故障排除

### FFmpeg链接错误
- 确保安装了完整的FFmpeg开发包
- Windows: 下载FFmpeg shared库并设置环境变量

### ONNX Runtime找不到
- 检查ONNXRUNTIME_DIR路径是否正确
- 确保include和lib目录存在

### OpenCV版本不兼容
- 使用vcpkg安装可确保版本兼容
- 或手动下载OpenCV 4.5+

### API连接失败
- 检查API服务器是否运行
- 确认防火墙设置
- 使用`--debug`参数查看详细错误

## 项目结构

```
cpp/
├── CMakeLists.txt           # CMake配置
├── README.md                # 本文件
├── include/                 # 头文件
│   ├── VideoReader.h
│   ├── YOLODetector.h
│   ├── TeamPredictor.h
│   ├── CoordinateTransform.h
│   └── ApiClient.h
├── src/                     # 源文件
│   ├── main.cpp
│   ├── VideoReader.cpp
│   ├── YOLODetector.cpp
│   ├── TeamPredictor.cpp
│   ├── CoordinateTransform.cpp
│   └── ApiClient.cpp
├── models/                  # 模型文件
│   ├── players.onnx
│   └── keypoints.onnx
├── config/                  # 配置文件
│   ├── config_players.json
│   ├── config_pitch.json
│   └── pitch_map_labels.json
├── resources/               # 资源文件
│   └── tactical_map.jpg
└── third_party/             # 第三方库
    └── httplib.h
```

## 许可证

本项目遵循原项目的许可证。

## 贡献

欢迎提交Issue和Pull Request！

## 联系方式

如有问题，请创建Issue或联系项目维护者。
