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
};

namespace FarmRecipes
{
	inline const TArray<FCraftRecipe>& All()
	{
		static const TArray<FCraftRecipe> Recipes = {
			{ TEXT("fence_kit"), TEXT("Fence Kit (build with G)"), { { TEXT("wood"), 5 } }, TEXT("fence_kit"), 1, false },
			{ TEXT("furnace_kit"), TEXT("Furnace Kit (build with G)"), { { TEXT("stone"), 10 }, { TEXT("wood"), 5 } }, TEXT("furnace_kit"), 1, false },
			{ TEXT("chest_kit"), TEXT("Storage Chest Kit (build with G)"), { { TEXT("wood"), 12 } }, TEXT("chest_kit"), 1, false },
			{ TEXT("sprinkler_kit"), TEXT("Sprinkler Kit (waters nearby plots)"), { { TEXT("stone"), 6 }, { TEXT("ore"), 2 } }, TEXT("sprinkler_kit"), 1, false },
			{ TEXT("axe"), TEXT("Axe (double wood per chop)"), { { TEXT("wood"), 4 }, { TEXT("ore"), 1 } }, TEXT("axe"), 1, false },
			{ TEXT("pickaxe"), TEXT("Pickaxe (double stone/ore per hit)"), { { TEXT("wood"), 4 }, { TEXT("ore"), 2 } }, TEXT("pickaxe"), 1, false },
			{ TEXT("steel_axe"), TEXT("Steel Axe (triple wood per chop)"), { { TEXT("axe"), 1 }, { TEXT("ore"), 5 } }, TEXT("steel_axe"), 1, false },
			{ TEXT("steel_pickaxe"), TEXT("Steel Pickaxe (triple stone/ore)"), { { TEXT("pickaxe"), 1 }, { TEXT("ore"), 5 } }, TEXT("steel_pickaxe"), 1, false },
			{ TEXT("ingot"), TEXT("Smelt Iron Ingot (at furnace)"), { { TEXT("ore"), 2 } }, TEXT("ingot"), 1, true },
			{ TEXT("hay_bundle"), TEXT("Hay Bundle (animal feed)"), { { TEXT("corn"), 2 } }, TEXT("hay"), 2, false }
		};
		return Recipes;
	}

	inline const FCraftRecipe* Find(FName Id)
	{
		return All().FindByPredicate([Id](const FCraftRecipe& Recipe) { return Recipe.Id == Id; });
	}
}
