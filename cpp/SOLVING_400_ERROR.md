# 解决API 400错误 - 实战指南

## 当前状态

你遇到的错误：
```
[ERROR] Failed to parse JSON in /api/frames: 400 Bad Request: 
The browser (or proxy) sent a request that this server could not understand.
```

这意味着Flask无法理解C++程序发送的HTTP请求。

---

## 🚀 快速解决步骤

### 步骤 1: 重启API服务器（使用更新的版本）

更新后的服务器会：
- ✅ 打印详细的请求信息
- ✅ 显示Content-Type和原始数据
- ✅ 使用 `force=True` 强制解析JSON（即使Content-Type不对）

```powershell
# 终端 1: 停止旧服务器（Ctrl+C）并启动新的
cd D:\git\Football-Analytics-with-Deep-Learning-and-Computer-Vision\cpp
python simple_api_server.py
```

### 步骤 2: 运行C++程序

```powershell
# 终端 2
cd build\Release
.\football_analytics.exe --video test.mp4 --debug
```

### 步骤 3: 查看服务器输出

现在服务器会显示类似这样的调试信息：

```
[DEBUG] /api/frames request received
  Content-Type: text/plain
  Content-Length: 245
  Method: POST
  Raw data length: 245 bytes
  Raw data preview: b'{"frameNumber":1,"timestamp":1234567890,...}'
```

**关键点查看：**
1. **Content-Type** - 应该是 `application/json`
2. **Raw data** - 应该是有效的JSON字符串

---

## 🔍 方法 2: 使用调试服务器查看完整请求

如果上面的方法还不够清楚，使用专门的调试服务器：

```powershell
# 停止 simple_api_server.py
# 启动调试服务器
python test_cpp_request.py
```

这个调试服务器会打印：
- ✅ 所有HTTP头
- ✅ 完整的原始数据
- ✅ 解码后的数据
- ✅ 如果是JSON，会格式化显示

然后运行C++程序，你会看到它实际发送的所有内容。

---

## 🐛 常见问题和解决方案

### 问题 1: Content-Type 不正确

**症状：**
```
Content-Type: text/plain
```
或
```
Content-Type: None
```

**原因：** C++ httplib 没有正确设置 Content-Type

**解决方案：** 检查 `ApiClient.cpp` 中的代码

找到这部分：
```cpp
auto result = client.Post(fullPath.c_str(), headers, jsonData, "application/json");
```

确保第4个参数是 `"application/json"`

---

### 问题 2: 使用了占位符版本的 httplib.h

**症状：**
- 编译成功但HTTP请求完全不工作
- 或者返回奇怪的错误

**解决方案：**

```powershell
cd cpp

# 1. 下载真正的 cpp-httplib
.\download_httplib.ps1

# 2. 检查文件大小（应该约700KB-1MB）
Get-Item third_party\httplib.h

# 3. 重新编译
.\build.bat
```

---

### 问题 3: JSON 格式错误

**症状：**
```
Raw data: b'{frameNumber:1,timestamp:1234567890}'
```

**问题：** JSON 键没有引号

**正确格式：**
```json
{"frameNumber":1,"timestamp":1234567890}
```

**检查 `ApiClient.cpp` 的 `frameDataToJson()` 函数：**

```cpp
// ❌ 错误 - 缺少引号
json << "{frameNumber:" << data.frameNumber << "}";

// ✅ 正确
json << "{\"frameNumber\":" << data.frameNumber << "}";
```

---

### 问题 4: 字符串值没有转义

**症状：**
```
Raw data: b'{"videoSource":"C:\video\test.mp4"}'
```

**问题：** 反斜杠需要转义

**解决：** 在 `ApiClient.cpp` 中转义特殊字符

```cpp
std::string escapeJson(const std::string& str) {
    std::string escaped;
    for (char c : str) {
        switch (c) {
            case '\"': escaped += "\\\""; break;
            case '\\': escaped += "\\\\"; break;
            case '\n': escaped += "\\n"; break;
            case '\r': escaped += "\\r"; break;
            case '\t': escaped += "\\t"; break;
            default: escaped += c;
        }
    }
    return escaped;
}

// 使用
json << "\"videoSource\":\"" << escapeJson(data.videoSource) << "\"";
```

---

## 📊 完整诊断流程

### 1. 测试服务器是否正常（用curl）

```bash
# 应该返回 200
curl -X POST http://localhost:8080/api/frames \
  -H "Content-Type: application/json" \
  -d '{"frameNumber":1,"timestamp":1234567890,"players":[],"keypoints":[],"balls":[]}'
```

如果这个成功，说明服务器正常，问题在C++端。

### 2. 查看C++发送的实际内容

使用 `test_cpp_request.py`：

```powershell
# 终端 1
python test_cpp_request.py

# 终端 2
cd build\Release
.\football_analytics.exe --video test.mp4
```

观察终端 1 的输出，看到底发送了什么。

### 3. 比较正确的请求和C++的请求

**正确的请求应该是：**
```http
POST /api/frames HTTP/1.1
Host: localhost:8080
Content-Type: application/json
Content-Length: 245

{"frameNumber":1,"timestamp":1234567890,"videoSource":"test.mp4",...}
```

**对比C++发送的是否一样。**

---

## 🔧 临时解决方案

如果暂时无法修复C++端，可以修改API服务器使其更宽容：

### 方案 A: 接受任何Content-Type

已经在更新的 `simple_api_server.py` 中实现（使用 `force=True`）

### 方案 B: 创建一个完全宽容的端点

```python
@app.route('/api/frames_raw', methods=['POST'])
def receive_frame_raw():
    """Accept any data format"""
    import json
    
    # Try to parse whatever we got
    try:
        if request.is_json:
            data = request.get_json()
        else:
            # Force parse as JSON
            data = json.loads(request.data.decode('utf-8'))
        
        print(f"Received frame {data.get('frameNumber', 0)}")
        return {'status': 'success'}, 200
    except Exception as e:
        print(f"Error: {e}")
        # Still return success to avoid blocking
        return {'status': 'error', 'message': str(e)}, 200
```

然后在C++中使用 `/api/frames_raw` 端点。

---

## ✅ 验证修复

修复后，服务器应该显示：

```
[DEBUG] /api/frames request received
  Content-Type: application/json
  Content-Length: 245
  Method: POST
  Raw data length: 245 bytes

[10:30:15] Frame    1 | Progress:   0.0% | Players: 18 | Keypoints: 12 | Balls: 1
```

没有 `[ERROR]` 信息，并且能看到正常的帧处理日志。

---

## 📝 检查清单

在报告问题前，请确认：

- [ ] 运行了更新后的 `simple_api_server.py`
- [ ] 使用真正的 `cpp-httplib`（不是占位符）
- [ ] 运行了 `test_cpp_request.py` 查看实际请求
- [ ] 用 curl 测试服务器正常工作
- [ ] 查看了服务器打印的调试信息
- [ ] 检查了 `ApiClient.cpp` 的 JSON 生成代码

---

## 🆘 还是不行？

提供以下信息：

1. **服务器输出**（包括 [DEBUG] 信息）
2. **C++程序输出**（使用 --debug）
3. **test_cpp_request.py 捕获的完整请求**
4. **curl 测试结果**
5. **httplib.h 文件大小**

---

**相关文件：**
- `simple_api_server.py` - 已更新，有详细调试
- `test_cpp_request.py` - 调试服务器
- `ApiClient.cpp` - C++端HTTP客户端
- `DEBUG_API_400.md` - 详细调试指南
