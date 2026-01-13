#!/usr/bin/env python3
"""
Simple API Server for Football Analytics
Receives detection data and prints logs (no database)
"""

from flask import Flask, request, jsonify
from datetime import datetime
import json
import sys

app = Flask(__name__)

# Configuration
DEBUG_JSON = False  # Set to True to print full JSON for each frame
VERBOSE_LOGGING = True  # Set to False to reduce console output

# Simple in-memory storage for current session
current_session = {
    'video_source': None,
    'total_frames': 0,
    'frames_received': 0,
    'start_time': None
}

@app.route('/api/health', methods=['GET'])
def health_check():
    """Health check endpoint"""
    print(f"[{datetime.now().strftime('%H:%M:%S')}] Health check")
    return jsonify({
        'status': 'healthy',
        'timestamp': datetime.now().isoformat(),
        'service': 'Football Analytics API'
    })

@app.route('/api/video/start', methods=['POST'])
def video_start():
    """Video processing start notification"""
    # Get raw data first (before any parsing attempts)
    raw_data = request.data
    
    try:
        # Debug: print request info
        print(f"\n[DEBUG] /api/video/start request received")
        print(f"  Content-Type: {request.content_type}")
        print(f"  Content-Length: {request.content_length}")
        print(f"  Raw data length: {len(raw_data)} bytes")
        
        # Always print raw data for debugging
        if len(raw_data) > 0:
            preview = raw_data[:500].decode('utf-8', errors='replace')
            print(f"  Raw data: {preview}")
        
        # Try to parse JSON with force flag (ignore Content-Type)
        data = request.get_json(force=True, silent=False)
        
        if data is None:
            print(f"[ERROR] No JSON data in /api/video/start request")
            decoded_full = raw_data.decode('utf-8', errors='replace')
            print(f"  Full decoded text:\n{decoded_full}")
            return jsonify({'status': 'error', 'message': 'No JSON data provided'}), 400
        
        video_source = data.get('videoSource', 'unknown')
        total_frames = data.get('totalFrames', 0)
    except Exception as e:
        print(f"[ERROR] Failed to parse JSON in /api/video/start: {e}")
        
        # Print the raw data that failed to parse
        try:
            decoded_full = raw_data.decode('utf-8', errors='replace')
            print(f"  Failed JSON string:\n{decoded_full}")
        except:
            print(f"  Could not decode raw data")
        
        import traceback
        traceback.print_exc()
        return jsonify({'status': 'error', 'message': str(e)}), 400
    
    current_session['video_source'] = video_source
    current_session['total_frames'] = total_frames
    current_session['frames_received'] = 0
    current_session['start_time'] = datetime.now()
    
    print("\n" + "=" * 80)
    print(f"VIDEO PROCESSING STARTED")
    print("=" * 80)
    print(f"Time:        {current_session['start_time'].strftime('%Y-%m-%d %H:%M:%S')}")
    print(f"Video:       {video_source}")
    print(f"Total frames: {total_frames}")
    print("=" * 80 + "\n")
    
    return jsonify({
        'status': 'success',
        'message': f'Video processing started for {video_source}'
    })

@app.route('/api/frames', methods=['POST'])
def receive_frame():
    """Receive single frame data"""
    # Get raw data first (before any parsing attempts)
    raw_data = request.data
    
    try:
        # Try to parse JSON with force flag first
        data = request.get_json(force=True, silent=False)
        
        # Debug output (if enabled)
        if VERBOSE_LOGGING or DEBUG_JSON:
            print(f"\n[DEBUG] /api/frames request received")
            print(f"  Content-Type: {request.content_type}")
            print(f"  Content-Length: {request.content_length}")
            print(f"  Raw data length: {len(raw_data)} bytes")
            
            # Print formatted JSON if debug mode is on
            if DEBUG_JSON and data is not None:
                formatted = json.dumps(data, indent=2, ensure_ascii=False)
                # Only print first 2000 chars for very large data
                if len(formatted) > 2000:
                    print(f"  Parsed JSON (first 2000 chars):\n{formatted[:2000]}\n  ... (truncated, total {len(formatted)} chars)")
                else:
                    print(f"  Parsed JSON (complete):\n{formatted}")
        
        if data is None:
            print(f"[ERROR] No JSON data in /api/frames request")
            decoded_full = raw_data.decode('utf-8', errors='replace')
            print(f"  Full decoded text:\n{decoded_full}")
            return jsonify({'status': 'error', 'message': 'No JSON data provided'}), 400
        
        frame_number = data.get('frameNumber', 0)
        timestamp = data.get('timestamp', 0)
    except Exception as e:
        print(f"[ERROR] Failed to parse JSON in /api/frames: {e}")
        print(f"  Exception type: {type(e).__name__}")
        
        # Print the raw data that failed to parse
        try:
            decoded_full = raw_data.decode('utf-8', errors='replace')
            print(f"  Failed JSON string:\n{decoded_full}")
        except:
            print(f"  Could not decode raw data")
        
        import traceback
        traceback.print_exc()
        return jsonify({'status': 'error', 'message': str(e)}), 400
    
    # Count detections
    num_players = len(data.get('players', []))
    num_keypoints = len(data.get('keypoints', []))
    num_balls = len(data.get('balls', []))
    
    current_session['frames_received'] += 1
    
    # Print progress every 30 frames
    if frame_number % 30 == 0 or frame_number == 1:
        progress = (current_session['frames_received'] / 
                   current_session['total_frames'] * 100 
                   if current_session['total_frames'] > 0 else 0)
        
        print(f"[{datetime.now().strftime('%H:%M:%S')}] "
              f"Frame {frame_number:4d} | "
              f"Progress: {progress:5.1f}% | "
              f"Players: {num_players:2d} | "
              f"Keypoints: {num_keypoints:2d} | "
              f"Balls: {num_balls:1d}")
    
    # Print detailed info every 100 frames (optional)
    if frame_number % 100 == 0:
        print(f"\n--- Frame {frame_number} Details ---")
        
        # Print player team info
        if 'players' in data and data['players']:
            teams = {}
            for player in data['players']:
                team_id = player.get('teamId', -1)
                teams[team_id] = teams.get(team_id, 0) + 1
            
            print(f"  Players by team: {teams}")
            
            # Print first player as example
            if len(data['players']) > 0:
                p = data['players'][0]
                print(f"  Example player: bbox=({p['bbox']['x']}, {p['bbox']['y']}, "
                      f"{p['bbox']['width']}, {p['bbox']['height']}), "
                      f"conf={p['confidence']:.2f}")
        
        # Print keypoint info
        if 'keypoints' in data and data['keypoints']:
            print(f"  Keypoints detected: {len(data['keypoints'])}")
            if len(data['keypoints']) > 0:
                k = data['keypoints'][0]
                print(f"  Example keypoint: {k['label']} at ({k['x']:.1f}, {k['y']:.1f})")
        
        print("---\n")
    
    return jsonify({
        'status': 'success',
        'frame_number': frame_number,
        'received': current_session['frames_received']
    })

