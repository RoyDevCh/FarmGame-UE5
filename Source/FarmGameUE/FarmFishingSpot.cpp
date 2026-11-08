#include "FarmFishingSpot.h"
#include "FarmAudio.h"
#include "FarmCharacter.h"
#include "FarmInventoryComponent.h"
#include "FarmItems.h"
#include "FarmPlayerController.h"
#include "Components/StaticMeshComponent.h"

AFarmFishingSpot::AFarmFishingSpot()
{
	PrimaryActorTick.bCanEverTick = false;
	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(SceneRoot);
	Mesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Mesh->SetCollisionResponseToAllChannels(ECR_Block);
}

void AFarmFishingSpot::BeginPlay()
{
	Super::BeginPlay();
	if (UStaticMesh* Cylinder = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cylinder.Cylinder")))
	{
		Mesh->SetStaticMesh(Cylinder);
	}
	Mesh->SetRelativeScale3D(FVector(6.f, 6.f, 0.08f));
	Mesh->SetRelativeLocation(FVector(0.f, 0.f, 6.f));
	Mesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}

FString AFarmFishingSpot::GetPrompt(const AFarmPlayerController* PC) const
{
	return TEXT("E: Fish (needs a fishing rod)");
}

void AFarmFishingSpot::Interact(AFarmCharacter* Character)
{
	if (!Character || !Character->GetInventory()) return;
	UFarmInventoryComponent* Inventory = Character->GetInventory();
	AFarmPlayerController* PC = Character->GetController<AFarmPlayerController>();

	if (Inventory->GetCount(TEXT("fishing_rod")) <= 0)
	{
		if (PC) PC->ShowToast(TEXT("You need a fishing rod"));
		return;
	}

	if (FMath::RandRange(0, 3) == 0)
	{
		if (PC) PC->ShowToast(TEXT("Nothing bit this time"));
		return;
	}

	Inventory->AddItem(TEXT("fish"), 1);
	FarmAudio::Play(this, TEXT("/Game/Art/Audio/pop.pop"), 0.7f);
	if (PC) PC->ShowToast(TEXT("Caught a fish"));
}
