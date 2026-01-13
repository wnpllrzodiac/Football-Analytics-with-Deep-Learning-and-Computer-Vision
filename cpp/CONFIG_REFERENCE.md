# 配置参考速查

## 📋 当前配置

### 依赖库路径

| 库名称 | 安装路径 | 状态 |
|--------|----------|------|
| **OpenCV** | `C:\OpenCV4.10.0\opencv\build` | ✅ 已配置 |
| **FFmpeg** | `D:\sdk\ffmpeg-8.0.1-full_build-shared` | ✅ 已配置 |
| **ONNX Runtime** | `C:\onnxruntime-win-x64-1.23.2` | ✅ 已配置 |
| **vcpkg** | `D:\git\vcpkg` (仅用于cpp-httplib) | ⚠️ 可选 |
| **nlohmann/json** | 自动下载 (FetchContent) | ✅ 自动 |

### 必需的DLL文件

运行程序前需要以下DLL可访问（通过PATH或复制到exe目录）：

#### FFmpeg DLLs
```
D:\sdk\ffmpeg-8.0.1-full_build-shared\bin\
├── avcodec-61.dll
├── avformat-61.dll
├── avutil-59.dll
├── swscale-8.dll
└── swresample-5.dll (如需要)
```

#### ONNX Runtime DLL
```
C:\onnxruntime-win-x64-1.23.2\lib\
└── onnxruntime.dll
```

#### OpenCV DLLs (from vcpkg)
```
D:\git\vcpkg\installed\x64-windows\bin\
└── opencv_world4XX.dll
```

## 🛠️ 快速命令

### 一键复制所有DLL到可执行文件目录

```powershell
# 进入构建目录
cd cpp\build\Release

# 复制所有必需DLL
copy D:\sdk\ffmpeg-8.0.1-full_build-shared\bin\*.dll .
copy C:\onnxruntime-win-x64-1.23.2\lib\onnxruntime.dll .
copy D:\git\vcpkg\installed\x64-windows\bin\opencv_world4*.dll .
```

### 设置环境变量（推荐）

```powershell
# 临时设置（仅当前会话）
$env:PATH += ";D:\sdk\ffmpeg-8.0.1-full_build-shared\bin;C:\onnxruntime-win-x64-1.23.2\lib;D:\git\vcpkg\installed\x64-windows\bin"

# 永久设置（需要管理员权限）
setx PATH "%PATH%;D:\sdk\ffmpeg-8.0.1-full_build-shared\bin;C:\onnxruntime-win-x64-1.23.2\lib;D:\git\vcpkg\installed\x64-windows\bin" /M
```

### 完整构建流程

```powershell
# 1. 转换模型
cd cpp
python convert_models.py

# 2. 复制资源
copy "..\tactical map.jpg" resources\tactical_map.jpg

# 3. 构建
.\build.bat

# 4. 复制DLL（如果未设置PATH）
cd build\Release
copy D:\sdk\ffmpeg-8.0.1-full_build-shared\bin\*.dll .
copy C:\onnxruntime-win-x64-1.23.2\lib\onnxruntime.dll .
copy D:\git\vcpkg\installed\x64-windows\bin\opencv_world4*.dll .

# 5. 运行
.\football_analytics.exe --video "..\..\Streamlit web app\demo_vid_1.mp4"
```

## 🔧 修改配置路径

### 修改 CMakeLists.txt

```cmake
# Line 18: FFmpeg路径
set(FFMPEG_DIR "您的路径" CACHE PATH "FFmpeg installation directory")

# Line 29: ONNX Runtime路径
set(ONNXRUNTIME_DIR "您的路径" CACHE PATH "ONNX Runtime installation directory")
```

### 修改 build.bat

```batch
# Line 20-22: 设置路径变量
set VCPKG_ROOT=您的vcpkg路径
set ONNXRUNTIME_DIR=您的ONNX Runtime路径
set FFMPEG_DIR=您的FFmpeg路径
```

## 🐛 常见问题快速解决

### CMake配置失败

```powershell
# 清理并重新配置
rmdir /s /q build
mkdir build
cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=D:/git/vcpkg/scripts/buildsystems/vcpkg.cmake ^
         -DONNXRUNTIME_DIR=C:/onnxruntime-win-x64-1.23.2 ^
         -DFFMPEG_DIR=D:/sdk/ffmpeg-8.0.1-full_build-shared
```

