#!/usr/bin/env python3
"""
Test JSON escaping in various scenarios
"""

import json
import sys

def test_windows_path():
    """Test Windows file path with backslashes"""
    test_cases = [
        r'C:\test\video.mp4',
        r'D:\videos\match_2024\game1.avi',
        r'\\server\share\video.mp4',
    ]
    
    print("=" * 80)
    print("Testing Windows Paths")
    print("=" * 80)
    
    for path in test_cases:
        print(f"\nOriginal path: {path}")
        
        # Correct JSON encoding
        correct_json = json.dumps({"videoSource": path})
        print(f"Correct JSON: {correct_json}")
        
        # What it looks like if not escaped (WRONG)
        wrong_json = f'{{"videoSource":"{path}"}}'
        print(f"Wrong JSON:   {wrong_json}")
        
        # Try to parse both
        try:
            parsed = json.loads(correct_json)
            print(f"✓ Correct JSON parses OK: {parsed}")
        except Exception as e:
            print(f"✗ Correct JSON failed: {e}")
        
        try:
            parsed = json.loads(wrong_json)
            print(f"✓ Wrong JSON parses (unexpected!): {parsed}")
        except Exception as e:
            print(f"✗ Wrong JSON fails (expected): {e}")

def test_special_chars():
    """Test other special characters"""
    test_cases = [
        'normal string',
        'string with "quotes"',
        'string with\nnewlines',
        'string with\ttabs',
        'path/with/forward/slashes',
    ]
    
    print("\n" + "=" * 80)
    print("Testing Special Characters")
    print("=" * 80)
    
    for text in test_cases:
        print(f"\nOriginal: {repr(text)}")
        correct_json = json.dumps({"text": text})
        print(f"Correct JSON: {correct_json}")
        
        try:
            parsed = json.loads(correct_json)
            print(f"✓ Parses OK")
        except Exception as e:
            print(f"✗ Failed: {e}")

def test_actual_request():
    """Test a complete request like the C++ sends"""
    print("\n" + "=" * 80)
    print("Testing Complete Request")
    print("=" * 80)
    
    # Simulate what C++ might send
    video_path = r"D:\videos\test\match.mp4"
    
    # Correct version (what should happen)
    correct_request = {
        "videoSource": video_path,
        "totalFrames": 3000,
        "timestamp": 1673611830000
    }
    
    correct_json = json.dumps(correct_request, indent=2)
    print(f"\nCorrect request JSON:\n{correct_json}")
    
    # Wrong version (what happens without escaping)
    wrong_json = f'{{"videoSource":"{video_path}","totalFrames":3000,"timestamp":1673611830000}}'
    print(f"\nWrong request JSON (unescaped backslashes):\n{wrong_json}")
    
    print("\nTrying to parse correct JSON:")
    try:
        parsed = json.loads(correct_json)
        print(f"✓ Success: {parsed}")
    except Exception as e:
        print(f"✗ Failed: {e}")
    
    print("\nTrying to parse wrong JSON:")
    try:
        parsed = json.loads(wrong_json)
        print(f"✓ Success (unexpected!): {parsed}")
    except Exception as e:
        print(f"✗ Failed (expected): {e}")

if __name__ == '__main__':
    print("\n")
    print("=" * 80)
    print("  JSON Escape Testing Tool")
    print("=" * 80)
    print()
    
    test_windows_path()
    test_special_chars()
    test_actual_request()
    
    print("\n" + "=" * 80)
    print("  Key Points:")
    print("=" * 80)
    print()
    print("  1. Backslashes in JSON strings MUST be escaped as \\\\")
    print("  2. Windows paths like C:\\test\\video.mp4 become C:\\\\test\\\\video.mp4")
    print("  3. Without escaping, JSON parsing will fail")
    print("  4. The C++ code now has escapeJsonString() to handle this")
    print()
    print("=" * 80)
