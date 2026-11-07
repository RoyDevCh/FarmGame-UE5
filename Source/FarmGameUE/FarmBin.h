#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FarmBin.generated.h"

class UStaticMeshComponent;
class AFarmCharacter;
class AFarmPlayerController;

/** Shipping bin: sell all harvested produce. */
UCLASS()
class FARMGAMEUE_API AFarmBin : public AActor
{
	GENERATED_BODY()

public:
	AFarmBin();

	void Interact(AFarmCharacter* Character);
	FString GetPrompt(const AFarmPlayerController* PC) const;

protected:
	UPROPERTY(VisibleAnywhere) UStaticMeshComponent* Mesh;
};
