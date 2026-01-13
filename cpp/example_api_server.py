#!/usr/bin/env python3
"""
Football Analytics - 示例API服务器
用于接收C++程序发送的检测数据

运行: python example_api_server.py
"""

from flask import Flask, request, jsonify
from datetime import datetime
import json
import os

app = Flask(__name__)

# 存储数据的目录
DATA_DIR = './api_data'
os.makedirs(DATA_DIR, exist_ok=True)

# 当前视频会话
current_session = None

@app.route('/api/health', methods=['GET'])
def health_check():
    """健康检查端点"""
    return jsonify({
        'status': 'healthy',
        'timestamp': datetime.now().isoformat(),
        'service': 'Football Analytics API'
    })

@app.route('/api/video/start', methods=['POST'])
def video_start():
    """视频处理开始通知"""
    global current_session
    
    data = request.json
    video_source = data.get('videoSource', 'unknown')
    total_frames = data.get('totalFrames', 0)
    
    # 创建新会话
    session_id = datetime.now().strftime('%Y%m%d_%H%M%S')
    current_session = {
        'session_id': session_id,
        'video_source': video_source,
        'total_frames': total_frames,
        'start_time': datetime.now().isoformat(),
        'frames_received': 0,
        'frames_data': []
    }
    
    print(f"\n{'='*60}")
    print(f"新视频处理会话开始")
    print(f"{'='*60}")
    print(f"会话ID: {session_id}")
    print(f"视频源: {video_source}")
    print(f"总帧数: {total_frames}")
    print(f"{'='*60}\n")
    
    return jsonify({
        'status': 'success',
        'session_id': session_id,
        'message': f'Video processing started for {video_source}'
    })

@app.route('/api/frames', methods=['POST'])
def receive_frame():
    """接收单帧数据"""
    global current_session
    
    if current_session is None:
        return jsonify({
            'status': 'error',
            'message': 'No active session. Call /api/video/start first.'
        }), 400
    
    data = request.json
    frame_number = data.get('frameNumber', 0)
    
    # 保存帧数据
    current_session['frames_data'].append(data)
    current_session['frames_received'] += 1
    
    # 显示进度
    if frame_number % 30 == 0:
        progress = (current_session['frames_received'] / 
                   current_session['total_frames'] * 100 
                   if current_session['total_frames'] > 0 else 0)
        
        print(f"\r接收进度: {progress:.1f}% "
              f"({current_session['frames_received']}/{current_session['total_frames']}) "
              f"帧 {frame_number}", end='', flush=True)
    
    return jsonify({
        'status': 'success',
        'frame_number': frame_number,
        'received': current_session['frames_received']
    })

@app.route('/api/frames/batch', methods=['POST'])
def receive_batch_frames():
    """接收批量帧数据"""
    global current_session
    
    if current_session is None:
        return jsonify({
            'status': 'error',
            'message': 'No active session. Call /api/video/start first.'
        }), 400
    
    data = request.json
    frames = data.get('frames', [])
    
    # 保存所有帧数据
    current_session['frames_data'].extend(frames)
    current_session['frames_received'] += len(frames)
    
    print(f"接收批量数据: {len(frames)} 帧")
    
    return jsonify({
        'status': 'success',
        'frames_received': len(frames),
        'total_received': current_session['frames_received']
    })

