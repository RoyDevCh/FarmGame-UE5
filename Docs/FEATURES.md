# FarmGame UE5 功能说明（v3）

本文按系统记录**当前代码真实实现**的内容：玩法、按键、数值、状态机（对应 2026-11-09 的源码；之后若代码再改，以源码为准）。
所有数值都来自源码常量与表，标注了出处；按键以 `Config/DefaultInput.ini` 的 ActionMappings 为准。
文末"已知行为边界"列出了实现细节里容易误解的地方。

## 0. 系统与代码入口

| 系统 | 主要代码 |
|---|---|
| 世界生成、日夜、天气、目标、存读档 | `AFarmGameMode`（`FarmGameMode.cpp`） |
| 地块与生长 | `AFarmPlot`（`FarmPlot.cpp`）+ `FarmLogic`（`FarmTypes.h`） |
| 背包、金钱、买卖、制作结算、箱子库存 | `UFarmInventoryComponent` |
| 输入、交互目标、建造、制作、攻击、菜单/商店/设置 | `AFarmPlayerController` |
| 角色、HP、死亡回城、手持工具 | `AFarmCharacter` |
| 发货箱 | `AFarmBin` |
| 商店（售卖亭） | `AFarmShop` + `SFarmShopWidget` |
| 开始/暂停菜单、设置面板 | `SFarmMenuWidget` / `SFarmSettingsWidget` |
| 动物 | `AFarmAnimal` |
| 采集点与工具倍率 | `AFarmResourceNode` + `FarmLogic::GatherYield` |
| 建筑（栅栏 / 熔炉 / 储物箱） | `AFarmBuilding` |
| 传送门 | `AFarmPortal` |
| HUD | `AFarmHUD::DrawHUD`（Canvas） |
| Slate 面板 | `SFarmInventoryWidget` / `SFarmMenuWidget` / `SFarmShopWidget` / `SFarmSettingsWidget` |
| 数据表 | `FarmItems::All`（27 项）、`FarmRecipes::All`（9 条）、`FarmCrops::MakeDefaults()`（5 种作物）、`FarmQuests::MakeDefault()`（8 段目标） |

## 1. 地图与初始布局

全工程只有一张地图 `/Game/Maps/Farm`（存在 `Content/Maps/Farm.umap`）。关卡里只放了 `FarmLandscape`
（5×5 组件、316×316、extent 15750，高度图 `Saved/Heightmaps/farm.raw`）和 `/Game/Foliage/GrassType` 的 480 个实例；
其余 Actor 都由 `AFarmGameMode::BeginPlay()` → `BuildWorld()` 用 `/Engine/BasicShapes` 现场生成，
`SpawnEnvironment()` 另外生成定向光、SkyAtmosphere、SkyLight、点光补光、指数雾与后期体积。
没有 Landscape 时才生成地面 Plane（缩放 140）。

| 对象 | 数量 | 位置（cm） | 说明 |
|---|---|---|---|
| 地面 | 0/1 | 原点 | 有 `FarmLandscape` 时跳过；否则 Plane 缩放 140，深绿动态材质 |
| 农田地块 | 36 | X、Y ∈ [-375, 375]，间距 150 | 6×6，中心在原点 |
| 发货箱 | 1 | (0, 620) | 卖货 + 买种子 |
| 售卖亭（商店） | 1 | (320, 1000) | `E` 打开商店；水井在 (700, 1000) |
| 农场传送门 | 1 | (700, 700) | 目标：矿洞 (2250, 2250, 140) |
| 矿洞传送门 | 1 | (2600, 2600) | 目标：农场 (700, 700, 140) |
| 矿脉 | 6 | 矿洞中心周边（最远 420） | 每个 4 单位矿 |
| 树（可采） | 5 | 农场外围 ±1600 内 | 每个 3 单位木 |
| 石头（可采） | 5 | 农场外围 | 每个 3 单位石 |
| 鸡 | 2 | (-700, 300)、(-780, 400) | 畜栏 |
| 牛 | 1 | (-620, 520) | 畜栏 |
| 野猪 | 1 | (1400, -200) | 农场东侧 |
| 装饰树 | 11 | 外围 | Tree1/2/3、枯树、棕榈 ×3、松树 ×3、仙人掌；只有视觉，不能交互 |
| 玩家出生点 | — | (0, -520, 140) | `SpawnDefaultPawnAtTransform` 覆写 |

相机是固定俯视：弹簧臂长 1400、俯角 -58°，不跟随鼠标（`bUsePawnControlRotation = false`）。移动速度 520。

## 2. 农田与作物

### 2.1 地块状态机

枚举 `EFarmPlotState`：`Untilled` / `Tilled` / `Planted` / `Harvestable`。

| 当前状态 | 触发 | 结果 |
|---|---|---|
| Untilled | `E` | → Tilled（翻土，无消耗） |
| Tilled | `E`，背包有当前选中作物的种子 | 扣 1 颗种子；记 `CropId`、`Stage = 0`、`LastGrowthDay = 当前天`、`bWatered = false` → Planted |
| Tilled | `E`，没有种子 | 状态不变；Toast 提示去发货箱按 `B` 买 |
| Planted | `E`，未浇水 | `bWatered = true`（土色变深） |
| Planted | `E`，已浇水 | 无变化，仅提示 `Growing... stage i/n` |
| Planted | 跨天结算 | 见 2.2；长满则 → Harvestable |
| Harvestable | `E` | 背包 + `YieldPerHarvest` 个作物；番茄 `bRegrowAfterHarvest` → 回 `Planted`、`Stage = 1`、需重新浇水；其余清空作物、`Stage = 0` → Tilled |

每次状态变化都会 `RefreshVisuals()`：土块按状态换色；作物是 Cylinder，阶段越高越高越粗（高度 0.18 → 1.15），
成熟时颜色向金黄插值 55%。

### 2.2 生长算法

入口 `AFarmPlot::AdvanceDay(Today)` → `FarmLogic::AdvanceCrop`（`FarmTypes.h`，纯函数、无 UObject 依赖，测试直接调它）：

    if (State != Planted || !CropDef) return;          // 只有已播种的地块参与
    if (!wasWateredToday)      { bWatered = false; return; }   // 没水不长
    if (IsMature(Stage))       { bWatered = false; return; }   // 成熟后不再推进
    if (Today - LastGrowthDay < DaysPerStage) return;          // 天数不够，水保留
    Stage += 1;  LastGrowthDay = Today;  bWatered = false;
    if (IsMature(Stage)) State = Harvestable;

