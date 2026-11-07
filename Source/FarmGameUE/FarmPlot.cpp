#include "FarmPlot.h"
#include "FarmCharacter.h"
#include "FarmPlayerController.h"
#include "FarmGameMode.h"
#include "FarmInventoryComponent.h"
#include "FarmItems.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"

AFarmPlot::AFarmPlot()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderMesh(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));

	SoilMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Soil"));
	SoilMesh->SetupAttachment(SceneRoot);
	if (CubeMesh.Succeeded()) SoilMesh->SetStaticMesh(CubeMesh.Object);
	SoilMesh->SetRelativeScale3D(FVector(1.1f, 1.1f, 0.12f));
	SoilMesh->SetRelativeLocation(FVector(0.f, 0.f, 6.f));
	SoilMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	CropMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Crop"));
	CropMesh->SetupAttachment(SceneRoot);
	if (CylinderMesh.Succeeded()) CropMesh->SetStaticMesh(CylinderMesh.Object);
	CropMesh->SetRelativeLocation(FVector(0.f, 0.f, 20.f));
	CropMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CropMesh->SetVisibility(false);
}

void AFarmPlot::BeginPlay()
{
	Super::BeginPlay();

	UMaterialInterface* BaseMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
	if (BaseMaterial)
	{
		SoilMaterial = UMaterialInstanceDynamic::Create(BaseMaterial, this);
		CropMaterial = UMaterialInstanceDynamic::Create(BaseMaterial, this);
		if (SoilMaterial) SoilMesh->SetMaterial(0, SoilMaterial);
		if (CropMaterial) CropMesh->SetMaterial(0, CropMaterial);
	}
	RefreshVisuals();
}

void AFarmPlot::InitPlot(int32 InIndex, const FVector& InLocation)
{
	Index = InIndex;
	SetActorLocation(InLocation);
}

void AFarmPlot::ApplyColor(UMaterialInstanceDynamic* Material, const FLinearColor& Color) const
{
	if (Material)
	{
		Material->SetVectorParameterValue(TEXT("Color"), Color);
	}
}

void AFarmPlot::RefreshVisuals()
{
	FLinearColor SoilColor(0.22f, 0.19f, 0.15f);
	switch (State)
	{
	case EFarmPlotState::Untilled:
		SoilColor = FLinearColor(0.22f, 0.19f, 0.15f);
		break;
	case EFarmPlotState::Tilled:
		SoilColor = FLinearColor(0.34f, 0.24f, 0.15f);
		break;
	case EFarmPlotState::Planted:
		SoilColor = bWatered ? FLinearColor(0.18f, 0.13f, 0.09f) : FLinearColor(0.34f, 0.24f, 0.15f);
		break;
	case EFarmPlotState::Harvestable:
		SoilColor = FLinearColor(0.30f, 0.21f, 0.13f);
		break;
	}
	ApplyColor(SoilMaterial, SoilColor);

	const bool bShowCrop = (State == EFarmPlotState::Planted || State == EFarmPlotState::Harvestable);
	CropMesh->SetVisibility(bShowCrop);
	if (!bShowCrop)
	{
		return;
	}

	const int32 MaxStage = CropDef ? FMath::Max(1, CropDef->Stages) - 1 : 1;
	const float StageAlpha = MaxStage > 0 ? static_cast<float>(Stage) / static_cast<float>(MaxStage) : 0.f;
	const float Height = FMath::Lerp(0.18f, 1.15f, StageAlpha);
	const float Width = FMath::Lerp(0.22f, 0.75f, StageAlpha);
	CropMesh->SetRelativeScale3D(FVector(Width, Width, Height));
	CropMesh->SetRelativeLocation(FVector(0.f, 0.f, 12.f + Height * 30.f));

	FLinearColor CropColor = CropDef ? CropDef->Color : FLinearColor(0.1f, 0.6f, 0.2f);
	if (State == EFarmPlotState::Harvestable)
	{
		CropColor = FMath::Lerp(CropColor, FLinearColor(0.95f, 0.75f, 0.15f), 0.55f);
	}
	ApplyColor(CropMaterial, CropColor);
}

void AFarmPlot::AdvanceDay(int32 Today)
{
	if (State != EFarmPlotState::Planted || !CropDef)
	{
		return;
	}
	FarmLogic::AdvanceCrop(*CropDef, Today, Stage, LastGrowthDay, bWatered, bWatered);
	if (FarmLogic::IsMature(*CropDef, Stage))
	{
		State = EFarmPlotState::Harvestable;
	}
	RefreshVisuals();
}

