#requires -Version 5.1
<#
.SYNOPSIS
    Runs the FarmGame UE automation tests headlessly and summarises the results.

.DESCRIPTION
    Locates UnrealEditor-Cmd.exe for the engine version referenced by the
    project, runs "Automation RunTests <filter>" with -TestExit, parses the
    "Test Completed" lines from the editor log and prints success/failed
    counts. Exits with code 1 when the editor fails, no test ran, or any test
    failed.

.PARAMETER ProjectPath
    Path to FarmGameUE.uproject, or to the folder containing it. Defaults to
    the project that owns this Scripts folder.

.PARAMETER EngineRoot
    Engine root folder, for example C:\Program Files\Epic Games\UE_5.8.
    When omitted the script tries UE_ENGINE_ROOT, the Epic registry key,
    LauncherInstalled.dat and the default Epic Games install folder.

.PARAMETER TestFilter
    Automation test filter passed to RunTests. Defaults to FarmGame.
    Use FarmGame.Extra to run only the supplementary tests.

.PARAMETER LogPath
    Absolute log file written by the editor. Defaults to
    <project>\Saved\Logs\FarmGameAutomation.log.

.EXAMPLE
    .\Scripts\run-tests.ps1

.EXAMPLE
    .\Scripts\run-tests.ps1 -TestFilter FarmGame.Extra

.EXAMPLE
    .\Scripts\run-tests.ps1 -EngineRoot 'D:\Epic\UE_5.8'
#>
[CmdletBinding()]
param(
    [string]$ProjectPath = '',
    [string]$EngineRoot = '',
    [string]$TestFilter = 'FarmGame',
    [string]$LogPath = ''
)

$ErrorActionPreference = 'Stop'

$ScriptRoot = $PSScriptRoot
if ([string]::IsNullOrEmpty($ScriptRoot)) {
    $ScriptRoot = Split-Path -Parent $MyInvocation.MyCommand.Path
}
$ProjectRoot = Split-Path -Parent $ScriptRoot

if ([string]::IsNullOrWhiteSpace($ProjectPath)) {
    $ProjectPath = Join-Path $ProjectRoot 'FarmGameUE.uproject'
}
if (-not [System.IO.Path]::IsPathRooted($ProjectPath)) {
    $ProjectPath = Join-Path (Get-Location).Path $ProjectPath
}
if (Test-Path -LiteralPath $ProjectPath -PathType Container) {
    $ProjectPath = Join-Path $ProjectPath 'FarmGameUE.uproject'
}
$ProjectPath = [System.IO.Path]::GetFullPath($ProjectPath)
if (-not (Test-Path -LiteralPath $ProjectPath -PathType Leaf)) {
    Write-Host "ERROR: project file not found: $ProjectPath" -ForegroundColor Red
    exit 1
}
$ProjectRoot = Split-Path -Parent $ProjectPath

# The .uproject records the engine version (for this project "5.8").
$EngineVersion = '5.8'
try {
    $ProjectJson = Get-Content -LiteralPath $ProjectPath -Raw | ConvertFrom-Json
    if ($ProjectJson.EngineAssociation -match '^\d+\.\d+$') {
        $EngineVersion = $ProjectJson.EngineAssociation
    }
}
catch {
    Write-Host "WARNING: could not read EngineAssociation, assuming $EngineVersion" -ForegroundColor Yellow
}

$EditorCmd = $null

if (-not [string]::IsNullOrWhiteSpace($EngineRoot)) {
    $ExplicitExe = Join-Path $EngineRoot 'Engine\Binaries\Win64\UnrealEditor-Cmd.exe'
    if (-not (Test-Path -LiteralPath $ExplicitExe -PathType Leaf)) {
        Write-Host "ERROR: UnrealEditor-Cmd.exe not found under -EngineRoot: $EngineRoot" -ForegroundColor Red
        exit 1
    }
    $EditorCmd = $ExplicitExe
}
else {
    $Candidates = New-Object System.Collections.Generic.List[string]
    if (-not [string]::IsNullOrWhiteSpace($env:UE_ENGINE_ROOT)) {
        $Candidates.Add($env:UE_ENGINE_ROOT)
    }

    foreach ($Hive in @('HKLM:', 'HKCU:')) {
        $RegKey = "$Hive\SOFTWARE\EpicGames\Unreal Engine\$EngineVersion"
        $Reg = Get-ItemProperty -Path $RegKey -ErrorAction SilentlyContinue
        if ($Reg -and $Reg.InstalledDirectory) {
            $Candidates.Add([string]$Reg.InstalledDirectory)
        }
    }

    if ($env:ProgramData) {
        $LauncherFile = Join-Path $env:ProgramData 'Epic\UnrealEngineLauncher\LauncherInstalled.dat'
        if (Test-Path -LiteralPath $LauncherFile -PathType Leaf) {
            try {
                $Launcher = Get-Content -LiteralPath $LauncherFile -Raw | ConvertFrom-Json
                foreach ($Install in @($Launcher.InstallationList)) {
                    if ($Install.AppName -eq "UE_$EngineVersion" -and $Install.InstallLocation) {
                        $Candidates.Add([string]$Install.InstallLocation)
                    }
                }
            }
            catch {
                Write-Host "WARNING: could not parse $LauncherFile" -ForegroundColor Yellow
            }
        }
    }

    if (-not [string]::IsNullOrWhiteSpace($env:ProgramFiles)) {
        $Candidates.Add((Join-Path $env:ProgramFiles "Epic Games\UE_$EngineVersion"))
    }

    $Checked = New-Object System.Collections.Generic.List[string]
    foreach ($Root in $Candidates) {
        if ([string]::IsNullOrWhiteSpace($Root)) {
            continue
        }
        $Exe = Join-Path $Root 'Engine\Binaries\Win64\UnrealEditor-Cmd.exe'
        if ($Checked.Contains($Exe)) {
            continue
        }
        $Checked.Add($Exe)
        if (Test-Path -LiteralPath $Exe -PathType Leaf) {
            $EditorCmd = $Exe
            break
        }
    }

    if (-not $EditorCmd) {
        Write-Host "ERROR: UnrealEditor-Cmd.exe not found for UE $EngineVersion." -ForegroundColor Red
        foreach ($Exe in $Checked) {
            Write-Host "  tried: $Exe"
        }
        Write-Host "Pass -EngineRoot <engine folder>, for example:"
        Write-Host "  .\Scripts\run-tests.ps1 -EngineRoot 'C:\Program Files\Epic Games\UE_5.8'"
        exit 1
    }
}

