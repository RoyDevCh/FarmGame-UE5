#include "FarmGameMode.h"
#include "FarmBin.h"
#include "FarmCharacter.h"
#include "FarmCropCatalog.h"
#include "FarmHUD.h"
#include "FarmInventoryComponent.h"
#include "FarmPlayerController.h"
#include "FarmAnimal.h"
#include "FarmArt.h"
#include "FarmPlot.h"
#include "FarmPortal.h"
#include "FarmShop.h"
#include "FarmResourceNode.h"
#include "FarmSaveGame.h"

#include "Components/DirectionalLightComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/PointLight.h"
#include "Landscape.h"
#include "EngineUtils.h"
#include "Engine/PostProcessVolume.h"
#include "Engine/ExponentialHeightFog.h"
#include "Components/ExponentialHeightFogComponent.h"
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

	Crops = FarmCrops::MakeDefaults();
	Quests = FarmQuests::MakeDefault();
}

void AFarmGameMode::BeginPlay()
{
	Super::BeginPlay();
	if (UGameplayStatics::HasOption(OptionsString, TEXT("autostart")))
	{
		bGameStarted = true;
		bSkipMenuOnStart = true;
	}
	// Start the first day around 09:00 so the farm is lit on spawn.
	DayTimer = DayLengthSeconds * (3.f / 18.f);
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

FString AFarmGameMode::GetTimeLabel() const
{
	const float P = GetDayProgress();
	const int32 Minutes = FMath::FloorToInt(6.f * 60.f + P * 18.f * 60.f);
	return FString::Printf(TEXT("%02d:%02d"), (Minutes / 60) % 24, Minutes % 60);
}

void AFarmGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (!bGameStarted || bGamePaused) return;
	if (DayLengthSeconds > 0.f)
	{
		DayTimer += DeltaSeconds;
		if (DayTimer >= DayLengthSeconds)
		{
			DayTimer = 0.f;
			AdvanceDay();
		}
	}

	if (SunLight)
	{
		const float P = GetDayProgress();
		const float Pitch = -14.f - 70.f * FMath::Sin(P * PI);
		SunLight->SetActorRotation(FRotator(Pitch, 35.f, 0.f));

		float Intensity = bRaining ? 1.9f : 5.0f;
		if (P > 0.8f)
		{
			Intensity *= FMath::Lerp(1.f, 0.35f, (P - 0.8f) / 0.2f);
		}
		Intensity = FMath::Max(Intensity, 1.1f);
		SunLight->GetLightComponent()->SetIntensity(Intensity);
		SunLight->GetLightComponent()->SetLightColor(bRaining ? FLinearColor(0.65f, 0.72f, 0.85f) : FLinearColor(1.f, 0.95f, 0.86f));
	}
}

void AFarmGameMode::AdvanceDay()
{
	Day++;
	bRaining = FMath::RandRange(0, 3) == 0;
	if (bRaining)
	{
		for (AFarmPlot* Plot : Plots)
		{
			if (Plot) Plot->WaterFromRain();
		}
	}
	for (AFarmPlot* Plot : Plots)
	{
		if (Plot) Plot->AdvanceDay(Day);
	}
	for (AFarmResourceNode* Node : ResourceNodes)
	{
		if (Node) Node->Regrow();
	}
	for (AFarmAnimal* Animal : Animals)
	{
		if (Animal) Animal->AdvanceDay();
	}
}

void AFarmGameMode::RegisterBuilding(AFarmBuilding* Building)
{
	if (Building) Buildings.Add(Building);
}

bool AFarmGameMode::IsBuildLocationFree(const FVector& Location, float Radius) const
{
	for (const AFarmPlot* Plot : Plots)
	{
		if (Plot && FVector::Dist2D(Plot->GetActorLocation(), Location) < Radius) return false;
	}
	for (const AFarmBuilding* Building : Buildings)
	{
		if (Building && FVector::Dist2D(Building->GetActorLocation(), Location) < Radius * 1.4f) return false;
	}
	if (Bin && FVector::Dist2D(Bin->GetActorLocation(), Location) < Radius * 0.8f) return false;
	return true;
}

AFarmBuilding* AFarmGameMode::SpawnBuilding(EBuildingType Type, const FVector& Location)
{
	UWorld* World = GetWorld();
	if (!World) return nullptr;
	AFarmBuilding* Building = World->SpawnActor<AFarmBuilding>(AFarmBuilding::StaticClass(), Location, FRotator::ZeroRotator);
	if (Building)
	{
		Building->Configure(Type);
		Buildings.Add(Building);
	}
	return Building;
}

