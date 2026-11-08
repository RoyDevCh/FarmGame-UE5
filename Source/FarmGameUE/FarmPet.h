#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FarmPet.generated.h"

class UStaticMeshComponent;
class AFarmCharacter;
class AFarmPlayerController;

/** Pure pet rules, kept free of UObject/world so they are unit-testable. */
namespace FarmPetLogic
{
	/** What a tamed pet is currently doing. */
	enum class EPetMode : uint8
	{
		Follow,
		Stay
	};

	/** Switches between follow and stay. */
	inline EPetMode ToggleMode(EPetMode Mode)
	{
		return Mode == EPetMode::Follow ? EPetMode::Stay : EPetMode::Follow;
	}

	/** Roll is 0..99; the pet is tamed when Roll < Threshold. */
	inline bool TryTame(int32 Roll, int32 Threshold)
	{
		return Threshold > 0 && Roll >= 0 && Roll < Threshold;
	}
}

/** Tameable pet: 50% tame chance, then follows the farmer or waits in place. */
UCLASS()
class FARMGAMEUE_API AFarmPet : public AActor
{
	GENERATED_BODY()

public:
	AFarmPet();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	void SetFollowTarget(AActor* InTarget) { FollowTarget = InTarget; }

	/** Tame attempt when wild, follow/stay toggle afterwards. */
	void Interact(AFarmCharacter* Character);

	/** Interaction label for the HUD, empty when the caller is farther away than FollowRadius. */
	FString GetPrompt(const AFarmPlayerController* PC) const;

	bool IsTamed() const { return bTamed; }
	FarmPetLogic::EPetMode GetMode() const { return Mode; }

	/** The pet only reports its prompt to a farmer inside this radius (cm). */
	UPROPERTY(EditAnywhere, Category = "Farm") float FollowRadius = 450.f;
	UPROPERTY() AActor* FollowTarget = nullptr;

protected:
	UPROPERTY(VisibleAnywhere) USceneComponent* SceneRoot;
	UPROPERTY(VisibleAnywhere) UStaticMeshComponent* Body;

	bool bTamed = false;
	FarmPetLogic::EPetMode Mode = FarmPetLogic::EPetMode::Follow;
};
