#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FarmBed.generated.h"

class UStaticMeshComponent;
class AFarmCharacter;
class AFarmPlayerController;

/** Bed: sleep to advance one day and heal a little. */
UCLASS()
class FARMGAMEUE_API AFarmBed : public AActor
{
	GENERATED_BODY()

public:
	AFarmBed();

	virtual void BeginPlay() override;

	FString GetPrompt(const AFarmPlayerController* PC) const;
	void Interact(AFarmCharacter* Character);

	UPROPERTY(VisibleAnywhere) USceneComponent* SceneRoot;
	UPROPERTY(VisibleAnywhere) UStaticMeshComponent* Mesh;
	UPROPERTY(VisibleAnywhere) UStaticMeshComponent* Pillow;
};
