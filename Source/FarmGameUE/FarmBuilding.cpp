#include "FarmBuilding.h"
#include "FarmArt.h"
#include "FarmTypes.h"
#include "FarmCharacter.h"
#include "FarmInventoryComponent.h"
#include "FarmPlayerController.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"

AFarmBuilding::AFarmBuilding()
{
	PrimaryActorTick.bCanEverTick = true;

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

void AFarmBuilding::Configure(EBuildingType InType)
{
	Type = InType;
	ApplyVisual();
}

void AFarmBuilding::ApplyVisual()
{
	const TCHAR* ArtPath = Type == EBuildingType::Fence ? TEXT("/Game/Art/Buildings/fence_long.fence_long") : TEXT("/Game/Art/Buildings/furnace.furnace");
	const float ArtHeight = Type == EBuildingType::Fence ? 110.f : 170.f;
	const bool bArtApplied = Type == EBuildingType::Fence ? FarmArt::Apply(Mesh, ArtPath, ArtHeight) : FarmArt::ApplyHeight(Mesh, ArtPath, ArtHeight);
	if (bArtApplied)
	{
		Detail->SetVisibility(false);
		return;
	}

	UMaterialInterface* BaseMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
	UMaterialInstanceDynamic* MID = BaseMaterial ? UMaterialInstanceDynamic::Create(BaseMaterial, this) : nullptr;

	if (Type == EBuildingType::Fence)
	{
		Mesh->SetRelativeScale3D(FVector(1.6f, 0.15f, 1.0f));
		Mesh->SetRelativeLocation(FVector(0.f, 0.f, 50.f));
		if (MID)
		{
			MID->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.42f, 0.30f, 0.16f));
			Mesh->SetMaterial(0, MID);
		}
	}
	else
	{
		Mesh->SetRelativeScale3D(FVector(1.4f, 1.4f, 1.2f));
		Mesh->SetRelativeLocation(FVector(0.f, 0.f, 60.f));
		if (MID)
		{
			MID->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.35f, 0.33f, 0.31f));
			Mesh->SetMaterial(0, MID);
		}
		Detail->SetVisibility(true);
		if (UStaticMesh* Cylinder = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cylinder.Cylinder")))
		{
			Detail->SetStaticMesh(Cylinder);
		}
		Detail->SetRelativeScale3D(FVector(0.45f, 0.45f, 1.8f));
		Detail->SetRelativeLocation(FVector(0.f, 0.f, 2.2f));
		if (BaseMaterial)
		{
			UMaterialInstanceDynamic* ChimneyMid = UMaterialInstanceDynamic::Create(BaseMaterial, this);
			ChimneyMid->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.22f, 0.20f, 0.19f));
			Detail->SetMaterial(0, ChimneyMid);
		}
	}
}

void AFarmBuilding::RestoreState(int32 InOre, int32 InIngots)
{
	OreCount = FMath::Clamp(InOre, 0, MaxOre());
	IngotCount = FMath::Max(0, InIngots);
}

void AFarmBuilding::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (Type != EBuildingType::Furnace || OreCount <= 0) return;

	FarmLogic::SmeltStep(OreCount, IngotCount, SmeltTimer, DeltaSeconds, SmeltSecondsPerOre());
}

FString AFarmBuilding::GetPrompt(const AFarmPlayerController* PC) const
{
	if (Type == EBuildingType::Fence)
	{
		return TEXT("Fence");
	}
	if (IngotCount > 0)
	{
		return FString::Printf(TEXT("E: Collect %d iron ingot(s)"), IngotCount);
	}
	if (OreCount > 0)
	{
		return FString::Printf(TEXT("Furnace: smelting %d ore (%.0fs each)"), OreCount, SmeltSecondsPerOre());
	}
	return FString::Printf(TEXT("E: Load ore + wood (max %d)"), MaxOre());
}

void AFarmBuilding::Interact(AFarmCharacter* Character)
{
	if (!Character || Type != EBuildingType::Furnace) return;
	UFarmInventoryComponent* Inventory = Character->GetInventory();
	AFarmPlayerController* PC = Character->GetController<AFarmPlayerController>();
	if (!Inventory) return;

	if (IngotCount > 0)
	{
		Inventory->AddItem(TEXT("ingot"), IngotCount);
		UE_LOG(LogTemp, Log, TEXT("FarmGame: furnace collected %d ingot"), IngotCount);
		if (PC) PC->ShowToast(FString::Printf(TEXT("+%d iron ingot"), IngotCount));
		IngotCount = 0;
		return;
	}

	if (OreCount >= MaxOre())
	{
		if (PC) PC->ShowToast(TEXT("Furnace is full"));
		return;
	}

	if (Inventory->RemoveItem(TEXT("ore"), 1) && Inventory->RemoveItem(TEXT("wood"), 1))
	{
		OreCount++;
		UE_LOG(LogTemp, Log, TEXT("FarmGame: furnace loaded ore (%d)"), OreCount);
		if (PC) PC->ShowToast(TEXT("Loaded 1 ore (burns 1 wood)"));
	}
	else
	{
		if (PC) PC->ShowToast(TEXT("Need 1 ore and 1 wood"));
	}
}