要点：

- **浇水是唯一门槛**：没浇水的当天不生长，且浇水标记被清掉，第二天要重新浇。
- **天数不够时浇水不清空**：土豆 `DaysPerStage = 2`，第 1 天浇一次，第 2 天跨天不长但水还在，第 3 天跨天生长。所以"每阶段 2 天"不等于"要浇 2 次水"。
- **跨天是结算点**：`AdvanceDay` 由 `AFarmGameMode::AdvanceDay()` 在自然过天或睡觉时对全部地块调用。
- 成熟后停在最后阶段（`Stage = Stages - 1`），不会自然消失，等玩家收获。

### 2.3 作物数据

`FCropDef` 由 `FarmCrops::MakeDefaults()`（`FarmCropCatalog.h`）返回，`AFarmGameMode` 构造函数调用它；`Stages` 含幼苗阶段（0 是刚种下）。

| 作物 | Id | 阶段数 | 每阶段天数 | 种子价 | 售价 | 单次收获 | 从播种到成熟 | 颜色 |
|---|---|---|---|---|---|---|---|---|
| 胡萝卜 | `carrot` | 3 | 1 | 3 | 8 | 1 | 2 天 | (0.95, 0.45, 0.08) 橙 |
| 土豆 | `potato` | 3 | 2 | 5 | 14 | 2 | 4 天 | (0.75, 0.62, 0.35) 土黄 |
| 玉米 | `corn` | 4 | 1 | 6 | 12 | 2 | 3 天 | (0.95, 0.85, 0.2) 黄 |
| 番茄 | `tomato` | 3 | 2 | 7 | 18 | 2 | 4 天 | (0.85, 0.18, 0.12) 红 |
| 南瓜 | `pumpkin` | 4 | 2 | 10 | 26 | 1 | 6 天 | (0.9, 0.55, 0.1) 橙 |

"从播种到成熟" = `(Stages - 1) × DaysPerStage`，即需要跨几次天。
番茄是唯一 `bRegrowAfterHarvest = true` 的作物：收获后不清地，回到阶段 1 继续长（要重新浇水），可以反复收。

### 2.4 按键

| 按键 | 作用 |
|---|---|
| `E` | 按状态分派：翻土 / 播种 / 浇水 / 收获 |
| `Q` | 切换当前种子（循环 `Crops` 数组），Toast 显示选中作物 |

## 3. 时间、天气与光照

### 3.1 一天

- `DayLengthSeconds = 180`（`AFarmGameMode` 成员默认值），`AFarmGameMode::Tick` 每帧累加 `DayTimer`。
- 计时到 180 秒：`DayTimer = 0`，调 `AdvanceDay()`：`Day += 1` → 掷天气 → 雨浇地 → 所有地块生长 → 采集点复活 → 动物产出。
- 时钟显示：`GetTimeLabel()` 把 0..1 的进度映射到 06:00 – 24:00（`6h + P × 18h`），走到 24:00 即换天。
- 第 1 天的起点被特意往后挪了：`AFarmGameMode::BeginPlay()` 里 `DayTimer = DayLengthSeconds * (3/18)`，所以开局是 **09:00**（保证一进游戏就是白天）；之后每天换天时 `DayTimer` 归零，从 06:00 开始。

### 3.2 睡觉（`T`）

`AFarmPlayerController::Sleep()` 直接调 `AFarmGameMode::AdvanceDay()`，Toast 显示 `day N -> N+1`。
它**不重置 `DayTimer`**，见"已知行为边界"。

### 3.3 天气

- 每天切换时掷一次：`bRaining = FMath::RandRange(0, 3) == 0`，即 25% 下雨。
- 下雨当天，所有 `Planted` 且未浇水的地块被 `WaterFromRain()` 免费浇水（`Tilled` / `Harvestable` 不受影响）。
- HUD 右上角显示 `Sunny` / `Rainy`，同时影响定向光强度与色温。
- 只有晴/雨两态，没有雨天对作物生长的额外加成，也没有雨滴粒子。

### 3.4 昼夜光照

`AFarmGameMode::Tick` 里每帧按进度 `P` 更新定向光：

| 参数 | 计算 |
|---|---|
| 俯仰角 | `Pitch = -14 - 70 × sin(P × π)`（正午最高、首尾最低） |
| 偏航角 | 固定 35° |
| 强度（晴） | 5.0，`P > 0.8` 时线性衰减到 35%，下限 1.1 |
| 强度（雨） | 1.9，同样受 `P > 0.8` 衰减，下限 1.1 |
| 颜色（晴） | (1.0, 0.95, 0.86) 暖白 |
| 颜色（雨） | (0.65, 0.72, 0.85) 冷灰蓝 |

环境光：SkyLight 强度 0.9、开启实时捕获；另生成一个 SkyAtmosphere 做天空。

## 4. 经济与物品

### 4.1 初始背包（`UFarmInventoryComponent::ResetForNewGame`）

| 项目 | 数量 |
|---|---|
| 金币 | 20 |
| `carrot_seed` | 3 |
| `corn_seed` | 1 |
| `wood` | 6 |
| `hay` | 1 |

新档只会初始化一次（`AFarmGameMode::bPlayerInitialized`）；之后读档会用存档覆盖。

### 4.2 物品表（`FarmItems::All`，27 项）

类别 `EItemKind`：Seed / Produce / Resource / AnimalProduct / Material / **Food**。

