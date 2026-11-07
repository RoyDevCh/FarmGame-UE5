#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "FarmPlayerController.generated.h"

class AFarmPlot;
class AFarmBin;

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

protected:
	TWeakObjectPtr<AActor> CurrentTarget;
	int32 SelectedCropIndex = 0;
	FString ToastText;
	float ToastEndTime = 0.f;

	void UpdateTarget();
};
