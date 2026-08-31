<#
.SYNOPSIS
    Produces a self-contained INQNET_GUI folder that runs without Qt installed.

.DESCRIPTION
    Runs windeployqt to pull in the Qt runtime and plugins, copies the vendor
    DLLs windeployqt knows nothing about, and copies the runtime config files
    next to the executable so the app finds them regardless of the working
    directory.

    Read the licensing note in README.md before redistributing the result:
    the vendor DLLs are proprietary and are NOT yours to hand out freely.

.EXAMPLE
    .\scripts\deploy-windows.ps1 -ExePath .\build\windows-release\release\PROGRAM.exe
#>
[CmdletBinding()]
param(
    [Parameter(Mandatory=$true)]
    [string]$ExePath,

    [string]$QtDir = 'C:\Qt\6.11.1\msvc2022_64',
    [string]$TimeTaggerDir = 'C:\Program Files\Swabian Instruments\Time Tagger',

    [ValidateSet('Release','Debug')]
    [string]$Config = 'Release',

    # Where to assemble the bundle. Defaults to dist\INQNET_GUI at the repo root.
    [string]$OutDir
)

$ErrorActionPreference = 'Stop'
$repoRoot = Split-Path -Parent $PSScriptRoot

if (-not (Test-Path $ExePath)) { throw "Executable not found: $ExePath" }
if (-not $OutDir) { $OutDir = Join-Path $repoRoot 'dist\INQNET_GUI' }

$windeployqt = Join-Path $QtDir 'bin\windeployqt.exe'
if (-not (Test-Path $windeployqt)) { throw "windeployqt not found at $windeployqt" }

New-Item -ItemType Directory -Path $OutDir -Force | Out-Null
Copy-Item $ExePath $OutDir -Force

Write-Host "=== Qt runtime (windeployqt) ===" -ForegroundColor Green
$env:PATH = "$QtDir\bin;$env:PATH"
# Exclusions, all things this app doesn't use, worth ~40 MB of bundle:
#   --no-system-d3d-compiler / --no-system-dxc-compiler  DirectX shader
#       compilers (d3dcompiler_47, dxcompiler + dxil), ~15 MB -- pulled in for
#       Qt Quick/RHI, and this is a widgets-only app.
#   --no-opengl-sw          Mesa software GL fallback, ~20 MB.
#   --no-compiler-runtime   Skips the 24.5 MB vc_redist.x64.exe *installer*;
#       the CRT DLLs are copied app-locally below instead, so nothing has to
#       be installed on the target machine.
$deployArgs = @("--$($Config.ToLower())", '--no-translations',
                '--no-system-d3d-compiler', '--no-system-dxc-compiler',
                '--no-opengl-sw', '--no-compiler-runtime',
                (Join-Path $OutDir 'PROGRAM.exe'))
# See the Invoke-Tool note in build-windows.ps1: windeployqt logs to stderr,
# which PowerShell 5.1 would otherwise turn into a terminating error.
$saved = $ErrorActionPreference
$ErrorActionPreference = 'Continue'
try {
    & $windeployqt @deployArgs 2>&1 | Out-Null
    $code = $LASTEXITCODE
}
finally { $ErrorActionPreference = $saved }
if ($code -ne 0) { throw "windeployqt failed with exit code $code" }

Write-Host "=== MSVC runtime (app-local) ===" -ForegroundColor Green
# Copying the CRT beside the .exe is a supported deployment mode and avoids
# making users run the redistributable installer. Without these, a machine
# that has never had Visual Studio on it fails to start the app.
$vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'
$crtCopied = $false
if (Test-Path $vswhere) {
    $vsPath = & $vswhere -latest -products * -property installationPath
    if ($vsPath) {
        $crtDir = Get-ChildItem (Join-Path $vsPath 'VC\Redist\MSVC') -Recurse -Directory `
                    -Filter 'Microsoft.VC*.CRT' -ErrorAction SilentlyContinue |
                  Where-Object { $_.FullName -like '*\x64\*' } |
                  Sort-Object FullName -Descending | Select-Object -First 1
        if ($crtDir) {
            Get-ChildItem $crtDir.FullName -Filter '*.dll' | ForEach-Object {
                Copy-Item $_.FullName $OutDir -Force; Write-Host "  $($_.Name)"
            }
            $crtCopied = $true
        }
    }
}
if (-not $crtCopied) {
    Write-Warning "  Could not locate the MSVC redistributable DLLs. The target machine will need the"
    Write-Warning "  'Microsoft Visual C++ 2015-2022 Redistributable (x64)' installed."
}

Write-Host "=== vendor DLLs ===" -ForegroundColor Green
# quTAG (qutools). libusb0 and FTD3XX are tdcbase's own runtime dependencies:
# omit them and the app fails to start with a missing-DLL dialog.
$quTagDlls = @('tdcbase.dll', 'FTD3XX.dll', 'libusb0.dll')
foreach ($dll in $quTagDlls) {
    $src = Join-Path $repoRoot "lib\DLL_64bit\$dll"
    if (Test-Path $src) { Copy-Item $src $OutDir -Force; Write-Host "  $dll" }
    else { Write-Warning "  missing: $src" }
}

# Time Tagger (Swabian). okFrontPanel is the Opal Kelly FPGA layer it needs.
# TimeTaggerD.dll is the debug build, selected by TimeTagger.h's
# #pragma comment(lib, ...) when _DEBUG is defined.
if ($Config -eq 'Debug') { $ttDlls = @('TimeTaggerD.dll', 'okFrontPanel.dll') }
else                     { $ttDlls = @('TimeTagger.dll',  'okFrontPanel.dll') }
foreach ($dll in $ttDlls) {
    $src = Join-Path $TimeTaggerDir "driver\x64\$dll"
    if (Test-Path $src) { Copy-Item $src $OutDir -Force; Write-Host "  $dll" }
    else { Write-Warning "  missing: $src" }
}

Write-Host "=== runtime config ===" -ForegroundColor Green
# The app opens these by bare relative path (e.g. QFile("databaseInfo.json")),
# which resolves against the working directory. Putting them beside the .exe
# makes it work when launched from its own folder.
Get-ChildItem (Join-Path $repoRoot 'runtime_data') -File |
    Where-Object { $_.Name -notlike '*.example' } |
    ForEach-Object { Copy-Item $_.FullName $OutDir -Force; Write-Host "  $($_.Name)" }

$size = (Get-ChildItem $OutDir -Recurse -File | Measure-Object -Property Length -Sum).Sum / 1MB
$count = (Get-ChildItem $OutDir -Recurse -File).Count
Write-Host ("`nBundle: {0} ({1:N1} MB, {2} files)" -f $OutDir, $size, $count) -ForegroundColor Green
Write-Host "Launch PROGRAM.exe from inside that folder." -ForegroundColor Green
