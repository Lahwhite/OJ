$ErrorActionPreference = "Stop"

$env:OJ_RANK_PORT = if ($env:OJ_RANK_PORT) { $env:OJ_RANK_PORT } else { "8092" }

$packageDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$env:PATH = "$packageDir;$env:PATH"

Set-Location $packageDir

if (-not (Test-Path ".\leaderboard_server.exe")) {
    Write-Error "leaderboard_server.exe not found. Run sync_runtime.ps1 or copy the binary into runtime_package."
}

Write-Host "Rank server: http://127.0.0.1:$env:OJ_RANK_PORT/rank"
Write-Host "Health:      http://127.0.0.1:$env:OJ_RANK_PORT/health"
.\leaderboard_server.exe
