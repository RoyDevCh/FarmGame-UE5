#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class AFarmPlayerController;
class SVerticalBox;

/** Slate inventory + crafting panel (Tab). */
class FARMGAMEUE_API SFarmInventoryWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SFarmInventoryWidget) {}
		SLATE_ARGUMENT(TWeakObjectPtr<AFarmPlayerController>, PlayerController)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	/** Rebuilds the item rows and craft buttons from the current player state. */
	void Refresh();

private:
	TWeakObjectPtr<AFarmPlayerController> PlayerController;
	TSharedPtr<SVerticalBox> ItemList;
	TSharedPtr<SVerticalBox> CraftList;
};
