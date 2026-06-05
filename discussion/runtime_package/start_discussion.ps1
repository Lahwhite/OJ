$ErrorActionPreference = "Stop"

# 运行包默认连接本机 MySQL，外部部署时可在启动前用环境变量覆盖。
$env:OJ_MYSQL_HOST = if ($env:OJ_MYSQL_HOST) { $env:OJ_MYSQL_HOST } else { "127.0.0.1" }
$env:OJ_MYSQL_PORT = if ($env:OJ_MYSQL_PORT) { $env:OJ_MYSQL_PORT } else { "3306" }
$env:OJ_MYSQL_USER = if ($env:OJ_MYSQL_USER) { $env:OJ_MYSQL_USER } else { "User" }
$env:OJ_MYSQL_PASSWORD = if ($env:OJ_MYSQL_PASSWORD) { $env:OJ_MYSQL_PASSWORD } else { "Management" }
$env:OJ_MYSQL_DATABASE = if ($env:OJ_MYSQL_DATABASE) { $env:OJ_MYSQL_DATABASE } else { "myOJ" }
# 讨论区默认端口和服务端 main.cpp 中的兜底端口不同，运行包以这里为准。
$env:OJ_DISCUSSION_PORT = if ($env:OJ_DISCUSSION_PORT) { $env:OJ_DISCUSSION_PORT } else { "8079" }
# Gemini 模型可以通过环境变量切换，未配置 API key 时不会真正调用。
$env:OJ_GEMINI_MODEL = if ($env:OJ_GEMINI_MODEL) { $env:OJ_GEMINI_MODEL } else { "gemini-2.5-flash" }

$discussionPort = 0
# 端口必须先校验，否则 Crow 启动失败时错误信息不够直观。
if (-not [int]::TryParse($env:OJ_DISCUSSION_PORT, [ref]$discussionPort) -or
    $discussionPort -lt 1 -or
    $discussionPort -gt 65535) {
    throw "OJ_DISCUSSION_PORT must be a valid TCP port number from 1 to 65535."
}

$discussionHost = if ($env:OJ_DISCUSSION_HOST) { $env:OJ_DISCUSSION_HOST.Trim() } else { "" }
if (-not $discussionHost) {
    # 访问地址需要显式给出局域网 IPv4，避免把 0.0.0.0 展示给普通用户。
    throw "OJ_DISCUSSION_HOST is required. Set it to this machine's LAN IPv4 address, for example: `$env:OJ_DISCUSSION_HOST='192.168.1.23'"
}
$parsedDiscussionHost = $null
# OJ_DISCUSSION_HOST 只用于打印访问链接，不参与服务绑定，因此必须是可访问的具体地址。
if ($discussionHost -notmatch '^(\d{1,3}\.){3}\d{1,3}$' -or
    -not [System.Net.IPAddress]::TryParse($discussionHost, [ref]$parsedDiscussionHost) -or
    $parsedDiscussionHost.AddressFamily -ne [System.Net.Sockets.AddressFamily]::InterNetwork) {
    throw "OJ_DISCUSSION_HOST must be this machine's LAN IPv4 address, for example 192.168.1.23. Do not include http:// or /discussion."
}
if ([System.Net.IPAddress]::IsLoopback($parsedDiscussionHost) -or $parsedDiscussionHost.Equals([System.Net.IPAddress]::Any)) {
    # 运行包面向局域网访问，127.0.0.1 和 0.0.0.0 都会让其他机器打不开页面。
    throw "OJ_DISCUSSION_HOST must be a LAN-reachable IPv4 address, not '$discussionHost'."
}

function Test-IsAdministrator {
    # 新建防火墙规则需要管理员权限，查询已有规则则不需要。
    $identity = [Security.Principal.WindowsIdentity]::GetCurrent()
    $principal = New-Object Security.Principal.WindowsPrincipal($identity)
    return $principal.IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)
}

function Ensure-FirewallRule {
    param(
        [int]$Port
    )

    # 防火墙规则按端口命名，同一端口重复启动不会重复创建。
    $ruleName = "OJ Discussion $Port"
    $existingRule = Get-NetFirewallRule -DisplayName $ruleName -ErrorAction SilentlyContinue
    if ($existingRule) {
        Write-Host "Firewall rule: '$ruleName' already exists"
        return
    }

    if (-not (Test-IsAdministrator)) {
        # 第一次开放端口时要求管理员运行，之后普通 PowerShell 也可以启动服务。
        throw "Creating firewall rule '$ruleName' requires Administrator PowerShell. Reopen PowerShell as Administrator and run this script once."
    }

    # Crow 监听所有网卡，入站 TCP 端口必须放行，局域网同学才能访问讨论区。
    New-NetFirewallRule `
        -DisplayName $ruleName `
        -Direction Inbound `
        -Protocol TCP `
        -LocalPort $Port `
        -Action Allow | Out-Null
    Write-Host "Firewall rule: created '$ruleName' for inbound TCP port $Port"
}

Ensure-FirewallRule -Port $discussionPort

# DLL 和 discussion_server.exe 都放在运行包目录，把它加到 PATH 方便加载依赖。
$packageDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$env:PATH = "$packageDir;$env:PATH"

# 从运行包目录启动，静态 web/sql 资源的相对路径才会命中。
Set-Location $packageDir
Write-Host "Discussion server: http://${discussionHost}:$discussionPort/discussion"
Write-Host "HTTP bind address: 0.0.0.0 (all network interfaces)"
Write-Host "MySQL: $env:OJ_MYSQL_USER@$env:OJ_MYSQL_HOST`:$env:OJ_MYSQL_PORT/$env:OJ_MYSQL_DATABASE"
if ($env:OJ_GEMINI_API_KEY -or $env:GEMINI_API_KEY) {
    # 配置密钥后 AI 总结按钮才会真正可用。
    Write-Host "Gemini summary: enabled, model=$env:OJ_GEMINI_MODEL"
} else {
    # 未配置时后端 summary 接口返回 503，普通讨论功能不受影响。
    Write-Host "Gemini summary: disabled; set OJ_GEMINI_API_KEY to enable it"
}
.\discussion_server.exe
