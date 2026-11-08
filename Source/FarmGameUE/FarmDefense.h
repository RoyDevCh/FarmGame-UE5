#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FarmDefense.generated.h"

class UStaticMeshComponent;
class AFarmAnimal;

/** Pure defense rules, kept free of UObject/world so they are unit-testable. */
namespace FarmDefenseLogic
{
	/** True when Dist is strictly inside Radius; a target on the edge counts as outside. */
	inline bool InRange(float Dist, float Radius) { return Dist < Radius; }

	/** True when Cooldown seconds have elapsed since LastFireTime. */
	inline bool ReadyToFire(float Now, float LastFireTime, float Cooldown) { return Now - LastFireTime >= Cooldown; }
}

/** Passive pressure plate: damages the first hostile animal in range, then disarms. */
UCLASS()
class FARMGAMEUE_API AFarmTrap : public AActor
{
	GENERATED_BODY()

public:
	AFarmTrap();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite) float Radius = 200.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 Damage = 40;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bArmed = true;

protected:
	UPROPERTY(VisibleAnywhere) USceneComponent* SceneRoot;
	UPROPERTY(VisibleAnywhere) UStaticMeshComponent* Body;
};

/** Passive tower: shoots the nearest hostile animal in range on a cooldown. */
UCLASS()
class FARMGAMEUE_API AFarmTower : public AActor
{
	GENERATED_BODY()

public:
	AFarmTower();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite) float Range = 900.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 Damage = 20;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float Cooldown = 1.5f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float LastFireTime = 0.f;

protected:
	UPROPERTY(VisibleAnywhere) USceneComponent* SceneRoot;
	UPROPERTY(VisibleAnywhere) UStaticMeshComponent* Body;
};
