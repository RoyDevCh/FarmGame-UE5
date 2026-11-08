#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FarmCampfire.generated.h"

class UStaticMeshComponent;
class UPointLightComponent;
class AFarmCharacter;
class AFarmPlayerController;

/** Pure campfire rules, kept free of UObject/world so they are unit-testable. */
namespace FarmCampfireLogic
{
	/** HP restored per second: a fire that is out never heals. */
	inline float HealPerSecond(bool bLit) { return bLit ? 4.f : 0.f; }

	/** True when a lit fire is close enough to warm the player (the rim itself is outside). */
	inline bool ShouldHeal(bool bLit, float Dist, float Radius) { return bLit && Radius > 0.f && Dist < Radius; }
}

/** Campfire: light it to slowly heal the farmer standing nearby, put it out to stop. */
UCLASS()
class FARMGAMEUE_API AFarmCampfire : public AActor
{
	GENERATED_BODY()

public:
	AFarmCampfire();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	void Interact(AFarmCharacter* Character);
	FString GetPrompt(const AFarmPlayerController* PC) const;

	/** Turns the fire on/off and applies the matching light state. */
	void SetLit(bool bInLit);

	bool IsLit() const { return bLit; }
	float GetRadius() const { return Radius; }

	static float LitLightIntensity() { return 3000.f; }

	UPROPERTY(VisibleAnywhere) USceneComponent* SceneRoot;
	UPROPERTY(VisibleAnywhere) UStaticMeshComponent* Body;
	UPROPERTY(VisibleAnywhere) UPointLightComponent* Light;

	UPROPERTY(VisibleAnywhere) bool bLit = false;
	UPROPERTY(EditAnywhere) float Radius = 320.f;
	UPROPERTY(VisibleAnywhere) float HealAccumulator = 0.f;
};
