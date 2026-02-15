#Requires -Version 5.1
Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

# ============================================================================
# Output helpers
# ============================================================================
function Print-Error   { param($msg) Write-Host "ERROR: $msg" -ForegroundColor Red }
function Print-Success { param($msg) Write-Host "v $msg" -ForegroundColor Green }
function Print-Info    { param($msg) Write-Host "> $msg" -ForegroundColor Cyan }
function Print-Warning { param($msg) Write-Host "! $msg" -ForegroundColor Yellow }
function Print-Section {
    param($msg)
    Write-Host ""
    Write-Host "========================================" -ForegroundColor Cyan
    Write-Host $msg -ForegroundColor Cyan
    Write-Host "========================================" -ForegroundColor Cyan
}

# Script root
$ScriptDir = $PSScriptRoot
Set-Location $ScriptDir

Print-Section "Variance Deltas Build Script (Windows)"

# ============================================================================
# Phase 1: Tool Detection
# ============================================================================
Print-Section "Phase 1: Checking for required build tools"

$MissingTools = @()

function Check-Tool {
    param($Tool, $InstallMsg)
    $found = Get-Command $Tool -ErrorAction SilentlyContinue
    if ($found) {
        Print-Success "$Tool found ($($found.Source))"
    } else {
        Print-Error "$Tool not found"
        Print-Warning $InstallMsg
        $script:MissingTools += $Tool
    }
}

Check-Tool "opam"  "Install opam for Windows: https://github.com/ocaml/opam/releases"
Check-Tool "dune"  "Install dune: opam install dune"
Check-Tool "cmake" "Install cmake via Visual Studio Build Tools or https://cmake.org"
Check-Tool "deno"  "Install deno: winget install DenoLand.Deno"

if ($MissingTools.Count -gt 0) {
    Print-Error "Missing required tools: $($MissingTools -join ', ')"
    Print-Error "Please install the missing tools and try again."
    exit 1
}

Print-Success "All required build tools found!"

# Set up opam environment
Print-Info "Setting up opam environment..."
try {
    $opamEnv = opam env --shell=powershell | Out-String
    Invoke-Expression $opamEnv
    Print-Success "opam environment configured"
} catch {
    Print-Warning "Could not configure opam environment automatically: $_"
    Print-Warning "Continuing — opam exec will be used for OCaml builds"
}

# ============================================================================
# Phase 1.5: C++ Library Dependency Check
# ============================================================================
Print-Section "Phase 1.5: Checking for C++ library dependencies"

Print-Info "Dependency resolution is handled by vcpkg and CMake."
if ($env:VCPKG_ROOT) {
    Print-Success "VCPKG_ROOT is set: $env:VCPKG_ROOT"
} else {
    Print-Warning "VCPKG_ROOT is not set. CMake will still attempt to find dependencies."
    Print-Warning "Set VCPKG_ROOT to your vcpkg installation directory for automatic package resolution."
}

Print-Success "C++ dependency check completed"

# ============================================================================
# Phase 2: Dependency Installation
# ============================================================================
Print-Section "Phase 2: Installing package dependencies"

# fg_parser dependencies
Print-Info "Installing fg_parser (OCaml) dependencies..."
Set-Location "$ScriptDir\fg_parser"
opam install . --deps-only -y
if ($LASTEXITCODE -ne 0) { Print-Error "Failed to install fg_parser dependencies"; exit 1 }
Print-Success "fg_parser dependencies installed"

# tree_frontend dependencies
Print-Info "Installing tree_frontend (npm) dependencies via Deno..."
Set-Location "$ScriptDir\tree_frontend"
deno install --node-modules-dir
if ($LASTEXITCODE -ne 0) { Print-Error "Failed to install tree_frontend dependencies"; exit 1 }
Print-Success "tree_frontend dependencies installed"

# ws_server dependencies
Print-Info "Caching ws_server (Deno) dependencies..."
Set-Location "$ScriptDir\ws_server"
deno cache main.ts
if ($LASTEXITCODE -ne 0) {
    Print-Warning "Failed to cache ws_server dependencies (may still work)"
}
Print-Success "ws_server dependencies cached"

