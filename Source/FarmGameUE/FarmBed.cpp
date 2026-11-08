#include "FarmBed.h"
#include "FarmAudio.h"
#include "FarmCharacter.h"
#include "FarmGameMode.h"
#include "FarmPlayerController.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"

AFarmBed::AFarmBed()
{
	PrimaryActorTick.bCanEverTick = false;
	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(SceneRoot);
	Mesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Mesh->SetCollisionResponseToAllChannels(ECR_Block);

	Pillow = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Pillow"));
	Pillow->SetupAttachment(SceneRoot);
	Pillow->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AFarmBed::BeginPlay()
{
	Super::BeginPlay();
	if (UStaticMesh* Cube = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube")))
	{
		Mesh->SetStaticMesh(Cube);
		Pillow->SetStaticMesh(Cube);
	}
	Mesh->SetRelativeScale3D(FVector(2.2f, 1.2f, 0.35f));
	Mesh->SetRelativeLocation(FVector(0.f, 0.f, 35.f));
	Pillow->SetRelativeScale3D(FVector(0.5f, 0.9f, 0.2f));
	Pillow->SetRelativeLocation(FVector(-0.7f, 0.f, 75.f));
}

FString AFarmBed::GetPrompt(const AFarmPlayerController* PC) const
{
	return TEXT("E: Sleep until tomorrow");
}

void AFarmBed::Interact(AFarmCharacter* Character)
{
	if (!Character) return;
	if (AFarmGameMode* GM = GetWorld() ? GetWorld()->GetAuthGameMode<AFarmGameMode>() : nullptr)
	{
		GM->AdvanceDay();
	}
	Character->Heal(20);
	FarmAudio::Play(this, TEXT("/Game/Art/Audio/pop.pop"), 0.6f);
	if (AFarmPlayerController* PC = Character->GetController<AFarmPlayerController>())
	{
		PC->ShowToast(TEXT("Slept until tomorrow (+20 HP)"));
	}
}