| Id | 名称 | 类别 | 买价 | 卖价 | 来源 → 用途 |
|---|---|---|---|---|---|
| `carrot_seed` | Carrot Seed | Seed | 3 | 1 | 发货箱 / 商店 → 种胡萝卜 |
| `potato_seed` | Potato Seed | Seed | 5 | 2 | 发货箱 / 商店 → 种土豆 |
| `corn_seed` | Corn Seed | Seed | 6 | 2 | 发货箱 / 商店 → 种玉米 |
| `tomato_seed` | Tomato Seed | Seed | 7 | 3 | 发货箱 / 商店 → 种番茄 |
| `pumpkin_seed` | Pumpkin Seed | Seed | 10 | 4 | 发货箱 / 商店 → 种南瓜 |
| `carrot` | Carrot | Produce | — | 8 | 收获 → 卖 / 任务 |
| `potato` | Potato | Produce | — | 14 | 收获 → 卖 |
| `corn` | Corn | Produce | — | 12 | 收获 → 卖 / 做干草 / 任务 |
| `tomato` | Tomato | Produce | — | 18 | 收获 → 卖（可重复结果） |
| `pumpkin` | Pumpkin | Produce | — | 26 | 收获 → 卖 |
| `wood` | Wood | Resource | 2 | 1 | 砍树 / 熔炉装填 / 做套件 |
| `stone` | Stone | Resource | 3 | 1 | 采石 → 熔炉套件 |
| `ore` | Ore | Resource | — | 4 | 矿脉 → 熔炉 / 工具 |
| `ingot` | Iron Ingot | Material | — | 18 | 熔炉 → 卖 / 任务 |
| `hay` | Hay | Resource | 4 | 1 | 玉米制作 → 喂动物 |
| `egg` | Egg | AnimalProduct | — | 10 | 鸡 → 卖 / `H` 吃 +5 |
| `milk` | Milk | AnimalProduct | — | 16 | 牛 → 卖 / `H` 吃 +8 |
| `meat` | Meat | AnimalProduct | — | 12 | 动物死亡 → 卖 / `H` 吃 +15 |
| `fence_kit` | Fence Kit | Material | — | 0 | 制作 → 建造栅栏 |
| `furnace_kit` | Furnace Kit | Material | — | 0 | 制作 → 建造熔炉 |
| `chest_kit` | Chest Kit | Material | — | 0 | 制作 → 建造储物箱 |
| `axe` | Axe | Material | — | 0 | 制作 → 采集 ×2 / 钢斧材料 |
| `pickaxe` | Pickaxe | Material | — | 0 | 制作 → 采矿 ×2 / 钢镐材料 |
| `steel_axe` | Steel Axe | Material | — | 0 | 制作 → 采集 ×3 / 任务 |
| `steel_pickaxe` | Steel Pickaxe | Material | — | 0 | 制作 → 采矿 ×3 |
| `apple` | Apple | Food | 5 | 3 | 商店 → `H` 吃 +10 / 任务 |
| `bread` | Bread | Food | 8 | 4 | 商店 → `H` 吃 +20 |

注意：

- 表里有买价的只有种子、wood / stone / hay 和两种食物；**发货箱的 `B` 只买当前种子**，其余购买走商店。
- `AFarmPlayerController::GetShopStock()` 是一张**独立的商店价目表**，不读 `FItemDef::BuyPrice`：
  例如 ore 在物品表买价为 0、商店卖 8；stone 在物品表买价 3、商店卖 2。改价格要改两处。
- 工具本身卖价为 0，不会被动卖货带走。

### 4.3 买：发货箱 + `B`

`AFarmPlayerController::BuySeed()`：当前交互目标是发货箱时，按 `B` 买 1 颗**当前选中作物**的种子。
扣款走 `UFarmInventoryComponent::BuyItem()`，价格是该种子在物品表里的 `BuyPrice`（当前与 `FCropDef::SeedCost` 相同）；余额不足则 Toast `Not enough gold`。

### 4.4 卖：发货箱 + `E`

`UFarmInventoryComponent::SellEverything()` 一次清空所有可卖物品：

- 卖：`Produce`、`AnimalProduct`、`Material`、**`Food`**（且 `SellPrice > 0`）—— 即 5 种作物、蛋/奶/肉、铁锭、苹果/面包。
- 不卖：`Seed`（种子留着下次种）、`Resource`（木/石/矿/干草是建材与燃料）。
- 价格为 0 的套件与工具（`fence_kit` / `furnace_kit` / `chest_kit` / 斧镐等）不会被卖掉。
- 返回本次总收入，Toast `Sold goods for +N gold`；没东西可卖时提示 `Nothing to sell`。

发货箱提示会实时算出可卖件数（`GetSellableCount`）。

### 4.5 商店：村口售卖亭（`AFarmShop` + `SFarmShopWidget`）

| 项目 | 内容 |
|---|---|
| 位置 | (320, 1000)，水井 (700, 1000) 旁边；模型 `/Game/Art/Props/Selling_Kiosk`，高度 260 |
| 打开 | 走近按 `E`（提示 `E: Open shop (seeds / hay / ore)`）；面板打开时 `SetGamePaused(true)` |
| 关闭 | 面板 `Close (Esc)` 按钮或 `Esc`；关闭后解除暂停 |
| 货架 | `GetShopStock()` 11 项：5 种种子 3/5/6/7/10、干草 4、矿石 8、木材 2、石头 2、苹果 5、面包 8 |
| 买 | 每行 `Buy` 按钮；金币不足时按钮禁用（`IsEnabled(bAfford)`）；成功扣钱、加 1 个、播放 craft 音 |
| 卖 | 底部 `Sell all goods` 按钮调 `SellEverything()`，Toast 显示收入或 `Nothing to sell` |
| 刷新 | 每次买卖后重建行与金币文本（`Refresh()`），价格与金币实时更新 |

细节：

- 购买会调用 `AFarmGameMode::NotifyProduced(ItemId, 1)`，所以"在商店买苹果"这一步目标能正常推进。
- 商店价格来自静态数组，不读物品表买价（见 4.2 的注意）；也没有数量选择、分类页与库存上限。
- 面板是 `FInputModeUIOnly` + 鼠标光标；打开期间玩家输入被 Slate 接管。

### 4.6 食物与进食（`H`）

| 项目 | 内容 |
|---|---|
| 物品 | `apple`（买 5 / 卖 3，+10 HP）、`bread`（买 8 / 卖 4，+20 HP），类别 `EItemKind::Food` |
| 按键 | `H` → `AFarmPlayerController::EatFood()` |
| 优先级 | 按固定顺序找**第一个背包里有的**：bread(+20) → apple(+10) → meat(+15) → milk(+8) → egg(+5) |
| 效果 | 扣 1 个、`AFarmCharacter::Heal(N)`（clamp 到 0..100）、Toast `Ate X (+N HP)`、播放 `pop` |
| 失败 | 什么都没有时 Toast `No food in the bag` |

