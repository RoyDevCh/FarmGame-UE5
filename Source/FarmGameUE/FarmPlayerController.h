#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "FarmPlayerController.generated.h"

class AFarmPlot;
class AFarmBin;
class AFarmBuilding;
class SFarmInventoryWidget;
class SFarmMenuWidget;
class SFarmShopWidget;
class SFarmSettingsWidget;

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
	void ToggleInventory();
	void EatFood();
	bool IsInventoryOpen() const { return bInventoryOpen; }
	bool IsSlateInventoryActive() const { return InventoryWidget.IsValid(); }

	struct FShopEntry
	{
		FName Id;
		int32 Price = 0;
	};
	static const TArray<FShopEntry>& GetShopStock();
	void OpenShop();
	void CloseShop();
	void BuyFromShop(FName ItemId, int32 Price);
	void SellAllAtShop();
	void OpenSettings();
	void CloseSettings();
	void ToggleFullscreen();
	void CycleQuality();
	void CycleVolume();
	int32 GetMasterVolumePercent() const { return FMath::RoundToInt(MasterVolume * 100.f); }
	float GetMasterVolume() const { return MasterVolume; }
	bool IsShopOpen() const { return bShopOpen; }

	// Start / pause menu
	void HandleMenuPrimary();
	void HandleMenuEscape();
	void HandleMenuQuit();
	void HandleMenuNewGame();
	UFUNCTION(BlueprintCallable, Category = "Farm") void DebugNewGame();
	UFUNCTION(Exec) void FarmNewGame();
	void HandleMenuCycleSlot();
	int32 GetMenuSlot() const;
	bool HasSaveInSlot(int32 Slot) const;
	void ShowMenu(bool bStartMode);
	void HideMenu();
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
	bool bInventoryOpen = false;
	TSharedPtr<SFarmInventoryWidget> InventoryWidget;
	TSharedPtr<SFarmMenuWidget> MenuWidget;
	TSharedPtr<SFarmShopWidget> ShopWidget;
	TSharedPtr<SFarmSettingsWidget> SettingsWidget;
	bool bSettingsFromMenu = false;
	float MasterVolume = 1.f;
	bool bShopOpen = false;
	bool bMenuStartMode = true;
	float AttackCooldown = 0.f;
	UPROPERTY() AFarmBuilding* Ghost = nullptr;

	void UpdateTarget();
	void UpdateGhost();
	void PlaceBuilding();
	void ExitBuildMode();
};
