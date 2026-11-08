#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FarmStatsComponent.generated.h"

/**
 * Pure player-attribute rules (stamina / hunger / XP curve).
 * Kept free of UObject and world access so automation tests can call it directly.
 */
namespace FarmStatsLogic
{
	/** Hard ceiling used by the flat helpers; per-component max values may be lower. */
	inline constexpr float MaxStatValue = 100.0f;
	/** Owner speed (cm/s) above which stamina drains instead of regenerating. */
	inline constexpr float MovingSpeedThreshold = 20.0f;
	inline constexpr float MoveDrainPerSecond = 4.0f;
	inline constexpr float IdleRegenPerSecond = 2.0f;
	inline constexpr float HungerDrainPerSecond = 0.5f;
	/** Stamina drain multiplier while hunger is empty. */
	inline constexpr float StarvingDrainMultiplier = 2.0f;
	inline constexpr int32 XPPerLevel = 100;

	/** Level 1 at 0 XP, then one level per full 100 XP. */
	inline int32 LevelForXP(int32 XP) { return 1 + FMath::Max(0, XP) / XPPerLevel; }

	/** Total XP needed to reach a level; level 1 needs 0. */
	inline int32 XPForLevel(int32 Level) { return (FMath::Max(1, Level) - 1) * XPPerLevel; }

	/** Flat clamp into [0, 100], used by UI bars and tests. */
	inline float ClampStat(float V) { return FMath::Clamp(V, 0.0f, MaxStatValue); }

	/** Clamp into [0, Max]; a zero max pins the value to 0. */
	inline float ClampToMax(float V, float Max) { return FMath::Clamp(V, 0.0f, FMath::Max(0.0f, Max)); }

	/** Result of one passive tick: the new values plus the signed stamina change. */
	struct FStatsStep
	{
		float Stamina = MaxStatValue;
		float Hunger = MaxStatValue;
		/** Negative when stamina was spent, positive when regenerated. */
		float StaminaDelta = 0.0f;
	};

	/**
	 * One passive tick. Hunger drains first so starving doubles the stamina cost
	 * on the very tick it hits zero. Moving costs stamina, standing still restores it.
	 */
	inline FStatsStep StepStats(float Stamina, float Hunger, float MaxStamina, float MaxHunger, bool bOwnerMoving, float DeltaSeconds)
	{
		const float Dt = FMath::Max(0.0f, DeltaSeconds);
		FStatsStep Step;
		Step.Hunger = ClampToMax(Hunger - HungerDrainPerSecond * Dt, MaxHunger);
		const bool bStarving = Step.Hunger <= 0.0f;
		const float StaminaRate = bOwnerMoving
			? -MoveDrainPerSecond * (bStarving ? StarvingDrainMultiplier : 1.0f)
			: IdleRegenPerSecond;
		Step.Stamina = ClampToMax(Stamina + StaminaRate * Dt, MaxStamina);
		Step.StaminaDelta = Step.Stamina - Stamina;
		return Step;
	}
}

/** Player stamina, hunger, XP and level. Ticks passively on the owning pawn. */
UCLASS(ClassGroup = (Farm), meta = (BlueprintSpawnableComponent))
class FARMGAMEUE_API UFarmStatsComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UFarmStatsComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Farm") float Stamina = 100.0f;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Farm") float Hunger = 100.0f;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Farm") float MaxStamina = 100.0f;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Farm") float MaxHunger = 100.0f;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Farm") int32 XP = 0;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Farm") int32 Level = 1;

	/** Adds XP and recomputes Level from the total, so out-of-order grants are safe. */
	UFUNCTION(BlueprintCallable, Category = "Farm") void AddXP(int32 Amount);

	/** Spends stamina; returns false and changes nothing when there is not enough. */
	UFUNCTION(BlueprintCallable, Category = "Farm") bool ConsumeStamina(float Amount);

	/** Refills hunger, clamped to MaxHunger. */
	UFUNCTION(BlueprintCallable, Category = "Farm") void Feed(float Amount);

	UFUNCTION(BlueprintPure, Category = "Farm") float GetStamina() const { return Stamina; }
	UFUNCTION(BlueprintPure, Category = "Farm") float GetHunger() const { return Hunger; }
	UFUNCTION(BlueprintPure, Category = "Farm") float GetMaxStamina() const { return MaxStamina; }
	UFUNCTION(BlueprintPure, Category = "Farm") float GetMaxHunger() const { return MaxHunger; }
	UFUNCTION(BlueprintPure, Category = "Farm") int32 GetLevel() const { return Level; }
	UFUNCTION(BlueprintPure, Category = "Farm") int32 GetXP() const { return XP; }

	/** XP still missing before the next level. */
	UFUNCTION(BlueprintPure, Category = "Farm") int32 GetXPToNextLevel() const;

	/** Hunger at zero, i.e. stamina drain is doubled. */
	UFUNCTION(BlueprintPure, Category = "Farm") bool IsStarving() const { return Hunger <= 0.0f; }

	/** True when the owning pawn moves fast enough to burn stamina. */
	UFUNCTION(BlueprintPure, Category = "Farm") bool IsOwnerMoving() const;

	/** Back to full stats, level 1, zero XP. */
	void ResetForNewGame();
};
