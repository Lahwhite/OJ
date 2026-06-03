$ErrorActionPreference = "Stop"

$env:OJ_MYSQL_HOST = if ($env:OJ_MYSQL_HOST) { $env:OJ_MYSQL_HOST } else { "127.0.0.1" }
$env:OJ_MYSQL_PORT = if ($env:OJ_MYSQL_PORT) { $env:OJ_MYSQL_PORT } else { "3306" }
$env:OJ_MYSQL_USER = if ($env:OJ_MYSQL_USER) { $env:OJ_MYSQL_USER } else { "User" }
$env:OJ_MYSQL_PASSWORD = if ($env:OJ_MYSQL_PASSWORD) { $env:OJ_MYSQL_PASSWORD } else { "Management" }
$env:OJ_MYSQL_DATABASE = if ($env:OJ_MYSQL_DATABASE) { $env:OJ_MYSQL_DATABASE } else { "myOJ" }
$env:OJ_DISCUSSION_PORT = if ($env:OJ_DISCUSSION_PORT) { $env:OJ_DISCUSSION_PORT } else { "8079" }
$env:OJ_GEMINI_MODEL = if ($env:OJ_GEMINI_MODEL) { $env:OJ_GEMINI_MODEL } else { "gemini-2.5-flash" }

$packageDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$env:PATH = "$packageDir;$env:PATH"

Set-Location $packageDir
Write-Host "Discussion server: http://127.0.0.1:$env:OJ_DISCUSSION_PORT/discussion"
Write-Host "MySQL: $env:OJ_MYSQL_USER@$env:OJ_MYSQL_HOST`:$env:OJ_MYSQL_PORT/$env:OJ_MYSQL_DATABASE"
if ($env:OJ_GEMINI_API_KEY -or $env:GEMINI_API_KEY) {
    Write-Host "Gemini summary: enabled, model=$env:OJ_GEMINI_MODEL"
} else {
    Write-Host "Gemini summary: disabled; set OJ_GEMINI_API_KEY to enable it"
}
.\discussion_server.exe
