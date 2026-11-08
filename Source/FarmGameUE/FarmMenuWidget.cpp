#include "FarmMenuWidget.h"
#include "FarmPlayerController.h"

#include "Widgets/SBoxPanel.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Styling/CoreStyle.h"

#define LOCTEXT_NAMESPACE "FarmMenu"

void SFarmMenuWidget::Construct(const FArguments& InArgs)
{
	PlayerController = InArgs._PlayerController;

	ChildSlot
	[
		SNew(SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
		.BorderBackgroundColor(FLinearColor(0.02f, 0.03f, 0.02f, 0.93f))
		[
			SNew(SBox).HAlign(HAlign_Center).VAlign(VAlign_Center)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0, 0, 0, 8)
				[
					SAssignNew(TitleText, STextBlock)
					.Text(LOCTEXT("Title", "FARM GAME"))
					.ColorAndOpacity(FLinearColor(1.f, 0.88f, 0.45f))
					.Font(FCoreStyle::GetDefaultFontStyle("Bold", 42))
				]
				+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0, 0, 0, 26)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("Body", "WASD move  |  E interact  |  F attack  |  H eat  |  Q seed  |  B buy  |  C craft  |  G build  |  Tab bag  |  T sleep  |  Esc pause\nC continue  |  N new game  |  S save slot  |  Q quit"))
					.ColorAndOpacity(FLinearColor(0.8f, 0.85f, 0.78f))
				]
				+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0, 4)
				[
					SAssignNew(PrimaryButton, SButton)
					.OnClicked_Lambda([this]() { if (PlayerController.IsValid()) PlayerController->HandleMenuPrimary(); return FReply::Handled(); })
					[
						SNew(SBox).WidthOverride(280.f).HeightOverride(46.f)
						[
							SAssignNew(PrimaryLabel, STextBlock)
							.Text(LOCTEXT("Continue", "Continue"))
							.Justification(ETextJustify::Center)
							.Font(FCoreStyle::GetDefaultFontStyle("Bold", 18))
						]
					]
				]
				+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0, 4)
				[
					SAssignNew(NewGameButton, SButton)
					.OnClicked_Lambda([this]() { if (PlayerController.IsValid()) PlayerController->HandleMenuNewGame(); return FReply::Handled(); })
					[
						SNew(SBox).WidthOverride(280.f).HeightOverride(36.f)
						[
							SNew(STextBlock).Text(LOCTEXT("NewGame", "New game")).Justification(ETextJustify::Center)
						]
					]
				]
				+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0, 4)
				[
					SNew(SButton)
					.OnClicked_Lambda([this]() { if (PlayerController.IsValid()) PlayerController->HandleMenuCycleSlot(); return FReply::Handled(); })
					[
						SNew(SBox).WidthOverride(280.f).HeightOverride(32.f)
						[
							SAssignNew(SlotLabel, STextBlock)
							.Text(LOCTEXT("Slot", "Save slot: 1"))
							.Justification(ETextJustify::Center)
						]
					]
				]
				+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0, 4)
				[
					SNew(SButton)
					.OnClicked_Lambda([this]() { if (PlayerController.IsValid()) PlayerController->HandleMenuQuit(); return FReply::Handled(); })
					[
						SNew(SBox).WidthOverride(280.f).HeightOverride(32.f)
						[
							SNew(STextBlock).Text(LOCTEXT("Quit", "Quit")).Justification(ETextJustify::Center)
						]
					]
				]
			]
		]
	];

	RefreshLabels();
}

void SFarmMenuWidget::SetStartMode(bool bStart)
{
	bStartMode = bStart;
	if (TitleText.IsValid()) TitleText->SetText(bStart ? LOCTEXT("StartTitle", "FARM GAME") : LOCTEXT("PauseTitle", "PAUSED"));
	if (PrimaryLabel.IsValid()) PrimaryLabel->SetText(bStart ? LOCTEXT("Continue", "Continue") : LOCTEXT("Resume", "Resume"));
	if (NewGameButton.IsValid()) NewGameButton->SetVisibility(bStart ? EVisibility::Visible : EVisibility::Collapsed);
	RefreshLabels();
}

void SFarmMenuWidget::RefreshLabels()
{
	if (!PlayerController.IsValid()) return;
	const int32 Slot = PlayerController->GetMenuSlot();
	const bool bHasSave = PlayerController->HasSaveInSlot(Slot);

	if (SlotLabel.IsValid())
	{
		SlotLabel->SetText(FText::FromString(FString::Printf(TEXT("Save slot: %d%s"), Slot, bHasSave ? TEXT("  (has save)") : TEXT("  (empty)"))));
	}
	if (PrimaryButton.IsValid())
	{
		PrimaryButton->SetEnabled(bStartMode ? bHasSave : true);
	}
}

FReply SFarmMenuWidget::OnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	const FKey Key = InKeyEvent.GetKey();
	if (PlayerController.IsValid())
	{
		if (Key == EKeys::Escape)
		{
			PlayerController->HandleMenuEscape();
			return FReply::Handled();
		}
		if (Key == EKeys::Enter || Key == EKeys::SpaceBar || Key == EKeys::C)
		{
			PlayerController->HandleMenuPrimary();
			return FReply::Handled();
		}
		if (Key == EKeys::N)
		{
			PlayerController->HandleMenuNewGame();
			return FReply::Handled();
		}
		if (Key == EKeys::S)
		{
			PlayerController->HandleMenuCycleSlot();
			return FReply::Handled();
		}
		if (Key == EKeys::Q)
		{
			PlayerController->HandleMenuQuit();
			return FReply::Handled();
		}
	}
	return SCompoundWidget::OnKeyDown(InGeometry, InKeyEvent);
}

#undef LOCTEXT_NAMESPACE
