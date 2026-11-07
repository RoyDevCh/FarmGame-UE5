# 设计对照：Unity 原版 → UE5 垂直切片

## 1. 本文范围与事实来源

- **Unity 侧**：本地参考工程 `_ref/FarmGame`（RoyDevCh/FarmGame，Unity 2021.3 + Farming Engine 框架）。
  它**不在本仓库内**，只作为对照物；本文提到的 Unity 类名、字段名都按该目录下的 `.cs` 文件核对过。
- **UE5 侧**：本仓库 `Source/FarmGameUE` 的实际代码。行为以代码为准，不以后续设想为准。
- 本文只回答两件事：**原版的系统在 UE5 里落到了哪个类 / 文件**，以及**哪些原版系统没有做、为什么**。
- 对应 2026-11-08 的源码状态；代码继续变动时以源码为准。

## 2. 架构总览

    AFarmGameMode ────────── 持有 Crops / Plots / Buildings / ResourceNodes / Animals / Portals
      │                       管 Day / DayTimer / 天气 / 存读档 / 世界生成
      ├─ AFarmPlot           地块状态机（生长由 GameMode 每天调用）
      ├─ AFarmBin            发货箱
      ├─ AFarmBuilding       栅栏 / 熔炉（熔炉自己 Tick 冶炼）
      ├─ AFarmResourceNode   树 / 石头 / 矿脉（每天 Regrow）
      ├─ AFarmAnimal         鸡 / 牛（漫游）/ 野猪（追击 + 咬）
      └─ AFarmPortal         农场 ↔ 矿洞

    AFarmCharacter ──── UFarmInventoryComponent（金币 + 物品） / HP / 相机
    AFarmPlayerController ── 输入、最近目标、建造模式、制作、攻击、Toast、存读档
    AFarmHUD ─────────── 读 GameMode / Controller / Character，Canvas 直接画
    FarmLogic（namespace）── 纯函数：AdvanceCrop / IsMature / SmeltStep（不碰 UObject，测试直接调）

| 类 | 职责 | Tick | 关键依赖 |
|---|---|---|---|
| `AFarmGameMode` | 世界生成、日夜、天气、每日推进、存读档 | 是 | 全部 Actor 的注册表 |
| `AFarmPlot` | 单块地的状态机与视觉 | 否 | `FCropDef`、`UFarmInventoryComponent` |
| `AFarmCharacter` | 玩家胶囊体、弹簧臂相机、HP | 否 | `UFarmInventoryComponent` |
| `AFarmPlayerController` | 所有按键行为与目标选择 | 是 | 上面所有 Actor 的查询 |
| `AFarmHUD` | 全屏信息 | 否（DrawHUD） | GameMode / Controller / Character |
| `AFarmAnimal` | 养殖与敌对 AI | 是 | 玩家位置 |
| `AFarmBuilding` | 建筑与冶炼 | 是 | 玩家背包（装填/取锭） |
| `AFarmResourceNode` | 采集 | 否 | 玩家背包 |
| `AFarmPortal` | 传送 | 否 | 玩家位置 |
| `AFarmBin` | 卖货 | 否 | 玩家背包 |
| `UFarmInventoryComponent` | 金币与物品的唯一真源 | 否 | `FarmItems` / `FarmRecipes` |

## 3. 逐系统映射

### 3.1 种植

