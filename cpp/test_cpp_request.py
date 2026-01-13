#!/usr/bin/env python3
"""
Test what the C++ program is actually sending
Run this alongside simple_api_server.py to see the raw requests
"""

from flask import Flask, request
import json

app = Flask(__name__)

@app.route('/api/<path:path>', methods=['GET', 'POST', 'PUT', 'DELETE'])
def catch_all(path):
    """Catch all requests and print everything"""
    print("\n" + "=" * 80)
    print(f"REQUEST to /api/{path}")
    print("=" * 80)
    
    print(f"Method: {request.method}")
    print(f"Content-Type: {request.content_type}")
    print(f"Content-Length: {request.content_length}")
    print(f"Headers:")
    for key, value in request.headers:
        print(f"  {key}: {value}")
    
    print(f"\nRaw data ({len(request.data)} bytes):")
    print(request.data)
    
    print(f"\nDecoded data:")
    try:
        decoded = request.data.decode('utf-8')
        print(decoded)
        
        # Try to pretty print if JSON
        try:
            parsed = json.loads(decoded)
            print(f"\nParsed JSON:")
            print(json.dumps(parsed, indent=2))
        except:
            print("(not valid JSON)")
    except Exception as e:
        print(f"Cannot decode: {e}")
    
    print("=" * 80 + "\n")
    
    # Always return success
    return {'status': 'success', 'message': 'Debug endpoint'}, 200

if __name__ == '__main__':
    print("=" * 80)
    print("  Debug API Server - Captures all requests")
    print("=" * 80)
    print("\n  This will print EVERYTHING the C++ program sends")
    print("  Run your C++ program and watch the output here\n")
    print("  Server: http://localhost:8080")
    print("=" * 80 + "\n")
    
    app.run(host='0.0.0.0', port=8080, debug=False)
