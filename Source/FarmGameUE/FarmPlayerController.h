#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "FarmPlayerController.generated.h"

class AFarmPlot;
class AFarmBin;
class AFarmBuilding;

UCLASS()
class FARMGAMEUE_API AFarmPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AFarmPlayerController();

	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
	virtual void Tick(float DeltaSeconds) override;

	FName GetSelectedCropId() const;
	FString GetSelectedCropLabel() const;
	FString GetPromptText() const;

	void ShowToast(const FString& Text);
	const FString& GetToastText() const { return ToastText; }
	float GetToastAlpha() const;

	AActor* GetInteractTarget() const { return CurrentTarget.Get(); }

	void Interact();
	void CycleSeed();
	void BuySeed();
	void SaveGame();
	void LoadGame();
	void Sleep();

	void Attack();
	void ToggleBuildMode();
	void ToggleCraftMenu();
	void CancelMode();
	void CraftRecipe(int32 Index);
	void Craft1();
	void Craft2();
	void Craft3();
	void Craft4();
	void Craft5();

	bool IsBuildMode() const { return BuildModeIndex >= 0; }
	bool IsCraftMenuOpen() const { return bCraftMenuOpen; }
	int32 GetBuildModeIndex() const { return BuildModeIndex; }
	FString GetBuildLabel() const;
	bool IsNearFurnace() const;
	bool CanCraftRecipe(int32 Index) const;

protected:
	TWeakObjectPtr<AActor> CurrentTarget;
	int32 SelectedCropIndex = 0;
	FString ToastText;
	float ToastEndTime = 0.f;

	int32 BuildModeIndex = -1;
	bool bCraftMenuOpen = false;
	float AttackCooldown = 0.f;
	UPROPERTY() AFarmBuilding* Ghost = nullptr;

	void UpdateTarget();
	void UpdateGhost();
	void PlaceBuilding();
	void ExitBuildMode();
};
