#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "FarmQuests.h"
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
class AFarmShop;
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

	/** Advances the active objective when the player produces the requested item. */
	void NotifyProduced(FName ItemId, int32 Amount);
	const TArray<FQuestState>& GetQuests() const { return Quests; }
	int32 GetActiveQuestIndex() const { return ActiveQuestIndex; }

	/** Starts the simulation (leaves the main menu). */
	void StartGame() { bGameStarted = true; }
	void SetGamePaused(bool bPaused) { bGamePaused = bPaused; }
	bool IsGameStarted() const { return bGameStarted; }
	bool IsGamePaused() const { return bGamePaused; }
	AFarmBin* GetBin() const { return Bin; }
	AFarmShop* GetShop() const { return Shop; }
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

	void SetSaveSlot(int32 InSlot) { SaveSlotIndex = FMath::Clamp(InSlot, 1, 3); }
	int32 GetSaveSlot() const { return SaveSlotIndex; }
	FString SaveSlotName() const { return FString::Printf(TEXT("FarmSlot%d"), SaveSlotIndex); }
	bool HasSaveInSlot(int32 Slot) const;
	/** Reloads the current map as a fresh game. */
	void NewGame();
	/** True once right after NewGame reloaded the map. */
	bool ConsumeSkipMenu();

protected:
	UPROPERTY() int32 Day = 1;
	UPROPERTY() float DayLengthSeconds = 180.f;
	float DayTimer = 0.f;
	UPROPERTY() TArray<FQuestState> Quests;
	int32 ActiveQuestIndex = 0;
	int32 SaveSlotIndex = 1;
	bool bGameStarted = false;
	bool bGamePaused = false;
	UPROPERTY() AFarmBin* Bin = nullptr;
	UPROPERTY() AFarmShop* Shop = nullptr;
	UPROPERTY() class ADirectionalLight* SunLight = nullptr;
	bool bRaining = false;
	float TotalGameMinutes = 0.f;
	bool bPlayerInitialized = false;

	void BuildWorld();
	void SpawnEnvironment();
	/** Spawns a decorative static mesh scaled so its largest dimension equals Size. */
	void SpawnProp(const TCHAR* Path, const FVector& Location, float Size, float Yaw = 0.f);
};
