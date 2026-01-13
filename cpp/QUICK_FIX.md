# 🚑 快速修复指南

## 当前配置状态

✅ **OpenCV**: `C:\OpenCV4.10.0\opencv\build` - 已配置  
✅ **FFmpeg**: `D:\sdk\ffmpeg-8.0.1-full_build-shared` - 已配置  
✅ **ONNX Runtime**: `C:\onnxruntime-win-x64-1.23.2` - 已配置  
⚠️ **cpp-httplib**: 需要下载真正的库文件

---

## ❌ 编译错误：httplib 方法缺失

### 错误信息
```
error C2039: 'set_connection_timeout': is not a member of 'httplib::Client'
error C2039: 'set_read_timeout': is not a member of 'httplib::Client'
```

### 🔧 立即修复（3步骤）

#### 步骤 1: 下载 cpp-httplib

**方法A - 自动下载（推荐）**:
```powershell
cd D:\git\Football-Analytics-with-Deep-Learning-and-Computer-Vision\cpp
.\download_httplib.ps1
```

**方法B - 手动下载**:
1. 访问: https://raw.githubusercontent.com/yhirose/cpp-httplib/master/httplib.h
2. 右键 → 另存为 → 保存到 `cpp\third_party\httplib.h`

#### 步骤 2: 验证下载

```powershell
# 检查文件大小（应该约700KB-1MB）
Get-Item .\third_party\httplib.h

# 如果显示很小（几KB），说明是占位符，需要重新下载
```

#### 步骤 3: 重新编译

```powershell
.\build.bat
```

---

## 📋 完整构建检查清单

在编译前确认以下项目：

### 依赖库
- [ ] OpenCV: `C:\OpenCV4.10.0\opencv\build`
  - [ ] 头文件: `include\opencv2\opencv.hpp`
  - [ ] 库文件: `x64\vc16\lib\opencv_world4100.lib`
  - [ ] DLL: `x64\vc16\bin\opencv_world4100.dll`

- [ ] FFmpeg: `D:\sdk\ffmpeg-8.0.1-full_build-shared`
  - [ ] 头文件: `include\libavformat\avformat.h`
  - [ ] 库文件: `lib\avformat.lib`, `lib\avcodec.lib`, 等
  - [ ] DLL: `bin\avformat-61.dll`, `bin\avcodec-61.dll`, 等

- [ ] ONNX Runtime: `C:\onnxruntime-win-x64-1.23.2`
  - [ ] 头文件: `include\onnxruntime_cxx_api.h`
  - [ ] 库文件: `lib\onnxruntime.lib`
  - [ ] DLL: `lib\onnxruntime.dll`

- [ ] cpp-httplib: `cpp\third_party\httplib.h`
  - [ ] 文件大小 > 700KB

### 项目文件
- [ ] 模型文件: `cpp\models\players.onnx`
- [ ] 模型文件: `cpp\models\keypoints.onnx`  
- [ ] 战术地图: `cpp\resources\tactical_map.jpg`

### 工具
- [ ] CMake 已安装
- [ ] Visual Studio 2019/2022 已安装
- [ ] Python 已安装（用于模型转换）

---

## 🎯 一键检查脚本

运行依赖检查脚本：

```powershell
.\check_dependencies.ps1
```

该脚本会自动检查所有依赖是否正确安装。

---

## 🚀 完整构建流程

如果是首次构建，按照以下顺序执行：

```powershell
# 1. 检查依赖
.\check_dependencies.ps1

# 2. 下载 cpp-httplib（如果尚未下载）
.\download_httplib.ps1

# 3. 转换模型（如果尚未转换）
python convert_models.py

# 4. 复制战术地图（如果尚未复制）
copy "..\tactical map.jpg" resources\tactical_map.jpg

# 5. 编译项目
.\build.bat

# 6. 复制运行时DLL
cd build\Release
copy C:\OpenCV4.10.0\opencv\build\x64\vc16\bin\opencv_world4100.dll .
copy D:\sdk\ffmpeg-8.0.1-full_build-shared\bin\*.dll .
copy C:\onnxruntime-win-x64-1.23.2\lib\onnxruntime.dll .

# 7. 运行程序
.\football_analytics.exe --video "..\..\Streamlit web app\demo_vid_1.mp4"
```

---

## 🔍 常见问题速查

### Q1: CMake配置失败

**解决**: 清理build目录后重试
```powershell
rmdir /s /q build
.\build.bat
```

### Q2: 找不到OpenCV

**原因**: vc版本不匹配  
**解决**: 检查 `C:\OpenCV4.10.0\opencv\build\x64\` 下有哪个vc版本（vc14/vc15/vc16/vc17），然后修改 `CMakeLists.txt` 第23行

### Q3: 运行时找不到DLL

**解决方案A - 复制DLL**:
```powershell
cd build\Release
copy C:\OpenCV4.10.0\opencv\build\x64\vc16\bin\*.dll .
copy D:\sdk\ffmpeg-8.0.1-full_build-shared\bin\*.dll .
copy C:\onnxruntime-win-x64-1.23.2\lib\onnxruntime.dll .
```

**解决方案B - 设置PATH**:
```powershell
$env:PATH += ";C:\OpenCV4.10.0\opencv\build\x64\vc16\bin;D:\sdk\ffmpeg-8.0.1-full_build-shared\bin;C:\onnxruntime-win-x64-1.23.2\lib"
```

### Q4: httplib 下载失败

**方法1**: 使用代理
```powershell
$env:HTTP_PROXY = "http://proxy:port"
.\download_httplib.ps1
```

**方法2**: 手动下载  
访问: https://github.com/yhirose/cpp-httplib  
下载: httplib.h  
保存到: `cpp\third_party\httplib.h`

**方法3**: 使用vcpkg
```powershell
cd D:\git\vcpkg
.\vcpkg install cpp-httplib:x64-windows
```

---

## 📞 获取更多帮助

- **详细文档**: 查看 `README.md`
- **安装指南**: 查看 `INSTALL_WINDOWS.md`
- **配置参考**: 查看 `CONFIG_REFERENCE.md`
- **httplib问题**: 查看 `HTTPLIB_SETUP.md`
- **快速开始**: 查看 `QUICKSTART.md`

---

**最后更新**: 2026-01-13  
**版本**: 1.1