注意：

- 优先级**不是按回血量排序**：有面包就一定先吃面包，哪怕只差 1 点血；满血时按 `H` 也会吃掉一个（没有"满血不吃"的判断）。
- `meat` / `milk` / `egg` 的类别是 `AnimalProduct`，但同样能被 `H` 吃；`Food` 类别目前只有苹果与面包。
- 没有饥饿/口渴/体力条，食物只用于回血。

## 5. 制作

### 5.1 配方（`FarmRecipes::All`，9 条）

| # | 配方 Id | 产物 | 消耗 | 需要熔炉 |
|---|---|---|---|---|
| 1 | `fence_kit` | 栅栏套件 ×1 | wood ×5 | 否 |
| 2 | `furnace_kit` | 熔炉套件 ×1 | stone ×10 + wood ×5 | 否 |
| 3 | `chest_kit` | 储物箱套件 ×1 | wood ×12 | 否 |
| 4 | `axe` | 斧 ×1 | wood ×4 + ore ×1 | 否 |
| 5 | `pickaxe` | 镐 ×1 | wood ×4 + ore ×2 | 否 |
| 6 | `steel_axe` | 钢斧 ×1 | axe ×1 + ore ×5 | 否 |
| 7 | `steel_pickaxe` | 钢镐 ×1 | pickaxe ×1 + ore ×5 | 否 |
| 8 | `ingot` | 铁锭 ×1 | ore ×2 | **是**（站在熔炉 320 内） |
| 9 | `hay_bundle` | 干草 ×2 | corn ×2 | 否 |

### 5.2 交互

| 按键 | 作用 |
|---|---|
| `C` | 打开 / 关闭制作菜单（不暂停游戏） |
| `1`–`5` | 结算第 1–5 条配方（栅栏 / 熔炉 / 箱子 / 斧 / 镐） |
| `Tab` 面板 | 列出全部 9 条配方，点按钮可做第 6–9 条（钢斧 / 钢镐 / 铁锭 / 干草） |
| `Esc` | 关闭菜单（优先于退出建造模式） |

判定顺序（`AFarmPlayerController::CraftRecipe`）：先在 320 内找熔炉（`bNeedsFurnace` 的配方）→ 再 `CanCraft` 检查材料 →
扣材料、加产物，并调 `NotifyProduced` 推进目标链。失败分别提示 `Stand next to a furnace to smelt` 与 `Missing ingredients`。
`C` 菜单里能做的配方显示绿色、不能做的灰色（`CanCraftRecipe`）；`Tab` 面板里不能做的按钮直接禁用。
数字键不受菜单开关限制：没开菜单按 `1`–`5` 也会制作。

### 5.3 两条做铁锭的路

代码里铁锭有两个来源，都真实可用：

1. **配方 8**：在 `Tab` 面板点 `Smelt Iron Ingot (at furnace)`（没有数字键），消耗 2 矿直接得 1 锭（要求站在熔炉旁）。
2. **熔炉装填**：`E` 给熔炉装 1 矿 + 1 木，8 秒后出 1 锭（见第 7 节）。

## 6. 建造

### 6.1 模式状态机（`G`）

    off (-1) --G--> Fence (0) --G--> Furnace (1) --G--> Chest (2) --G--> off (-1)
    任意状态 --Esc--> off

进入/切换模式会 Toast 当前类型，HUD 底部中央显示 `BUILD MODE: Fence/Furnace/Chest`。

### 6.2 预览

- 进入模式时生成一个"鬼影" `AFarmBuilding`：关碰撞、关 Tick、`Configure` 成当前类型。
- 每帧（`UpdateGhost`）放在角色前方 220 处，X/Y 各自按 100 取整（吸附 100 网格），Z 归零。
- 退出模式时销毁鬼影。

### 6.3 放置（`E`）

`AFarmPlayerController::PlaceBuilding()` 的判定顺序：

1. 背包里有对应套件？没有 → `Need X (craft it with C)`。
2. 位置空不空？`AFarmGameMode::IsBuildLocationFree(位置, 105)`：
   - 离任何地块 < 105 → 拒绝
   - 离任何已有建筑 < 105 × 1.4 = 147 → 拒绝
   - 离发货箱 < 105 × 0.8 = 84 → 拒绝
   - 不检查采集点、动物、传送门、商店
3. 通过则扣 1 个套件、`SpawnBuilding` 生成建筑并注册进 `AFarmGameMode::Buildings`。

### 6.4 建筑种类

| 类型 | 交互 | 说明 |
|---|---|---|
| `Fence` | 无（提示 `Fence`） | `fence_long` 模型，高 110；回退是 1.6 × 0.15 × 1.0 木色方块 |
| `Furnace` | `E` 装填 / 取锭 | `furnace` 模型，高 170；回退是 1.4 × 1.4 × 1.2 石色方块 + 圆柱烟囱 |
| `Chest` | `E` 存入 / 取出 | `Chest_stone` 模型，高 95；只收 `Resource`（木/石/矿/干草），不是通用容器 |

### 6.5 储物箱（`Chest`）

- 空箱按 `E` 把背包里**全部 Resource** 一次存进去（`DepositResources`），Toast `Stored N resources`；没资源时 `No resources to store`。
- 箱内有货时按 `E` 全部取回（`WithdrawResources`），Toast `Took out N resources`。
- 库存是 `TMap<FName,int32>`，随存档保存（`FBuildingSaveData::Storage`）；不占容量、不分类。

## 7. 熔炉冶炼

### 7.1 状态

| 状态 | 进入条件 | `E` 的效果 | 提示 |
|---|---|---|---|
| 空 | `OreCount = 0`、`IngotCount = 0` | 扣 1 矿 + 1 木 → `OreCount = 1` | `E: Load ore + wood (max 5)` |
| 冶炼中 | `OreCount > 0` | 继续装填（直到 5） | `Furnace: smelting N ore (8s each)` |
| 有产出 | `IngotCount > 0` | 一次性取走全部铁锭 | `E: Collect N iron ingot(s)` |
| 满仓 | `OreCount >= 5` | 拒绝，Toast `Furnace is full` | 同上 |

