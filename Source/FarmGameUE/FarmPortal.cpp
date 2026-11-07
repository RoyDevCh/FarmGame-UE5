#include "FarmPortal.h"
#include "FarmCharacter.h"
#include "FarmPlayerController.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"

AFarmPortal::AFarmPortal()
{
	PrimaryActorTick.bCanEverTick = false;
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	SetRootComponent(Mesh);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> Cylinder(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	if (Cylinder.Succeeded()) Mesh->SetStaticMesh(Cylinder.Object);
	Mesh->SetRelativeScale3D(FVector(0.9f, 0.9f, 0.15f));
	Mesh->SetRelativeLocation(FVector(0.f, 0.f, 12.f));
	Mesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

	UMaterialInterface* BaseMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
	if (BaseMaterial)
	{
		UMaterialInstanceDynamic* MID = UMaterialInstanceDynamic::Create(BaseMaterial, this);
		MID->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.15f, 0.55f, 0.95f));
		Mesh->SetMaterial(0, MID);
	}
}

void AFarmPortal::Configure(const FText& InLabel, const FVector& InDestination)
{
	Label = InLabel;
	Destination = InDestination;
}

FString AFarmPortal::GetPrompt() const
{
	return FString::Printf(TEXT("E: Travel to %s"), *Label.ToString());
}

void AFarmPortal::Interact(AFarmCharacter* Character)
{
	if (!Character) return;
	Character->SetActorLocation(Destination, false, nullptr, ETeleportType::TeleportPhysics);
	UE_LOG(LogTemp, Log, TEXT("FarmGame: travelled to %s"), *Label.ToString());
	if (AFarmPlayerController* PC = Character->GetController<AFarmPlayerController>())
	{
		PC->ShowToast(FString::Printf(TEXT("Travelled to %s"), *Label.ToString()));
	}
}
