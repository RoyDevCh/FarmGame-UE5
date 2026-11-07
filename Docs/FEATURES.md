# FarmGame UE5 功能说明（v2）

本文按系统记录**当前代码真实实现**的内容：玩法、按键、数值、状态机（对应 2026-11-08 的源码；之后若代码再改，以源码为准）。
所有数值都来自源码常量与表，标注了出处；按键以 `Config/DefaultInput.ini` 的 ActionMappings 为准。
文末"已知行为边界"列出了实现细节里容易误解的地方。

## 0. 系统与代码入口

| 系统 | 主要代码 |
|---|---|
| 世界生成、日夜、天气、存读档 | `AFarmGameMode`（`FarmGameMode.cpp`） |
| 地块与生长 | `AFarmPlot`（`FarmPlot.cpp`）+ `FarmLogic`（`FarmTypes.h`） |
| 背包、金钱、买卖、制作结算 | `UFarmInventoryComponent` |
| 输入、交互目标、建造、制作、攻击 | `AFarmPlayerController` |
| 角色、HP、死亡回城 | `AFarmCharacter` |
| 发货箱 | `AFarmBin` |
| 动物 | `AFarmAnimal` |
| 采集点 | `AFarmResourceNode` |
| 建筑与熔炉 | `AFarmBuilding` |
| 传送门 | `AFarmPortal` |
| HUD | `AFarmHUD::DrawHUD` |
| 数据表 | `FarmItems::All`（16 项）、`FarmRecipes::All`（4 条）、`AFarmGameMode` 构造函数（3 种作物） |

## 1. 地图与初始布局

全工程只有一张地图 `/Game/Maps/Farm`（存在 `Content/Maps/Farm.umap`），关卡本身是空的：
`AFarmGameMode::BeginPlay()` → `BuildWorld()` 用 `/Engine/BasicShapes` 的 Plane / Cube / Cylinder / Sphere 现场生成全部 Actor，
`SpawnEnvironment()` 另外生成定向光、SkyAtmosphere、SkyLight。

| 对象 | 数量 | 位置（cm） | 说明 |
|---|---|---|---|
| 地面 | 1 | 原点 | Plane 缩放 140，深绿动态材质 |
| 农田地块 | 36 | X、Y ∈ [-375, 375]，间距 150 | 6×6，中心在原点 |
| 发货箱 | 1 | (0, 620) | 卖货 + 买种子 |
| 农场传送门 | 1 | (700, 700) | 目标：矿洞 (2250, 2250, 140) |
| 矿洞传送门 | 1 | (2600, 2600) | 目标：农场 (700, 700, 140) |
| 矿脉 | 6 | 矿洞中心周边（最远 420） | 每个 4 单位矿 |
| 树（可采） | 5 | 农场外围 ±1600 内 | 每个 3 单位木 |
| 石头（可采） | 5 | 农场外围 | 每个 3 单位石 |
| 鸡 | 2 | (-700, 300)、(-780, 400) | 畜栏 |
| 牛 | 1 | (-620, 520) | 畜栏 |
| 野猪 | 1 | (1400, -200) | 农场东侧 |
| 装饰树 | 6 | 外围 | 只有视觉，不能交互 |
| 玩家出生点 | — | (0, -520, 140) | `SpawnDefaultPawnAtTransform` 覆写 |

相机是固定俯视：弹簧臂长 1500、俯角 -62°，不跟随鼠标（`bUsePawnControlRotation = false`）。移动速度 520。

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
| Harvestable | `E` | 背包 + `YieldPerHarvest` 个作物；清空作物、`Stage = 0` → Tilled |

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

`FCropDef` 在 `AFarmGameMode::AFarmGameMode()` 里硬编码三条；`Stages` 含幼苗阶段（0 是刚种下）。