### 找不到DLL

```powershell
# 验证DLL路径
dir D:\sdk\ffmpeg-8.0.1-full_build-shared\bin\avformat*.dll
dir C:\onnxruntime-win-x64-1.23.2\lib\onnxruntime.dll
dir D:\git\vcpkg\installed\x64-windows\bin\opencv_world*.dll

# 如果存在，添加到PATH或复制
```

### 链接错误

```powershell
# 验证.lib文件存在
dir D:\sdk\ffmpeg-8.0.1-full_build-shared\lib\avformat.lib
dir C:\onnxruntime-win-x64-1.23.2\lib\onnxruntime.lib

# 如果缺失，重新下载对应的库
```

## 📊 验证安装

### 检查列表

- [ ] FFmpeg 头文件: `D:\sdk\ffmpeg-8.0.1-full_build-shared\include\libavformat\avformat.h`
- [ ] FFmpeg 库文件: `D:\sdk\ffmpeg-8.0.1-full_build-shared\lib\avformat.lib`
- [ ] FFmpeg DLL文件: `D:\sdk\ffmpeg-8.0.1-full_build-shared\bin\avformat-61.dll`
- [ ] ONNX Runtime 头文件: `C:\onnxruntime-win-x64-1.23.2\include\onnxruntime_cxx_api.h`
- [ ] ONNX Runtime 库文件: `C:\onnxruntime-win-x64-1.23.2\lib\onnxruntime.lib`
- [ ] ONNX Runtime DLL: `C:\onnxruntime-win-x64-1.23.2\lib\onnxruntime.dll`
- [ ] vcpkg 工具链: `D:\git\vcpkg\scripts\buildsystems\vcpkg.cmake`
- [ ] OpenCV (vcpkg): 运行 `D:\git\vcpkg\vcpkg list opencv4`
- [ ] 模型文件: `cpp\models\players.onnx` 和 `cpp\models\keypoints.onnx`
- [ ] 战术地图: `cpp\resources\tactical_map.jpg`

### 验证命令

```powershell
# 验证所有路径
if (Test-Path D:\sdk\ffmpeg-8.0.1-full_build-shared\include) { "✅ FFmpeg headers OK" } else { "❌ FFmpeg headers missing" }
if (Test-Path D:\sdk\ffmpeg-8.0.1-full_build-shared\lib) { "✅ FFmpeg libs OK" } else { "❌ FFmpeg libs missing" }
if (Test-Path D:\sdk\ffmpeg-8.0.1-full_build-shared\bin) { "✅ FFmpeg DLLs OK" } else { "❌ FFmpeg DLLs missing" }
if (Test-Path C:\onnxruntime-win-x64-1.23.2\include) { "✅ ONNX Runtime headers OK" } else { "❌ ONNX Runtime headers missing" }
if (Test-Path C:\onnxruntime-win-x64-1.23.2\lib) { "✅ ONNX Runtime libs OK" } else { "❌ ONNX Runtime libs missing" }
if (Test-Path D:\git\vcpkg\scripts\buildsystems\vcpkg.cmake) { "✅ vcpkg OK" } else { "❌ vcpkg missing" }
```

## 🚀 性能提示

### 编译优化

在 `CMakeLists.txt` 中添加：
```cmake
# 启用优化
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /O2 /Ot")
```

### 运行时优化

```powershell
# 使用更高的置信度阈值加快处理
.\football_analytics.exe --player-conf 0.7 --keypoint-conf 0.8

# 减少输出日志
.\football_analytics.exe --video test.mp4 2>nul
```

## 📞 获取帮助

如果遇到问题：

1. 查看 `INSTALL_WINDOWS.md` - 详细安装指南
2. 查看 `README.md` - 项目文档
3. 查看 `QUICKSTART.md` - 快速开始
4. 检查本文件的常见问题部分
5. 提交 Issue 到项目仓库

---

**最后更新**: 2026-01-13  
**配置版本**: 1.0  
**测试平台**: Windows 11, Visual Studio 2022, CMake 3.28
