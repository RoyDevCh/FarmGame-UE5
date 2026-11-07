#Requires -Version 5.1
<#
.SYNOPSIS
    FarmGameUE 打包脚本：先用 UBT 编译 FarmGameUEEditor，再可选执行 RunUAT BuildCookRun（cook / stage / pak / archive）。

.DESCRIPTION
    本脚本是 .github/workflows/build.yml 的调用入口，也可以在本机命令行直接跑。
    - 只依赖引擎自带的批处理：<EngineRoot>\Engine\Build\BatchFiles\Build.bat 与 RunUAT.bat。
    - 找不到引擎、编译失败、BuildCookRun 失败时，脚本以非零退出码结束，并打印当次日志路径。
    - 所有输出同时写进 <Project>\Saved\Logs\Package\package-<时间戳>.log（Saved 已被 .gitignore 忽略）。

.PARAMETER Configuration
    编译配置，默认 Development。可选 DebugGame / Development。
    本脚本编译的是 Editor target（FarmGameUEEditor），Editor target 不支持 Shipping，所以这里没有 Shipping 选项。

.PARAMETER Platform
    目标平台，默认 Win64。工程只在 Win64 上验证过。

.PARAMETER SkipCook
    只编译，跳过 BuildCookRun（cook / stage / pak / archive）。CI 用这个模式。

.PARAMETER ArchiveDir
    BuildCookRun -archive 的输出目录。默认 <Project>\Saved\Archive（在 .gitignore 里，不会污染仓库）。

.PARAMETER EngineRoot
    UE 根目录（即包含 Engine 子目录的那一层），默认 C:\Program Files\Epic Games\UE_5.8。
    也可以用环境变量 UE_ROOT 指定。查找顺序：-EngineRoot > 环境变量 UE_ROOT > 默认路径 > 注册表（按 .uproject 的 EngineAssociation）。

.EXAMPLE
    powershell -NoProfile -ExecutionPolicy Bypass -File .\Scripts\package.ps1 -SkipCook

.EXAMPLE
    powershell -NoProfile -ExecutionPolicy Bypass -File .\Scripts\package.ps1 -ArchiveDir D:\FarmGameUE_Package

.EXAMPLE
    powershell -NoProfile -ExecutionPolicy Bypass -File .\Scripts\package.ps1 -EngineRoot "D:\UE_5.8" -Configuration Development
#>

[CmdletBinding()]
param(
    [ValidateSet('DebugGame', 'Development')]
    [string]$Configuration = 'Development',

    [ValidateSet('Win64')]
    [string]$Platform = 'Win64',

    [switch]$SkipCook,

    [string]$ArchiveDir,

    [string]$EngineRoot
)

$ErrorActionPreference = 'Continue'
$script:TotalStart = Get-Date
$script:LogFile = $null

# ---------------------------------------------------------------- 路径与日志
$ScriptDir  = $PSScriptRoot
if (-not $ScriptDir) { $ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path }
$ProjectDir  = Split-Path -Parent $ScriptDir
$ProjectFile = Join-Path $ProjectDir 'FarmGameUE.uproject'
$TargetName  = 'FarmGameUEEditor'
$ProjectName = 'FarmGameUE'
$LogDir      = Join-Path $ProjectDir 'Saved\Logs\Package'

try {
    if (-not (Test-Path -LiteralPath $LogDir)) {
        New-Item -ItemType Directory -Force -Path $LogDir | Out-Null
    }
    $stamp = Get-Date -Format 'yyyyMMdd-HHmmss'
    $script:LogFile = Join-Path $LogDir ('package-' + $stamp + '.log')
    Set-Content -LiteralPath $script:LogFile -Encoding UTF8 -Value ('FarmGameUE package log  ' + (Get-Date -Format 'yyyy-MM-dd HH:mm:ss'))
} catch {
    Write-Host ('[WARN] 无法创建日志目录 ' + $LogDir + '：' + $_.Exception.Message)
    $script:LogFile = $null
}