| 作物 | Id | 阶段数 | 每阶段天数 | 种子价 | 售价 | 单次收获 | 从播种到成熟 | 颜色 |
|---|---|---|---|---|---|---|---|---|
| 胡萝卜 | `carrot` | 3 | 1 | 3 | 8 | 1 | 2 天 | (0.95, 0.45, 0.08) 橙 |
| 土豆 | `potato` | 3 | 2 | 5 | 14 | 2 | 4 天 | (0.75, 0.62, 0.35) 土黄 |
| 玉米 | `corn` | 4 | 1 | 6 | 12 | 2 | 3 天 | (0.95, 0.85, 0.2) 黄 |

"从播种到成熟" = `(Stages - 1) × DaysPerStage`，即需要跨几次天。

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

### 4.2 物品表（`FarmItems::All`，16 项）

类别 `EItemKind`：Seed / Produce / Resource / AnimalProduct / Material。

| Id | 名称 | 类别 | 买价 | 卖价 | 来源 → 用途 |
|---|---|---|---|---|---|
| `carrot_seed` | Carrot Seed | Seed | 3 | 1 | 发货箱 `B` → 种胡萝卜 |
| `potato_seed` | Potato Seed | Seed | 5 | 2 | 发货箱 `B` → 种土豆 |
| `corn_seed` | Corn Seed | Seed | 6 | 2 | 发货箱 `B` → 种玉米 |
| `carrot` | Carrot | Produce | — | 8 | 收获 → 卖 |
| `potato` | Potato | Produce | — | 14 | 收获 → 卖 |
| `corn` | Corn | Produce | — | 12 | 收获 → 卖 / 做干草 |
| `wood` | Wood | Resource | 2 | 1 | 砍树 / 熔炉装填 / 做套件 |
| `stone` | Stone | Resource | 3 | 1 | 采石 → 熔炉套件 |
| `ore` | Ore | Resource | — | 4 | 矿脉 → 熔炉 |
| `ingot` | Iron Ingot | Material | — | 18 | 熔炉 → 卖 |
| `hay` | Hay | Resource | 4 | 1 | 玉米制作 → 喂动物 |
| `egg` | Egg | AnimalProduct | — | 10 | 鸡 → 卖 |
| `milk` | Milk | AnimalProduct | — | 16 | 牛 → 卖 |
| `meat` | Meat | AnimalProduct | — | 12 | 动物死亡 → 卖 |
| `fence_kit` | Fence Kit | Material | — | 0 | 制作 → 建造栅栏 |
| `furnace_kit` | Furnace Kit | Material | — | 0 | 制作 → 建造熔炉 |

注意：表里有买价的只有种子（以及 wood / stone / hay），但**界面上只有"在发货箱旁买当前种子"这一条购买路径**；
wood / stone / hay 的买价目前是不可达数据。

### 4.3 买：发货箱 + `B`

`AFarmPlayerController::BuySeed()`：当前交互目标是发货箱时，按 `B` 买 1 颗**当前选中作物**的种子。
价格用 `FCropDef::SeedCost`，余额不足则 Toast `Not enough gold`。

### 4.4 卖：发货箱 + `E`

`UFarmInventoryComponent::SellEverything()` 一次清空所有可卖物品：

- 卖：`Produce`、`AnimalProduct`、`Material`（且 `SellPrice > 0`）—— 即胡萝卜/土豆/玉米、蛋/奶/肉、铁锭。
- 不卖：`Seed`（种子留着下次种）、`Resource`（木/石/矿/干草是建材与燃料）。
- 价格为 0 的 `fence_kit` / `furnace_kit` 不会被卖掉。
- 返回本次总收入，Toast `Sold goods for +N gold`；没东西可卖时提示 `Nothing to sell`。

发货箱提示会实时算出可卖件数（`GetSellableCount`）。

## 5. 制作

### 5.1 配方（`FarmRecipes::All`，4 条）

