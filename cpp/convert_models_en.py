#!/usr/bin/env python3
"""
Football Analytics - Model Conversion Script
Convert PyTorch YOLOv8 models to ONNX format
"""

import os
import sys
from pathlib import Path

def check_ultralytics():
    """Check if ultralytics is installed"""
    try:
        from ultralytics import YOLO
        return True
    except ImportError:
        print("Error: ultralytics library not installed")
        print("Please run: pip install ultralytics")
        return False

def convert_model(pt_path, output_dir, model_name):
    """
    Convert a single model
    
    Args:
        pt_path: PyTorch model path
        output_dir: Output directory
        model_name: Output model name
    """
    from ultralytics import YOLO
    
    if not os.path.exists(pt_path):
        print(f"Error: Model file not found: {pt_path}")
        return False
    
    print(f"\nConverting model: {pt_path}")
    print(f"Output path: {output_dir}/{model_name}")
    
    try:
        # Load model
        model = YOLO(pt_path)
        
        # Export to ONNX
        success = model.export(
            format='onnx',
            imgsz=640,          # Input size
            dynamic=False,      # Fixed batch size
            simplify=True,      # Simplify ONNX graph
            opset=12,           # ONNX opset version
            half=False          # Use FP32 precision
        )
        
        if success:
            # Move and rename file
            exported_file = str(Path(pt_path).with_suffix('.onnx'))
            output_file = os.path.join(output_dir, model_name)
            
            if os.path.exists(exported_file):
                os.makedirs(output_dir, exist_ok=True)
                
                # Remove existing file if present
                if os.path.exists(output_file):
                    os.remove(output_file)
                
                # Move file
                os.rename(exported_file, output_file)
                print(f"SUCCESS: Converted to {output_file}")
                
                # Display file size
                file_size = os.path.getsize(output_file) / (1024 * 1024)
                print(f"  File size: {file_size:.2f} MB")
                
                return True
            else:
                print(f"ERROR: Exported file not found: {exported_file}")
                return False
        else:
            print("ERROR: Model export failed")
            return False
            
    except Exception as e:
        print(f"ERROR during conversion: {str(e)}")
        return False

def main():
    """Main function"""
    print("=" * 60)
    print("Football Analytics - Model Conversion Tool")
    print("=" * 60)
    
    # Check dependencies
    if not check_ultralytics():
        sys.exit(1)
    
    # Define model paths (relative to project root)
    project_root = Path(__file__).parent.parent
    
    models_to_convert = [
        {
            'name': 'Player Detection Model',
            'pt_path': project_root / 'models' / 'Yolo8L Players' / 'weights' / 'best.pt',
            'output_name': 'players.onnx'
        },
        {
            'name': 'Keypoint Detection Model',
            'pt_path': project_root / 'models' / 'Yolo8M Field Keypoints' / 'weights' / 'best.pt',
            'output_name': 'keypoints.onnx'
        }
    ]
    
    # Output directory
    output_dir = Path(__file__).parent / 'models'
    output_dir.mkdir(exist_ok=True)
    
    print(f"\nOutput directory: {output_dir}")
    print(f"Found {len(models_to_convert)} models to convert\n")
    
    # Convert all models
    results = []
    for i, model_info in enumerate(models_to_convert, 1):
        print(f"\n[{i}/{len(models_to_convert)}] Processing: {model_info['name']}")
        print("-" * 60)
        
        success = convert_model(
            str(model_info['pt_path']),
            str(output_dir),
            model_info['output_name']
        )
        
        results.append((model_info['name'], success))
    
    # Display summary
    print("\n" + "=" * 60)
    print("Conversion Results Summary")
    print("=" * 60)
    
    for name, success in results:
        status = "SUCCESS" if success else "FAILED"
        print(f"{status}: {name}")
    
    # Statistics
    success_count = sum(1 for _, success in results if success)
    total_count = len(results)
    
    print("\n" + "=" * 60)
    print(f"Total: {success_count}/{total_count} models converted successfully")
    print("=" * 60)
    
    if success_count == total_count:
        print("\nAll models converted successfully!")
        print("\nNext steps:")
        print("1. Copy tactical_map.jpg to cpp/resources/ directory")
        print("2. Run build.bat to compile C++ project")
        print("3. Run program: football_analytics.exe --video path/to/video.mp4")
        return 0
    else:
        print("\nSome models failed to convert, please check error messages")
        return 1

if __name__ == '__main__':
    sys.exit(main())
