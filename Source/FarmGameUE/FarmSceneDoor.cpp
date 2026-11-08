#include "FarmSceneDoor.h"
#include "FarmCharacter.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"

AFarmSceneDoor::AFarmSceneDoor()
{
	PrimaryActorTick.bCanEverTick = false;
	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(SceneRoot);
	Mesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Mesh->SetCollisionResponseToAllChannels(ECR_Block);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (Cube.Succeeded()) Mesh->SetStaticMesh(Cube.Object);
}

void AFarmSceneDoor::BeginPlay()
{
	Super::BeginPlay();
	Mesh->SetRelativeScale3D(FVector(1.0f, 2.2f, 0.35f));
	Mesh->SetRelativeLocation(FVector(0.f, 0.f, 110.f));
}

FString AFarmSceneDoor::GetPrompt(const AFarmPlayerController* PC) const
{
	return FString::Printf(TEXT("E: Enter %s"), *TargetMap.ToString().RightChop(TargetMap.ToString().Find(TEXT("/"), ESearchCase::IgnoreCase, ESearchDir::FromEnd) + 1));
}

void AFarmSceneDoor::Interact(AFarmCharacter* Character)
{
	if (!Character) return;
	const FString Options = FString::Printf(TEXT("spawn=%s&autostart=1"), *SpawnOption);
	UGameplayStatics::OpenLevel(this, TargetMap, true, Options);
}
