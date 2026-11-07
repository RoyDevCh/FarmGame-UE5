#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FarmTypes.h"
#include "FarmInventoryComponent.generated.h"

/** Money, seeds and harvested produce owned by the player. */
UCLASS(ClassGroup = (Farm), meta = (BlueprintSpawnableComponent))
class FARMGAMEUE_API UFarmInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UFarmInventoryComponent();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Farm") int32 Gold = 20;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Farm") TMap<FName, int32> Seeds;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Farm") TMap<FName, int32> Produce;

	bool CanAfford(int32 Cost) const { return Gold >= Cost; }

	/** Buys seeds, returns false (and changes nothing) when the player cannot afford it. */
	UFUNCTION(BlueprintCallable, Category = "Farm") bool BuySeed(FName CropId, int32 SeedCost, int32 Amount = 1);

	/** Removes one seed; returns false when none are left. */
	UFUNCTION(BlueprintCallable, Category = "Farm") bool ConsumeSeed(FName CropId, int32 Amount = 1);

	UFUNCTION(BlueprintCallable, Category = "Farm") void AddSeed(FName CropId, int32 Amount);

	UFUNCTION(BlueprintCallable, Category = "Farm") void AddProduce(FName CropId, int32 Amount);

	UFUNCTION(BlueprintPure, Category = "Farm") int32 GetSeedCount(FName CropId) const;

	UFUNCTION(BlueprintPure, Category = "Farm") int32 GetProduceCount(FName CropId) const;

	/** Sells every harvested item, returns the gold earned. */
	UFUNCTION(BlueprintCallable, Category = "Farm") int32 SellAllProduce(const TArray<FCropDef>& Crops);

	UFUNCTION(BlueprintPure, Category = "Farm") int32 GetTotalProduce() const;

	void ResetForNewGame();
};
