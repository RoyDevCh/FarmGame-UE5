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
			{ TEXT("carrot_seed"), TEXT("Carrot Seed"), EItemKind::Seed, 3, 1 },
			{ TEXT("potato_seed"), TEXT("Potato Seed"), EItemKind::Seed, 5, 2 },
			{ TEXT("corn_seed"), TEXT("Corn Seed"), EItemKind::Seed, 6, 2 },
			{ TEXT("tomato_seed"), TEXT("Tomato Seed"), EItemKind::Seed, 7, 3 },
			{ TEXT("pumpkin_seed"), TEXT("Pumpkin Seed"), EItemKind::Seed, 10, 4 },
			{ TEXT("carrot"), TEXT("Carrot"), EItemKind::Produce, 0, 8 },
			{ TEXT("potato"), TEXT("Potato"), EItemKind::Produce, 0, 14 },
			{ TEXT("corn"), TEXT("Corn"), EItemKind::Produce, 0, 12 },
			{ TEXT("tomato"), TEXT("Tomato"), EItemKind::Produce, 0, 18 },
			{ TEXT("pumpkin"), TEXT("Pumpkin"), EItemKind::Produce, 0, 26 },
			{ TEXT("wood"), TEXT("Wood"), EItemKind::Resource, 2, 1 },
			{ TEXT("stone"), TEXT("Stone"), EItemKind::Resource, 3, 1 },
			{ TEXT("ore"), TEXT("Ore"), EItemKind::Resource, 0, 4 },
			{ TEXT("ingot"), TEXT("Iron Ingot"), EItemKind::Material, 0, 18 },
			{ TEXT("hay"), TEXT("Hay"), EItemKind::Resource, 4, 1 },
			{ TEXT("egg"), TEXT("Egg"), EItemKind::AnimalProduct, 0, 10 },
			{ TEXT("milk"), TEXT("Milk"), EItemKind::AnimalProduct, 0, 16 },
			{ TEXT("meat"), TEXT("Meat"), EItemKind::AnimalProduct, 0, 12 },
			{ TEXT("fence_kit"), TEXT("Fence Kit"), EItemKind::Material, 0, 0 },
			{ TEXT("furnace_kit"), TEXT("Furnace Kit"), EItemKind::Material, 0, 0 },
			{ TEXT("chest_kit"), TEXT("Chest Kit"), EItemKind::Material, 0, 0 },
			{ TEXT("sprinkler_kit"), TEXT("Sprinkler Kit"), EItemKind::Material, 0, 0 },
			{ TEXT("axe"), TEXT("Axe"), EItemKind::Material, 0, 0 },
			{ TEXT("pickaxe"), TEXT("Pickaxe"), EItemKind::Material, 0, 0 },
			{ TEXT("steel_axe"), TEXT("Steel Axe"), EItemKind::Material, 0, 0 },
			{ TEXT("apple"), TEXT("Apple"), EItemKind::Food, 5, 3 },
			{ TEXT("bread"), TEXT("Bread"), EItemKind::Food, 8, 4 },
			{ TEXT("steel_pickaxe"), TEXT("Steel Pickaxe"), EItemKind::Material, 0, 0 }
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
