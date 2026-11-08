#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class AFarmPlayerController;
class STextBlock;

/** Minimal settings panel: fullscreen toggle and overall quality. */
class FARMGAMEUE_API SFarmSettingsWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SFarmSettingsWidget) {}
		SLATE_ARGUMENT(TWeakObjectPtr<AFarmPlayerController>, PlayerController)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	void Refresh();

	virtual bool SupportsKeyboardFocus() const override { return true; }
	virtual FReply OnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

private:
	TWeakObjectPtr<AFarmPlayerController> PlayerController;
	TSharedPtr<STextBlock> FullscreenLabel;
	TSharedPtr<STextBlock> QualityLabel;
	TSharedPtr<STextBlock> VolumeLabel;
};
