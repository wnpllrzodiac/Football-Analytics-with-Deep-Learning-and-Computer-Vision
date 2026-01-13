# 模型转换指南

## 问题：程序在 "Loading detection models..." 后退出

**原因**: ONNX模型文件不存在或路径不正确

---

## 快速修复步骤

### 步骤 1: 安装 Python 依赖

```powershell
pip install ultralytics
```

### 步骤 2: 运行模型转换

```powershell
cd D:\git\Football-Analytics-with-Deep-Learning-and-Computer-Vision\cpp
python convert_models_en.py
```

### 步骤 3: 验证转换结果

```powershell
dir models

# 应该看到:
# models\players.onnx
# models\keypoints.onnx
```

### 步骤 4: 复制战术地图

```powershell
copy "..\tactical map.jpg" resources\tactical_map.jpg
```

### 步骤 5: 重新运行程序

```powershell
cd build\Release
.\football_analytics.exe --video "..\..\Streamlit web app\demo_vid_1.mp4"
```

---

## 详细说明

### 模型文件位置

**原始 PyTorch 模型** (在项目根目录):
```
Football-Analytics-with-Deep-Learning-and-Computer-Vision\
├── models\
│   ├── Yolo8L Players\
│   │   └── weights\
│   │       └── best.pt          # 球员检测模型
│   └── Yolo8M Field Keypoints\
│       └── weights\
│           └── best.pt          # 关键点检测模型
```

**转换后的 ONNX 模型** (在cpp目录):
```
cpp\
└── models\
    ├── players.onnx             # 转换后的球员检测模型
    └── keypoints.onnx           # 转换后的关键点检测模型
```

### 转换过程

转换脚本会:
1. 读取项目根目录的 PyTorch `.pt` 模型
2. 使用 ultralytics 库转换为 ONNX 格式
3. 保存到 `cpp\models\` 目录

### 预期输出

成功转换后应该看到:

```
============================================================
Football Analytics - Model Conversion Tool
============================================================

Output directory: D:\git\Football-Analytics-with-Deep-Learning-and-Computer-Vision\cpp\models
Found 2 models to convert

[1/2] Processing: Player Detection Model
------------------------------------------------------------
Converting model: D:\git\Football-Analytics-with-Deep-Learning-and-Computer-Vision\models\Yolo8L Players\weights\best.pt
...
SUCCESS: Converted to ...models\players.onnx
  File size: XX.XX MB

[2/2] Processing: Keypoint Detection Model
------------------------------------------------------------
Converting model: D:\git\Football-Analytics-with-Deep-Learning-and-Computer-Vision\models\Yolo8M Field Keypoints\weights\best.pt
...
SUCCESS: Converted to ...models\keypoints.onnx
  File size: XX.XX MB

============================================================
Conversion Results Summary
============================================================
SUCCESS: Player Detection Model
SUCCESS: Keypoint Detection Model

============================================================
Total: 2/2 models converted successfully
============================================================

All models converted successfully!
```

---

## 故障排除

### 问题 1: ultralytics 未安装

**错误信息**:
```
Error: ultralytics library not installed
```

**解决方案**:
```powershell
pip install ultralytics torch torchvision
```

### 问题 2: 找不到 PyTorch 模型文件

**错误信息**:
```
Error: Model file not found: ...
```

**解决方案**:
1. 确认原始模型文件存在:
   ```powershell
   dir "..\models\Yolo8L Players\weights\best.pt"
   dir "..\models\Yolo8M Field Keypoints\weights\best.pt"
   ```

2. 如果不存在，需要先训练模型或从项目源获取模型文件

### 问题 3: 转换过程中出错

**可能原因**:
- PyTorch 版本不兼容
- 模型文件损坏
- 磁盘空间不足

**解决方案**:
```powershell
# 更新依赖
pip install --upgrade ultralytics torch torchvision

# 检查磁盘空间
```

### 问题 4: 程序仍然无法加载模型

**检查列表**:
1. 确认模型文件存在:
   ```powershell
   Test-Path cpp\models\players.onnx
   Test-Path cpp\models\keypoints.onnx
   ```

2. 检查模型文件大小（不应该是0字节）:
   ```powershell
   Get-Item cpp\models\*.onnx | Select-Object Name, Length
   ```

3. 确认C++程序中的模型路径正确:
   - 默认路径: `./models/players.onnx` 和 `./models/keypoints.onnx`
   - 相对于可执行文件的位置

---

## 手动指定模型路径

如果需要使用不同的模型位置，可以通过命令行参数指定:

```powershell
.\football_analytics.exe ^
  --video demo.mp4 ^
  --player-model "D:\path\to\players.onnx" ^
  --keypoint-model "D:\path\to\keypoints.onnx"
```

---

## 验证模型文件

### 使用 Python 验证 ONNX 模型

```python
import onnx

# 检查球员检测模型
model = onnx.load("cpp/models/players.onnx")
onnx.checker.check_model(model)
print("Player model is valid!")

# 检查关键点检测模型
model = onnx.load("cpp/models/keypoints.onnx")
onnx.checker.check_model(model)
print("Keypoint model is valid!")
```

---

## 一键完成所有步骤

创建批处理脚本 `setup_models.bat`:

```batch
@echo off
echo Installing Python dependencies...
pip install ultralytics

echo Converting models...
cd cpp
python convert_models_en.py

echo Copying tactical map...
copy "..\tactical map.jpg" resources\tactical_map.jpg

echo Setup complete!
pause
```

然后运行:
```powershell
.\setup_models.bat
```

---

## 支持

如有问题:
1. 查看 `QUICK_FIX.md`
2. 查看 `README.md`
3. 提交 Issue 到项目仓库
