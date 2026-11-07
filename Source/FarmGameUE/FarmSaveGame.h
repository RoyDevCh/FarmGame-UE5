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
	UPROPERTY() TMap<FName, int32> Seeds;
	UPROPERTY() TMap<FName, int32> Produce;
	UPROPERTY() TArray<FPlotSaveData> Plots;
};
