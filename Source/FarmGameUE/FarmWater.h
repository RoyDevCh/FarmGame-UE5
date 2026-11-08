#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "UObject/WeakObjectPtrTemplates.h"
#include "FarmWater.generated.h"

class UBoxComponent;
class UStaticMeshComponent;
class UMaterialInterface;
class UPrimitiveComponent;
class AFarmCharacter;

/**
 * Traversal decisions in plain C++ so automation tests can exercise them without a
 * world. Shared by the water volume and the ladder.
 */
namespace FarmWaterLogic
{
	/** Dry-land walk speed (cm/s). Used as the swim-speed base and as the fallback on exit. */
	constexpr float BaseWalkSpeed = 600.f;

	/** Swimming runs at 45% of the walk speed (600 -> 270 cm/s). */
	constexpr float SwimSpeedMultiplier = 0.45f;

	/** 1.0 on dry land, 0.45 in water. */
	inline float SpeedMultiplier(bool bInWater)
	{
		return bInWater ? SwimSpeedMultiplier : 1.f;
	}

	/** What the farmer is doing right now; None is plain walking. */
	enum class ETraversal : uint8
	{
		None,
		Swim,
		Climb
	};

	/** A ladder always wins: climbing out of a pond must not read as swimming. */
	inline ETraversal TraversalFor(bool bInWater, bool bOnLadder)
	{
		if (bOnLadder) return ETraversal::Climb;
		if (bInWater) return ETraversal::Swim;
		return ETraversal::None;
	}
}

/** Speeds captured when a pawn enters the water, restored when it leaves. */
struct FWaterEntrySpeeds
{
	float WalkSpeed = 0.f;
	float SwimSpeed = 0.f;
};

/**
 * Pond volume: a 600x600x120 query-only box whose bottom sits on the actor origin,
 * so the actor is placed on the pond floor. Pawns that overlap it switch to
 * MOVE_Swimming at 0.45x speed; leaving switches them back to MOVE_Walking.
 */
UCLASS()
class FARMGAMEUE_API AFarmWater : public AActor
{
	GENERATED_BODY()

public:
	AFarmWater();

	virtual void BeginPlay() override;

	/** Overlap box (600x600x120) that decides who is swimming. */
	UPROPERTY(VisibleAnywhere, Category = "Farm|Water") UBoxComponent* Volume;

	/** Flat blue plane drawn at the water surface; no collision. */
	UPROPERTY(VisibleAnywhere, Category = "Farm|Water") UStaticMeshComponent* Mesh;

	/** Surface tint; alpha drives the translucent material's Opacity. */
	UPROPERTY(EditAnywhere, Category = "Farm|Water") FLinearColor WaterColor = FLinearColor(0.10f, 0.45f, 0.90f, 0.55f);

	/** Optional parent material override. Defaults to /Engine/EngineDebugMaterials/M_SimpleTranslucent. */
	UPROPERTY(EditAnywhere, Category = "Farm|Water") UMaterialInterface* WaterMaterial = nullptr;

protected:
	UFUNCTION()
	void OnVolumeBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnVolumeEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

private:
	/** Per-pawn speeds captured on entry, restored on exit. */
	TMap<TWeakObjectPtr<AFarmCharacter>, FWaterEntrySpeeds> EntrySpeeds;
};
