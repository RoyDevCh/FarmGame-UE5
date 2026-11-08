#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FarmLadder.generated.h"

class USceneComponent;
class UStaticMeshComponent;
class AFarmCharacter;
class AFarmPlayerController;

/** Ladder rules in plain C++ so they can be reasoned about without a world. */
namespace FarmLadderLogic
{
	/** One climb moves the farmer this far up (cm). */
	constexpr float ClimbHeight = 420.f;

	/** The farmer must stand this close to the foot of the ladder (cm, horizontal). */
	constexpr float ClimbReach = 120.f;

	/** Horizontal reach check; the capsule centre sits above the feet, so Z is ignored. */
	inline bool CanClimb(const FVector& CharacterLocation, const FVector& LadderBase)
	{
		return FVector::Dist2D(CharacterLocation, LadderBase) <= ClimbReach;
	}
}

/**
 * Two-rail ladder prop: interacting teleports the farmer up ClimbHeight when they
 * stand at its foot. The PlayerController finds interactables by distance, so the
 * rails carry no collision.
 */
UCLASS()
class FARMGAMEUE_API AFarmLadder : public AActor
{
	GENERATED_BODY()

public:
	AFarmLadder();

	/** Climbs the character up when they are within ClimbReach of the base. */
	void Interact(AFarmCharacter* Character);

	FString GetPrompt(const AFarmPlayerController* PC = nullptr) const;

	UPROPERTY(VisibleAnywhere, Category = "Farm|Ladder") USceneComponent* SceneRoot;
	UPROPERTY(VisibleAnywhere, Category = "Farm|Ladder") UStaticMeshComponent* LeftRail;
	UPROPERTY(VisibleAnywhere, Category = "Farm|Ladder") UStaticMeshComponent* RightRail;
};