function Write-Log {
    param(
        [string]$Message,
        [string]$Level = 'INFO'
    )
    $line = '[{0}] [{1}] {2}' -f (Get-Date -Format 'HH:mm:ss'), $Level, $Message
    Write-Host $line
    if ($script:LogFile) {
        Add-Content -LiteralPath $script:LogFile -Value $line -Encoding UTF8 -ErrorAction SilentlyContinue
    }
}

function Exit-Failure {
    param(
        [string]$Step,
        [int]$ExitCode = 1
    )
    $elapsed = (Get-Date) - $script:TotalStart
    Write-Log ('[FAIL] ' + $Step) 'ERROR'
    Write-Log ('退出码: ' + $ExitCode) 'ERROR'
    Write-Log ('总耗时: ' + $elapsed.ToString('mm\:ss')) 'ERROR'
    if ($script:LogFile) {
        Write-Log ('日志文件: ' + $script:LogFile) 'ERROR'
    } else {
        Write-Log '日志文件: 未生成（Saved\Logs\Package 不可写）' 'ERROR'
    }
    Write-Log '常见错误（Live Coding 占用 / 缺少 .NET SDK / RunUAT 路径）见 Docs/BUILD.md' 'ERROR'
    exit 1
}

function Exit-Success {
    param([string]$Message)
    $elapsed = (Get-Date) - $script:TotalStart
    Write-Log $Message 'OK'
    Write-Log ('总耗时: ' + $elapsed.ToString('mm\:ss'))
    if ($script:LogFile) {
        Write-Host ('日志文件: ' + $script:LogFile)
    }
    if ($ArchiveDir -and (Test-Path -LiteralPath $ArchiveDir)) {
        Write-Host ('归档目录: ' + $ArchiveDir)
        $exe = Join-Path $ArchiveDir ('Windows\' + $ProjectName + '.exe')
        if (Test-Path -LiteralPath $exe) {
            Write-Host ('可执行文件: ' + $exe)
        }
    }
    exit 0
}

function Invoke-Tool {
    param(
        [string]$FilePath,
        [string[]]$Arguments,
        [string]$StepName
    )

    Write-Log ('执行: "' + $FilePath + '" ' + ($Arguments -join ' '))
    $start = Get-Date

    & $FilePath @Arguments 2>&1 | ForEach-Object {
        $text = [string]$_
        Write-Host $text
        if ($script:LogFile) {
            Add-Content -LiteralPath $script:LogFile -Value $text -Encoding UTF8 -ErrorAction SilentlyContinue
        }
    }

    $exitCode = $LASTEXITCODE
    if ($null -eq $exitCode) { $exitCode = 1 }
    $elapsed = (Get-Date) - $start
    Write-Log ('{0} 结束：退出码 {1}，耗时 {2}' -f $StepName, $exitCode, $elapsed.ToString('mm\:ss'))
    return [pscustomobject]@{ ExitCode = [int]$exitCode; Elapsed = $elapsed }
}

function Get-EngineCandidates {
    param([string]$ExplicitRoot)

    $list = New-Object System.Collections.ArrayList

    if ($ExplicitRoot) {
        [void]$list.Add([pscustomobject]@{ Source = '-EngineRoot 参数'; Path = $ExplicitRoot })
    }
    if ($env:UE_ROOT) {
        [void]$list.Add([pscustomobject]@{ Source = '环境变量 UE_ROOT'; Path = $env:UE_ROOT })
    }
    [void]$list.Add([pscustomobject]@{ Source = '默认安装路径'; Path = 'C:\Program Files\Epic Games\UE_5.8' })

    $association = '5.8'
    if (Test-Path -LiteralPath $ProjectFile) {
        try {
            $json = Get-Content -LiteralPath $ProjectFile -Raw | ConvertFrom-Json
            if ($json.EngineAssociation) { $association = [string]$json.EngineAssociation }
        } catch {
        }
    }
    foreach ($hive in @('HKLM:', 'HKCU:')) {
        $key = $hive + '\SOFTWARE\EpicGames\Unreal Engine\' + $association
        if (Test-Path -LiteralPath $key) {
            $installed = (Get-ItemProperty -LiteralPath $key -ErrorAction SilentlyContinue).InstalledDirectory
            if ($installed) {
                [void]$list.Add([pscustomobject]@{ Source = ('注册表 ' + $key); Path = $installed })
            }
        }
    }
    return $list
}

# ---------------------------------------------------------------- 定位引擎
Write-Log '====== FarmGameUE package ======'

if (-not (Test-Path -LiteralPath $ProjectFile)) {
    Write-Log ('找不到工程文件: ' + $ProjectFile) 'ERROR'
    Exit-Failure '检查工程文件' 2
}

$tried = New-Object System.Collections.ArrayList
$resolved = $null
$resolvedSource = ''

# 候选路径可能在没插的盘上（例如 Q:\UE_5.8）或不可达的 UNC 上：这里只做探测，
# 用字符串拼路径 + try/catch，别让 Join-Path 抛出 PowerShell 报错盖掉后面的说明。
$probeEap = $ErrorActionPreference
$ErrorActionPreference = 'Continue'
foreach ($candidate in (Get-EngineCandidates -ExplicitRoot $EngineRoot)) {
    $path = ''
    if ($candidate.Path) { $path = ([string]$candidate.Path).Trim().TrimEnd('\') }
    if (-not $path) { continue }

    $buildBatProbe = $path + '\Engine\Build\BatchFiles\Build.bat'
    $usable = $false
    try {
        if (Test-Path -LiteralPath $buildBatProbe) { $usable = $true }
    } catch {
        $usable = $false
    }

    if ($usable) {
        [void]$tried.Add($path + '  [可用]  来源: ' + $candidate.Source)
        $resolved = $path
        $resolvedSource = [string]$candidate.Source
        break
    }
    [void]$tried.Add($path + '  [无效：缺少 Engine\Build\BatchFiles\Build.bat]  来源: ' + $candidate.Source)
}
$ErrorActionPreference = $probeEap

if (-not $resolved) {
    Write-Log '找不到可用的 Unreal Engine 5.8 安装。已尝试：' 'ERROR'
    foreach ($t in $tried) { Write-Log ('  - ' + $t) 'ERROR' }
    Write-Log '处理办法：装 UE 5.8 到默认路径，或用 -EngineRoot "C:\Program Files\Epic Games\UE_5.8" 指定，或设置环境变量 UE_ROOT。' 'ERROR'
    Exit-Failure '定位 UE 根目录' 2
}

$EngineRoot  = $resolved
$BuildBat    = Join-Path $EngineRoot 'Engine\Build\BatchFiles\Build.bat'
$RunUatBat   = Join-Path $EngineRoot 'Engine\Build\BatchFiles\RunUAT.bat'
$EditorCmd   = Join-Path $EngineRoot 'Engine\Binaries\Win64\UnrealEditor-Cmd.exe'
$BuildVersion = Join-Path $EngineRoot 'Engine\Build\Build.version'

Write-Log ('工程目录: ' + $ProjectDir)
Write-Log ('工程文件: ' + $ProjectFile)
Write-Log ('引擎根目录: ' + $EngineRoot)
if ($resolvedSource) { Write-Log ('引擎位置来源: ' + $resolvedSource) }

if (Test-Path -LiteralPath $BuildVersion) {
    try {
        $bv = Get-Content -LiteralPath $BuildVersion -Raw | ConvertFrom-Json
        Write-Log ('引擎版本: ' + $bv.MajorVersion + '.' + $bv.MinorVersion + '.' + $bv.PatchVersion + ' (CL ' + $bv.Changelist + ', ' + $bv.BranchName + ')')
    } catch {
        Write-Log ('无法解析 Build.version: ' + $_.Exception.Message) 'WARN'
    }
}

Write-Log ('配置: ' + $Configuration + ' | 平台: ' + $Platform + ' | SkipCook: ' + $SkipCook.IsPresent)
Write-Log ('日志文件: ' + $script:LogFile)

# ---------------------------------------------------------------- 预检
$editorProcs = @(Get-Process -Name 'UnrealEditor' -ErrorAction SilentlyContinue)
if ($editorProcs.Count -gt 0) {
    $pids = ($editorProcs | ForEach-Object { $_.Id }) -join ', '
    Write-Log ('警告: 有 UnrealEditor 正在运行（PID: ' + $pids + '）。编辑器占用 Binaries\Win64\UnrealEditor-FarmGameUE.dll 且开着 Live Coding 时，UBT 会直接报错，建议先关掉编辑器。') 'WARN'
}

$bundledDotnet = Join-Path $EngineRoot 'Engine\Binaries\ThirdParty\DotNet\10.0\win-x64\dotnet.exe'
if (-not (Test-Path -LiteralPath $bundledDotnet)) {
    Write-Log ('警告: 引擎自带 .NET SDK 不在 ' + $bundledDotnet + '，Build.bat 可能报 "Unable to find an install of Dotnet SDK"。') 'WARN'
}

# ---------------------------------------------------------------- 1) 编译 Editor target
$buildArgs = @(
    $TargetName,
    $Platform,
    $Configuration,
    ('-Project=' + $ProjectFile),
    '-WaitMutex'
)
$buildResult = Invoke-Tool -FilePath $BuildBat -Arguments $buildArgs -StepName 'UnrealBuildTool'

if ($buildResult.ExitCode -ne 0) {
    Write-Log '编译失败。先看日志里 UBT 的最后几行；常见原因是 Live Coding 占用、缺少 .NET SDK、引擎根目录不对（见 Docs/BUILD.md）。' 'ERROR'
    Exit-Failure ('编译 ' + $TargetName) $buildResult.ExitCode
}
Write-Log '编译成功。' 'OK'

if ($SkipCook.IsPresent) {
    Exit-Success '已指定 -SkipCook：跳过 BuildCookRun（cook / stage / pak / archive），只做了编译。'
}

# ---------------------------------------------------------------- 2) BuildCookRun
if (-not (Test-Path -LiteralPath $RunUatBat)) {
    Write-Log ('找不到 RunUAT.bat: ' + $RunUatBat) 'ERROR'
    Write-Log '正确路径是 <EngineRoot>\Engine\Build\BatchFiles\RunUAT.bat；不是 Engine\Build\BatchFiles\AutomationTool\RunUAT.bat。' 'ERROR'
    Exit-Failure '检查 RunUAT.bat' 2
}

if (-not $ArchiveDir) {
    $ArchiveDir = Join-Path $ProjectDir 'Saved\Archive'
} elseif (-not [System.IO.Path]::IsPathRooted($ArchiveDir)) {
    $ArchiveDir = Join-Path $ProjectDir $ArchiveDir
}
try {
    $ArchiveDir = [System.IO.Path]::GetFullPath($ArchiveDir)
} catch {
}
Write-Log ('归档目录: ' + $ArchiveDir)

# -build 会编译 Game target（FarmGameUE），和上面第 1 步的 Editor target 不是同一个 target：
# BuildCookRun 要 -stage 的客户端 exe 来自 Game target，所以这里不能加 -nocompile。
$cookArgs = @(
    'BuildCookRun',
    ('-project=' + $ProjectFile),
    '-noP4',
    ('-platform=' + $Platform),
    ('-clientconfig=' + $Configuration),
    '-build',
    '-cook',
    '-stage',
    '-pak',
    '-archive',
    ('-archivedirectory=' + $ArchiveDir),
    '-utf8output',
    '-unattended'
)
$cookResult = Invoke-Tool -FilePath $RunUatBat -Arguments $cookArgs -StepName 'RunUAT BuildCookRun'

if ($cookResult.ExitCode -ne 0) {
    Write-Log 'BuildCookRun 失败。常见原因：Game target 编译错误、Cook 时资产缺失、-archivedirectory 无写权限（见 Docs/BUILD.md）。' 'ERROR'
    Exit-Failure 'RunUAT BuildCookRun' $cookResult.ExitCode
}

Exit-Success '打包完成。'
