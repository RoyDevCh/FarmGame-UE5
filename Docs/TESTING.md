# FarmGameUE 自动化测试

本工程使用 UE 自带的 **Automation Test** 框架（`IMPLEMENT_SIMPLE_AUTOMATION_TEST`）验证纯逻辑。
测试不创建 `UWorld`、不启动 PIE，因此可以在 `UnrealEditor-Cmd.exe` 中以 `-nullrhi` 无界面运行。

当前共 **38 条**测试（`FarmGameTests.cpp` 32 条 + `FarmGameExtraTests.cpp` 6 条）：

- 基础测试 32 条：`Source/FarmGameUE/FarmGameTests.cpp`
- 补充测试 6 条：`Source/FarmGameUE/FarmGameExtraTests.cpp`

> 数量按 2026-11-09 的源码清点；本机同日全量运行 38/38 Success（`Saved/Logs/FarmGameAutomation.log`，日志内自报 38 tests performed）。

测试代码全部包在 `#if WITH_DEV_AUTOMATION_TESTS` 中，只在开发/编辑器构建里编译，Shipping 构建不包含。
现有依赖（`Core`、`CoreUObject`、`Engine`）已足够，**不需要改 `FarmGameUE.Build.cs`**。

## 测试清单

### 基础测试（FarmGameTests.cpp，32 条）

| 测试名 | 验证内容 |
|---|---|
| `FarmGame.CropGrowth.AdvancesWithWater` | 胡萝卜（3 阶段、每阶段 1 天）浇水跨天后阶段 +1，生长日更新，并消耗浇水状态 |
| `FarmGame.CropGrowth.BlockedWithoutWater` | 未浇水不生长；每阶段 2 天的土豆需要累计 2 天才会推进 |
| `FarmGame.CropGrowth.MaturesAndStops` | 4 阶段玉米连续推进 10 天，停在第 3 阶段（成熟），`IsMature` 为真 |
| `FarmGame.Inventory.BuyAndSell` | 初始 20 金币/3 胡萝卜种子/6 木材；买种子扣钱；卖 2 胡萝卜 + 1 ingot 得 34，种子保留 |
| `FarmGame.Inventory.CannotBuyWithoutGold` | 金币不足时购买失败，金币与种子数量不变 |
| `FarmGame.Crafting.ConsumesIngredientsAndOutputs` | 围栏配方消耗 5 木材（6→1）并产出 1 个 fence_kit；材料不足时再次制作失败；ingot 配方带熔炉标记 |
| `FarmGame.Items.CatalogIsConsistent` | 物品目录不少于 10 项；胡萝卜种子/胡萝卜存在；胡萝卜售价 8；种子判定与 `SeedId(玉米)` 正确 |
| `FarmGame.Crafting.HayFromCorn` | 2 玉米合成 2 干草；材料耗尽后再次合成失败 |
| `FarmGame.Inventory.SellSkipsSeedsAndResources` | 出售时木材/矿石保留，鸡蛋与牛奶卖出共 36 金币 |
| `FarmGame.Recipes.CatalogReferencesKnownItems` | 每个配方的产出物与配料都在物品目录中，配料数量为正，且配方有输入 |
| `FarmGame.Crafting.FurnaceSmeltsOneOrePerInterval` | 熔炉未满 8 秒不产出；满 8 秒消耗 1 矿石产出 1 ingot；矿石数量不会变成负数 |
| `FarmGame.Crops.CatalogSanity` | 作物表恰好 5 种、Id 唯一、至少 2 阶段、每阶段至少 1 天、单次产量 ≥1、单次利润 >0，且种子与产物都在物品表里 |
| `FarmGame.Items.IdsAreUnique` | 物品 Id 非空且唯一、Label 非空，物品表至少 18 项 |
| `FarmGame.Recipes.IdsAreUnique` | 配方 Id 唯一、`OutputCount > 0`，配方表至少 4 条 |
| `FarmGame.Inventory.UnknownItemRejected` | 购买不存在的物品失败，金币与背包都不变 |
| `FarmGame.Inventory.EmptySaleIsNoop` | 空背包卖货收入 0、金币不变、`GetSellableCount` 为 0 |
| `FarmGame.Crafting.ExactMaterialsConsumeAll` | 恰好 10 石 + 5 木做熔炉套件：材料清零、产出 1 个 |
| `FarmGame.Save.DefaultsAreSane` | `UFarmSaveGame` 默认 1 天 / 20 金 / 100 HP，地块、建筑、动物数组为空 |
| `FarmGame.Crops.RegrowAfterHarvest` | 番茄 `StageAfterHarvest` 为 1 且标记 regrow，其余 4 种为 -1（清地） |
| `FarmGame.Inventory.ChestDepositWithdraw` | 存入只搬 Resource（7 木 + 3 石 + 1 干草 = 11），胡萝卜留在背包；取回 11 个后箱子清空 |
| `FarmGame.Crafting.ChestRecipeCostsWood` | 箱子配方只有 1 条输入（wood ×12），`chest_kit` 在物品表里 |
| `FarmGame.Quests.AdvanceToCompletion` | 默认 8 段；进度逐次 +1、到目标判定完成、封顶后不可重复完成 |
| `FarmGame.Quests.DefaultsReferenceItems` | 每段目标标题非空、Target > 0，匹配的物品都在物品表里 |
| `FarmGame.Tools.GatherYieldDoubles` | `GatherYield(0/1/2)` 分别为 1/2/3 |
| `FarmGame.Tools.CatalogHasAxeAndPickaxe` | 斧/镐/钢斧/钢镐都在物品表与配方表里，且配方产出就是该工具 |
| `FarmGame.Shop.StockIsValid` | 商店至少 8 种货，每种都在物品表里且价格 > 0 |
| `FarmGame.Food.CatalogHasAppleAndBread` | 苹果/面包在物品表里、类别为 `Food`、买价 > 0 |
| `FarmGame.Food.ShopStockIncludesFood` | 商店货架包含苹果与面包，且价格 > 0 |
| `FarmGame.Tools.SteelRecipesConsumeBaseTool` | 钢斧/钢镐配方各有 2 条输入：基础工具 ×1 + 矿石 ×5 |
| `FarmGame.Quests.LastObjectiveIsSteelAxe` | 8 段目标的第 7 段是苹果、第 8 段是钢斧（Target = 1） |
| `FarmGame.Inventory.SpendGoldIsAtomic` | 20 金花 5 金后剩 15；再花 999999 失败且金币不变 |
| `FarmGame.Recipes.AllOutputsExistInCatalog` | 所有配方的产物都在物品表里，`OutputCount ≥ 1` |

