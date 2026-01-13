# Football Analytics C++ - 完整工具集

本项目包含完整的工具链，用于构建、运行和调试C++版本的足球分析程序。

---

## 📚 文档索引

### 快速开始
- **`QUICKSTART.md`** - 从零开始的完整指南
- **`QUICK_FIX.md`** - 常见问题快速修复
- **`CONFIG_REFERENCE.md`** - 配置参考速查

### 安装配置
- **`README.md`** - 项目总体文档和架构说明
- **`INSTALL_WINDOWS.md`** - Windows详细安装指南
- **`ALL_TOOLS_README.md`** - 本文件：工具集总览

### 专题指南
- **`HTTPLIB_SETUP.md`** - cpp-httplib 库安装指南
- **`ONNX_TROUBLESHOOTING.md`** - ONNX Runtime 故障排查
- **`MODEL_CONVERT_GUIDE.md`** - 模型转换详细指南
- **`API_SERVER_GUIDE.md`** - API 服务器使用指南
- **`GPU_ACCELERATION.md`** - GPU 加速配置指南（CUDA/DirectML）
- **`FIX_CUDA_ERROR.md`** - 修复 CUDA provider 错误

### 调试指南
- **`DEBUG_API_400.md`** - API 400 错误调试
- **`JSON_ESCAPE_FIX.md`** - JSON 转义问题修复
- **`SOLVING_400_ERROR.md`** - 解决 API 400 错误实战
- **`FIX_400_QUICK_GUIDE.md`** - 400 错误快速修复（3步）
- **`VIEW_FULL_JSON.md`** - 查看完整 JSON 数据

---

## 🛠️ 工具脚本

### 构建和编译
```powershell
build.bat              # 主构建脚本（CMake + 编译）
rebuild.bat            # 快速重新编译（不配置）
```

### 模型转换
```powershell
convert_models.py      # 模型转换脚本（中文）
convert_models_en.py   # 模型转换脚本（英文，推荐）
setup_models.bat       # 一键模型设置
```

### 依赖检查
```powershell
check_dependencies.ps1 # 检查所有依赖库
diagnose_onnx.ps1      # 诊断 ONNX Runtime 问题
```

### 依赖下载和修复
```powershell
download_httplib.ps1   # 下载 cpp-httplib 库
fix_onnx.ps1          # 修复 ONNX Runtime DLL 问题
```

### API 服务器
```powershell
simple_api_server.py   # 简单API服务器（推荐）
example_api_server.py  # 完整API服务器（带数据存储）
start_api_server.bat   # 一键启动API服务器（正常模式）
start_api_debug.bat    # 启动API服务器（调试模式，显示完整JSON）
test_api.py           # API服务器测试脚本
test_cpp_request.py    # 捕获C++发送的完整请求
test_json_escape.py    # JSON转义测试工具
quick_test_api.bat     # 快速测试API
```

---

## 🚀 典型工作流程

### 首次设置（完整流程）

```powershell
# 1. 检查依赖
cd D:\git\Football-Analytics-with-Deep-Learning-and-Computer-Vision\cpp
.\check_dependencies.ps1

# 2. 下载 cpp-httplib
.\download_httplib.ps1

# 3. 修复 ONNX Runtime
.\fix_onnx.ps1

# 4. 设置模型
.\setup_models.bat

# 5. 编译项目
.\build.bat

# 6. 启动 API 服务器（新终端）
.\start_api_server.bat

# 7. 运行程序（另一个终端）
cd build\Release
.\football_analytics.exe --video "..\..\Streamlit web app\demo_vid_1.mp4"
```

---

### 日常使用（已设置完成）

```powershell
# 终端 1: 启动 API 服务器
cd cpp
python simple_api_server.py

# 终端 2: 运行程序
cd cpp\build\Release
.\football_analytics.exe --video test.mp4
```

---

### 故障排查流程

```powershell
# 遇到编译错误
.\check_dependencies.ps1  # 检查所有依赖

# 遇到 httplib 错误
.\download_httplib.ps1    # 下载正确的 httplib

# 遇到 ONNX Runtime 错误
.\diagnose_onnx.ps1       # 诊断问题
.\fix_onnx.ps1           # 自动修复

# 遇到模型加载错误
python convert_models_en.py  # 重新转换模型
```

---

## 📋 完整命令参考

### 编译相关

```powershell
# 完整构建
.\build.bat

# 清理重建
rmdir /s /q build
.\build.bat

# 只配置 CMake
cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=D:/git/vcpkg/scripts/buildsystems/vcpkg.cmake

# 只编译
cmake --build . --config Release
```

### 模型相关

```powershell
# 转换模型
python convert_models_en.py

# 检查模型文件
dir models\*.onnx
Get-Item models\*.onnx | Format-Table Name, @{Label="Size(MB)"; Expression={[math]::Round($_.Length/1MB, 2)}}
```

### API 服务器

