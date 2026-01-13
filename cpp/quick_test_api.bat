@echo off
REM Quick test for API server

echo Testing API server...
echo.

REM Test health endpoint
echo [1/3] Testing GET /api/health
curl -s http://localhost:8080/api/health
echo.
echo.

REM Test video/start
echo [2/3] Testing POST /api/video/start
curl -s -X POST http://localhost:8080/api/video/start ^
  -H "Content-Type: application/json" ^
  -d "{\"videoSource\":\"test.mp4\",\"totalFrames\":100}"
echo.
echo.

REM Test frames
echo [3/3] Testing POST /api/frames
curl -s -X POST http://localhost:8080/api/frames ^
  -H "Content-Type: application/json" ^
  -d "{\"frameNumber\":1,\"timestamp\":1234567890,\"players\":[],\"keypoints\":[],\"balls\":[]}"
echo.
echo.

echo Done! Check the API server console for logs.
pause