Set-Location $ScriptDir

# ============================================================================
# Phase 3: Clean Phase
# ============================================================================
Print-Section "Phase 3: Cleaning previous build artifacts"

Print-Info "Removing top-level build directory..."
Remove-Item -Path "$ScriptDir\build" -Recurse -Force -ErrorAction SilentlyContinue
Print-Success "Removed build\"

Print-Info "Cleaning fg_parser..."
Set-Location "$ScriptDir\fg_parser"
dune clean 2>$null; $true  # ignore errors
Print-Success "Cleaned fg_parser"

Print-Info "Cleaning graph_backend..."
Remove-Item -Path "$ScriptDir\graph_backend\build" -Recurse -Force -ErrorAction SilentlyContinue
Print-Success "Cleaned graph_backend"

Print-Info "Cleaning tree_frontend..."
Remove-Item -Path "$ScriptDir\tree_frontend\build" -Recurse -Force -ErrorAction SilentlyContinue
Print-Success "Cleaned tree_frontend"

Print-Info "Cleaning ws_server..."
Remove-Item -Path "$ScriptDir\ws_server\build" -Recurse -Force -ErrorAction SilentlyContinue
Print-Success "Cleaned ws_server"

Print-Info "Cleaning ranger..."
Remove-Item -Path "$ScriptDir\ranger\cpp_version\build" -Recurse -Force -ErrorAction SilentlyContinue
Print-Success "Cleaned ranger"

Set-Location $ScriptDir
Print-Success "All clean phases completed"

# ============================================================================
# Phase 4: Build Phase
# ============================================================================
Print-Section "Phase 4: Building all components"

# Build fg_parser
Print-Info "Building fg_parser (OCaml)..."
Set-Location "$ScriptDir\fg_parser"
opam exec -- dune build
if ($LASTEXITCODE -ne 0) { Print-Error "Failed to build fg_parser"; exit 1 }
Print-Success "fg_parser built successfully"

# Build graph_backend
Print-Info "Building graph_backend (C++)..."
Set-Location "$ScriptDir\graph_backend"
New-Item -ItemType Directory -Path "build" -Force | Out-Null
Set-Location "build"

$CmakeArgs = @(
    "-G", "Visual Studio 17 2022",
    "-A", "x64",
    "-DCMAKE_BUILD_TYPE=Release",
    "-DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded"
)
if ($env:VCPKG_ROOT) {
    $CmakeArgs += "-DCMAKE_TOOLCHAIN_FILE=$env:VCPKG_ROOT\scripts\buildsystems\vcpkg.cmake"
    $CmakeArgs += "-DVCPKG_TARGET_TRIPLET=x64-windows-static"
}

& cmake @CmakeArgs ..\src
if ($LASTEXITCODE -ne 0) {
    Print-Error "CMake configuration failed for graph_backend"
    Print-Error "Make sure C++ dependencies are installed via vcpkg"
    exit 1
}
Print-Success "CMake configuration completed (Release mode)"

cmake --build . --config Release
if ($LASTEXITCODE -ne 0) { Print-Error "Failed to build graph_backend"; exit 1 }
Print-Success "graph_backend built successfully"

# Build ranger
Print-Info "Building ranger (C++)..."
Set-Location "$ScriptDir\ranger\cpp_version"
New-Item -ItemType Directory -Path "build" -Force | Out-Null
Set-Location "build"