void AFarmPlot::WaterFromRain()
{
	if (State == EFarmPlotState::Planted && !bWatered)
	{
		bWatered = true;
		RefreshVisuals();
	}
}

FString AFarmPlot::GetPrompt(const AFarmPlayerController* PC) const
{
	switch (State)
	{
	case EFarmPlotState::Untilled:
		return TEXT("E: Till soil");
	case EFarmPlotState::Tilled:
		return FString::Printf(TEXT("E: Plant %s   Q: change seed"), PC ? *PC->GetSelectedCropLabel() : TEXT("crop"));
	case EFarmPlotState::Planted:
		if (!bWatered)
		{
			return TEXT("E: Water");
		}
		return FString::Printf(TEXT("Growing... stage %d/%d"), Stage + 1, CropDef ? CropDef->Stages : 1);
	case EFarmPlotState::Harvestable:
		return FString::Printf(TEXT("E: Harvest %s"), CropDef ? *CropDef->DisplayName.ToString() : TEXT("crop"));
	default:
		return TEXT("");
	}
}

void AFarmPlot::Interact(AFarmCharacter* Character)
{
	if (!Character)
	{
		return;
	}
	UFarmInventoryComponent* Inventory = Character->GetInventory();
	AFarmGameMode* GM = GetWorld() ? GetWorld()->GetAuthGameMode<AFarmGameMode>() : nullptr;
	AFarmPlayerController* PC = Character->GetController<AFarmPlayerController>();

	switch (State)
	{
	case EFarmPlotState::Untilled:
		State = EFarmPlotState::Tilled;
		RefreshVisuals();
		UE_LOG(LogTemp, Log, TEXT("FarmGame: plot %d tilled"), Index);
		break;

	case EFarmPlotState::Tilled:
	{
		if (!Inventory || !GM || !PC)
		{
			break;
		}
		const FName SelectedId = PC->GetSelectedCropId();
		if (Inventory->RemoveItem(FarmItems::SeedId(SelectedId), 1))
		{
			CropId = SelectedId;
			CropDef = GM->FindCrop(SelectedId);
			Stage = 0;
			bWatered = false;
			LastGrowthDay = GM->GetDay();
			State = EFarmPlotState::Planted;
			RefreshVisuals();
			UE_LOG(LogTemp, Log, TEXT("FarmGame: plot %d planted %s"), Index, *CropId.ToString());
		}
		else
		{
			PC->ShowToast(FString::Printf(TEXT("No %s seeds left - press B to buy at the bin"), *PC->GetSelectedCropLabel()));
		}
		break;
	}

	case EFarmPlotState::Planted:
		if (!bWatered)
		{
			bWatered = true;
			RefreshVisuals();
			UE_LOG(LogTemp, Log, TEXT("FarmGame: plot %d watered"), Index);
		}
		break;

	case EFarmPlotState::Harvestable:
	{
		if (Inventory && !CropId.IsNone())
		{
			const int32 Yield = CropDef ? CropDef->YieldPerHarvest : 1;
			Inventory->AddItem(CropId, Yield);
			UE_LOG(LogTemp, Log, TEXT("FarmGame: plot %d harvested %s x%d"), Index, *CropId.ToString(), Yield);
			if (PC)
			{
				PC->ShowToast(FString::Printf(TEXT("+%d %s"), Yield, CropDef ? *CropDef->DisplayName.ToString() : TEXT("crop")));
			}
		}
		State = EFarmPlotState::Tilled;
		CropId = NAME_None;
		CropDef = nullptr;
		Stage = 0;
		bWatered = false;
		RefreshVisuals();
		break;
	}
	default:
		break;
	}
}

FPlotSaveData AFarmPlot::CaptureState() const
{
	FPlotSaveData Data;
	Data.Index = Index;
	Data.State = State;
	Data.CropId = CropId;
	Data.Stage = Stage;
	Data.bWatered = bWatered;
	Data.LastGrowthDay = LastGrowthDay;
	return Data;
}

void AFarmPlot::RestoreState(const FPlotSaveData& Data, const FCropDef* InCrop)
{
	State = Data.State;
	CropId = Data.CropId;
	Stage = Data.Stage;
	bWatered = Data.bWatered;
	LastGrowthDay = Data.LastGrowthDay;
	CropDef = InCrop;
	RefreshVisuals();
}
