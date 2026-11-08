#pragma once

#include "CoreMinimal.h"
#include "FarmItems.h"

struct FCraftIngredient
{
	FName ItemId = NAME_None;
	int32 Count = 1;
};

struct FCraftRecipe
{
	FName Id;
	const TCHAR* Label;
	TArray<FCraftIngredient> Inputs;
	FName OutputId;
	int32 OutputCount;
	/** Requires standing near a furnace. */
	bool bNeedsFurnace;
	/** Requires standing near a mixing pot. */
	bool bNeedsMixingPot = false;
};

namespace FarmRecipes
{
	inline const TArray<FCraftRecipe>& All()
	{
		static const TArray<FCraftRecipe> Recipes = {
			{ TEXT("bread"), TEXT("Bread"), { { TEXT("flour"), 1 }, { TEXT("water"), 1 } }, TEXT("bread"), 1, true },
			{ TEXT("chicken_curry"), TEXT("Chicken Curry"), { { TEXT("chicken_raw"), 1 }, { TEXT("coconut_half"), 1 }, { TEXT("herbs"), 1 } }, TEXT("chicken_curry"), 1, true },
			{ TEXT("fish_n_chip"), TEXT("Fish & Chips"), { { TEXT("fish_raw"), 1 }, { TEXT("potato"), 1 } }, TEXT("fish_n_chip"), 1, true },
			{ TEXT("ham_brunch"), TEXT("Ham Brunch"), { { TEXT("ham_raw"), 1 }, { TEXT("egg"), 1 }, { TEXT("potato"), 1 } }, TEXT("ham_brunch"), 1, true },
			{ TEXT("nachos"), TEXT("Nachos N Salsa"), { { TEXT("corn"), 1 }, { TEXT("tomato"), 1 } }, TEXT("nachos"), 1, false },
			{ TEXT("pokebowl"), TEXT("Pokebowl"), { { TEXT("fish_raw"), 1 }, { TEXT("rice"), 1 }, { TEXT("carrot"), 1 }, { TEXT("coconut_half"), 1 } }, TEXT("pokebowl"), 1, false },
			{ TEXT("pop_corn"), TEXT("Pop Corn"), { { TEXT("corn"), 2 } }, TEXT("pop_corn"), 1, true },
			{ TEXT("skewer"), TEXT("Skewer"), { { TEXT("meat_raw"), 1 } }, TEXT("skewer"), 1, true },
			{ TEXT("soup"), TEXT("Soup"), { { TEXT("water"), 1 } }, TEXT("soup"), 1, true },
			{ TEXT("spaghetti"), TEXT("Spaghetti Bolognese"), { { TEXT("wheat"), 1 }, { TEXT("tomato"), 1 }, { TEXT("meat_raw"), 1 } }, TEXT("spaghetti"), 1, true },
			{ TEXT("taco"), TEXT("Beef Taco"), { { TEXT("corn"), 1 }, { TEXT("meat_raw"), 1 } }, TEXT("taco"), 1, true },
			{ TEXT("armor_metal"), TEXT("Metal Armor"), { { TEXT("skin"), 1 }, { TEXT("ingot"), 2 } }, TEXT("armor_metal"), 1, false },
			{ TEXT("armor_wood"), TEXT("Wood Armor"), { { TEXT("skin"), 2 }, { TEXT("wood"), 3 } }, TEXT("armor_wood"), 1, false },
			{ TEXT("axe"), TEXT("Axe"), { { TEXT("wood"), 1 }, { TEXT("stone"), 1 } }, TEXT("axe"), 1, false },
			{ TEXT("backpack"), TEXT("Backpack"), { { TEXT("rope"), 1 }, { TEXT("skin"), 2 } }, TEXT("backpack"), 1, false },
			{ TEXT("baseball_bat"), TEXT("Baseball Bat"), { { TEXT("wood"), 2 } }, TEXT("baseball_bat"), 1, false },
			{ TEXT("battle_axe"), TEXT("Battle Axe"), { { TEXT("wood"), 1 }, { TEXT("ingot"), 3 } }, TEXT("battle_axe"), 1, false },
			{ TEXT("boots"), TEXT("Boots"), { { TEXT("skin"), 2 }, { TEXT("grass"), 1 } }, TEXT("boots"), 1, false },
			{ TEXT("bow"), TEXT("Bow"), { { TEXT("wood"), 1 }, { TEXT("rope"), 1 } }, TEXT("bow"), 1, false },
			{ TEXT("cleaver"), TEXT("Cleaver"), { { TEXT("wood"), 1 }, { TEXT("stone"), 1 } }, TEXT("cleaver"), 1, false },
			{ TEXT("fishing_rod"), TEXT("Fishing Rod"), { { TEXT("wood"), 1 }, { TEXT("grass"), 1 } }, TEXT("fishing_rod"), 1, false },
			{ TEXT("hammer"), TEXT("Hammer"), { { TEXT("wood"), 1 }, { TEXT("stone"), 1 } }, TEXT("hammer"), 1, false },
			{ TEXT("hat"), TEXT("Hat"), { { TEXT("grass"), 5 } }, TEXT("hat"), 1, false },
			{ TEXT("helmet_metal"), TEXT("Metal Helmet"), { { TEXT("skin"), 1 }, { TEXT("ingot"), 1 } }, TEXT("helmet_metal"), 1, false },
			{ TEXT("helmet_wood"), TEXT("Wood Helmet"), { { TEXT("wood"), 3 }, { TEXT("stone"), 1 } }, TEXT("helmet_wood"), 1, false },
			{ TEXT("hoe"), TEXT("Hoe"), { { TEXT("wood"), 1 }, { TEXT("stone"), 1 } }, TEXT("hoe"), 1, false },
			{ TEXT("nutgun"), TEXT("Nut-gun"), { { TEXT("wood"), 1 }, { TEXT("ore"), 1 }, { TEXT("gold_ore"), 1 }, { TEXT("acorn"), 1 } }, TEXT("nutgun"), 1, false },
			{ TEXT("pickaxe"), TEXT("Pickaxe"), { { TEXT("wood"), 1 }, { TEXT("stone"), 1 } }, TEXT("pickaxe"), 1, false },
			{ TEXT("pitchfork"), TEXT("Pitchfork"), { { TEXT("wood"), 1 }, { TEXT("stone"), 1 } }, TEXT("pitchfork"), 1, false },
			{ TEXT("ring"), TEXT("Ring"), { { TEXT("gold_bar"), 1 }, { TEXT("diamond"), 1 } }, TEXT("ring"), 1, false },
			{ TEXT("santa_hat"), TEXT("Santa Hat"), { { TEXT("grass"), 5 } }, TEXT("santa_hat"), 1, false },
			{ TEXT("scythe"), TEXT("Scythe"), { { TEXT("wood"), 1 }, { TEXT("stone"), 1 } }, TEXT("scythe"), 1, false },
			{ TEXT("shovel"), TEXT("Shovel"), { { TEXT("wood"), 1 }, { TEXT("stone"), 1 } }, TEXT("shovel"), 1, false },
			{ TEXT("slingshot"), TEXT("Slingshot"), { { TEXT("wood"), 1 }, { TEXT("grass"), 1 } }, TEXT("slingshot"), 1, false },
			{ TEXT("sword"), TEXT("Sword"), { { TEXT("wood"), 1 }, { TEXT("ore"), 2 } }, TEXT("sword"), 1, false },
			{ TEXT("torch"), TEXT("Torch"), { { TEXT("wood"), 1 }, { TEXT("grass"), 2 } }, TEXT("torch"), 1, false },
			{ TEXT("vest"), TEXT("Vest"), { { TEXT("skin"), 1 } }, TEXT("vest"), 1, false },
			{ TEXT("watering_can"), TEXT("Watering Can"), { { TEXT("stone"), 1 }, { TEXT("ore"), 1 } }, TEXT("watering_can"), 1, false },
			{ TEXT("winter_boots"), TEXT("Winter Boots"), { { TEXT("skin"), 2 }, { TEXT("grass"), 1 } }, TEXT("winter_boots"), 1, false },
			{ TEXT("potion_black"), TEXT("Black Potion"), { { TEXT("potion_vial"), 1 }, { TEXT("herbs"), 1 }, { TEXT("mushroom_black"), 1 } }, TEXT("potion_black"), 1, false },
			{ TEXT("potion_vial"), TEXT("Vial"), { { TEXT("stone"), 1 } }, TEXT("potion_vial"), 1, false },
			{ TEXT("potion_green"), TEXT("Green Potion"), { { TEXT("potion_vial"), 1 }, { TEXT("herbs"), 1 }, { TEXT("mushroom_green"), 1 } }, TEXT("potion_green"), 1, false },
			{ TEXT("potion_orange"), TEXT("Orange Potion"), { { TEXT("potion_vial"), 1 }, { TEXT("herbs"), 1 }, { TEXT("carrot"), 1 } }, TEXT("potion_orange"), 1, false },
			{ TEXT("potion_purple"), TEXT("Purple Potion"), { { TEXT("potion_vial"), 1 }, { TEXT("herbs"), 1 }, { TEXT("blueberry"), 1 } }, TEXT("potion_purple"), 1, false },
			{ TEXT("potion_red"), TEXT("Red Potion"), { { TEXT("potion_vial"), 1 }, { TEXT("herbs"), 1 }, { TEXT("mushroom_red"), 1 } }, TEXT("potion_red"), 1, false },
			{ TEXT("potion_white"), TEXT("White Potion"), { { TEXT("potion_vial"), 1 }, { TEXT("herbs"), 1 }, { TEXT("coconut_half"), 1 } }, TEXT("potion_white"), 1, false },
			{ TEXT("potion_yellow"), TEXT("Yellow Potion"), { { TEXT("potion_vial"), 1 }, { TEXT("herbs"), 1 }, { TEXT("corn"), 1 } }, TEXT("potion_yellow"), 1, false },
			{ TEXT("arrow"), TEXT("Arrow"), { { TEXT("wood"), 3 } }, TEXT("arrow"), 5, false },
			{ TEXT("gold_bar"), TEXT("Gold Bar"), { { TEXT("gold_ore"), 3 } }, TEXT("gold_bar"), 1, false },
			{ TEXT("healing_mix"), TEXT("Healing Mixture"), { { TEXT("herbs"), 1 }, { TEXT("water"), 1 } }, TEXT("healing_mix"), 1, false },
			{ TEXT("jug"), TEXT("Jug"), { { TEXT("stone"), 1 } }, TEXT("jug"), 1, false },
			{ TEXT("ingot"), TEXT("Metal Bar"), { { TEXT("ore"), 3 } }, TEXT("ingot"), 1, false },
			{ TEXT("rope"), TEXT("Rope"), { { TEXT("grass"), 3 } }, TEXT("rope"), 1, false },
			{ TEXT("fence_kit"), TEXT("Fence Kit (build with G)"), { { TEXT("wood"), 5 } }, TEXT("fence_kit"), 1, false },
			{ TEXT("furnace_kit"), TEXT("Furnace Kit (build with G)"), { { TEXT("stone"), 10 }, { TEXT("wood"), 5 } }, TEXT("furnace_kit"), 1, false },
			{ TEXT("chest_kit"), TEXT("Storage Chest Kit (build with G)"), { { TEXT("wood"), 12 } }, TEXT("chest_kit"), 1, false },
			{ TEXT("steel_axe"), TEXT("Steel Axe"), { { TEXT("axe"), 1 }, { TEXT("ore"), 5 } }, TEXT("steel_axe"), 1, false },
			{ TEXT("steel_pickaxe"), TEXT("Steel Pickaxe"), { { TEXT("pickaxe"), 1 }, { TEXT("ore"), 5 } }, TEXT("steel_pickaxe"), 1, false },
			{ TEXT("cheese"), TEXT("Cheese (at mixing pot)"), { { TEXT("milk"), 2 } }, TEXT("cheese"), 1, false },
			{ TEXT("sprinkler_kit"), TEXT("Sprinkler Kit"), { { TEXT("stone"), 6 }, { TEXT("ore"), 2 } }, TEXT("sprinkler_kit"), 1, false },
			{ TEXT("mixing_pot_kit"), TEXT("Mixing Pot Kit (build with G)"), { { TEXT("stone"), 8 }, { TEXT("wood"), 6 } }, TEXT("mixing_pot_kit"), 1, false },
			{ TEXT("hay_bundle"), TEXT("Hay Bundle (animal feed)"), { { TEXT("corn"), 2 } }, TEXT("hay"), 2, false },
			{ TEXT("cooked_fish"), TEXT("Cooked Fish (at fire)"), { { TEXT("fish"), 1 } }, TEXT("cooked_fish"), 1, true },
		};
		return Recipes;
	}

	inline const FCraftRecipe* Find(FName Id)
	{
		return All().FindByPredicate([Id](const FCraftRecipe& Recipe) { return Recipe.Id == Id; });
	}
}