装填时如果矿石不够会提示 `Need 1 ore and 1 wood`。

### 7.2 数值

| 参数 | 值 | 出处 |
|---|---|---|
| 同时容纳矿石 | 5 | `AFarmBuilding::MaxOre()` |
| 每块矿石耗时 | 8 秒 | `AFarmBuilding::SmeltSecondsPerOre()` |
| 换算 | 1 矿 → 1 锭 | `FarmLogic::SmeltStep` |
| 装填成本 | 1 矿 + 1 木 | `AFarmBuilding::Interact` |

冶炼在 `AFarmBuilding::Tick` 里跑；`FarmLogic::SmeltStep` 用累加计时器，每满 8 秒扣 1 矿、加 1 锭，余数保留到下一块。

## 8. 采集

采集点 `AFarmResourceNode` 三种类型：

| 类型 | 产出 | 数量 | 每天复活 |
|---|---|---|---|
| `Tree` | wood ×1（工具 ×2 / 钢制 ×3） | 5 个点 × 3 | 是 |
| `Rock` | stone ×1（工具 ×2 / 钢制 ×3） | 5 个点 × 3 | 是 |
| `Ore` | ore ×1（工具 ×2 / 钢制 ×3） | 6 个点 × 4 | 是 |

规则：

- `E` 采一次扣 1 次剩余量，产出 `1/2/3`（空手/工具/钢制，见 8.1），Toast `+N Wood/Stone/Ore`。
- 采光后 Actor 隐藏，提示 `Depleted - regrows tomorrow`。
- 每天 `AdvanceDay` 里 `Regrow()`：数量回满并重新显示。
- 农场外围另有装饰树（`Tree1` / `Tree2` / `Tree3` / `TreeDead` 等 `AStaticMeshActor`），只有视觉，不参与采集。

### 8.1 工具与产出倍率

工具是普通物品（`EItemKind::Material`，卖价 0），靠制作得到：

| 工具 | 配方 | 采集倍率 | 适用采集点 |
|---|---|---|---|
| （空手） | — | ×1 | 树 / 石头 / 矿 |
| `axe` | wood ×4 + ore ×1 | ×2 | 树 |
| `pickaxe` | wood ×4 + ore ×2 | ×2 | 石头 / 矿脉 |
| `steel_axe` | `axe` ×1 + ore ×5 | ×3 | 树 |
| `steel_pickaxe` | `pickaxe` ×1 + ore ×5 | ×3 | 石头 / 矿脉 |

判定在 `AFarmResourceNode::Interact` 里每次交互现算：

    Tree:      steel_axe ? 2 : (axe ? 1 : 0)
    Rock/Ore:  steel_pickaxe ? 2 : (pickaxe ? 1 : 0)
    Amount = FarmLogic::GatherYield(Tier) = 1 + clamp(Tier, 0, 2)

要点：

- 只看"背包里有没有"，不消耗工具、没有耐久；做出钢斧后就一直按 ×3 结算。
- 采集点剩余次数每次 `E` 固定 -1，**不会**因为倍率多扣；所以 3 次量的树，钢斧一次就能拿 3 木并让树消失。
- Toast 会带 `(tool)` 后缀（Tier > 0），提示 `+3 Wood (tool)`。

### 8.2 手持工具外观

`AFarmCharacter::Tick` 每帧调 `UpdateToolVisual()`：

| 条件 | 手上模型 |
|---|---|
| 背包有 `steel_axe` 或 `axe` | `/Game/Art/Props/Axe` |
| 否则有 `steel_pickaxe` 或 `pickaxe` | `/Game/Art/Props/Pickaxe` |
| 都没有 | 隐藏 |

- `ToolMesh` 在 `BeginPlay` 里挂到骨骼 `hand_R`（`SnapToTargetNotIncludingScale`）。
- 缩放按"模型高度 = 45 cm"统一算；相对位置与旋转被清零，握持角度没有逐模型校准（见第 15 节）。
- 钢制工具没有独立美术，复用普通斧/镐模型。

## 9. 动物

### 9.1 家畜：鸡 / 牛

状态机（每只动物各自一份）：

    Hungry --E(有干草)--> Fed --跨天--> ProductReady --E--> Hungry
    Hungry --E(没干草)--> Hungry（Toast: Need hay (craft from corn)）
    Fed    --E--> 无变化（提示 Well fed - product ready tomorrow）

| 类型 | 数量 | HP | 产出 | 产出物卖价 |
|---|---|---|---|---|
| Chicken | 2 | 30 | `egg` ×1 | 10 |
| Cow | 1 | 70 | `milk` ×1 | 16 |

- 喂食消耗 1 个 `hay`；干草由玉米做（配方 9，`hay_bundle`：2 玉米 → 2 干草），初始背包也有 1 个。
- `AdvanceDay` 时：`bFed` 为真 → `bProductReady = true`、`bFed = false`。也就是"今天喂，明天收"。
- 产出只累积 1 份（布尔标记），不叠层；不收就一直留着。
- 移动：每 2–5 秒换一个随机方向，速度 60，离出生点超过 450 会被拉回。

### 9.2 野猪

- `IsHostile()` 由类型决定，只有 Boar 为真。
- 玩家在 700 内：以 260 的速度直线追击；在 210 内每 1.6 秒造成 8 点伤害。
- 不会反击式撤退，也不会攻击动物或建筑；只追玩家。
- 与家畜一样有 60 HP，但不会自然回血。

### 9.3 死亡掉落

`AFarmAnimal::ApplyDamage`：HP 归零 → 给攻击者 meat（野猪 2 个、其它 1 个）→ `Destroy()`。
**注意**：死亡逻辑不区分敌友，打死自家鸡 / 牛同样给 1 个 meat。

## 10. 战斗与生命

### 10.1 玩家攻击（鼠标左键 / `F`）

| 参数 | 值 |
|---|---|
| 冷却 | 0.45 秒 |
| 伤害 | 34 |
| 距离 | 320（2D 距离） |
| 朝向 | **不检查**：背对目标也能打到 |
| 选目标 | 范围内最近的活着的动物 |

