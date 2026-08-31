<#
.SYNOPSIS
    Builds INQNET_GUI on Windows with MSVC + Qt, without Qt Creator.

.DESCRIPTION
    Locates Visual Studio via vswhere, imports the MSVC x64 environment,
    then runs qmake and jom/nmake. Pass -Deploy to also produce a
    self-contained folder that runs on a machine with no Qt installed.

.EXAMPLE
    .\scripts\build-windows.ps1
    .\scripts\build-windows.ps1 -Config Debug
    .\scripts\build-windows.ps1 -Deploy
#>
[CmdletBinding()]
param(
    [ValidateSet('Release','Debug')]
    [string]$Config = 'Release',

    # Qt kit directory (the one containing bin\qmake.exe). Must be an MSVC
    # kit -- see the compiler note in README.md; a MinGW kit cannot link the
    # vendor SDKs.
    [string]$QtDir = 'C:\Qt\6.11.1\msvc2022_64',

    # Swabian Instruments Time Tagger SDK root.
    [string]$TimeTaggerDir = 'C:\Program Files\Swabian Instruments\Time Tagger',

    [string]$BuildDir,
    [switch]$Deploy
)

$ErrorActionPreference = 'Stop'
$repoRoot = Split-Path -Parent $PSScriptRoot
$proFile  = Join-Path $repoRoot 'source\INQNET_GUI.pro'

# Windows PowerShell 5.1 wraps a native command's stderr in an ErrorRecord,
# which under $ErrorActionPreference='Stop' aborts the script even when the
# tool exited 0 (jom and qmake both write progress to stderr). Run native
# tools with the preference relaxed and gate on the exit code instead.
function Invoke-Tool {
    param([string]$Exe, [string[]]$ToolArgs, [string]$What)
    $saved = $ErrorActionPreference
    $ErrorActionPreference = 'Continue'
    try {
        & $Exe @ToolArgs 2>&1 | ForEach-Object { Write-Host $_ }
        $code = $LASTEXITCODE
    }
    finally { $ErrorActionPreference = $saved }
    if ($code -ne 0) { throw "$What failed with exit code $code" }
}

if (-not $BuildDir) { $BuildDir = Join-Path $repoRoot "build\windows-$($Config.ToLower())" }

# ---- Sanity checks, with actionable messages instead of a confusing failure
# 200 lines into a build ----
if (-not (Test-Path (Join-Path $QtDir 'bin\qmake.exe'))) {
    throw "No qmake at $QtDir\bin\qmake.exe. Install the Qt MSVC kit (see README.md) or pass -QtDir."
}
if (-not (Test-Path (Join-Path $QtDir 'mkspecs\modules\qt_lib_serialport.pri'))) {
    throw "Qt SerialPort module missing from $QtDir. Install it with the Qt Maintenance Tool (see README.md)."
}
if (-not (Test-Path (Join-Path $TimeTaggerDir 'driver\include\TimeTagger.h'))) {
    throw "Time Tagger SDK not found at $TimeTaggerDir. Install it or pass -TimeTaggerDir."
}
if (-not (Test-Path (Join-Path $repoRoot 'lib\DLL_64bit\tdcbase.lib'))) {
    throw "quTAG SDK import library missing: lib\DLL_64bit\tdcbase.lib (see README.md)."
}

# runtime_data\databaseInfo.json is gitignored but the .pro's copydata step
# requires it, so a fresh clone would fail the build here.
$dbInfo = Join-Path $repoRoot 'runtime_data\databaseInfo.json'
if (-not (Test-Path $dbInfo)) {
    Copy-Item (Join-Path $repoRoot 'runtime_data\databaseInfo.json.example') $dbInfo
    Write-Host "Created runtime_data\databaseInfo.json from the template -- fill in real MySQL credentials." -ForegroundColor Yellow
}

# ---- Import the MSVC x64 environment ----
$vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'
if (-not (Test-Path $vswhere)) { throw "vswhere.exe not found -- is Visual Studio installed?" }

$vsPath = & $vswhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
if (-not $vsPath) { throw "No Visual Studio with the C++ toolset found. Install the 'Desktop development with C++' workload." }

$vcvars = Join-Path $vsPath 'VC\Auxiliary\Build\vcvars64.bat'
if (-not (Test-Path $vcvars)) { throw "vcvars64.bat not found at $vcvars" }

Write-Host "Using Visual Studio: $vsPath" -ForegroundColor Cyan
Write-Host "Using Qt:            $QtDir" -ForegroundColor Cyan

# vcvars64.bat only sets variables inside its own cmd session, so run it and
# import the resulting environment into this process.
& cmd /c "`"$vcvars`" >nul 2>&1 && set" | ForEach-Object {
    if ($_ -match '^([^=]+)=(.*)$') {
        Set-Item -Path "Env:\$($Matches[1])" -Value $Matches[2] -ErrorAction SilentlyContinue
    }
}

$jom = Join-Path $QtDir '..\..\Tools\QtCreator\bin\jom\jom.exe'
$env:PATH = "$QtDir\bin;$env:PATH"

New-Item -ItemType Directory -Path $BuildDir -Force | Out-Null
Push-Location $BuildDir
try {
    Write-Host "`n=== qmake ===" -ForegroundColor Green
    Invoke-Tool 'qmake' @($proFile, '-spec', 'win32-msvc', "CONFIG+=$($Config.ToLower())",
                          "TIMETAGGER_DIR=$($TimeTaggerDir -replace '\\','/')") 'qmake'

    Write-Host "`n=== compiling ===" -ForegroundColor Green
    $makefile = "Makefile.$Config"
    if (Test-Path $jom) {
        Invoke-Tool $jom @('-f', $makefile, '-j', $env:NUMBER_OF_PROCESSORS) 'Build'
    } else {
        Write-Host "jom not found, falling back to nmake (single-threaded)" -ForegroundColor Yellow
        Invoke-Tool 'nmake' @('-f', $makefile) 'Build'
    }

    $exe = Join-Path $BuildDir "$($Config.ToLower())\PROGRAM.exe"
    Write-Host "`nBuilt: $exe" -ForegroundColor Green
}
finally { Pop-Location }

if ($Deploy) {
    & (Join-Path $PSScriptRoot 'deploy-windows.ps1') `
        -ExePath (Join-Path $BuildDir "$($Config.ToLower())\PROGRAM.exe") `
        -QtDir $QtDir -TimeTaggerDir $TimeTaggerDir -Config $Config
}
