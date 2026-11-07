#include "FarmAnimal.h"
#include "FarmArt.h"
#include "FarmAudio.h"
#include "FarmCharacter.h"
#include "FarmInventoryComponent.h"
#include "FarmItems.h"
#include "FarmPlayerController.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"

AFarmAnimal::AFarmAnimal()
{
	PrimaryActorTick.bCanEverTick = true;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	Body = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Body"));
	Body->SetupAttachment(SceneRoot);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (Cube.Succeeded()) Body->SetStaticMesh(Cube.Object);
	Body->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

	Head = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Head"));
	Head->SetupAttachment(Body);
	Head->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Head->SetVisibility(false);
}

void AFarmAnimal::Configure(EAnimalType InType, const FVector& InHome)
{
	Type = InType;
	HomeLocation = InHome;
	switch (Type)
	{
	case EAnimalType::Chicken:
		MaxHealth = 30;
		break;
	case EAnimalType::Cow:
		MaxHealth = 70;
		break;
	case EAnimalType::Boar:
		MaxHealth = 60;
		break;
	}
	Health = MaxHealth;
	SetActorLocation(InHome);
	ApplyVisual();
}

void AFarmAnimal::ApplyVisual()
{
	UMaterialInterface* BaseMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
	UMaterialInstanceDynamic* MID = BaseMaterial ? UMaterialInstanceDynamic::Create(BaseMaterial, this) : nullptr;

	const TCHAR* ArtPath = Type == EAnimalType::Chicken ? TEXT("/Game/Art/Animals/Chicken.Chicken")
		: Type == EAnimalType::Cow ? TEXT("/Game/Art/Animals/Cow.Cow")
		: TEXT("/Game/Art/Animals/bear.bear");
	const float ArtHeight = Type == EAnimalType::Chicken ? 55.f : Type == EAnimalType::Cow ? 140.f : 120.f;
	if (FarmArt::ApplyHeight(Body, ArtPath, ArtHeight))
	{
		Head->SetVisibility(false);
		return;
	}

	FVector Scale(0.6f, 0.9f, 0.6f);
	FLinearColor Color(0.95f, 0.95f, 0.9f);
	switch (Type)
	{
	case EAnimalType::Chicken:
		Scale = FVector(0.45f, 0.6f, 0.5f);
		Color = FLinearColor(0.95f, 0.95f, 0.9f);
		break;
	case EAnimalType::Cow:
		Scale = FVector(1.0f, 1.5f, 0.9f);
		Color = FLinearColor(0.85f, 0.8f, 0.75f);
		break;
	case EAnimalType::Boar:
		Scale = FVector(0.8f, 1.2f, 0.75f);
		Color = FLinearColor(0.35f, 0.3f, 0.28f);
		break;
	}
	Body->SetRelativeScale3D(Scale);
	Body->SetRelativeLocation(FVector(0.f, 0.f, Scale.Z * 50.f));
	if (MID)
	{
		MID->SetVectorParameterValue(TEXT("Color"), Color);
		Body->SetMaterial(0, MID);
	}

	Head->SetVisibility(true);
	if (UStaticMesh* Sphere = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Sphere.Sphere")))
	{
		Head->SetStaticMesh(Sphere);
	}
	Head->SetRelativeScale3D(FVector(0.5f, 0.5f, 0.5f));
	Head->SetRelativeLocation(FVector(0.9f, 0.f, 0.9f));
	if (BaseMaterial && Type == EAnimalType::Boar)
	{
		UMaterialInstanceDynamic* HeadMid = UMaterialInstanceDynamic::Create(BaseMaterial, this);
		HeadMid->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.2f, 0.16f, 0.14f));
		Head->SetMaterial(0, HeadMid);
	}
}

