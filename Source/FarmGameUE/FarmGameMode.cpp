#include "FarmGameMode.h"
#include "FarmBin.h"
#include "FarmCharacter.h"
#include "FarmHUD.h"
#include "FarmInventoryComponent.h"
#include "FarmPlayerController.h"
#include "FarmPlot.h"
#include "FarmSaveGame.h"

#include "Components/DirectionalLightComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"

AFarmGameMode::AFarmGameMode()
{
	PrimaryActorTick.bCanEverTick = true;
	DefaultPawnClass = AFarmCharacter::StaticClass();
	PlayerControllerClass = AFarmPlayerController::StaticClass();
	HUDClass = AFarmHUD::StaticClass();

	FCropDef Carrot;
	Carrot.Id = TEXT("carrot");
	Carrot.DisplayName = FText::FromString(TEXT("Carrot"));
	Carrot.Stages = 3;
	Carrot.DaysPerStage = 1;
	Carrot.SeedCost = 3;
	Carrot.SellPrice = 8;
	Carrot.YieldPerHarvest = 1;
	Carrot.Color = FLinearColor(0.95f, 0.45f, 0.08f);

	FCropDef Potato;
	Potato.Id = TEXT("potato");
	Potato.DisplayName = FText::FromString(TEXT("Potato"));
	Potato.Stages = 3;
	Potato.DaysPerStage = 2;
	Potato.SeedCost = 5;
	Potato.SellPrice = 14;
	Potato.YieldPerHarvest = 2;
	Potato.Color = FLinearColor(0.75f, 0.62f, 0.35f);

	FCropDef Corn;
	Corn.Id = TEXT("corn");
	Corn.DisplayName = FText::FromString(TEXT("Corn"));
	Corn.Stages = 4;
	Corn.DaysPerStage = 1;
	Corn.SeedCost = 6;
	Corn.SellPrice = 12;
	Corn.YieldPerHarvest = 2;
	Corn.Color = FLinearColor(0.95f, 0.85f, 0.2f);

	Crops = { Carrot, Potato, Corn };
}

void AFarmGameMode::BeginPlay()
{
	Super::BeginPlay();
	BuildWorld();
}

APawn* AFarmGameMode::SpawnDefaultPawnAtTransform_Implementation(AController* NewPlayer, const FTransform& SpawnTransform)
{
	const FTransform FarmSpawn(FRotator::ZeroRotator, FVector(0.f, -520.f, 140.f));
	return Super::SpawnDefaultPawnAtTransform_Implementation(NewPlayer, FarmSpawn);
}

const FCropDef* AFarmGameMode::FindCrop(FName Id) const
{
	return Crops.FindByPredicate([Id](const FCropDef& Crop) { return Crop.Id == Id; });
}

void AFarmGameMode::InitializePlayer(AFarmCharacter* Character)
{
	if (!Character || bPlayerInitialized) return;
	if (UFarmInventoryComponent* Inventory = Character->GetInventory())
	{
		Inventory->ResetForNewGame();
	}
	bPlayerInitialized = true;
}

void AFarmGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (DayLengthSeconds <= 0.f) return;
	DayTimer += DeltaSeconds;
	if (DayTimer >= DayLengthSeconds)
	{
		DayTimer = 0.f;
		AdvanceDay();
	}
}

void AFarmGameMode::AdvanceDay()
{
	Day++;
	for (AFarmPlot* Plot : Plots)
	{
		if (Plot) Plot->AdvanceDay(Day);
	}
}

