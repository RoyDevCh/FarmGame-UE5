# FarmGame (Unity) -> FarmGameUE data parity

_Generated 2026-11-09T06:55:47+08:00. Read-only analysis: no Source file, no asset and no build output was modified._

| | |
|---|---|
| Source (truth) | Assets/FarmingEngine/Resources - 348 *.asset parsed, 0 unparsed |
| Target | Source/FarmGameUE/FarmItems.h, Source/FarmGameUE/FarmRecipes.h |
| Matching | normalized id, then order-insensitive token key, then explicit rename aliases, then title/label; every UE entry is claimed at most once |

## Summary

| Catalog | Original | Matched | Missing | Match rate | UE-only entries |
|---|---|---|---|---|---|
| Items (ItemData) | 141 | 25 | 116 | 18% | 9 |
| Recipes (craft cost present) | 81 | 9 | 72 | 11% | 7 |

The UE catalogs hold 34 items and 16 recipes; 9 item ids and 7 recipe ids have no original counterpart.

## 1. What was parsed

| Asset type | Count | Used for |
|---|---|---|
| ItemData | 141 | items + craft recipes |
| Action* | 52 | verb actions; ActionCook = cooking conversions |
| GroupData | 39 | craft fillers / near stations |
| ConstructionData | 29 | build recipes |
| CharacterData | 27 | animals/NPCs (referenced by item character_data) |
| PlantData | 21 | growth data (no craft cost) |
| LevelData | 18 | levels (ignored) |
| BonusEffectData | 6 | effects (ignored) |
| AttributeData | 5 | stats (ignored) |
| LootData | 4 | loot tables (ignored) |
| WeatherData | 3 | weather (ignored) |
| AssetData | 1 | asset registry (ignored) |
| GameData | 1 | global config (ignored) |
| SpawnData | 1 | spawns (ignored) |

Field mapping: id -> FItemDef::Id, title -> Label, buy_cost -> BuyPrice, sell_cost -> SellPrice, icon GUID -> Assets/FarmingEngine/Sprites/Icons/<name>.png (141/141 resolved), inventory_max -> stack size (no UE field), craft_items (repeated GUIDs) + craft_quantity -> FCraftRecipe::Inputs / OutputCount.

## 2. Source schema notes

