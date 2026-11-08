#include "FarmEquipWidget.h"

#include "FarmInventoryComponent.h"
#include "FarmItems.h"
#include "FarmPlayerController.h"

#include "InputCoreTypes.h"
#include "Styling/CoreStyle.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "FarmEquip"

void SFarmEquipWidget::Construct(const FArguments& InArgs)
{
	PlayerController = InArgs._PlayerController;
	Inventory = InArgs._Inventory;

	ChildSlot
	[
		SNew(SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
		.BorderBackgroundColor(FLinearColor(0.03f, 0.04f, 0.03f, 0.94f))
		[
			SNew(SBox).HAlign(HAlign_Center).VAlign(VAlign_Center)
			[
				SNew(SVerticalBox)

				+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0, 0, 0, 12)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("Title", "EQUIPMENT"))
					.ColorAndOpacity(FLinearColor(1.f, 0.88f, 0.45f))
					.Font(FCoreStyle::GetDefaultFontStyle("Bold", 30))
				]

				+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
				[
					SAssignNew(Rows, SVerticalBox)
				]

				+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0, 14, 0, 0)
				[
					SNew(SButton)
					.OnClicked_Lambda([this]()
					{
						if (AFarmPlayerController* PC = PlayerController.Get())
						{
							PC->CloseEquipPanel();
						}
						return FReply::Handled();
					})
					[
						SNew(SBox).WidthOverride(200.f).HeightOverride(34.f)
						[
							SNew(STextBlock)
							.Text(LOCTEXT("Close", "Close (Esc)"))
							.Justification(ETextJustify::Center)
						]
					]
				]
			]
		]
	];

	Refresh();
}

void SFarmEquipWidget::Refresh()
{
	if (!Rows.IsValid())
	{
		return;
	}

	Rows->ClearChildren();
	EquippedTexts.Empty();

	UFarmInventoryComponent* Bag = Inventory.Get();
	if (!Bag)
	{
		return;
	}

	for (const FName& ToolId : FarmEquip::ToolIds())
	{
		const int32 Count = Bag->GetCount(ToolId);
		const FName RowToolId = ToolId;
		TSharedPtr<STextBlock> EquippedText;

		Rows->AddSlot().AutoHeight().Padding(2.f)
		[
			SNew(SHorizontalBox)

			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0, 0, 10, 0)
			[
				SNew(SBox).WidthOverride(200.f)
				[
					SNew(STextBlock)
					.Text(FText::FromString(FarmEquip::RowLabel(ToolId, Count)))
					.ColorAndOpacity(FLinearColor(0.92f, 0.92f, 0.82f))
				]
			]

			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0, 0, 10, 0)
			[
				SAssignNew(EquippedText, STextBlock)
				.Text(FText::FromString(FarmEquip::EquippedSuffix()))
				.ColorAndOpacity(FarmEquip::EquippedColor())
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 12))
			]

			+ SHorizontalBox::Slot().AutoWidth()
			[
				SNew(SButton)
				.OnClicked_Lambda([this, RowToolId]()
				{
					EquipTool(RowToolId);
					return FReply::Handled();
				})
				[
					SNew(SBox).WidthOverride(80.f)
					[
						SNew(STextBlock)
						.Text(LOCTEXT("Equip", "Equip"))
						.Justification(ETextJustify::Center)
					]
				]
			]
		];

		EquippedTexts.Add(ToolId, EquippedText);
	}

	UpdateEquippedHighlight();
}

void SFarmEquipWidget::EquipTool(FName ToolId)
{
	if (AFarmPlayerController* PC = PlayerController.Get())
	{
		PC->EquipToolItem(ToolId);
	}

	UpdateEquippedHighlight();
}

void SFarmEquipWidget::UpdateEquippedHighlight()
{
	const UFarmInventoryComponent* Bag = Inventory.Get();
	const FName EquippedTool = Bag ? Bag->EquippedTool : NAME_None;

	for (const TPair<FName, TSharedPtr<STextBlock>>& Row : EquippedTexts)
	{
		if (Row.Value.IsValid())
		{
			Row.Value->SetVisibility(FarmEquip::IsEquipped(EquippedTool, Row.Key) ? EVisibility::Visible : EVisibility::Hidden);
		}
	}
}

FReply SFarmEquipWidget::OnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	if (InKeyEvent.GetKey() == EKeys::Escape)
	{
		if (AFarmPlayerController* PC = PlayerController.Get())
		{
			PC->CloseEquipPanel();
		}
		return FReply::Handled();
	}

	return SCompoundWidget::OnKeyDown(InGeometry, InKeyEvent);
}

#undef LOCTEXT_NAMESPACE
