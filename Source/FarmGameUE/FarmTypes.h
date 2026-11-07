#pragma once

#include "CoreMinimal.h"
#include "FarmTypes.generated.h"

/** Visual/logical state of one farm plot. */
UENUM(BlueprintType)
enum class EFarmPlotState : uint8
{
	Untilled     UMETA(DisplayName = "Untilled"),
	Tilled       UMETA(DisplayName = "Tilled"),
	Planted      UMETA(DisplayName = "Planted"),
	Harvestable  UMETA(DisplayName = "Harvestable")
};

/** Data-driven definition of one crop. Mirrors Unity PlantData (id/title/stages/sell value). */
USTRUCT(BlueprintType)
struct FCropDef
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite) FName Id = NAME_None;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FText DisplayName;
	/** Number of growth stages, stage 0 is the seed and (Stages-1) is mature. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 Stages = 3;
	/** Game days needed to move one stage forward. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 DaysPerStage = 1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 SeedCost = 3;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 SellPrice = 8;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 YieldPerHarvest = 1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FLinearColor Color = FLinearColor(0.12f, 0.55f, 0.18f);
};

/** Serialized state of one plot (save game). */
USTRUCT()
struct FPlotSaveData
{
	GENERATED_BODY()

	UPROPERTY() int32 Index = 0;
	UPROPERTY() EFarmPlotState State = EFarmPlotState::Untilled;
	UPROPERTY() FName CropId = NAME_None;
	UPROPERTY() int32 Stage = 0;
	UPROPERTY() bool bWatered = false;
	UPROPERTY() int32 LastGrowthDay = 0;
};

/** Pure farming rules, kept free of UObject/world so they are unit-testable. */
namespace FarmLogic
{
	/** Delta days needed before the next stage. */
	inline int32 DaysForNextStage(const FCropDef& Crop) { return FMath::Max(1, Crop.DaysPerStage); }

	inline bool IsMature(const FCropDef& Crop, int32 Stage) { return Stage >= FMath::Max(1, Crop.Stages) - 1; }

	/**
	 * Advances a planted crop when the day changed and the plot was watered.
	 * Returns true when the stage changed; always clears watering on a day change.
	 */
	inline bool AdvanceCrop(const FCropDef& Crop, int32 Today, int32& InOutStage, int32& InOutLastGrowthDay, bool& bInOutWatered, bool bWasWateredToday)
	{
		if (!bWasWateredToday)
		{
			bInOutWatered = false;
			return false;
		}
		if (IsMature(Crop, InOutStage))
		{
			bInOutWatered = false;
			return false;
		}
		if (Today - InOutLastGrowthDay < DaysForNextStage(Crop))
		{
			return false; // not enough days yet, keep water
		}
		InOutStage = FMath::Min(InOutStage + 1, FMath::Max(1, Crop.Stages) - 1);
		InOutLastGrowthDay = Today;
		bInOutWatered = false;
		return true;
	}
}