| Unity 原版 | UE5 实现 | 差异 |
|---|---|---|
| `Data/PlantData.cs`（ScriptableObject；`growth_stage_prefabs`、`plant_prefab`） | `FCropDef`（USTRUCT；`Stages`、`DaysPerStage`、`SeedCost`、`SellPrice`、`YieldPerHarvest`、`Color`） | 原版每阶段一个预制体；本版只有一个按比例缩放的 Cylinder |
| `Gameplay/Plant.cs`：`growth_stage`、`grow_time`、`time_type`、`grow_require_water`、`soil_range` | `AFarmPlot::Stage` / `bWatered`，`FCropDef::DaysPerStage`，`FarmLogic::AdvanceCrop` | 原版支持"游戏天 / 游戏小时"两种时间单位；本版只按天 |
| `Plant.cs`：`regrow_on_death`、`death_on_harvest`、`fruit`、`fruit_grow_time` | 无 | 本版收获后地块直接回 `Tilled`，没有"多次结果"与"收获即死"分支 |
| `Gameplay/Soil.cs`：`watered`、雨天自动浇、`Sprinkler` 自动浇 | `AFarmPlot::bWatered` + `WaterFromRain()` | 没有独立 Soil 实体，水状态直接挂在地块上；洒水器未实现 |
| `Actions/ActionHoe.cs` / `ActionPlant.cs` / `ActionWaterPlant.cs` / `ActionHarvest.cs` | `AFarmPlot::Interact()` 按 `EFarmPlotState` 分派 | 原版是 4 个独立 Action 类（可由道具/工具切换）；本版合成一个状态分派函数 |
| `Gameplay/Selectable.cs`（`use_range = 2` 米） | `AFarmPlayerController::UpdateTarget()`（320 cm 内取最近） | 原版是"点击 → 走到 use_range → 触发动作"；本版是"贴近 → 按 E" |

### 3.2 时间、天气与光照

| Unity 原版 | UE5 实现 | 差异 |
|---|---|---|
| `TheGame.cs`（`onNewDay` / `onSkipTime` / `beforeSave`） | `AFarmGameMode`（`Day` / `DayTimer` / `AdvanceDay()`） | 原版是全局单例 + UnityAction 事件；本版是 GameMode 成员 + 直接调用 |
| `Actions/ActionSleep.cs` / `ActionSleepDay.cs`（场景里的床） | `T` 键 `AFarmPlayerController::Sleep()` | 原版需要走到床上交互；本版任意位置按 T |
| `Gameplay/WeatherSystem.cs`（`weathers` 列表、`weather_change_time = 6` 游戏小时、`GetLightMult()`） | `AFarmGameMode::bRaining`（每天 1/4 概率）+ Tick 里的太阳角度/强度 | 原版是多天气资产、按游戏小时切换、对外提供光照倍率；本版只有晴/雨、按天掷骰 |
| 原版由时间/天气系统驱动的昼夜光照 | Tick 里手写曲线：`Pitch = -14 - 70·sin(P·π)`，强度 5.0 / 1.9 | 不是曲线资产，改要改代码 |

### 3.3 交互与输入

| Unity 原版 | UE5 实现 | 差异 |
|---|---|---|
| `Player/PlayerControls.cs` + `PlayerControlsGamepad.cs` + `PlayerControlsMouse.cs` + `Tools/InControlWrap.cs` | `Config/DefaultInput.ini` 的 ActionMappings + `AFarmPlayerController::SetupInputComponent()` 的 `BindAction` | 原版三套输入（键鼠 / 手柄 / 触屏）+ InControl 插件；本版只有键鼠 legacy 映射（默认 PlayerInput 类虽然是 EnhancedInput，但工程里没有 InputAction 资产） |
| `Selectable` + `SAction` + `UI/ActionSelector.cs`（动作列表 / 动作条） | `UpdateTarget()` 最近目标 + `GetPromptText()` + `Interact()` 分派 | 原版一个物体可以挂多个动作供玩家选；本版每个物体只有一种 E 行为 |

### 3.4 物品与经济

| Unity 原版 | UE5 实现 | 差异 |
|---|---|---|
| `Data/ItemData.cs`（ScriptableObject）、`Data/GroupData.cs` 分组、`Gameplay/Item.cs` | `FItemDef` + `FarmItems::All()`（18 项，`FName` 字符串 Id） | 原版是资产引用与分组过滤；本版是静态表 + 字符串 Id |
| `Data/PlayerData.cs` / `Data/InventoryData.cs` / `Player/PlayerCharacterInventory.cs` | `UFarmInventoryComponent`（`Gold` + `TMap<FName, int32>`） | 原版有格子、堆叠（`ItemStack`）与拖拽；本版是无容量字典 |
| `Gameplay/ShopNPC.cs` + `Actions/ActionShop.cs` + `UI/ShopPanel.cs` | `AFarmBin`（`E` 卖全部）+ `B` 买当前种子 | 没有商店 NPC、没有分类定价界面 |