### 补充测试（FarmGameExtraTests.cpp，6 条）

| 测试名 | 验证内容 |
|---|---|
| `FarmGame.Extra.CropMultiStageContinuous` | 合成作物 Wheat（4 阶段、每阶段 2 天）逐日检查：第 1 天不足间隔不推进，第 2/4/6 天各推进一级（0→1→2→3）并记录生长日，第 8 天成熟停止并清除浇水 |
| `FarmGame.Extra.SmeltTwoStepsInARow` | 3 矿石、5 秒步长、4 秒/锭，连续两次 `SmeltStep` 产出 2 个 ingot，剩余 1 矿石，计时器余量依次为 1/2 秒 |
| `FarmGame.Extra.SellIncludesIngotKeepsSeedsAndResources` | `SellEverything` 卖出 ingot 和胡萝卜共 44 金币，种子、木材、矿石、干草全部保留；`GetSellableCount` 只统计可卖项 |
| `FarmGame.Extra.CraftFailureKeepsMaterials` | 熔炉配方（10 石头 + 5 木材）在 0 石头和 5 石头两种失败情况下都不扣木材/石头，也不产出 fence_kit |
| `FarmGame.Extra.SeedIdsForThreeCrops` | 胡萝卜/土豆/玉米分别映射到 `carrot_seed`/`potato_seed`/`corn_seed`，且三者在物品目录中存在并标记为种子 |
| `FarmGame.Extra.RecipeOutputsInCatalog` | 遍历全部配方，产出物在物品目录中存在且 `OutputCount > 0`；fence_kit/furnace_kit/ingot/hay 四个预期产出均存在 |

## 如何运行

