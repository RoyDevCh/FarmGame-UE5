#include "FarmRideable.h"
#include "FarmArt.h"
#include "FarmCharacter.h"
#include "FarmPlayerController.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"

namespace
{
	/** Cruise speed while the farmer sits on the mount (cm/s). */
	constexpr float MountedWalkSpeed = 900.f;
	/** Walking speed restored when the farmer climbs down (cm/s). */
	constexpr float OnFootWalkSpeed = 600.f;
	/** Seat offset above the mount root, so the rider sits on its back (cm). */
	const FVector SeatOffset(0.f, 0.f, 120.f);
	/** Where the farmer lands when climbing down (cm, world offset). */
	const FVector DismountOffset(150.f, 0.f, 0.f);
}

AFarmRideable::AFarmRideable()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	Body = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Body"));
	Body->SetupAttachment(SceneRoot);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (Cube.Succeeded()) Body->SetStaticMesh(Cube.Object);
	Body->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	// Cow-sized fallback box until the imported mesh is applied in BeginPlay.
	Body->SetRelativeScale3D(FVector(1.0f, 1.5f, 0.9f));
	Body->SetRelativeLocation(FVector(0.f, 0.f, 45.f));
}

void AFarmRideable::BeginPlay()
{
	Super::BeginPlay();
	if (!Body) return;

	// Prefer the imported cow art; keep the constructor's cube when it is missing.
	if (FarmArt::ApplyHeight(Body, TEXT("/Game/Art/Animals/Cow.Cow"), 170.f))
	{
		return;
	}

	UMaterialInterface* BaseMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
	if (UMaterialInstanceDynamic* MID = BaseMaterial ? UMaterialInstanceDynamic::Create(BaseMaterial, this) : nullptr)
	{
		MID->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.85f, 0.8f, 0.75f));
		Body->SetMaterial(0, MID);
	}
}

bool AFarmRideable::TryMount(AFarmCharacter* Character)
{
	if (!Character || Rider)
	{
		return false;
	}

	const float Dist = FVector::Dist2D(GetActorLocation(), Character->GetActorLocation());
	if (!FarmRideLogic::CanMount(bRidden, Dist, MountRadius))
	{
		UE_LOG(LogTemp, Log, TEXT("FarmGame: mount refused (ridden=%d dist=%.0f radius=%.0f)"),
			bRidden ? 1 : 0, Dist, MountRadius);
		return false;
	}

	Rider = Character;
	bRidden = true;

	// KeepRelative keeps the farmer's facing, then the seat offset puts them on the back.
	Character->AttachToComponent(SceneRoot, FAttachmentTransformRules::KeepRelativeTransform);
	Character->SetActorRelativeLocation(SeatOffset);

	if (UCharacterMovementComponent* Movement = Character->GetCharacterMovement())
	{
		Movement->MaxWalkSpeed = MountedWalkSpeed;
	}

	if (AFarmPlayerController* PC = Character->GetController<AFarmPlayerController>())
	{
		PC->ShowToast(TEXT("Mounted"));
	}
	UE_LOG(LogTemp, Log, TEXT("FarmGame: mounted %s"), *GetNameSafe(Character));
	return true;
}

void AFarmRideable::Dismount()
{
	if (!Rider)
	{
		bRidden = false;
		return;
	}

	AFarmCharacter* Farmer = Rider;
	Rider = nullptr;
	bRidden = false;

	Farmer->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	if (UCharacterMovementComponent* Movement = Farmer->GetCharacterMovement())
	{
		Movement->MaxWalkSpeed = OnFootWalkSpeed;
	}
	// Land beside the mount instead of inside its collision.
	Farmer->SetActorLocation(GetActorLocation() + DismountOffset, false, nullptr, ETeleportType::TeleportPhysics);

	if (AFarmPlayerController* PC = Farmer->GetController<AFarmPlayerController>())
	{
		PC->ShowToast(TEXT("Dismounted"));
	}
	UE_LOG(LogTemp, Log, TEXT("FarmGame: dismounted %s"), *GetNameSafe(Farmer));
}

FString AFarmRideable::GetPrompt(const AFarmPlayerController* PC) const
{
	// Every interactable shares this signature; the controller already filters by range.
	(void)PC;
	return bRidden ? TEXT("E: Dismount") : TEXT("E: Ride");
}