if ([string]::IsNullOrWhiteSpace($LogPath)) {
    $LogPath = Join-Path $ProjectRoot 'Saved\Logs\FarmGameAutomation.log'
}
elseif (-not [System.IO.Path]::IsPathRooted($LogPath)) {
    $LogPath = Join-Path (Get-Location).Path $LogPath
}
$LogPath = [System.IO.Path]::GetFullPath($LogPath)
$LogDir = Split-Path -Parent $LogPath
if (-not (Test-Path -LiteralPath $LogDir)) {
    New-Item -ItemType Directory -Path $LogDir -Force | Out-Null
}
if (Test-Path -LiteralPath $LogPath) {
    Remove-Item -LiteralPath $LogPath -Force
}

$TestExit = 'Automation Test Queue Empty'
$EditorArgs = @(
    $ProjectPath,
    "-ExecCmds=Automation RunTests $TestFilter",
    "-TestExit=$TestExit",
    '-unattended',
    '-nopause',
    '-nosplash',
    '-nullrhi',
    '-nosound',
    '-stdout',
    '-Culture=en',
    '-log',
    "-abslog=$LogPath"
)

Write-Host "FarmGame automation tests"
Write-Host "  Engine : $EditorCmd"
Write-Host "  Project: $ProjectPath"
Write-Host "  Filter : $TestFilter"
Write-Host "  Log    : $LogPath"
Write-Host ""

& $EditorCmd @EditorArgs
$EditorExitCode = 0
if ($null -ne $LASTEXITCODE) {
    $EditorExitCode = [int]$LASTEXITCODE
}

if (-not (Test-Path -LiteralPath $LogPath -PathType Leaf)) {
    Write-Host "ERROR: editor log was not written: $LogPath" -ForegroundColor Red
    exit 1
}

# Each finished test logs: Test Completed. Result={Success} Name={FarmGame....} Path={...}
$Results = @{}
$Order = New-Object System.Collections.Generic.List[string]
foreach ($Line in [System.IO.File]::ReadLines($LogPath)) {
    if ($Line -notmatch 'Test Completed\.') {
        continue
    }
    $ResultMatch = [regex]::Match($Line, 'Result=\{(?<Value>[^}]+)\}')
    $NameMatch = [regex]::Match($Line, 'Name=\{(?<Value>[^}]+)\}')
    if (-not $ResultMatch.Success -or -not $NameMatch.Success) {
        continue
    }
    $Name = $NameMatch.Groups['Value'].Value
    if (-not $Results.ContainsKey($Name)) {
        $Order.Add($Name)
    }
    $Results[$Name] = $ResultMatch.Groups['Value'].Value
}

$Success = 0
$Failed = 0
$Skipped = 0
$FailedTests = New-Object System.Collections.Generic.List[string]
foreach ($Name in $Order) {
    $Result = [string]$Results[$Name]
    if ($Result -match '^(Success|Passed)$') {
        $Success++
    }
    elseif ($Result -match '^Skipped$') {
        $Skipped++
    }
    else {
        $Failed++
        $FailedTests.Add("$Name [$Result]")
    }
}

Write-Host ""
Write-Host "=============================================="
Write-Host " FarmGame automation summary"
Write-Host "=============================================="
Write-Host (" Test Completed lines : {0}" -f $Order.Count)
Write-Host (" Success              : {0}" -f $Success)
Write-Host (" Failed               : {0}" -f $Failed)
if ($Skipped -gt 0) {
    Write-Host (" Skipped              : {0}" -f $Skipped)
}
Write-Host (" Editor exit code     : {0}" -f $EditorExitCode)
if ($FailedTests.Count -gt 0) {
    Write-Host ""
    Write-Host "Failed tests:" -ForegroundColor Red
    foreach ($TestName in $FailedTests) {
        Write-Host "  $TestName" -ForegroundColor Red
    }
}
Write-Host ""

if ($Order.Count -eq 0) {
    Write-Host "RESULT: FAIL - no 'Test Completed' lines found in the log." -ForegroundColor Red
    Write-Host "Check the log for compile or load errors: $LogPath"
    exit 1
}
if ($Failed -gt 0) {
    Write-Host "RESULT: FAIL" -ForegroundColor Red
    exit 1
}
if ($Success -eq 0) {
    Write-Host "RESULT: FAIL - no test reported Success." -ForegroundColor Red
    exit 1
}
if ($EditorExitCode -ne 0) {
    Write-Host "RESULT: FAIL - UnrealEditor-Cmd exited with code $EditorExitCode." -ForegroundColor Red
    exit 1
}

Write-Host "RESULT: PASS" -ForegroundColor Green
exit 0
