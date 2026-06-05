$ErrorActionPreference = "Stop"

$env:OJ_MYSQL_HOST = if ($env:OJ_MYSQL_HOST) { $env:OJ_MYSQL_HOST } else { "127.0.0.1" }
$env:OJ_MYSQL_PORT = if ($env:OJ_MYSQL_PORT) { $env:OJ_MYSQL_PORT } else { "3306" }
$env:OJ_MYSQL_USER = if ($env:OJ_MYSQL_USER) { $env:OJ_MYSQL_USER } else { "User" }
$env:OJ_MYSQL_PASSWORD = if ($env:OJ_MYSQL_PASSWORD) { $env:OJ_MYSQL_PASSWORD } else { "Management" }
$env:OJ_MYSQL_DATABASE = if ($env:OJ_MYSQL_DATABASE) { $env:OJ_MYSQL_DATABASE } else { "myOJ" }
$env:OJ_DISCUSSION_PORT = if ($env:OJ_DISCUSSION_PORT) { $env:OJ_DISCUSSION_PORT } else { "8079" }
$env:OJ_GEMINI_MODEL = if ($env:OJ_GEMINI_MODEL) { $env:OJ_GEMINI_MODEL } else { "gemini-2.5-flash" }

$discussionPort = 0
if (-not [int]::TryParse($env:OJ_DISCUSSION_PORT, [ref]$discussionPort) -or
    $discussionPort -lt 1 -or
    $discussionPort -gt 65535) {
    throw "OJ_DISCUSSION_PORT must be a valid TCP port number from 1 to 65535."
}

$discussionHost = if ($env:OJ_DISCUSSION_HOST) { $env:OJ_DISCUSSION_HOST.Trim() } else { "" }
if (-not $discussionHost) {
    throw "OJ_DISCUSSION_HOST is required. Set it to this machine's LAN IPv4 address, for example: `$env:OJ_DISCUSSION_HOST='192.168.1.23'"
}
$parsedDiscussionHost = $null
if ($discussionHost -notmatch '^(\d{1,3}\.){3}\d{1,3}$' -or
    -not [System.Net.IPAddress]::TryParse($discussionHost, [ref]$parsedDiscussionHost) -or
    $parsedDiscussionHost.AddressFamily -ne [System.Net.Sockets.AddressFamily]::InterNetwork) {
    throw "OJ_DISCUSSION_HOST must be this machine's LAN IPv4 address, for example 192.168.1.23. Do not include http:// or /discussion."
}
if ([System.Net.IPAddress]::IsLoopback($parsedDiscussionHost) -or $parsedDiscussionHost.Equals([System.Net.IPAddress]::Any)) {
    throw "OJ_DISCUSSION_HOST must be a LAN-reachable IPv4 address, not '$discussionHost'."
}

function Test-IsAdministrator {
    $identity = [Security.Principal.WindowsIdentity]::GetCurrent()
    $principal = New-Object Security.Principal.WindowsPrincipal($identity)
    return $principal.IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)
}

function Ensure-FirewallRule {
    param(
        [int]$Port
    )

    $ruleName = "OJ Discussion $Port"
    $existingRule = Get-NetFirewallRule -DisplayName $ruleName -ErrorAction SilentlyContinue
    if ($existingRule) {
        Write-Host "Firewall rule: '$ruleName' already exists"
        return
    }

    if (-not (Test-IsAdministrator)) {
        throw "Creating firewall rule '$ruleName' requires Administrator PowerShell. Reopen PowerShell as Administrator and run this script once."
    }

    New-NetFirewallRule `
        -DisplayName $ruleName `
        -Direction Inbound `
        -Protocol TCP `
        -LocalPort $Port `
        -Action Allow | Out-Null
    Write-Host "Firewall rule: created '$ruleName' for inbound TCP port $Port"
}

Ensure-FirewallRule -Port $discussionPort

$packageDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$env:PATH = "$packageDir;$env:PATH"

Set-Location $packageDir
Write-Host "Discussion server: http://${discussionHost}:$discussionPort/discussion"
Write-Host "HTTP bind address: 0.0.0.0 (all network interfaces)"
Write-Host "MySQL: $env:OJ_MYSQL_USER@$env:OJ_MYSQL_HOST`:$env:OJ_MYSQL_PORT/$env:OJ_MYSQL_DATABASE"
if ($env:OJ_GEMINI_API_KEY -or $env:GEMINI_API_KEY) {
    Write-Host "Gemini summary: enabled, model=$env:OJ_GEMINI_MODEL"
} else {
    Write-Host "Gemini summary: disabled; set OJ_GEMINI_API_KEY to enable it"
}
.\discussion_server.exe
