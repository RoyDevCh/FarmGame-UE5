#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class AFarmPlayerController;
class SVerticalBox;
class STextBlock;

/** Shop panel: list of buyable goods with prices. */
class FARMGAMEUE_API SFarmShopWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SFarmShopWidget) {}
		SLATE_ARGUMENT(TWeakObjectPtr<AFarmPlayerController>, PlayerController)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	void Refresh();

	virtual bool SupportsKeyboardFocus() const override { return true; }
	virtual FReply OnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

private:
	TWeakObjectPtr<AFarmPlayerController> PlayerController;
	TSharedPtr<SVerticalBox> Rows;
	TSharedPtr<STextBlock> GoldText;
};
