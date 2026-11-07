#include "FarmCharacter.h"
#include "FarmArt.h"
#include "FarmGameMode.h"
#include "FarmInventoryComponent.h"
#include "FarmPlayerController.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"

AFarmCharacter::AFarmCharacter()
{
	PrimaryActorTick.bCanEverTick = false;

	GetCapsuleComponent()->InitCapsuleSize(42.f, 88.f);
	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 540.f, 0.f);
	GetCharacterMovement()->MaxWalkSpeed = 520.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;

	GetMesh()->SetVisibility(false);

	BodyMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Body"));
	BodyMesh->SetupAttachment(GetCapsuleComponent());
	BodyMesh->SetRelativeLocation(FVector(0.f, 0.f, -88.f));
	BodyMesh->SetRelativeScale3D(FVector(0.45f, 0.45f, 1.0f));
	BodyMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(RootComponent);
	SpringArm->TargetArmLength = 1500.f;
	SpringArm->SetRelativeRotation(FRotator(-62.f, 0.f, 0.f));
	SpringArm->bDoCollisionTest = false;
	SpringArm->bUsePawnControlRotation = false;
	SpringArm->bInheritPitch = false;
	SpringArm->bInheritYaw = false;
	SpringArm->bInheritRoll = false;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
	Camera->bUsePawnControlRotation = false;

	Inventory = CreateDefaultSubobject<UFarmInventoryComponent>(TEXT("Inventory"));
}

void AFarmCharacter::BeginPlay()
{
	Super::BeginPlay();
	HomeLocation = GetActorLocation();
	Health = MaxHealth;

	if (FarmArt::ApplyHeight(BodyMesh, TEXT("/Game/Art/Character/FarmerBoy.FarmerBoy"), 170.f))
	{
		BodyMesh->SetRelativeLocation(FVector(0.f, 0.f, -88.f));
	}
	UE_LOG(LogTemp, Log, TEXT("FarmGame: player spawned at %s"), *HomeLocation.ToCompactString());

	if (AFarmGameMode* GM = GetWorld() ? GetWorld()->GetAuthGameMode<AFarmGameMode>() : nullptr)
	{
		GM->InitializePlayer(this);
	}
}

void AFarmCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	PlayerInputComponent->BindAxis(TEXT("MoveForward"), this, &AFarmCharacter::MoveForward);
	PlayerInputComponent->BindAxis(TEXT("MoveRight"), this, &AFarmCharacter::MoveRight);
}

void AFarmCharacter::ApplyDamage(int32 Amount, AActor* Source)
{
	if (Amount <= 0 || IsDead()) return;
	Health = FMath::Max(0, Health - Amount);
	UE_LOG(LogTemp, Log, TEXT("FarmGame: damage %d from %s at distance %.0f -> HP %d"), Amount,
		*GetNameSafe(Source), Source ? FVector::Dist2D(Source->GetActorLocation(), GetActorLocation()) : -1.f, Health);
	if (Health <= 0)
	{
		Respawn();
	}
}

void AFarmCharacter::Respawn()
{
	SetActorLocation(HomeLocation);
	Health = MaxHealth;
	if (AFarmPlayerController* PC = GetController<AFarmPlayerController>())
	{
		PC->ShowToast(TEXT("You passed out... back at the farm"));
	}
}

void AFarmCharacter::MoveForward(float Value)
{
	if (FMath::Abs(Value) > KINDA_SMALL_NUMBER && Controller)
	{
		AddMovementInput(FVector(1.f, 0.f, 0.f), Value);
	}
}

void AFarmCharacter::MoveRight(float Value)
{
	if (FMath::Abs(Value) > KINDA_SMALL_NUMBER && Controller)
	{
		AddMovementInput(FVector(0.f, 1.f, 0.f), Value);
	}
}
