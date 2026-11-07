#include "FarmResourceNode.h"
#include "FarmCharacter.h"
#include "FarmItems.h"
#include "FarmInventoryComponent.h"
#include "FarmPlayerController.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"

AFarmResourceNode::AFarmResourceNode()
{
	PrimaryActorTick.bCanEverTick = false;
	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(SceneRoot);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (Cube.Succeeded()) Mesh->SetStaticMesh(Cube.Object);
	Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

	Detail = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Detail"));
	Detail->SetupAttachment(Mesh);
	Detail->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Detail->SetVisibility(false);
}

void AFarmResourceNode::Configure(EResourceType InType, int32 InAmount)
{
	Type = InType;
	MaxAmount = FMath::Max(1, InAmount);
	Remaining = MaxAmount;
	ApplyVisual();
}

void AFarmResourceNode::ApplyVisual()
{
	UMaterialInterface* BaseMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
	UMaterialInstanceDynamic* MID = BaseMaterial ? UMaterialInstanceDynamic::Create(BaseMaterial, this) : nullptr;

	FLinearColor Color = FLinearColor(0.3f, 0.2f, 0.1f);
	FVector Scale(0.5f, 0.5f, 3.2f);
	FVector Offset(0.f, 0.f, 160.f);
	if (Type == EResourceType::Rock)
	{
		Color = FLinearColor(0.45f, 0.45f, 0.48f);
		Scale = FVector(1.8f, 1.6f, 1.2f);
		Offset = FVector(0.f, 0.f, 60.f);
	}
	else if (Type == EResourceType::Ore)
	{
		Color = FLinearColor(0.32f, 0.3f, 0.36f);
		Scale = FVector(1.6f, 1.5f, 1.1f);
		Offset = FVector(0.f, 0.f, 55.f);
	}

	Mesh->SetRelativeScale3D(Scale);
	Mesh->SetRelativeLocation(Offset);
	if (MID)
	{
		MID->SetVectorParameterValue(TEXT("Color"), Color);
		Mesh->SetMaterial(0, MID);
	}

	// Tree crown
	if (Type == EResourceType::Tree)
	{
		Detail->SetVisibility(true);
		if (UStaticMesh* Sphere = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Sphere.Sphere")))
		{
			Detail->SetStaticMesh(Sphere);
		}
		Detail->SetRelativeScale3D(FVector(2.4f, 2.4f, 1.9f));
		Detail->SetRelativeLocation(FVector(0.f, 0.f, 0.95f));
		if (BaseMaterial)
		{
			UMaterialInstanceDynamic* LeafMid = UMaterialInstanceDynamic::Create(BaseMaterial, this);
			LeafMid->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.10f, 0.42f, 0.15f));
			Detail->SetMaterial(0, LeafMid);
		}
	}
}

FString AFarmResourceNode::GetPrompt(const AFarmPlayerController* PC) const
{
	if (Remaining <= 0)
	{
		return TEXT("Depleted - regrows tomorrow");
	}
	switch (Type)
	{
	case EResourceType::Tree: return FString::Printf(TEXT("E: Chop tree (wood) [%d left]"), Remaining);
	case EResourceType::Rock: return FString::Printf(TEXT("E: Mine rock (stone) [%d left]"), Remaining);
	case EResourceType::Ore: return FString::Printf(TEXT("E: Mine ore vein [%d left]"), Remaining);
	}
	return FString();
}

void AFarmResourceNode::Interact(AFarmCharacter* Character)
{
	if (!Character || Remaining <= 0) return;
	UFarmInventoryComponent* Inventory = Character->GetInventory();
	AFarmPlayerController* PC = Character->GetController<AFarmPlayerController>();
	if (!Inventory) return;

	Remaining--;
	FName Yield = TEXT("wood");
	if (Type == EResourceType::Rock) Yield = TEXT("stone");
	else if (Type == EResourceType::Ore) Yield = TEXT("ore");

	Inventory->AddItem(Yield, 1);
	UE_LOG(LogTemp, Log, TEXT("FarmGame: gathered %s (%d left)"), *Yield.ToString(), Remaining);
	if (PC)
	{
		PC->ShowToast(FString::Printf(TEXT("+1 %s"), *FarmItems::LabelOf(Yield)));
	}
	if (Remaining <= 0)
	{
		SetActorHiddenInGame(true);
	}
}

void AFarmResourceNode::Regrow()
{
	Remaining = MaxAmount;
	SetActorHiddenInGame(false);
}
