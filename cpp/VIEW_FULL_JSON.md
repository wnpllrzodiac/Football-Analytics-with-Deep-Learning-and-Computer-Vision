# 如何查看完整的 JSON 数据

## 问题
服务器打印的 JSON 数据被截断了：
```
Raw data preview: {"frameNumber":27,"timestamp":1768287833172,"videoSource":"..\\..\\Streamlit web app\\demo_vid_1.mp4","players":[{"bbox":{"x":950,"y":437,"width":623,"height":181},"classId":0,"confidence":634.7478,"label":"player","teamId":1},{"bbox":{"x":920,"y":412,"width":539,"height":118},"classId":0,"confidence":594.3393,"label":"player","teamId":0},{"bbox":{"x":863,"y":400,"width":465,"height":74},"classId":0,"confidence":550.6458,"label":"player","teamId":0},{"bbox":{"x":847,"y":385,"width":427,"height":
```

## 解决方案

### 方法 1: 启用 DEBUG_JSON 模式（推荐）

#### 步骤 1: 编辑配置文件

打开 `simple_api_server.py`，找到这一行：

```python
DEBUG_JSON = False  # Set to True to print full JSON for each frame
```

改为：

```python
DEBUG_JSON = True  # Set to True to print full JSON for each frame
```

#### 步骤 2: 重启服务器

```powershell
# 停止旧服务器 (Ctrl+C)
python simple_api_server.py
```

现在每个请求会显示：

```json
[DEBUG] /api/frames request received
  Content-Type: application/json
  Content-Length: 2456
  Raw data length: 2456 bytes
  Parsed JSON (complete):
{
  "frameNumber": 27,
  "timestamp": 1768287833172,
  "videoSource": "..\\..\\Streamlit web app\\demo_vid_1.mp4",
  "players": [
    {
      "bbox": {
        "x": 950,
        "y": 437,
        "width": 623,
        "height": 181
      },
      "classId": 0,
      "confidence": 634.7478,
      "label": "player",
      "teamId": 1
    },
    ...完整的数据...
  ],
  "keypoints": [...],
  "balls": [...]
}
```

---

### 方法 2: 使用调试脚本（快速）

```powershell
# 直接启动调试模式服务器
.\start_api_debug.bat
```

这会自动启用 JSON 调试输出。

---

### 方法 3: 使用 test_cpp_request.py（最详细）

```powershell
# 终端 1 - 启动完整捕获服务器
python test_cpp_request.py

# 终端 2 - 运行 C++ 程序
cd build\Release
.\football_analytics.exe --video test.mp4
```

这会显示：
- 所有 HTTP 头
- 完整的原始数据（不截断）
- 格式化的 JSON

---

## 注意事项

### 性能影响

启用 `DEBUG_JSON = True` 会：
- ✅ 显示完整 JSON 数据
- ✅ 更容易调试
- ❌ 稍微降低性能（因为要格式化大量数据）
- ❌ 控制台输出非常多

**建议：**
- 调试时：`DEBUG_JSON = True`
- 生产运行：`DEBUG_JSON = False`

### 数据量限制

如果 JSON 非常大（超过 2000 字符），仍然会截断并显示：

```
Parsed JSON (first 2000 chars):
{
  ...数据...
}
... (truncated, total 5432 chars)
```

要查看更大的数据，修改这一行：

```python
if len(formatted) > 2000:  # 改成更大的值，如 10000
```

---

## 当前状态验证

### ✅ 好消息：JSON 格式正确！

从你提供的片段可以看到：

```json
"videoSource":"..\\..\\Streamlit web app\\demo_vid_1.mp4"
```

注意 `\\` （双反斜杠）— 这说明 C++ 的 `escapeJsonString()` 函数**正在工作**！

如果没有转义，会显示：
```json
"videoSource":"..\..\\Streamlit web app\\demo_vid_1.mp4"  ❌ 错误
```

但实际显示的是：
```json
"videoSource":"..\\..\\Streamlit web app\\demo_vid_1.mp4"  ✅ 正确
```

### 为什么还在打印原始数据？

因为 `VERBOSE_LOGGING = True`，它会在每个请求时打印调试信息。

要减少输出，设置：
```python
VERBOSE_LOGGING = False
```

然后只会看到帧处理进度：
```
[15:00:01] Frame    1 | Progress:   0.0% | Players: 18 | Keypoints: 12 | Balls: 1
[15:00:02] Frame   31 | Progress:   1.0% | Players: 20 | Keypoints: 14 | Balls: 1
```

---

## 配置对比

| 配置 | DEBUG_JSON | VERBOSE_LOGGING | 输出内容 |
|------|------------|-----------------|---------|
| 简洁模式 | False | False | 仅帧进度 |
| 正常模式 | False | True | 帧进度 + 简要调试信息 |
| 调试模式 | True | True | 帧进度 + 完整 JSON + 详细信息 |

---

## 快速命令

```powershell
# 简洁模式（推荐生产）
# 编辑文件：DEBUG_JSON = False, VERBOSE_LOGGING = False
python simple_api_server.py

# 正常模式（当前默认）
# 编辑文件：DEBUG_JSON = False, VERBOSE_LOGGING = True
python simple_api_server.py

# 调试模式（查看完整 JSON）
# 编辑文件：DEBUG_JSON = True, VERBOSE_LOGGING = True
python simple_api_server.py

# 或使用快捷脚本
.\start_api_debug.bat
```

---

## 总结

✅ **你的问题已经解决了！**

- JSON 格式正确（反斜杠已转义）
- 数据正在成功发送
- 只是显示被截断了（为了避免控制台输出过多）

如果想查看完整 JSON：
1. 设置 `DEBUG_JSON = True`
2. 重启服务器
3. 查看格式化的完整输出

**程序现在应该可以正常工作了！** 🎉
