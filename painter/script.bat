@echo off
:: 检查是否以管理员权限运行
net session >nul 2>&1
if %errorLevel% NEQ 0 (
    echo 请以管理员权限运行此脚本
    pause
    exit /b
)

:: 以管理员权限运行的命令
echo "调用外部接口用"
pause