### 3.5 制作与建造

| Unity 原版 | UE5 实现 | 差异 |
|---|---|---|
| `Data/CraftData.cs`（`craft_items` / `craft_fillers` / `craft_requirements` / `craft_near` / `craft_duration` / `craft_xp`） | `FCraftRecipe`（`Inputs` / `OutputId` / `OutputCount` / `bNeedsFurnace`） | 本版只保留"材料 + 是否要熔炉"；没有填充物、前置、耗时、经验 |
| `Gameplay/CraftStation.cs`（`craft_groups`、`range = 3`） | `AFarmPlayerController::IsNearFurnace()`（320 cm） | 原版是通用工作台分组（火源/水源都能当 `craft_near`）；本版只有熔炉这一种前置 |
| `UI/CraftPanel.cs` + `CraftInfoPanel.cs` + `Player/PlayerCharacterCraft.cs` | `C` 菜单（HUD 画）+ `1`–`5` 结算 | 没有分类、搜索、数量选择、制作进度条 |
| `Gameplay/Buildable.cs`（`build_distance = 2`、`grid_size = 1`、`obstacle_layer`、`build_obstacle_radius`）+ `Actions/ActionBuild.cs` | `G` 建造模式 + `UpdateGhost()`（前方 220、吸附 100）+ `IsBuildLocationFree()` | 原版按碰撞层检查地面/障碍；本版只用半径比较地块 / 建筑 / 发货箱 |
| `Gameplay/Construction.cs` + `Data/ConstructionData.cs` | `AFarmBuilding`（`EBuildingType`：Fence / Furnace） | 原版建筑来自资产、有"在建 → 完成"过程；本版两种固定建筑、放置即完成 |

### 3.6 动物与战斗

| Unity 原版 | UE5 实现 | 差异 |
|---|---|---|
| `Gameplay/AnimalLivestock.cs`（`LivestockState`、`item_produce`、`item_eat_count`、`item_produce_time = 24` 游戏小时、`eat_interval_time = 12`、`item_collect_type`） | `AFarmAnimal`（`bFed` → 跨天 → `bProductReady` → `E` 收取） | 原版按小时累积、可吃多口才产出、产物按类型掉地上或手收；本版一次喂食、次日必产 1 个、直接进背包 |
| `Gameplay/AnimalWild.cs` | `AFarmAnimal` 的 Boar 分支（700 内追击、210 内每 1.6 秒咬 8） | 原版有 `WanderBehavior` 等可配置项；本版是写死的直线追击 |
| `Player/PlayerCharacterCombat.cs` + `Actions/ActionAttack.cs` | `AFarmPlayerController::Attack()`（左键、34 伤害、0.45 秒冷却） | 没有武器、耐久、攻击动画与受击反馈 |
| `Gameplay/Destructible.cs` + `Gameplay/Regrowth.cs` + `Actions/ActionCut.cs` | `AFarmResourceNode` + `Regrow()` | 原版靠 HP 与掉落表；本版是"剩余次数"计数，每天回满 |
| `Player/PlayerCharacterAttribute.cs` + `UI/AttributeBar.cs` / `XPBar.cs` + `Actions/ActionEat.cs` / `ActionDrinkPond.cs` | `AFarmCharacter` 只有 HP | 没有体力 / 饥饿 / 口渴 / XP |

### 3.7 场景与传送

| Unity 原版 | UE5 实现 | 差异 |
|---|---|---|
| `Assets/FarmingEngine/Scenes/Farm.unity`、`House.unity`、`Mine.unity`、`Test.unity`（`ProjectSettings/EditorBuildSettings.asset` 里四个都在构建列表） | 只有 `/Game/Maps/Farm` 一张图 | 见 4.5 |
| `Gameplay/ExitZone.cs` + `Actions/ActionScene.cs` + `Tools/SceneNav.cs`（`entry_index`） | `AFarmPortal`（同图 `SetActorLocation`） | 原版换场景并记录从哪个入口进；本版只是位移 |
| `Gameplay/Door.cs` / `Ladder.cs` / `Zone.cs` | 无 | 没有室内、门、梯子与区域触发 |