void AFarmAnimal::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (Health <= 0) return;

	APawn* PlayerPawn = nullptr;
	if (UWorld* World = GetWorld())
	{
		if (APlayerController* PC = World->GetFirstPlayerController())
		{
			PlayerPawn = PC->GetPawn();
		}
	}
	const FVector MyLocation = GetActorLocation();

	if (IsHostile() && PlayerPawn)
	{
		const float DistToPlayer = FVector::Dist2D(MyLocation, PlayerPawn->GetActorLocation());
		AttackCooldown = FMath::Max(0.f, AttackCooldown - DeltaSeconds);
		if (DistToPlayer < 700.f)
		{
			const FVector Dir = (PlayerPawn->GetActorLocation() - MyLocation).GetSafeNormal2D();
			SetActorLocation(MyLocation + Dir * 260.f * DeltaSeconds);
			if (DistToPlayer < 210.f && AttackCooldown <= 0.f)
			{
				AttackCooldown = 1.6f;
				if (AFarmCharacter* Farmer = Cast<AFarmCharacter>(PlayerPawn))
				{
					Farmer->ApplyDamage(8, this);
				}
			}
		}
		return;
	}

	WanderTimer -= DeltaSeconds;
	if (WanderTimer <= 0.f)
	{
		WanderTimer = FMath::FRandRange(2.f, 5.f);
		WanderDir = FVector(FMath::FRandRange(-1.f, 1.f), FMath::FRandRange(-1.f, 1.f), 0.f).GetSafeNormal();
	}
	FVector Next = MyLocation + WanderDir * 60.f * DeltaSeconds;
	if (FVector::Dist2D(Next, HomeLocation) > 450.f)
	{
		Next = HomeLocation + (Next - HomeLocation).GetSafeNormal() * 450.f;
	}
	SetActorLocation(Next);
}

FName AFarmAnimal::ProductItem() const
{
	return Type == EAnimalType::Cow ? FName(TEXT("milk")) : FName(TEXT("egg"));
}

FString AFarmAnimal::GetPrompt(const AFarmPlayerController* PC) const
{
	if (IsHostile())
	{
		return FString::Printf(TEXT("Wild boar (HP %d/%d) - Left click to attack"), Health, MaxHealth);
	}
	if (bProductReady)
	{
		return FString::Printf(TEXT("E: Collect %s"), *FarmItems::LabelOf(ProductItem()));
	}
	if (bFed)
	{
		return TEXT("Well fed - product ready tomorrow");
	}
	return TEXT("E: Feed hay (craft hay from corn with C)");
}

void AFarmAnimal::Interact(AFarmCharacter* Character)
{
	if (!Character || IsHostile() || Health <= 0) return;
	UFarmInventoryComponent* Inventory = Character->GetInventory();
	AFarmPlayerController* PC = Character->GetController<AFarmPlayerController>();
	if (!Inventory) return;

	if (bProductReady)
	{
		Inventory->AddItem(ProductItem(), 1);
		bProductReady = false;
		UE_LOG(LogTemp, Log, TEXT("FarmGame: collected %s"), *ProductItem().ToString());
		if (PC) PC->ShowToast(FString::Printf(TEXT("+1 %s"), *FarmItems::LabelOf(ProductItem())));
		return;
	}

	if (!bFed)
	{
		if (Inventory->RemoveItem(TEXT("hay"), 1))
		{
			bFed = true;
			UE_LOG(LogTemp, Log, TEXT("FarmGame: animal fed (%s)"), *UEnum::GetValueAsString(Type));
			if (PC) PC->ShowToast(TEXT("Animal fed"));
		}
		else if (PC)
		{
			PC->ShowToast(TEXT("Need hay (craft from corn)"));
		}
	}
}

void AFarmAnimal::RestoreState(bool bInFed, bool bInProduct, int32 InHealth)
{
	bFed = bInFed;
	bProductReady = bInProduct;
	Health = FMath::Clamp(InHealth, 1, MaxHealth);
}

void AFarmAnimal::AdvanceDay()
{
	if (IsHostile()) return;
	if (bFed)
	{
		bProductReady = true;
		bFed = false;
	}
}

void AFarmAnimal::ApplyDamage(int32 Amount, AFarmCharacter* Attacker)
{
	if (Health <= 0) return;
	Health -= Amount;
	FarmAudio::Play(this, TEXT("/Game/Art/Audio/flesh_hit.flesh_hit"), 0.7f);
	UE_LOG(LogTemp, Log, TEXT("FarmGame: animal hit for %d (hp %d)"), Amount, Health);
	if (Health > 0) return;
	UE_LOG(LogTemp, Log, TEXT("FarmGame: animal died"));

	if (Attacker && Attacker->GetInventory())
	{
		Attacker->GetInventory()->AddItem(IsHostile() ? FName(TEXT("meat")) : FName(TEXT("meat")), IsHostile() ? 2 : 1);
	}
	if (AFarmPlayerController* PC = Attacker ? Attacker->GetController<AFarmPlayerController>() : nullptr)
	{
		PC->ShowToast(FString::Printf(TEXT("+%d meat"), IsHostile() ? 2 : 1));
	}
	Destroy();
}
