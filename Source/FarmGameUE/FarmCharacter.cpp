#include "FarmCharacter.h"
#include "Animation/AnimSequence.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/SkeletalMesh.h"
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
	PrimaryActorTick.bCanEverTick = true;

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
	SpringArm->TargetArmLength = 1400.f;
	SpringArm->SetRelativeRotation(FRotator(-58.f, 0.f, 0.f));
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

	if (USkeletalMeshComponent* Skel = GetMesh())
	{
		if (USkeletalMesh* SkelAsset = LoadObject<USkeletalMesh>(nullptr, TEXT("/Game/Art/Character/FarmerBoySkel.FarmerBoySkel")))
		{
			Skel->SetSkeletalMesh(SkelAsset);
			const float MeshHeight = SkelAsset->GetBounds().BoxExtent.Z * 2.f;
			const float SkelScale = MeshHeight > 1.f ? 170.f / MeshHeight : 1.f;
			Skel->SetRelativeScale3D(FVector(SkelScale));
			Skel->SetRelativeLocation(FVector(0.f, 0.f, -88.f));
			Skel->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));
			Skel->SetVisibility(true, true);
			if (BodyMesh) BodyMesh->SetVisibility(false);

			IdleAnim = LoadObject<UAnimSequence>(nullptr, TEXT("/Game/Art/Character/character_idle.character_idle"));
			RunAnim = LoadObject<UAnimSequence>(nullptr, TEXT("/Game/Art/Character/character_run.character_run"));
			AttackAnim = LoadObject<UAnimSequence>(nullptr, TEXT("/Game/Art/Character/character_attack.character_attack"));
			if (IdleAnim) Skel->PlayAnimation(IdleAnim, true);
		}
		else if (FarmArt::ApplyHeight(BodyMesh, TEXT("/Game/Art/Character/FarmerBoy.FarmerBoy"), 170.f))
		{
			BodyMesh->SetRelativeLocation(FVector(0.f, 0.f, -88.f));
		}
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

void AFarmCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	USkeletalMeshComponent* Skel = GetMesh();
	if (!Skel || !Skel->GetSkeletalMeshAsset()) return;

	const float Now = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f;
	if (Now < AttackAnimUntil) return;

	const bool bMoving = GetVelocity().Size2D() > 20.f;
	if (bMoving == bRunningAnim && Skel->IsPlaying()) return;

	bRunningAnim = bMoving;
	if (bRunningAnim && RunAnim)
	{
		Skel->PlayAnimation(RunAnim, true);
	}
	else if (!bRunningAnim && IdleAnim)
	{
		Skel->PlayAnimation(IdleAnim, true);
	}
}

void AFarmCharacter::Heal(int32 Amount)
{
	Health = FMath::Clamp(Health + Amount, 0, MaxHealth);
	UE_LOG(LogTemp, Log, TEXT("FarmGame: healed %d -> HP %d"), Amount, Health);
}

void AFarmCharacter::PlayAttackAnim()
{
	if (!AttackAnim) return;
	if (USkeletalMeshComponent* Skel = GetMesh())
	{
		Skel->PlayAnimation(AttackAnim, false);
		AttackAnimUntil = GetWorld() ? GetWorld()->GetTimeSeconds() + 0.7f : 0.f;
	}
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
