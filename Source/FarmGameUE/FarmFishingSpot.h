#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FarmFishingSpot.generated.h"

class UStaticMeshComponent;
class AFarmCharacter;
class AFarmPlayerController;

/** Pond fishing spot: needs a fishing rod in the bag. */
UCLASS()
class FARMGAMEUE_API AFarmFishingSpot : public AActor
{
	GENERATED_BODY()

public:
	AFarmFishingSpot();

	virtual void BeginPlay() override;

	FString GetPrompt(const AFarmPlayerController* PC) const;
	void Interact(AFarmCharacter* Character);

	UPROPERTY(VisibleAnywhere) USceneComponent* SceneRoot;
	UPROPERTY(VisibleAnywhere) UStaticMeshComponent* Mesh;
};