bool AFarmGameMode::ConsumeSkipMenu()
{
	const bool bSkip = bSkipMenuOnStart;
	bSkipMenuOnStart = false;
	return bSkip;
}

bool AFarmGameMode::HasSaveInSlot(int32 Slot) const
{
	const int32 Clamped = FMath::Clamp(Slot, 1, 3);
	return UGameplayStatics::DoesSaveGameExist(FString::Printf(TEXT("FarmSlot%d"), Clamped), 0);
}

void AFarmGameMode::NewGame()
{
	UWorld* World = GetWorld();
	if (!World) return;
	FString MapName = World->GetMapName();
	MapName.RemoveFromStart(World->StreamingLevelsPrefix);
	UGameplayStatics::OpenLevel(this, FName(*MapName), true, TEXT("autostart=1"));
}

void AFarmGameMode::NotifyProduced(FName ItemId, int32 Amount)
{
	if (!Quests.IsValidIndex(ActiveQuestIndex)) return;
	FQuestState& Quest = Quests[ActiveQuestIndex];
	if (Quest.ItemId != ItemId) return;

	if (FarmQuests::Advance(Quest, Amount))
	{
		UE_LOG(LogTemp, Log, TEXT("FarmGame: quest complete %s"), *Quest.Title);
		ActiveQuestIndex++;
		if (Quests.IsValidIndex(ActiveQuestIndex))
		{
			UE_LOG(LogTemp, Log, TEXT("FarmGame: quest next %s"), *Quests[ActiveQuestIndex].Title);
		}
	}
}

