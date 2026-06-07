$ErrorActionPreference = "Stop"

$env:OJ_RANK_PORT = if ($env:OJ_RANK_PORT) { $env:OJ_RANK_PORT } else { "8092" }

$packageDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$env:PATH = "$packageDir;$env:PATH"

Set-Location $packageDir

if (-not (Test-Path ".\leaderboard_server.exe")) {
    Write-Error "leaderboard_server.exe not found. Run sync_runtime.ps1 or copy the binary into runtime_package."
}

# Optional MySQL (omit to use in-memory demo)
if (-not $env:OJ_MYSQL_HOST) { $env:OJ_MYSQL_HOST = "127.0.0.1" }
if (-not $env:OJ_MYSQL_DATABASE) { $env:OJ_MYSQL_DATABASE = "myOJ" }

Write-Host "Rank server: http://127.0.0.1:$env:OJ_RANK_PORT/rank"
Write-Host "Health:      http://127.0.0.1:$env:OJ_RANK_PORT/health"
Write-Host "MySQL:       $env:OJ_MYSQL_USER@$env:OJ_MYSQL_HOST`:$env:OJ_MYSQL_PORT/$env:OJ_MYSQL_DATABASE"
Write-Host "Starting leaderboard_server.exe (keep this window open)..."
Write-Host ""

& ".\leaderboard_server.exe"
$code = $LASTEXITCODE
if ($code -ne 0) {
    Write-Host ""
    Write-Error @"
leaderboard_server exited with code $code.
Common causes:
  1) Missing DLL in runtime_package (need libcurl/libssl/libcrypto etc.)
  2) Port $env:OJ_RANK_PORT already in use
Fix: re-run sync_runtime.ps1, or install MSYS2 and add C:\msys64\ucrt64\bin to PATH.
"@
}
