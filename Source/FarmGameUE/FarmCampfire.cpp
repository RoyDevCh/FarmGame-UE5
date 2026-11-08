#include "FarmCampfire.h"
#include "FarmCharacter.h"
#include "FarmPlayerController.h"
#include "Components/PointLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"

AFarmCampfire::AFarmCampfire()
{
	PrimaryActorTick.bCanEverTick = true;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	Body = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Body"));
	Body->SetupAttachment(SceneRoot);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (Cube.Succeeded()) Body->SetStaticMesh(Cube.Object);
	Body->SetRelativeScale3D(FVector(1.1f, 1.1f, 0.4f));
	Body->SetRelativeLocation(FVector(0.f, 0.f, 20.f));
	Body->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Body->SetCollisionResponseToAllChannels(ECR_Block);

	UMaterialInterface* BaseMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
	if (BaseMaterial)
	{
		UMaterialInstanceDynamic* MID = UMaterialInstanceDynamic::Create(BaseMaterial, this);
		MID->SetVectorParameterValue(TEXT("Color"), FLinearColor(1.f, 0.45f, 0.08f));
		Body->SetMaterial(0, MID);
	}

	Light = CreateDefaultSubobject<UPointLightComponent>(TEXT("Light"));
	Light->SetupAttachment(SceneRoot);
	Light->SetRelativeLocation(FVector(0.f, 0.f, 120.f));
	Light->SetMobility(EComponentMobility::Movable);
	Light->SetLightColor(FLinearColor(1.f, 0.55f, 0.2f));
	Light->SetAttenuationRadius(900.f);
	Light->SetSourceRadius(12.f);
	Light->SetCastShadows(false);
	Light->SetIntensity(0.f);
	Light->SetVisibility(false);
}

void AFarmCampfire::BeginPlay()
{
	Super::BeginPlay();
	SetLit(bLit);
}

void AFarmCampfire::SetLit(bool bInLit)
{
	bLit = bInLit;
	if (Light)
	{
		Light->SetVisibility(bLit);
		Light->SetIntensity(bLit ? LitLightIntensity() : 0.f);
	}
	if (!bLit) HealAccumulator = 0.f;
}

void AFarmCampfire::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (!bLit || DeltaSeconds <= 0.f) return;

	const float Rate = FarmCampfireLogic::HealPerSecond(bLit);
	if (Rate <= 0.f) return;

	AFarmCharacter* Farmer = nullptr;
	if (UWorld* World = GetWorld())
	{
		if (APlayerController* PC = World->GetFirstPlayerController())
		{
			Farmer = Cast<AFarmCharacter>(PC->GetPawn());
		}
	}
	if (!Farmer || Farmer->IsDead()) return;
	if (Farmer->GetHealth() >= Farmer->GetMaxHealth())
	{
		HealAccumulator = 0.f;
		return;
	}

	const float Dist = FVector::Dist2D(GetActorLocation(), Farmer->GetActorLocation());
	if (!FarmCampfireLogic::ShouldHeal(bLit, Dist, Radius)) return;

	HealAccumulator = FMath::Min(HealAccumulator + Rate * DeltaSeconds, 32.f);
	while (HealAccumulator >= 1.f)
	{
		HealAccumulator -= 1.f;
		Farmer->Heal(1);
		if (Farmer->GetHealth() >= Farmer->GetMaxHealth()) break;
	}
}

FString AFarmCampfire::GetPrompt(const AFarmPlayerController* PC) const
{
	return bLit ? TEXT("E: Put out campfire") : TEXT("E: Light campfire");
}

void AFarmCampfire::Interact(AFarmCharacter* Character)
{
	if (!Character) return;
	SetLit(!bLit);
	UE_LOG(LogTemp, Log, TEXT("FarmGame: campfire %s"), bLit ? TEXT("lit") : TEXT("out"));
	if (AFarmPlayerController* PC = Character->GetController<AFarmPlayerController>())
	{
		PC->ShowToast(bLit ? TEXT("Campfire lit") : TEXT("Campfire out"));
	}
}