@app.route('/api/frames/batch', methods=['POST'])
def receive_batch_frames():
    """Receive batch frame data"""
    try:
        data = request.get_json()
        if data is None:
            print(f"[ERROR] No JSON data in /api/frames/batch request")
            return jsonify({'status': 'error', 'message': 'No JSON data provided'}), 400
        
        frames = data.get('frames', [])
    except Exception as e:
        print(f"[ERROR] Failed to parse JSON in /api/frames/batch: {e}")
        return jsonify({'status': 'error', 'message': str(e)}), 400
    
    current_session['frames_received'] += len(frames)
    
    print(f"[{datetime.now().strftime('%H:%M:%S')}] "
          f"Received batch: {len(frames)} frames")
    
    return jsonify({
        'status': 'success',
        'frames_received': len(frames),
        'total_received': current_session['frames_received']
    })

@app.route('/api/video/complete', methods=['POST'])
def video_complete():
    """Video processing complete notification"""
    try:
        data = request.get_json()
        if data is None:
            print(f"[ERROR] No JSON data in /api/video/complete request")
            return jsonify({'status': 'error', 'message': 'No JSON data provided'}), 400
        
        video_source = data.get('videoSource', 'unknown')
    except Exception as e:
        print(f"[ERROR] Failed to parse JSON in /api/video/complete: {e}")
        return jsonify({'status': 'error', 'message': str(e)}), 400
    
    end_time = datetime.now()
    duration = (end_time - current_session['start_time']).total_seconds() if current_session['start_time'] else 0
    
    print("\n" + "=" * 80)
    print(f"VIDEO PROCESSING COMPLETED")
    print("=" * 80)
    print(f"Video:           {video_source}")
    print(f"Frames received: {current_session['frames_received']}/{current_session['total_frames']}")
    print(f"Duration:        {duration:.1f} seconds")
    if duration > 0:
        print(f"Avg FPS:         {current_session['frames_received']/duration:.2f}")
    print(f"Completed at:    {end_time.strftime('%Y-%m-%d %H:%M:%S')}")
    print("=" * 80 + "\n")
    
    # Reset session
    current_session['video_source'] = None
    current_session['total_frames'] = 0
    current_session['frames_received'] = 0
    current_session['start_time'] = None
    
    return jsonify({
        'status': 'success',
        'frames_received': current_session['frames_received']
    })

@app.route('/api/test', methods=['POST'])
def test_endpoint():
    """Test endpoint - print received JSON"""
    try:
        data = request.get_json()
        
        print("\n" + "-" * 80)
        print(f"[{datetime.now().strftime('%H:%M:%S')}] TEST endpoint called")
        print("-" * 80)
        
        if data is None:
            print("NO JSON DATA")
            print(f"Content-Type: {request.content_type}")
            print(f"Raw data: {request.data}")
        else:
            print(json.dumps(data, indent=2))
        
        print("-" * 80 + "\n")
        
        return jsonify({
            'status': 'success',
            'message': 'Test data received',
            'data_keys': list(data.keys()) if data else []
        })
    except Exception as e:
        print(f"[ERROR] in /api/test: {e}")
        return jsonify({'status': 'error', 'message': str(e)}), 400

def main():
    """Main function"""
    print("\n" + "=" * 80)
    print("  Football Analytics - Simple API Server")
    print("=" * 80)
    
    # Show configuration
    print("\n  Configuration:")
    print(f"    DEBUG_JSON:       {DEBUG_JSON} (Set to True to print full JSON)")
    print(f"    VERBOSE_LOGGING:  {VERBOSE_LOGGING}")
    
    print("\n  API Endpoints:")
    print("    GET  /api/health             - Health check")
    print("    POST /api/video/start        - Start video processing")
    print("    POST /api/frames             - Receive frame data")
    print("    POST /api/frames/batch       - Receive batch frame data")
    print("    POST /api/video/complete     - Complete video processing")
    print("    POST /api/test               - Test endpoint (prints JSON)")
    
    print("\n  Server URL: http://localhost:8080")
    
    if not DEBUG_JSON:
        print("\n  💡 Tip: To see full JSON data, set DEBUG_JSON = True in the file")
    
    print("=" * 80 + "\n")
    
    # Run server
    app.run(host='0.0.0.0', port=8080, debug=False, threaded=True)

if __name__ == '__main__':
    try:
        main()
    except KeyboardInterrupt:
        print("\n\nServer stopped by user")
    except Exception as e:
        print(f"\nError: {e}")