void AFarmGameMode::BuildWorld()
{
	SpawnEnvironment();
	UE_LOG(LogTemp, Warning, TEXT("FarmGame: building world"));

	UWorld* World = GetWorld();
	if (!World) return;

	UStaticMesh* PlaneMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Plane.Plane"));
	UStaticMesh* CubeMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
	UMaterialInterface* BaseMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));

	// Ground
	if (PlaneMesh)
	{
		AStaticMeshActor* Ground = World->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator);
		if (Ground)
		{
			UStaticMeshComponent* Mesh = Ground->GetStaticMeshComponent();
			Mesh->SetStaticMesh(PlaneMesh);
			Mesh->SetMobility(EComponentMobility::Movable);
			Ground->SetActorScale3D(FVector(140.f, 140.f, 1.f));
			if (BaseMaterial)
			{
				UMaterialInstanceDynamic* MID = UMaterialInstanceDynamic::Create(BaseMaterial, Ground);
				MID->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.10f, 0.23f, 0.09f));
				Mesh->SetMaterial(0, MID);
			}
		}
	}

	// Farm plots 6x6
	const int32 GridSize = 6;
	const float Spacing = 150.f;
	const float Offset = (GridSize - 1) * Spacing * 0.5f;
	int32 Index = 0;
	for (int32 X = 0; X < GridSize; ++X)
	{
		for (int32 Y = 0; Y < GridSize; ++Y)
		{
			const FVector Location(X * Spacing - Offset, Y * Spacing - Offset, 0.f);
			AFarmPlot* Plot = World->SpawnActor<AFarmPlot>(AFarmPlot::StaticClass(), Location, FRotator::ZeroRotator);
			if (Plot)
			{
				Plot->InitPlot(Index++, Location);
				Plots.Add(Plot);
			}
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("FarmGame: spawned %d plots"), Plots.Num());

	// Shipping bin
	Bin = World->SpawnActor<AFarmBin>(AFarmBin::StaticClass(), FVector(0.f, 620.f, 0.f), FRotator::ZeroRotator);

	// A few decorative trees around the farm
	if (CubeMesh)
	{
		const FVector TreeSpots[] = {
			FVector(-1200.f, -900.f, 0.f), FVector(1300.f, -700.f, 0.f), FVector(-1500.f, 800.f, 0.f),
			FVector(1600.f, 1100.f, 0.f), FVector(-400.f, 1700.f, 0.f), FVector(900.f, 1800.f, 0.f)
		};
		for (const FVector& Spot : TreeSpots)
		{
			AStaticMeshActor* Trunk = World->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), Spot + FVector(0.f, 0.f, 220.f), FRotator::ZeroRotator);
			if (Trunk)
			{
				Trunk->GetStaticMeshComponent()->SetStaticMesh(CubeMesh);
				Trunk->GetStaticMeshComponent()->SetMobility(EComponentMobility::Movable);
				Trunk->SetActorScale3D(FVector(0.5f, 0.5f, 4.5f));
				if (BaseMaterial)
				{
					UMaterialInstanceDynamic* MID = UMaterialInstanceDynamic::Create(BaseMaterial, Trunk);
					MID->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.35f, 0.22f, 0.12f));
					Trunk->GetStaticMeshComponent()->SetMaterial(0, MID);
				}
			}
			AStaticMeshActor* Crown = World->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), Spot + FVector(0.f, 0.f, 520.f), FRotator::ZeroRotator);
			if (Crown)
			{
				Crown->GetStaticMeshComponent()->SetStaticMesh(CubeMesh);
				Crown->GetStaticMeshComponent()->SetMobility(EComponentMobility::Movable);
				Crown->SetActorScale3D(FVector(2.6f, 2.6f, 2.2f));
				if (BaseMaterial)
				{
					UMaterialInstanceDynamic* MID = UMaterialInstanceDynamic::Create(BaseMaterial, Crown);
					MID->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.10f, 0.42f, 0.15f));
					Crown->GetStaticMeshComponent()->SetMaterial(0, MID);
				}
			}
		}
	}
}

void AFarmGameMode::SpawnEnvironment()
{
	UWorld* World = GetWorld();
	if (!World) return;

	ADirectionalLight* Sun = World->SpawnActor<ADirectionalLight>(ADirectionalLight::StaticClass(), FVector(0.f, 0.f, 3000.f), FRotator(-48.f, 35.f, 0.f));
	if (Sun)
	{
		Sun->GetLightComponent()->SetMobility(EComponentMobility::Movable);
		Sun->GetLightComponent()->SetIntensity(4.f);
		Sun->GetLightComponent()->SetLightColor(FLinearColor(1.f, 0.96f, 0.88f));
		Sun->GetLightComponent()->SetCastShadows(true);
	}

	ASkyAtmosphere* Atmosphere = World->SpawnActor<ASkyAtmosphere>(ASkyAtmosphere::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator);
	(void)Atmosphere;

	ASkyLight* Sky = World->SpawnActor<ASkyLight>(ASkyLight::StaticClass(), FVector(0.f, 0.f, 800.f), FRotator::ZeroRotator);
	if (Sky)
	{
		if (USkyLightComponent* SkyComp = Cast<USkyLightComponent>(Sky->GetLightComponent()))
		{
			SkyComp->SetMobility(EComponentMobility::Movable);
			SkyComp->bRealTimeCapture = true;
			SkyComp->SetIntensity(0.9f);
		}
	}
}

void AFarmGameMode::SaveGame(UFarmInventoryComponent* Inventory)
{
	UFarmSaveGame* Save = Cast<UFarmSaveGame>(UGameplayStatics::CreateSaveGameObject(UFarmSaveGame::StaticClass()));
	if (!Save) return;

	Save->Day = Day;
	if (Inventory)
	{
		Save->Gold = Inventory->Gold;
		Save->Seeds = Inventory->Seeds;
		Save->Produce = Inventory->Produce;
	}
	for (AFarmPlot* Plot : Plots)
	{
		if (Plot) Save->Plots.Add(Plot->CaptureState());
	}
	UGameplayStatics::SaveGameToSlot(Save, TEXT("FarmSlot"), 0);
}

bool AFarmGameMode::LoadGame(UFarmInventoryComponent* Inventory)
{
	if (!UGameplayStatics::DoesSaveGameExist(TEXT("FarmSlot"), 0))
	{
		return false;
	}
	UFarmSaveGame* Save = Cast<UFarmSaveGame>(UGameplayStatics::LoadGameFromSlot(TEXT("FarmSlot"), 0));
	if (!Save) return false;

	Day = Save->Day;
	if (Inventory)
	{
		Inventory->Gold = Save->Gold;
		Inventory->Seeds = Save->Seeds;
		Inventory->Produce = Save->Produce;
	}
	for (const FPlotSaveData& Data : Save->Plots)
	{
		if (Plots.IsValidIndex(Data.Index))
		{
			Plots[Data.Index]->RestoreState(Data, FindCrop(Data.CropId));
		}
	}
	return true;
}
