#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class AFarmPlayerController;
class STextBlock;
class SButton;

/** Full-screen start / pause menu. */
class FARMGAMEUE_API SFarmMenuWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SFarmMenuWidget) {}
		SLATE_ARGUMENT(TWeakObjectPtr<AFarmPlayerController>, PlayerController)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	/** Start mode shows "Start game"; pause mode shows "Resume". */
	void SetStartMode(bool bStart);

	virtual bool SupportsKeyboardFocus() const override { return true; }
	virtual FReply OnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

private:
	TWeakObjectPtr<AFarmPlayerController> PlayerController;
	TSharedPtr<STextBlock> TitleText;
	TSharedPtr<STextBlock> BodyText;
	TSharedPtr<SButton> PrimaryButton;
	TSharedPtr<STextBlock> PrimaryLabel;
};
