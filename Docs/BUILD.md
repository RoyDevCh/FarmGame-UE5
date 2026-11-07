# 构建与打包（BUILD）

本文说明 FarmGameUE 在 Windows 上怎么从源码编译、怎么打出可运行的包、CI 怎么跑，以及出错时先看哪里。

| 文件 | 作用 |
| --- | --- |
| `Scripts/package.ps1` | 一键脚本：先编译 `FarmGameUEEditor`，再可选跑 BuildCookRun（cook / stage / pak / archive） |
| `.github/workflows/build.yml` | GitHub Actions：Windows self-hosted runner，编译 → 跑 Automation 测试 → 上传日志 |
| `Docs/BUILD.md` | 本文件 |

工程有两个 target（见 `Source/FarmGameUE.Target.cs`、`Source/FarmGameUEEditor.Target.cs`）：

- `FarmGameUE`：Game target，打包 `-stage` 出来的客户端 exe 来自它。
- `FarmGameUEEditor`：Editor target，编辑器、PIE、自动化测试用它，产物是 `Binaries\Win64\UnrealEditor-FarmGameUE.dll`（`Binaries/` 已被 git 忽略）。

## 1. 环境要求

| 项 | 本机实测 | 说明 |
| --- | --- | --- |
| Unreal Engine | 5.8.4 / CL 58902052 | 装在 `C:\Program Files\Epic Games\UE_5.8` |
| C++ 工具链 | Visual Studio Community 2026 (18.10) | 需要「使用 C++ 的桌面开发」工作负载（MSVC + Windows SDK） |
| .NET SDK | 引擎自带 10.0.203 | `Engine\Binaries\ThirdParty\DotNet\10.0\win-x64`；`Build.bat` / `RunUAT.bat` 会自动把它加到 PATH 并设 `DOTNET_ROOT`，一般不用单独装 |
| PowerShell | Windows 自带 5.1 | 脚本按 5.1 写，不要求 PowerShell 7（没装 `pwsh` 也能跑） |
| 磁盘 | — | 编译产物在 `Intermediate` / `Saved`，cook 还要更多空间 |

`FarmGameUE.uproject` 里 `EngineAssociation` 是 `5.8`，另外启用了 `ModelContextProtocol`、`AllToolsets`、`PythonScriptPlugin` 三个插件——它们在引擎的 `Engine/Plugins/Experimental` 下，不是工程内插件。

## 2. 手动编译（只出编辑器用的 DLL）

在仓库根目录执行：

~~~powershell
& "C:\Program Files\Epic Games\UE_5.8\Engine\Build\BatchFiles\Build.bat" `
    FarmGameUEEditor Win64 Development `
    -Project="$PWD\FarmGameUE.uproject" `
    -WaitMutex
~~~

- 参数顺序固定：`Build.bat <Target> <Platform> <Configuration> [额外参数...]`。
- `Project` 要给绝对路径，否则 UBT 会去当前目录（`Engine\Source`）找工程。
- `WaitMutex` 让多个 UBT 实例排队，本机和 CI 同时跑时不会互相踩。
- 退出码不是 0 就是失败，先看最后 30 行 UBT 输出。

## 3. 手动打包（BuildCookRun）

~~~powershell
& "C:\Program Files\Epic Games\UE_5.8\Engine\Build\BatchFiles\RunUAT.bat" BuildCookRun `
    -project="$PWD\FarmGameUE.uproject" `
    -noP4 -platform=Win64 -clientconfig=Development `
    -build -cook -stage -pak -archive `
    -archivedirectory="$PWD\Saved\Archive" `
    -utf8output -unattended
~~~

参数含义：

| 参数 | 说明 |
| --- | --- |
| `BuildCookRun` | UAT 的构建 / 烹饪 / 打包命令 |
| `-noP4` | 不走 Perforce（本仓库是 git） |
| `-platform=Win64` | 目标平台 |
| `-clientconfig=Development` | 客户端配置，与编译配置一致 |
| `-build` | 编译 Game target `FarmGameUE`（和第 2 步的 Editor target 不是同一个 target） |
| `-cook` | 烹饪内容（`Content` 里目前只有 `/Game/Maps/Farm`） |
| `-stage` | 把二进制和内容拷进 staging 目录 |
| `-pak` | 把内容打进 pak 文件 |
| `-archive` + `-archivedirectory=` | 把 staging 结果归档到指定目录 |
| `-utf8output`、`-unattended` | 日志用 UTF-8；无人值守，不弹交互提示 |