### 3.8 存档

| Unity 原版 | UE5 实现 | 差异 |
|---|---|---|
| `Tools/SaveSystem.cs` + `Tools/SaveTool.cs`（JSON；`PlayerData` 按 `UniqueID` 存每个对象的 custom int） | `UFarmSaveGame`（`USaveGame`）+ `AFarmGameMode::SaveGame()/LoadGame()` | 原版是增量式、能记住"哪个对象被移除了"；本版是整体快照，只记录明确列出的 9 类字段 |
| `PlayerData.current_scene` / `current_entry_index` | 无 | 单地图不需要 |

### 3.9 UI

| Unity 原版 | UE5 实现 | 差异 |
|---|---|---|
| `UI/TheUI.cs`、`UIPanel`、`ItemSlotPanel`、`BagPanel`、`InventoryPanel`、`CraftPanel`、`StoragePanel`、`ShopPanel`、`TimeClockUI`、`HPBar`、`XPBar`、`TooltipPanel` | `AFarmHUD::DrawHUD()` 一处绘制 | Canvas 立即模式；没有 Widget、拖拽、tooltip、手柄焦点 |

### 3.10 测试

| Unity 原版 | UE5 实现 | 差异 |
|---|---|---|
| `Assets/FarmingEngine/Tests/`：`DataLayerTests.cs`、`GameplayInteractionTests.cs`、`SceneSmokeTests.cs`（Unity Test Framework） | `Source/FarmGameUE/FarmGameTests.cpp`（11 条）+ `FarmGameExtraTests.cpp`（6 条），共 17 条 UE Automation | 本版只覆盖纯逻辑与组件，没有场景级/交互级测试 |

## 4. 关键设计决策

### 4.1 世界在运行时程序化生成

- **做法**：`/Game/Maps/Farm` 是空关卡，`AFarmGameMode::BuildWorld()` 在 BeginPlay 里生成地面、36 块地、发货箱、动物、采集点、传送门、装饰树。
- **理由**：布局只由代码决定，改一个常量就能改全图；不依赖手摆场景和资产，无头/CI 环境也可复现。
- **代价**：编辑器里不能拖拽微调；每次 PIE 都重建；将来换真美术要改成生成蓝图或 DataAsset 驱动的 Actor。

### 4.2 纯函数规则层 FarmLogic

- **做法**：`AdvanceCrop` / `IsMature` / `SmeltStep` 都是 `FarmTypes.h` 里的 inline 纯函数，不碰 UObject 与 World。
- **理由**：24 条测试里有 6 条直接调这些函数，不需要起世界、不需要 Tick，跑得快且稳定。
- **代价**：状态仍然分散在 `AFarmPlot` / `AFarmBuilding` 成员上，规则函数只是"算一次"，Actor 负责搬运。

### 4.3 数据是 C++ 静态表

- **做法**：`FCropDef` 硬编码在 `AFarmGameMode` 构造函数；`FItemDef` 在 `FarmItems::All()`；`FCraftRecipe` 在 `FarmRecipes::All()`。
- **理由**：零 `.uasset` 依赖（全工程除地图外没有资产），编译期即可看到全部数值，改数值不用开编辑器。
- **代价**：策划改不了；没有导入校验，只能靠 `FarmGame.Recipes.CatalogReferencesKnownItems` 这类测试兜底；后续要迁 DataAsset / DataTable。

### 4.4 Canvas HUD 而不是 UMG

- **做法**：`AFarmHUD::DrawHUD` 逐帧 `DrawText` / `DrawRect`。
- **理由**：不需要 Widget 蓝图，HUD 与数据在同一处，改文案只改 C++。
- **代价**：没有布局系统、没有本地化表、没有手柄焦点与自适应；制作菜单只能用数字键选择。

