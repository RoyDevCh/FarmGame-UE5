#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FarmItems.h"
#include "FarmRecipes.h"
#include "FarmInventoryComponent.generated.h"

/** Money and a unified item bag (seeds, produce, resources, animal products). */
UCLASS(ClassGroup = (Farm), meta = (BlueprintSpawnableComponent))
class FARMGAMEUE_API UFarmInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UFarmInventoryComponent();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Farm") int32 Gold = 20;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Farm") TMap<FName, int32> Items;

	UFUNCTION(BlueprintPure, Category = "Farm") int32 GetCount(FName ItemId) const;
	UFUNCTION(BlueprintCallable, Category = "Farm") void AddItem(FName ItemId, int32 Amount);
	UFUNCTION(BlueprintCallable, Category = "Farm") bool RemoveItem(FName ItemId, int32 Amount = 1);

	UFUNCTION(BlueprintPure, Category = "Farm") bool CanAfford(int32 Cost) const { return Gold >= Cost; }
	UFUNCTION(BlueprintCallable, Category = "Farm") bool SpendGold(int32 Cost);
	UFUNCTION(BlueprintCallable, Category = "Farm") void AddGold(int32 Amount);

	/** Buys N of an item at its catalog price. */
	UFUNCTION(BlueprintCallable, Category = "Farm") bool BuyItem(FName ItemId, int32 Amount = 1);

	/** Sells everything in the bag that has a sell price above zero. */
	UFUNCTION(BlueprintCallable, Category = "Farm") int32 SellEverything();

	bool CanCraft(const FCraftRecipe& Recipe) const;
	/** Consumes ingredients, adds outputs. */
	bool Craft(const FCraftRecipe& Recipe);

	UFUNCTION(BlueprintPure, Category = "Farm") int32 GetTotalItemCount() const;

	/** Items SellEverything would sell right now. */
	UFUNCTION(BlueprintPure, Category = "Farm") int32 GetSellableCount() const;

	/** Moves every Resource item into Storage; returns how many moved. */
	int32 DepositResources(TMap<FName, int32>& Storage);

	/** Moves everything in Storage back into the bag; returns how many moved. */
	int32 WithdrawResources(TMap<FName, int32>& Storage);

	void ResetForNewGame();
};