**别把第 2 步的 `Build.bat` 当 `BuildCookRun` 的替代**：`Build.bat FarmGameUEEditor` 只产出编辑器模块，`-stage` 要的客户端 exe 来自 Game target，必须由 `-build`（或去掉 `-nocompile`）来编译。

## 4. 一键脚本 package.ps1

~~~powershell
# 编译 + 打包（默认 Development / Win64，归档到 Saved\Archive）
powershell -NoProfile -ExecutionPolicy Bypass -File .\Scripts\package.ps1

# 只编译，不 cook（CI 用的就是这个）
powershell -NoProfile -ExecutionPolicy Bypass -File .\Scripts\package.ps1 -SkipCook

# 换归档目录 / 换引擎位置
powershell -NoProfile -ExecutionPolicy Bypass -File .\Scripts\package.ps1 -ArchiveDir D:\FarmGameUE_Package
powershell -NoProfile -ExecutionPolicy Bypass -File .\Scripts\package.ps1 -EngineRoot "D:\UE_5.8"
~~~

参数：

| 参数 | 默认值 | 说明 |
| --- | --- | --- |
| `-Configuration` | `Development` | 可选 `DebugGame` / `Development`。脚本编译的是 Editor target，Editor target 不支持 `Shipping`，所以这里没有 Shipping |
| `-Platform` | `Win64` | 本工程只在 Win64 验证过 |
| `-SkipCook` | 关 | 只编译，跳过 BuildCookRun |
| `-ArchiveDir` | `<仓库>\Saved\Archive` | 归档目录，相对路径按仓库根解析；`Saved/` 已被 git 忽略 |
| `-EngineRoot` | `C:\Program Files\Epic Games\UE_5.8` | UE 根目录（含 `Engine` 子目录的那一层） |

定位引擎的顺序：`-EngineRoot` → 环境变量 `UE_ROOT` → 默认安装路径 → 注册表 `HKLM/HKCU:\SOFTWARE\EpicGames\Unreal Engine\<EngineAssociation>`。都不命中时脚本直接失败，并列出试过哪些路径。

脚本做的事：

1. 校验工程文件和引擎目录（要求同时存在 `Build.bat`、`RunUAT.bat`、`UnrealEditor-Cmd.exe`）。
2. 预检：有 `UnrealEditor` 进程在跑就告警（Live Coding / DLL 占用会让编译失败）；引擎自带 .NET SDK 缺失也告警。
3. 跑 `Build.bat FarmGameUEEditor Win64 <配置> -Project=... -WaitMutex`。
4. 没给 `-SkipCook` 时跑 `RunUAT.bat BuildCookRun ...`。
5. 任一步失败：打印退出码、日志路径、指向本文件的提示，并以 `1` 退出。

## 5. 产物与日志

| 内容 | 位置 |
| --- | --- |
| 编辑器模块 | `Binaries\Win64\UnrealEditor-FarmGameUE.dll` 与 `pdb` |
| 打包脚本日志 | `Saved\Logs\Package\package-<yyyyMMdd-HHmmss>.log`（每次运行一个文件） |
| 编辑器运行日志 | `Saved\Logs\FarmGameUE.log` |
| 归档产物 | `-ArchiveDir`（默认 `Saved\Archive`）；目录结构由 UAT 决定，一般是 `<ArchiveDir>\Windows\` 下；脚本发现 `<ArchiveDir>\Windows\FarmGameUE.exe` 会把它打印出来 |
| UAT 自己的日志 | 控制台输出和 `Saved\Logs` 下的文件，内容都会进 package 日志 |

## 6. 自动化测试

测试写在 `Source/FarmGameUE/FarmGameTests.cpp`（基础）和 `Source/FarmGameUE/FarmGameExtraTests.cpp`（`FarmGame.Extra.*` 补充），用 `IMPLEMENT_SIMPLE_AUTOMATION_TEST` 注册，名字统一以 `FarmGame.` 开头，分 `CropGrowth` / `Inventory` / `Crafting` / `Items` / `Recipes` / `Extra` 几组；逐条清单见 `Docs/TESTING.md`。它们是 `EditorContext + EngineFilter` 测试，而且整段包在 `#if WITH_DEV_AUTOMATION_TESTS` 里——只能由编辑器运行，打包出来的 exe 里没有这些测试。

本机跑（结果同时写进 `Saved\Logs\FarmGameUE.log`）：

~~~powershell
& "C:\Program Files\Epic Games\UE_5.8\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" `
    "$PWD\FarmGameUE.uproject" `
    -ExecCmds="Automation RunTests FarmGame" `
    -TestExit="Automation Test Queue Empty" `
    -unattended -nopause -nosplash -nullrhi -log
