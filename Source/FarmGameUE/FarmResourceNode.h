#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FarmResourceNode.generated.h"

class UStaticMeshComponent;
class AFarmCharacter;
class AFarmPlayerController;

UENUM()
enum class EResourceType : uint8
{
	Tree,
	Rock,
	Ore
};

/** Gatherable tree / rock / ore vein. */
UCLASS()
class FARMGAMEUE_API AFarmResourceNode : public AActor
{
	GENERATED_BODY()

public:
	AFarmResourceNode();

	void Configure(EResourceType InType, int32 InAmount);
	void Interact(AFarmCharacter* Character);
	FString GetPrompt(const AFarmPlayerController* PC) const;
	void Regrow();

	EResourceType GetResourceType() const { return Type; }
	int32 GetRemaining() const { return Remaining; }

protected:
	UPROPERTY(VisibleAnywhere) USceneComponent* SceneRoot;
	UPROPERTY(VisibleAnywhere) UStaticMeshComponent* Mesh;
	UPROPERTY(VisibleAnywhere) UStaticMeshComponent* Detail;
	EResourceType Type = EResourceType::Tree;
	int32 Remaining = 3;
	int32 MaxAmount = 3;

	void ApplyVisual();
};
