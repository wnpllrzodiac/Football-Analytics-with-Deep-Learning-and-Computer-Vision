#!/usr/bin/env python3
"""
Football Analytics - 模型转换脚本
将PyTorch YOLOv8模型转换为ONNX格式
"""

import os
import sys
from pathlib import Path

def check_ultralytics():
    """检查ultralytics是否已安装"""
    try:
        from ultralytics import YOLO
        return True
    except ImportError:
        print("错误: 未安装ultralytics库")
        print("请运行: pip install ultralytics")
        return False

def convert_model(pt_path, output_dir, model_name):
    """
    转换单个模型
    
    Args:
        pt_path: PyTorch模型路径
        output_dir: 输出目录
        model_name: 输出模型名称
    """
    from ultralytics import YOLO
    
    if not os.path.exists(pt_path):
        print(f"错误: 模型文件不存在: {pt_path}")
        return False
    
    print(f"\n正在转换模型: {pt_path}")
    print(f"输出路径: {output_dir}/{model_name}")
    
    try:
        # 加载模型
        model = YOLO(pt_path)
        
        # 导出为ONNX
        success = model.export(
            format='onnx',
            imgsz=640,          # 输入尺寸
            dynamic=False,      # 固定批次大小
            simplify=True,      # 简化ONNX图
            opset=12,           # ONNX操作集版本
            half=False          # 使用FP32精度
        )
        
        if success:
            # 移动并重命名文件
            exported_file = str(Path(pt_path).with_suffix('.onnx'))
            output_file = os.path.join(output_dir, model_name)
            
            if os.path.exists(exported_file):
                os.makedirs(output_dir, exist_ok=True)
                
                # 如果目标文件已存在，先删除
                if os.path.exists(output_file):
                    os.remove(output_file)
                
                # 移动文件
                os.rename(exported_file, output_file)
                print(f"✓ 转换成功: {output_file}")
                
                # 显示文件大小
                file_size = os.path.getsize(output_file) / (1024 * 1024)
                print(f"  文件大小: {file_size:.2f} MB")
                
                return True
            else:
                print(f"✗ 导出文件未找到: {exported_file}")
                return False
        else:
            print("✗ 模型导出失败")
            return False
            
    except Exception as e:
        print(f"✗ 转换过程中出错: {str(e)}")
        return False

def main():
    """主函数"""
    print("=" * 60)
    print("Football Analytics - 模型转换工具")
    print("=" * 60)
    
    # 检查依赖
    if not check_ultralytics():
        sys.exit(1)
    
    # 定义模型路径（相对于项目根目录）
    project_root = Path(__file__).parent.parent
    
    models_to_convert = [
        {
            'name': '球员检测模型',
            'pt_path': project_root / 'models' / 'Yolo8L Players' / 'weights' / 'best.pt',
            'output_name': 'players.onnx'
        },
        {
            'name': '关键点检测模型',
            'pt_path': project_root / 'models' / 'Yolo8M Field Keypoints' / 'weights' / 'best.pt',
            'output_name': 'keypoints.onnx'
        }
    ]
    
    # 输出目录
    output_dir = Path(__file__).parent / 'models'
    output_dir.mkdir(exist_ok=True)
    
    print(f"\n输出目录: {output_dir}")
    print(f"找到 {len(models_to_convert)} 个模型待转换\n")
    
    # 转换所有模型
    results = []
    for i, model_info in enumerate(models_to_convert, 1):
        print(f"\n[{i}/{len(models_to_convert)}] 正在处理: {model_info['name']}")
        print("-" * 60)
        
        success = convert_model(
            str(model_info['pt_path']),
            str(output_dir),
            model_info['output_name']
        )
        
        results.append((model_info['name'], success))
    
    # 显示总结
    print("\n" + "=" * 60)
    print("转换结果总结")
    print("=" * 60)
    
    for name, success in results:
        status = "✓ 成功" if success else "✗ 失败"
        print(f"{status}: {name}")
    
    # 统计
    success_count = sum(1 for _, success in results if success)
    total_count = len(results)
    
    print("\n" + "=" * 60)
    print(f"总计: {success_count}/{total_count} 个模型转换成功")
    print("=" * 60)
    
    if success_count == total_count:
        print("\n✓ 所有模型转换完成！")
        print("\n下一步:")
        print("1. 复制 tactical_map.jpg 到 cpp/resources/ 目录")
        print("2. 运行 build.bat 编译C++项目")
        print("3. 运行程序: football_analytics.exe --video path/to/video.mp4")
        return 0
    else:
        print("\n✗ 部分模型转换失败，请检查错误信息")
        return 1

if __name__ == '__main__':
    sys.exit(main())