cmake -G "Visual Studio 17 2022" -A x64 `
    -DCMAKE_BUILD_TYPE=Release `
    -DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded `
    ..
if ($LASTEXITCODE -ne 0) { Print-Error "ranger CMake configuration failed"; exit 1 }
Print-Success "ranger CMake configuration completed"

cmake --build . --config Release
if ($LASTEXITCODE -ne 0) { Print-Error "Failed to build ranger"; exit 1 }
Print-Success "ranger built successfully"

# Build tree_frontend
Print-Info "Building tree_frontend (Svelte/TypeScript)..."
Set-Location "$ScriptDir\tree_frontend"
deno run -A npm:vite build
if ($LASTEXITCODE -ne 0) { Print-Error "Failed to build tree_frontend"; exit 1 }
Print-Success "tree_frontend built successfully"

# Build ws_server
Print-Info "Building ws_server (Deno)..."
Set-Location "$ScriptDir\ws_server"
New-Item -ItemType Directory -Path "build" -Force | Out-Null
deno compile --allow-net --allow-read --allow-run --output build\ws_server.exe main.ts
if ($LASTEXITCODE -ne 0) { Print-Error "Failed to build ws_server"; exit 1 }
Print-Success "ws_server built successfully"

Set-Location $ScriptDir
Print-Success "All components built successfully"

# ============================================================================
# Phase 5: Assembly Phase
# ============================================================================
Print-Section "Phase 5: Assembling build directory"

Print-Info "Creating build directory structure..."
New-Item -ItemType Directory -Path "$ScriptDir\build\client" -Force | Out-Null

Print-Info "Copying model_parser executable..."
Copy-Item "$ScriptDir\fg_parser\_build\default\bin\model_parser.exe" `
          "$ScriptDir\build\model_parser.exe" -ErrorAction Stop
Print-Success "Copied model_parser.exe"

Print-Info "Copying graph_backend executable..."
Copy-Item "$ScriptDir\graph_backend\build\Release\graph_test.exe" `
          "$ScriptDir\build\graph_test.exe" -ErrorAction Stop
Print-Success "Copied graph_test.exe"

Print-Info "Copying ranger executable..."
Copy-Item "$ScriptDir\ranger\cpp_version\build\Release\ranger.exe" `
          "$ScriptDir\build\ranger.exe" -ErrorAction Stop
Print-Success "Copied ranger.exe"

Print-Info "Copying ws_server executable..."
Copy-Item "$ScriptDir\ws_server\build\ws_server.exe" `
          "$ScriptDir\build\ws_server.exe" -ErrorAction Stop
Print-Success "Copied ws_server.exe"

Print-Info "Copying tree_frontend static files..."
Copy-Item "$ScriptDir\tree_frontend\build\*" "$ScriptDir\build\client\" -Recurse -ErrorAction Stop
Print-Success "Copied frontend files"

Print-Success "Build directory assembled"

# ============================================================================
# Phase 6: Verification Phase
# ============================================================================
Print-Section "Phase 6: Verifying build outputs"

$VerificationFailed = $false

function Verify-File {
    param($File, $Description)
    if (Test-Path $File) {
        Print-Success "$Description exists"
    } else {
        Print-Error "$Description missing"
        $script:VerificationFailed = $true
    }
}

Verify-File "$ScriptDir\build\ws_server.exe"             "ws_server.exe"
Verify-File "$ScriptDir\build\graph_test.exe"            "graph_test.exe"
Verify-File "$ScriptDir\build\model_parser.exe"          "model_parser.exe"
Verify-File "$ScriptDir\build\ranger.exe"                "ranger.exe"
Verify-File "$ScriptDir\build\client\index.html"         "Frontend index.html"
Verify-File "$ScriptDir\build\client\_app\version.json"  "Frontend app bundle"

if ($VerificationFailed) {
    Print-Error "Build verification failed"
    exit 1
}

Print-Success "All build outputs verified!"

# ============================================================================
# Build Complete
# ============================================================================
Print-Section "Build completed successfully!"

Write-Host ""
Print-Info "Build artifacts location: $ScriptDir\build\"
Write-Host ""
Print-Info "To run the application:"
Write-Host "  cd build"
Write-Host "  .\ws_server.exe -M <model file> -D <data file> -S <stan file prefix> -N <number of chains> [--port <port>]"
Write-Host ""
Print-Info "Example:"
Write-Host "  cd build"
Write-Host "  .\ws_server.exe -M ..\data\model_spec -D ..\data\data.json -S ..\data\posterior_samples -N 4"
Write-Host ""
Print-Info "The server defaults to port 8765. Specify --port to use a different port."
Write-Host ""
Print-Success "Build script finished!"