```powershell
# 启动简单服务器
python simple_api_server.py

# 启动完整服务器（带数据保存）
python example_api_server.py

# 测试服务器
python test_api.py

# 测试健康端点
curl http://localhost:8080/api/health
```

### 程序运行

```powershell
# 基本运行
.\football_analytics.exe --video demo.mp4

# 完整参数
.\football_analytics.exe ^
  --video demo.mp4 ^
  --api-url http://localhost:8080 ^
  --player-conf 0.6 ^
  --keypoint-conf 0.7 ^
  --team1-name "Team A" ^
  --team2-name "Team B" ^
  --debug

# 查看帮助
.\football_analytics.exe --help
```

---

## 🔧 配置文件

### CMake 配置
- **`CMakeLists.txt`** - CMake 构建配置

### 项目配置
- **`config/config_players.json`** - 球员检测配置
- **`config/config_pitch.json`** - 球场关键点配置
- **`config/pitch_map_labels.json`** - 球场关键点映射

### 构建配置
- **`build.bat`** - 构建脚本配置
  - VCPKG_ROOT
  - ONNXRUNTIME_DIR
  - FFMPEG_DIR
  - OPENCV_DIR

---

## 📁 目录结构

```
cpp/
├── 📄 文档 (*.md)
│   ├── README.md
│   ├── QUICKSTART.md
│   ├── QUICK_FIX.md
│   ├── INSTALL_WINDOWS.md
│   ├── CONFIG_REFERENCE.md
│   ├── HTTPLIB_SETUP.md
│   ├── ONNX_TROUBLESHOOTING.md
│   ├── MODEL_CONVERT_GUIDE.md
│   ├── API_SERVER_GUIDE.md
│   └── ALL_TOOLS_README.md (本文件)
│
├── 🛠️ 构建工具
│   └── build.bat
│
├── 🔧 依赖工具
│   ├── check_dependencies.ps1
│   ├── diagnose_onnx.ps1
│   ├── download_httplib.ps1
│   └── fix_onnx.ps1
│
├── 🔄 模型工具
│   ├── convert_models.py
│   ├── convert_models_en.py
│   └── setup_models.bat
│
├── 🌐 API 服务器
│   ├── simple_api_server.py
│   ├── example_api_server.py
│   ├── start_api_server.bat
│   └── test_api.py
│
├── 📂 源代码
│   ├── include/          # 头文件
│   ├── src/             # 源文件
│   ├── third_party/     # 第三方库
│   ├── config/          # 配置文件
│   ├── models/          # ONNX 模型
│   └── resources/       # 资源文件
│
└── 📦 构建输出
    └── build/           # CMake 构建目录
```

---

## 🎯 按场景选择工具

### 场景 1: 首次安装
1. `check_dependencies.ps1` - 检查环境
2. `download_httplib.ps1` - 下载库
3. `fix_onnx.ps1` - 配置 ONNX Runtime
4. `setup_models.bat` - 设置模型
5. `build.bat` - 编译项目

### 场景 2: 编译失败
1. `check_dependencies.ps1` - 确认依赖
2. 查看对应的故障排查文档
3. 运行对应的修复脚本

### 场景 3: 运行时错误
1. `diagnose_onnx.ps1` - 诊断 ONNX 问题
2. `fix_onnx.ps1` - 自动修复
3. 检查模型文件

### 场景 4: 开发测试
1. `start_api_server.bat` - 启动 API 服务器
2. `test_api.py` - 测试 API
3. 运行程序并观察日志

---

## 💡 最佳实践

### 开发环境设置
1. 使用 PowerShell（而非 CMD）
2. 以管理员权限运行需要的命令
3. 保持依赖库版本一致

### 调试技巧
1. 使用 `--debug` 参数运行程序
2. 查看 API 服务器日志
3. 使用诊断脚本确认环境

### 性能优化
1. 使用 Release 模式编译
2. 调整置信度阈值
3. 考虑使用 TensorRT（GPU加速）

---

## 🆘 获取帮助

### 快速参考
- **常见问题**: `QUICK_FIX.md`
- **配置问题**: `CONFIG_REFERENCE.md`
- **ONNX 问题**: `ONNX_TROUBLESHOOTING.md`
- **模型问题**: `MODEL_CONVERT_GUIDE.md`
- **API 问题**: `API_SERVER_GUIDE.md`

### 诊断工具
```powershell
# 运行所有诊断
.\check_dependencies.ps1
.\diagnose_onnx.ps1

# 查看程序帮助
.\football_analytics.exe --help
```

### 社区支持
- 查看项目 Issues
- 阅读相关文档
- 运行诊断脚本获取详细信息

---

## 📝 更新日志

**Version 1.0** (2026-01-13)
- 完整的C++实现
- 所有工具脚本
- 详细文档集

---

**祝您使用愉快！** 🎉

如有问题，请先查看对应的文档和运行诊断脚本。