- There is **no CraftData asset and no AnimalData asset** in this project. CraftData is only the shared base class; the crafting fields are serialized onto ItemData (141), ConstructionData (29) and PlantData (21). Animals are ItemData entries whose character_data points at a CharacterData (27).
- A craft cost is a **flat array of item GUIDs**; the required count is the number of duplicate entries. craft_fillers (GroupData) and craft_requirements (other craftables) are separate arrays.
- PlantData never carries a craft cost: the seed ItemData holds plant_data, so plants are growth targets, not recipes.
- craft_near (a GroupData) is the station requirement; 10 recipe entries have one, all pointing at the Fire group (9 cooking recipes plus Candle). nachos/pokebowl are cookable without a station.
- Icons are Sprite references, never paths; they resolve 1:1 through <file>.png.meta to Sprites/Icons/*.png.
- One dangling reference exists in the source: barn and house require craft_requirements GUID 91a4afcd..., which has no .meta anywhere under Assets (the asset was deleted). It appears as ?91a4afcd in data_parity.json.

## 3. Items

### 3.1 Matched items (25)

buy/sell are the original buy_cost/sell_cost; d is the UE value minus the original.

| Original id | Title | orig buy | orig sell | UE id | UE buy | UE sell | d buy | d sell | matched by |
|---|---|---|---|---|---|---|---|---|---|
| `bread` | Bread | 4 | 4 | `bread` | 8 | 4 | +4 | +0 | exact |
| `soup` | Soup | 5 | 5 | `soup` | 0 | 22 | -5 | +17 | exact |
| `axe` | Axe | 1 | 1 | `axe` | 0 | 0 | -1 | -1 | exact |
| `fish_rod` | Fishing Rod | 1 | 1 | `fishing_rod` | 0 | 0 | -1 | -1 | alias-exact |
| `pickaxe` | Pickaxe | 1 | 1 | `pickaxe` | 0 | 0 | -1 | -1 | exact |
| `apple` | Apple | 3 | 3 | `apple` | 5 | 3 | +2 | +0 | exact |
| `carrot` | Carrot | 2 | 2 | `carrot` | 0 | 8 | -2 | +6 | exact |
| `corn` | Corn | 2 | 2 | `corn` | 0 | 12 | -2 | +10 | exact |
| `egg` | Egg | 2 | 2 | `egg` | 0 | 10 | -2 | +8 | exact |
| `fish_cooked` | Fish | 2 | 2 | `cooked_fish` | 0 | 20 | -2 | +18 | token |
| `meat_cooked` | Meat | 2 | 2 | `meat` | 0 | 12 | -2 | +10 | label |
| `milk` | Milk | 2 | 2 | `milk` | 0 | 16 | -2 | +14 | exact |
| `potato` | Potato | 2 | 2 | `potato` | 0 | 14 | -2 | +12 | exact |
| `pumpkin` | Pumpkin | 3 | 3 | `pumpkin` | 0 | 26 | -3 | +23 | exact |
| `seed_carrot` | Carrot Seed | 1 | 1 | `carrot_seed` | 3 | 1 | +2 | +0 | token |
| `seed_corn` | Corn Seed | 1 | 1 | `corn_seed` | 6 | 2 | +5 | +1 | token |
| `seed_potato` | Potato Seed | 1 | 1 | `potato_seed` | 5 | 2 | +4 | +1 | token |
| `seed_pumpkin` | Seed Pumpkin | 1 | 1 | `pumpkin_seed` | 10 | 4 | +9 | +3 | token |
| `seed_tomato` | Tomato Seed | 1 | 1 | `tomato_seed` | 7 | 3 | +6 | +2 | exact |
| `tomato` | Tomato | 2 | 2 | `tomato` | 0 | 18 | -2 | +16 | exact |
| `hay` | Hay | 1 | 1 | `hay` | 4 | 1 | +3 | +0 | exact |
| `metal_bar` | Metal Bar | 8 | 8 | `ingot` | 0 | 18 | -8 | +10 | alias-exact |
| `metal_ore` | Metal | 4 | 4 | `ore` | 0 | 4 | -4 | +0 | alias-exact |
| `rock` | Rock | 1 | 1 | `stone` | 3 | 1 | +2 | +0 | alias-exact |
| `wood` | Wood | 1 | 1 | `wood` | 2 | 1 | +1 | +0 | exact |

**Price mismatches: 25 of 25 matched items** - not a single matched pair keeps both original prices.

The port systematically rebalances the economy: every seed is dearer than the original (carrot_seed 3/1 vs seed_carrot 1/1, pumpkin_seed 10/4 vs seed_pumpkin 1/1), while raw produce keeps a sell price and loses its buy price. meat_cooked is the only label-only match (title Meat); the original splits meat_raw / meat_cooked and the port has a single meat.

### 3.2 UE items with no original counterpart (9)

| UE id | Label | Kind | buy | sell |
|---|---|---|---|---|
| `fence_kit` | Fence Kit | Material | 0 | 0 |
| `furnace_kit` | Furnace Kit | Material | 0 | 0 |
| `chest_kit` | Chest Kit | Material | 0 | 0 |
| `sprinkler_kit` | Sprinkler Kit | Material | 0 | 0 |
| `mixing_pot_kit` | Mixing Pot Kit | Material | 0 | 0 |
| `fish` | Fish | Food | 0 | 12 |
| `cheese` | Cheese | Food | 0 | 14 |
| `steel_axe` | Steel Axe | Material | 0 | 0 |
| `steel_pickaxe` | Steel Pickaxe | Material | 0 | 0 |

These are port inventions: the *_kit build items (fence_kit, furnace_kit, chest_kit, sprinkler_kit, mixing_pot_kit), steel_axe, steel_pickaxe, cheese and a generic fish (the original has fish_raw / fish_cooked).

### 3.3 Missing original items (116) - top 20 by sell value

| # | Original id | Title | Category | buy | sell | stack | icon |
|---|---|---|---|---|---|---|---|
| 1 | `diamond` | Diamond | Other | 100 | 100 | 20 | Diamond.png |
| 2 | `mushroom_black` | Mushroom | Food | 66 | 66 | 20 | MushroomBlack.png |
| 3 | `horse_item` | Horse | Animals | 40 | 40 | 1 | horse_small.png |
| 4 | `gold_bar` | Gold Bar | Other | 20 | 20 | 20 | GoldBar.png |
| 5 | `cow_item` | Cow | Animals | 10 | 10 | 1 | cow_small.png |
| 6 | `gold_ore` | Gold | Other | 10 | 10 | 20 | RockGold.png |
| 7 | `pig_item` | Pig | Animals | 10 | 10 | 1 | pig_small.png |
| 8 | `sheep_item` | Sheep | Animals | 10 | 10 | 1 | sheep_small.png |
| 9 | `mushroom_green` | Mushroom | Food | 6 | 6 | 20 | MushroomGreen.png |
| 10 | `mushroom_red` | Mushroom | Food | 6 | 6 | 20 | MushroomRed.png |
| 11 | `chick_item` | Chick | Animals | 5 | 5 | 1 | chicken_small.png |
| 12 | `chicken_curry` | Chicken Curry | Cooking | 5 | 5 | 20 | chicken-curry.png |
| 13 | `fish_n_chip` | Fish & Chips | Cooking | 5 | 5 | 20 | Fish N Chips.png |
| 14 | `ham_brunch` | Ham Brunch | Cooking | 5 | 5 | 20 | HamBrunch.png |
| 15 | `hat_halloween` | Hat Halloween | Equipment | 5 | 5 | 1 | witch_hat.png |
| 16 | `mask_halloween` | Halloween Mask | Other | 5 | 5 | 1 | mask_halloween.png |
| 17 | `nachos` | Nachos N Salsa | Cooking | 5 | 5 | 20 | NachosSalsa.png |
| 18 | `pokebowl` | Pokebowl | Cooking | 5 | 5 | 20 | PokeBowl.png |
| 19 | `potion_black` | Black Potion | Food | 5 | 5 | 20 | potion-black.png |
| 20 | `potion_green` | Green Potion | Food | 5 | 5 | 20 | potion-green.png |

### 3.4 Missing original items - full list (116)

| Original id | Title | Category | buy | sell | stack | icon |
|---|---|---|---|---|---|---|
| `chick_item` | Chick | Animals | 5 | 5 | 1 | chicken_small.png |
| `cow_item` | Cow | Animals | 10 | 10 | 1 | cow_small.png |
| `horse_item` | Horse | Animals | 40 | 40 | 1 | horse_small.png |
| `pig_item` | Pig | Animals | 10 | 10 | 1 | pig_small.png |
| `rabbit_item` | Rabbit | Animals | 5 | 5 | 1 | rabbit_small.png |
| `sheep_item` | Sheep | Animals | 10 | 10 | 1 | sheep_small.png |
| `chicken_curry` | Chicken Curry | Cooking | 5 | 5 | 20 | chicken-curry.png |
| `fish_n_chip` | Fish & Chips | Cooking | 5 | 5 | 20 | Fish N Chips.png |
| `ham_brunch` | Ham Brunch | Cooking | 5 | 5 | 20 | HamBrunch.png |
| `nachos` | Nachos N Salsa | Cooking | 5 | 5 | 20 | NachosSalsa.png |
| `pokebowl` | Pokebowl | Cooking | 5 | 5 | 20 | PokeBowl.png |
| `pop_corn` | Pop Corn | Cooking | 3 | 3 | 20 | PopCorn.png |
| `skewer` | Skewer | Cooking | 1 | 1 | 20 | Meat Skewer.png |
| `spaghetti` | Spaghetti Bolognese | Cooking | 5 | 5 | 20 | Spaghetti.png |
| `taco` | Beef Taco | Cooking | 5 | 5 | 20 | Beef Taco.png |
| `armor_metal` | Metal Armor | Equipment | 1 | 1 | 1 | ArmorStone.png |
| `armor_wood` | Wood Armor | Equipment | 1 | 1 | 1 | Armor.png |
| `backpack` | Backpack | Equipment | 1 | 1 | 1 | backpack.png |
| `baseball_bat` | Baseball Bat | Equipment | 1 | 1 | 1 | bat.png |
| `battle_axe` | Battle Axe | Equipment | 1 | 1 | 1 | AxeDouble.png |
| `boots` | Boots | Equipment | 1 | 1 | 1 | Boots.png |
| `bow` | Bow | Equipment | 1 | 1 | 1 | BowIcon.png |
| `cleaver` | Cleaver | Equipment | 1 | 1 | 1 | cleaver.png |
| `hammer` | Hammer | Equipment | 1 | 1 | 1 | Hammer.png |
| `hat` | Hat | Equipment | 2 | 2 | 1 | Hat.png |
| `hat_halloween` | Hat Halloween | Equipment | 5 | 5 | 1 | witch_hat.png |
| `helmet_metal` | Metal Helmet | Equipment | 1 | 1 | 1 | HelmetRock.png |
| `helmet_wood` | Wood Helmet | Equipment | 1 | 1 | 1 | HelmetWood.png |
| `hoe` | Hoe | Equipment | 1 | 1 | 1 | hoe.png |
| `nutgun` | Nut-gun | Equipment | 1 | 1 | 1 | gun.png |
| `pitchfork` | Pitchfork | Equipment | 1 | 1 | 1 | Pitchfork.png |
| `ring` | Ring | Equipment | 1 | 1 | 1 | ring.png |
| `santa_hat` | Santa Hat | Equipment | 5 | 5 | 1 | winter_hat.png |
| `scythe` | Scythe | Equipment | 1 | 1 | 1 | scythe.png |
| `shovel` | Shovel | Equipment | 1 | 1 | 1 | Shovel.png |
| `slingshot` | Slingshot | Equipment | 1 | 1 | 1 | slingshoot.png |
| `sword` | Sword | Equipment | 4 | 4 | 1 | SwordIcon.png |
| `torch` | Torch | Equipment | 1 | 1 | 1 | Torch.png |
| `vest` | Vest | Equipment | 1 | 1 | 1 | Vest.png |
| `watering_can` | Watering Can | Equipment | 1 | 1 | 1 | WaterCan.png |
| `watering_can_full` | Watering Can | Equipment | 1 | 1 | 1 | WaterCanFull.png |
| `wheat` | Wheat | Equipment | 2 | 2 | 20 | Wheat.png |
| `winter_boots` | Winter Boots | Equipment | 1 | 1 | 1 | winter_boots.png |
| `acorn` | Acorn | Food | 1 | 1 | 20 | Acorn.png |
| `bacon` | Bacon | Food | 2 | 2 | 20 | Bacon.png |
| `bean` | Bean | Food | 2 | 2 | 20 | bean fuit.png |
| `blueberry` | Blueberries | Food | 2 | 2 | 20 | Blueberry.png |
| `candy_black` | Candy | Food | 1 | 1 | 20 | Candy.png |
| `candy_cane` | Candy Cane | Food | 1 | 1 | 20 | cane.png |
| `candy_orange` | Candy | Food | 1 | 1 | 20 | Candy2.png |
| `cherry` | Cherry | Food | 3 | 3 | 20 | cherry.png |
| `chicken_cooked` | Chicken | Food | 2 | 2 | 20 | batak.png |
| `chicken_raw` | Raw Chicken | Food | 1 | 1 | 20 | batak_raw.png |
| `coconut` | Coconut | Food | 4 | 4 | 20 | Coconut.png |
| `coconut_half` | Coconut | Food | 1 | 1 | 20 | Coconut Halfs.png |
| `egg_cooked` | Egg | Food | 1 | 1 | 20 | EggCook.png |
| `fish_raw` | Raw Fish | Food | 2 | 2 | 20 | FishRaw.png |
| `flour` | Flour | Food | 2 | 2 | 20 | flour.png |
| `ham_cooked` | Ham | Food | 2 | 2 | 20 | ham.png |
| `ham_raw` | Raw Ham | Food | 1 | 1 | 20 | ham_raw.png |
| `honey` | Honey | Food | 2 | 2 | 5 | Honey.png |
| `meat_raw` | Raw Meat | Food | 2 | 2 | 20 | MeatRaw.png |
| `mushroom_black` | Mushroom | Food | 66 | 66 | 20 | MushroomBlack.png |
| `mushroom_green` | Mushroom | Food | 6 | 6 | 20 | MushroomGreen.png |
| `mushroom_red` | Mushroom | Food | 6 | 6 | 20 | MushroomRed.png |
| `orange` | Orange | Food | 3 | 3 | 20 | orange.png |
| `pepper` | Pepper | Food | 2 | 2 | 20 | RedPepper.png |
| `potion_black` | Black Potion | Food | 5 | 5 | 20 | potion-black.png |
| `potion_green` | Green Potion | Food | 5 | 5 | 20 | potion-green.png |
| `potion_orange` | Orange Potion | Food | 5 | 5 | 20 | potion-orange.png |
| `potion_purple` | Purple Potion | Food | 5 | 5 | 20 | potion-purple.png |
| `potion_red` | Red Potion | Food | 5 | 5 | 20 | potion-red.png |
| `potion_vial` | Vial | Food | 5 | 5 | 20 | potion-empty.png |
| `potion_water` | Blue Potion | Food | 5 | 5 | 20 | potion-blue.png |
| `potion_white` | White Potion | Food | 5 | 5 | 20 | potion-white.png |
| `potion_yellow` | Yellow Potion | Food | 5 | 5 | 20 | potion-yellow.png |
| `raspberry` | Raspberries | Food | 2 | 2 | 20 | Raspberry.png |
| `rice` | Rice | Food | 2 | 2 | 20 | RiceGrain.png |
| `seed_apple` | Apple Seed | Food | 1 | 1 | 20 | seed_tree.png |
| `seed_bean` | Bean Seed | Food | 1 | 1 | 20 | bean.png |
| `seed_blueberry` | Blueberry Seed | Food | 1 | 1 | 20 | Seeds.png |
| `seed_cherry` | Cherry Seed | Food | 1 | 1 | 20 | seed_tree.png |
| `seed_orange` | Orange Seed | Food | 1 | 1 | 20 | seed_tree.png |
| `seed_pepper` | Seed Pepper | Food | 1 | 1 | 20 | Seeds.png |
| `seed_rice` | Rice Seed | Food | 1 | 1 | 20 | Seeds.png |
| `seed_strawberry` | Strawberry Seed | Food | 1 | 1 | 20 | Seeds.png |
| `seed_tulip` | Tulip Seed | Food | 1 | 1 | 20 | Seeds.png |
| `seed_wheat` | Wheat Seed | Food | 1 | 1 | 20 | Seeds.png |
| `strawberry` | Strawberry | Food | 2 | 2 | 20 | strawberry.png |
| `water` | Water | Food | 1 | 1 | 5 | Water Container Full 2.png |
| `arrow` | Arrow | Other | 1 | 1 | 20 | Arrows.png |
| `candle_item` | Candle | Other | 1 | 1 | 5 | Candle.png |
| `coin` | Gold Coin | Other | 1 | 1 | 999 | Gold.png |
| `diamond` | Diamond | Other | 100 | 100 | 20 | Diamond.png |
| `flag_item` | Flag | Other | 1 | 1 | 1 | flag.png |
| `gift` | Gift | Other | 1 | 1 | 5 | gift.png |
| `gold_bar` | Gold Bar | Other | 20 | 20 | 20 | GoldBar.png |
| `gold_ore` | Gold | Other | 10 | 10 | 20 | RockGold.png |
| `grass` | Grass | Other | 1 | 1 | 20 | Grass.png |
| `halloween_note` | Happy Halloween! | Other | 1 | 1 | 20 | scroll.png |
| `healing_mix` | Healing Mixture | Other | 1 | 1 | 20 | HealingHerbs.png |
| `herbs` | Herbs | Other | 1 | 1 | 20 | Herb.png |
| `jug` | Jug | Other | 1 | 1 | 5 | Container Empty.png |
| `lure_item` | Lure | Other | 1 | 1 | 5 | Lure.png |
| `map` | Treasure Map | Other | 0 | 0 | 1 | map.png |
| `mask_halloween` | Halloween Mask | Other | 5 | 5 | 1 | mask_halloween.png |
| `repair_kit` | Repair Kit | Other | 5 | 5 | 20 | RepairKit.png |
| `rope` | Rope | Other | 1 | 1 | 20 | rope.png |
| `scroll` | Scroll | Other | 1 | 1 | 5 | scroll.png |
| `scroll_alchemy` | Alchemy Blueprint | Other | 1 | 1 | 5 | scroll.png |
| `scroll_gun` | Nutgun Blueprint | Other | 1 | 1 | 5 | scroll.png |
| `skin` | Skin | Other | 1 | 1 | 20 | SkinIcon.png |
| `snowball` | Snowball | Other | 0 | 0 | 20 | snowball.png |
| `trap_item` | Trap | Other | 1 | 1 | 5 | BearTrap.png |
| `tulip` | Tulip | Other | 2 | 2 | 20 | tulip.png |
| `wool` | Wool | Other | 2 | 2 | 20 | wool.png |

## 4. Recipes

An original recipe is any CraftData-derived asset (ItemData / ConstructionData / PlantData) with a non-empty cost array: **81 entries** (28 construction, 53 item, 0 plant). The port defines **16** recipes.

### 4.1 Matched recipes (9)

Inputs are compared after mapping original item ids through the rename aliases (rock->stone, metal_ore->ore, metal_bar->ingot, fish_rod->fishing_rod).

| Original id | UE id | orig inputs | UE inputs | inputs equal | orig out | UE out | matched by |
|---|---|---|---|---|---|---|---|
| `fence` | `fence_kit` | wood x3 | wood x5 | **no** | 1 | 1 | strip-kit |
| `furnace` | `furnace_kit` | ore x2, stone x3 | stone x10, wood x5 | **no** | 1 | 1 | strip-kit |
| `sprinkler` | `sprinkler_kit` | ore x3 | ore x2, stone x6 | **no** | 1 | 1 | strip-kit |
| `bread` | `bread` | flour x1, water x1 | corn x2 | **no** | 1 | 1 | exact |
| `soup` | `soup` | water x1 | carrot x1, potato x1 | **no** | 1 | 1 | exact |
| `axe` | `axe` | stone x1, wood x1 | ore x1, wood x4 | **no** | 1 | 1 | exact |
| `fish_rod` | `fishing_rod` | grass x1, wood x1 | ore x1, wood x4 | **no** | 1 | 1 | alias-output |
| `pickaxe` | `pickaxe` | stone x1, wood x1 | ore x2, wood x4 | **no** | 1 | 1 | exact |
| `metal_bar` | `ingot` | ore x3 | ore x2 | **no** | 1 | 1 | alias-output |

**No matched recipe agrees on inputs.** The port both adds and drops ingredients and inflates quantities: fence wood x3 -> wood x5, furnace rock x3 + metal_ore x2 -> stone x10 + wood x5, sprinkler metal_ore x3 -> stone x6 + ore x2, axe/pickaxe wood x1 + rock x1 -> wood x4 + ore x1/x2. The cooking recipes were rebuilt around the port own items (bread flour + water -> corn x2, soup water -> carrot + potato).

### 4.2 UE recipes with no original counterpart (7)

| UE id | Label | Inputs | Output | Count |
|---|---|---|---|---|
| `chest_kit` | Storage Chest Kit (build with G) | wood x12 | `chest_kit` | 1 |
| `steel_axe` | Steel Axe (triple wood per chop) | axe x1, ore x5 | `steel_axe` | 1 |
| `steel_pickaxe` | Steel Pickaxe (triple stone/ore) | pickaxe x1, ore x5 | `steel_pickaxe` | 1 |
| `hay_bundle` | Hay Bundle (animal feed) | corn x2 | `hay` | 2 |
| `mixing_pot_kit` | Mixing Pot Kit (build with G) | stone x8, wood x6 | `mixing_pot_kit` | 1 |
| `cheese` | Cheese (at mixing pot) | milk x2 | `cheese` | 1 |
| `cooked_fish` | Cooked Fish (at mixing pot) | fish x1 | `cooked_fish` | 1 |

chest_kit is the notable gap: the original has two distinct chest constructions (chest_wood, chest_stone), so the kit name maps onto neither. The other six have no original analogue at all.

### 4.3 Missing original recipes (72) - top 20 by input count

| # | Original id | Kind | Inputs | Output count | craftable |
|---|---|---|---|---|---|
| 1 | `barn` | construction | wood x7, grass x5 | 1 | learn-only |
| 2 | `house` | construction | wood x7, grass x5 | 1 | learn-only |
| 3 | `windmill` | construction | rock x3, wood x2, metal_ore x1 | 1 | yes |
| 4 | `armor_wood` | item | skin x2, wood x3 | 1 | yes |
| 5 | `bbq` | construction | wood x3, rock x2 | 1 | yes |
| 6 | `chest_stone` | construction | rock x4, metal_ore x1 | 1 | yes |
| 7 | `chest_wood` | construction | wood x4, rock x1 | 1 | yes |
| 8 | `fence_door` | construction | wood x5 | 1 | yes |
| 9 | `firepit` | construction | wood x3, rock x2 | 1 | learn-only |
| 10 | `furnacemix` | construction | rock x3, metal_ore x2 | 1 | learn-only |
| 11 | `hat` | item | grass x5 | 1 | yes |
| 12 | `santa_hat` | item | grass x5 | 1 | learn-only |
| 13 | `station_alchemy` | construction | wood x3, gold_ore x2 | 1 | yes |
| 14 | `station_workshop` | construction | wood x3, metal_ore x2 | 1 | yes |
| 15 | `well` | construction | rock x5 | 1 | yes |
| 16 | `battle_axe` | item | wood x1, metal_bar x3 | 1 | yes |
| 17 | `helmet_wood` | item | wood x3, rock x1 | 1 | yes |
| 18 | `nutgun` | item | wood x1, metal_ore x1, gold_ore x1, acorn x1 | 1 | learn-only |
| 19 | `pokebowl` | item | fish_raw x1, rice x1, carrot x1, coconut_half x1 | 1 | yes |
| 20 | `armor_metal` | item | skin x1, metal_bar x2 | 1 | yes |

### 4.4 Missing original recipes - full list (72)

| Original id | Kind | Inputs | Fillers | Output | count | craftable |
|---|---|---|---|---|---|---|
| `barn` | construction | wood x7, grass x5 | - | `barn` | 1 | learn-only |
| `bbq` | construction | wood x3, rock x2 | - | `bbq` | 1 | yes |
| `bed` | construction | wood x3 | - | `bed` | 1 | yes |
| `candle` | construction | grass x2 | - | `candle` | 1 | yes |
| `chair` | construction | wood x2 | - | `chair` | 1 | yes |
| `chest_stone` | construction | rock x4, metal_ore x1 | - | `chest_stone` | 1 | yes |
| `chest_wood` | construction | wood x4, rock x1 | - | `chest_wood` | 1 | yes |
| `fence_door` | construction | wood x5 | - | `fence_door` | 1 | yes |
| `firepit` | construction | wood x3, rock x2 | - | `firepit` | 1 | learn-only |
| `flag` | construction | wood x1, grass x1 | - | `flag` | 1 | yes |
| `fridge` | construction | metal_ore x3 | - | `fridge` | 1 | learn-only |
| `furnacemix` | construction | rock x3, metal_ore x2 | - | `furnacemix` | 1 | learn-only |
| `hay_trough` | construction | wood x2 | - | `hay_trough` | 1 | yes |
| `house` | construction | wood x7, grass x5 | - | `house` | 1 | learn-only |
| `lure` | construction | wood x1, carrot x1 | - | `lure` | 1 | yes |
| `plant_pot` | construction | wood x1, rock x1 | - | `plant_pot` | 1 | yes |
| `roof` | construction | wood x3 | - | `roof` | 1 | yes |
| `stairs` | construction | wood x3 | - | `stairs` | 1 | yes |
| `station_alchemy` | construction | wood x3, gold_ore x2 | - | `station_alchemy` | 1 | yes |
| `station_workshop` | construction | wood x3, metal_ore x2 | - | `station_workshop` | 1 | yes |
| `table` | construction | wood x3 | - | `table` | 1 | yes |
| `trap` | construction | metal_ore x2 | - | `trap` | 1 | yes |
| `wall` | construction | wood x3 | - | `wall` | 1 | yes |
| `well` | construction | rock x5 | - | `well` | 1 | yes |
| `windmill` | construction | rock x3, wood x2, metal_ore x1 | - | `windmill` | 1 | yes |
| `armor_metal` | item | skin x1, metal_bar x2 | - | `armor_metal` | 1 | yes |
| `armor_wood` | item | skin x2, wood x3 | - | `armor_wood` | 1 | yes |
| `arrow` | item | wood x3 | - | `arrow` | 5 | yes |
| `backpack` | item | rope x1, skin x2 | - | `backpack` | 1 | yes |
| `baseball_bat` | item | wood x2 | - | `baseball_bat` | 1 | yes |
| `battle_axe` | item | wood x1, metal_bar x3 | - | `battle_axe` | 1 | yes |
| `boots` | item | skin x2, grass x1 | - | `boots` | 1 | yes |
| `bow` | item | wood x1, rope x1 | - | `bow` | 1 | yes |
| `chicken_curry` | item | chicken_raw x1, coconut_half x1, herbs x1 | - | `chicken_curry` | 1 | yes |
| `cleaver` | item | wood x1, rock x1 | - | `cleaver` | 1 | yes |
| `fish_n_chip` | item | fish_raw x1, potato x1 | - | `fish_n_chip` | 1 | yes |
| `gold_bar` | item | gold_ore x3 | - | `gold_bar` | 1 | yes |
| `ham_brunch` | item | ham_raw x1, egg x1, potato x1 | - | `ham_brunch` | 1 | yes |
| `hammer` | item | wood x1, rock x1 | - | `hammer` | 1 | yes |
| `hat` | item | grass x5 | - | `hat` | 1 | yes |
| `healing_mix` | item | herbs x1, water x1 | - | `healing_mix` | 1 | yes |
| `helmet_metal` | item | skin x1, metal_bar x1 | - | `helmet_metal` | 1 | yes |
| `helmet_wood` | item | wood x3, rock x1 | - | `helmet_wood` | 1 | yes |
| `hoe` | item | wood x1, rock x1 | - | `hoe` | 1 | yes |
| `jug` | item | rock x1 | - | `jug` | 1 | yes |
| `nachos` | item | corn x1, tomato x1 | - | `nachos` | 1 | yes |
| `nutgun` | item | wood x1, metal_ore x1, gold_ore x1, acorn x1 | - | `nutgun` | 1 | learn-only |
| `pitchfork` | item | wood x1, rock x1 | - | `pitchfork` | 1 | yes |
| `pokebowl` | item | fish_raw x1, rice x1, carrot x1, coconut_half x1 | - | `pokebowl` | 1 | yes |
| `pop_corn` | item | corn x2 | - | `pop_corn` | 1 | yes |
| `potion_black` | item | potion_vial x1, herbs x1, mushroom_black x1 | - | `potion_black` | 1 | yes |
| `potion_green` | item | potion_vial x1, herbs x1, mushroom_green x1 | - | `potion_green` | 1 | yes |
| `potion_orange` | item | potion_vial x1, herbs x1, carrot x1 | - | `potion_orange` | 1 | yes |
| `potion_purple` | item | potion_vial x1, herbs x1, blueberry x1 | - | `potion_purple` | 1 | yes |
| `potion_red` | item | potion_vial x1, herbs x1, mushroom_red x1 | - | `potion_red` | 1 | yes |
| `potion_vial` | item | rock x1 | - | `potion_vial` | 1 | yes |
| `potion_white` | item | potion_vial x1, herbs x1, coconut_half x1 | - | `potion_white` | 1 | yes |
| `potion_yellow` | item | potion_vial x1, herbs x1, corn x1 | - | `potion_yellow` | 1 | yes |
| `ring` | item | gold_bar x1, diamond x1 | - | `ring` | 1 | yes |
| `rope` | item | grass x3 | - | `rope` | 1 | yes |
| `santa_hat` | item | grass x5 | - | `santa_hat` | 1 | learn-only |
| `scythe` | item | wood x1, rock x1 | - | `scythe` | 1 | yes |
| `shovel` | item | wood x1, rock x1 | - | `shovel` | 1 | yes |
| `skewer` | item | meat_raw x1 | Veggie x1 | `skewer` | 1 | yes |
| `slingshot` | item | wood x1, grass x1 | - | `slingshot` | 1 | yes |
| `spaghetti` | item | wheat x1, tomato x1, meat_raw x1 | - | `spaghetti` | 1 | yes |
| `sword` | item | wood x1, metal_ore x2 | - | `sword` | 1 | yes |
| `taco` | item | corn x1, meat_raw x1 | - | `taco` | 1 | yes |
| `torch` | item | wood x1, grass x2 | - | `torch` | 1 | yes |
| `vest` | item | skin x1 | - | `vest` | 1 | yes |
| `watering_can` | item | rock x1, metal_ore x1 | - | `watering_can` | 1 | yes |
| `winter_boots` | item | skin x2, grass x1 | - | `winter_boots` | 1 | learn-only |

## 5. Data that is lost but is not a recipe

ActionCook assets convert a raw item into a cooked one at the Fire group. The port models exactly one of these five as a recipe (cooked_fish); the other four have no equivalent:

| From | To | Station | Action asset |
|---|---|---|---|
| `chicken_raw` | `chicken_cooked` | Fire | Actions/CookChicken.asset |
| `egg` | `egg_cooked` | Fire | Actions/CookEgg.asset |
| `fish_raw` | `fish_cooked` | Fire | Actions/CookFish.asset |
| `ham_raw` | `ham_cooked` | Fire | Actions/CookHam.asset |
| `meat_raw` | `meat_cooked` | Fire | Actions/CookMeat.asset |

Also not comparable today: equip_bonus, eat_bonus, eat_* stats, durability / armor / damage, groups membership (craft menu tabs) and character_data - FItemDef has no field for any of them.

## 6. Parse issues

None. All 348 *.asset files parsed: each is a single --- !u!114 &11400000 MonoBehaviour document with plain key: value scalars and inline {fileID, guid, type} reference flow mappings. 0 files skipped, 0 fields failed to read (the single dangling GUID noted in section 2 is a deleted asset, not a parse failure).

Two structures need care rather than a naive YAML reader:

1. Reference flow mappings wrap across lines when long (plant_prefab: {fileID: ..., guid: ..., then a newline and     type: 3}), so m_Script / icon GUIDs are read with a wrap-tolerant regex.
2. craft_items / craft_fillers / craft_requirements have no count field - counts are duplicate entries - so they are aggregated instead of read as key/value pairs.