| # | 配方 Id | 产物 | 消耗 | 需要熔炉 |
|---|---|---|---|---|
| 1 | `fence_kit` | 栅栏套件 ×1 | wood ×5 | 否 |
| 2 | `furnace_kit` | 熔炉套件 ×1 | stone ×10 + wood ×5 | 否 |
| 3 | `ingot` | 铁锭 ×1 | ore ×2 | **是**（站在熔炉 320 内） |
| 4 | `hay_bundle` | 干草 ×2 | corn ×2 | 否 |

### 5.2 交互

| 按键 | 作用 |
|---|---|
| `C` | 打开 / 关闭制作菜单（不暂停游戏） |
| `1`–`5` | 结算第 1–5 条配方；只有 4 条配方，`5` 是空操作 |
| `Esc` | 关闭菜单（优先于退出建造模式） |

判定顺序（`AFarmPlayerController::CraftRecipe`）：先在 320 内找熔炉（`bNeedsFurnace` 的配方）→ 再 `CanCraft` 检查材料 →
扣材料、加产物。失败分别提示 `Stand next to a furnace to smelt` 与 `Missing ingredients`。
菜单里能做的配方显示绿色、不能做的灰色（`CanCraftRecipe`）。

### 5.3 两条做铁锭的路

代码里铁锭有两个来源，都真实可用：

1. **配方 3**：`C` 菜单 + `3`，消耗 2 矿直接得 1 锭（要求站在熔炉旁）。
2. **熔炉装填**：`E` 给熔炉装 1 矿 + 1 木，8 秒后出 1 锭（见第 7 节）。

## 6. 建造

### 6.1 模式状态机（`G`）

    off (-1) --G--> Fence (0) --G--> Furnace (1) --G--> off (-1)
    任意状态 --Esc--> off

进入/切换模式会 Toast 当前类型，HUD 底部中央显示 `BUILD MODE: Fence/Furnace`。

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
   - 不检查采集点、动物、传送门
3. 通过则扣 1 个套件、`SpawnBuilding` 生成建筑并注册进 `AFarmGameMode::Buildings`。

### 6.4 建筑种类

| 类型 | 交互 | 说明 |
|---|---|---|
| `Fence` | 无 | 1.6 × 0.15 × 1.0 的木色方块，不能通行 |
| `Furnace` | `E` 装填 / 取锭 | 1.4 × 1.4 × 1.2 的石色方块 + 圆柱烟囱 |

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
| `Tree` | wood ×1 | 5 个点 × 3 | 是 |
| `Rock` | stone ×1 | 5 个点 × 3 | 是 |
| `Ore` | ore ×1 | 6 个点 × 4 | 是 |

规则：

- `E` 采一次扣 1 单位，Toast `+1 Wood/Stone/Ore`。
- 采光后 Actor 隐藏，提示 `Depleted - regrows tomorrow`。
- 每天 `AdvanceDay` 里 `Regrow()`：数量回满并重新显示。
- 农场外围另有 6 棵纯装饰树（`AStaticMeshActor`，两个方块拼成），不参与采集。

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

- 喂食消耗 1 个 `hay`；干草由玉米做（配方 4），初始背包也有 1 个。
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

### 10.1 玩家攻击（鼠标左键）

| 参数 | 值 |
|---|---|
| 冷却 | 0.45 秒 |
| 伤害 | 34 |
| 距离 | 320（2D 距离） |
| 角度 | 与角色朝向点积 ≥ 0.1（约 ±84°） |
| 选目标 | 范围内最近的活着的动物 |

野猪 60 HP → 2 下打死；鸡 30 HP → 1 下；牛 70 HP → 3 下。

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

## 12. 存档

### 12.1 入口

- `Ctrl+S` 保存，`Ctrl+L` 读取，槽位名 `FarmSlot`、用户索引 0，落盘到 `Saved/SaveGames/FarmSlot.sav`。
- 数据载体 `UFarmSaveGame : USaveGame`。

### 12.2 存了什么

