#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class AFarmPlayerController;
class AFarmBuilding;
class SVerticalBox;
class STextBlock;

/** Chest storage panel: deposit / withdraw all. */
class FARMGAMEUE_API SFarmStorageWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SFarmStorageWidget) {}
		SLATE_ARGUMENT(TWeakObjectPtr<AFarmPlayerController>, PlayerController)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	void SetChest(AFarmBuilding* InChest) { Chest = InChest; }
	void Refresh();

	virtual bool SupportsKeyboardFocus() const override { return true; }
	virtual FReply OnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

private:
	TWeakObjectPtr<AFarmPlayerController> PlayerController;
	TWeakObjectPtr<AFarmBuilding> Chest;
	TSharedPtr<SVerticalBox> Rows;
	TSharedPtr<STextBlock> Summary;
};
