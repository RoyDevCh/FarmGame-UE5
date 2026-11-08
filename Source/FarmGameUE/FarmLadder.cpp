#include "FarmLadder.h"
#include "FarmCharacter.h"
#include "FarmPlayerController.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"

namespace
{
	/** Gap between the two rails and their height (cm). */
	constexpr float RailSpacing = 70.f;
	constexpr float RailHeight = 440.f;
	constexpr float RailThickness = 0.06f;
}

AFarmLadder::AFarmLadder()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(TEXT("/Engine/BasicShapes/Cube.Cube"));

	LeftRail = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LeftRail"));
	RightRail = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RightRail"));

	UStaticMeshComponent* Rails[2] = { LeftRail, RightRail };
	for (UStaticMeshComponent* Rail : Rails)
	{
		Rail->SetupAttachment(SceneRoot);
		if (Cube.Succeeded()) Rail->SetStaticMesh(Cube.Object);
		// A 100 cm cube becomes a 6 x 6 x 440 cm rail standing on the actor origin.
		Rail->SetRelativeScale3D(FVector(RailThickness, RailThickness, RailHeight / 100.f));
		Rail->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	LeftRail->SetRelativeLocation(FVector(0.f, -RailSpacing * 0.5f, RailHeight * 0.5f));
	RightRail->SetRelativeLocation(FVector(0.f, RailSpacing * 0.5f, RailHeight * 0.5f));
}

FString AFarmLadder::GetPrompt(const AFarmPlayerController* PC) const
{
	return TEXT("E: Climb ladder");
}

void AFarmLadder::Interact(AFarmCharacter* Character)
{
	if (!Character) return;

	// The ladder base is the actor origin (the rails stand on it).
	if (!FarmLadderLogic::CanClimb(Character->GetActorLocation(), GetActorLocation()))
	{
		return;
	}

	const FVector Destination = Character->GetActorLocation() + FVector(0.f, 0.f, FarmLadderLogic::ClimbHeight);
	Character->SetActorLocation(Destination, false, nullptr, ETeleportType::TeleportPhysics);
	UE_LOG(LogTemp, Log, TEXT("FarmGame: climbed ladder +%.0f"), FarmLadderLogic::ClimbHeight);

	if (AFarmPlayerController* PC = Character->GetController<AFarmPlayerController>())
	{
		PC->ShowToast(TEXT("Climbed up"));
	}
}
