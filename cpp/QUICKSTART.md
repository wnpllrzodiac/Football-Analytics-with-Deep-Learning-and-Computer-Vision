# 快速开始指南

从零开始构建和运行Football Analytics C++程序的完整步骤。

## 前置要求

- Windows 10/11
- Python 3.8+ (用于模型转换)
- Visual Studio 2019/2022 或 MinGW
- 至少 8GB RAM
- NVIDIA GPU (可选，用于加速)

## 步骤1: 安装依赖

### 1.1 安装vcpkg

```powershell
# 下载vcpkg
cd C:\
git clone https://github.com/microsoft/vcpkg.git
cd vcpkg

# 初始化
.\bootstrap-vcpkg.bat

# 集成到系统
.\vcpkg integrate install
```

### 1.2 安装C++库

```powershell
# 进入vcpkg目录
cd C:\vcpkg

# 安装所有必需的库（可能需要30-60分钟）
.\vcpkg install opencv4:x64-windows
.\vcpkg install ffmpeg:x64-windows
.\vcpkg install nlohmann-json:x64-windows
.\vcpkg install cpp-httplib:x64-windows
```

### 1.3 安装ONNX Runtime

```powershell
# 下载ONNX Runtime (Windows, x64)
# 访问: https://github.com/microsoft/onnxruntime/releases
# 下载: onnxruntime-win-x64-*.zip

# 解压到C盘
# 假设解压后路径为: C:\onnxruntime
```

### 1.4 安装Python依赖

```powershell
# 安装ultralytics（用于模型转换）
pip install ultralytics flask
```

## 步骤2: 准备项目

### 2.1 复制战术地图

```powershell
# 进入项目cpp目录
cd D:\git\Football-Analytics-with-Deep-Learning-and-Computer-Vision\cpp

# 复制战术地图图片
copy ..\tactical map.jpg resources\tactical_map.jpg
```

### 2.2 转换模型

```powershell
# 运行模型转换脚本
python convert_models.py
```

预期输出：
```
==================================================
Football Analytics - 模型转换工具
==================================================

[1/2] 正在处理: 球员检测模型
------------------------------------------------------------
✓ 转换成功: cpp\models\players.onnx

[2/2] 正在处理: 关键点检测模型
------------------------------------------------------------
✓ 转换成功: cpp\models\keypoints.onnx

总计: 2/2 个模型转换成功
```

## 步骤3: 编译项目

### 3.1 修改构建脚本

编辑 `build.bat`，确保路径正确：

```batch
set VCPKG_ROOT=C:\vcpkg
set ONNXRUNTIME_DIR=C:\onnxruntime
```

### 3.2 运行构建

```powershell
# 双击运行或在命令行执行
.\build.bat
```

编译成功后，可执行文件位于：
```
cpp\build\Release\football_analytics.exe
```

## 步骤4: 启动API服务器

打开新的终端窗口：

```powershell
# 进入cpp目录
cd D:\git\Football-Analytics-with-Deep-Learning-and-Computer-Vision\cpp

# 启动示例API服务器
python example_api_server.py
```

服务器将在 http://localhost:8080 上运行。

## 步骤5: 运行程序

打开另一个终端窗口：

```powershell
# 进入构建目录
cd D:\git\Football-Analytics-with-Deep-Learning-and-Computer-Vision\cpp\build\Release

# 复制必要的DLL文件到当前目录（如果需要）
# 这些DLL通常在vcpkg和ONNX Runtime的bin目录下

# 运行程序（使用演示视频）
.\football_analytics.exe --video ..\..\..\Streamlit web app\demo_vid_1.mp4
```

### 完整命令示例

```powershell
.\football_analytics.exe ^
  --video "..\..\..\Streamlit web app\demo_vid_1.mp4" ^
  --api-url http://localhost:8080 ^
  --player-conf 0.6 ^
  --keypoint-conf 0.7 ^
  --team1-name "France" ^
  --team2-name "Switzerland" ^
  --debug
```

## 预期输出

### 控制台输出

```
==================================================
  Football Analytics with Deep Learning & CV
  C++ Implementation
==================================================

[1/7] Initializing video reader...
Video opened successfully:
  Resolution: 1920x1080
  FPS: 30.0
  Total frames: 3000

[2/7] Loading detection models...
YOLO Detector initialized successfully
  Model: ./models/players.onnx
  Input size: 640x640

[3/7] Initializing team predictor...
Team colors configured:
  Team 1: France
  Team 2: Switzerland

[4/7] Initializing coordinate transform...
Tactical map loaded: ./resources/tactical_map.jpg

[5/7] Connecting to API server...
API connection test successful (status: 200)

[6/7] Processing video frames...
Progress: 100.0% (3000/3000) | FPS: 15.23 | Frame time: 65ms

[7/7] Finalizing...

==================================================
  Processing Complete!
==================================================
Total frames processed: 3000
Total time: 197 seconds
Average FPS: 15.23
==================================================
```

### API服务器输出

```
==================================================
新视频处理会话开始
==================================================
会话ID: 20260113_105423
视频源: demo_vid_1.mp4
总帧数: 3000
==================================================

接收进度: 100.0% (3000/3000) 帧 3000

==================================================
视频处理完成
==================================================
会话ID: 20260113_105423
视频源: demo_vid_1.mp4
总接收帧数: 3000
数据已保存: ./api_data/session_20260113_105423.json

统计信息:
  平均每帧球员数: 18.3
  平均每帧关键点数: 12.5
  检测到球的帧数: 2456
==================================================
```

## 故障排除

### 问题1: 找不到DLL文件

**解决方案：**
```powershell
# 将以下DLL复制到可执行文件目录：
# - opencv_world4XX.dll (从vcpkg)
# - ffmpeg相关DLL (从vcpkg)
# - onnxruntime.dll (从ONNX Runtime)

# 或者将这些目录添加到系统PATH
```

### 问题2: ONNX Runtime初始化失败

**解决方案：**
- 确认ONNX Runtime路径正确
- 检查是否下载了正确的版本（CPU vs GPU）
- 确保CMake配置时指定了正确的路径

### 问题3: 模型加载失败

**解决方案：**
- 确认模型文件存在于 `cpp/models/` 目录
- 重新运行 `convert_models.py`
- 检查模型转换是否成功

### 问题4: API连接失败

**解决方案：**
- 确认API服务器正在运行
- 检查防火墙设置
- 尝试使用 `--debug` 参数查看详细错误

### 问题5: 视频无法打开

**解决方案：**
- 确认视频路径正确
- 确认FFmpeg支持该视频格式
- 尝试使用绝对路径

## 性能优化建议

### 1. 使用GPU加速

如果有NVIDIA GPU，安装CUDA和TensorRT：
- 下载CUDA Toolkit
- 下载TensorRT
- 使用TensorRT版本的ONNX Runtime

### 2. 调整置信度阈值

降低置信度阈值会检测到更多目标，但会增加误检：
```bash
--player-conf 0.5  # 默认0.6
```

提高阈值会减少误检，但可能漏检：
```bash
--player-conf 0.7
```

### 3. 批处理优化

修改代码支持批量处理帧和批量发送API请求。

## 下一步

1. **自定义球队颜色**：修改main.cpp中的球队颜色配置
2. **保存结果视频**：添加视频写入功能
3. **数据库集成**：将API服务器连接到实际数据库
4. **实时流处理**：支持RTSP等实时视频流
5. **Web界面**：开发前端界面展示结果

## 支持

如遇到问题，请：
1. 查看 `README.md` 获取详细文档
2. 检查日志输出中的错误信息
3. 在项目仓库提交Issue

祝您使用愉快！🎉