~~~

- `-TestExit="Automation Test Queue Empty"`：引擎打完日志 `...Automation Test Queue Empty N tests performed.` 后自己退出，不用靠超时杀进程。
- 不要写成 `Automation RunTests FarmGame;Quit`：`Quit` 可能在测试排队阶段就把编辑器关掉。
- 单条结果在日志里长这样：

~~~
LogAutomationController: Display: Test Completed. Result={Success} Name={AdvancesWithWater} Path={FarmGame.CropGrowth.AdvancesWithWater}
~~~

- 只筛失败的：`Select-String -Path Saved\Logs\FarmGameUE.log -Pattern 'Result=\{(Fail|Error)\}'`。

## 7. CI（.github/workflows/build.yml）

- **触发**：push 到 `main`（只改 `Docs/` 或 `*.md` 不触发）、对 `main` 的 PR、手动 `workflow_dispatch`（可填 `engine_root`）。
- **runner**：`[self-hosted, windows, ue5]`。GitHub 托管的 `windows-latest` 上没有 UE，这个 workflow 只能跑在自托管机上。
- **引擎路径**：手动触发的 `engine_root` → 仓库变量 `UE_ROOT`（Settings → Secrets and variables → Actions → Variables）→ 机器环境变量 `UE_ROOT` → `C:\Program Files\Epic Games\UE_5.8`。
- **步骤**：

  1. `actions/checkout@v4`（`clean: false`，保留自托管机上的 `Intermediate` / `Saved` / `DerivedDataCache`，构建走增量；要干净重建就手动删这三个目录）
  2. 校验 UE 5.8：要求同时存在 `Build.bat`、`RunUAT.bat`、`UnrealEditor-Cmd.exe`，并从 `Engine\Build\Build.version` 读版本号确认是 5.8.x；不满足时输出带标题的 `::error::` 注解（说明该装在哪、该设哪个变量、该给 runner 打什么标签）然后失败。
  3. `Scripts/package.ps1 -SkipCook` 编译 `FarmGameUEEditor`。
  4. `UnrealEditor-Cmd.exe ... -ExecCmds="Automation RunTests FarmGame" -TestExit="Automation Test Queue Empty" -nullrhi -abslog=Saved\Logs\AutomationTests.log`，随后解析日志：打印通过数、失败数和失败用例名，并写进 job summary。
  5. 上传 artifact `farmgameue-logs-<run_id>`（含 `Saved/Logs/**`、`Saved/Automation/**`，保留 14 天），条件是 `if: always()`，构建失败也能拿到日志。

- **判定规则**：日志里没有 `Automation Test Queue Empty`（没跑完 / 超时）→ 失败；一条都没跑（过滤词对不上）→ 失败；出现 `Result={Fail|Error|NotRun}` → 失败。
- **CI 只做编译 + 测试，不 cook**：完整打包在本机跑 `package.ps1`（不带 `-SkipCook`）。
- 本机复现 CI：

~~~powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\Scripts\package.ps1 -SkipCook
& "C:\Program Files\Epic Games\UE_5.8\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" `
    "$PWD\FarmGameUE.uproject" -ExecCmds="Automation RunTests FarmGame" `
    -TestExit="Automation Test Queue Empty" -unattended -nopause -nosplash -nullrhi -log
Select-String -Path Saved\Logs\FarmGameUE.log -Pattern 'Result=\{'
~~~

## 8. 常见错误

### 8.1 编辑器开着 / Live Coding 占用

现象一，UBT 直接抛异常：

~~~
Unable to build while Live Coding is active. Exit the editor and game, or press Ctrl+Alt+F11 if iterating on code in the editor or game
~~~

现象二，链接阶段文件被占：

~~~
LNK1104: 无法打开文件 "...\Binaries\Win64\UnrealEditor-FarmGameUE.dll"
~~~

原因：UnrealEditor 进程占着 `Binaries\Win64\*.dll` 和 Live Coding 的互斥量，UBT 无法覆盖输出。

处理：

1. 关掉编辑器和 PIE（`Get-Process UnrealEditor` 确认为空）。
2. 只想快速验证改动时，在编辑器里按 **Ctrl+Alt+F11** 走 Live Coding 编译——它只重编改动过的文件，**不能**替代完整构建；打包前必须关掉编辑器重跑 `package.ps1`。
3. 终端里 `Build.bat` 打印 `is already running, waiting for existing script to terminate...` 是脚本锁在等另一个实例，不是报错。

