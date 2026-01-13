#!/usr/bin/env python3
"""
Test script for Football Analytics API Server
"""

import requests
import json
import time

BASE_URL = "http://localhost:8080"

def test_health():
    """Test health endpoint"""
    print("\n[1/5] Testing /api/health...")
    try:
        response = requests.get(f"{BASE_URL}/api/health", timeout=5)
        print(f"  Status: {response.status_code}")
        print(f"  Response: {json.dumps(response.json(), indent=2)}")
        return response.status_code == 200
    except Exception as e:
        print(f"  ERROR: {e}")
        return False

def test_video_start():
    """Test video start endpoint"""
    print("\n[2/5] Testing /api/video/start...")
    data = {
        "videoSource": "test_video.mp4",
        "totalFrames": 10,
        "timestamp": int(time.time() * 1000)
    }
    
    try:
        response = requests.post(f"{BASE_URL}/api/video/start", json=data, timeout=5)
        print(f"  Status: {response.status_code}")
        print(f"  Response: {json.dumps(response.json(), indent=2)}")
        return response.status_code == 200
    except Exception as e:
        print(f"  ERROR: {e}")
        return False

def test_frame_data():
    """Test frame data endpoint"""
    print("\n[3/5] Testing /api/frames...")
    
    for i in range(1, 6):
        frame_data = {
            "frameNumber": i,
            "timestamp": int(time.time() * 1000),
            "videoSource": "test_video.mp4",
            "players": [
                {
                    "bbox": {
                        "x": 100 + i * 10,
                        "y": 200,
                        "width": 50,
                        "height": 80
                    },
                    "classId": 0,
                    "confidence": 0.95,
                    "label": "player",
                    "teamId": i % 2,
                    "tacMapPosition": {
                        "x": 450.5,
                        "y": 320.2
                    }
                }
            ],
            "keypoints": [
                {
                    "label": "Center circle",
                    "x": 640.0,
                    "y": 360.0,
                    "confidence": 0.98
                }
            ],
            "balls": [
                {
                    "x": 500.5 + i,
                    "y": 300.2,
                    "confidence": 0.87
                }
            ]
        }
        
        try:
            response = requests.post(f"{BASE_URL}/api/frames", json=frame_data, timeout=5)
            if i == 1:
                print(f"  Frame {i}: Status {response.status_code}")
                print(f"  Response: {json.dumps(response.json(), indent=2)}")
            else:
                print(f"  Frame {i}: Status {response.status_code}")
            
            if response.status_code != 200:
                return False
                
        except Exception as e:
            print(f"  ERROR on frame {i}: {e}")
            return False
        
        time.sleep(0.1)  # Small delay between frames
    
    return True

def test_batch_frames():
    """Test batch frame endpoint"""
    print("\n[4/5] Testing /api/frames/batch...")
    
    frames = []
    for i in range(6, 11):
        frames.append({
            "frameNumber": i,
            "timestamp": int(time.time() * 1000),
            "players": [],
            "keypoints": [],
            "balls": []
        })
    
    data = {"frames": frames}
    
    try:
        response = requests.post(f"{BASE_URL}/api/frames/batch", json=data, timeout=5)
        print(f"  Status: {response.status_code}")
        print(f"  Response: {json.dumps(response.json(), indent=2)}")
        return response.status_code == 200
    except Exception as e:
        print(f"  ERROR: {e}")
        return False

def test_video_complete():
    """Test video complete endpoint"""
    print("\n[5/5] Testing /api/video/complete...")
    data = {
        "videoSource": "test_video.mp4",
        "timestamp": int(time.time() * 1000)
    }
    
    try:
        response = requests.post(f"{BASE_URL}/api/video/complete", json=data, timeout=5)
        print(f"  Status: {response.status_code}")
        print(f"  Response: {json.dumps(response.json(), indent=2)}")
        return response.status_code == 200
    except Exception as e:
        print(f"  ERROR: {e}")
        return False

def main():
    """Main test function"""
    print("=" * 80)
    print("Football Analytics API Server - Test Suite")
    print("=" * 80)
    print(f"Testing server at: {BASE_URL}")
    
    # Check if server is running
    print("\nChecking if server is running...")
    try:
        response = requests.get(f"{BASE_URL}/api/health", timeout=2)
        print("  Server is running!")
    except Exception as e:
        print(f"  ERROR: Cannot connect to server!")
        print(f"  {e}")
        print("\nPlease start the server first:")
        print("  python simple_api_server.py")
        return 1
    
    # Run tests
    results = []
    
    results.append(("Health Check", test_health()))
    results.append(("Video Start", test_video_start()))
    results.append(("Frame Data", test_frame_data()))
    results.append(("Batch Frames", test_batch_frames()))
    results.append(("Video Complete", test_video_complete()))
    
    # Summary
    print("\n" + "=" * 80)
    print("Test Results Summary")
    print("=" * 80)
    
    passed = 0
    failed = 0
    
    for name, result in results:
        status = "PASS" if result else "FAIL"
        color = "✓" if result else "✗"
        print(f"  {color} {name}: {status}")
        
        if result:
            passed += 1
        else:
            failed += 1
    
    print("\n" + "=" * 80)
    print(f"Total: {passed} passed, {failed} failed out of {len(results)} tests")
    print("=" * 80 + "\n")
    
    return 0 if failed == 0 else 1

if __name__ == '__main__':
    try:
        exit_code = main()
        exit(exit_code)
    except KeyboardInterrupt:
        print("\n\nTests interrupted by user")
        exit(1)
    except Exception as e:
        print(f"\nUnexpected error: {e}")
        exit(1)