void AFarmGameMode::BuildWorld()
{
	SpawnEnvironment();
	UE_LOG(LogTemp, Log, TEXT("FarmGame: building world"));

	UWorld* World = GetWorld();
	if (!World) return;

	bool bHasLandscape = false;
	for (TActorIterator<ALandscape> It(World); It; ++It)
	{
		bHasLandscape = true;
		break;
	}

	UStaticMesh* PlaneMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Plane.Plane"));
	UStaticMesh* CubeMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
	UMaterialInterface* BaseMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));

	// Ground (skip when the map has a landscape)
	if (PlaneMesh && !bHasLandscape)
	{
		AStaticMeshActor* Ground = World->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator);
		if (Ground)
		{
			UStaticMeshComponent* Mesh = Ground->GetStaticMeshComponent();
			Mesh->SetStaticMesh(PlaneMesh);
			Mesh->SetMobility(EComponentMobility::Movable);
			Ground->SetActorScale3D(FVector(140.f, 140.f, 1.f));
			if (UMaterialInterface* GrassMat = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Art/Materials/M_FarmGrassTiled.M_FarmGrassTiled")))
			{
				Mesh->SetMaterial(0, GrassMat);
			}
			else if (BaseMaterial)
			{
				UMaterialInstanceDynamic* MID = UMaterialInstanceDynamic::Create(BaseMaterial, Ground);
				MID->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.07f, 0.17f, 0.06f));
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

	UE_LOG(LogTemp, Log, TEXT("FarmGame: spawned %d plots"), Plots.Num());

	// Shipping bin
	Bin = World->SpawnActor<AFarmBin>(AFarmBin::StaticClass(), FVector(0.f, 620.f, 0.f), FRotator::ZeroRotator);

	// Farm <-> mine portals (the mine is a clearing in the same map)
	{
		const FVector MineCenter(2600.f, 2600.f, 0.f);
		AFarmPortal* ToMine = World->SpawnActor<AFarmPortal>(AFarmPortal::StaticClass(), FVector(700.f, 700.f, 0.f), FRotator::ZeroRotator);
		if (ToMine)
		{
			ToMine->Configure(FText::FromString(TEXT("Mine")), MineCenter + FVector(-350.f, -350.f, 140.f));
			Portals.Add(ToMine);
		}
		AFarmPortal* ToFarm = World->SpawnActor<AFarmPortal>(AFarmPortal::StaticClass(), MineCenter, FRotator::ZeroRotator);
		if (ToFarm)
		{
			ToFarm->Configure(FText::FromString(TEXT("Farm")), FVector(700.f, 700.f, 140.f));
			Portals.Add(ToFarm);
		}

		const FVector OreSpots[] = {
			MineCenter + FVector(150.f, 100.f, 0.f), MineCenter + FVector(300.f, -120.f, 0.f),
			MineCenter + FVector(-200.f, 220.f, 0.f), MineCenter + FVector(120.f, 330.f, 0.f),
			MineCenter + FVector(420.f, 260.f, 0.f), MineCenter + FVector(-320.f, -160.f, 0.f)
		};
		for (const FVector& Spot : OreSpots)
		{
			AFarmResourceNode* Node = World->SpawnActor<AFarmResourceNode>(AFarmResourceNode::StaticClass(), Spot, FRotator::ZeroRotator);
			if (Node)
			{
				Node->Configure(EResourceType::Ore, 4);
				ResourceNodes.Add(Node);
			}
		}
	}

	// Animals: two chickens and a cow in the pen, one hostile boar out east
	{
		struct FAnimalSpot { FVector Location; EAnimalType Type; };
		const FAnimalSpot AnimalSpots[] = {
			{ FVector(-700.f, 300.f, 40.f), EAnimalType::Chicken },
			{ FVector(-780.f, 400.f, 40.f), EAnimalType::Chicken },
			{ FVector(-620.f, 520.f, 40.f), EAnimalType::Cow },
			{ FVector(1400.f, -200.f, 40.f), EAnimalType::Boar }
		};
		for (const FAnimalSpot& Spot : AnimalSpots)
		{
			AFarmAnimal* Animal = World->SpawnActor<AFarmAnimal>(AFarmAnimal::StaticClass(), Spot.Location, FRotator::ZeroRotator);
			if (Animal)
			{
				Animal->Configure(Spot.Type, Spot.Location);
				Animals.Add(Animal);
			}
		}
	}

	// Gatherable trees and rocks around the farm
	struct FNodeSpot { FVector Location; EResourceType Type; };
	const FNodeSpot NodeSpots[] = {
		{ FVector(-1500.f, -500.f, 0.f), EResourceType::Tree },
		{ FVector(-1450.f, 300.f, 0.f), EResourceType::Tree },
		{ FVector(1500.f, -500.f, 0.f), EResourceType::Tree },
		{ FVector(1550.f, 350.f, 0.f), EResourceType::Tree },
		{ FVector(-600.f, 1500.f, 0.f), EResourceType::Tree },
		{ FVector(-1000.f, -1450.f, 0.f), EResourceType::Rock },
		{ FVector(400.f, -1500.f, 0.f), EResourceType::Rock },
		{ FVector(1050.f, 1350.f, 0.f), EResourceType::Rock },
		{ FVector(1600.f, 750.f, 0.f), EResourceType::Rock },
		{ FVector(-1600.f, 900.f, 0.f), EResourceType::Rock }
	};
	for (const FNodeSpot& Spot : NodeSpots)
	{
		AFarmResourceNode* Node = World->SpawnActor<AFarmResourceNode>(AFarmResourceNode::StaticClass(), Spot.Location, FRotator::ZeroRotator);
		if (Node)
		{
			Node->Configure(Spot.Type, 3);
			ResourceNodes.Add(Node);
		}
	}
	UE_LOG(LogTemp, Log, TEXT("FarmGame: resource nodes %d"), ResourceNodes.Num());

	// Imported scenery: farm buildings and nature props.
	SpawnProp(TEXT("/Game/Art/Buildings/barn.barn"), FVector(-1500.f, 1450.f, 0.f), 620.f, 25.f);
	SpawnProp(TEXT("/Game/Art/Buildings/house.house"), FVector(1500.f, -1450.f, 0.f), 600.f, -20.f);
	SpawnProp(TEXT("/Game/Art/Buildings/Windmill.Windmill"), FVector(1500.f, 1250.f, 0.f), 950.f, 40.f);
	SpawnProp(TEXT("/Game/Art/Buildings/wall_square.wall_square"), FVector(-1100.f, 1700.f, 0.f), 280.f, 10.f);

	const FVector PalmSpots[] = { FVector(-2200.f, 200.f, 0.f), FVector(2300.f, -300.f, 0.f), FVector(-300.f, -2100.f, 0.f) };
	for (const FVector& Spot : PalmSpots) SpawnProp(TEXT("/Game/Art/Environment/Palm_Tree.Palm_Tree"), Spot, 800.f);

	const FVector PineSpots[] = { FVector(2400.f, 1800.f, 0.f), FVector(-2400.f, -600.f, 0.f), FVector(600.f, 2200.f, 0.f) };
	for (const FVector& Spot : PineSpots) SpawnProp(TEXT("/Game/Art/Environment/Pine_cone.Pine_cone"), Spot, 650.f);

	const FVector BushSpots[] = {
		FVector(-600.f, -700.f, 0.f), FVector(650.f, -650.f, 0.f), FVector(-700.f, 650.f, 0.f),
		FVector(750.f, 700.f, 0.f), FVector(0.f, 1100.f, 0.f), FVector(0.f, -1100.f, 0.f)
	};
	for (const FVector& Spot : BushSpots) SpawnProp(TEXT("/Game/Art/Environment/Bush.Bush"), Spot, 120.f);

	const FVector FlowerSpots[] = {
		FVector(-500.f, -450.f, 0.f), FVector(520.f, -480.f, 0.f), FVector(-540.f, 480.f, 0.f),
		FVector(560.f, 520.f, 0.f), FVector(-900.f, 100.f, 0.f), FVector(900.f, -100.f, 0.f),
		FVector(150.f, 900.f, 0.f), FVector(-150.f, -900.f, 0.f)
	};
	for (const FVector& Spot : FlowerSpots) SpawnProp(TEXT("/Game/Art/Environment/Flower.Flower"), Spot, 40.f);

	// Scatter grass clumps outside the farm plot area (only without a landscape;
	// the landscape uses painted foliage instead).
	if (!bHasLandscape)
	{
		FRandomStream Rng(1337);
		for (int32 i = 0; i < 220; ++i)
		{
			const FVector P(Rng.FRandRange(-2600.f, 2600.f), Rng.FRandRange(-2600.f, 2600.f), 0.f);
			if (FVector::Dist2D(P, FVector::ZeroVector) < 520.f) continue;
			SpawnProp(TEXT("/Game/Art/Environment/grass.grass"), P, Rng.FRandRange(30.f, 70.f), Rng.FRandRange(0.f, 360.f));
		}
	}

	// Village props imported from the original project.
	SpawnProp(TEXT("/Game/Art/Props/Well.Well"), FVector(700.f, 1000.f, 0.f), 240.f, 15.f);
	Shop = World->SpawnActor<AFarmShop>(AFarmShop::StaticClass(), FVector(320.f, 1000.f, 0.f), FRotator(0.f, 100.f, 0.f));
	if (Shop) UE_LOG(LogTemp, Log, TEXT("FarmGame: shop spawned"));
	SpawnProp(TEXT("/Game/Art/Props/Bench.Bench"), FVector(950.f, 820.f, 0.f), 130.f, -35.f);
	SpawnProp(TEXT("/Game/Art/Props/LampPost.LampPost"), FVector(240.f, 760.f, 0.f), 280.f);
	SpawnProp(TEXT("/Game/Art/Props/LampPost.LampPost"), FVector(1080.f, 760.f, 0.f), 280.f);
	SpawnProp(TEXT("/Game/Art/Props/Fountain.Fountain"), FVector(1420.f, 1250.f, 0.f), 420.f);
	SpawnProp(TEXT("/Game/Art/Environment/Pond.Pond"), FVector(-1050.f, 1450.f, 0.f), 720.f);
	SpawnProp(TEXT("/Game/Art/Props/WoodenCrate.WoodenCrate"), FVector(-1280.f, 980.f, 0.f), 90.f, 12.f);
	SpawnProp(TEXT("/Game/Art/Props/WoodenCrate.WoodenCrate"), FVector(-1210.f, 1030.f, 0.f), 80.f, -20.f);
	SpawnProp(TEXT("/Game/Art/Props/trough.trough"), FVector(1500.f, -380.f, 0.f), 130.f);
	SpawnProp(TEXT("/Game/Art/Props/Sign.Sign"), FVector(220.f, 640.f, 0.f), 100.f, -20.f);
	SpawnProp(TEXT("/Game/Art/Props/anvil.anvil"), FVector(900.f, 320.f, 0.f), 110.f);
	SpawnProp(TEXT("/Game/Art/Props/cauldron.cauldron"), FVector(1180.f, 940.f, 0.f), 130.f);
	SpawnProp(TEXT("/Game/Art/Props/StoneTile1.StoneTile1"), FVector(520.f, 860.f, 0.f), 300.f);
	SpawnProp(TEXT("/Game/Art/Environment/soil_pile.soil_pile"), FVector(470.f, 420.f, 0.f), 110.f);
	SpawnProp(TEXT("/Game/Art/Props/Tree1.Tree1"), FVector(-2000.f, -300.f, 0.f), 820.f);
	SpawnProp(TEXT("/Game/Art/Props/Tree2.Tree2"), FVector(2100.f, 600.f, 0.f), 760.f, 40.f);
	SpawnProp(TEXT("/Game/Art/Props/Tree3.Tree3"), FVector(-1800.f, 2050.f, 0.f), 880.f, 80.f);
	SpawnProp(TEXT("/Game/Art/Props/TreeDead.TreeDead"), FVector(2550.f, 2650.f, 0.f), 700.f, 30.f);
	SpawnProp(TEXT("/Game/Art/Props/Cactus.Cactus"), FVector(2150.f, 2850.f, 0.f), 220.f);

	SpawnProp(TEXT("/Game/Art/Environment/metal_rock.metal_rock"), FVector(2400.f, 2300.f, 0.f), 120.f);
	SpawnProp(TEXT("/Game/Art/Environment/rock2.rock2"), FVector(-2100.f, 2100.f, 0.f), 90.f);


}

