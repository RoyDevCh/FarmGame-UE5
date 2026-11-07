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
	/** When true the plot returns to stage 1 after harvest instead of being cleared. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bRegrowAfterHarvest = false;
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

/** Serialized building state. */
USTRUCT()
struct FBuildingSaveData
{
	GENERATED_BODY()

	UPROPERTY() int32 Type = 0;
	UPROPERTY() FVector Location = FVector::ZeroVector;
	UPROPERTY() int32 OreCount = 0;
	UPROPERTY() int32 IngotCount = 0;
	UPROPERTY() TMap<FName, int32> Storage;
};

/** Serialized animal state. */
USTRUCT()
struct FAnimalSaveData
{
	GENERATED_BODY()

	UPROPERTY() int32 Type = 0;
	UPROPERTY() FVector Location = FVector::ZeroVector;
	UPROPERTY() bool bFed = false;
	UPROPERTY() bool bProductReady = false;
	UPROPERTY() int32 Health = 30;
};

/** Pure farming rules, kept free of UObject/world so they are unit-testable. */
namespace FarmLogic
{
	/** Delta days needed before the next stage. */
	inline int32 DaysForNextStage(const FCropDef& Crop) { return FMath::Max(1, Crop.DaysPerStage); }

	inline bool IsMature(const FCropDef& Crop, int32 Stage) { return Stage >= FMath::Max(1, Crop.Stages) - 1; }

	/** Stage a plot keeps after harvest; -1 means the crop is removed. */
	inline int32 StageAfterHarvest(const FCropDef& Crop) { return Crop.bRegrowAfterHarvest ? 1 : -1; }

	/** Furnace tick: moves one ore to an ingot every SecondsPerOre. */
	inline bool SmeltStep(int32& OreCount, int32& IngotCount, float& Timer, float DeltaSeconds, float SecondsPerOre)
	{
		if (OreCount <= 0 || SecondsPerOre <= 0.f) return false;
		Timer += DeltaSeconds;
		if (Timer < SecondsPerOre) return false;
		Timer -= SecondsPerOre;
		OreCount--;
		IngotCount++;
		return true;
	}

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