### 方式一：一键脚本（推荐）

在工程根目录运行：

```powershell
cd C:\Users\rjq51\Documents\dsh\UE5adaptation\FarmGameUE
powershell -NoProfile -ExecutionPolicy Bypass -File .\Scripts\run-tests.ps1
```

脚本支持 Windows PowerShell 5.1 与 PowerShell 7。

只跑补充测试（`FarmGame.Extra`，6 条）：

```powershell
.\Scripts\run-tests.ps1 -TestFilter FarmGame.Extra
```

引擎不在默认位置时显式指定：

```powershell
.\Scripts\run-tests.ps1 -EngineRoot 'D:\Epic\UE_5.8'
```

参数：

| 参数 | 默认值 | 说明 |
|---|---|---|
| `-ProjectPath` | 脚本所在工程根目录下的 `FarmGameUE.uproject` | 可传 .uproject 路径或包含它的目录 |
| `-EngineRoot` | 自动探测 | 引擎根目录（其下应有 `Engine\Binaries\Win64\UnrealEditor-Cmd.exe`） |
| `-TestFilter` | `FarmGame` | 传给 `Automation RunTests` 的过滤器，也可用 `FarmGame.Extra`、`FarmGame.CropGrowth` 等 |
| `-LogPath` | `<工程>\Saved\Logs\FarmGameAutomation.log` | 编辑器日志路径，脚本会先删除旧文件 |

脚本做的事：

1. 从 `.uproject` 的 `EngineAssociation` 读取引擎版本（本工程为 `5.8`）；
2. 定位 `UnrealEditor-Cmd.exe`：指定 `-EngineRoot` 时只用该目录；否则依次尝试环境变量
   `UE_ENGINE_ROOT`、注册表 `HKLM/HKCU\SOFTWARE\EpicGames\Unreal Engine\<版本>\InstalledDirectory`、
   `%ProgramData%\Epic\UnrealEngineLauncher\LauncherInstalled.dat`、
   `%ProgramFiles%\Epic Games\UE_<版本>`；
3. 以 `-unattended -nopause -nosplash -nullrhi -nosound -stdout -Culture=en -log` 运行
   `Automation RunTests <Filter>`，并用 `-TestExit=Automation Test Queue Empty` 让编辑器在测试队列跑完后自动退出；
4. 解析日志中的 `Test Completed` 行，按 `Result={...}` 统计成功/失败，打印失败测试名；
5. 全部成功退出码为 0；只要有失败、没有跑出任何测试、编辑器异常退出或日志缺失，退出码为 1。

全部通过时的输出示例：

```text
 FarmGame automation summary
==============================================
 Test Completed lines : 38
 Success              : 38
 Failed               : 0
 Editor exit code     : 0

RESULT: PASS
```

### 方式二：手动运行 UnrealEditor-Cmd

```powershell
$Editor  = 'C:\Program Files\Epic Games\UE_5.8\Engine\Binaries\Win64\UnrealEditor-Cmd.exe'
$Project = (Resolve-Path .\FarmGameUE.uproject).Path
$Log     = Join-Path $PWD 'Saved\Logs\FarmGameManual.log'
$EditorArgs = @(
    $Project,
    '-ExecCmds=Automation RunTests FarmGame',
    '-TestExit=Automation Test Queue Empty',
    '-unattended',
    '-nopause',
    '-nosplash',
    '-nullrhi',
    '-nosound',
    '-stdout',
    '-Culture=en',
    '-log',
    "-abslog=$Log"
)

& $Editor @EditorArgs
```

注意 `-TestExit=Automation Test Queue Empty` 必须作为一个完整参数传入（PowerShell 中整体加引号）。
测试结束后查看结果行：

```powershell
Select-String -Path .\Saved\Logs\FarmGameManual.log -Pattern 'Test Completed'
```

日志格式：

```text
LogAutomationController: Display: Test Completed. Result={Success} Name={FarmGame.CropGrowth.AdvancesWithWater} Path={...}
```

按前缀运行子集：`Automation RunTests FarmGame.Extra`、`Automation RunTests FarmGame.Quests`、`Automation RunTests FarmGame.Tools`、`Automation RunTests FarmGame.Shop`、`Automation RunTests FarmGame.Food`、`Automation RunTests FarmGame.Inventory` 等。