### 4.5 单地图 + 传送门代替四个场景

- **做法**：矿洞是同一张地图上 `(2600, 2600)` 的一块空地，两个 `AFarmPortal` 互相传送。
- **理由**：避开了 Level Streaming、跨场景存档、入口索引（原版 `entry_index`）、场景资源加载这一整套机制。
- **代价**：没有 House/室内；"矿洞"其实是露天空地；地图越大，可视性与 Tick 成本越差。

### 4.6 全量快照存档

- **做法**：`UFarmSaveGame` 存 9 类字段（天、当天进度、天气、金币、物品、HP、地块、建筑、动物），读档时整体覆盖并重建建筑/动物。
- **理由**：实现简单、行为可预测，`Ctrl+S` 之后状态完全可复现。
- **代价**：新增系统要同时改 `FarmSaveGame.h`、`SaveGame()`、`LoadGame()` 三处；没列进快照的东西（采集点剩余、玩家位置）读档后必然丢失。

### 4.7 "最近目标 + E" 的交互模型

- **做法**：`AFarmPlayerController::UpdateTarget()` 每帧遍历地块 / 建筑 / 采集点 / 动物 / 传送门 / 发货箱，取 320 内最近者。
- **理由**：不需要 NavMesh、不需要点击寻路，键鼠都能操作，状态机也简单。
- **代价**：玩家站在两种物体中间时目标不可预判；物体数量上去后是每帧 O(n)；没有原版"一个物体多个动作"的概念。

## 5. 未实现项与原因

| 未实现 | 原版出处 | 当前状态 | 原因 / 前置条件 |
|---|---|---|---|
| UMG 背包与制作面板 | `UI/BagPanel`、`InventoryPanel`、`CraftPanel`、`ItemSlotPanel` | Canvas HUD 代替 | 需要 Widget 蓝图资产；本工程刻意保持"除地图外零资产"。要做需先引入 Content 资产与 UI 工作流（见 4.4） |
| House 场景 | `Scenes/House.unity` | 无 | 见 4.5：需要场景切换、入口索引、室内碰撞与家具 |
| 宠物（驯服 / 跟随 / 待命） | `Gameplay/Pet.cs`、`Actions/ActionPetTame` / `ActionPetFollow` / `ActionPetStay` | 无 | 跟随需要 AIController / NavMesh 或自定义避障；本版动物都是 `SetActorLocation` 直线移动，工程未启用导航系统 |
| 塔防 / 陷阱 / 投射物 | `Gameplay/TowerAttack`、`Trap`、`Projectile`、`Actions/ActionSetTrap` | 无 | 需要伤害来源分类、AI 选目标、投射物物理；属于战斗子系统的第二阶段 |
| 洒水器 | `Gameplay/Sprinkler` | 无（用"雨天自动浇水"代替） | 需要"自动浇水装置"这类建筑与范围判定；当前只有栅栏/熔炉两种建筑 |
| 箱子 / 仓库 | `Gameplay/Storage`、`Actions/ActionStorage`、`UI/StoragePanel` | 无（背包无限） | 需要容器数据与容器 UI；在 Canvas HUD 下做拖拽成本高 |
| 商店 NPC | `Gameplay/ShopNPC`、`UI/ShopPanel`、`Actions/ActionShop` | 用发货箱买卖 | 需要 NPC、对话/商店 UI、带分类的定价数据 |
| 混合锅 / 烹饪 | `Gameplay/MixingPot`、`Actions/ActionCook` / `ActionAddFuel` | 无 | 需要多材料槽与燃料燃烧；当前熔炉只做 1:1 冶炼 |
| 体力 / 饥饿 / XP / 等级 | `Player/PlayerCharacterAttribute`、`UI/AttributeBar` / `XPBar`、`Actions/ActionEat` / `ActionDrinkPond` | 只有 HP | 需要属性系统、消耗与恢复来源、UI 条，还要先定玩法平衡 |
| 音频 | `TheAudio.cs`、各处的 `build_audio` / `gather_audio` | 无 | 需要音频资产与授权；工程里没有 SoundCue / SoundWave |
| 手柄 / 触屏 | `Player/PlayerControlsGamepad`、`UI/JoystickMobile` | 仅键鼠 | 需要 Enhanced Input 资产与触摸 UI 层 |
| 背包格 / 堆叠上限 | `Data/InventoryData`、`Gameplay/ItemStack`、`UI/ItemSlotPanel` | `TMap` 无上限 | 需要容量规则 + 容器 UI |
| 阅读 / 学习 / 对话 / 任务 | `Gameplay/ReadObject`、`Actions/ActionRead` / `ActionLearn`、`Tools/DialogueQuestsWrap` | 无 | 需要文本资产与任务系统，超出垂直切片范围 |
| 骑乘 / 载具 | `Gameplay/AnimalRide`、`Actions/ActionRide` | 无 | 需要角色挂载点与移动控制切换 |
| 任意位置种植 | `Plant` 是可建造对象（`Buildable` + `Selectable`），能放在任意地面 | 只能种在 36 块地里 | 本版用"地块 = 语义单元"简化状态与存档；自由种植需要空间索引 + 每株植物的独立序列化 |
| 美术资产 | 337 个 FBX / 325 张贴图 / 203 个材质 | 导入 17 个 FBX（`Content/Art`）+ 运行时包围盒归一化；其余用 `/Engine/BasicShapes` | 只导入关键道具，控制体积；许可仍需确认 |
| 数据资产化 | 一整套 ScriptableObject | C++ 静态表 | 见 4.3 |

