#include "FarmShop.h"
#include "FarmArt.h"
#include "FarmCharacter.h"
#include "FarmPlayerController.h"
#include "Components/StaticMeshComponent.h"

AFarmShop::AFarmShop()
{
	PrimaryActorTick.bCanEverTick = false;
	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(SceneRoot);
	Mesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Mesh->SetCollisionResponseToAllChannels(ECR_Block);
}

void AFarmShop::BeginPlay()
{
	Super::BeginPlay();
	FarmArt::ApplyHeight(Mesh, TEXT("/Game/Art/Props/Selling_Kiosk.Selling_Kiosk"), 260.f);
}

FString AFarmShop::GetPrompt(const AFarmPlayerController* PC) const
{
	return TEXT("E: Open shop (seeds / hay / ore)");
}

void AFarmShop::Interact(AFarmCharacter* Character)
{
	if (!Character) return;
	if (AFarmPlayerController* PC = Character->GetController<AFarmPlayerController>())
	{
		PC->OpenShop();
	}
}
