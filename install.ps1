param(
    [string]$Prefix = "$env:LOCALAPPDATA"
)

$ErrorActionPreference = "Stop"

# Colors for output
function Print-Error($msg)   { Write-Host "ERROR: $msg" -ForegroundColor Red }
function Print-Success($msg) { Write-Host "✓ $msg" -ForegroundColor Green }
function Print-Info($msg)    { Write-Host "→ $msg" -ForegroundColor Blue }
function Print-Warning($msg) { Write-Host "⚠ $msg" -ForegroundColor Yellow }
function Print-Section($msg) {
    Write-Host ""
    Write-Host "========================================" -ForegroundColor Blue
    Write-Host $msg -ForegroundColor Blue
    Write-Host "========================================" -ForegroundColor Blue
}

# Get script directory (where the archive was extracted)
$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path

# Installation paths
$AppDir = Join-Path $Prefix "variance-deltas"

Print-Section "Variance Deltas Installation Script"

Print-Info "Install prefix: $Prefix"
Print-Info "Application directory: $AppDir"

# Verify build artifacts exist (look in script's own directory)
Print-Section "Verifying build artifacts"
$MissingFiles = @()

foreach ($file in @("vd.exe", "vd-backend.exe", "vd-model-parser.exe", "ranger.exe")) {
    if (-not (Test-Path (Join-Path $ScriptDir $file))) {
        $MissingFiles += $file
    }
}

if (-not (Test-Path (Join-Path $ScriptDir "client"))) {
    $MissingFiles += "client/"
}

if ($MissingFiles.Count -gt 0) {
    Print-Error "Missing files: $($MissingFiles -join ', ')"
    Print-Error "Make sure you are running this script from the extracted archive directory."
    exit 1
}

Print-Success "All build artifacts found"

# Remove old installation if it exists
Print-Section "Removing previous installation (if any)"

if (Test-Path $AppDir) {
    Print-Info "Removing old application directory: $AppDir"
    Remove-Item -Recurse -Force $AppDir
    Print-Success "Removed old application directory"
} else {
    Print-Info "No previous application directory found"
}

# Create installation directory
Print-Section "Creating installation directory"

New-Item -ItemType Directory -Path $AppDir -Force | Out-Null
Print-Success "Created application directory: $AppDir"

# Install application files
Print-Section "Installing application files"

Print-Info "Copying vd.exe..."
Copy-Item (Join-Path $ScriptDir "vd.exe") (Join-Path $AppDir "vd.exe")
Print-Success "Installed vd.exe (main executable)"

Print-Info "Copying vd-backend.exe..."
Copy-Item (Join-Path $ScriptDir "vd-backend.exe") (Join-Path $AppDir "vd-backend.exe")
Print-Success "Installed vd-backend.exe"

Print-Info "Copying vd-model-parser.exe..."
Copy-Item (Join-Path $ScriptDir "vd-model-parser.exe") (Join-Path $AppDir "vd-model-parser.exe")
Print-Success "Installed vd-model-parser.exe"

Print-Info "Copying ranger.exe..."
Copy-Item (Join-Path $ScriptDir "ranger.exe") (Join-Path $AppDir "ranger.exe")
Print-Success "Installed ranger.exe"

Print-Info "Copying client files..."
Copy-Item -Recurse (Join-Path $ScriptDir "client") (Join-Path $AppDir "client")
Print-Success "Installed client files"

# Add to PATH
Print-Section "Updating PATH"

$UserPath = [Environment]::GetEnvironmentVariable("Path", "User")
if ($UserPath -split ";" | Where-Object { $_ -eq $AppDir }) {
    Print-Success "$AppDir is already in your PATH"
} else {
    Print-Info "Adding $AppDir to user PATH..."
    [Environment]::SetEnvironmentVariable("Path", "$UserPath;$AppDir", "User")
    Print-Success "Added $AppDir to user PATH"
    Print-Warning "You may need to restart your terminal for PATH changes to take effect."
}

# Done
Print-Section "Installation complete!"

Write-Host ""
Print-Info "Installation summary:"
Write-Host "  Application files: $AppDir"
Write-Host "  Command:           vd.exe"
Write-Host ""
Print-Info "You can now run (after restarting your terminal if PATH was updated):"
Write-Host "  vd -M <model> -D <data> -S <stan> -N <num>"
Write-Host ""
Print-Info "Or run directly:"
Write-Host "  $AppDir\vd.exe -M <model> -D <data> -S <stan> -N <num>"
Write-Host ""
Print-Success "Installation finished successfully!"
