#include "FarmShopWidget.h"
#include "FarmCharacter.h"
#include "FarmInventoryComponent.h"
#include "FarmItems.h"
#include "FarmPlayerController.h"

#include "Widgets/SBoxPanel.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Images/SImage.h"
#include "FarmIcon.h"
#include "Widgets/Input/SButton.h"
#include "Styling/CoreStyle.h"

#define LOCTEXT_NAMESPACE "FarmShop"

void SFarmShopWidget::Construct(const FArguments& InArgs)
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
					SNew(STextBlock).Text(LOCTEXT("Title", "SHOP"))
					.ColorAndOpacity(FLinearColor(1.f, 0.88f, 0.45f))
					.Font(FCoreStyle::GetDefaultFontStyle("Bold", 30))
				]
				+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0, 0, 0, 12)
				[
					SAssignNew(GoldText, STextBlock)
					.Text(LOCTEXT("Gold", "Gold: 0"))
					.ColorAndOpacity(FLinearColor(0.9f, 0.9f, 0.8f))
				]
				+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
				[
					SAssignNew(Rows, SVerticalBox)
				]
				+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0, 10, 0, 0)
				[
					SNew(SButton)
					.OnClicked_Lambda([this]() { if (PlayerController.IsValid()) PlayerController->SellAllAtShop(); return FReply::Handled(); })
					[
						SNew(SBox).WidthOverride(260.f).HeightOverride(34.f)
						[
							SNew(STextBlock).Text(LOCTEXT("SellAll", "Sell all goods")).Justification(ETextJustify::Center)
						]
					]
				]
				+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0, 14, 0, 0)
				[
					SNew(SButton)
					.OnClicked_Lambda([this]() { if (PlayerController.IsValid()) PlayerController->CloseShop(); return FReply::Handled(); })
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

void SFarmShopWidget::Refresh()
{
	if (!Rows.IsValid()) return;
	Rows->ClearChildren();

	AFarmPlayerController* PC = PlayerController.Get();
	AFarmCharacter* Farmer = PC ? Cast<AFarmCharacter>(PC->GetPawn()) : nullptr;
	UFarmInventoryComponent* Inventory = Farmer ? Farmer->GetInventory() : nullptr;
	if (!Inventory) return;

	if (GoldText.IsValid())
	{
		GoldText->SetText(FText::FromString(FString::Printf(TEXT("Gold: %d"), Inventory->Gold)));
	}

	for (const AFarmPlayerController::FShopEntry& Entry : AFarmPlayerController::GetShopStock())
	{
		const bool bAfford = Inventory->Gold >= Entry.Price;
		const TSharedPtr<FSlateBrush> Icon = FarmIcon::BrushFor(Entry.Id);
		Rows->AddSlot().AutoHeight().Padding(2.f)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0, 0, 8, 0)
			[
				SNew(SImage)
				.Image_Lambda([Icon]() { return Icon.IsValid() ? Icon.Get() : FCoreStyle::Get().GetBrush("WhiteBrush"); })
				.ColorAndOpacity(Icon.IsValid() ? FLinearColor::White : FLinearColor(0.f, 0.f, 0.f, 0.f))
			]
			+ SHorizontalBox::Slot().AutoWidth().Padding(0, 0, 12, 0).VAlign(VAlign_Center)
			[
				SNew(SBox).WidthOverride(230.f)
				[
					SNew(STextBlock)
					.Text(FText::FromString(FString::Printf(TEXT("%s  -  %dg"), *FarmItems::LabelOf(Entry.Id), Entry.Price)))
				]
			]
			+ SHorizontalBox::Slot().AutoWidth()
			[
				SNew(SButton)
				.IsEnabled(bAfford)
				.OnClicked_Lambda([PC, Entry]() { if (PC) PC->BuyFromShop(Entry.Id, Entry.Price); return FReply::Handled(); })
				[
					SNew(SBox).WidthOverride(80.f)
					[
						SNew(STextBlock).Text(LOCTEXT("Buy", "Buy")).Justification(ETextJustify::Center)
					]
				]
			]
		];
	}
}

FReply SFarmShopWidget::OnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	if (InKeyEvent.GetKey() == EKeys::Escape && PlayerController.IsValid())
	{
		PlayerController->CloseShop();
		return FReply::Handled();
	}
	return SCompoundWidget::OnKeyDown(InGeometry, InKeyEvent);
}

#undef LOCTEXT_NAMESPACE