| 字段 | 内容 |
|---|---|
| `Day` | 天数 |
| `DayTimer` | 当天已过的时间（秒），读档后从原处继续 |
| `bRaining` | 当天是否下雨 |
| `Gold` | 金币 |
| `Items` | 全部物品 `TMap<FName, int32>` |
| `PlayerHealth` | 玩家 HP |
| `Plots` | 36 条 `FPlotSaveData`：索引、状态、作物 Id、阶段、是否浇水、上次生长日 |
| `Buildings` | 每座建筑：类型、位置、矿石数、铁锭数 |
| `Animals` | 每只动物：类型、位置、是否已喂、是否有产物、HP |

### 12.3 读档做了什么

1. 覆盖天数、`DayTimer`、天气、金币、物品、HP。
2. 按索引恢复地块（作物指针用 `FCropDef` 目录重新查一次）。
3. **销毁并重建**全部建筑 Actor，再 `RestoreState` 回填矿石/铁锭。
4. **销毁并重建**全部动物 Actor，再 `RestoreState` 回填喂养/产物/HP（HP 会被 clamp 到至少 1）。
5. 找不到存档时 Toast `No save found`。

### 12.4 不保存的内容

玩家位置（始终在出生点或读档时当前位置）、采集点剩余量与隐藏状态、当前选中的种子、太阳/相机状态（每帧重算）、
Toast 与 UI 状态、野猪的巡逻位置（动物位置会存，但读档后从存的位置重新开始漫游）。

## 13. HUD

全部由 `AFarmHUD::DrawHUD` 用 Canvas 立即模式绘制，没有 UMG / Widget 资产：

| 位置 | 内容 |
|---|---|
| 左上 | `Day N      Gold N g` |
| 右上 | 时间 `HH:MM` + 天气 `Sunny/Rainy` |
| 左下 | HP 条 + `HP n / 100` |
| 左中 | `Selected seed: X (Q to change)` |
| 左中 | 每作物一行：种子数、收获数、种子价、售价 |
| 左中 | `Wood / Stone / Ore / Ingot / Hay` 计数 |
| 左中 | `Kits: fence N   furnace N` |
| 底部中央 | 交互提示（随目标变化）；建造模式横幅 |
| 底部左 | 操作帮助行 |
| 底部 | 制作菜单（按 `C` 展开，逐条列出配方、材料、能否制作） |
| 顶部中央 | Toast（3.5 秒，最后 1.2 秒淡出） |

## 14. 自动化测试（17 条）

全部是 `EAutomationTestFlags::EditorContext | EngineFilter` 的简单测试，前缀 `FarmGame.`。
基础 11 条在 `FarmGameTests.cpp`，补充 6 条在 `FarmGameExtraTests.cpp`，都包在 `#if WITH_DEV_AUTOMATION_TESTS` 里。

| # | 全名 | 验证什么 |
|---|---|---|
| 1 | `FarmGame.CropGrowth.AdvancesWithWater` | 浇水后跨一天推进 1 阶、更新生长日、消耗水 |
| 2 | `FarmGame.CropGrowth.BlockedWithoutWater` | 没水不长；土豆 1 天不够、2 天才长 |
| 3 | `FarmGame.CropGrowth.MaturesAndStops` | 连推 10 天停在最后阶段并判定成熟 |
| 4 | `FarmGame.Inventory.BuyAndSell` | 初始 20 金 / 3 胡萝卜种子 / 6 木；买种子扣钱加物品；卖货收入 34（2 胡萝卜 + 1 铁锭） |
| 5 | `FarmGame.Inventory.CannotBuyWithoutGold` | 钱不够时购买失败且状态不变 |
| 6 | `FarmGame.Crafting.ConsumesIngredientsAndOutputs` | 栅栏套件扣 5 木、第二次因材料不足失败；铁锭配方带熔炉前置标记 |
| 7 | `FarmGame.Items.CatalogIsConsistent` | 物品表非空、胡萝卜售价 8、种子判定与 `SeedId` 派生 |
| 8 | `FarmGame.Crafting.HayFromCorn` | 2 玉米 → 2 干草，玉米清零 |
| 9 | `FarmGame.Inventory.SellSkipsSeedsAndResources` | 卖货跳过木/矿，蛋 + 奶收入 36 |
| 10 | `FarmGame.Recipes.CatalogReferencesKnownItems` | 每条配方的产物与材料都能在物品表里找到 |
| 11 | `FarmGame.Crafting.FurnaceSmeltsOneOrePerInterval` | 冶炼 8 秒一块、空炉不产 |

