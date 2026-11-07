#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "FarmTypes.h"
#include "FarmGameMode.generated.h"

class AFarmPlot;
class AFarmBin;
class AFarmCharacter;
class UFarmInventoryComponent;

/** Owns the farm world, crop catalog, day cycle and save/load. */
UCLASS()
class FARMGAMEUE_API AFarmGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AFarmGameMode();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual APawn* SpawnDefaultPawnAtTransform_Implementation(AController* NewPlayer, const FTransform& SpawnTransform) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Farm") TArray<FCropDef> Crops;
	UPROPERTY() TArray<AFarmPlot*> Plots;

	const FCropDef* FindCrop(FName Id) const;
	int32 GetDay() const { return Day; }
	AFarmBin* GetBin() const { return Bin; }
	float GetDayLengthSeconds() const { return DayLengthSeconds; }

	void AdvanceDay();
	void InitializePlayer(AFarmCharacter* Character);
	void SaveGame(UFarmInventoryComponent* Inventory);
	bool LoadGame(UFarmInventoryComponent* Inventory);

protected:
	UPROPERTY() int32 Day = 1;
	UPROPERTY() float DayLengthSeconds = 180.f;
	float DayTimer = 0.f;
	UPROPERTY() AFarmBin* Bin = nullptr;
	bool bPlayerInitialized = false;

	void BuildWorld();
	void SpawnEnvironment();
};