## 如何新增测试

1. 在 `Source/FarmGameUE/` 下新建 `.cpp`（或加入现有测试文件）。模块内所有 `.cpp` 会被 UBT 自动编译，
   不需要注册表或额外的构建配置。
2. 包含 `Misc/AutomationTest.h` 以及被测逻辑的头文件，整个文件用
   `#if WITH_DEV_AUTOMATION_TESTS` / `#endif` 包住。
3. 用 `IMPLEMENT_SIMPLE_AUTOMATION_TEST` 注册，测试路径统一以 `FarmGame.` 开头，
   建议遵循 `FarmGame.<系统>.<行为>`，补充测试用 `FarmGame.Extra.*`。
4. 测试标志沿用 `EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter`。
5. 重新编译 `FarmGameUE` 编辑器目标后运行脚本；测试按名称自动被发现，无需手工登记。

最小模板：

```cpp
#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "FarmTypes.h"
#include "FarmInventoryComponent.h"
#include "FarmItems.h"
#include "FarmRecipes.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFarmMyNewTest, "FarmGame.Extra.MyNewRule",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFarmMyNewTest::RunTest(const FString& Parameters)
{
	UFarmInventoryComponent* Inventory = NewObject<UFarmInventoryComponent>();
	TestNotNull(TEXT("inventory created"), Inventory);
	if (!Inventory)
	{
		return false;
	}

	Inventory->ResetForNewGame();
	TestEqual(TEXT("starting gold"), Inventory->Gold, 20);
	return true;
}

#endif
```

编写约定：

- 只测纯逻辑：`FarmLogic::AdvanceCrop` / `FarmLogic::SmeltStep`、`FarmItems`、`FarmRecipes`，
  或用 `NewObject<UFarmInventoryComponent>()` 测背包；不要依赖世界、Tick、随机数或真实时间。
- 断言用 `TestTrue` / `TestFalse` / `TestEqual` / `TestNotNull`；
  断言失败会记为测试失败，函数仍可返回 `true`。只有必须先中断（如指针为空）时才提前返回 `false`。
- 浮点比较传容差，例如 `TestEqual(TEXT("timer"), Timer, 2.f, 0.001f)`。
- 测试路径和类名必须唯一，避免重复注册。
- 新增测试后同步更新本文件的清单。

## 排查

| 现象 | 处理 |
|---|---|
| 找不到 `UnrealEditor-Cmd.exe` | 用 `-EngineRoot` 指向 `...\UE_5.8` 这样的引擎根目录 |
| 日志里没有 `Test Completed` | 模块编译失败或过滤器无匹配；先在日志中搜 `Error`，确认测试路径前缀 |
| 脚本一直不退出 | 确认 `-TestExit=Automation Test Queue Empty` 作为单个参数传入，且测试队列为空后会退出 |
| 测试数量与预期不符 | 新增 `.cpp` 后需要重新编译编辑器目标；用 `-TestFilter FarmGame` 跑全量确认 |

## 边界

当前测试覆盖：纯规则（生长、冶炼、采集倍率、目标推进）、全部数据表（物品 / 配方 / 作物 / 目标 / 商店货架）、
`UFarmInventoryComponent`（买卖、制作、卖货、箱子存取），以及 `UFarmSaveGame` 的默认值。

**不覆盖**：Actor 交互（`E` 的分派）、地块状态机与视觉、实际存读档往返、`NotifyProduced` 的奖励发放与 HUD 目标行、
Canvas HUD、Slate 菜单 / 商店 / 设置（焦点、点击、Esc 返回）、手持工具姿态与渲染。
这些需要 PIE 内 Functional Test、Slate 自动化或手工验证；本批测试刻意保持可在命令行的无渲染环境中稳定运行。

已知的 UI 缺口：商店的购买/一键卖货、开始菜单的 Continue/新游戏/切槽、设置的全屏/画质都**没有**自动化 UI 测试，
只有商店货架数据（`FarmGame.Shop.StockIsValid`）和目标表（`FarmGame.Quests.*`）被覆盖。