void AFarmGameMode::SpawnProp(const TCHAR* Path, const FVector& Location, float Size, float Yaw)
{
	UWorld* World = GetWorld();
	if (!World) return;
	UStaticMesh* Mesh = FarmArt::LoadMesh(Path);
	if (!Mesh) return;

	AStaticMeshActor* Prop = World->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), Location, FRotator(0.f, Yaw, 0.f));
	if (!Prop) return;

	UStaticMeshComponent* Comp = Prop->GetStaticMeshComponent();
	Comp->SetStaticMesh(Mesh);
	Comp->SetMobility(EComponentMobility::Movable);
	const FBox Box = Mesh->GetBoundingBox();
	const FVector Extent = Box.GetSize();
	const float MaxDim = FMath::Max3(Extent.X, Extent.Y, Extent.Z);
	const float Scale = MaxDim > KINDA_SMALL_NUMBER ? Size / MaxDim : 1.f;
	Prop->SetActorScale3D(FVector(Scale));
	Prop->SetActorLocation(Location + FVector(0.f, 0.f, -Box.Min.Z * Scale));
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
		SunLight = Sun;
	}

	ASkyAtmosphere* Atmosphere = World->SpawnActor<ASkyAtmosphere>(ASkyAtmosphere::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator);
	(void)Atmosphere;

	// Soft sky fill (point light, so there is no second directional light conflict).
	APointLight* FillLight = World->SpawnActor<APointLight>(APointLight::StaticClass(), FVector(0.f, 0.f, 1800.f), FRotator::ZeroRotator);
	if (FillLight)
	{
		if (UPointLightComponent* PointComp = Cast<UPointLightComponent>(FillLight->GetLightComponent()))
		{
			PointComp->SetMobility(EComponentMobility::Movable);
			PointComp->SetIntensity(60000.f);
			PointComp->SetAttenuationRadius(9000.f);
			PointComp->SetCastShadows(false);
			PointComp->SetLightColor(FLinearColor(0.8f, 0.86f, 1.f));
		}
	}

	if (APostProcessVolume* PPV = World->SpawnActor<APostProcessVolume>(APostProcessVolume::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator))
	{
		PPV->bUnbound = true;
		FPostProcessSettings& S = PPV->Settings;
		S.bOverride_AutoExposureBias = true;
		S.AutoExposureBias = -0.35f;
		S.bOverride_ColorSaturation = true;
		S.ColorSaturation = FVector4(1.12f, 1.12f, 1.12f, 1.f);
		S.bOverride_ColorContrast = true;
		S.ColorContrast = FVector4(1.06f, 1.06f, 1.06f, 1.f);
		S.bOverride_VignetteIntensity = true;
		S.VignetteIntensity = 0.32f;
		S.bOverride_AmbientOcclusionIntensity = true;
		S.AmbientOcclusionIntensity = 0.7f;
		S.bOverride_BloomIntensity = true;
		S.BloomIntensity = 0.12f;
	}

	if (AExponentialHeightFog* Fog = World->SpawnActor<AExponentialHeightFog>(AExponentialHeightFog::StaticClass(), FVector(0.f, 0.f, -200.f), FRotator::ZeroRotator))
	{
		if (UExponentialHeightFogComponent* FogComp = Fog->GetComponent())
		{
			FogComp->SetFogDensity(0.015f);
			FogComp->SetFogHeightFalloff(0.2f);
			FogComp->SetFogInscatteringColor(FLinearColor(0.55f, 0.7f, 0.85f));
		}
	}

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

