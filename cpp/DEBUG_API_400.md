# 调试 API 400 错误指南

## 问题现象

```
127.0.0.1 - - [13/Jan/2026 14:50:30] "POST /api/video/start HTTP/1.1" 400 -
127.0.0.1 - - [13/Jan/2026 14:50:33] "POST /api/frames HTTP/1.1" 400 -
```

HTTP 400 错误表示"Bad Request"（错误的请求），通常是由于：
1. ❌ JSON格式不正确
2. ❌ Content-Type头不正确
3. ❌ 请求体为空或损坏

---

## 快速修复步骤

### 步骤 1: 更新API服务器

已经更新了 `simple_api_server.py`，添加了详细的错误信息。

```powershell
# 重新启动服务器
cd cpp
python simple_api_server.py
```

### 步骤 2: 重新运行C++程序

现在服务器会打印详细的错误信息，告诉你到底哪里出错了。

---

## 诊断方法

### 方法 1: 使用 curl 测试（确认服务器正常）

```bash
# 测试健康端点（应该返回200）
curl http://localhost:8080/api/health

# 测试video/start（应该返回200）
curl -X POST http://localhost:8080/api/video/start \
  -H "Content-Type: application/json" \
  -d '{"videoSource":"test.mp4","totalFrames":100}'

# 如果返回400，查看服务器控制台的错误信息
```

### 方法 2: 检查C++端的HTTP请求

可能的问题：

#### 问题 A: Content-Type 头缺失或错误

在 `ApiClient.cpp` 中检查：

```cpp
httplib::Headers headers;
headers.emplace("Content-Type", "application/json");  // 必须有这个！
```

#### 问题 B: JSON格式错误

C++生成的JSON可能有问题。在 `ApiClient.cpp` 的 `sendPostRequest` 中添加调试：

```cpp
if (debugMode_) {
    std::cout << "POST " << host_ << ":" << port_ << fullPath << std::endl;
    std::cout << "Headers:" << std::endl;
    for (const auto& h : headers) {
        std::cout << "  " << h.first << ": " << h.second << std::endl;
    }
    std::cout << "Body: " << jsonData << std::endl;  // 打印完整JSON
}
```

---

## 常见问题和解决方案

### 问题 1: httplib.h 占位符版本

如果你用的是我之前创建的简化版 `httplib.h`，它可能不能正确发送POST请求。

**解决方案**: 下载真正的 cpp-httplib

```powershell
cd cpp
.\download_httplib.ps1
```

然后重新编译：
```powershell
.\build.bat
```

---

### 问题 2: JSON 序列化问题

C++代码中的JSON生成可能有问题。

**检查 ApiClient.cpp 的 `frameDataToJson` 函数**：

```cpp
std::string ApiClient::frameDataToJson(const FrameData& data) {
    std::ostringstream json;
    
    json << "{";
    json << "\"frameNumber\":" << data.frameNumber << ",";
    json << "\"timestamp\":" << data.timestamp << ",";
    json << "\"videoSource\":\"" << data.videoSource << "\"";
    // ... 确保所有引号和逗号都正确
    json << "}";
    
    return json.str();
}
```

**常见JSON错误**:
- ❌ 缺少引号: `{frameNumber:1}` 
- ✅ 正确格式: `{"frameNumber":1}`
- ❌ 多余逗号: `{"a":1,}`
- ✅ 正确格式: `{"a":1}`

---

### 问题 3: cpp-httplib 不发送Content-Type

某些版本的 cpp-httplib 可能不正确设置 Content-Type。

**临时解决方案**: 修改 `ApiClient.cpp`

```cpp
auto result = client.Post(fullPath.c_str(), jsonData, "application/json");
// 第三个参数明确指定Content-Type
```

---

## 调试技巧

### 技巧 1: 在Python端添加原始数据打印

已经更新的 `simple_api_server.py` 会打印：
- Content-Type header
- 原始请求数据
- JSON解析错误

### 技巧 2: 使用 Postman 或类似工具测试

1. 打开 Postman
2. 创建 POST 请求到 `http://localhost:8080/api/video/start`
3. Headers 添加: `Content-Type: application/json`
4. Body 选择 "raw" 和 "JSON"
5. 输入:
   ```json
   {
     "videoSource": "test.mp4",
     "totalFrames": 100,
     "timestamp": 1234567890
   }
   ```
6. 发送请求

如果Postman能成功，说明服务器正常，问题在C++端。

### 技巧 3: 使用 C++ 的 --debug 模式

```powershell
.\football_analytics.exe --video test.mp4 --debug
```

这会打印详细的HTTP请求信息。

---

## 验证清单

运行C++程序前，确认：

- [ ] API服务器正在运行
- [ ] 使用最新的 `simple_api_server.py`（有错误处理）
- [ ] 使用真正的 `cpp-httplib`（不是占位符）
- [ ] C++程序用 `--debug` 模式运行
- [ ] 查看服务器控制台的错误信息

---

## 测试流程

### 1. 测试服务器

```powershell
# 终端 1: 启动服务器
cd cpp
python simple_api_server.py
```

```powershell
# 终端 2: 测试
curl http://localhost:8080/api/health
# 应该看到: {"status":"healthy",...}

curl -X POST http://localhost:8080/api/test \
  -H "Content-Type: application/json" \
  -d '{"test":"data"}'
# 服务器应该打印接收到的JSON
```

### 2. 测试C++程序

```powershell
cd cpp\build\Release
.\football_analytics.exe --video test.mp4 --debug
```

**观察服务器输出**，应该看到：
- 如果成功：正常的日志输出
- 如果失败：`[ERROR]` 开头的详细错误信息

---

## 示例：正确的请求

### video/start 请求

```http
POST /api/video/start HTTP/1.1
Host: localhost:8080
Content-Type: application/json

{"videoSource":"demo.mp4","totalFrames":3000,"timestamp":1673611830000}
```

### frames 请求

```http
POST /api/frames HTTP/1.1
Host: localhost:8080
Content-Type: application/json

{
  "frameNumber":1,
  "timestamp":1673611830000,
  "videoSource":"demo.mp4",
  "players":[
    {
      "bbox":{"x":100,"y":200,"width":50,"height":80},
      "classId":0,
      "confidence":0.95,
      "label":"player",
      "teamId":0,
      "tacMapPosition":{"x":450.5,"y":320.2}
    }
  ],
  "keypoints":[],
  "balls":[]
}
```

---

## 下一步

1. **重启服务器**（使用更新的代码）
2. **运行C++程序**
3. **查看服务器控制台** - 现在会显示详细错误
4. **根据错误信息修复问题**

常见的错误信息：
- `No JSON data in request` → Content-Type或请求体问题
- `Failed to parse JSON` → JSON格式错误
- `Connection refused` → 服务器未运行

---

## 获取帮助

如果问题仍然存在：

1. **复制完整的错误信息**（包括服务器输出）
2. **使用 `--debug` 模式运行C++程序**
3. **测试 curl 命令是否工作**
4. **检查 cpp-httplib 版本**

**相关文件**:
- `simple_api_server.py` - API服务器（已更新）
- `ApiClient.cpp` - C++端HTTP客户端
- `HTTPLIB_SETUP.md` - httplib库安装指南
