#include "FarmWater.h"
#include "FarmCharacter.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"

namespace
{
	/** Pond footprint: 600 x 600 x 120 cm. */
	constexpr float VolumeHalfWidth = 300.f;
	constexpr float VolumeHalfDepth = 300.f;
	constexpr float VolumeHalfHeight = 60.f;

	/** UCharacterMovementComponent's default MaxSwimSpeed, used when no entry was captured. */
	constexpr float DefaultMaxSwimSpeed = 300.f;
}

AFarmWater::AFarmWater()
{
	PrimaryActorTick.bCanEverTick = false;

	Volume = CreateDefaultSubobject<UBoxComponent>(TEXT("Volume"));
	SetRootComponent(Volume);
	Volume->SetBoxExtent(FVector(VolumeHalfWidth, VolumeHalfDepth, VolumeHalfHeight));
	Volume->SetRelativeLocation(FVector(0.f, 0.f, VolumeHalfHeight));
	Volume->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Volume->SetCollisionObjectType(ECC_WorldDynamic);
	Volume->SetCollisionResponseToAllChannels(ECR_Ignore);
	Volume->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	Volume->SetGenerateOverlapEvents(true);

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(Volume);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> Plane(TEXT("/Engine/BasicShapes/Plane.Plane"));
	if (Plane.Succeeded()) Mesh->SetStaticMesh(Plane.Object);
	// The plane is 100x100 cm; 6x makes it match the 600x600 volume, 58 sits just under the surface.
	Mesh->SetRelativeScale3D(FVector(6.f, 6.f, 1.f));
	Mesh->SetRelativeLocation(FVector(0.f, 0.f, VolumeHalfHeight - 2.f));
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Mesh->SetCastShadow(false);
}

void AFarmWater::BeginPlay()
{
	Super::BeginPlay();

	Volume->OnComponentBeginOverlap.AddDynamic(this, &AFarmWater::OnVolumeBeginOverlap);
	Volume->OnComponentEndOverlap.AddDynamic(this, &AFarmWater::OnVolumeEndOverlap);

	// Explicit material first, then the engine's translucent debug material (it has Color +
	// Opacity parameters), then the opaque basic shape material for cooked builds that strip
	// EngineDebugMaterials.
	UMaterialInterface* Base = WaterMaterial;
	if (!Base)
	{
		Base = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/EngineDebugMaterials/M_SimpleTranslucent.M_SimpleTranslucent"));
	}
	if (!Base)
	{
		Base = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
	}
	if (Base)
	{
		UMaterialInstanceDynamic* MID = UMaterialInstanceDynamic::Create(Base, this);
		if (MID)
		{
			MID->SetVectorParameterValue(TEXT("Color"), WaterColor);
			MID->SetScalarParameterValue(TEXT("Opacity"), WaterColor.A);
			Mesh->SetMaterial(0, MID);
		}
	}
}

void AFarmWater::OnVolumeBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AFarmCharacter* Character = Cast<AFarmCharacter>(OtherActor);
	if (!Character) return;

	UCharacterMovementComponent* Movement = Character->GetCharacterMovement();
	if (!Movement) return;

	// Remember the dry-land speeds once; nested overlap events must not capture the swim speed.
	FWaterEntrySpeeds& Speeds = EntrySpeeds.FindOrAdd(Character);
	if (Speeds.WalkSpeed <= 0.f)
	{
		Speeds.WalkSpeed = Movement->MaxWalkSpeed;
		Speeds.SwimSpeed = Movement->MaxSwimSpeed;
	}

	Movement->MaxWalkSpeed = FarmWaterLogic::BaseWalkSpeed * FarmWaterLogic::SpeedMultiplier(true);
	// MOVE_Swimming moves at MaxSwimSpeed, so mirror the slowed speed there too.
	Movement->MaxSwimSpeed = Movement->MaxWalkSpeed;
	Movement->SetMovementMode(MOVE_Swimming);
}

void AFarmWater::OnVolumeEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	AFarmCharacter* Character = Cast<AFarmCharacter>(OtherActor);
	if (!Character) return;

	UCharacterMovementComponent* Movement = Character->GetCharacterMovement();
	if (!Movement) return;

	// Restore the speed the farmer actually had before entering (AFarmCharacter ships 520,
	// not 600), so leaving water never silently speeds the character up.
	FWaterEntrySpeeds Speeds;
	if (const FWaterEntrySpeeds* Found = EntrySpeeds.Find(Character))
	{
		Speeds = *Found;
		EntrySpeeds.Remove(Character);
	}

	Movement->MaxWalkSpeed = Speeds.WalkSpeed > 0.f ? Speeds.WalkSpeed : FarmWaterLogic::BaseWalkSpeed;
	Movement->MaxSwimSpeed = Speeds.SwimSpeed > 0.f ? Speeds.SwimSpeed : DefaultMaxSwimSpeed;
	Movement->SetMovementMode(MOVE_Walking);
}