void AFarmGameMode::SaveGame(AFarmCharacter* Character)
{
	UFarmSaveGame* Save = Cast<UFarmSaveGame>(UGameplayStatics::CreateSaveGameObject(UFarmSaveGame::StaticClass()));
	if (!Save) return;

	Save->Day = Day;
	Save->DayTimer = DayTimer;
	Save->bRaining = bRaining;
	if (Character)
	{
		if (UFarmInventoryComponent* Inventory = Character->GetInventory())
		{
			Save->Gold = Inventory->Gold;
			Save->Items = Inventory->Items;
		}
		Save->PlayerHealth = Character->GetHealth();
	}
	for (AFarmPlot* Plot : Plots)
	{
		if (Plot) Save->Plots.Add(Plot->CaptureState());
	}
	for (AFarmBuilding* Building : Buildings)
	{
		if (!Building) continue;
		FBuildingSaveData Data;
		Data.Type = static_cast<int32>(Building->GetBuildingType());
		Data.Location = Building->GetActorLocation();
		Data.OreCount = Building->GetOreCount();
		Data.IngotCount = Building->GetIngotCount();
		Data.Storage = Building->GetStorage();
		Save->Buildings.Add(Data);
	}
	for (AFarmAnimal* Animal : Animals)
	{
		if (!Animal) continue;
		FAnimalSaveData Data;
		Data.Type = static_cast<int32>(Animal->GetType());
		Data.Location = Animal->GetActorLocation();
		Data.bFed = Animal->IsFed();
		Data.bProductReady = Animal->HasProduct();
		Data.Health = Animal->GetHealth();
		Save->Animals.Add(Data);
	}
	UGameplayStatics::SaveGameToSlot(Save, SaveSlotName(), 0);
	UE_LOG(LogTemp, Log, TEXT("FarmGame: saved day %d (buildings %d, animals %d)"), Day, Save->Buildings.Num(), Save->Animals.Num());
}

