#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FarmRideable.generated.h"

class UStaticMeshComponent;
class AFarmCharacter;
class AFarmPlayerController;

/** Pure mount rules, kept free of UObject/world so they are unit-testable. */
namespace FarmRideLogic
{
	/** A free mount can be ridden only from strictly inside its radius. */
	inline bool CanMount(bool bAlreadyRiding, float Dist, float Radius)
	{
		return !bAlreadyRiding && Dist < Radius;
	}
}

/** Rideable cow/horse: walk up, press E to ride, press E again to climb down. */
UCLASS()
class FARMGAMEUE_API AFarmRideable : public AActor
{
	GENERATED_BODY()

public:
	AFarmRideable();

	virtual void BeginPlay() override;

	/** Mounts Character when it is close enough and the saddle is free. */
	bool TryMount(AFarmCharacter* Character);

	/** Drops the rider beside the mount and restores the on-foot walk speed. */
	void Dismount();

	/** Interaction label for the HUD: ride when free, dismount when ridden. */
	FString GetPrompt(const AFarmPlayerController* PC) const;

	bool IsRidden() const { return bRidden; }
	AFarmCharacter* GetRider() const { return Rider; }

	/** The farmer may mount while strictly inside this radius (cm). */
	UPROPERTY(EditAnywhere, Category = "Farm") float MountRadius = 260.f;

protected:
	UPROPERTY(VisibleAnywhere, Category = "Farm") USceneComponent* SceneRoot;
	UPROPERTY(VisibleAnywhere, Category = "Farm") UStaticMeshComponent* Body;

	UPROPERTY(VisibleAnywhere, Category = "Farm") bool bRidden = false;
	UPROPERTY() AFarmCharacter* Rider = nullptr;
};