补充 6 条（`FarmGameExtraTests.cpp`）：

| # | 全名 | 验证什么 |
|---|---|---|
| 12 | `FarmGame.Extra.CropMultiStageContinuous` | 4 阶段 / 每阶段 2 天的作物按天推进：第 1 天不动但水保留，第 2/4/6 天各进 1 阶，第 8 天成熟停住 |
| 13 | `FarmGame.Extra.SmeltTwoStepsInARow` | 连续两次冶炼各出 1 锭，计时余量依次为 1 秒 / 2 秒 |
| 14 | `FarmGame.Extra.SellIncludesIngotKeepsSeedsAndResources` | 卖货卖出铁锭与胡萝卜（44 金），种子 / 木 / 矿 / 干草保留；`GetSellableCount` 只统计可卖项 |
| 15 | `FarmGame.Extra.CraftFailureKeepsMaterials` | 熔炉套件在 0 石与 5 石两种不足情况下都不扣任何材料、不产出 |
| 16 | `FarmGame.Extra.SeedIdsForThreeCrops` | 三种作物都能映射到对应种子，且种子在物品表里确实被标成 Seed |
| 17 | `FarmGame.Extra.RecipeOutputsInCatalog` | 每条配方的产物都在物品表里、产出数量 > 0，四个预期产物（fence_kit / furnace_kit / ingot / hay）都存在 |

运行方式见 README 的"测试"一节（`-TestExit` 而不是 `;Quit`），逐条说明另见 `Docs/TESTING.md`。

## 15. 已知行为边界

这些不是"没做"，而是当前实现的真实细节，写文档时容易被说错：

1. **睡觉不重置当日计时器**：`Sleep()` 只调 `AdvanceDay()`，`DayTimer` 继续累加。临近换天时睡觉，下一天会明显变短；
   只有自然到点（`DayTimer >= 180`）才会把 `DayTimer` 清零。
2. **数字键不受制作菜单约束**：`Craft1`–`Craft5` 在任何时候都生效，`C` 菜单只是显示；菜单打开时游戏也不暂停。
3. **配方 5 号键是空操作**：只有 4 条配方。
4. **建造只查地块 / 建筑 / 发货箱**：采集点、动物、传送门不参与 `IsBuildLocationFree`，可以在它们身上盖建筑。
5. **雨天只浇 `Planted`**：`Tilled`（空耕地）和 `Harvestable`（成熟待收）不会被雨影响。
6. **杀自家动物也有肉**：`ApplyDamage` 不区分敌友。
7. **熔炉装填是短路扣料**：先扣 1 矿、再扣 1 木；背包里有矿没木时，矿已被扣掉而装填失败（这块矿就丢了）。
8. **交互目标是每帧重算的"最近可达物体"**：地块、建筑、采集点、动物、传送门、发货箱共用一个 320 的判定池（`UpdateTarget` 的 `BestDistance` 初值），
   取最近者；没有射线、没有朝向要求。动物也在池子里，喂食 / 收蛋都是走到旁边按 `E`。
9. **相机不转**：鼠标不动视角，角色朝向由移动方向决定（`bOrientRotationToMovement`），所以攻击方向 = 移动方向。
10. **读档会重建建筑与动物**：读档后原有 Actor 指针全部失效（当前没有别处缓存这些指针）。
11. **物品没有容量与堆叠上限**：`TMap<FName, int32>` 无限装；`fence_kit` / `furnace_kit` 卖出价为 0，永远不会被卖货带走。