野猪 60 HP → 2 下打死；鸡 30 HP → 1 下；牛 70 HP → 3 下。
`F` 与鼠标左键绑同一个 `Attack` action（`Config/DefaultInput.ini`），没打中目标也会播挥击动画与音效。

### 10.2 玩家生命

- 初始与上限都是 100（`AFarmCharacter::Health / MaxHealth`）。
- 受伤 `ApplyDamage`，HP 归零时 `Respawn()`：传送回出生点、回满血、Toast `You passed out... back at the farm`。
- 没有无敌帧，也没有死亡结算界面（不扣钱、不丢东西）。
- HUD 左下角是 HP 条（240 × 16 像素，按比例填充）+ `HP n / 100` 文本。

## 11. 传送门

| 传送门 | 位置 | 目标 |
|---|---|---|
| Mine | (700, 700) | (2250, 2250, 140) —— 矿洞空地 |
| Farm | (2600, 2600) | (700, 700, 140) |

- `E` 交互，`SetActorLocation(..., ETeleportType::TeleportPhysics)`，Toast `Travelled to Mine/Farm`。
- HUD 提示 `E: Travel to Mine` 之类的固定文案。
- 矿洞是同一张地图上的一块空地，不是独立关卡：世界其它部分（农场、畜栏）一直存在，只是离得远。

## 12. 开始/暂停菜单与存档

### 12.1 开始/暂停菜单（`SFarmMenuWidget`）

`AFarmPlayerController::BeginPlay()` 创建菜单控件并 `ShowMenu(true)`：`SetGamePaused(true)`、`FInputModeUIOnly`、显示鼠标。
菜单打开期间 `AFarmGameMode::Tick` 提前返回，所以**天数计时、太阳角度、天气切换都停住**（动物漫游与熔炉 Tick 不受影响，见第 15 节）。

| 元素 | 开始菜单 | 暂停菜单 |
|---|---|---|
| 标题 | `FARM GAME` | `PAUSED` |
| 主按钮 | `Continue`（当前槽无存档时禁用） | `Resume`（始终可用） |
| `New game` | 显示；重载地图并带 `?autostart=1` | 隐藏（但 `N` 键仍可触发） |
| `Save slot: N` | 显示 `(empty)` / `(has save)`，点击循环 1→2→3→1 | 同左 |
| `Settings` | 打开设置面板 | 同左 |
| `Quit` | `QuitGame` | 同左 |

按钮行为（`AFarmPlayerController`）：

- `HandleMenuPrimary()`：开始时若当前槽有存档 → `LoadGame(pawn)` + `StartGame()` + 隐藏菜单；**没有存档时**走 `NewGame()`。
  暂停使用引擎级 `UGameplayStatics::SetGamePaused`，动物/建筑/日光等全部 Actor 一并冻结；`HideMenu()` 恢复。设置面板从暂停菜单打开，`Back (Esc)` 会回到暂停菜单而不是直接继续游戏。
- `HandleMenuNewGame()`：`AFarmGameMode::NewGame()` 重新 `OpenLevel` 当前地图，选项 `autostart=1`；
  新 GameMode 在 `BeginPlay` 看到该选项就直接 `StartGame()`，玩家控制器用 `ConsumeSkipMenu()` 跳过菜单。
- `HandleMenuCycleSlot()`：`SaveSlot = (SaveSlot % 3) + 1`，立即刷新 `(empty)/(has save)`。
- `HandleMenuQuit()`：`UKismetSystemLibrary::QuitGame`（PIE 里退出 PIE）。

键盘：

| 按键 | 作用 |
|---|---|
| `C` / `Enter` / `Space` | 主按钮（Continue / Resume） |
| `N` | 新游戏（暂停菜单里按钮隐藏，但键仍有效） |
| `S` | 循环存档槽 |
| `Q` | 退出游戏 |
| `Esc` | 暂停菜单：关闭并继续；开始菜单：无动作（`HandleMenuEscape` 只在非开始模式生效） |

注意：主按钮的禁用状态只拦鼠标点击，`C` / `Enter` 仍会走 `HandleMenuPrimary()`；开始菜单空槽时按 `C` 等于新游戏。

### 12.2 三个存档槽

| 项目 | 内容 |
|---|---|
| 槽位 | `FarmSlot1` / `FarmSlot2` / `FarmSlot3`（`AFarmGameMode::SaveSlotIndex`，默认 1，`SetSaveSlot` clamp 到 1..3） |
| 选择 | 只在开始/暂停菜单按 `S`（或点 `Save slot`）切换；**不进存档**，每次启动/新游戏都回到槽 1 |
| 文件 | `Saved/SaveGames/FarmSlot1.sav` ~ `FarmSlot3.sav`（`UserIndex = 0`） |
| 判定 | `HasSaveInSlot(Slot)` → `DoesSaveGameExist("FarmSlotN", 0)`；菜单只显示 `(empty)` / `(has save)`，不显示天数或时间 |
| 存/读 | `Ctrl+S` / `Ctrl+L` 永远作用于当前槽；没有自动存档、没有槽位复制/删除 |

### 12.3 入口与数据载体

- `Ctrl+S` 保存，`Ctrl+L` 读取，槽位名 `FarmSlotN`、用户索引 0。
- 数据载体 `UFarmSaveGame : USaveGame`，字段见 12.4。

### 12.4 存了什么

| 字段 | 内容 |
|---|---|
| `Day` | 天数 |
| `DayTimer` | 当天已过的时间（秒），读档后从原处继续 |
| `bRaining` | 当天是否下雨 |
| `Gold` | 金币 |
| `Items` | 全部物品 `TMap<FName, int32>` |
| `PlayerHealth` | 玩家 HP |
| `Plots` | 36 条 `FPlotSaveData`：索引、状态、作物 Id、阶段、是否浇水、上次生长日 |
| `Buildings` | 每座建筑：类型、位置、矿石数、铁锭数、箱子库存 `Storage` |
| `Animals` | 每只动物：类型、位置、是否已喂、是否有产物、HP |

### 12.5 读档做了什么

