#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class AFarmPlayerController;
class STextBlock;
class SButton;

/** Full-screen start / pause menu with save slot selection. */
class FARMGAMEUE_API SFarmMenuWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SFarmMenuWidget) {}
		SLATE_ARGUMENT(TWeakObjectPtr<AFarmPlayerController>, PlayerController)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	void SetStartMode(bool bStart);
	void RefreshLabels();

	virtual bool SupportsKeyboardFocus() const override { return true; }
	virtual FReply OnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

private:
	TWeakObjectPtr<AFarmPlayerController> PlayerController;
	TSharedPtr<STextBlock> TitleText;
	TSharedPtr<STextBlock> PrimaryLabel;
	TSharedPtr<SButton> PrimaryButton;
	TSharedPtr<SButton> NewGameButton;
	TSharedPtr<STextBlock> SlotLabel;
	bool bStartMode = true;
};