bool AFarmGameMode::LoadGame(AFarmCharacter* Character)
{
	if (!UGameplayStatics::DoesSaveGameExist(SaveSlotName(), 0))
	{
		return false;
	}
	UFarmSaveGame* Save = Cast<UFarmSaveGame>(UGameplayStatics::LoadGameFromSlot(SaveSlotName(), 0));
	if (!Save) return false;

	Day = Save->Day;
	DayTimer = Save->DayTimer;
	bRaining = Save->bRaining;
	if (Character)
	{
		if (UFarmInventoryComponent* Inventory = Character->GetInventory())
		{
			Inventory->Gold = Save->Gold;
			Inventory->Items = Save->Items;
		}
		Character->SetHealth(Save->PlayerHealth);
	}
	for (const FPlotSaveData& Data : Save->Plots)
	{
		if (Plots.IsValidIndex(Data.Index))
		{
			Plots[Data.Index]->RestoreState(Data, FindCrop(Data.CropId));
		}
	}

	for (AFarmBuilding* Building : Buildings)
	{
		if (Building) Building->Destroy();
	}
	Buildings.Reset();
	for (const FBuildingSaveData& Data : Save->Buildings)
	{
		if (AFarmBuilding* Building = SpawnBuilding(static_cast<EBuildingType>(Data.Type), Data.Location))
		{
			Building->RestoreState(Data.OreCount, Data.IngotCount);
			Building->RestoreStorage(Data.Storage);
		}
	}

	for (AFarmAnimal* Animal : Animals)
	{
		if (Animal) Animal->Destroy();
	}
	Animals.Reset();
	for (const FAnimalSaveData& Data : Save->Animals)
	{
		if (AFarmAnimal* Animal = GetWorld()->SpawnActor<AFarmAnimal>(AFarmAnimal::StaticClass(), Data.Location, FRotator::ZeroRotator))
		{
			Animal->Configure(static_cast<EAnimalType>(Data.Type), Data.Location);
			Animal->RestoreState(Data.bFed, Data.bProductReady, Data.Health);
			Animals.Add(Animal);
		}
	}
	UE_LOG(LogTemp, Log, TEXT("FarmGame: loaded day %d (buildings %d, animals %d)"), Day, Buildings.Num(), Animals.Num());
	return true;
}