## 6. 如果继续推进：建议顺序

1. **数据资产化**（`FCropDef` → `UDataAsset`）：后面所有系统都会吃它，越晚改越贵。
2. **UMG 背包 / 制作面板**：同时解决"容器 UI"这个公共问题，也是箱子/商店的前置。
3. **House 场景 + 场景切换**：把 `AFarmPortal` 升级成 `ExitZone` 等价物，顺带补 `entry_index` 存档。
4. **属性系统**（体力 / 饥饿）：让"吃饭、睡觉、下雨"这些已有行为有成本，玩法深度立刻上来。
5. **战斗第二阶段**（陷阱、投射物、塔防）：本版已有 HP、伤害、敌对 AI 的底座。
6. **宠物与骑乘**：依赖导航，建议放到有 NavMesh 之后。
7. **音频与美术**：最后替换，不影响逻辑。

## 7. 数值与数据的修改入口

| 想改 | 改哪里 |
|---|---|
| 作物数值（阶段、天数、价格、产量、颜色） | `AFarmGameMode::AFarmGameMode()` 里的 `FCropDef` 常量 |
| 物品价格与类别 | `FarmItems::All()` |
| 配方 | `FarmRecipes::All()` |
| 一天多长 | `AFarmGameMode::DayLengthSeconds`（默认 180） |
| 下雨概率 | `AFarmGameMode::AdvanceDay()` 的 `FMath::RandRange(0, 3) == 0` |
| 熔炉容量 / 速度 | `AFarmBuilding::MaxOre()` / `SmeltSecondsPerOre()` |
| 建造间距与吸附 | `AFarmGameMode::IsBuildLocationFree()` 与调用处的 105；吸附网格在 `AFarmPlayerController::UpdateGhost()`（100） |
| 交互半径 | `AFarmPlayerController::UpdateTarget()` 的 `BestDistance = 320` |
| 野猪速度 / 伤害 / 范围 | `AFarmAnimal::Tick()` 里的 700 / 260 / 210 / 1.6 / 8 |
| 玩家攻击 | `AFarmPlayerController::Attack()` 里的 34 / 0.45 / 320 / 0.1 |
| 初始物资 | `UFarmInventoryComponent::ResetForNewGame()` |
| 存档字段 | `FarmSaveGame.h` + `AFarmGameMode::SaveGame()` / `LoadGame()`（三处要一起改） |
| 按键 | `Config/DefaultInput.ini` 的 ActionMappings + `AFarmPlayerController::SetupInputComponent()` |
