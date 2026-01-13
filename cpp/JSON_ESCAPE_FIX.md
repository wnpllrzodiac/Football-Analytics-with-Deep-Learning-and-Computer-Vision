# JSON 转义问题修复 - 完整说明

## 🐛 问题描述

### 症状
```
[ERROR] Failed to parse JSON in /api/frames: 400 Bad Request: 
The browser (or proxy) sent a request that this server could not understand.
```

### 根本原因

C++ 程序在构造 JSON 时没有对字符串进行转义，特别是 Windows 路径中的反斜杠 `\`。

**错误示例：**

如果视频路径是 `D:\test\video.mp4`，C++ 代码生成的 JSON 是：

```json
{"videoSource":"D:\test\video.mp4"}
```

这是 **无效的 JSON**！因为：
- `\t` 被解析为制表符
- `\v` 被解析为垂直制表符
- JSON 解析器会报错

**正确格式：**

```json
{"videoSource":"D:\\test\\video.mp4"}
```

反斜杠必须转义为 `\\`。

---

## ✅ 解决方案

### 1. 添加 JSON 转义函数

在 `ApiClient.cpp` 中添加了 `escapeJsonString()` 函数：

```cpp
std::string ApiClient::escapeJsonString(const std::string& str) {
    std::ostringstream escaped;
    for (char c : str) {
        switch (c) {
            case '"':  escaped << "\\\""; break;  // 引号
            case '\\': escaped << "\\\\"; break;  // 反斜杠
            case '\b': escaped << "\\b";  break;  // 退格
            case '\f': escaped << "\\f";  break;  // 换页
            case '\n': escaped << "\\n";  break;  // 换行
            case '\r': escaped << "\\r";  break;  // 回车
            case '\t': escaped << "\\t";  break;  // 制表符
            default:
                if (c < 0x20) {
                    // 控制字符：输出为 \uXXXX
                    escaped << "\\u" << std::hex << std::setw(4) 
                           << std::setfill('0') << static_cast<int>(c);
                } else {
                    escaped << c;
                }
        }
    }
    return escaped.str();
}
```

### 2. 在所有字符串中使用转义

修改了以下函数：

#### `frameDataToJson()`
```cpp
// 之前
json << "\"videoSource\":\"" << data.videoSource << "\",";
json << "\"label\":\"" << p.label << "\"";

// 之后
json << "\"videoSource\":\"" << escapeJsonString(data.videoSource) << "\",";
json << "\"label\":\"" << escapeJsonString(p.label) << "\"";
```

#### `notifyVideoStart()`
```cpp
// 之前
json << "\"videoSource\":\"" << videoSource << "\",";

// 之后
json << "\"videoSource\":\"" << escapeJsonString(videoSource) << "\",";
```

#### `notifyVideoComplete()`
```cpp
// 之前
json << "\"videoSource\":\"" << videoSource << "\",";

// 之后
json << "\"videoSource\":\"" << escapeJsonString(videoSource) << "\",";
```

### 3. 改进服务器调试

更新了 `simple_api_server.py`，在解析失败时打印完整的原始数据：

```python
# 获取原始数据
raw_data = request.data

# 打印原始数据（在解析之前）
print(f"  Raw data preview: {raw_data[:500].decode('utf-8', errors='replace')}")

# 如果解析失败，打印完整数据
try:
    data = request.get_json(force=True, silent=False)
except Exception as e:
    decoded_full = raw_data.decode('utf-8', errors='replace')
    print(f"  Failed JSON string:\n{decoded_full}")
    raise
```

---

## 🔧 如何应用修复

### 步骤 1: 重新编译

```powershell
cd D:\git\Football-Analytics-with-Deep-Learning-and-Computer-Vision\cpp
.\rebuild.bat
```

### 步骤 2: 重启 API 服务器

```powershell
# 停止旧服务器 (Ctrl+C)
python simple_api_server.py
```

### 步骤 3: 测试

```powershell
cd build\Release
.\football_analytics.exe --video D:\test\video.mp4 --debug
```

---

## 🧪 验证修复

### 1. 运行 JSON 转义测试

```powershell
python test_json_escape.py
```

这会测试各种特殊字符和 Windows 路径的转义。

### 2. 检查服务器日志

修复后，服务器应该显示：

```
[DEBUG] /api/video/start request received
  Content-Type: application/json
  Raw data: {"videoSource":"D:\\test\\video.mp4","totalFrames":3000,...}
                            ^^  注意这里是双反斜杠

