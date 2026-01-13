# 修复 400 错误 - 3步快速指南

## ❌ 问题
```
POST /api/frames HTTP/1.1" 400 -
[ERROR] Failed to parse JSON: 400 Bad Request
```

## ✅ 原因
C++ 发送的 JSON 中，Windows 路径的反斜杠没有转义。

**错误示例：**
```json
{"videoSource":"D:\test\video.mp4"}
```

**正确示例：**
```json
{"videoSource":"D:\\test\\video.mp4"}
```

---

## 🚀 修复步骤（3步）

### 步骤 1: 重新编译
```powershell
cd D:\git\Football-Analytics-with-Deep-Learning-and-Computer-Vision\cpp
.\rebuild.bat
```

等待显示：
```
[SUCCESS] Build complete!
```

### 步骤 2: 重启 API 服务器
```powershell
# 按 Ctrl+C 停止旧服务器
# 然后重新启动
python simple_api_server.py
```

### 步骤 3: 运行测试
```powershell
cd build\Release
.\football_analytics.exe --video test.mp4 --debug
```

---

## ✅ 预期结果

服务器应该显示：

```
[DEBUG] /api/video/start request received
  Raw data: {"videoSource":"D:\\test\\video.mp4",...}
                           ^^^ 注意双反斜杠

================================================================================
VIDEO PROCESSING STARTED
================================================================================
Video: test.mp4

[15:00:01] Frame    1 | Players: 18 | Keypoints: 12 | Balls: 1
```

**没有 400 错误！**

---

## 📋 检查清单

- [ ] 运行 `rebuild.bat`
- [ ] 看到 "Build successful!"
- [ ] 重启 API 服务器
- [ ] 运行 C++ 程序
- [ ] 没有 400 错误
- [ ] 看到帧处理日志

---

## 🔍 还是失败？

### 检查 1: 确认编译成功
```powershell
Get-Item build\Release\football_analytics.exe | Select-Object LastWriteTime
```
应该显示最近的时间。

### 检查 2: 查看发送的实际数据
```powershell
# 终端 1 - 启动调试服务器
python test_cpp_request.py

# 终端 2 - 运行程序
cd build\Release
.\football_analytics.exe --video test.mp4
```

在终端 1 查看 C++ 发送的完整请求。

### 检查 3: 测试 JSON 转义
```powershell
python test_json_escape.py
```

---

## 📖 详细文档

- **完整说明：** `JSON_ESCAPE_FIX.md`
- **调试指南：** `SOLVING_400_ERROR.md`
- **API 文档：** `API_SERVER_GUIDE.md`

---

**简而言之：运行 `rebuild.bat`，重启服务器，问题解决！** 🎉
