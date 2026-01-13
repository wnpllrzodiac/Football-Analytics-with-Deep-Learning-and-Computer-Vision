# cpp-httplib 库安装指南

## 问题说明

编译错误：
```
error C2039: 'set_connection_timeout': is not a member of 'httplib::Client'
error C2039: 'set_read_timeout': is not a member of 'httplib::Client'
```

这是因为 `third_party/httplib.h` 是一个占位符文件，不是完整的 cpp-httplib 库。

## 解决方案

### 方案 1: 自动下载（推荐）

运行PowerShell脚本自动下载：

```powershell
cd cpp
.\download_httplib.ps1
```

脚本会自动：
1. 备份现有的占位符文件
2. 从GitHub下载最新的 cpp-httplib
3. 保存到 `third_party/httplib.h`

### 方案 2: 手动下载

1. 访问 cpp-httplib GitHub仓库:
   ```
   https://github.com/yhirose/cpp-httplib
   ```

2. 下载 `httplib.h` 文件:
   ```
   https://raw.githubusercontent.com/yhirose/cpp-httplib/master/httplib.h
   ```

3. 替换项目中的文件:
   ```
   D:\git\Football-Analytics-with-Deep-Learning-and-Computer-Vision\cpp\third_party\httplib.h
   ```

### 方案 3: 使用vcpkg安装

```powershell
cd D:\git\vcpkg
.\vcpkg install cpp-httplib:x64-windows
```

然后修改 `CMakeLists.txt`：

```cmake
# 在 CMakeLists.txt 中添加
find_package(httplib CONFIG REQUIRED)

# 在 target_link_libraries 中添加
target_link_libraries(football_analytics
    ...
    httplib::httplib
)
```

## 验证安装

下载完成后，检查文件大小：

```powershell
Get-Item .\third_party\httplib.h | Select-Object Name, Length

# 应该显示类似：
# Name        Length
# ----        ------
# httplib.h   约 700KB-1MB
```

## 重新编译

```powershell
# 清理旧构建
rmdir /s /q build

# 重新编译
.\build.bat
```

## 关于 cpp-httplib

- **类型**: Header-only C++ HTTP库
- **作者**: yhirose
- **协议**: MIT License
- **GitHub**: https://github.com/yhirose/cpp-httplib
- **大小**: 单个头文件，约1MB
- **特点**:
  - 完全header-only，无需编译
  - 支持HTTP和HTTPS
  - 简单易用的API
  - 支持超时、重定向等特性

## 替代方案

如果不想使用HTTP API上传数据，可以：

1. **直接数据库连接**: 修改 `ApiClient.cpp` 使用MySQL/PostgreSQL C++客户端
2. **文件输出**: 将结果保存为JSON文件，而不是通过API上传
3. **禁用API功能**: 注释掉main.cpp中的API相关代码

### 示例：修改为文件输出

在 `ApiClient.cpp` 中：

```cpp
bool ApiClient::sendFrameData(const FrameData& data) {
    // 保存到文件而不是发送HTTP请求
    std::string filename = "frame_" + std::to_string(data.frameNumber) + ".json";
    std::ofstream file(filename);
    file << frameDataToJson(data);
    file.close();
    return true;
}
```

## 故障排除

### 下载失败

如果自动下载失败，可能是网络问题：

**临时解决方案（使用占位符）**:
当前的占位符已更新，包含基本方法定义。虽然不是完整实现，但可以通过编译。实际的HTTP请求不会真正发送，但不会影响其他功能的测试。

**代理设置**:
```powershell
# 设置代理
$env:HTTP_PROXY = "http://your-proxy:port"
$env:HTTPS_PROXY = "http://your-proxy:port"

# 然后重试下载
.\download_httplib.ps1
```

### 编译仍然失败

如果下载后仍然失败，检查：

1. 文件是否完整下载（应该约700KB-1MB）
2. 文件编码是否正确（应该是UTF-8）
3. 文件内容开头应该是：
   ```cpp
   //  _     _   _   _ _ _ _
   // | |   | | | | | (_) |
   // | |__ | | | |_| |_| | ___
   // |  _ \| | |  _  | | |/ _ \
   // | | | | | | | | | | |  __/
   // |_| |_|_| |_| |_|_|_|\___|
   ```

## 支持

如有问题，请：
1. 查看项目Issues
2. 访问 cpp-httplib 官方文档
3. 检查本文档的故障排除部分
