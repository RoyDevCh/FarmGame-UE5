#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "FarmTypes.h"
#include "FarmSaveGame.generated.h"

UCLASS()
class FARMGAMEUE_API UFarmSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	UPROPERTY() int32 Day = 1;
	UPROPERTY() int32 Gold = 20;
	UPROPERTY() TMap<FName, int32> Items;
	UPROPERTY() TArray<FPlotSaveData> Plots;
	UPROPERTY() TArray<FBuildingSaveData> Buildings;
	UPROPERTY() TArray<FAnimalSaveData> Animals;
	UPROPERTY() float DayTimer = 0.f;
	UPROPERTY() bool bRaining = false;
	UPROPERTY() int32 PlayerHealth = 100;
};
