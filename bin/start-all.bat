@echo off
setlocal enabledelayedexpansion
chcp 936 >nul
title OJ 系统一键启动

echo ================================
echo   OJ 系统正在启动，请稍候...
echo ================================

echo.
echo [0/6] 获取本机 WLAN IP...
set LOCAL_IP=127.0.0.1
set FOUND=0
for /f "tokens=*" %%a in ('ipconfig') do (
    set LINE=%%a
    if "!FOUND!"=="1" (
        echo !LINE! | findstr /c:"IPv4" >nul
        if !errorlevel! equ 0 (
            for /f "tokens=2 delims=:" %%x in ("!LINE!") do (
                set RAW=%%x
                set LOCAL_IP=!RAW: =!
            )
            set FOUND=0
        )
    )
    echo !LINE! | findstr /c:"无线局域网适配器 WLAN" >nul
    if !errorlevel! equ 0 set FOUND=1
)
echo 本机 WLAN IP: %LOCAL_IP%

:: 更新评测引擎 judge_cli.json 中的 web_host
echo.
echo [0.5/6] 更新评测引擎配置...
set "CONFIG_FILE=%~dp0..\judge\config\judge_cli.json"
if exist "%CONFIG_FILE%" (
    powershell -NoProfile -Command "(Get-Content '%CONFIG_FILE%') -replace '(\""web_host\"":\s*\"").*?(\"")', ('${1}%LOCAL_IP%${2}') | Set-Content '%CONFIG_FILE%'"
    echo 已将 web_host 更新为 %LOCAL_IP%
) else (
    echo 警告：未找到配置文件 %CONFIG_FILE%，跳过更新。
)

echo.
echo [1/6] 启动 Redis 服务...
start "Redis" /min redis-server.exe
ping 127.0.0.1 -n 2 >nul

echo [2/6] 启动题目与评测服务 (problem-management.jar)...
start "Problem-Management" /min java -jar ..\Problems\problem-management.jar
ping 127.0.0.1 -n 2 >nul

echo [3/6] 启动用户与在线运行服务 (Users.jar)...
start "Users" /min cmd /c "cd /d ..\Users && java -jar Users.jar"
ping 127.0.0.1 -n 2 >nul

echo [4/6] 启动 Ollama 服务...
where ollama >nul 2>&1
if %errorlevel% equ 0 (
    start "Ollama" /min ollama list
    echo Ollama 服务已启动。
) else (
    echo 警告：未找到 Ollama，请确保已安装并手动启动。
)
ping 127.0.0.1 -n 2 >nul

echo [5/6] 启动 OI Wiki 服务...
set "OIWIKI_DIR=%~dp0..\OI_wiki_RAG\OI-wiki"
if exist "%OIWIKI_DIR%" (
    pushd "%OIWIKI_DIR%"
    start "OI-Wiki" /min python -m http.server 8000
    popd
    echo OI Wiki 服务已启动于 http://%LOCAL_IP%:8000
) else (
    echo 警告：未找到 OI Wiki 目录，跳过启动。
)
ping 127.0.0.1 -n 2 >nul

echo [5.3/6] 启动 RAG API（独立 PowerShell 窗口）...
set "RAG_DIR=%~dp0..\OI_wiki_RAG"
if exist "%RAG_DIR%" (
    start "RAG-API" powershell -NoExit -Command "Set-ExecutionPolicy RemoteSigned -Scope CurrentUser -Force; cd '%RAG_DIR%'; if (-not (Test-Path 'venv\Scripts\python.exe')) { python -m venv venv }; .\venv\Scripts\Activate.ps1; pip install -r requirements.txt; python rag_api.py"
    echo RAG API 已在新 PowerShell 窗口启动。
) else (
    echo 警告：未找到 RAG 目录 %RAG_DIR%，跳过启动。
)
ping 127.0.0.1 -n 2 >nul

echo [5.5/6] 启动讨论区服务（独立 PowerShell 窗口）...
set "DISC_DIR=%~dp0..\discussion\runtime_package"
if exist "%DISC_DIR%" (
    start "Discussion" powershell -NoExit -Command "Set-ExecutionPolicy RemoteSigned -Scope CurrentUser -Force; cd '%DISC_DIR%'; $env:OJ_DISCUSSION_HOST='%LOCAL_IP%'; .\start_discussion.ps1"
    echo 讨论区服务已在新窗口启动，访问地址：http://%LOCAL_IP%:8079/discussion
) else (
    echo 警告：未找到讨论区目录 %DISC_DIR%，跳过启动。
)
ping 127.0.0.1 -n 2 >nul

echo [5.8/6] 启动排行榜服务（独立 PowerShell 窗口）...
set "RANK_DIR=%~dp0..\rank\runtime_package"
if exist "%RANK_DIR%" (
    start "Rank" powershell -NoExit -Command "Set-ExecutionPolicy RemoteSigned -Scope CurrentUser -Force; cd '%RANK_DIR%'; $env:OJ_MYSQL_HOST='127.0.0.1'; $env:OJ_MYSQL_PORT='3306'; $env:OJ_MYSQL_USER='User'; $env:OJ_MYSQL_PASSWORD='Management'; $env:OJ_MYSQL_DATABASE='myOJ'; .\start_rank.ps1"
    echo 排行榜服务已在新窗口启动，访问地址：http://%LOCAL_IP%:8092/rank
) else (
    echo 警告：未找到排行榜目录 %RANK_DIR%，跳过启动。
)
ping 127.0.0.1 -n 2 >nul

echo [6/6] 启动 Docker Desktop...
set DOCKER_PATH="C:\Program Files\Docker\Docker\Docker Desktop.exe"
if exist %DOCKER_PATH% (
    start "" /min %DOCKER_PATH%
    echo Docker Desktop 正在启动...
) else (
    echo 警告：未找到 Docker Desktop，请手动启动。
)
ping 127.0.0.1 -n 2 >nul

echo.
echo ================================
echo   启动完成！请先访问以下地址登录或注册：
echo.
echo   首页/用户模块: http://%LOCAL_IP%:8081
echo.
echo   登录后可通过首页导航进入：
echo   题目模块:     http://%LOCAL_IP%:8080/problems
echo   排行榜:       http://%LOCAL_IP%:8092/rank
echo   讨论区:       http://%LOCAL_IP%:8079/discussion
echo   OI Wiki:      http://%LOCAL_IP%:8000
echo ================================
echo.
echo 提示：在线代码运行需等待 Docker Desktop 完全启动。
echo      智能问答需确保 Ollama 已启动并下载了所需模型。

pause