1. 覆盖天数、`DayTimer`、天气、金币、物品、HP。
2. 按索引恢复地块（作物指针用 `FCropDef` 目录重新查一次）。
3. **销毁并重建**全部建筑 Actor，再 `RestoreState` 回填矿石/铁锭、`RestoreStorage` 回填箱子库存。
4. **销毁并重建**全部动物 Actor，再 `RestoreState` 回填喂养/产物/HP（HP 会被 clamp 到至少 1）。
5. 恢复目标链：`Quests` 与 `ActiveQuestIndex` 从存档回填（旧存档没有这两项时保持当前进度）。
6. 找不到存档时 Toast `No save found`。

### 12.6 不保存的内容

玩家位置（始终在出生点或读档时当前位置）、采集点剩余量与隐藏状态、当前选中的种子、太阳/相机状态（每帧重算）、
Toast 与 UI 状态、野猪的巡逻位置（动物位置会存，但读档后从存的位置重新开始漫游）、

**当前存档槽**（每次启动/新游戏回到 `FarmSlot1`）、设置（全屏/画质走 `UGameUserSettings::ApplySettings`，代码没有调 `SaveSettings`，也不在存档里）。

## 13. HUD

常驻 HUD 由 `AFarmHUD::DrawHUD` 用 Canvas 立即模式绘制；`Tab` 背包/制作、开始/暂停菜单、商店、设置都是运行时构建的 Slate 控件（无 Widget 资产）：

| 位置 | 内容 |
|---|---|
| 左上 | `Day N      Gold N g` |
| 右上 | 时间 `HH:MM` + 天气 `Sunny/Rainy` |
| 左下 | HP 条 + `HP n / 100` |
| 左侧 34% 高度 | 目标行 `Objective: <标题>  (进度/目标)   +Ng`；全部完成后 `Objective: all complete` |
| 左中 | `Selected seed: X (Q to change)` |
| 左中 | 每作物一行：种子数、收获数、种子价、售价 |
| 左中 | `Wood / Stone / Ore / Ingot / Hay` 计数 |
| 左中 | `Kits: fence N   furnace N`（箱子套件不上 HUD，只在 `Tab` 面板里看） |
| 底部中央 | 交互提示（随目标变化）；建造模式横幅 |
| 底部左 | 操作帮助行 |
| 底部 | 制作菜单（按 `C` 展开，逐条列出 9 条配方、材料、能否制作） |
| 顶部中央 | Toast（3.5 秒，最后 1.2 秒淡出） |

Slate 面板（层级从下到上）：`SFarmInventoryWidget`（Tab）→ `SFarmMenuWidget`（开始/暂停）→ `SFarmShopWidget`（商店）→ `SFarmSettingsWidget`（设置）。
菜单、商店、设置打开时都用 `FInputModeUIOnly` 抢焦点；背包用 `GameAndUI` 保留移动输入。

### 13.3 材质与贴图

地面、土壤、木材使用 `Content/Art/Textures` 下程序生成的可平铺噪声贴图（`T_FarmGrass` / `T_FarmDirt` / `T_FarmWood`），由 `M_FarmGrassTiled` 等材质按 UV 平铺采样；缺失时回退到纯色材质。

### 13.4 动画

角色与动物使用导入的 SkeletalMesh + AnimSequence，由代码直接 `PlayAnimation`（无 AnimBP）：

| 对象 | 动画 | 触发 |
|---|---|---|
| 角色 | `character_idle` / `character_run` / `character_attack` | 静止 / 移动 / 左键或 `F` 攻击 |
| 鸡 | `chicken_idle` / `chicken_walk` | 站立 / 行走 |
| 牛 | `cow_idle` / `cow_walk` | 站立 / 行走 |
| 野猪 | `bear_idle`（初始）/ `bear_attack`（受击） | 追击分支提前 return，不会切 `bear_run`；被玩家打中时播 attack |

### 13.5 音效

7 个原版 WAV 位于 `Content/Art/Audio`，由 `FarmAudio::Play` 在以下时机 2D 播放：

| 动作 | 资源 |
|---|---|
| 锄地 | `hit_wood` |
| 播种 / 收获 | `pop` |
| 浇水 / 挥击 | `fast_woosh` |
| 制作 / 卖货 / 收铁锭 | `craft` |
| 建造 | `build` |
| 装填熔炉 | `hit_stone` |
| 命中动物 | `flesh_hit` |

### 13.6 目标链（8 段）

`FarmQuests::MakeDefault()` 定义 8 段线性目标，`AFarmGameMode` 构造函数持有：

| # | 标题 | 匹配物品 | 目标 | 奖励 |
|---|---|---|---|---|
| 1 | Harvest 3 carrots | `carrot` | 3 | 20 g |
| 2 | Craft a furnace kit | `furnace_kit` | 1 | 30 g |
| 3 | Smelt an iron ingot | `ingot` | 1 | 40 g |
| 4 | Craft an axe | `axe` | 1 | 50 g |
| 5 | Harvest 5 corn | `corn` | 5 | 60 g |
| 6 | Smelt 3 iron ingots | `ingot` | 3 | 80 g |
| 7 | Buy an apple at the shop | `apple` | 1 | 30 g |
| 8 | Craft a steel axe | `steel_axe` | 1 | 100 g |

推进方式：

- 所有产出/购买都走 `AFarmGameMode::NotifyProduced(ItemId, Amount)`：收获地块、`CraftRecipe`、熔炉取锭、商店购买。
- **只匹配当前那一段**（`ActiveQuestIndex`）；提前攒下的物品不会补记。
- `FarmQuests::Advance` 把进度 cap 在 `Target`，返回"是否刚好完成"；完成后 `Inventory->AddGold(Reward)`、索引 +1、写日志。
- HUD 显示 `Objective: <标题>  (进度/目标)   +<奖励>g`；全部完成后显示 `Objective: all complete`。
- 第 3 段既可熔炉装填后取锭，也可在熔炉旁用配方 8 直接冶炼；第 6 段一次取 3 锭会一次推进 3 点。
- 第 7 段只认商店购买（`BuyFromShop`），在发货箱按 `B` 买种子不会触发。

注意：奖励只有金币且自动到账，没有奖励选择/领取界面；目标进度**不写进存档**（见 12.6）：`Ctrl+L` 保持内存里的当前段，重启后 Continue 从第 1 段开始。

## 14. 自动化测试（38 条）

