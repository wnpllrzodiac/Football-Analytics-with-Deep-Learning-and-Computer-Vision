# Windows 安装和配置指南

## 依赖库路径配置

本项目需要以下依赖库，请确保路径正确配置。

### 1. FFmpeg (已配置)

**安装路径**: `D:\sdk\ffmpeg-8.0.1-full_build-shared`

**目录结构**:
```
D:\sdk\ffmpeg-8.0.1-full_build-shared\
├── bin\
│   ├── avcodec-61.dll
│   ├── avformat-61.dll
│   ├── avutil-59.dll
│   ├── swscale-8.dll
│   └── ... (其他DLL文件)
├── include\
│   ├── libavcodec\
│   ├── libavformat\
│   ├── libavutil\
│   ├── libswscale\
│   └── ...
└── lib\
    ├── avcodec.lib
    ├── avformat.lib
    ├── avutil.lib
    ├── swscale.lib
    └── ...
```

### 2. ONNX Runtime (已配置)

**安装路径**: `C:\onnxruntime-win-x64-1.23.2`

**目录结构**:
```
C:\onnxruntime-win-x64-1.23.2\
├── include\
│   ├── onnxruntime_c_api.h
│   ├── onnxruntime_cxx_api.h
│   └── ...
└── lib\
    ├── onnxruntime.dll
    ├── onnxruntime.lib
    └── ...
```

### 3. vcpkg (已配置)

**安装路径**: `D:\git\vcpkg`

**必需安装的包**:
```powershell
cd D:\git\vcpkg
.\vcpkg install opencv4:x64-windows
.\vcpkg install cpp-httplib:x64-windows
```

**注意**: `nlohmann-json` 现在通过 CMake FetchContent 自动下载，无需手动安装。

## 构建步骤

### 步骤 1: 验证依赖路径

确认以下目录存在：

```powershell
# 检查 FFmpeg
dir D:\sdk\ffmpeg-8.0.1-full_build-shared\include
dir D:\sdk\ffmpeg-8.0.1-full_build-shared\lib

# 检查 ONNX Runtime
dir C:\onnxruntime-win-x64-1.23.2\include
dir C:\onnxruntime-win-x64-1.23.2\lib

# 检查 vcpkg
dir D:\git\vcpkg\scripts\buildsystems\vcpkg.cmake
```

### 步骤 2: 转换模型

```powershell
cd D:\git\Football-Analytics-with-Deep-Learning-and-Computer-Vision\cpp
python convert_models.py
```

### 步骤 3: 复制资源文件

```powershell
# 复制战术地图
copy "..\tactical map.jpg" resources\tactical_map.jpg
```

### 步骤 4: 编译项目

```powershell
# 运行构建脚本
.\build.bat
```

如果构建成功，可执行文件将位于：
```
cpp\build\Release\football_analytics.exe
```

### 步骤 5: 复制运行时DLL

在运行程序之前，需要将DLL文件复制到可执行文件目录：

```powershell
# 进入可执行文件目录
cd build\Release

# 复制 FFmpeg DLL
copy D:\sdk\ffmpeg-8.0.1-full_build-shared\bin\*.dll .

# 复制 ONNX Runtime DLL
copy C:\onnxruntime-win-x64-1.23.2\lib\onnxruntime.dll .

# 复制 OpenCV DLL (从vcpkg)
copy D:\git\vcpkg\installed\x64-windows\bin\opencv_world4*.dll .
```

**或者设置环境变量**（推荐）：

```powershell
# 添加到系统 PATH（管理员权限）
setx PATH "%PATH%;D:\sdk\ffmpeg-8.0.1-full_build-shared\bin;C:\onnxruntime-win-x64-1.23.2\lib;D:\git\vcpkg\installed\x64-windows\bin"
```

### 步骤 6: 运行程序

```powershell
cd build\Release

# 运行示例
.\football_analytics.exe --video "..\..\Streamlit web app\demo_vid_1.mp4"
```

## 故障排除

### 问题 1: CMake 找不到 FFmpeg

**错误信息**:
```
FFmpeg not found. Please set FFMPEG_DIR
```

**解决方案**:
```powershell
# 手动指定 FFMPEG_DIR
cmake .. -DFFMPEG_DIR=D:/sdk/ffmpeg-8.0.1-full_build-shared
```

### 问题 2: 链接器错误 (LNK2019)

**错误信息**:
```
unresolved external symbol av_register_all
```

**解决方案**:
确认 `D:\sdk\ffmpeg-8.0.1-full_build-shared\lib` 目录下存在以下文件：
- `avcodec.lib`
- `avformat.lib`
- `avutil.lib`
- `swscale.lib`

### 问题 3: 运行时找不到 DLL

**错误信息**:
```
The code execution cannot proceed because avformat-61.dll was not found.
```

**解决方案**:
```powershell
# 方法 1: 复制 DLL 到可执行文件目录（见步骤5）

# 方法 2: 添加到 PATH
set PATH=%PATH%;D:\sdk\ffmpeg-8.0.1-full_build-shared\bin
```

### 问题 4: nlohmann_json 下载失败

**错误信息**:
```
CMake Error at ... FetchContent failed
```

**解决方案**:
```powershell
# 方法 1: 使用代理
set HTTP_PROXY=http://your-proxy:port
set HTTPS_PROXY=http://your-proxy:port

# 方法 2: 手动下载并使用 vcpkg
cd D:\git\vcpkg
.\vcpkg install nlohmann-json:x64-windows

# 然后在 CMakeLists.txt 中修改为:
find_package(nlohmann_json CONFIG REQUIRED)
```

## 配置不同的FFmpeg版本

如果您使用不同版本的FFmpeg，请修改以下文件：

### CMakeLists.txt
```cmake
set(FFMPEG_DIR "您的FFmpeg路径" CACHE PATH "FFmpeg installation directory")
```

### build.bat
```batch
set FFMPEG_DIR=您的FFmpeg路径
```

## 使用不同的构建类型

### Debug 构建
```powershell
cmake .. -DCMAKE_BUILD_TYPE=Debug ...
cmake --build . --config Debug
```

### Release 构建（默认）
```powershell
cmake .. -DCMAKE_BUILD_TYPE=Release ...
cmake --build . --config Release
```

## 验证安装

运行以下命令验证所有组件正常工作：

```powershell
# 1. 检查可执行文件
.\football_analytics.exe --help

# 2. 测试 API 连接（先启动API服务器）
python ..\..\example_api_server.py

# 3. 处理测试视频
.\football_analytics.exe --video test.mp4 --debug
```

## 性能优化

### 使用 GPU 加速

如果您有 NVIDIA GPU，可以使用 CUDA 版本的 ONNX Runtime：

1. 下载 ONNX Runtime GPU 版本
2. 修改 CMakeLists.txt 中的路径
3. 在代码中启用 CUDA Provider（见 YOLODetector.cpp）

### 多线程优化

在 main.cpp 中可以实现：
- 视频读取线程
- 推理线程
- API 上传线程

## 参考资源

- **FFmpeg文档**: https://ffmpeg.org/documentation.html
- **ONNX Runtime文档**: https://onnxruntime.ai/docs/
- **OpenCV文档**: https://docs.opencv.org/
- **vcpkg**: https://vcpkg.io/

## 支持

如遇到其他问题，请查看：
1. `README.md` - 项目总体文档
2. `QUICKSTART.md` - 快速开始指南
3. 项目 Issues 页面
