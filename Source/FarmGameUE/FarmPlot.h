#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FarmTypes.h"
#include "FarmPlot.generated.h"

class UStaticMeshComponent;
class UMaterialInstanceDynamic;
class AFarmCharacter;
class AFarmPlayerController;

/** One grid cell of farmland: till -> plant -> water -> grow -> harvest. */
UCLASS()
class FARMGAMEUE_API AFarmPlot : public AActor
{
	GENERATED_BODY()

public:
	AFarmPlot();

	void InitPlot(int32 InIndex, const FVector& InLocation);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Farm") int32 Index = 0;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Farm") EFarmPlotState State = EFarmPlotState::Untilled;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Farm") FName CropId = NAME_None;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Farm") int32 Stage = 0;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Farm") bool bWatered = false;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Farm") int32 LastGrowthDay = 0;

	void SetCropDef(const FCropDef* InCrop) { CropDef = InCrop; RefreshVisuals(); }

	/** E-key interaction; context depends on the current state. */
	void Interact(AFarmCharacter* Character);

	/** HUD text for the current state. */
	FString GetPrompt(const AFarmPlayerController* PC) const;

	/** Called by the game mode when a new day starts. */
	void AdvanceDay(int32 Today);
	/** Rain waters the plot for free. */
	void WaterFromRain();

	bool IsHarvestable() const { return State == EFarmPlotState::Harvestable; }

	FPlotSaveData CaptureState() const;
	void RestoreState(const FPlotSaveData& Data, const FCropDef* InCrop);

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere) USceneComponent* SceneRoot;
	UPROPERTY(VisibleAnywhere) UStaticMeshComponent* SoilMesh;
	UPROPERTY(VisibleAnywhere) UStaticMeshComponent* CropMesh;

	UPROPERTY(Transient) UMaterialInstanceDynamic* SoilMaterial = nullptr;
	UPROPERTY(Transient) UMaterialInstanceDynamic* CropMaterial = nullptr;
	UPROPERTY(Transient) UStaticMesh* FallbackCropMesh = nullptr;
	const FCropDef* CropDef = nullptr;
	bool bArtSoil = false;

	void RefreshVisuals();
	void ApplyColor(UMaterialInstanceDynamic* Material, const FLinearColor& Color) const;
};
