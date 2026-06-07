$ErrorActionPreference = "Stop"

$packageDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$rankDir = Resolve-Path (Join-Path $packageDir "..")
$buildDir = Join-Path $rankDir "build-msys2"
$ucrtBin = "C:\msys64\ucrt64\bin"

Write-Host "Rank root: $rankDir"
Write-Host "Package:   $packageDir"

if (-not (Test-Path $buildDir)) {
    Write-Host "Creating build-msys2 ..."
    if (-not (Test-Path $ucrtBin)) {
        Write-Error "MSYS2 UCRT64 not found at $ucrtBin"
    }
    $env:Path = "$ucrtBin;$env:Path"

    C:\msys64\usr\bin\bash.exe -lc "pacman -S --noconfirm --needed mingw-w64-ucrt-x86_64-libmariadbclient 2>&1 | tail -3"

    Push-Location $rankDir
    cmake -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_COMPILER=g++ -DOJ_RANK_ENABLE_MYSQL=ON -S . -B build-msys2
    Pop-Location
}

$env:Path = "$ucrtBin;$env:Path"
Push-Location $rankDir
cmake --build build-msys2 --target leaderboard_server -j
Pop-Location

$exe = Join-Path $buildDir "leaderboard_server.exe"
if (-not (Test-Path $exe)) {
    Write-Error "Build failed: $exe"
}

Copy-Item $exe (Join-Path $packageDir "leaderboard_server.exe") -Force

$webSrc = Join-Path $rankDir "web"
$webDst = Join-Path $packageDir "web"
New-Item -ItemType Directory -Force -Path $webDst | Out-Null
Copy-Item (Join-Path $webSrc "*") $webDst -Force

$sqlDst = Join-Path $packageDir "sql"
New-Item -ItemType Directory -Force -Path $sqlDst | Out-Null
Copy-Item (Join-Path $rankDir "sql\leaderboard_schema.sql") (Join-Path $sqlDst "leaderboard_schema.sql") -Force
$seedSrc = Join-Path $rankDir "sql\seed_demo.sql"
if (Test-Path $seedSrc) {
    Copy-Item $seedSrc (Join-Path $sqlDst "seed_demo.sql") -Force
}

function Copy-DllIfExists($name) {
    $src = Join-Path $ucrtBin $name
    if (Test-Path $src) {
        Copy-Item $src (Join-Path $packageDir $name) -Force
        return $true
    }
    Write-Warning "Skip missing runtime DLL: $src"
    return $false
}

# MinGW runtime + libmariadb and its transitive dependencies
$dlls = @(
    "libgcc_s_seh-1.dll",
    "libstdc++-6.dll",
    "libwinpthread-1.dll",
    "libmariadb.dll",
    "libcurl-4.dll",
    "libssl-3-x64.dll",
    "libcrypto-3-x64.dll",
    "libnghttp2-14.dll",
    "libnghttp3-9.dll",
    "libngtcp2-16.dll",
    "libngtcp2_crypto_ossl-0.dll",
    "libssh2-1.dll",
    "libbrotlicommon.dll",
    "libbrotlidec.dll",
    "libzstd.dll",
    "zlib1.dll",
    "libidn2-0.dll",
    "libunistring-5.dll",
    "libpsl-5.dll",
    "libiconv-2.dll",
    "libintl-8.dll"
)
$copied = 0
foreach ($dll in $dlls) {
    if (Copy-DllIfExists $dll) { $copied++ }
}

Write-Host "Runtime package synced to $packageDir (dlls=$copied)"