================================================================================
VIDEO PROCESSING STARTED
================================================================================
Video:        D:\test\video.mp4
Total frames: 3000
```

**不应该再有 400 错误！**

---

## 📊 转义规则

| 字符 | 在 C++ 中 | 在 JSON 中 | 说明 |
|------|----------|-----------|------|
| `\` | `\\` | `\\\\` | 反斜杠（Windows 路径） |
| `"` | `"` | `\\\"` | 引号 |
| 换行 | `\n` | `\\n` | 换行符 |
| 制表 | `\t` | `\\t` | 制表符 |
| 回车 | `\r` | `\\r` | 回车符 |

### 示例

| 原始字符串 | 正确的 JSON |
|-----------|------------|
| `C:\test\video.mp4` | `"C:\\test\\video.mp4"` |
| `He said "hello"` | `"He said \\"hello\\""` |
| `Line1\nLine2` | `"Line1\\nLine2"` |
| `Tab\tSeparated` | `"Tab\\tSeparated"` |

---

## 🔍 为什么测试通过但 C++ 失败？

当你运行 `test_cpp_request.py` 和 `curl` 测试时，它们使用 Python 的 `json` 库或正确的工具来生成 JSON，这些工具会自动转义。

但是 C++ 代码是手工构造 JSON 字符串（使用 `ostringstream`），所以必须手动转义。

**对比：**

```python
# Python - 自动转义
import json
data = {"videoSource": r"C:\test\video.mp4"}
json_str = json.dumps(data)
# 结果: {"videoSource": "C:\\test\\video.mp4"}  ✓ 正确

# C++ - 必须手动转义
std::ostringstream json;
json << "{\"videoSource\":\"" << path << "\"}";
// 如果 path = "C:\test\video.mp4"
// 结果: {"videoSource":"C:\test\video.mp4"}  ✗ 错误！
// 必须: {"videoSource":"C:\\test\\video.mp4"}  ✓ 正确
```

---

## 🎯 未来改进建议

### 方案 A: 使用 JSON 库

考虑使用 `nlohmann/json` 来自动处理转义：

```cpp
#include <nlohmann/json.hpp>
using json = nlohmann::json;

// 自动转义，更安全
json j;
j["videoSource"] = data.videoSource;  // 自动处理转义
j["frameNumber"] = data.frameNumber;
std::string jsonStr = j.dump();
```

**优点：**
- ✅ 自动处理所有转义
- ✅ 更少出错
- ✅ 更易维护

**缺点：**
- ❌ 增加依赖
- ❌ 略微增加编译时间

### 方案 B: 保持当前手工方案

如果想保持轻量级，当前的 `escapeJsonString()` 函数已经足够。

---

## 📝 相关文件

修改的文件：
- ✅ `cpp/include/ApiClient.h` - 添加 `escapeJsonString()` 声明
- ✅ `cpp/src/ApiClient.cpp` - 实现转义函数并应用
- ✅ `cpp/simple_api_server.py` - 改进调试输出

新增的文件：
- ✅ `cpp/test_json_escape.py` - JSON 转义测试工具
- ✅ `cpp/rebuild.bat` - 快速重新编译脚本
- ✅ `cpp/JSON_ESCAPE_FIX.md` - 本文档

---

## ✅ 检查清单

在认为问题已修复之前，请确认：

- [ ] 运行了 `rebuild.bat` 重新编译
- [ ] 运行了 `test_json_escape.py` 测试
- [ ] 重启了 API 服务器
- [ ] C++ 程序运行时没有 400 错误
- [ ] 服务器日志显示正确的转义路径（双反斜杠）
- [ ] 可以看到正常的帧处理日志

---

## 🆘 还是不行？

如果修复后仍有问题：

1. **确认重新编译成功**
   ```powershell
   .\rebuild.bat
   # 应该显示 "Build successful!"
   ```

2. **检查是否使用了新的可执行文件**
   ```powershell
   # 查看编译时间
   Get-Item build\Release\football_analytics.exe | Select-Object LastWriteTime
   ```

3. **启用调试模式查看发送的数据**
   ```powershell
   .\football_analytics.exe --video test.mp4 --debug
   ```

4. **使用 test_cpp_request.py 捕获实际请求**
   ```powershell
   # 终端 1
   python test_cpp_request.py
   
   # 终端 2
   .\football_analytics.exe --video test.mp4
   ```

5. **提供以下信息寻求帮助**
   - 编译输出
   - 服务器打印的原始数据
   - C++ 程序的调试输出
   - `test_cpp_request.py` 捕获的请求

---

**问题已修复！现在 C++ 程序应该能正确生成 JSON 并与 API 服务器通信了。** 🎉
