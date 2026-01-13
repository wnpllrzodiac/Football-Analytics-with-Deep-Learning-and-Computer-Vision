@echo off
echo ================================================================================
echo   启动 API 服务器 (调试模式 - 显示完整 JSON)
echo ================================================================================
echo.
echo 这会打印每个请求的完整 JSON 数据，用于调试
echo.

cd /d %~dp0

echo 检查依赖...
python -c "import flask" 2>nul
if %errorlevel% neq 0 (
    echo [ERROR] Flask not installed
    echo Installing Flask...
    pip install flask
)

echo.
echo 启动服务器...
echo.
echo 提示: 按 Ctrl+C 停止服务器
echo.

REM 创建临时Python脚本来启用调试模式
python -c "import sys; code = open('simple_api_server.py', encoding='utf-8').read(); code = code.replace('DEBUG_JSON = False', 'DEBUG_JSON = True'); exec(code)"
