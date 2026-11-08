#include "FarmStatsComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"

UFarmStatsComponent::UFarmStatsComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PrePhysics;
}

void UFarmStatsComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	const FarmStatsLogic::FStatsStep Step = FarmStatsLogic::StepStats(Stamina, Hunger, MaxStamina, MaxHunger, IsOwnerMoving(), DeltaTime);
	Stamina = Step.Stamina;
	Hunger = Step.Hunger;
}

void UFarmStatsComponent::AddXP(int32 Amount)
{
	if (Amount <= 0) return;
	XP += Amount;
	Level = FarmStatsLogic::LevelForXP(XP);
}

bool UFarmStatsComponent::ConsumeStamina(float Amount)
{
	if (Amount <= 0.0f) return true;
	if (Stamina < Amount) return false;
	Stamina = FarmStatsLogic::ClampToMax(Stamina - Amount, MaxStamina);
	return true;
}

void UFarmStatsComponent::Feed(float Amount)
{
	if (Amount <= 0.0f) return;
	Hunger = FarmStatsLogic::ClampToMax(Hunger + Amount, MaxHunger);
}

int32 UFarmStatsComponent::GetXPToNextLevel() const
{
	return FMath::Max(0, FarmStatsLogic::XPForLevel(Level + 1) - XP);
}

bool UFarmStatsComponent::IsOwnerMoving() const
{
	const AActor* Owner = GetOwner();
	if (!Owner) return false;
	const APawn* Pawn = Cast<APawn>(Owner);
	return Pawn && Pawn->GetVelocity().Size2D() > FarmStatsLogic::MovingSpeedThreshold;
}

void UFarmStatsComponent::ResetForNewGame()
{
	Stamina = MaxStamina;
	Hunger = MaxHunger;
	XP = 0;
	Level = FarmStatsLogic::LevelForXP(0);
}
