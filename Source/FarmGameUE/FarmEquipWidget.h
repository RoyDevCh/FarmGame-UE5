#pragma once

#include "CoreMinimal.h"
#include "FarmItems.h"
#include "Widgets/SCompoundWidget.h"

class AFarmPlayerController;
class UFarmInventoryComponent;
class SVerticalBox;
class STextBlock;

/**
 * Pure, Slate-free decisions behind the equipment panel. They stay inline in the
 * header so automation tests can exercise them without building a widget.
 */
namespace FarmEquip
{
	/** The four tool ids, in panel row order. */
	inline const TArray<FName>& ToolIds()
	{
		static const TArray<FName> Tools = {
			TEXT("axe"),
			TEXT("pickaxe"),
			TEXT("steel_axe"),
			TEXT("steel_pickaxe")
		};
		return Tools;
	}

	/** True when the inventory's EquippedTool is ToolId; NAME_None means nothing is equipped. */
	inline bool IsEquipped(FName EquippedTool, FName ToolId)
	{
		return !EquippedTool.IsNone() && EquippedTool == ToolId;
	}

	/** Row label, e.g. "Axe x2". Negative counts are clamped to 0. */
	inline FString RowLabel(FName ToolId, int32 Count)
	{
		return FString::Printf(TEXT("%s x%d"), *FarmItems::LabelOf(ToolId), FMath::Max(Count, 0));
	}

	/** Suffix appended to the equipped row. */
	inline FString EquippedSuffix()
	{
		return TEXT("EQUIPPED");
	}

	/** Highlight colour of the equipped suffix (yellow). */
	inline FLinearColor EquippedColor()
	{
		return FLinearColor(1.f, 0.85f, 0.2f, 1.f);
	}
}

/**
 * Slate equipment panel: a dark board listing the four tools with their inventory
 * counts and an Equip button per row. The equipped row keeps a yellow "EQUIPPED"
 * suffix. Escape or the close button calls the player controller's CloseEquipPanel().
 */
class FARMGAMEUE_API SFarmEquipWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SFarmEquipWidget) {}
		SLATE_ARGUMENT(TWeakObjectPtr<AFarmPlayerController>, PlayerController)
		SLATE_ARGUMENT(TWeakObjectPtr<UFarmInventoryComponent>, Inventory)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	/** Rebuilds the tool rows from the current inventory. Call it after inventory changes. */
	void Refresh();

	virtual bool SupportsKeyboardFocus() const override { return true; }
	virtual FReply OnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

private:
	/** Sends EquipToolItem to the controller, then updates the row highlight. */
	void EquipTool(FName ToolId);

	/** Re-colours the rows from Inventory->EquippedTool without rebuilding the tree. */
	void UpdateEquippedHighlight();

	TWeakObjectPtr<AFarmPlayerController> PlayerController;
	TWeakObjectPtr<UFarmInventoryComponent> Inventory;
	TSharedPtr<SVerticalBox> Rows;

	/** One suffix text block per tool id, kept so the highlight can update in place. */
	TMap<FName, TSharedPtr<STextBlock>> EquippedTexts;
};
