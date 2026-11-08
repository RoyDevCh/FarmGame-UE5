#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FarmSceneDoor.generated.h"

class UStaticMeshComponent;
class AFarmCharacter;
class AFarmPlayerController;

/** Door / exit zone that travels to another level. */
UCLASS()
class FARMGAMEUE_API AFarmSceneDoor : public AActor
{
	GENERATED_BODY()

public:
	AFarmSceneDoor();

	virtual void BeginPlay() override;

	FString GetPrompt(const AFarmPlayerController* PC) const;
	void Interact(AFarmCharacter* Character);

	UPROPERTY(EditAnywhere) FName TargetMap = TEXT("/Game/Maps/Farm");
	UPROPERTY(EditAnywhere) FString SpawnOption = TEXT("farm");

	UPROPERTY(VisibleAnywhere) USceneComponent* SceneRoot;
	UPROPERTY(VisibleAnywhere) UStaticMeshComponent* Mesh;
};
