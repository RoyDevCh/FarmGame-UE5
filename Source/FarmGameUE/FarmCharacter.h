#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "FarmCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UStaticMeshComponent;
class UFarmInventoryComponent;

/** Top-down farmer pawn. */
UCLASS()
class FARMGAMEUE_API AFarmCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AFarmCharacter();

	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	UFarmInventoryComponent* GetInventory() const { return Inventory; }

	void ApplyDamage(int32 Amount, AActor* Source);
	void Heal(int32 Amount);
	void PlayAttackAnim();
	void UpdateToolVisual();
	virtual void Tick(float DeltaSeconds) override;
	void SetHealth(int32 InHealth) { Health = FMath::Clamp(InHealth, 0, MaxHealth); }
	void Respawn();
	int32 GetHealth() const { return Health; }
	int32 GetMaxHealth() const { return MaxHealth; }
	bool IsDead() const { return Health <= 0; }

protected:
	UPROPERTY(VisibleAnywhere, Category = "Farm") USpringArmComponent* SpringArm;
	UPROPERTY(VisibleAnywhere, Category = "Farm") UCameraComponent* Camera;
	UPROPERTY(VisibleAnywhere, Category = "Farm") UStaticMeshComponent* BodyMesh;
	UPROPERTY(VisibleAnywhere, Category = "Farm") UStaticMeshComponent* ToolMesh;
	UPROPERTY(VisibleAnywhere, Category = "Farm") UFarmInventoryComponent* Inventory;

	UPROPERTY(VisibleAnywhere, Category = "Farm") int32 Health = 100;
	UPROPERTY(VisibleAnywhere, Category = "Farm") int32 MaxHealth = 100;
	FVector HomeLocation = FVector::ZeroVector;
	UPROPERTY() class UAnimSequence* IdleAnim = nullptr;
	UPROPERTY() class UAnimSequence* RunAnim = nullptr;
	UPROPERTY() class UAnimSequence* AttackAnim = nullptr;
	bool bRunningAnim = false;
	float AttackAnimUntil = 0.f;

	void MoveForward(float Value);
	void MoveRight(float Value);
};