@app.route('/api/video/complete', methods=['POST'])
def video_complete():
    """视频处理完成通知"""
    global current_session
    
    if current_session is None:
        return jsonify({
            'status': 'error',
            'message': 'No active session.'
        }), 400
    
    data = request.json
    video_source = data.get('videoSource', 'unknown')
    
    # 保存会话数据到文件
    current_session['end_time'] = datetime.now().isoformat()
    
    output_file = os.path.join(
        DATA_DIR, 
        f"session_{current_session['session_id']}.json"
    )
    
    with open(output_file, 'w', encoding='utf-8') as f:
        json.dump(current_session, f, indent=2, ensure_ascii=False)
    
    print(f"\n\n{'='*60}")
    print(f"视频处理完成")
    print(f"{'='*60}")
    print(f"会话ID: {current_session['session_id']}")
    print(f"视频源: {video_source}")
    print(f"总接收帧数: {current_session['frames_received']}")
    print(f"数据已保存: {output_file}")
    print(f"{'='*60}\n")
    
    # 统计信息
    stats = calculate_statistics(current_session['frames_data'])
    print("统计信息:")
    print(f"  平均每帧球员数: {stats['avg_players']:.1f}")
    print(f"  平均每帧关键点数: {stats['avg_keypoints']:.1f}")
    print(f"  检测到球的帧数: {stats['frames_with_ball']}")
    print(f"{'='*60}\n")
    
    # 重置会话
    session_data = current_session
    current_session = None
    
    return jsonify({
        'status': 'success',
        'session_id': session_data['session_id'],
        'frames_received': session_data['frames_received'],
        'output_file': output_file,
        'statistics': stats
    })

def calculate_statistics(frames_data):
    """计算统计信息"""
    if not frames_data:
        return {
            'avg_players': 0,
            'avg_keypoints': 0,
            'frames_with_ball': 0
        }
    
    total_players = sum(len(frame.get('players', [])) for frame in frames_data)
    total_keypoints = sum(len(frame.get('keypoints', [])) for frame in frames_data)
    frames_with_ball = sum(1 for frame in frames_data if frame.get('balls', []))
    
    return {
        'avg_players': total_players / len(frames_data),
        'avg_keypoints': total_keypoints / len(frames_data),
        'frames_with_ball': frames_with_ball,
        'total_frames': len(frames_data)
    }

@app.route('/api/sessions', methods=['GET'])
def list_sessions():
    """列出所有会话"""
    sessions = []
    
    for filename in os.listdir(DATA_DIR):
        if filename.startswith('session_') and filename.endswith('.json'):
            filepath = os.path.join(DATA_DIR, filename)
            with open(filepath, 'r', encoding='utf-8') as f:
                session = json.load(f)
                sessions.append({
                    'session_id': session['session_id'],
                    'video_source': session['video_source'],
                    'frames_received': session['frames_received'],
                    'start_time': session['start_time']
                })
    
    return jsonify({
        'status': 'success',
        'count': len(sessions),
        'sessions': sessions
    })

@app.route('/api/session/<session_id>', methods=['GET'])
def get_session(session_id):
    """获取指定会话的详细信息"""
    filepath = os.path.join(DATA_DIR, f'session_{session_id}.json')
    
    if not os.path.exists(filepath):
        return jsonify({
            'status': 'error',
            'message': f'Session {session_id} not found'
        }), 404
    
    with open(filepath, 'r', encoding='utf-8') as f:
        session = json.load(f)
    
    return jsonify({
        'status': 'success',
        'session': session
    })

def main():
    """主函数"""
    print("=" * 60)
    print("Football Analytics - 示例API服务器")
    print("=" * 60)
    print()
    print("API端点:")
    print("  GET  /api/health               - 健康检查")
    print("  POST /api/video/start          - 开始视频处理")
    print("  POST /api/frames               - 接收帧数据")
    print("  POST /api/frames/batch         - 批量接收帧数据")
    print("  POST /api/video/complete       - 完成视频处理")
    print("  GET  /api/sessions             - 列出所有会话")
    print("  GET  /api/session/<id>         - 获取会话详情")
    print()
    print("数据存储目录:", os.path.abspath(DATA_DIR))
    print("=" * 60)
    print()
    print("服务器启动中...")
    print("访问: http://localhost:8080")
    print("=" * 60)
    print()
    
    app.run(host='0.0.0.0', port=8080, debug=False)

if __name__ == '__main__':
    main()
