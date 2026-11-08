#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FarmBuilding.generated.h"

class UStaticMeshComponent;
class AFarmCharacter;
class AFarmPlayerController;

UENUM()
enum class EBuildingType : uint8
{
	Fence,
	Furnace,
	Chest,
	Sprinkler
};

/** Player-placed building. Furnace smelts ore into ingots over time. */
UCLASS()
class FARMGAMEUE_API AFarmBuilding : public AActor
{
	GENERATED_BODY()

public:
	AFarmBuilding();

	void Configure(EBuildingType InType);
	void Interact(AFarmCharacter* Character);
	FString GetPrompt(const AFarmPlayerController* PC) const;
	virtual void Tick(float DeltaSeconds) override;

	EBuildingType GetBuildingType() const { return Type; }
	int32 GetOreCount() const { return OreCount; }
	int32 GetIngotCount() const { return IngotCount; }
	void RestoreState(int32 InOre, int32 InIngots);
	void RestoreStorage(const TMap<FName, int32>& InStorage) { Storage = InStorage; }

	const TMap<FName, int32>& GetStorage() const { return Storage; }
	TMap<FName, int32>& GetStorageMutable() { return Storage; }
	int32 GetStoredTotal() const;

	static int32 MaxOre() { return 5; }
	static float SmeltSecondsPerOre() { return 8.f; }

protected:
	UPROPERTY(VisibleAnywhere) USceneComponent* SceneRoot;
	UPROPERTY(VisibleAnywhere) UStaticMeshComponent* Mesh;
	UPROPERTY(VisibleAnywhere) UStaticMeshComponent* Detail;

	EBuildingType Type = EBuildingType::Fence;
	int32 OreCount = 0;
	int32 IngotCount = 0;
	float SmeltTimer = 0.f;
	UPROPERTY() TMap<FName, int32> Storage;

	void ApplyVisual();
};
