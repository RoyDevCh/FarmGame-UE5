#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FarmPortal.generated.h"

class UStaticMeshComponent;
class AFarmCharacter;
class AFarmPlayerController;

/** Teleports the player between the farm and the mine clearing. */
UCLASS()
class FARMGAMEUE_API AFarmPortal : public AActor
{
	GENERATED_BODY()

public:
	AFarmPortal();

	void Configure(const FText& InLabel, const FVector& InDestination);
	void Interact(AFarmCharacter* Character);
	FString GetPrompt() const;

protected:
	UPROPERTY(VisibleAnywhere) UStaticMeshComponent* Mesh;
	FText Label;
	FVector Destination = FVector::ZeroVector;
};
