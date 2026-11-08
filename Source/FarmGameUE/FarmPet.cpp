#include "FarmPet.h"
#include "FarmCharacter.h"
#include "FarmPlayerController.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Pawn.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"

namespace
{
	/** Chase speed in cm/s. */
	constexpr float FollowSpeed = 220.f;
	/** The pet stops once it is this close to its target (cm). */
	constexpr float StopDistance = 150.f;
	/** Roll (0..99) must be below this for a successful tame: 50%. */
	constexpr int32 TameThreshold = 50;
}

AFarmPet::AFarmPet()
{
	PrimaryActorTick.bCanEverTick = true;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	Body = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Body"));
	Body->SetupAttachment(SceneRoot);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (Cube.Succeeded()) Body->SetStaticMesh(Cube.Object);
	Body->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	Body->SetRelativeScale3D(FVector(0.45f, 0.65f, 0.5f));
	Body->SetRelativeLocation(FVector(0.f, 0.f, 25.f));
}

void AFarmPet::BeginPlay()
{
	Super::BeginPlay();
	if (!Body) return;

	if (!Body->GetStaticMesh())
	{
		if (UStaticMesh* Cube = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube")))
		{
			Body->SetStaticMesh(Cube);
		}
	}

	UMaterialInterface* BaseMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
	if (UMaterialInstanceDynamic* MID = BaseMaterial ? UMaterialInstanceDynamic::Create(BaseMaterial, this) : nullptr)
	{
		MID->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.92f, 0.55f, 0.22f));
		Body->SetMaterial(0, MID);
	}
}

void AFarmPet::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (!bTamed || Mode != FarmPetLogic::EPetMode::Follow || !FollowTarget || DeltaSeconds <= 0.f) return;

	const FVector MyLocation = GetActorLocation();
	const FVector TargetLocation = FollowTarget->GetActorLocation();
	const float Distance = FVector::Dist2D(MyLocation, TargetLocation);
	if (Distance <= StopDistance) return;

	const FVector Dir = (TargetLocation - MyLocation).GetSafeNormal2D();
	if (Dir.IsNearlyZero()) return;

	const float Step = FMath::Min(FollowSpeed * DeltaSeconds, Distance - StopDistance);
	AddActorWorldOffset(Dir * Step);
	SetActorRotation(FRotator(0.f, Dir.Rotation().Yaw, 0.f));
}

FString AFarmPet::GetPrompt(const AFarmPlayerController* PC) const
{
	if (PC)
	{
		const APawn* Pawn = PC->GetPawn();
		if (Pawn && FVector::Dist2D(Pawn->GetActorLocation(), GetActorLocation()) > FollowRadius)
		{
			return FString();
		}
	}
	if (!bTamed)
	{
		return TEXT("E: Tame pet");
	}
	// The prompt names the action E performs, so a following pet offers "Stay".
	return Mode == FarmPetLogic::EPetMode::Follow ? TEXT("E: Stay") : TEXT("E: Follow");
}

void AFarmPet::Interact(AFarmCharacter* Character)
{
	if (!Character) return;
	AFarmPlayerController* PC = Character->GetController<AFarmPlayerController>();

	if (!bTamed)
	{
		const int32 Roll = FMath::RandRange(0, 99);
		bTamed = FarmPetLogic::TryTame(Roll, TameThreshold);
		UE_LOG(LogTemp, Log, TEXT("FarmGame: pet tame roll %d -> %s"), Roll, bTamed ? TEXT("tamed") : TEXT("refused"));
		if (!bTamed)
		{
			if (PC) PC->ShowToast(TEXT("The pet is shy - try again"));
			return;
		}
		Mode = FarmPetLogic::EPetMode::Follow;
		if (!FollowTarget) SetFollowTarget(Character);
		if (PC) PC->ShowToast(TEXT("Pet tamed - it will follow you"));
		return;
	}

	Mode = FarmPetLogic::ToggleMode(Mode);
	if (Mode == FarmPetLogic::EPetMode::Follow)
	{
		SetFollowTarget(Character);
	}
	if (PC)
	{
		PC->ShowToast(Mode == FarmPetLogic::EPetMode::Follow ? TEXT("Pet follows you") : TEXT("Pet stays here"));
	}
}
