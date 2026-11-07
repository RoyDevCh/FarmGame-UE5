#pragma once

#include "CoreMinimal.h"
#include "FarmTypes.h"

/** Default crop catalog (also used by automation tests). */
namespace FarmCrops
{
	inline TArray<FCropDef> MakeDefaults()
	{
		TArray<FCropDef> Crops;

		FCropDef Carrot;
		Carrot.Id = TEXT("carrot");
		Carrot.DisplayName = FText::FromString(TEXT("Carrot"));
		Carrot.Stages = 3;
		Carrot.DaysPerStage = 1;
		Carrot.SeedCost = 3;
		Carrot.SellPrice = 8;
		Carrot.YieldPerHarvest = 1;
		Carrot.Color = FLinearColor(0.95f, 0.45f, 0.08f);
		Crops.Add(Carrot);

		FCropDef Potato;
		Potato.Id = TEXT("potato");
		Potato.DisplayName = FText::FromString(TEXT("Potato"));
		Potato.Stages = 3;
		Potato.DaysPerStage = 2;
		Potato.SeedCost = 5;
		Potato.SellPrice = 14;
		Potato.YieldPerHarvest = 2;
		Potato.Color = FLinearColor(0.75f, 0.62f, 0.35f);
		Crops.Add(Potato);

		FCropDef Corn;
		Corn.Id = TEXT("corn");
		Corn.DisplayName = FText::FromString(TEXT("Corn"));
		Corn.Stages = 4;
		Corn.DaysPerStage = 1;
		Corn.SeedCost = 6;
		Corn.SellPrice = 12;
		Corn.YieldPerHarvest = 2;
		Corn.Color = FLinearColor(0.95f, 0.85f, 0.2f);
		Crops.Add(Corn);

		FCropDef Tomato;
		Tomato.Id = TEXT("tomato");
		Tomato.DisplayName = FText::FromString(TEXT("Tomato"));
		Tomato.Stages = 3;
		Tomato.DaysPerStage = 2;
		Tomato.SeedCost = 7;
		Tomato.SellPrice = 18;
		Tomato.YieldPerHarvest = 2;
		Tomato.bRegrowAfterHarvest = true;
		Tomato.Color = FLinearColor(0.85f, 0.18f, 0.12f);
		Crops.Add(Tomato);

		FCropDef Pumpkin;
		Pumpkin.Id = TEXT("pumpkin");
		Pumpkin.DisplayName = FText::FromString(TEXT("Pumpkin"));
		Pumpkin.Stages = 4;
		Pumpkin.DaysPerStage = 2;
		Pumpkin.SeedCost = 10;
		Pumpkin.SellPrice = 26;
		Pumpkin.YieldPerHarvest = 1;
		Pumpkin.Color = FLinearColor(0.9f, 0.55f, 0.1f);
		Crops.Add(Pumpkin);

		return Crops;
	}

	/** Gold earned by one full harvest minus the seed cost. */
	inline int32 ProfitPerHarvest(const FCropDef& Crop)
	{
		return Crop.SellPrice * FMath::Max(1, Crop.YieldPerHarvest) - Crop.SeedCost;
	}
}
