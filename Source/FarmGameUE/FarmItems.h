#pragma once

#include "CoreMinimal.h"

/** Broad category used by selling / crafting / UI filters. */
enum class EItemKind : uint8
{
	Seed,
	Produce,
	Resource,
	AnimalProduct,
	Material,
	Food
};

struct FItemDef
{
	FName Id;
	const TCHAR* Label;
	EItemKind Kind;
	int32 BuyPrice;
	int32 SellPrice;
};

namespace FarmItems
{
	inline const TArray<FItemDef>& All()
	{
		static const TArray<FItemDef> Items = {
			{ TEXT("carrot_seed"), TEXT("Carrot Seed"), EItemKind::Seed, 1, 1 },
			{ TEXT("potato_seed"), TEXT("Potato Seed"), EItemKind::Seed, 1, 1 },
			{ TEXT("corn_seed"), TEXT("Corn Seed"), EItemKind::Seed, 1, 1 },
			{ TEXT("tomato_seed"), TEXT("Tomato Seed"), EItemKind::Seed, 1, 1 },
			{ TEXT("pumpkin_seed"), TEXT("Pumpkin Seed"), EItemKind::Seed, 1, 1 },
			{ TEXT("carrot"), TEXT("Carrot"), EItemKind::Produce, 2, 2 },
			{ TEXT("potato"), TEXT("Potato"), EItemKind::Produce, 2, 2 },
			{ TEXT("corn"), TEXT("Corn"), EItemKind::Produce, 2, 2 },
			{ TEXT("tomato"), TEXT("Tomato"), EItemKind::Produce, 2, 2 },
			{ TEXT("pumpkin"), TEXT("Pumpkin"), EItemKind::Produce, 3, 3 },
			{ TEXT("wood"), TEXT("Wood"), EItemKind::Resource, 1, 1 },
			{ TEXT("stone"), TEXT("Stone"), EItemKind::Resource, 1, 1 },
			{ TEXT("ore"), TEXT("Ore"), EItemKind::Resource, 4, 4 },
			{ TEXT("ingot"), TEXT("Iron Ingot"), EItemKind::Material, 8, 8 },
			{ TEXT("hay"), TEXT("Hay"), EItemKind::Resource, 1, 1 },
			{ TEXT("egg"), TEXT("Egg"), EItemKind::AnimalProduct, 2, 2 },
			{ TEXT("milk"), TEXT("Milk"), EItemKind::AnimalProduct, 2, 2 },
			{ TEXT("meat"), TEXT("Meat"), EItemKind::AnimalProduct, 2, 2 },
			{ TEXT("fence_kit"), TEXT("Fence Kit"), EItemKind::Material, 0, 0 },
			{ TEXT("furnace_kit"), TEXT("Furnace Kit"), EItemKind::Material, 0, 0 },
			{ TEXT("chest_kit"), TEXT("Chest Kit"), EItemKind::Material, 0, 0 },
			{ TEXT("sprinkler_kit"), TEXT("Sprinkler Kit"), EItemKind::Material, 0, 0 },
			{ TEXT("mixing_pot_kit"), TEXT("Mixing Pot Kit"), EItemKind::Material, 0, 0 },
			{ TEXT("fishing_rod"), TEXT("Fishing Rod"), EItemKind::Material, 1, 1 },
			{ TEXT("fish"), TEXT("Fish"), EItemKind::Food, 0, 12 },
			{ TEXT("cooked_fish"), TEXT("Cooked Fish"), EItemKind::Food, 2, 2 },
			{ TEXT("cheese"), TEXT("Cheese"), EItemKind::Food, 0, 14 },
			{ TEXT("soup"), TEXT("Soup"), EItemKind::Food, 5, 5 },
			{ TEXT("axe"), TEXT("Axe"), EItemKind::Material, 1, 1 },
			{ TEXT("pickaxe"), TEXT("Pickaxe"), EItemKind::Material, 1, 1 },
			{ TEXT("steel_axe"), TEXT("Steel Axe"), EItemKind::Material, 0, 0 },
			{ TEXT("apple"), TEXT("Apple"), EItemKind::Food, 3, 3 },
			{ TEXT("bread"), TEXT("Bread"), EItemKind::Food, 4, 4 },
			{ TEXT("steel_pickaxe"), TEXT("Steel Pickaxe"), EItemKind::Material, 0, 0 },
			{ TEXT("acorn"), TEXT("Acorn"), EItemKind::Food, 1, 1 },
			{ TEXT("apple_seed"), TEXT("Apple Seed"), EItemKind::Seed, 1, 1 },
			{ TEXT("armor_metal"), TEXT("Metal Armor"), EItemKind::Material, 1, 1 },
			{ TEXT("armor_wood"), TEXT("Wood Armor"), EItemKind::Material, 1, 1 },
			{ TEXT("arrow"), TEXT("Arrow"), EItemKind::Resource, 1, 1 },
			{ TEXT("backpack"), TEXT("Backpack"), EItemKind::Material, 1, 1 },
			{ TEXT("bacon"), TEXT("Bacon"), EItemKind::Food, 2, 2 },
			{ TEXT("baseball_bat"), TEXT("Baseball Bat"), EItemKind::Material, 1, 1 },
			{ TEXT("battle_axe"), TEXT("Battle Axe"), EItemKind::Material, 1, 1 },
			{ TEXT("bean"), TEXT("Bean"), EItemKind::Food, 2, 2 },
			{ TEXT("bean_seed"), TEXT("Bean Seed"), EItemKind::Seed, 1, 1 },
			{ TEXT("blueberry"), TEXT("Blueberries"), EItemKind::Food, 2, 2 },
			{ TEXT("blueberry_seed"), TEXT("Blueberry Seed"), EItemKind::Seed, 1, 1 },
			{ TEXT("boots"), TEXT("Boots"), EItemKind::Material, 1, 1 },
			{ TEXT("bow"), TEXT("Bow"), EItemKind::Material, 1, 1 },
			{ TEXT("candle_item"), TEXT("Candle"), EItemKind::Resource, 1, 1 },
			{ TEXT("candy_black"), TEXT("Candy"), EItemKind::Food, 1, 1 },
			{ TEXT("candy_cane"), TEXT("Candy Cane"), EItemKind::Food, 1, 1 },
			{ TEXT("candy_orange"), TEXT("Candy"), EItemKind::Food, 1, 1 },
			{ TEXT("cherry"), TEXT("Cherry"), EItemKind::Food, 3, 3 },
			{ TEXT("cherry_seed"), TEXT("Cherry Seed"), EItemKind::Seed, 1, 1 },
			{ TEXT("chick_item"), TEXT("Chick"), EItemKind::Material, 5, 5 },
			{ TEXT("chicken_cooked"), TEXT("Chicken"), EItemKind::Food, 2, 2 },
			{ TEXT("chicken_curry"), TEXT("Chicken Curry"), EItemKind::Food, 5, 5 },
			{ TEXT("chicken_raw"), TEXT("Raw Chicken"), EItemKind::Food, 1, 1 },
			{ TEXT("cleaver"), TEXT("Cleaver"), EItemKind::Material, 1, 1 },
			{ TEXT("coconut"), TEXT("Coconut"), EItemKind::Food, 4, 4 },
			{ TEXT("coconut_half"), TEXT("Coconut"), EItemKind::Food, 1, 1 },
			{ TEXT("coin"), TEXT("Gold Coin"), EItemKind::Resource, 1, 1 },
			{ TEXT("cow_item"), TEXT("Cow"), EItemKind::Material, 10, 10 },
			{ TEXT("diamond"), TEXT("Diamond"), EItemKind::Resource, 100, 100 },
			{ TEXT("egg_cooked"), TEXT("Egg"), EItemKind::Food, 1, 1 },
			{ TEXT("fish_n_chip"), TEXT("Fish & Chips"), EItemKind::Food, 5, 5 },
			{ TEXT("fish_raw"), TEXT("Raw Fish"), EItemKind::Food, 2, 2 },
			{ TEXT("flag_item"), TEXT("Flag"), EItemKind::Resource, 1, 1 },
			{ TEXT("flour"), TEXT("Flour"), EItemKind::Food, 2, 2 },
			{ TEXT("gift"), TEXT("Gift"), EItemKind::Resource, 1, 1 },
			{ TEXT("gold_bar"), TEXT("Gold Bar"), EItemKind::Resource, 20, 20 },
			{ TEXT("gold_ore"), TEXT("Gold"), EItemKind::Resource, 10, 10 },
			{ TEXT("grass"), TEXT("Grass"), EItemKind::Resource, 1, 1 },
			{ TEXT("halloween_note"), TEXT("Happy Halloween!"), EItemKind::Resource, 1, 1 },
			{ TEXT("ham_brunch"), TEXT("Ham Brunch"), EItemKind::Food, 5, 5 },
			{ TEXT("ham_cooked"), TEXT("Ham"), EItemKind::Food, 2, 2 },
			{ TEXT("ham_raw"), TEXT("Raw Ham"), EItemKind::Food, 1, 1 },
			{ TEXT("hammer"), TEXT("Hammer"), EItemKind::Material, 1, 1 },
			{ TEXT("hat"), TEXT("Hat"), EItemKind::Material, 2, 2 },
			{ TEXT("hat_halloween"), TEXT("Hat Halloween"), EItemKind::Material, 5, 5 },
			{ TEXT("healing_mix"), TEXT("Healing Mixture"), EItemKind::Resource, 1, 1 },
			{ TEXT("helmet_metal"), TEXT("Metal Helmet"), EItemKind::Material, 1, 1 },
			{ TEXT("helmet_wood"), TEXT("Wood Helmet"), EItemKind::Material, 1, 1 },
			{ TEXT("herbs"), TEXT("Herbs"), EItemKind::Resource, 1, 1 },
			{ TEXT("hoe"), TEXT("Hoe"), EItemKind::Material, 1, 1 },
			{ TEXT("honey"), TEXT("Honey"), EItemKind::Food, 2, 2 },
			{ TEXT("horse_item"), TEXT("Horse"), EItemKind::Material, 40, 40 },
			{ TEXT("jug"), TEXT("Jug"), EItemKind::Resource, 1, 1 },
			{ TEXT("lure_item"), TEXT("Lure"), EItemKind::Resource, 1, 1 },
			{ TEXT("map"), TEXT("Treasure Map"), EItemKind::Resource, 0, 0 },
			{ TEXT("mask_halloween"), TEXT("Halloween Mask"), EItemKind::Resource, 5, 5 },
			{ TEXT("meat_raw"), TEXT("Raw Meat"), EItemKind::Food, 2, 2 },
			{ TEXT("mushroom_black"), TEXT("Mushroom"), EItemKind::Food, 66, 66 },
			{ TEXT("mushroom_green"), TEXT("Mushroom"), EItemKind::Food, 6, 6 },
			{ TEXT("mushroom_red"), TEXT("Mushroom"), EItemKind::Food, 6, 6 },
			{ TEXT("nachos"), TEXT("Nachos N Salsa"), EItemKind::Food, 5, 5 },
			{ TEXT("nutgun"), TEXT("Nut-gun"), EItemKind::Material, 1, 1 },
			{ TEXT("orange"), TEXT("Orange"), EItemKind::Food, 3, 3 },
			{ TEXT("orange_seed"), TEXT("Orange Seed"), EItemKind::Seed, 1, 1 },
			{ TEXT("pepper"), TEXT("Pepper"), EItemKind::Food, 2, 2 },
			{ TEXT("pepper_seed"), TEXT("Seed Pepper"), EItemKind::Seed, 1, 1 },
			{ TEXT("pig_item"), TEXT("Pig"), EItemKind::Material, 10, 10 },
			{ TEXT("pitchfork"), TEXT("Pitchfork"), EItemKind::Material, 1, 1 },
			{ TEXT("pokebowl"), TEXT("Pokebowl"), EItemKind::Food, 5, 5 },
			{ TEXT("pop_corn"), TEXT("Pop Corn"), EItemKind::Food, 3, 3 },
			{ TEXT("potion_black"), TEXT("Black Potion"), EItemKind::Food, 5, 5 },
			{ TEXT("potion_green"), TEXT("Green Potion"), EItemKind::Food, 5, 5 },
			{ TEXT("potion_orange"), TEXT("Orange Potion"), EItemKind::Food, 5, 5 },
			{ TEXT("potion_purple"), TEXT("Purple Potion"), EItemKind::Food, 5, 5 },
			{ TEXT("potion_red"), TEXT("Red Potion"), EItemKind::Food, 5, 5 },
			{ TEXT("potion_vial"), TEXT("Vial"), EItemKind::Food, 5, 5 },
			{ TEXT("potion_water"), TEXT("Blue Potion"), EItemKind::Food, 5, 5 },
			{ TEXT("potion_white"), TEXT("White Potion"), EItemKind::Food, 5, 5 },
			{ TEXT("potion_yellow"), TEXT("Yellow Potion"), EItemKind::Food, 5, 5 },
			{ TEXT("rabbit_item"), TEXT("Rabbit"), EItemKind::Material, 5, 5 },
			{ TEXT("raspberry"), TEXT("Raspberries"), EItemKind::Food, 2, 2 },
			{ TEXT("repair_kit"), TEXT("Repair Kit"), EItemKind::Resource, 5, 5 },
			{ TEXT("rice"), TEXT("Rice"), EItemKind::Food, 2, 2 },
			{ TEXT("rice_seed"), TEXT("Rice Seed"), EItemKind::Seed, 1, 1 },
			{ TEXT("ring"), TEXT("Ring"), EItemKind::Material, 1, 1 },
			{ TEXT("rope"), TEXT("Rope"), EItemKind::Resource, 1, 1 },
			{ TEXT("santa_hat"), TEXT("Santa Hat"), EItemKind::Material, 5, 5 },
			{ TEXT("scroll"), TEXT("Scroll"), EItemKind::Resource, 1, 1 },
			{ TEXT("scroll_alchemy"), TEXT("Alchemy Blueprint"), EItemKind::Resource, 1, 1 },
			{ TEXT("scroll_gun"), TEXT("Nutgun Blueprint"), EItemKind::Resource, 1, 1 },
			{ TEXT("scythe"), TEXT("Scythe"), EItemKind::Material, 1, 1 },
			{ TEXT("sheep_item"), TEXT("Sheep"), EItemKind::Material, 10, 10 },
			{ TEXT("shovel"), TEXT("Shovel"), EItemKind::Material, 1, 1 },
			{ TEXT("skewer"), TEXT("Skewer"), EItemKind::Food, 1, 1 },
			{ TEXT("skin"), TEXT("Skin"), EItemKind::Resource, 1, 1 },
			{ TEXT("slingshot"), TEXT("Slingshot"), EItemKind::Material, 1, 1 },
			{ TEXT("snowball"), TEXT("Snowball"), EItemKind::Resource, 0, 0 },
			{ TEXT("spaghetti"), TEXT("Spaghetti Bolognese"), EItemKind::Food, 5, 5 },
			{ TEXT("strawberry"), TEXT("Strawberry"), EItemKind::Food, 2, 2 },
			{ TEXT("strawberry_seed"), TEXT("Strawberry Seed"), EItemKind::Seed, 1, 1 },
			{ TEXT("sword"), TEXT("Sword"), EItemKind::Material, 4, 4 },
			{ TEXT("taco"), TEXT("Beef Taco"), EItemKind::Food, 5, 5 },
			{ TEXT("torch"), TEXT("Torch"), EItemKind::Material, 1, 1 },
			{ TEXT("trap_item"), TEXT("Trap"), EItemKind::Resource, 1, 1 },
			{ TEXT("tulip"), TEXT("Tulip"), EItemKind::Resource, 2, 2 },
			{ TEXT("tulip_seed"), TEXT("Tulip Seed"), EItemKind::Seed, 1, 1 },
			{ TEXT("vest"), TEXT("Vest"), EItemKind::Material, 1, 1 },
			{ TEXT("water"), TEXT("Water"), EItemKind::Food, 1, 1 },
			{ TEXT("watering_can"), TEXT("Watering Can"), EItemKind::Material, 1, 1 },
			{ TEXT("watering_can_full"), TEXT("Watering Can"), EItemKind::Material, 1, 1 },
			{ TEXT("wheat"), TEXT("Wheat"), EItemKind::Material, 2, 2 },
			{ TEXT("wheat_seed"), TEXT("Wheat Seed"), EItemKind::Seed, 1, 1 },
			{ TEXT("winter_boots"), TEXT("Winter Boots"), EItemKind::Material, 1, 1 },
			{ TEXT("wool"), TEXT("Wool"), EItemKind::Resource, 2, 2 },
		};
		return Items;
	}

	inline const FItemDef* Find(FName Id)
	{
		return All().FindByPredicate([Id](const FItemDef& Item) { return Item.Id == Id; });
	}

	inline FString LabelOf(FName Id)
	{
		const FItemDef* Def = Find(Id);
		return Def ? FString(Def->Label) : Id.ToString();
	}

	inline int32 SellPriceOf(FName Id)
	{
		const FItemDef* Def = Find(Id);
		return Def ? Def->SellPrice : 0;
	}

	inline bool IsSeed(FName Id)
	{
		const FItemDef* Def = Find(Id);
		return Def && Def->Kind == EItemKind::Seed;
	}

	inline FName SeedId(FName CropId)
	{
		return FName(*(CropId.ToString() + TEXT("_seed")));
	}
}
