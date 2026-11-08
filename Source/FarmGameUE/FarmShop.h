#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FarmShop.generated.h"

class UStaticMeshComponent;
class AFarmCharacter;
class AFarmPlayerController;

/** Village kiosk: buy seeds, hay and ore for gold. */
UCLASS()
class FARMGAMEUE_API AFarmShop : public AActor
{
	GENERATED_BODY()

public:
	AFarmShop();

	virtual void BeginPlay() override;

	FString GetPrompt(const AFarmPlayerController* PC) const;
	void Interact(AFarmCharacter* Character);

	UPROPERTY(VisibleAnywhere) USceneComponent* SceneRoot;
	UPROPERTY(VisibleAnywhere) UStaticMeshComponent* Mesh;
};
