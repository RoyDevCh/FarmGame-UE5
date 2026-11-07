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

protected:
	UPROPERTY(VisibleAnywhere, Category = "Farm") USpringArmComponent* SpringArm;
	UPROPERTY(VisibleAnywhere, Category = "Farm") UCameraComponent* Camera;
	UPROPERTY(VisibleAnywhere, Category = "Farm") UStaticMeshComponent* BodyMesh;
	UPROPERTY(VisibleAnywhere, Category = "Farm") UFarmInventoryComponent* Inventory;

	void MoveForward(float Value);
	void MoveRight(float Value);
};
