#include "FarmDefense.h"
#include "FarmAnimal.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"

AFarmTrap::AFarmTrap()
{
	PrimaryActorTick.bCanEverTick = true;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	Body = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Body"));
	Body->SetupAttachment(SceneRoot);
}

void AFarmTrap::BeginPlay()
{
	Super::BeginPlay();
	Body->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	if (UStaticMesh* Cube = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube")))
	{
		Body->SetStaticMesh(Cube);
	}
	const float PlateScale = FMath::Max(0.1f, Radius / 50.f);
	Body->SetRelativeScale3D(FVector(PlateScale, PlateScale, 0.08f));
	Body->SetRelativeLocation(FVector(0.f, 0.f, 4.f));

	UMaterialInterface* BaseMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
	if (UMaterialInstanceDynamic* MID = BaseMaterial ? UMaterialInstanceDynamic::Create(BaseMaterial, this) : nullptr)
	{
		MID->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.16f, 0.17f, 0.19f));
		Body->SetMaterial(0, MID);
	}
}

void AFarmTrap::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (!bArmed) return;
	UWorld* World = GetWorld();
	if (!World) return;

	const FVector MyLocation = GetActorLocation();
	for (TActorIterator<AFarmAnimal> It(World); It; ++It)
	{
		AFarmAnimal* Animal = *It;
		if (!Animal || !Animal->IsHostile()) continue;
		if (!FarmDefenseLogic::InRange(FVector::Dist2D(MyLocation, Animal->GetActorLocation()), Radius)) continue;
		Animal->ApplyDamage(Damage, nullptr);
		bArmed = false;
		UE_LOG(LogTemp, Log, TEXT("FarmGame: trap triggered"));
		return;
	}
}

AFarmTower::AFarmTower()
{
	PrimaryActorTick.bCanEverTick = true;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	Body = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Body"));
	Body->SetupAttachment(SceneRoot);
}

void AFarmTower::BeginPlay()
{
	Super::BeginPlay();
	Body->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	if (UStaticMesh* Cylinder = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cylinder.Cylinder")))
	{
		Body->SetStaticMesh(Cylinder);
	}
	Body->SetRelativeScale3D(FVector(0.7f, 0.7f, 2.8f));
	Body->SetRelativeLocation(FVector(0.f, 0.f, 140.f));

	UMaterialInterface* BaseMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
	if (UMaterialInstanceDynamic* MID = BaseMaterial ? UMaterialInstanceDynamic::Create(BaseMaterial, this) : nullptr)
	{
		MID->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.33f, 0.32f, 0.30f));
		Body->SetMaterial(0, MID);
	}
}

void AFarmTower::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	UWorld* World = GetWorld();
	if (!World) return;

	const float Now = World->GetTimeSeconds();
	if (!FarmDefenseLogic::ReadyToFire(Now, LastFireTime, Cooldown)) return;

	const FVector MyLocation = GetActorLocation();
	AFarmAnimal* Nearest = nullptr;
	float BestDistance = Range;
	for (TActorIterator<AFarmAnimal> It(World); It; ++It)
	{
		AFarmAnimal* Animal = *It;
		if (!Animal || !Animal->IsHostile()) continue;
		const float Dist = FVector::Dist2D(MyLocation, Animal->GetActorLocation());
		if (!FarmDefenseLogic::InRange(Dist, Range) || Dist >= BestDistance) continue;
		BestDistance = Dist;
		Nearest = Animal;
	}

	if (!Nearest) return;
	Nearest->ApplyDamage(Damage, nullptr);
	LastFireTime = Now;
	UE_LOG(LogTemp, Log, TEXT("FarmGame: tower fired"));
}