### 8.2 缺少 .NET SDK

现象（`Build.bat`，退出码 999）：

~~~
ERROR: Unable to find an install of Dotnet SDK.  Please make sure you have it installed and that `dotnet` is a globally available command.
~~~

`RunUAT.bat` 对应的是：

~~~
RunUAT.bat ERROR: Unable to find a install of Dotnet SDK.  Please make sure you have it installed and that `dotnet` is a globally available command.
~~~

两个可能的原因：

1. 设了 **`UE_USE_SYSTEM_DOTNET=1`**，但系统里没有 `dotnet` 或 SDK 低于 10.0（`Engine\Build\BatchFiles\GetDotnetPath.bat` 要求主版本 10）。
2. 引擎自带的 SDK 没了：`Engine\Binaries\ThirdParty\DotNet\10.0\win-x64` 被删、被杀软隔离，或引擎安装不完整。

处理：

~~~powershell
# 看引擎自带的那份在不在、里面有哪些 SDK
& "C:\Program Files\Epic Games\UE_5.8\Engine\Binaries\ThirdParty\DotNet\10.0\win-x64\dotnet.exe" --list-sdks
~~~

- 正常会打印 `10.0.203 [...\Engine\Binaries\ThirdParty\DotNet\10.0\win-x64\sdk]`。
- 没了就用 Epic 启动器对 UE_5.8 做一次 **Verify**，或重装引擎。
- 一定要走系统 SDK：装 .NET SDK 10 并确认 `dotnet --list-sdks` 能看到，再设 `UE_USE_SYSTEM_DOTNET=1`。

### 8.3 RunUAT 路径不对

正确的是：

~~~
<引擎根>\Engine\Build\BatchFiles\RunUAT.bat
~~~

下面这个路径**不存在**（别照抄旧教程）：

~~~
<引擎根>\Engine\Build\BatchFiles\AutomationTool\RunUAT.bat
~~~

常见表现：

- `'RunUAT.bat' 不是内部或外部命令`：路径写错，或者把 `RunUAT.bat` 拷到了别处。它**不能**拷走——脚本开头会检查自己在不在 `Engine\Build\BatchFiles` 里，否则直接报：

  ~~~text
  ERROR: The batch file does not appear to be located in the Engine\Build\BatchFiles directory. This script must be run from within that directory.
  ~~~

- 用 `package.ps1` 时，`-EngineRoot` 要指到**含 `Engine` 子目录的那一层**（`C:\Program Files\Epic Games\UE_5.8`），不是 `...\UE_5.8\Engine`。

### 8.4 其它

| 现象 | 原因 / 处理 |
| --- | --- |
| `不能加载文件 ...\package.ps1，因为在此系统上禁止运行脚本` | PowerShell 执行策略。改用 `powershell -NoProfile -ExecutionPolicy Bypass -File .\Scripts\package.ps1` |
| `RunUAT` 在 stage 阶段找不到 `FarmGameUE.exe` | 客户端没编译。确认命令里有 `-build`、没有 `-nocompile`（`package.ps1` 已经是这么写的） |
| `BuildCookRun` 在 cook 阶段报找不到资产 | 内容只有 `Content/Maps/Farm.umap`，`Config/DefaultEngine.ini` 的 `GameDefaultMap` / `EditorStartupMap` 都指向 `/Game/Maps/Farm`；地图被删或改名就会这样 |
| `archivedirectory` 写不进去 | 没权限或路径含特殊字符，换到 `D:\FarmGameUE_Package` 这类纯 ASCII 路径 |
| CI 报「一条测试都没跑」 | `Automation RunTests` 的过滤词和测试名对不上；测试名见 `FarmGameTests.cpp` 里的 `FarmGame.*` 字符串 |
| 编译很慢 | 首次全量编译正常；之后靠 `Intermediate` / `Saved` 增量，别随手删缓存 |

## 9. 快速自检

~~~powershell
# 1. 引擎在不在
Test-Path "C:\Program Files\Epic Games\UE_5.8\Engine\Build\BatchFiles\Build.bat"

# 2. 有没有编辑器进程占着 DLL
Get-Process UnrealEditor -ErrorAction SilentlyContinue

# 3. 编译（和 CI 同款）
powershell -NoProfile -ExecutionPolicy Bypass -File .\Scripts\package.ps1 -SkipCook

# 4. 打包产物
Get-ChildItem .\Saved\Archive -Recurse -Filter FarmGameUE.exe | Select-Object FullName
~~~
