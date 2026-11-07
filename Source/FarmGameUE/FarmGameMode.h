#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "FarmTypes.h"
#include "FarmBuilding.h"
#include "FarmGameMode.generated.h"

class AFarmPlot;
class AFarmBin;
class AFarmCharacter;
class AFarmBuilding;
class AFarmResourceNode;
class AFarmAnimal;
class AFarmPortal;
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
	UPROPERTY() TArray<AFarmBuilding*> Buildings;
	UPROPERTY() TArray<AFarmResourceNode*> ResourceNodes;
	UPROPERTY() TArray<AFarmAnimal*> Animals;
	UPROPERTY() TArray<AFarmPortal*> Portals;

	const FCropDef* FindCrop(FName Id) const;
	int32 GetDay() const { return Day; }
	AFarmBin* GetBin() const { return Bin; }
	float GetDayLengthSeconds() const { return DayLengthSeconds; }
	/** 0..1 progress through the current day. */
	float GetDayProgress() const { return DayLengthSeconds > 0.f ? FMath::Clamp(DayTimer / DayLengthSeconds, 0.f, 1.f) : 0.f; }
	bool IsRaining() const { return bRaining; }
	FString GetWeatherLabel() const { return bRaining ? TEXT("Rainy") : TEXT("Sunny"); }
	FString GetTimeLabel() const;

	void AdvanceDay();
	void InitializePlayer(AFarmCharacter* Character);
	void RegisterBuilding(AFarmBuilding* Building);
	/** Rejects placement too close to plots, buildings or the bin. */
	bool IsBuildLocationFree(const FVector& Location, float Radius = 130.f) const;
	AFarmBuilding* SpawnBuilding(EBuildingType Type, const FVector& Location);
	void SaveGame(AFarmCharacter* Character);
	bool LoadGame(AFarmCharacter* Character);

protected:
	UPROPERTY() int32 Day = 1;
	UPROPERTY() float DayLengthSeconds = 180.f;
	float DayTimer = 0.f;
	UPROPERTY() AFarmBin* Bin = nullptr;
	UPROPERTY() class ADirectionalLight* SunLight = nullptr;
	bool bRaining = false;
	float TotalGameMinutes = 0.f;
	bool bPlayerInitialized = false;

	void BuildWorld();
	void SpawnEnvironment();
	/** Spawns a decorative static mesh scaled so its largest dimension equals Size. */
	void SpawnProp(const TCHAR* Path, const FVector& Location, float Size, float Yaw = 0.f);
};
