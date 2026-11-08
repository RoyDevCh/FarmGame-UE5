#include "FarmStorageWidget.h"
#include "FarmBuilding.h"
#include "FarmCharacter.h"
#include "FarmInventoryComponent.h"
#include "FarmItems.h"
#include "FarmPlayerController.h"

#include "Widgets/SBoxPanel.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Styling/CoreStyle.h"

#define LOCTEXT_NAMESPACE "FarmStorage"

void SFarmStorageWidget::Construct(const FArguments& InArgs)
{
	PlayerController = InArgs._PlayerController;

	ChildSlot
	[
		SNew(SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
		.BorderBackgroundColor(FLinearColor(0.03f, 0.04f, 0.03f, 0.94f))
		[
			SNew(SBox).HAlign(HAlign_Center).VAlign(VAlign_Center)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0, 0, 0, 6)
				[
					SNew(STextBlock).Text(LOCTEXT("Title", "STORAGE")).ColorAndOpacity(FLinearColor(1.f, 0.88f, 0.45f)).Font(FCoreStyle::GetDefaultFontStyle("Bold", 30))
				]
				+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0, 0, 0, 12)
				[
					SAssignNew(Summary, STextBlock).Text(LOCTEXT("Empty", "Empty"))
				]
				+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
				[
					SAssignNew(Rows, SVerticalBox)
				]
				+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0, 12, 0, 0)
				[
					SNew(SButton)
					.OnClicked_Lambda([this]() { if (PlayerController.IsValid()) PlayerController->StorageDepositAll(); return FReply::Handled(); })
					[
						SNew(SBox).WidthOverride(280.f).HeightOverride(36.f)
						[
							SNew(STextBlock).Text(LOCTEXT("Deposit", "Deposit all resources")).Justification(ETextJustify::Center)
						]
					]
				]
				+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0, 4)
				[
					SNew(SButton)
					.OnClicked_Lambda([this]() { if (PlayerController.IsValid()) PlayerController->StorageWithdrawAll(); return FReply::Handled(); })
					[
						SNew(SBox).WidthOverride(280.f).HeightOverride(36.f)
						[
							SNew(STextBlock).Text(LOCTEXT("Withdraw", "Withdraw all")).Justification(ETextJustify::Center)
						]
					]
				]
				+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0, 10, 0, 0)
				[
					SNew(SButton)
					.OnClicked_Lambda([this]() { if (PlayerController.IsValid()) PlayerController->CloseStorage(); return FReply::Handled(); })
					[
						SNew(SBox).WidthOverride(200.f).HeightOverride(34.f)
						[
							SNew(STextBlock).Text(LOCTEXT("Close", "Close (Esc)")).Justification(ETextJustify::Center)
						]
					]
				]
			]
		]
	];
}

void SFarmStorageWidget::Refresh()
{
	if (!Rows.IsValid()) return;
	Rows->ClearChildren();
	if (!Chest.IsValid()) return;
	const TMap<FName, int32>& Storage = Chest->GetStorage();
	int32 Total = 0;
	for (const TPair<FName, int32>& Pair : Storage)
	{
		Total += Pair.Value;
		Rows->AddSlot().AutoHeight().Padding(1.f)
		[
			SNew(STextBlock).Text(FText::FromString(FString::Printf(TEXT("%s  x%d"), *FarmItems::LabelOf(Pair.Key), Pair.Value)))
		];
	}
	if (Summary.IsValid())
	{
		Summary->SetText(FText::FromString(Total > 0 ? FString::Printf(TEXT("%d items stored"), Total) : TEXT("Empty")));
	}
}

FReply SFarmStorageWidget::OnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	if (InKeyEvent.GetKey() == EKeys::Escape && PlayerController.IsValid())
	{
		PlayerController->CloseStorage();
		return FReply::Handled();
	}
	return SCompoundWidget::OnKeyDown(InGeometry, InKeyEvent);
}

#undef LOCTEXT_NAMESPACE