全部是 `EAutomationTestFlags::EditorContext | EngineFilter` 的简单测试，前缀 `FarmGame.`。
基础 32 条在 `FarmGameTests.cpp`，补充 6 条在 `FarmGameExtraTests.cpp`，都包在 `#if WITH_DEV_AUTOMATION_TESTS` 里。

| 分组 | 条数 | 覆盖 |
|---|---|---|
| `FarmGame.CropGrowth` | 3 | 浇水推进、缺水不推进、成熟停住 |
| `FarmGame.Crops` | 2 | 5 种作物目录自洽；番茄收后回阶段 1、其余清空 |
| `FarmGame.Inventory` | 7 | 买卖、金币不足、卖货过滤、未知物品、空卖、箱子存取、扣钱原子性 |
| `FarmGame.Crafting` | 5 | 扣料产出、干草、熔炉计时、刚好用完材料、箱子配方成本 |
| `FarmGame.Items` | 2 | 目录一致性（胡萝卜售价 8 / SeedId）、Id 唯一 |
| `FarmGame.Recipes` | 3 | 材料在目录中、Id 唯一、全部产物在目录中 |
| `FarmGame.Save` | 1 | 存档默认值（1 天 / 20 金 / 100 HP / 空数组） |
| `FarmGame.Quests` | 3 | 推进与封顶、默认 8 段、最后一段是钢斧 |
| `FarmGame.Tools` | 3 | 采集倍率 1/2/3、四种工具的目录与配方、钢制配方吃基础工具 |
| `FarmGame.Shop` | 1 | 商店货架都在物品表里且有正价格 |
| `FarmGame.Food` | 2 | 苹果/面包是 Food 且有买价；商店进了这两种食物 |
| `FarmGame.Extra` | 6 | 多阶段连续生长、连续冶炼、卖货含锭、制作失败保材料、三种种子映射、配方产物 |
| **合计** | **38** | `FarmGameTests.cpp` 32 条 + `FarmGameExtraTests.cpp` 6 条 |

本机 2026-11-09 全量运行 **38/38 Success**，日志 `Saved/Logs/FarmGameAutomation.log`（编辑器自报 38 tests performed）。
运行方式见 README 的"测试"一节（`-TestExit` 而不是 `;Quit`），逐条说明另见 `Docs/TESTING.md`。

## 15. 已知行为边界

这些不是"没做"，而是当前实现的真实细节，写文档时容易被说错：

1. **睡觉不重置当日计时器**：`Sleep()` 只调 `AdvanceDay()`，`DayTimer` 继续累加。临近换天时睡觉，下一天会明显变短；
   只有自然到点（`DayTimer >= 180`）才会把 `DayTimer` 清零。
2. **暂停只冻结 GameMode 的 Tick**：菜单/商店/设置打开时 `AFarmGameMode::Tick` 提前返回，天数、太阳、天气停住；
   但 `AFarmAnimal::Tick`（漫游/野猪追击）、`AFarmBuilding::Tick`（熔炉冶炼）仍照常跑，所以"暂停"不是全局 `SetPause`。
3. **设置返回会解除暂停**：`CloseSettings()` 无条件 `SetGamePaused(false)`，而暂停菜单没有跟着关；结果是菜单还在屏幕上、游戏已在背后继续跑。
4. **目标进度不存档**：`Quests` / `ActiveQuestIndex` 不在 `UFarmSaveGame` 里；`Ctrl+L` 保持内存进度，重启后 Continue 从第 1 段开始，奖励可以再拿一次。
5. **存档槽不记忆**：`SaveSlotIndex` 每次启动或 New game（重载地图）都回到 1，只有菜单里的 `S` 能切；槽位只是存档文件名，不写进存档内容。
6. **数字键不受制作菜单约束**：`Craft1`–`Craft5` 在任何时候都生效，`C` 菜单只是显示；菜单打开时游戏也不暂停。
7. **只有前 5 条配方有数字键**：9 条配方里 `1`–`5` 对应栅栏 / 熔炉 / 箱子 / 斧 / 镐；钢斧、钢镐、铁锭、干草只能在 `Tab` 面板点按钮。
8. **商店价格是独立表**：`GetShopStock()` 不读 `FItemDef::BuyPrice`（矿石物品表买价 0、商店 8；石头物品表 3、商店 2），改价要改两处。
9. **建造只查地块 / 建筑 / 发货箱**：采集点、动物、传送门、商店都不参与 `IsBuildLocationFree`，可以在它们身上盖建筑。
10. **雨天只浇 `Planted`**：`Tilled`（空耕地）和 `Harvestable`（成熟待收）不会被雨影响。
11. **杀自家动物也有肉**：`ApplyDamage` 不区分敌友。
12. **熔炉装填是短路扣料**：先扣 1 矿、再扣 1 木；背包里有矿没木时，矿已被扣掉而装填失败（这块矿就丢了）。
13. **交互目标是每帧重算的最近物体**：地块、建筑、采集点、动物、传送门、发货箱、商店共用一个 320 的判定池（`UpdateTarget` 的 `BestDistance` 初值），
    取最近者；没有射线、没有朝向要求。动物也在池子里，喂食 / 收蛋都是走到旁边按 `E`。
14. **相机不转**：鼠标不动视角，角色朝向由移动方向决定（`bOrientRotationToMovement`）；攻击选目标只看距离，不看朝向。
15. **满血也能吃东西**：`EatFood()` 不检查 HP，照常扣 1 个食物再 `Heal`（被 clamp 回 100），所以满血按 `H` 会浪费食物。
16. **读档会重建建筑与动物**：读档后原有 Actor 指针全部失效（当前没有别处缓存这些指针）。
17. **物品没有容量与堆叠上限**：`TMap<FName, int32>` 无限装；卖出价为 0 的套件与工具（`fence_kit` / `furnace_kit` / `chest_kit` / 斧镐）永远不会被卖货带走。
18. **开始菜单的禁用按钮拦不住键盘**：`Continue` 在空槽时只禁用鼠标点击；按 `C` / `Enter` 仍会走 `HandleMenuPrimary()` 并触发 `NewGame()`。
19. **暂停菜单里的 `N` 仍能新游戏**：`New game` 按钮在暂停模式被隐藏，但 `SFarmMenuWidget::OnKeyDown` 没屏蔽 `N`。
