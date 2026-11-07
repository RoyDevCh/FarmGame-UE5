#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FarmAnimal.generated.h"

class UStaticMeshComponent;
class AFarmCharacter;
class AFarmPlayerController;

UENUM()
enum class EAnimalType : uint8
{
	Chicken,
	Cow,
	Boar
};

/** Livestock that produces goods when fed, plus hostile boars. */
UCLASS()
class FARMGAMEUE_API AFarmAnimal : public AActor
{
	GENERATED_BODY()

public:
	AFarmAnimal();

	void Configure(EAnimalType InType, const FVector& InHome);
	virtual void Tick(float DeltaSeconds) override;

	void Interact(AFarmCharacter* Character);
	FString GetPrompt(const AFarmPlayerController* PC) const;
	void AdvanceDay();
	void ApplyDamage(int32 Amount, AFarmCharacter* Attacker);

	EAnimalType GetType() const { return Type; }
	int32 GetHealth() const { return Health; }
	int32 GetMaxHealth() const { return MaxHealth; }
	bool IsHostile() const { return Type == EAnimalType::Boar; }
	bool HasProduct() const { return bProductReady; }
	bool IsFed() const { return bFed; }

	FVector HomeLocation = FVector::ZeroVector;

	void RestoreState(bool bInFed, bool bInProduct, int32 InHealth);

protected:
	UPROPERTY(VisibleAnywhere) USceneComponent* SceneRoot;
	UPROPERTY(VisibleAnywhere) UStaticMeshComponent* Body;
	UPROPERTY(VisibleAnywhere) UStaticMeshComponent* Head;

	EAnimalType Type = EAnimalType::Chicken;
	int32 Health = 30;
	int32 MaxHealth = 30;
	bool bFed = false;
	bool bProductReady = false;
	float AttackCooldown = 0.f;
	float WanderTimer = 0.f;
	FVector WanderDir = FVector::ZeroVector;

	void ApplyVisual();
	FName ProductItem() const;
};
